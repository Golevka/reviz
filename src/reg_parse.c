#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "nfa.h"


/* LL(1) parser modules */
static struct NFA __LL_expression(char **statement);
static struct NFA __LL_term(char **statement);
static struct NFA __LL_primary(char **statement);

/* expression:
       expression term
       expression | term
       term                */
static struct NFA __LL_expression(char **statement)
{
    struct NFA lhs = __LL_term(statement);
    struct NFA ret, rhs;
    char ch;

    for ( ; ; lhs = ret)
    {
        ch = **statement;

        if (isalnum(ch) || ch == '(') { /* expression term */
            rhs = __LL_term(statement);
            ret = NFA_concatenate(&lhs, &rhs);
        }
        else if (ch == '|') {           /* expression | term */
            *statement += 1;            /* eat '|' */
            rhs = __LL_term(statement);
            ret = NFA_alternate(&lhs, &rhs);
        }
        else {
            return lhs;                 /* term  */
        }
    }

    return ret;   /* we should never reach here */
}

/* term:
       term *
       term +
       primary    */
static struct NFA __LL_term(char **statement)
{
    struct NFA lhs = __LL_primary(statement);
    struct NFA ret;
    char ch = **statement;

    if (ch == '*') {            /* term * */
        ret = NFA_Kleene_closure(&lhs);
        *statement += 1;        /* eat the Kleene star */
    }
    else if (ch == '+') {       /* term + */
        ret = NFA_positive_closure(&lhs);
        *statement += 1;        /* eat the positive closure */
    }
    else if (ch == '?') {       /* term ? */
        ret = NFA_optional(&lhs);
        *statement += 1;        /* eat the optional (question) mark */
    }
    else {
        return lhs;             /* primary */
    }

    return ret;
}

/* primary:
       ALNUM
       ( expression )    */
static struct NFA __LL_primary(char **statement)
{
    struct NFA ret;
    char ch = **statement;

    if (isalnum(ch)) {          /* ALNUM */
        ret = NFA_create_atomic(ch);
        *statement += 1;        /* eat the character */
    }
    else if (ch == '(')         /* ( expression ) */
    {
        *statement += 1;        /* eat '(' */
        ret = __LL_expression(statement);
        if (**statement != ')') {
            fprintf(stderr, "no matching ')' found\n"); exit(-1);
        }
        *statement +=1;         /* eat ')' */
    }
    else {
        fprintf(stderr, "unrecognized character \"%c\"\n", ch);
        exit(-1);
    }

    return ret;
}

/* LL parser driver/interface */
struct NFA reg_to_NFA(const char *regexp)
{
    char **cur = (char **)(&regexp);
    struct NFA nfa = __LL_expression(cur);

    if (**cur != '\0') {
        fprintf(stderr, "unexcepted character \"%c\"\n", **cur);
        exit(-1);
    }

    return nfa;
}
