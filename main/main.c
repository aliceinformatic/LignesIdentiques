#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include "holdall.h"
#include "hashtable.h"
#include "line.h"

#define OPT_CHAR '-'
#define OPT_FILTER_SHORT "-f"
#define OPT_SORT_SHORT "-s"
#define OPT_UPPERCASING_SHORT "-u"
#define OPT_HELP_SHORT "-h"
#define OPT_FILTER "--filter="
#define OPT_SORT "--sort="
#define OPT_UPPERCASING "--uppercasing"
#define OPT_HELP "--help"

#define DEFAULT_SIZE 10
#define MUL 2

#define CHECK_FN_SIZE(filenames, files, fn_size, fn_length)   \
  if (fn_size == fn_length) {                                 \
    fn_size *= MUL;                                           \
    void *tmp = realloc(filenames, sizeof(char *) * fn_size); \
    if (tmp == NULL) {                                        \
      goto malloc_error;                                      \
    }                                                         \
    filenames = tmp;                                          \
    tmp = realloc(files, sizeof(FILE *) * fn_size);           \
    if (tmp == NULL) {                                        \
      goto malloc_error;                                      \
    }                                                         \
    files = tmp;                                              \
  }

// print_size_t_tab, print_size_t_comma: affiche un size_t
// suivi respectivement d'une tab ou d'une virgule
void print_size_t_tab(size_t n);
void print_size_t_comma(size_t n);

// lptr_hfun(a) : fonction de hachage pour les pointeurs de pointeurs
//  de line
size_t lptr_hfun(const void *a);

// close_files(files, length) : ferme tous les fichiers du tableau
//    de fichier files de longueur length. Renvoie 0 si tout s'est bien
//    passé, -1 sinon.
int close_files(FILE **files, size_t length);

// lptrcmp_sd(a, b), lptrcmp_lc(a, b) comparent respectivement deux pointeurs de
// pointeurs de line
//  selon la fonction strcmp ou la fonction strcoll.
int lptrcmp_sd(const void *a, const void *b);
int lptrcmp_lc(const void *a, const void *b);

// free_holdall(a) : libère les ressources associées à a puis renvoie 0
int free_holdall(void *a);
// free_holdall_mult(a) : affiche la line a dans le cas où il y aurait plusieurs
// fichiers
//  puis libère les ressources qui lui sont associé et renvoi 0
int free_holdall_mult(void *a);
// free_holdall_single(a) : affiche la line a dans le cas où il y aurait un seul
// fichier
//  puis libère les ressources qui lui sont associé et renvoi 0
int free_holdall_single(void *a);

