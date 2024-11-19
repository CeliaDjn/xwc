#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#define ARG__OPT_LONG "--"
#define ARG_HELP "help"

#define PUNCTUATION_LIKE_SPACE__OPT_LONG "punctuation-like-space"
#define NUMERIC__OPT_LONG "numeric"
#define LEXICO__OPT_LONG "lexicographical"
#define INITIAL__OPT_LONG "initial"
#define REVERSE__OPT_LONG "reverse"
#define SORT__OPT_LONG "sort"
#define NONE__OPT_LONG "none"

#define PUNCTUATION_LIKE_SPACE__OPT_SHORT 'p'
#define NUMERIC__OPT_SHORT 'n'
#define LEXICO__OPT_SHORT 'l'
#define INITIAL__OPT_SHORT 'i'
#define REVERSE__OPT_SHORT 'R'
#define SORT__OPT_SHORT 's'
#define NONE__OPT_SHORT 'S'

struct option long_options[] = {
  {PUNCTUATION_LIKE_SPACE__OPT_LONG, no_argument, 0,
   PUNCTUATION_LIKE_SPACE__OPT_SHORT},
  {SORT__OPT_LONG, required_argument, 0, SORT__OPT_SHORT },
  {REVERSE__OPT_LONG, no_argument, 0, REVERSE__OPT_SHORT},
  {NUMERIC__OPT_LONG, no_argument, 0, NUMERIC__OPT_SHORT },
  {LEXICO__OPT_LONG, no_argument, 0, LEXICO__OPT_SHORT},
  {INITIAL__OPT_LONG, required_argument, 0, INITIAL__OPT_SHORT},
  {NONE__OPT_LONG, no_argument, 0, NONE__OPT_SHORT},
  {0, 0, 0, 0}
};

//  usage : affiche un message d'aide détaillé pour le programme. Ce message
//    comprend des informations sur l'utilisation du programme,
//    les options disponibles et leur signification.
//    Le message est affiché sur la sortie standard
static void usage(char *argv[]);

//  is_help_requested : vérifie si l'aide a été demandée dans les arguments
//    de la ligne de commande.
static bool is_help_requested(int argc, char *argv[]);

void option_arg(int argc, char *argv[],
    bool *punctuation_like_space, bool *lexicographical, bool *reverse,
    bool *numeric, bool *initial, long int *limit, size_t *index,
    char *last_option) {
  if (is_help_requested(argc, argv)) {
    usage(argv);
  }
  int opt;
  char *endptr;
  errno = 0;
  bool error_encountered = false;
  while ((opt
        = getopt_long(argc, argv, "plni:s:RS", long_options, NULL)) != -1) {
    switch (opt) {
      case PUNCTUATION_LIKE_SPACE__OPT_SHORT:
        *punctuation_like_space = true;
        break;
      case LEXICO__OPT_SHORT:
        *lexicographical = true;
        *last_option = 'l';
        break;
      case NUMERIC__OPT_SHORT:
        *numeric = true;
        *last_option = 'n';
        break;
      case REVERSE__OPT_SHORT:
        *reverse = true;
        break;
      case NONE__OPT_SHORT:
        *last_option = 's';
        break;
      case INITIAL__OPT_SHORT:
        *limit = strtoll(optarg, &endptr, 10);
        if ((errno != 0) || (endptr == optarg) || (*endptr != '\0')) {
          fprintf(stderr, "invalid argument '%s' \n", optarg);
          error_encountered = true;
        }
        if (*limit != 0) {
          *initial = true;
        }
        break;
      case SORT__OPT_SHORT:
        if ((strcmp(optarg, "n") == 0) || (strcmp(optarg, "S") == 0)) {
          fprintf(stderr, "ambiguous argument '%s' \n", optarg);
          error_encountered = true;
        } else if (strcmp(optarg, NUMERIC__OPT_LONG) == 0) {
          *numeric = true;
          *last_option = 'n';
        } else if ((strcmp(optarg, LEXICO__OPT_LONG) == 0)
            || (strcmp(optarg, "l") == 0)) {
          *lexicographical = true;
          *last_option = 'l';
        } else if (strcmp(optarg, REVERSE__OPT_LONG) == 0) {
          *reverse = true;
        } else if (strcmp(optarg, NONE__OPT_LONG) == 0) {
          *last_option = 's';
        } else {
          fprintf(stderr, "Invalid argument for option -s or --sort\n");
          error_encountered = true;
        }
        break;
      default:
        fprintf(stderr, "Try « %s --help » for more informations.\n", argv[0]);
        exit(EXIT_FAILURE);
        break;
    }
  }
  if (error_encountered) {
    fprintf(stderr, "Try « %s --help » for more informations.\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  *index = (size_t) optind;
  if (*last_option == 's') {
    *lexicographical = false;
    *numeric = false;
  }
}

void usage(char *argv[]) {
  printf("Usage: %s [OPTION]... FILE [FILE]...\n\n", argv[0]);
  printf("Exclusive word counting. Print the number of occurrences of each "
      "word"
      "that appears in one and only one of given text\nFILES.\n\n");
  printf("A word is, by default, a maximum length sequence of characters that "
      "do not belong to the white-space characters set.\n\n");
  printf("Results are displayed in columns on the standard output. Columns are "
      "separated by the tab character. Lines are \nterminated by the "
      "end-of-line "
      "character. A header line shows the FILE names: the name of the first "
      "FILE"
      " appears in\nthe second column, that of the second in the third, and so "
      "on."
      " For the following lines, a word appears in the first\ncolumn, its"
      "number"
      " of occurrences in the FILE in which it appears to the exclusion of all "
      "others in the column\nassociated with the FILE. No tab characters are "
      "written on a line after the number of occurrences.\n\n");
  printf("Mandatory arguments to long options are mandatory for short options"
      " too.\n\n");
  printf("Program Information\n");
  printf("  --help    Print this help message and exit.\n\n");
  printf("Processing\n");
  printf("\tThis program processes words using a 'hash-table' data structure"
      ".\n\n");
  printf("Input Control\n");
  printf("  -i, --initial=VALUE    Set the maximal number of significant"
      "initial letters for words to VALUE.\n");
  printf("\t\t0 means without limitation.Default is 0.\n\n");
  printf("  -p, --punctuation-like-space    Make the punctuation characters "
      "play the same role as white-space\n");
  printf("\t\tcharacters in the meaning of words."
      "\n\n");
  printf("Output Control\n");
  printf("  -l            Same as --sort=lexicographical.\n\n");
  printf("  -n            Same as --sort=numeric.\n\n");
  printf("  -S            Same as --sort=none.\n\n");
  printf("  -s, --sort=TYPE    Sort the results in ascending order, by default,"
      " according to TYPE. The available\n\t\tvalues for TYPE are:"
      " 'lexicographical',sort on words, 'numeric', sort on number of\n\t\t"
      "occurrences, first key,and words, second key, and 'none', don't try to"
      "sort"
      ", take it as\n\t\tit comes. Default is 'none'.\n\n");
  printf("  -R, --reverse    Sort in descending order on the single or "
      "first key instead of ascending order. This \n\t\toption has no effect "
      "if the -S option is enable.\n");
  exit(EXIT_SUCCESS);
}

bool is_help_requested(int argc, char *argv[]) {
  for (int k = 1; k < argc; ++k) {
    if (strcmp(argv[k], ARG__OPT_LONG ARG_HELP) == 0) {
      return true;
    }
  }
  return false;
}
