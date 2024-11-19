//  Affiche sur la sortie standard le nombre d’occurrences de chaque word qui
// apparaît dans un et un seul des fichiers de texte donnés.

//  Les résultats sont affichés en colonnes sur la sortie standard. Les colonnes
// sont séparées par le caractère de tabulation. Une ligne d’en-tête montre les
// noms des fichiers. Un word apparaît dans la première colonne, son nombre
// d’occurrences dans le fichier dans lequel il apparaît à l’exclusion de tous
// les autres dans la colonne associée au fichier.

//  Limitations :
//   Les mots sont obtenus par lecture sur un ou plusieurs fichiers dont le.s
//   nom.s figure.nt sur la ligne de commande.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <locale.h>

#include "hashtable.h"
#include "holdall.h"
#include "option.h"

#define CAPACITY_MIN 1
#define CAPACITY_MUL 2

typedef struct word_stats {
  size_t flag_file;
  size_t *count;
  char *word;
} word_stats;

//  str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//    et Pike pour les chaines de caractères.
static size_t str_hashfun(const char *s);

//  sfree : libère la zone mémoire pointée par p->count, p->word ,p et
//    renvoie zéro
static int sfree(word_stats *p);

// compar_count : compare le nombre d'occurrences de deux mots.
//    Renvoie 1 si le nombre d'occurrences du premier mot est supérieur à celui
//    du deuxième,-1 si le nombre d'occurrences du premier mot est inférieur à
//    celui du deuxième, et 0 si les deux mots ont le même nombre d'occurrences.
static int compar_count(word_stats *p1, word_stats *p2);

// compar_str : compare lexicographiquement deux mots.
//    Renvoie un nombre négatif si le premier word vient avant le deuxième dans
//    l'ordre lexicographique,
//    un nombre positif si le premier word vient après le deuxième,
//    et 0 si les deux mots sont identiques.
static int compar_str(word_stats *p1, word_stats *p2);

// compar_str_reverse : version inversée de compar_str.
static int compar_str_reverse(word_stats *p1, word_stats *p2);

// compar_count_reverse : version inversée de compar_count.
static int compar_count_reverse(word_stats *p1, word_stats *p2);

//  display_wc : Affiche le mot et le nombre d'occurrences de ce mot dans un
//    fichier spécifique (indiqué par "ct->flag_file"`) la fonction affiche le
//    mot, suivi d'un certain nombre de tabulations (égal à `ct->flag_file`),
//    puis le nombre d'occurrences de ce mot. Sans effet si le mot apparait
//    dans plus d'un fichier renvoie 0 en cas de succès, -1 sinon.
static int display_wc(word_stats *p);

//  display_files : affiche les noms des fichiers à partir de l'index
//    fstart_index dans argv jusqu'à argc-1, en les indentant selon leur
//    longueur.Retourne 0 en cas de succès, -1 en cas d'erreur.
static int display_files(char **argv, size_t fstart_index, size_t argc);

//  add_word : Vérifie d'abord si le mot w existe déjà dans la hashtable
//    référencée par ht. Si c'est le cas, vérifie ensuite si le mot vient de
//    plusieurs fichiers différents. Si c'est le cas, met flag_file à 0,
//    sinon incrémente simplement ses occurrences.
//    Sinon, elle tente de créer une nouvelle entrée de type
//    word_stats dont le champ word est initialisé avec le mot,
//    count est initialisé à 1 et flag_file à i (l'indice du fichier courant),
//    puis tente d'ajouter à ht et hcptr.
//    Retourne 0 en cas de succès, -1 en cas d'erreur.
static int add_word(hashtable *ht, holdall *hcptr, char *w, size_t *index,
    bool word_cut_detected, size_t file_index, size_t fdind,
    char *filename);

int main(int argc, char *argv[]) {
  setlocale(LC_COLLATE, "fr_FR.UTF-8");
  int r = EXIT_SUCCESS;
  bool ponctuation = false;
  bool lexic = false;
  bool reverse = false;
  bool numeric = false;
  bool word_length_limit = false;
  long int limit = 0;
  size_t fdind = 0;
  char first_key = ' ';
  option_arg(argc, argv, &ponctuation, &lexic, &reverse, &numeric,
      &word_length_limit, &limit, &fdind, &first_key);
  hashtable *ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
      (size_t (*)(const void *))str_hashfun);
  holdall *hcptr = holdall_empty();
  if (ht == NULL || hcptr == NULL) {
    goto error_capacity;
  }
  size_t nbrf = (size_t) argc;
  if (fdind == nbrf) {
    goto error_no_file;
  }
  bool word_cut_detected = false;
  for (size_t i = fdind; i < nbrf; i++) {
    FILE *file = fopen(argv[i], "r");
    if (file == NULL) {
      goto error_opening;
    }
    int c;
    size_t index = 0;
    char *w = malloc(CAPACITY_MIN + 1);
    if (w == NULL) {
      goto error_capacity;
    }
    size_t tab_max = CAPACITY_MIN;
    while ((c = fgetc(file)) != EOF) {
      if (!isspace(c) && (!ponctuation || !ispunct(c))) {
        if (!word_length_limit || index < (size_t) limit) {
          if (index >= tab_max) {
            if (tab_max > SIZE_MAX / CAPACITY_MUL) {
              free(w);
              fclose(file);
              goto error_capacity;
            }
            tab_max = CAPACITY_MUL * tab_max;
            char *temp = realloc(w, tab_max + 1);
            if (temp == NULL) {
              free(w);
              fclose(file);
              goto error_capacity;
            }
            w = temp;
          }
          w[index++] = (char) c;
        } else {
          word_cut_detected = true;
        }
      } else if (index > 0) {
        if (add_word(ht, hcptr, w, &index, word_cut_detected, i, fdind,
            argv[i]) != 0) {
          fclose(file);
          goto error_capacity;
        }
      }
    }
    if (index > 0) {
      if (add_word(ht, hcptr, w, &index, word_cut_detected, i, fdind,
          argv[i]) != 0) {
        fclose(file);
        goto error_capacity;
      }
    }
    free(w);
    if (!feof(file)) {
      goto error_read;
    }
    if (fclose(file) != 0) {
      goto error_closing;
    }
  }
  if (numeric || lexic) {
    if (first_key == 'n') {
      holdall_sort(hcptr,
          (int (*)(const void *,
              const void *))(compar_str));
      holdall_sort(hcptr,
          (int (*)(const void *,
              const void *))(reverse ? compar_count_reverse : compar_count));
    } else if (first_key == 'l') {
      holdall_sort(hcptr,
          (int (*)(const void *,
              const void *))(reverse ? compar_str_reverse : compar_str));
    }
  }
  if (display_files(argv, fdind, nbrf) != 0
      || holdall_apply(hcptr, (int (*)(void *))display_wc) != 0) {
    goto error_write;
  }
  goto dispose;
