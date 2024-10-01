//  Partie implantation du module holdall.

#include "holdall.h"

//  struct holdall, holdall : implantation par liste dynamique simplement
//    chainée.

//  Si la macroconstante HOLDALL_PUT_TAIL est définie et que sa macro-évaluation
//    donne une entier non nul, l'insertion dans la liste a lieu en queue. Dans
//    le cas contraire, elle a lieu en tête.

typedef struct choldall choldall;

struct choldall {
  void *ref;
  choldall *next;
};

struct holdall {
  choldall *head;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  choldall **tailptr;
#endif
  size_t count;
};

holdall *holdall_empty(void) {
  holdall *ha = malloc(sizeof *ha);
  if (ha == NULL) {
    return NULL;
  }
  ha->head = NULL;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  ha->tailptr = &ha->head;
#endif
  ha->count = 0;
  return ha;
}

void holdall_dispose(holdall **haptr) {
  if (*haptr == NULL) {
    return;
  }
  choldall *p = (*haptr)->head;
  while (p != NULL) {
    choldall *t = p;
    p = p->next;
    free(t);
  }
  free(*haptr);
  *haptr = NULL;
}

int holdall_put(holdall *ha, void *ref) {
  choldall *p = malloc(sizeof *p);
  if (p == NULL) {
    return -1;
  }
  p->ref = ref;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  p->next = NULL;
  *ha->tailptr = p;
  ha->tailptr = &p->next;
#else
  p->next = ha->head;
  ha->head = p;
#endif
  ha->count += 1;
  return 0;
}

size_t holdall_count(holdall *ha) {
  return ha->count;
}

int holdall_apply(holdall *ha,
    int (*fun)(void *)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun(p->ref);
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context(holdall *ha,
    void *context, void *(*fun1)(void *context, void *ptr),
    int (*fun2)(void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(p->ref, fun1(context, p->ref));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context2(holdall *ha,
    void *context1, void *(*fun1)(void *context1, void *ptr),
    void *context2, int (*fun2)(void *context2, void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(context2, p->ref, fun1(context1, p->ref));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

#if defined HOLDALL_WANT_EXT && HOLDALL_WANT_EXT != 0

// Tri fusion d'un fourre-tout.

// holdall_split(ha, ha1, ha2) : coupe le fourre-tout ha en deux fourre-touts
//  ha1 et ha2, de même taille ou de taille égale à la taille de ha + 1 /2.
void holdall_split(holdall * ha, holdall * ha1, holdall * ha2){
  choldall *p = ha->head;
  choldall **q = &ha1->head;
  choldall **r = &ha2->head;
  while (p != NULL) {
    *q = p;
    q = &p->next;
    p = p->next;
    if (p != NULL) {
      *r = p;
      r = &p->next;
      p = p->next;
    }
  }
  *q = NULL;
  *r = NULL;
  if(ha1->head != NULL){
    if(ha2->head != NULL){
      ha2->count = ha->count / 2;
      ha1->count = ha->count - ha2->count;
    } else {
      ha1->count = 1;
      ha2->count = 0;
    }
  } else {
    if(ha2->head != NULL){
      ha1->count = 0;
      ha2->count = 1;
    } else {
      ha1->count = 0;
      ha2->count = 0;
    }
  }
}

// holdall_merge(ha1, ha2, ha, compar) : fusionne les fourre-touts ha1 et ha2 en un
// fourre-tout ha selon la fonction de comparaison compar.
void holdall_merge(holdall * ha1, holdall * ha2, holdall * ha , int (*compar)(const void *, const void *)){
  choldall *p1 = ha1->head;
  choldall *p2 = ha2->head;
  choldall **q = &ha->head;
  while(p1 != NULL || p2 != NULL){
    if(p1 == NULL){
      *q = p2;
      q = &p2->next;
      p2 = p2->next;
    } else if(p2 == NULL){
      *q = p1;
      q = &p1->next;
      p1 = p1->next;
    } else if(compar(p1->ref, p2->ref) <= 0){
      *q = p1;
      q = &p1->next;
      p1 = p1->next;
    } else {
      *q = p2;
      q = &p2->next;
      p2 = p2->next;
    }
  }
  ha->count = ha1->count + ha2->count;
}
// holdall_sort(ha, compar) : trie le fourre-tout ha selon la fonction de
// comparaison compar (tri fusion).
void holdall_sort(holdall *ha,
    int (*compar)(const void *, const void *)){
  if (ha->count < 1) {
    return;
  }
  holdall ha1, ha2;
  holdall_split(ha, &ha1, &ha2);
  if(ha1.count > 1){
    holdall_sort(&ha1, compar);
  }
  if(ha2.count > 1){
    holdall_sort(&ha2, compar);
  }
  holdall_merge(&ha1, &ha2, ha, compar);
}

#endif
