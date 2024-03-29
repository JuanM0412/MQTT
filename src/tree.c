#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/tree.h"

TreeNode* createTreeNode(const char *name) {
    TreeNode *new_node = (TreeNode*)malloc(sizeof(TreeNode));
    if (new_node == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el nuevo nodo.\n");
        exit(EXIT_FAILURE);
    }
    
    new_node->name = strdup(name);
    new_node->children = NULL;
    new_node->num_children = 0;
    new_node->messages = NULL;
    new_node->num_messages = 0;
    
    return new_node;
}


void freeTreeNode(TreeNode *node) {
    if (node == NULL) return;

    free(node->name);

    for (int i = 0; i < node->num_messages; i++) {
        free(node->messages[i]);
    }

    free(node->messages);

    for (int i = 0; i < node->num_children; i++) {
        freeTreeNode(node->children[i]);
    }

    free(node->children);
    free(node);
}


void insertMessage(TreeNode *root, const char *input) {
    char *copy = strdup(input);
    char *token = strtok(copy, "/");
    TreeNode *current_node = root;

    while (token != NULL) {
        if (strchr(token, ' ') != NULL) {
            if (current_node->messages == NULL) {
                current_node->messages = (char **)malloc(sizeof(char *));
            } else {
                current_node->messages = (char **)realloc(current_node->messages, (current_node->num_messages + 1) * sizeof(char *));
            }
            current_node->messages[current_node->num_messages] = strdup(token);
            current_node->num_messages++;
            break;
        }

        int child_index = -1;
        for (int i = 0; i < current_node->num_children; i++) {
            if (strcmp(current_node->children[i]->name, token) == 0) {
                child_index = i;
                break;
            }
        }

        if (child_index == -1) {
            current_node->children = (TreeNode **)realloc(current_node->children, (current_node->num_children + 1) * sizeof(TreeNode *));
            current_node->children[current_node->num_children] = createTreeNode(token);
            current_node->num_children++;
            child_index = current_node->num_children - 1;
        }

        current_node = current_node->children[child_index];

        token = strtok(NULL, "/");
    }

    free(copy);
}

void printTree(TreeNode *node, int depth) {
    if (node == NULL) return;

    printf("%*s- %s\n", depth * 4, "", node->name);
    for (int i = 0; i < node->num_messages; i++) {
        printf("%*s  * %s\n", depth * 4, "", node->messages[i]);
    }
    
    for (int i = 0; i < node->num_children; i++) {
        printTree(node->children[i], depth + 1);
    }
}
