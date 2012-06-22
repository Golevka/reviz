#ifndef __NFA_HEADER__
#define __NFA_HEADER__


#include <stdlib.h>
#include <stdio.h>


/* definition of some transition characters. 
   (NFATT here means "NFA transition type") */
enum NFA_transition_type {
    NFATT_NONE,        /* placeholder */
    NFATT_CHARACTER,   /* "traditional" alphabet transition */
    NFATT_EPSILON      /* epsilon transition */
};

/* Transition from one NFA state to another */
struct NFA_transition
{
    /* type of the transition. It can be an epsilon transition, traditional
     * alphabet transition, or just a placeholder  */
    enum NFA_transition_type trans_type;
    char trans_char;   /* If trans_type is TT_ALPHABET, then trans_char
                        * indicates the transition label */
};

/* state in NFA, each state has at most 2 transitions if the NFA is constructed
 * from basic constructs of regular expressions. */
struct NFA_state
{
    struct NFA_state      *to[2];          /* destination of transition */
    struct NFA_transition  transition[2];  /* transitions from this state */
};

/* Non determined automata (NFA) */
struct NFA
{
    struct NFA_state *start;     /* start state */
    struct NFA_state *terminate; /* terminate state */

    /* Notice that there should be only one terminate state if the NFA is
     * constructed purly from basic regular expression constructs */
};


/* Create a new isolated NFA state, there's no transitions going out of it */
struct NFA_state *alloc_NFA_state(void);

/* Free allocated space for specified NFA state */
void free_NFA_state(struct NFA_state *state);


/* get number of transitions going out from specified NFA state */
int NFA_state_transition_num(const struct NFA_state *state);

/* Add another transition to specified NFA state, this function returns 0 on
 * success, or it would return an -1 when there's already 2 transitions going
 * out of this state */
int NFA_state_add_transition(struct NFA_state *state, 
    enum NFA_transition_type trans_type, char trans_char, 
    struct NFA_state *to_state);

/* Add an epsilon transition from "from" to "to" */
int NFA_epsilon_move(struct NFA_state *from, struct NFA_state *to);


/* DEBUGGING ROUTINE: dump specified NFA state to fp */
void __dump_NFA_state(const struct NFA_state *state, FILE *fp);

/* Dump DOT code to vizualize specified NFA */
void NFA_dump_graphviz_code(const struct NFA *nfa, FILE *fp);

/* Check if the string matches the pattern implied by the nfa */
int NFA_pattern_match(const struct NFA *nfa, const char *str);


/* Operators in regular expression */
struct NFA NFA_create_atomic(char c);                                 /* c   */
struct NFA NFA_concatenate(const struct NFA *A, const struct NFA *B); /* AB  */
struct NFA NFA_alternate(const struct NFA *A, const struct NFA *B);   /* A|B */
struct NFA NFA_optional(const struct NFA *A);                         /* A?  */
struct NFA NFA_Kleene_closure(const struct NFA *A);                   /* A*  */
struct NFA NFA_positive_closure(const struct NFA *A);                 /* A+  */

/* Free an NFA */
void NFA_dispose(struct NFA *nfa);



#endif /* __NFA_HEADER__ */
