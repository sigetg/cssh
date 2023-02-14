#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct node
{
    pid_t pid;
    struct node *next;
    struct node *prev;
} node;

/* make a new empty list */
node *new_list();

/* add new pid to list */
void add_node(node *head, pid_t pid);

/* remove a pid from the list */
void remove_node(node *head, pid_t pid);

/* free every node in the list */
void free_list(node *head);
