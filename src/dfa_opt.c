#include "glist.h"
#include "dfa.h"



/* Each state set contains one or more DFA states, DFA optimization procedure
 * is to merge multiple undistinguished states to one unique state, which
 * decreases the number of states/transitions of the resulting DFA.
 *
 * We have a generic list named dfa_states containing all DFA states in this
 * state set, it is also equiped with a pair of pointers to make it a node in a
 * linked list, the linked list represents the resulting DFA, and each node
 * (state set) is a state of it.
 */
static struct __DFA_state_set
{
    struct __DFA_state_set *prev;
    struct __DFA_state_set *next;
    struct generic_list *dfa_states; /* one or multiple DFA states merged up to
                                      * this state set*/
};


static struct __DFA_state_set *__create_empty_stateset_list(void)
{
    struct __DFA_state_set *head = 
        (struct __DFA_state_set *) malloc(sizeof(struct __DFA_state_set));

    /* doubly linked list */
    head->prev = head->next = head;
    return head;
}

static void __insert_DFA_state_set_after(
    struct __DFA_state_set *e, struct __DFA_state_set *pivot)
{
    e->prev = pivot;
    e->next = pivot->next;
    pivot->next->prev = e;
    pivot->next = e;
}

static void __insert_states_after(
    const struct generic_list *states, struct __DFA_state_set *pivot)
{
    struct __DFA_state_set *new_node = 
        (struct __DFA_state_set *) malloc(sizeof(struct __DFA_state_set));

    new_node->dfa_states = *states;
    __insert_DFA_state_set_after(new_node, pivot);
}


MAKE_COMPARE_FUNCTION(addr, struct DFA_state*)
MAKE_COMPARE_FUNCTION(char, char)


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
    const struct DFA_state *state = (const struct DFA_state*)states->p_dat;
    int i_state = 0, n_states = states->length;

    for ( ; i_state < n_states; i_state++, state++) {
        __DFA_state_collect_transition_chars(state, trans_chars);
    }
}


static struct __DFA_state_set *initialize_DFA_state_set(
    struct DFA_state *dfa_start)
{
    int i_state = 0, n_state;
    struct generic_list state_list, acceptable, nonacceptable;
    struct DFA_state *state;
    struct __DFA_state_set *ll_state_set = __create_empty_stateset_list();

    create_generic_list(struct DFA_state *, &state_list);
    create_generic_list(struct DFA_state *, &acceptable);
    create_generic_list(struct DFA_state *, &nonacceptable);

    /* get all states in the DFA */
    DFA_traverse(dfa_start, &state_list);
    n_state = state_list.length;

    /* Initialize state sets by placing all acceptable states to the acceptable
     * list, non-acceptable states goes to nonacceptable list */
    for (state = (struct DFA_state *) state_list.p_dat; 
         i_state < n_state; i_state++)
    {
        state->is_acceptable ? 
            generic_list_push_back(&acceptable,    &state):
            generic_list_push_back(&nonacceptable, &state);
    }

    __insert_states_after(&acceptable,    ll_state_set);
    __insert_states_after(&nonacceptable, ll_state_set);

    destroy_generic_list(&state_list);
}


/* static int spawn_distinguishable_states( */
/*     const struct generic_list *states, char c) */
/* { */
/*     char c_to; */
/*     struct generic_list state_class; */
/*      const struct DFA_state  */
/*         *state = (const struct DFA_state*)states->p_dat,  */
/*         *target; */

/*     int n_class = 0, i_state = 0, n_states = states->length; */
/*     int i_trans; */

/*     for ( ; i_state < n_states; i_state++, states++) */
/*     { */
/*         target = DFA_target_of_trans(state, c); */
/*         if (target != NULL) */
/*         { */
/*             /\* get target class *\/ */
/*             /\* get the generic list corresponding to that class *\/ */
/*             /\* add target to that generic list *\/ */
/*         } */
/*         break; */
/*     } */
/* } */
