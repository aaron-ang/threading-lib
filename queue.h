#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef struct queue_node
{
    int thread_index;
    struct queue_node *next;
} queue_node_t;

typedef struct
{
    queue_node_t *front;
    queue_node_t *rear;
    int size;
} queue_t;

static void queue_init(queue_t *q)
{
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

static bool queue_is_empty(queue_t *q)
{
    return q->size == 0;
}

static void queue_enqueue(queue_t *q, int thread_index)
{
    queue_node_t *new_node = malloc(sizeof(queue_node_t));
    assert(new_node);

    new_node->thread_index = thread_index;
    new_node->next = NULL;

    if (queue_is_empty(q))
    {
        q->front = new_node;
    }
    else
    {
        q->rear->next = new_node;
    }
    q->rear = new_node;
    q->size++;
}

static int queue_dequeue(queue_t *q)
{
    if (queue_is_empty(q))
        return -1;

    queue_node_t *node = q->front;
    int thread_index = node->thread_index;

    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;

    free(node);
    q->size--;

    return thread_index;
}
