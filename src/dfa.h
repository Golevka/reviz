#ifndef __DFA_HEADER__
#define __DFA_HEADER__

#include <stdio.h>
#include <stdlib.h>

#include "glist.h"

struct DFA_state; /* forward type declaration */

/* Transition of a DFA state */
struct DFA_transition {
  struct DFA_state *to; /* destination of the transition */
  char trans_char;      /* transition character */
};

/* State in DFA, it can also be used to represent an entire DFA if it is a
 * start state */
struct DFA_state {
  int is_acceptable; /* if this state is an acceptable state */

  struct DFA_transition *trans; /* an array of transitions going out from
                                 * this state */
  int n_transitions;            /* number of transitions  */
  int _capacity;                /* reserved space for transitions */
};

/* Create an empty (isolated), non-acceptable state */
struct DFA_state *alloc_DFA_state(void);

/* Free allocated space for specified DFA state */
void free_DFA_state(struct DFA_state *state);

/* Destroy the entire DFA */
void DFA_dispose(struct DFA_state *start);

/* Turn specified DFA state to an acceptable one */
void DFA_make_acceptable(struct DFA_state *state);

/* Add transition between specified DFA states

       /----\  trans_char  /--\
       |from|------------>>|to|
       \----/              \--/
*/
void DFA_add_transition(struct DFA_state *from, struct DFA_state *to,
                        char trans_char);

/* Get the target state of specified state under certain transition, if there's
 * no such transition then NULL is returned */
struct DFA_state *DFA_target_of_trans(struct DFA_state *state, char trans_char);

/* Traverse from specified state and add all reachable states to a generic
 * list */
void DFA_traverse(struct DFA_state *state, struct generic_list *visited);

/* Generate DOT code to vizualize the DFA */
void DFA_dump_graphviz_code(const struct DFA_state *start_state, FILE *fp);

#endif /* __DFA_HEADER__ */
