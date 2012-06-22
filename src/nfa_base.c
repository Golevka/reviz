#include <assert.h>

#include "glist.h"
#include "nfa.h"


/* Create a new isolated NFA state, there's no transitions going out of it */
struct NFA_state *alloc_NFA_state(void)
{
    struct NFA_state *state = 
        (struct NFA_state*)malloc(sizeof(struct NFA_state));
    struct NFA_transition null_transition = {NFATT_NONE, 0};
    
    /* create an isolated NFA state node */
    state->to[0] = state->to[1] = NULL;
    state->transition[0] = state->transition[1] = null_transition;

    return state;
}

/* Free allocated space for specified NFA state */
void free_NFA_state(struct NFA_state *state) {
    free(state);
}

/* get number of transitions going out from specified NFA state */
int NFA_state_transition_num(const struct NFA_state *state)
{
    if (state->transition[1].trans_type != NFATT_NONE) return 2;
    if (state->transition[0].trans_type != NFATT_NONE) return 1;
    else  return 0;
}

/* Add another transition to specified NFA state, this function returns 0 on
 * success, or it would return an -1 when there's already 2 transitions going
 * out of this state */
int NFA_state_add_transition(struct NFA_state *state, 
    enum NFA_transition_type trans_type, char trans_char, 
    struct NFA_state *to_state)
{
    int i_trans = NFA_state_transition_num(state);
    if (i_trans >= 2)  return -1;  /* no empty slot avaliable */
    else {
        state->transition[i_trans].trans_type = trans_type;
        state->transition[i_trans].trans_char = trans_char;
        state->to[i_trans]                    = to_state;
        return 0;
    }
}

/* Add an epsilon transition from "from" to "to */
int NFA_epsilon_move(struct NFA_state *from, struct NFA_state *to)
{
    return NFA_state_add_transition(from, NFATT_EPSILON, 0, to);
}

/* DEBUGGING ROUTINE: dump specified NFA state to fp */
void __dump_NFA_state(const struct NFA_state *state, FILE *fp)
{
    int n_trans = NFA_state_transition_num(state);
    int i_trans = 0;

    fprintf(fp, "num of transitions: %d\n", n_trans);
    for ( ; i_trans < n_trans; i_trans++)
    {
        switch (state->transition[i_trans].trans_type)
        {
        case NFATT_CHARACTER:
            fprintf(fp, "   alphabet transition: %c\n", 
                state->transition[i_trans].trans_char);
            break;
           
        case NFATT_EPSILON:
            fprintf(fp, "   epsilon transition\n");
            break;

        default:
            fprintf(fp, "ERROR: You should never reach here\n");
            abort();
        }
    }
}


/* Create an NFA for recognizing single character */
struct NFA NFA_create_atomic(char c)
{
    struct NFA nfa;

    nfa.start     = alloc_NFA_state();
    nfa.terminate = alloc_NFA_state();

    assert(c != '\0');
    NFA_state_add_transition(nfa.start, NFATT_CHARACTER, c, nfa.terminate);

    return nfa;
}

/* C = AB */
struct NFA NFA_concatenate(const struct NFA *A, const struct NFA *B)
{
    struct NFA C;
    C.start     = A->start;
    C.terminate = B->terminate;

    NFA_epsilon_move(A->terminate, B->start);

    return C;
}

/* C = A|B */
struct NFA NFA_alternate(const struct NFA *A, const struct NFA *B)
{
    struct NFA C;
    C.start     = alloc_NFA_state();
    C.terminate = alloc_NFA_state();

    NFA_epsilon_move(C.start,      A->start);
    NFA_epsilon_move(C.start,      B->start);
    NFA_epsilon_move(A->terminate, C.terminate);
    NFA_epsilon_move(B->terminate, C.terminate);

    return C;
}

/* C = A? = A|epsilon */
struct NFA NFA_optional(const struct NFA *A)
{
    struct NFA C;
    C.start     = alloc_NFA_state();
    C.terminate = A->terminate;

    NFA_epsilon_move(C.start, A->start);
    NFA_epsilon_move(C.start, A->terminate);

    return C;
}

/* C = A* */
struct NFA NFA_Kleene_closure(const struct NFA *A)
{
    struct NFA C;
    C.start     = alloc_NFA_state();
    C.terminate = alloc_NFA_state();

    NFA_epsilon_move(A->terminate, C.start);
    NFA_epsilon_move(C.start,      A->start);
    NFA_epsilon_move(C.start,      C.terminate);

    return C;
}

/* C = A+ = AA* */
struct NFA NFA_positive_closure(const struct NFA *A)
{
    struct NFA C;
    C.start     = alloc_NFA_state();
    C.terminate = alloc_NFA_state();

    NFA_epsilon_move(C.start,      A->start);
    NFA_epsilon_move(A->terminate, C.start);
    NFA_epsilon_move(A->terminate, C.terminate);

    return C;
}


MAKE_COMPARE_FUNCTION(addr, int*)

/* Traverse the NFA while recording addresses of all states in a generic
 * list */
static void __NFA_traverse(
    struct NFA_state *state, struct generic_list *visited)
{
    int i_to = 0, n_to = NFA_state_transition_num(state);
    for ( ; i_to < n_to; i_to++)
    {
        /* DFS of graphs */
        if (generic_list_add(
                visited, &state->to[i_to], __cmp_addr) != 0) {
            __NFA_traverse(state->to[i_to], visited);
        }
    }
}

/* Free an NFA */
void NFA_dispose(struct NFA *nfa)
{
    struct generic_list visited;

    struct NFA_state **cur;
    int i_state = 0;

    /* traverse the NFA and record all states in a generic list */
    create_generic_list(struct NFA_state*, &visited);
    generic_list_push_back(&visited, &nfa->start);
    __NFA_traverse(nfa->start, &visited);

    /* free all states */
    for (cur = (struct NFA_state**) visited.p_dat; 
         i_state < visited.length; i_state++, cur++)
    {
        free_NFA_state(*cur);
    }

    destroy_generic_list(&visited);
}
