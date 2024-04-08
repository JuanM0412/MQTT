#ifndef TREE_H
#define TREE_H

typedef struct TreeNode {
    char *name;
    struct TreeNode **children;
    int num_children;
    char **messages;
    int num_messages;
    int *users; 
    int num_users;
} TreeNode;

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


TreeNode* createTreeNode(const char *name);
void subscribeAllAdjacent(TreeNode *node, int user);
void freeTreeNode(TreeNode *node);
void printTree(TreeNode *node, int depth);
void publish(TreeNode *root, const char *topic, const char *message);
void subscribe(TreeNode *root, const char *topic, int user, int wildcard_flag);

#endif /* TREE_H */