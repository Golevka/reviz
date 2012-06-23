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

/* Simplify DFA by merging undistinguishable states */
struct DFA_state *DFA_optimize(const struct DFA_state *dfa);


int main(int argc, char *argv[])
{
    struct NFA nfa;
    struct DFA_state *dfa, *dfa_opt;

    FILE *fp_nfa, *fp_dfa, *fp_dfa_opt;

    if (argc == 2)
    {
        if ( (fp_nfa = fopen("nfa.dot", "w")) == NULL) {
            perror("fopen nfa.dot error"); exit(-1);
        }
        if ( (fp_dfa = fopen("dfa.dot", "w")) == NULL) {
            perror("fopen dfa.dot error"); exit(-1);
        }
        if ( (fp_dfa_opt = fopen("dfa_opt.dot", "w")) == NULL) {
            perror("fopen dfa_opt.dot error"); exit(-1);
        }

        fprintf(stderr, "regexp: %s\n", argv[1]);

        /* parse regexp and generate NFA and DFA */
        nfa = reg_to_NFA(argv[1]);
        dfa = NFA_to_DFA(&nfa);
        dfa_opt = DFA_optimize(dfa);

        /* dump NFA and DFA as graphviz code */
        NFA_dump_graphviz_code(&nfa, fp_nfa);
        DFA_dump_graphviz_code(dfa, fp_dfa);
        DFA_dump_graphviz_code(dfa_opt, fp_dfa_opt);

        /* finalize */
        NFA_dispose(&nfa);    fclose(fp_nfa);
        DFA_dispose(dfa);     fclose(fp_dfa);
        DFA_dispose(dfa_opt); fclose(fp_dfa_opt);
    }
    else {
        printf("usage: %s 'regexp'\n", argv[0]);
    }

    return 0;
}
