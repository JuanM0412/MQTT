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

TreeNode* createTreeNode(const char *name);
void freeTreeNode(TreeNode *node);
void insertMessage(TreeNode *root, const char *input);
void printTree(TreeNode *node, int depth);
void publish(TreeNode *root, const char *topic, const char *message);
void subscribe(TreeNode *root, const char *topic, int user);

#endif /* TREE_H */