error_capacity:
  fprintf(stderr, "*** Error: Not enough memory\n");
  goto error;
error_read:
  fprintf(stderr, "*** Error: A read error occurs\n");
  goto error;
error_write:
  fprintf(stderr, "*** Error: A write error occurs\n");
  goto error;
error_opening:
  fprintf(stderr, "*** Error: Error when opening file\n");
  goto error;
error_closing:
  fprintf(stderr, "*** Error: Error when closing file\n");
  goto error;
error_no_file:
  fprintf(stderr, "****  error : At least one file must be specified.\n"
      "Try --help for more informations.\n");
  goto error;
error:
  r = EXIT_FAILURE;
  goto dispose;
dispose:
  hashtable_dispose(&ht);
  if (hcptr != NULL) {
    holdall_apply(hcptr, (int (*)(void *))sfree);
  }
  holdall_dispose(&hcptr);
  return r;
}

size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
    h = 37 * h + *p;
  }
  return h;
}

int sfree(word_stats *p) {
  free(p->count);
  free(p->word);
  free(p);
  return 0;
}

int compar_count(word_stats *p1, word_stats *p2) {
  return (*(p1->count) > *(p2->count)) - (*(p1->count) < *(p2->count));
}

int compar_str(word_stats *p1, word_stats *p2) {
  return strcoll(p1->word, p2->word);
}

int compar_str_reverse(word_stats *p1, word_stats *p2) {
  return strcoll(p2->word, p1->word);
}

int compar_count_reverse(word_stats *p1, word_stats *p2) {
  return compar_count(p2, p1);
}

int display_wc(word_stats *p) {
  if (p->flag_file != 0) {
    if (fprintf(stdout, "%s", p->word) < 0) {
      return -1;
    }
    for (size_t i = 0; i < p->flag_file; i++) {
      if (fprintf(stdout, "\t") < 0) {
        return -1;
      }
    }
    if (fprintf(stdout, "%zu\n", *(p->count)) < 0) {
      return -1;
    }
  }
  return 0;
}

int display_files(char **argv, size_t fstart_index, size_t argc) {
  for (size_t i = fstart_index; i < argc; i++) {
    size_t tab_length = strlen(argv[i]);
    if (fprintf(stdout, "\t%*s", (int) tab_length, argv[i]) < 0) {
      return -1;
    }
  }
  if (fprintf(stdout, "\n") < 0) {
    return -1;
  }
  return 0;
}

int add_word(hashtable *ht, holdall *hcptr, char *w, size_t *index,
    bool word_cut_detected, size_t file_index, size_t fdind,
    char *filename) {
  w[*index] = '\0';
  if (word_cut_detected) {
    fprintf(stderr, "word  '%s' from file : %s was cut \n", w, filename);
    word_cut_detected = false;
  }
  word_stats *p = hashtable_search(ht, w);
  if (p != NULL) {
    if ((p->flag_file) + (size_t) fdind - 1 == file_index) {
      (*(p->count)) += 1;
    } else {
      p->flag_file = 0;
    }
  } else {
    char *s = malloc(strlen(w) + 1);
    if (s == NULL) {
      return -1;
    }
    strcpy(s, w);
    size_t *cptr = malloc(sizeof(*cptr));
    if (cptr == NULL) {
      free(s);
      return -1;
    }
    *cptr = 1;
    p = malloc(sizeof(*p));
    if (p == NULL) {
      free(s);
      free(cptr);
      return -1;
    }
    p->count = cptr;
    p->flag_file = file_index - (size_t) fdind + 1;
    p->word = s;
    if (hashtable_add(ht, s, p) == NULL || holdall_put(hcptr, p) != 0) {
      free(s);
      free(cptr);
      free(p);
      return -1;
    }
  }
  *index = 0;
  return 0;
}
