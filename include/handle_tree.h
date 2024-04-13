#ifndef HANDLE_TREE_H
#define HANDLE_TREE_H

#include "tree.h"
#include <pthread.h>

#define TreeNode struct TreeNode

typedef struct {
    TreeNode *tree;
    pthread_mutex_t mutex;
} Tree;


Tree* get_tree();
void insert_publish(const char *topic, const char *message);
void insert_subscribe(const char *topic, int connfd);

#endif /* HANDLE_TREE_H */
