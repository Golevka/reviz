#ifndef __GENERIC_LIST_HEADER__
#define __GENERIC_LIST_HEADER__



/* Generic linear data structure similar to std::vector in C++, this structure
 * is also used as sets although it would be very inefficient. */
struct generic_list
{
    int elem_size;   /* size of each element */
    int length;      /* num of elements in the list */
    int capacity;    /* capacity of this list, can be expanded dynamically */
    char *p_dat;     /* pointer to actual data */
};

#define INITIAL_CAPACITY  8     /* default capacity value for newly created
                                 * generic list */

/* Create a generic list for some kind of data elements, elem_size specified
 * the size in bytes of each data element, initial_capacity is the amount of
 * space reserved for furture use (it cannot be zero). */
void __create_generic_list(
    int elem_size, int initial_capacity, struct generic_list *glist);

/* Interface wrapper for __create_generic_list */
#define create_generic_list(type, glist)                                \
    __create_generic_list(sizeof(type), INITIAL_CAPACITY, glist)

void generic_list_duplicate(
    struct generic_list *dest, const struct generic_list *src);

/* Free the memory allocated for the generic list */
void destroy_generic_list(struct generic_list *glist);


/* Append an element to the tail of specified generic list */
void generic_list_push_back(struct generic_list *glist, const void *elem);

/* Remove the last element in the list */
void generic_list_pop_back(struct generic_list *glist);

/* Get the pointer to the element on the tail of the list */
void *generic_list_back(struct generic_list *glist);

/* Get pointer to the first element  */
void *generic_list_front(struct generic_list *glist);

/* Empty the generic list */
void generic_list_clear(struct generic_list *glist);


/* Compare function template for POD types */
#define MAKE_COMPARE_FUNCTION(postfix, type) \
    static int __cmp_##postfix (const void *a_, const void *b_)         \
    {                                                                   \
        type a = *((type*) a_);                                         \
        type b = *((type*) b_);                                         \
        if (a < b) return -1;                                           \
        else if (a > b) return 1;                                       \
        else return 0;                                                  \
    }

/* Find element in the list using specified compare function, it returns the
 * pointer to the element found in the list, or a NULL is returned if *elem is
 * not in glist */
void *generic_list_find(
    struct generic_list *glist, const void *elem, 
    int(*cmp)(const void*, const void*));

/* Add an element to the list only if this element is not in the list (we
 * actually regard the glist as a set). It would return 1 if *elem is actually
 * appended, or it would return 0 when there's already an *elem in the list. */
int generic_list_add(
    struct generic_list *glist, const void *elem, 
    int(*cmp)(const void*, const void*));



#endif /* __GENERIC_LIST_HEADER__ */
