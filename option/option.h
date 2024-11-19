// Partie interface du module option pour la gestion des arguments sur la ligne
//de commande

#ifndef OPTION__H
#define OPTION__H

#include <stdlib.h>

//  option_arg : analyse les options de la ligne de commande. Elle met à jour
//    plusieurs variables en fonction des options spécifiées par l’utilisateur.
//    En cas d’erreur, elle affiche un message d’erreur et termine le programme.
//    Si l’option de tri est définie sur 'none', les options de tri
//    lexicographique et numérique sont désactivées. Elle met également à jour
//    l’index pour pointer vers le premier argument non-option de la ligne de
//    commande.
extern void option_arg(int argc, char *argv[],
    bool *punctuation_like_space, bool *lexicographical, bool *reverse,
    bool *numeric, bool *initial, long int *limit, size_t *index,
    char *last_option);

#endif
