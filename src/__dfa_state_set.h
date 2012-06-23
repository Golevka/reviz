
/* Each state set contains one or more DFA states, DFA optimization procedure
 * is to merge multiple undistinguished states to one unique state, which
 * decreases the number of states/transitions of the resulting DFA.
 *
 * We have a generic list named dfa_states containing all DFA states in this
 * state set, it is also equiped with a pair of pointers to make it a node in a
 * linked list, the linked list represents the resulting DFA, and each node
 * (state set) is a state of it.
 */
struct __DFA_state_set
{
    struct __DFA_state_set *prev;
    struct __DFA_state_set *next;
    struct generic_list dfa_states; /* one or multiple DFA states merged up to
                                      * this state set*/
};


static struct __DFA_state_set *__create_empty_stateset_list(void)
{
    struct __DFA_state_set *head = 
        (struct __DFA_state_set *) malloc(sizeof(struct __DFA_state_set));

    /* doubly linked list */
    head->prev = head->next = head;
    head->dfa_states.length = 0;

    return head;
}

static void __destroy_DFA_stateset_list(struct __DFA_state_set *head)
{
    struct __DFA_state_set *cur = head->next, *next;
    free(head);           /* free head node first  */

    /* then free the rest of the list */
    for ( ; cur != head; cur = next)
    {
        next = cur->next;
        destroy_generic_list(&cur->dfa_states);
        free(cur);
    }    
}

static struct __DFA_state_set *__find_state_set(
    struct __DFA_state_set *ll_head, const struct DFA_state *state)
{
    struct __DFA_state_set *cur = ll_head->next;
    for ( ; cur != ll_head; cur = cur->next)
    {
        if (generic_list_find(&cur->dfa_states, &state, __cmp_addr) != NULL) {
            return cur;
        }
    }

    return NULL;
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

static void __remove_DFA_state_set(struct __DFA_state_set *state_set)
{
    state_set->prev->next = state_set->next;
    state_set->next->prev = state_set->prev;
    
    destroy_generic_list(&state_set->dfa_states);
    free(state_set);
}


/* ROUTINE FOR DEBUGGING */
static void __dump_DFA_state_set(struct __DFA_state_set *ll_head)
{
    struct __DFA_state_set *cur = ll_head->next;

    int i_state, n_state;
    struct DFA_state **state;

    for ( ; cur != ll_head; cur = cur->next)
    {
        state = (struct DFA_state **) cur->dfa_states.p_dat;
        n_state = cur->dfa_states.length;

        printf("%p : {", (void*) cur);
        for (i_state = 0; i_state < n_state; i_state++, state++)
        {
            printf("  %p  ", (void*)(*state));
        }
        printf("}\n");
    }
    printf("\n");
}