int main(int argc, char *argv[]) {
  size_t fn_size = DEFAULT_SIZE;
  size_t fn_length = 0;
  char **filenames = malloc(sizeof(char *) * fn_size);
  FILE **files = malloc(sizeof(FILE *) * fn_size);
  if (argc < 2) {
    goto syntax_error;
  }
  int fstdin = 0;
  int upp = 0;
  setlocale(LC_ALL, "");
  int (*lptrcmp)(const void *, const void *) = lptrcmp_sd;
  int (*filter)(int) = NULL;
  const char *type[12] = {
    "alpha", "alnum", "blank", "cntrl", "digit",
    "graph", "lower", "print", "punct", "space", "upper", "xdigit"
  };
  int (*filter_type[12])(int) = {
    isalpha, isalnum, isblank, iscntrl,
    isdigit, isgraph, islower, isprint, ispunct, isspace, isupper, isxdigit
  };
  for (size_t i = 1; i < (size_t) argc; i++) {
    if (strcmp(argv[i], OPT_FILTER_SHORT) == 0) {
      if (i + 1 >= (size_t) argc) {
        goto syntax_error;
      }
      i++;
      char *option = argv[i];
      for (int j = 0; j < 12; j++) {
        if (strcmp(option, type[j]) == 0) {
          filter = filter_type[j];
        }
      }
      if (filter == NULL) {
        fprintf(stderr, "Error: option filter %s unknown\n", option);
        goto syntax_error;
      }
    } else if (strncmp(argv[i], OPT_FILTER, strlen(OPT_FILTER) - 1) == 0) {
      char *option = argv[i] + strlen(OPT_FILTER);
      for (int j = 0; j < 12; j++) {
        if (strcmp(option, type[j]) == 0) {
          filter = filter_type[j];
        }
      }
      if (filter == NULL) {
        fprintf(stderr, "Error: option filter %s unknown\n", option);
        goto syntax_error;
      }
    } else if (strcmp(argv[i], OPT_SORT_SHORT) == 0) {
      if (i + 1 >= (size_t) argc) {
        goto syntax_error;
      }
      i++;
      char *option = argv[i];
      if (strcmp(option, (char *) "standard") == 0) {
        lptrcmp = lptrcmp_sd;
      } else if (strcmp(option, (char *) "local") == 0) {
        lptrcmp = lptrcmp_lc;
      } else {
        fprintf(stderr, "Error: option sort %s unknown\n", option);
        goto syntax_error;
      }
    } else if (strncmp(argv[i], OPT_SORT, strlen(OPT_SORT) - 1) == 0) {
      char *option = argv[i] + strlen(OPT_SORT);
      if (strcmp(option, (char *) "standard") == 0) {
        lptrcmp = lptrcmp_sd;
      } else if (strcmp(option, (char *) "local") == 0) {
        lptrcmp = lptrcmp_lc;
      } else {
        fprintf(stderr, "Error: option sort %s unknown\n", option);
        goto syntax_error;
      }
    } else if (strcmp(argv[i], OPT_UPPERCASING_SHORT) == 0
        || strcmp(argv[i], OPT_UPPERCASING) == 0) {
      upp = 1;
    } else if (strcmp(argv[i], OPT_HELP_SHORT) == 0
        || strcmp(argv[i], OPT_HELP) == 0) {
      goto help;
    } else if (argv[i][0] == OPT_CHAR && argv[i][1] == '\0' && fstdin == 0) {
      fstdin = 1;
      CHECK_FN_SIZE(filenames, files, fn_size, fn_length)
      filenames[fn_length] = (char *) "stdin";
      files[fn_length] = stdin;
      fn_length++;
    } else {
      for (size_t j = 0; j < fn_length; j++) {
        if (strcmp(argv[i], filenames[j]) == 0) {
          fprintf(stderr, "Error: file %s already given\n", argv[i]);
          goto syntax_error;
        }
      }
      CHECK_FN_SIZE(filenames, files, fn_size, fn_length)
      filenames[fn_length] = argv[i];
      files[fn_length] = fopen(argv[i], "r");
      if (files[fn_length] == NULL) {
        goto file_error;
      }
      fn_length++;
    }
  }
  if (fn_length == 0) {
    goto syntax_error;
  }
  for (size_t i = 0; i < fn_length; i++) {
    printf("%s\t", filenames[i]);
  }
  printf("\n");
  hashtable *ht = hashtable_empty(lptrcmp, lptr_hfun);
  if (ht == NULL) {
    goto malloc_error;
  }
  holdall *ha = holdall_empty();
  if (ha == NULL) {
    hashtable_dispose(&ht);
    goto malloc_error;
  }
  size_t str_size = DEFAULT_SIZE;
  size_t str_length = 0;
  char *str = malloc(sizeof(char) * str_size);
  if (str == NULL) {
    hashtable_dispose(&ht);
    holdall_dispose(&ha);
    goto malloc_error;
  }
  line *l = line_empty(NULL, (int (*)(const void *,
      const void *))strcmp, fn_length);
  if (l == NULL) {
    hashtable_dispose(&ht);
    holdall_dispose(&ha);
    free(str);
    goto malloc_error;
  }
  line **lptr = &l;
  int c;
  size_t lnum = 1;
  for (int i = (int) fn_length - 1; i >= 0; i--) {
    while (1) {
      c = fgetc(files[i]);
      if (c == '\n' || c == EOF || c == '\0') {
        str[str_length] = '\0';
        if (str[0] != (char) 0) {
          line_change(l, str);
          line **res = hashtable_search(ht, lptr);
          if (res == NULL) {
            char *strtmp = malloc(sizeof(char) * str_size);
            if (strtmp == NULL) {
              line_dispose(lptr);
              hashtable_dispose(&ht);
              holdall_dispose(&ha);
              free(filenames);
              close_files(files, fn_length);
              free(files);
              goto malloc_error;
            }
            strcpy(strtmp, str);
            line *t = line_empty(strtmp, (int (*)(const void *,
                const void *))strcmp,
                fn_length);
            if (t == NULL) {
              free(strtmp);
              line_dispose(lptr);
              hashtable_dispose(&ht);
              holdall_dispose(&ha);
              free(filenames);
              close_files(files, fn_length);
              free(files);
              goto malloc_error;
            }
            line **tmp = malloc(sizeof(line *));
            if (tmp == NULL) {
              line_dispose(&t);
              line_dispose(lptr);
              hashtable_dispose(&ht);
              holdall_dispose(&ha);
              free(filenames);
              close_files(files, fn_length);
              free(files);
              goto malloc_error;
            }
            *tmp = t;
            hashtable_add(ht, tmp, tmp);
            holdall_put(ha, tmp);
            line_add(filenames[i], lnum, *tmp);
          } else {
            line_add(filenames[i], lnum, *res);
          }
        }
        str_length = 0;
        lnum++;
        if (c == EOF) {
          break;
        }
      } else {
        if (str_length >= str_size - 1) {
          str_size *= MUL;
          char *tmp = realloc(str, sizeof(char) * str_size);
          if (tmp == NULL) {
            line_dispose(lptr);
            hashtable_dispose(&ht);
            holdall_dispose(&ha);
            free(filenames);
            close_files(files, fn_length);
            free(files);
            goto malloc_error;
          }
          str = tmp;
        }
        if (upp == 1) {
          c = toupper(c);
        }
        if (filter != NULL) {
          if (filter(c) != 0) {
            str[str_length] = (char) c;
            str_length++;
          }
        } else {
          str[str_length] = (char) c;
          str_length++;
        }
      }
    }
    lnum = 1;
  }
  str[str_length] = '\0';
  line_dispose(lptr);
  holdall_sort(ha, lptrcmp);
  if (fn_length > 1) {
    holdall_apply(ha, free_holdall_mult);
  } else {
    holdall_apply(ha, free_holdall_single);
  }
  holdall_dispose(&ha);
  hashtable_dispose(&ht);
  free(filenames);
  close_files(files, fn_length);
  free(files);
  return EXIT_SUCCESS;
syntax_error:
  fprintf(stderr,
      "Syntax : %s FILENAME ... OPTION ...\n%s "
      OPT_HELP " or %s "OPT_HELP_SHORT " for help\n",
      argv[0], argv[0], argv[0]);
  free(filenames);
  close_files(files, fn_length);
  free(files);
  return EXIT_FAILURE;
malloc_error:
  fprintf(stderr,
      "malloc_error : something went wrong when allocating memory\n");
  return EXIT_FAILURE;
file_error:
  fprintf(stderr, "file_error : something went wrong when reading %s\n",
      filenames[fn_length]);
  free(filenames);
  close_files(files, fn_length);
  free(files);
  return EXIT_FAILURE;
help:
  fprintf(stderr, "HELP :\n"
      "Syntax : ./lnid FILENAME ... OPTION ...\n\n"
      "lnid lit toutes les lignes des fichiers fournis grâce au(x) FILENAME,"
      " dans le cas où\n\n"
      "\tPlusieurs fichiers sont fournis :\n"
      "\t\tAffiche les lignes, triées par ordre lexicographique, qui sont"
      " présentes dans tous les fichiers\n"
      "\t\tet leur nombre d'occurrences.\n"
      "\tUn fichier est fourni :\n"
      "\t\tAffiche les lignes, triées par ordre lexicographique, qui sont "
      "répétées"
      " dans le fichier et le numéro\n"
      "\t\tdes lignes où elles se situent\n\n"
      "\n\t"OPT_FILTER_SHORT " CLASS / "OPT_FILTER "=CLASS : \n\t\tOption ne"
      " retenant des lignes que les caractères décrits par CLASS.\n\t\t"
      "La valeur de CLASS est l'un des suffixes des douzes testes"
      " d'appartenance"
      " à une catégorie\n\t\t"
      "de caractère de l'en-tête standard <ctype.h>\n"
      "\n\t"OPT_SORT_SHORT " WORD / "OPT_SORT "=WORD : \n\t\tOption servant"
      " à trier dans l'ordre croissant des chaines associées\n\t\t"
      "aux contenus, où WORD vaut standard pour une utilisation de strcmp et"
      " locale pour celle de strcoll.\n"
      "\n\t"OPT_UPPERCASING_SHORT " / "OPT_UPPERCASING " : \n\t\tOption "
      "convertissant, via la fonction toupper et avant que ne soit appliquer "
      "l'éventuelle\n\t\t"
      "fonction spécifié par --filter, tout caractère lu correspondant à une "
      "lettre minuscule en le caractère\n"
      "\t\tmajuscule associé.\n");
  free(filenames);
  close_files(files, fn_length);
  free(files);
  return EXIT_FAILURE;
}

