#include "glist.h"
#include "nfa.h"



MAKE_COMPARE_FUNCTION(addr, struct NFA_state*)


/* dump the transition from state to state->to[i_to] */
static void __NFA_transition_dump_graphviz(
    const struct NFA_state *state, int i_to, FILE *fp)
{
    switch (state->transition[i_to].trans_type)
    {
    case NFATT_EPSILON:
        fprintf(fp, "    addr_%p -> addr_%p [ label = \"epsilon\" ];\n", 
            (void*)state,
            (void*)state->to[i_to]);
        break;

    case NFATT_CHARACTER:
        fprintf(fp, "    addr_%p -> addr_%p [ label = \"%c\" ];\n", 
            (void*)state, 
            (void*)state->to[i_to], 
            state->transition[i_to].trans_char);
        break;

    default:
        abort();  /* you should never reach here */
    }
}

/* dump the transitions to *all reachable* states from specified state */
static void __NFA_reachable_states_dump_graphviz(
    const struct NFA_state *state, struct generic_list *visited, FILE *fp)
{
    int n_to = NFA_state_transition_num(state);
    int i_to = 0;

    for ( ; i_to < n_to; i_to++)
    {
        /* dump this transition */
        __NFA_transition_dump_graphviz(state, i_to, fp);

        /* dump the transition target if it has not ever been dumped, exactly
         * the same way with DFS. */
        if (generic_list_add(
                visited, &state->to[i_to], __cmp_addr) != 0) {
            __NFA_reachable_states_dump_graphviz(state->to[i_to], visited, fp);
        }
    }
}

/* Dump DOT code to vizualize specified NFA */
void NFA_dump_graphviz_code(const struct NFA *nfa, FILE *fp)
{
    /* prepare a stack for recursive dump */
    struct generic_list visited_state;
    create_generic_list(struct NFA_state*, &visited_state);

    fprintf(fp, 
        "digraph finite_state_machine {\n"
        "    rankdir=LR;\n"
        "    size=\"8,5\"\n"
        "    node [shape = doublecircle label=\"\"]; addr_%p\n"
        "    node [shape = circle]\n", (void*)nfa->terminate);

    /* dump the finite state machine recursively */
    generic_list_push_back(&visited_state, &nfa->start);
    __NFA_reachable_states_dump_graphviz(nfa->start, &visited_state, fp);

    /* dump start mark */
    fprintf(fp, "    node [shape = none label=\"\"]; start\n");
    fprintf(fp, "    start -> addr_%p [ label = \"start\" ]\n", 
        (void*)nfa->start);

    /* done */
    fprintf(fp, "}\n");
    destroy_generic_list(&visited_state);
}


/* Match the given substring in a recursive fasion */
static int __NFA_is_substate_match(
    const struct NFA_state *state, const char *str)
{
    char c = str[0];  /* transition to match */
    int i_trans = 0, n_trans = NFA_state_transition_num(state);
    int is_matched = 0;

    /* If we reached the terminate state while consumed the entire string*/
    if (c == '\0' && n_trans == 0)   return 1;   /* str matched the nfa */

    for ( ; i_trans < n_trans; i_trans++)
    {
        /* if it is an epsilon move, we can take this way instantly */
        if (state->transition[i_trans].trans_type == NFATT_EPSILON) {
            is_matched = __NFA_is_substate_match(state->to[i_trans], str);
        }
        /* or it must be a character transition, check if we can take it */
        else if (state->transition[i_trans].trans_char == c) {
            is_matched = __NFA_is_substate_match(state->to[i_trans], str + 1);
        }

        if (is_matched) return 1;
    }

    return 0;  /* not matched */
}

/* Check if the string matches the pattern implied by the nfa */
int NFA_pattern_match(const struct NFA *nfa, const char *str)
{
    /* find a sequence of transitions recursively */
    return __NFA_is_substate_match(nfa->start, str);
}
