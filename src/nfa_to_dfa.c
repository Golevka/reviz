#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "glist.h"
#include "nfa.h"
#include "dfa.h"



MAKE_COMPARE_FUNCTION(char, char)
MAKE_COMPARE_FUNCTION(addr, struct NFA_state*)


/* In the NFA to DFA process, multiple NFA states were merged to an unique DFA
 * state. An DFA state entry is an correspondence between a set of NFA states
 * label (addr) and an DFA state. */
struct __dfa_state_entry
{
    struct generic_list nfa_states;    /* set of NFA states */
    struct DFA_state   *dfa_state;     /* corresponded DFA state */
};

/* Create a new DFA state and bind it with specified set of NFA states */
static void __create_dfa_state_entry(
    const struct generic_list *states, struct __dfa_state_entry *entry)
{
    /* the dfa state entry object keeps a copy of NFA state labels (addrs) and
     * sort these labels for fast set comparisons */
    generic_list_duplicate(&entry->nfa_states, states);
    qsort(
        entry->nfa_states.p_dat, 
        entry->nfa_states.length, 
        entry->nfa_states.elem_size, 
        __cmp_addr);

    /* create a new DFA state for this set of NFA states */
    entry->dfa_state = alloc_DFA_state();
}

/* Free memory allocated for the entry object */
static void __destroy_dfa_state_entry(struct __dfa_state_entry *entry) {
    destroy_generic_list(&entry->nfa_states);
}

/* compare if two generic lists are equivalent: 
     (set(label_a) == set(label_b)) 
*/
static int __cmp_dfa_state_entry(const void *a_, const void *b_)
{
    struct NFA_state **sa, **sb;
    int i = 0, length;

    struct __dfa_state_entry *a = (struct __dfa_state_entry*) a_;
    struct __dfa_state_entry *b = (struct __dfa_state_entry*) b_;

    struct generic_list *label_a = &a->nfa_states;
    struct generic_list *label_b = &b->nfa_states;

    if (label_a->length != label_b->length) return 1;   /* not equal */

    /* compare the elements of the states label list */
    length = label_a->length;
    for (sa = (struct NFA_state **)label_a->p_dat, 
             sb = (struct NFA_state **)label_b->p_dat; 
         i < length; i++, sa++, sb++)
    {
        if (*sa != *sb) return 1;
    }

    return 0;  /* a_ == b_ */
}

/* Calculate the epsilon closure of specified state, all states in the
 * resulting closure are appended to the visited list */
static void __NFA_state_epsilon_closure(
    const struct NFA_state *state, struct generic_list *visited)
{
    int i_trans = 0, n_trans = NFA_state_transition_num(state);

    for ( ; i_trans < n_trans; i_trans++)
    {
        if (state->transition[i_trans].trans_type == NFATT_EPSILON)
        {
            /* storm down if we have not visited this state yet */
            if (generic_list_add(
                    visited, &state->to[i_trans], __cmp_addr) != 0) {
                __NFA_state_epsilon_closure(state->to[i_trans], visited);
            }
        }
    }
}

/* Figure out the epsilon closure of a set of states. */
static void __NFA_epsilon_closure(struct generic_list *states)
{
    const struct NFA_state *state;
    int i_state = 0, n_state = states->length;

    for ( ; i_state < n_state; i_state++)
    {
        /* states->p_dat might be relocated while appending more elements */
        state = *(((const struct NFA_state**) states->p_dat) + i_state);
        __NFA_state_epsilon_closure(state, states);
    }
}

/* Get all possible transitions from specified set of states */
static void __NFA_collect_transition_chars(
    struct generic_list *states, struct generic_list *trans_char)
{
    struct NFA_state **s = (struct NFA_state**) states->p_dat;

    int i_state = 0, i_trans;
    int n_states = states->length, n_trans;

    for ( ; i_state < n_states; i_state++, s++)
    {
        n_trans = NFA_state_transition_num(*s);
        for (i_trans = 0; i_trans < n_trans; i_trans++)
        {
            if ((*s)->transition[i_trans].trans_type == NFATT_CHARACTER)
            {
                generic_list_add(
                    trans_char,
                    &((*s)->transition[i_trans].trans_char), 
                    __cmp_char);
            }
        }
    }
}

/* Get all possible successor states under transition c to a set of given
 * states */
static void __NFA_collect_target_states(
    struct generic_list *states, char c, struct generic_list *new_states)
{
    struct NFA_state **s = (struct NFA_state**) states->p_dat;

    int i_state = 0, i_trans;
    int n_states = states->length, n_trans;

