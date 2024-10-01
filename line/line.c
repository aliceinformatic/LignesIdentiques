#include "line.h"
#include <stdio.h>

// numéros de ligne
typedef struct ncell ncell;
struct ncell {
  size_t numline;
  ncell *next;
};
// cellule de fichier dans laquelle se trouve les numéros des lignes qui
// correspondent à la valeur de la ligne
typedef struct fcell fcell;
struct fcell {
  char *fname;
  size_t occ;
  ncell *head;
  ncell *tail;
  fcell *next;
};

typedef struct line {
  char *value;
  size_t nbfile;
  size_t nbfilemax;
  fcell *head;
  int (*comp)(const void *, const void *);
} line;

line *line_empty(char *lstr, int (*comp)(const void *,
    const void *), size_t nbfilemax) {
  line *l = malloc(sizeof *l);
  if (l == NULL) {
    return NULL;
  }
  l->value = lstr;
  l->nbfile = 0;
  l->nbfilemax = nbfilemax;
  l->head = NULL;
  l->comp = comp;
  return l;
}

size_t line_nbfile(line *l) {
  if (l == NULL) {
    return 0;
  }
  return l->nbfile;
}

size_t line_nbfilemax(line *l) {
  if (l == NULL) {
    return 0;
  }
  return l->nbfilemax;
}

char *line_value(line *l) {
  if (l == NULL) {
    return 0;
  }
  return l->value;
}

void *line_search(line *l, char *fname) {
  if (l == NULL) {
    return NULL;
  }
  fcell *f = l->head;
  while (f != NULL) {
    if (l->comp(f->fname, fname) == 0) {
      return (void *) f;
    }
    f = f->next;
  }
  return NULL;
}

bool line_is_in(line *l, char *fname) {
  return line_search(l, fname) != NULL;
}

size_t line_occfile(line *l, char *fname) {
  fcell *f = (fcell *) line_search(l, fname);
  if (f == NULL) {
    return 0;
  }
  return f->occ;
}

size_t line_head_occfile(line *l) {
  if (l == NULL || l->head == NULL) {
    return 0;
  }
  return l->head->occ;
}

void line_map_occfile(void (*fun)(size_t), line *l) {
  if (l == NULL) {
    return;
  }
  fcell *f = l->head;
  while (f != NULL) {
    fun(f->occ);
    f = f->next;
  }
}

void line_map_head_num(void (*fun)(size_t), line *l) {
  if (l == NULL) {
    return;
  }
  if (l->head == NULL) {
    return;
  }
  ncell *n = (l->head)->head;
  while (n != NULL && n->next != NULL) {
    fun(n->numline);
    n = n->next;
  }
}

void line_map_head_num_tail(void (*fun)(size_t), line *l) {
  fun(l->head->tail->numline);
}

// fcell_dispose : sans effet si *fptr vaut NULL. Libère sinon les ressources
//    allouées à la gestion du fichier associé à *fptr puis affecte NULL à
//    *fptr.
void fcell_dispose(fcell **fptr) {
  if ((*fptr)->head == NULL) {
    free(*fptr);
    *fptr = NULL;
    return;
  }
  ncell *n = (*fptr)->head;
  (*fptr)->head = n->next;
  free(n);
  fcell_dispose(fptr);
}

void line_dispose(line **lptr) {
  if ((*lptr)->head == NULL) {
    if ((*lptr)->value != NULL) {
      free((*lptr)->value);
    }
    free(*lptr);
    *lptr = NULL;
    return;
  }
  fcell *f = (*lptr)->head;
  (*lptr)->head = f->next;
  fcell_dispose(&f);
  line_dispose(lptr);
}

void *line_add(char *fname, size_t numline, line *l) {
  if (l == NULL) {
    return NULL;
  }
  fcell *f = line_search(l, fname);
  ncell *nn = malloc(sizeof *nn);
  nn->numline = numline;
  nn->next = NULL;
  if (f == NULL) {
    fcell *nf = malloc(sizeof *nf);
    nf->fname = fname;
    nf->occ = 1;
    nf->next = l->head;
    l->head = nf;
    l->nbfile += 1;
    nf->head = nn;
    nf->tail = nn;
  } else {
    ncell *t = f->tail;
    t->next = nn;
    f->tail = nn;
    f->occ += 1;
  }
  return (void *) nn;
}

void line_change(line *l, char *str) {
  if (l == NULL) {
    return;
  }
  l->value = str;
}
