#include "pid_list.h"

/* this implementation is a doubly-linked, circular
 * list with a dummy head node
 */

/* make a new empty list */
node *new_list()
{
    node *head = (node *)malloc(sizeof(node));
    head->pid = (pid_t)-1;
    head->next = head;
    head->prev = head;
    return head;
}

/* add new pid list */
void add_node(node *head, pid_t pid)
{
    if (head == NULL)
    {
        return;
    }

    node *new = (node *)malloc(sizeof(node));
    new->pid = pid;

    // add node to the end of the list
    new->prev = head->prev;
    new->next = head;

    head->prev->next = new;
    head->prev = new;
}

/* remove a pid from the list */
void remove_node(node *head, pid_t pid)
{
    if (head == NULL)
    {
        return;
    }

    node *cur = head->next;
    while (cur->pid != pid && cur != head)
    {
        cur = cur->next;
    }

    if (cur == head)
    {
        // didn't find pid, so just return
        return;
    }

    // remove cur from the list
    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;

    free(cur);
}

/* free every node in the list */
void free_list(node *head)
{
    if (head == NULL)
    {
        return;
    }

    node *cur = head->next;
    while (cur != head)
    {
        cur = cur->next;
        free(cur->prev);
    }
    free(head);
}