    for ( ; i_state < n_states; i_state++, s++)
    {
        n_trans = NFA_state_transition_num(*s);
        for (i_trans = 0; i_trans < n_trans; i_trans++)
        {
            if ((*s)->transition[i_trans].trans_type == NFATT_CHARACTER && 
                (*s)->transition[i_trans].trans_char == c)
            {
                generic_list_add(new_states, &((*s)->to[i_trans]), __cmp_addr);
            }
        }
    }
}

static struct DFA_state *__get_DFA_state_address(
    struct generic_list *dfa_state_entry_list, 
    const struct generic_list *states, int *is_new_entry)
{
    struct __dfa_state_entry entry, *addr;
    __create_dfa_state_entry(states, &entry);

    /* search in all logged entries first */
    addr = (struct __dfa_state_entry *)
        generic_list_find(dfa_state_entry_list, &entry, __cmp_dfa_state_entry);

    if (addr == NULL)    /* not found, we need to add a new entry/DFA state */
    {
        *is_new_entry = 1;
        generic_list_push_back(dfa_state_entry_list, &entry);
        return entry.dfa_state;
    }
    else                 /* entry/DFA state already exists */
    {
        *is_new_entry = 0;
        __destroy_dfa_state_entry(&entry);
        free_DFA_state(entry.dfa_state);
        return addr->dfa_state;
    }
}

/* Mark DFA states containing the terminate state of NFA as acceptable */
static void __mark_acceptable_states(
    const struct NFA_state *terminator, 
    struct generic_list *dfa_state_entry_list)
{
    struct __dfa_state_entry *entry;

    int i_entry = 0;
    for (entry = (struct __dfa_state_entry *) dfa_state_entry_list->p_dat; 
         i_entry < dfa_state_entry_list->length; i_entry++, entry++)
    {
        /* check if the terminator of NFA is merged into this DFA state */
        if (generic_list_find(
                &entry->nfa_states, &terminator, __cmp_addr) != NULL)
        {
            /* if so, this DFA state becomes acceptable */
            DFA_make_acceptable(entry->dfa_state);
        }
    }
}

static void __NFA_to_DFA_rec(
    struct generic_list *states, 
    struct generic_list *dfa_state_entry_list)
{
    struct generic_list trans_char, new_states;
    struct DFA_state *from, *to;
    int  i_char = 0;
    int  if_rec, dummy;  /* if this state has already been created */
    char *c;

    create_generic_list(char, &trans_char);
    create_generic_list(struct NFA_state*, &new_states);

    /* get all transition characters in states, we gonna storm each way down in
     * the next for loop. */
    __NFA_collect_transition_chars(states, &trans_char);

    for (c = (char*)trans_char.p_dat; 
         i_char < trans_char.length; i_char++, c++)
    {
        /* get the epsilon closure of target states under transition *c */
        __NFA_collect_target_states(states, *c, &new_states);
        __NFA_epsilon_closure(&new_states);

        /* Here we need to add states and new_states to the DFA, and connect
         * them together with transition. */
        from = __get_DFA_state_address(dfa_state_entry_list, states, &dummy);
        to   = __get_DFA_state_address(dfa_state_entry_list, &new_states, &if_rec);
        DFA_add_transition(from, to, *c);

        /* DFS: storm down this way and get its all successor states */
        if (if_rec)
            __NFA_to_DFA_rec(&new_states, dfa_state_entry_list);

        generic_list_clear(&new_states);
    }

    destroy_generic_list(&trans_char);
    destroy_generic_list(&new_states);
}


/* Convert an NFA to DFA, this function returns the start state of the
 * resulting DFA */
struct DFA_state *NFA_to_DFA(const struct NFA *nfa)
{
    int i_list = 0;
    struct generic_list start_states;
    struct generic_list dfa_state_entry_list;
    struct DFA_state *dfa_start_state;

    create_generic_list(struct NFA_state*, &start_states);
    create_generic_list(struct __dfa_state_entry, &dfa_state_entry_list);

    /* recursive: we start from the epsilon closure of the start state and
     * storm all the way down. */
    generic_list_push_back(&start_states, &nfa->start);
    __NFA_epsilon_closure(&start_states);
    __NFA_to_DFA_rec(&start_states, &dfa_state_entry_list);

    /* mark DFA states containing the terminate state of NFA as acceptable */
    __mark_acceptable_states(nfa->terminate, &dfa_state_entry_list);

    /* start state of generated DFA should be the first created one */
    dfa_start_state = 
        ((struct __dfa_state_entry*) dfa_state_entry_list.p_dat)[0].dfa_state;

    /* The final clean ups */
    for ( ; i_list < dfa_state_entry_list.length; i_list++)
    {
        /* we need to destroy all sublists */
        __destroy_dfa_state_entry(
            ((struct __dfa_state_entry *) dfa_state_entry_list.p_dat) + i_list);
    }

    destroy_generic_list(&start_states);
    destroy_generic_list(&dfa_state_entry_list);

    return dfa_start_state;
}