#define DEFUN_PRINT_SIZE_T(fun, separator) \
  void print_size_t ## fun(size_t n) {     \
    printf("%zu%s", n, separator);         \
  }

DEFUN_PRINT_SIZE_T(_tab, "\t")
DEFUN_PRINT_SIZE_T(_comma, ",")

int close_files(FILE **files, size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (fclose(files[i]) == EOF) {
      return -1;
    }
  }
  return 0;
}

// lcmp_sd(a, b) et lcmp_lc comparent respectivement deux pointeurs de line
// selon la fonction
//  strcmp et strcoll
#define DEFUN_LCMP(fun, cmp)                                    \
  int fun(const void *a, const void *b) {                       \
    return cmp(line_value((line *) a), line_value((line *) b)); \
  }
DEFUN_LCMP(lcmp_sd, strcmp)
DEFUN_LCMP(lcmp_lc, strcoll)

#define DEFUN_LCMP_PTR(fun, lcmp)             \
  int fun(const void *a, const void *b) {     \
    return lcmp(*(line **) a, *(line **) b);  \
  }

DEFUN_LCMP_PTR(lptrcmp_sd, lcmp_sd)
DEFUN_LCMP_PTR(lptrcmp_lc, lcmp_lc)

// str_hashfun(s) : fonction de hashage pour les chaines de caractères
size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; p++) {
    h = 37 * h + *p;
  }
  return h;
}

size_t lptr_hfun(const void *a) {
  return str_hashfun(line_value(*(line **) a));
}

int free_holdall(void *a) {
  line_dispose((line **) a);
  free(a);
  return 0;
}

int free_holdall_mult(void *a) {
  if (line_nbfile(*(line **) a) == line_nbfilemax(*(line **) a)) {
    line_map_occfile(print_size_t_tab, *(line **) a);
    printf("%s\n", line_value(*(line **) a));
  }
  return free_holdall(a);
}

int free_holdall_single(void *a) {
  if (line_head_occfile(*(line **) a) > 1) {
    line_map_head_num(print_size_t_comma, *(line **) a);
    line_map_head_num_tail(print_size_t_tab, *(line **) a);
    printf("%s\n", line_value(*(line **) a));
  }
  return free_holdall(a);
}
