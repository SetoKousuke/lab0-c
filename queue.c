#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *l = (struct list_head *) malloc(sizeof(struct list_head));

    if (l)
        INIT_LIST_HEAD(l);

    return l;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *i, *t;

    list_for_each_entry_safe (i, t, l, list) {
        free(i->value);
        free(i);
    }

    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (s && head) {
        element_t *t;
        if (!(t = (element_t *) malloc(sizeof(element_t))))
            return false;
        t->value = strdup(s);
        list_add(&(t->list), head);
        // cppcheck-suppress memleak
        return true;
    }
    return false;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (s && head) {
        element_t *t;
        if (!(t = (element_t *) malloc(sizeof(element_t))))
            return false;
        t->value = strdup(s);
        list_add_tail(&(t->list), head);
        // cppcheck-suppress memleak
        return true;
    }
    return false;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *t = list_first_entry(head, element_t, list);
    list_del(&(t->list));

    if (sp) {
        strncpy(sp, t->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return t;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *t = list_last_entry(head, element_t, list);
    list_del(&(t->list));

    if (sp) {
        strncpy(sp, t->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return t;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int sz = 0;
    struct list_head *t;

    list_for_each (t, head)
        sz++;

    return sz;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *first, *second;
    first = head->next;
    second = first->next;
    while (first != head && second != head) {
        list_move(first, second);
        first = first->next;
        second = first->next;
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head) {}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head) {}
