//  Fonctionnement général :
//  - la structure de données ne stocke pas d'objets mais des références vers
//      ces objets. Les références sont du type générique « void * » ;
//  - si des opérations d'allocation dynamique sont effectuées, elles le sont
//      pour la gestion propre de la structure de données, et en aucun cas pour
//      réaliser des copies ou des destructions d'objets ;
//  - les fonctions qui possèdent un paramètre de type « line * » ou « line ** »
//      ont un comportement indéterminé lorsque ce paramètre ou sa déréférence
//      n'est pas l'adresse d'un contrôleur préalablement renvoyée avec succès
//      par la fonction line_empty et non révoquée depuis par la fonction
//      line_dispose ;
//  - aucune fonction ne peut ajouter NULL à la structure de données ;
//  - En cas de succès, les fonctions de type de retour « void * » renvoient
//      une référence actuellement ou auparavant stockée par la structure de
//      données ;
//  - l'implantation des fonctions dont la spécification ne précise pas qu'elles
//      doivent gérer les cas de dépassement de capacité ne doivent avoir
//      affaire avec aucun problème de la sorte.

#include <stdbool.h>
#include <stdlib.h>

//- STANDARD -------------------------------------------------------------------

// struct line, line :
typedef struct line line;

//  line_empty : tente d'allouer les ressources nécessaires pour gérer une ligne
//    initialement vide. Renvoie NULL en cas d'erreur. Renvoie sinon un pointeur
//    vers le contrôleur associé à la ligne.
extern line *line_empty(char *lstr, int (*comp)(const void *, const void *), size_t nbfilemax);

// line_value : renvoie la chaîne de caractère associée à l.
extern char *line_value(line *l);

// line_nbfile : renvoie le nombre de fichiers dans lequel se trouve la ligne l.
extern size_t line_nbfile(line *l);

// line_nbfilemax : renvoie le nombre de fichiers maximum de la ligne l.
extern size_t line_nbfilemax(line *l);

// line_search : recherche dans la ligne associée à l le nom d'un fichier égal à
//    celui de fname au sens de la fonction de comparaison. Si la recherche est
//    négative, renvoie NULL. Renvoie sinon le fichier trouvé.
extern void *line_search(line *l, char *fname);

// line_is_in : renvoie true ou false selon que la ligne est déjà présente dans
//    le fichier fname ou non.
extern bool line_is_in(line *l, char *fname);

// line_occfile : renvoie le nombre d'occurence de la ligne l dans le fichier
//    fname.
extern size_t line_occfile(line *l, char *fname);

// line_occfile : renvoie le nombre d'occurence de la ligne l dans le fichier
//    de tête.
extern size_t line_head_occfile(line *l);

// line_map_occfile : applique la fonction fun à chaque occurence des fichiers
//    de l.
extern void line_map_occfile(void (*fun)(size_t), line *l);

// line_map_head_num : applique la fonction fun à chaque numéro de ligne du
//    fichier en tête sauf la queue.
extern void line_map_head_num(void (*fun)(size_t), line *l);

// line_map_head_num_tail : applique la fonction fun au numéro de la ligne
//  en queue du fichier tête
extern void line_map_head_num_tail(void (*fun)(size_t), line *l);

// line_dispose : sans effet si *lptr vaut NULL. Libère sinon les ressources
//    allouées à la gestion de la ligne associée à *lptr puis affecte NULL à
//    *lptr.
extern void line_dispose(line **lptr);

// line_add : renvoie NULL si la ligne vaut NULL. Tente sinon d'ajouter numline
//    à la liste associée à fname. Renvoie NULL en cas de dépassement de
//    capacité ; renvoie sinon numline
extern void *line_add(char *fname, size_t numline, line *l);

// line_change : modifie la valeur de la ligne l par la chaîne de caractères
//    str.
extern void line_change(line *l, char* str);
