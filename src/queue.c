#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/queue.h"

QueueNode *createQueueNode(TreeNode *data) {
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    if (newNode) {
        newNode->data = data;
        newNode->next = NULL;
    }
    return newNode;
}

Queue *createQueue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (queue) {
        queue->front = queue->rear = NULL;
    }
    return queue;
}

int isEmpty(Queue *queue) {
    return queue->front == NULL;
}

void enqueue(Queue *queue, TreeNode *data) {
    QueueNode *newNode = createQueueNode(data);
    if (newNode) {
        if (isEmpty(queue)) {
            queue->front = queue->rear = newNode;
        } else {
            queue->rear->next = newNode;
            queue->rear = newNode;
        }
    }
}

TreeNode *dequeue(Queue *queue) {
    if (isEmpty(queue)) {
        return NULL;
    }
    QueueNode *temp = queue->front;
    TreeNode *data = temp->data;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return data;
}

void freeQueue(Queue *queue) {
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}