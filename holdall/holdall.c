//  Partie implantation du module holdall.

#include "holdall.h"

//  struct holdall, holdall : implantation par liste dynamique simplement
//    chainée.

//  Si la macroconstante HOLDALL_PUT_TAIL est définie et que sa macro-évaluation
//    donne un entier non nul, l'insertion dans la liste a lieu en queue. Dans
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

//  merge : fusionne les deux listes triées associées a left et right,Renvoie un
//    pointeur associé a la liste fusionnée en cas de succès.Renvoie sinon NULL.
static struct choldall *merge(struct choldall *left, struct choldall *right,
    int (*compar)(const void *, const void *)) {
  struct choldall *result = NULL;
  struct choldall **tailPtr = &result;
  while (left != NULL && right != NULL) {
    if (compar(left->ref, right->ref) <= 0) {
      *tailPtr = left;
      left = left->next;
    } else {
      *tailPtr = right;
      right = right->next;
    }
    tailPtr = &((*tailPtr)->next);
  }
  *tailPtr = (left != NULL) ? left : right;
  return result;
}

//  mergeSortRecursive : sans effet si *list vaut NULL. Trie sinon récursivement
//    une liste selon la fonction compar en utilisant l’algorithme de tri
//    fusion.
static void mergeSortRecursive(struct choldall **list, int (*compar)(
    const void *, const void *)) {
  if (*list == NULL || (*list)->next == NULL) {
    return;
  }
  struct choldall *slow = *list;
  struct choldall *fast = (*list)->next;
  while (fast != NULL && fast->next != NULL) {
    slow = slow->next;
    fast = fast->next->next;
  }
  struct choldall *mid = slow->next;
  slow->next = NULL;
  mergeSortRecursive(list, compar);
  mergeSortRecursive(&mid, compar);
  *list = merge(*list, mid, compar);
}

void holdall_sort(holdall *ha, int (*compar)(const void *, const void *)) {
  mergeSortRecursive(&(ha->head), compar);
}

#endif
