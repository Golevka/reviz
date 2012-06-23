#include "glist.h"
#include "dfa.h"

#include "nfa.h"                /* to be removed after test */



MAKE_COMPARE_FUNCTION(addr, struct DFA_state*)
MAKE_COMPARE_FUNCTION(char, char)

#include "__dfa_state_set.h"


/* stuff all transition characters emitted by specified state to trans_chars */
static void __DFA_state_collect_transition_chars(
    const struct DFA_state *state, struct generic_list *trans_chars)
{
    int i_trans = 0;
    for ( ; i_trans < state->n_transitions; i_trans++)
    {
        generic_list_add(
            trans_chars, &state->trans[i_trans].trans_char, 
            __cmp_char);
    }
}

/* Collect all transition chars of states in specified state set */
static void DFA_states_collect_transition_chars(
    const struct generic_list *states, struct generic_list *trans_chars)
{
    struct DFA_state **state = (struct DFA_state **) states->p_dat;
    int i_state = 0, n_states = states->length;

    for ( ; i_state < n_states; i_state++, state++) {
        __DFA_state_collect_transition_chars(*state, trans_chars);
    }
}


/* Initialize 2 state sets for DFA optimization process, one for all
 * non-acceptable states, one for the rest of them (acceptable states). */
static struct __DFA_state_set *initialize_DFA_state_set(
    struct DFA_state *dfa_start)
{
    int i_state = 0, n_state;
    struct generic_list state_list, acceptable, nonacceptable;
    struct DFA_state **state;
    struct __DFA_state_set *ll_state_set = __create_empty_stateset_list();

    create_generic_list(struct DFA_state *, &state_list);
    create_generic_list(struct DFA_state *, &acceptable);
    create_generic_list(struct DFA_state *, &nonacceptable);

    /* get all states in the DFA */
    generic_list_push_back(&state_list, &dfa_start);
    DFA_traverse(dfa_start, &state_list);

    /* Initialize state sets by placing all acceptable states to the acceptable
     * list, non-acceptable states goes to nonacceptable list */
    n_state = state_list.length;
    for (state = (struct DFA_state **) state_list.p_dat; 
         i_state < n_state; i_state++, state++)
    {
        (*state)->is_acceptable ? 
            generic_list_push_back(&acceptable,    state):
            generic_list_push_back(&nonacceptable, state);
    }

    /* we've done constructing the 2 initial state sets */
    __insert_states_after(&acceptable,    ll_state_set);
    __insert_states_after(&nonacceptable, ll_state_set);

    destroy_generic_list(&state_list);
    return ll_state_set;
}


/* Split state_set to 2 distinguishable state sets by looking at if states in
 * state_set are distinguishable under transition c 

                   c   state_split_0
       state_set ----<
                       state_split_1
 */
static int split_distinguishable_states(
    struct __DFA_state_set *ll_head,
    struct __DFA_state_set *state_set, char c)
{
    struct generic_list state_split_0, state_split_1;

    struct DFA_state
        **state = (struct DFA_state**)(state_set->dfa_states.p_dat),
        *target;

    struct __DFA_state_set *ref;

    int i_state = 0, n_states = state_set->dfa_states.length;

    create_generic_list(struct DFA_state *, &state_split_0);
    create_generic_list(struct DFA_state *, &state_split_1);

    /* use the first state as reference state, all states transiting to ref
     * goes to state_split_0, otherwise pushed to state_split_1 */
    ref = __find_state_set(ll_head, DFA_target_of_trans(*state, c));
    generic_list_push_back(&state_split_0, state);

    i_state++, state++;    
    for ( ; i_state < n_states; i_state++, state++)
    {
        target = DFA_target_of_trans(*state, c);

        if (ref != NULL)
            if (target != NULL)
                /* test if this state is distinguishable with ref state under
                 * transition c */
                ref == __find_state_set(ll_head, target) ?
                    generic_list_push_back(&state_split_0, state):
                    generic_list_push_back(&state_split_1, state);

            else   /* no such transition, distinguishable */
                generic_list_push_back(&state_split_1, state);

        else
            target != NULL ?
                generic_list_push_back(&state_split_1, state):
                generic_list_push_back(&state_split_0, state);
    }

    /* we're done splitting the state set, now it's time to submit our 
     * changes */
    if (state_split_1.length != 0) /* if we really splitted state_set to 2
                                    * distinguishable states */
    {
        __insert_states_after(&state_split_1, state_set);
        __insert_states_after(&state_split_0, state_set);
        __remove_DFA_state_set(state_set);
        return 1;
    }
    else                        /* not splitted, no change to commit */
    {
        destroy_generic_list(&state_split_0);
        destroy_generic_list(&state_split_1);
        return 0;
    }
}


/* Split the state set into 2 distinguishable sets, splitted sets might also be
 * splitable. */
static int split_state_set(
    struct __DFA_state_set *ll_head, struct __DFA_state_set *state_set)
{
    struct generic_list trans_chars;
    int i_char = 0, n_chars;
    char *c;

    create_generic_list(char, &trans_chars);
    DFA_states_collect_transition_chars(&state_set->dfa_states, &trans_chars);

    /* investigate every possible transitions to find distinguishable states */
    n_chars = trans_chars.length;
    for (c = (char*)trans_chars.p_dat; i_char < n_chars; i_char++, c++)
    {
        if (split_distinguishable_states(ll_head, state_set, *c))
        {
            /* distinguishable state found and splitted, return immediately
             * instead of doing more splits  */
            destroy_generic_list(&trans_chars);
            return 1;
        }
    }

    /* no distinguishable transition/states found */
    destroy_generic_list(&trans_chars);
    return 0;
}


static struct __DFA_state_set *merge_DFA_states(struct DFA_state *dfa)
{
    struct __DFA_state_set *ss = initialize_DFA_state_set(dfa);
    struct __DFA_state_set *cur, *next;
    int changed;

    do {
        changed = 0;
        for (cur = ss->next; cur != ss; cur = next)
        {
            next = cur->next;
            /* changed += __split_state_set(ss, cur); */
            if (split_state_set(ss, cur))
            {
                __dump_DFA_state_set(ss);
                changed++;
            }
        }
    } while (changed != 0);

    return ss;
}


/* Compile basic regular expression to NFA */
struct NFA reg_to_NFA(const char *regexp);

/* Convert an NFA to DFA, this function returns the start state of the
 * resulting DFA */
struct DFA_state *NFA_to_DFA(const struct NFA *nfa);



void test(void)
{
    struct NFA nfa = reg_to_NFA("((a|b|c)+)|d|(e*|f)");
    struct DFA_state *dfa = NFA_to_DFA(&nfa);


    struct __DFA_state_set *ss;

    FILE *fp = fopen("out.dot", "w");
    DFA_dump_graphviz_code(dfa, fp);
    fclose(fp);


    ss = merge_DFA_states(dfa);

    
    __destroy_DFA_stateset_list(ss);


    NFA_dispose(&nfa);
    DFA_dispose(dfa);
}
