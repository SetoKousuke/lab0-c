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
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

typedef int
    __attribute__((nonnull(2, 3))) (*list_cmp_func_t)(void *,
                                                      const struct list_head *,
                                                      const struct list_head *);

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
        if (!(t->value = strdup(s))) {
            free(t);
            return false;
        }
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
        if (!(t->value = strdup(s))) {
            free(t);
            return false;
        }
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
    if (!head || list_empty(head))
        return false;
    struct list_head *f, *s;  // fast & slow ptr
    for (s = head->next, f = s->next; f != head && f != head->next;
         f = f->next->next, s = s->next) {
    }
    element_t *t = list_entry(s, element_t, list);
    list_del(s);
    free(t->value);
    free(t);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    if (list_is_singular(head))
        return true;
    element_t *iter, *tmp;
    int trig = 0;
    list_for_each_entry_safe (iter, tmp, head, list)
        if ((iter->list.next != head) &&
            !strcmp(iter->value,
                    // cppcheck-suppress nullPointer
                    list_entry(iter->list.next, element_t, list)->value)) {
            list_del(&iter->list);
            q_release_element(iter);
            trig = 1;
        } else if (trig) {
            list_del(&iter->list);
            q_release_element(iter);
            trig = 0;
        }
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
    while (second != head && first != head) {
        list_move(first, second);
        first = first->next;
        second = first->next;
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *t1, *t2;
    t1 = head->prev;
    while (t1 != head) {
        t2 = t1->prev;
        t1->prev = t1->next;
        t1->next = t2;
        t1 = t1->next;
    }
    t2 = t1->prev;
    t1->prev = t1->next;
    t1->next = t2;

    return;
}

__attribute__((nonnull(2, 3, 4))) static struct list_head *
merge(void *priv, list_cmp_func_t cmp, struct list_head *a, struct list_head *b)
{
    // cppcheck-suppress unassignedVariable
    struct list_head *head, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

__attribute__((nonnull(2, 3, 4, 5))) static void merge_final(
    void *priv,
    list_cmp_func_t cmp,
    struct list_head *head,
    struct list_head *a,
    struct list_head *b)
{
    struct list_head *tail = head;
    char count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(priv, a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    tail->next = b;
    do {
        /*
         * If the merge is highly unbalanced (e.g. the input is
         * already sorted), this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if (unlikely(!++count))
            cmp(priv, b, b);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

__attribute__((nonnull(2, 3))) void list_sort(void *priv,
                                              struct list_head *head,
                                              list_cmp_func_t cmp)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    /*
     * Data structure invariants:
     * - All lists are singly linked and null-terminated; prev
     *   pointers are not maintained.
     * - pending is a prev-linked "list of lists" of sorted
     *   sublists awaiting further merging.
     * - Each of the sorted sublists is power-of-two in size.
     * - Sublists are sorted by size and age, smallest & newest at front.
     * - There are zero to two sublists of each size.
     * - A pair of pending sublists are merged as soon as the number
     *   of following pending elements equals their size (i.e.
     *   each time count reaches an odd multiple of that size).
     *   That ensures each later final merge will be at worst 2:1.
     * - Each round consists of:
     *   - Merging the two sublists selected by the highest bit
     *     which flips when count is incremented, and
     *   - Adding an element from the input as a size-1 sublist.
     */
    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(priv, cmp, b, a);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(priv, cmp, pending, list);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(priv, cmp, head, pending, list);
}

int custom_cmp(void *priv, const struct list_head *a, const struct list_head *b)
{
    element_t *ta, *tb;
    ta = list_entry(a, element_t, list);
    tb = list_entry(b, element_t, list);
    return strcmp(ta->value, tb->value);
}
/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    list_cmp_func_t cmp = &custom_cmp;
    // convert to a null-terminated singly-linked list
    list_sort(NULL, head, cmp);

    // https://github.com/torvalds/linux/blob/master/lib/list_sort.c
}
