#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "glist.h"



/* Create a generic list for some kind of data elements, elem_size specified
 * the size in bytes of each data element, initial_capacity is the amount of
 * space reserved for furture use (it cannot be zero). */
void __create_generic_list(
    int elem_size, int initial_capacity, struct generic_list *glist)
{
    assert(initial_capacity != 0);    /* zero capacity list would not be
                                       * correctly expanded */
    glist->elem_size = elem_size;
    glist->capacity  = initial_capacity;
    glist->length    = 0;
    glist->p_dat     = (char*)malloc(elem_size * initial_capacity);
}

/* "Copy constructor" */
void generic_list_duplicate(
    struct generic_list *dest, const struct generic_list *src)
{
    __create_generic_list(src->elem_size, src->capacity, dest);
    dest->length = src->length;
    memcpy(dest->p_dat, src->p_dat, src->length * src->elem_size);
}

/* Free the memory allocated for the generic list */
void destroy_generic_list(struct generic_list *glist) {
    free(glist->p_dat);
}

/* Append an element to the tail of specified generic list */
void generic_list_push_back(struct generic_list *glist, const void *elem)
{
    /* if we're running out of space */
    if (glist->capacity == glist->length)
    {
        glist->capacity *= 2;   /* expand two-fold */
        glist->p_dat = 
            (char*)realloc(glist->p_dat, glist->elem_size * glist->capacity);
    }

    /* append *elem to the tail of glist */
    memcpy(
        glist->p_dat + glist->elem_size * glist->length++, elem, 
        glist->elem_size);
}

/* Remove the last element in the list */
void generic_list_pop_back(struct generic_list *glist)
{
    assert(glist->length != 0);  /* check for stack downflow */
    glist->length -= 1;
}

/* Get the pointer to the element on the tail of the list */
void *generic_list_back(struct generic_list *glist)
{
    assert(glist->length != 0);
    return glist->p_dat + glist->elem_size * (glist->length - 1);
}

/* Get pointer to the first element  */
void *generic_list_front(struct generic_list *glist)
{
    assert(glist->length != 0);
    return glist->p_dat;
}

/* Empty the generic list */
void generic_list_clear(struct generic_list *glist) {
    glist->length = 0;
}

/* Find element in the list using specified compare function, it returns the
 * pointer to the element found in the list, or a NULL is returned if *elem is
 * not in glist */
void *generic_list_find(
    struct generic_list *glist, const void *elem, 
    int(*cmp)(const void*, const void*))
{
    int i = 0;
    char *cur = glist->p_dat;

    for ( ; i < glist->length; i++, cur += glist->elem_size) {
        if (cmp((void*)cur, elem) == 0) return cur;  /* found */
    }

    return NULL;   /* not found */
}

/* Add an element to the list only if this element is not in the list (we
 * actually regard the glist as a set). It would return 1 if *elem is actually
 * appended, or it would return 0 when there's already an *elem in the list. */
int generic_list_add(
    struct generic_list *glist, const void *elem, 
    int(*cmp)(const void*, const void*))
{
    /* see if elem is already in the list*/
    void *p_same = generic_list_find(glist, elem, cmp);

    if (p_same == NULL)
    {
        /* elem not in the list, so append it */
        generic_list_push_back(glist, elem);
        return 1;
    }
    else {
        return 0;  /* elem already in the list, do nothing */
    }
}
