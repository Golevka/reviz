#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "nfa.h"
#include "dfa.h"


/* Compile basic regular expression to NFA */
struct NFA reg_to_NFA(const char *regexp);

/* Convert an NFA to DFA, this function returns the start state of the
 * resulting DFA */
struct DFA_state *NFA_to_DFA(const struct NFA *nfa);


int main(int argc, char *argv[])
{
    struct NFA nfa;
    struct DFA_state *dfa;

    FILE *fp_nfa, *fp_dfa;

    if (argc == 2)
    {
        if ( (fp_nfa = fopen("nfa.dot", "w")) == NULL) {
            perror("fopen nfa.dot error"); exit(-1);
        }
        if ( (fp_dfa = fopen("dfa.dot", "w")) == NULL) {
            perror("fopen dfa.dot error"); exit(-1);
        }

        fprintf(stderr, "regexp: %s\n", argv[1]);

        /* parse regexp and generate NFA and DFA */
        nfa = reg_to_NFA(argv[1]);
        dfa = NFA_to_DFA(&nfa);

        /* dump NFA and DFA as graphviz code */
        NFA_dump_graphviz_code(&nfa, fp_nfa);
        DFA_dump_graphviz_code(dfa, fp_dfa);

        /* finalize */
        NFA_dispose(&nfa);
        DFA_dispose(dfa);
        fclose(fp_nfa);
        fclose(fp_dfa);
    }
    else {
        printf("usage: %s 'regexp'\n", argv[0]);
    }

    return 0;
}
