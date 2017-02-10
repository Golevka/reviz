#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dfa.h"
#include "glist.h"

MAKE_COMPARE_FUNCTION(addr, struct DFA_state *)

/* Create an empty (isolated), non-acceptable state */
struct DFA_state *alloc_DFA_state(void) {
  struct DFA_state *state =
      (struct DFA_state *)malloc(sizeof(struct DFA_state));

  state->_capacity = 4;
  state->n_transitions = 0; /* isolated  */
  state->is_acceptable = 0; /* non-acceptable */
  state->trans = (struct DFA_transition *)malloc(state->_capacity *
                                                 sizeof(struct DFA_transition));

  return state;
}

/* Free allocated space for specified DFA state */
void free_DFA_state(struct DFA_state *state) {
  free(state->trans); /* free array of transitions */
  free(state);        /* free the state object */
}

/* Traverse from specified state and add all reachable states to a generic
 * list */
void DFA_traverse(struct DFA_state *state, struct generic_list *visited) {
  int i_trans = 0, n_trans = state->n_transitions;
  for (; i_trans < n_trans; i_trans++) {
    /* DFS */
    if (generic_list_add(visited, &state->trans[i_trans].to, __cmp_addr) != 0) {
      DFA_traverse(state->trans[i_trans].to, visited);
    }
  }
}

/* Destroy the entire DFA */
void DFA_dispose(struct DFA_state *start) {
  struct generic_list state_list;
  struct DFA_state **cur;
  int i_state = 0;

  create_generic_list(struct DFA_state *, &state_list);
  generic_list_push_back(&state_list, &start);
  DFA_traverse(start, &state_list);

  for (cur = (struct DFA_state **)state_list.p_dat; i_state < state_list.length;
       i_state++, cur++) {
    free_DFA_state(*cur);
  }

  destroy_generic_list(&state_list);
}

/* Turn specified DFA state to an acceptable one */
void DFA_make_acceptable(struct DFA_state *state) { state->is_acceptable = 1; }

/* Add transition between specified DFA states

       /----\  trans_char  /--\
       |from|------------>>|to|
       \----/              \--/
*/
void DFA_add_transition(struct DFA_state *from, struct DFA_state *to,
                        char trans_char) {
  /* If we're running out of space */
  if (from->n_transitions == from->_capacity) {
    from->_capacity *= 2; /* expand two-fold */
    from->trans = (struct DFA_transition *)realloc(
        from->trans, from->_capacity * sizeof(struct DFA_transition));
  }

  /* add transition */
  from->trans[from->n_transitions].to = to;
  from->trans[from->n_transitions].trans_char = trans_char;

  from->n_transitions++;
}

/* Get the target state of specified state under certain transition, if there's
 * no such transition then NULL is returned */
struct DFA_state *DFA_target_of_trans(struct DFA_state *state,
                                      char trans_char) {
  /* we have to iterate through all transitions to find the one we want */
  int i_trans = 0, n_trans = state->n_transitions;

  /* , so here we have to do a bad linear search */
  for (; i_trans < n_trans; i_trans++) {
    if (state->trans[i_trans].trans_char == trans_char) {
      return state->trans[i_trans].to; /* transition found */
    }
  }

  return NULL; /* we haven't found specified transition */
}

/* dump the transitions to *all reachable* states from specified state */
static void __DFA_reachable_states_dump_graphviz(const struct DFA_state *state,
                                                 struct generic_list *visited,
                                                 FILE *fp) {
  /* we'll storm down each way (transition) and dump each target state
   * recursively */
  int n_trans = state->n_transitions;
  int i_trans = 0;

  for (; i_trans < n_trans; i_trans++) {
    /* dump source state and target state, acceptable states are presented
     * as double circles */
    if (state->is_acceptable) /* source state */
      fprintf(fp, "    node [shape = doublecircle label=\"\"]; addr_%p\n",
              (void *)state); /* target state */

    if (state->trans[i_trans].to->is_acceptable)
      fprintf(fp, "    node [shape = doublecircle label=\"\"]; addr_%p\n",
              (void *)state->trans[i_trans].to);

    fprintf(fp, "    node [shape = circle label=\"\"]\n");

    /* if (state->is_acceptable)   /\* source state *\/ */
    /*     fprintf(fp, */
    /*         "    node [shape = doublecircle label=\"%p\"]; addr_%p\n", */
    /*         (void*) state, (void*) state);      /\* target state *\/ */
    /* else */
    /*     fprintf(fp, "    node [shape = circle label=\"%p\"]; addr_%p\n",  */
    /*         (void*) state, (void*) state); */

    /* if (state->trans[i_trans].to->is_acceptable) */
    /*     fprintf(fp, */
    /*         "    node [shape = doublecircle label=\"%p\"]; addr_%p\n", */
    /*         (void*)state->trans[i_trans].to, */
    /*         (void*)state->trans[i_trans].to); */
    /* else */
    /*     fprintf(fp, "    node [shape = circle label=\"%p\"]; addr_%p\n",  */
    /*         (void*)state->trans[i_trans].to, */
    /*         (void*)state->trans[i_trans].to); */

    /* dump the transition from source state and target state */
    fprintf(fp, "    addr_%p -> addr_%p [ label = \"%c\" ]\n", (void *)state,
            (void *)state->trans[i_trans].to, state->trans[i_trans].trans_char);

    /* dump the successor states of this target state (in recursive
     * fashion) */
    if (generic_list_add(visited, &state->trans[i_trans].to, __cmp_addr) != 0) {
      __DFA_reachable_states_dump_graphviz(state->trans[i_trans].to, visited,
                                           fp);
    }
  }
}

/* Generate DOT code to vizualize the DFA */
void DFA_dump_graphviz_code(const struct DFA_state *start_state, FILE *fp) {
  struct generic_list visited_state;
  create_generic_list(struct DFA_state *, &visited_state);

  fprintf(fp,
          "digraph finite_state_machine {\n"
          "    rankdir=LR;\n"
          "    size=\"8,5\"\n"
          "    node [shape = circle label=\"\"]\n");

  generic_list_push_back(&visited_state, &start_state);
  __DFA_reachable_states_dump_graphviz(start_state, &visited_state, fp);

  /* dump start mark */
  fprintf(fp, "    node [shape = none label=\"\"]; start\n");
  fprintf(fp, "    start -> addr_%p [ label = \"start\" ]\n",
          (void *)start_state);

  /* done */
  fprintf(fp, "}\n");
  destroy_generic_list(&visited_state);
}
