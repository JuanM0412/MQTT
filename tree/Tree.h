#ifndef TREE_H
#define TREE_H

typedef struct TreeNode {
    char *name;
    struct TreeNode **children;
    int num_children;
    char **messages;
    int num_messages;
<<<<<<< HEAD
    int *users; 
    int num_users; 
=======
>>>>>>> 3c24461 (Tree basic structure)
} TreeNode;

TreeNode* createTreeNode(const char *name);
void freeTreeNode(TreeNode *node);
<<<<<<< HEAD
void insertMessage(TreeNode *root, const char *topic, const char *message);
void printTree(TreeNode *node, int depth);
void PUB(TreeNode *root, const char *topic, const char *message); 
void SUB(TreeNode *root, const char *topic, int user); 
=======
void insertMessage(TreeNode *root, const char *input);
void printTree(TreeNode *node, int depth);
>>>>>>> 3c24461 (Tree basic structure)

#endif /* TREE_H */
