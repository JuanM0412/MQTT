#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tree.h"



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
    new_node->users = NULL; 
    new_node->num_users = 0; 
    
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


void PUB(TreeNode *root, const char *topic, const char *message) {
    char *topic_copy = strdup(topic);
    char *token = strtok(topic_copy, "/");
    TreeNode *current_node = root;

    while (token != NULL) {
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

    if (current_node->messages == NULL) {
        current_node->messages = (char **)malloc(sizeof(char *));
    } else {
        current_node->messages = (char **)realloc(current_node->messages, (current_node->num_messages + 1) * sizeof(char *));
    }
    current_node->messages[current_node->num_messages] = strdup(message);
    current_node->num_messages++;

    free(topic_copy);
}


void printTree(TreeNode *node, int depth) {
    if (node == NULL) return;

    printf("%*s- %s\n", depth * 4, "", node->name);

    for (int i = 0; i < node->num_messages; i++) {
        printf("%*s  * Message: %s\n", depth * 4, "", node->messages[i]);
    }

    for (int i = 0; i < node->num_users; i++) {
        printf("%*s  * User: %d\n", depth * 4, "", node->users[i]);
    }

    for (int i = 0; i < node->num_children; i++) {
        printTree(node->children[i], depth + 1);
    }
}



void SUB(TreeNode *root, const char *topic, int user) {
    char *topic_copy = strdup(topic);
    char *token = strtok(topic_copy, "/");
    TreeNode *current_node = root;

    while (token != NULL) {
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

    if (current_node->users == NULL) {
        current_node->users = (int *)malloc(sizeof(int));
    } else {
        current_node->users = (int *)realloc(current_node->users, (current_node->num_users + 1) * sizeof(int));
    }
    current_node->users[current_node->num_users] = user;
    current_node->num_users++;

    free(topic_copy);
}




int main() {
    TreeNode *root = createTreeNode("/");

    char input_topic[100];
    char input_message[100];
    int input_user;
    char option;
    char continue_input = 'y';

    while (continue_input == 'y' || continue_input == 'Y') {
        printf("Seleccione una opción:\n");
        printf("PUB (P) o SUB (S): ");
        scanf(" %c", &option);
        getchar(); 

        printf("Ingrese el topic en el formato /nodo1/nodo2/nodoN: \n");
        fgets(input_topic, sizeof(input_topic), stdin);
        input_topic[strcspn(input_topic, "\n")] = '\0';

        if (option == 'P' || option == 'p') {
            printf("Ingrese el mensaje: \n");
            fgets(input_message, sizeof(input_message), stdin);
            input_message[strcspn(input_message, "\n")] = '\0';

            PUB(root, input_topic, input_message);
        } else if (option == 'S' || option == 's') {
            printf("Ingrese el usuario: \n");
            scanf("%d", &input_user);
            getchar(); 

            SUB(root, input_topic, input_user);
        } else {
            printf("Opción no válida.\n");
        }

        printf("¿Desea insertar otro mensaje? (y/n): ");
        scanf(" %c", &continue_input);
        getchar(); 
    }

    printTree(root, 0);

    freeTreeNode(root);

    return 0;
}



