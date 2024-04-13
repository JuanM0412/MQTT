#ifndef QUEUE_H
#define QUEUE_H

#include "tree.h"

typedef struct QueueNode {
    TreeNode *data;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
} Queue;

QueueNode *createQueueNode(TreeNode *data);
Queue *createQueue();
int isEmpty(Queue *queue);
void enqueue(Queue *queue, TreeNode *data);
TreeNode *dequeue(Queue *queue);
void freeQueue(Queue *queue);

#endif /* QUEUE_H */