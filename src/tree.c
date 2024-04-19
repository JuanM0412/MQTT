#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "../include/tree.h"
#include "../include/packet.h"
#include "../include/encode.h"
#include "../include/queue.h"
#include "../include/send_packets_to_client.h"

//This function get a Topic, and when match a "+"
//returns the subtopic after the appearance of the wildcard.
char* getSubtopic(const char* str) {
    char* substr = strstr(str, "+");
    if (substr != NULL) {
        return substr + 1; 
    } else {
        return NULL; 
    }
}

//This function creates a new node for the tree
//and returns it
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

//This function frees the memory of the tree
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

//This function print in the console the structure of the tree
void printHex(const char *str) {
    while (*str != '\0') {
        printf("%02X ", (unsigned char)*str);
        str++;
    }
    printf("\n");
}

void printTree(TreeNode *node, int depth) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    printf("- %s\n", node->name);

    if (node->messages != NULL) {
        for (int i = 0; i < node->num_messages; i++) {
            for (int j = 0; j < depth + 1; j++) {
                printf("  ");
            }
            printf("* Message: ");
            printHex(node->messages[i]);
        }
    }

    if (node->users != NULL) {
        for (int i = 0; i < node->num_users; i++) {
            for (int j = 0; j < depth + 1; j++) {
                printf("  ");
            }
            printf("* User: %02X\n", node->users[i]);
        }
    }

    for (int i = 0; i < node->num_children; i++) {
        printTree(node->children[i], depth + 1);
    }
}

//This function handles SUB requests for the tree.
//The parameter root, in the first call, is the root of the tree.
//The parameter topic, is the topic requested by the user in the SUB package.
//The parameter user, is the ID of the socket associated to the user.
//The parameter wildcard_flag show if any wildcard of type "+" has been found. 
//On the first call to the function it starts at zero
void subscribe(TreeNode *root, const char *topic, int user, int wildcard_flag) {
    char *topic_copy = strdup(topic);
    char *token = strtok(topic_copy, "/");
    TreeNode *current_node = root;

    //This loop iterates over each element of the topic.
    while (token != NULL) {

        //Validates if the element of the topic is a single-level wildcard 
        if (strcmp(token, "+") == 0) {

            //If it is the last one, subscribe the user to each child of the current node.
            if (strtok(NULL, "/") == NULL) {
                for (int i = 0; i < current_node->num_children; i++){
                    if (current_node->children[i]->users == NULL) {
                        current_node->children[i]->users = (int *)malloc(sizeof(int));
                    } else {
                        current_node->children[i]->users = (int *)realloc(current_node->children[i]->users, (current_node->children[i]->num_users + 1) * sizeof(int));
                    }
                    current_node->children[i]->users[current_node->children[i]->num_users] = user;
                    current_node->children[i]->num_users++;
                }
                return;
            
            // If it isn't the last one, calls recursively the subscribe function for each child of
            //  current node with the subtopic associated.
            } else{
                char *subtopic = getSubtopic(topic);
                for (int i = 0; i < current_node->num_children; i++){
                    char *subtopic_copy = strdup(subtopic);
                    // The flag for the wildcard is useful 
                    // in order to not create unnecessary subscriptions in 
                    // the recursively calls.
                    subscribe(current_node->children[i], subtopic_copy, user, 1);
                }
                return;
                
            }
        }

        //Validates if the element of the topic is a multiple-level wildcard 
        if (strcmp(token, "#") == 0) {
            //The "#" only should be in the last element of the topic.
            if (strtok(NULL, "/") != NULL) {
                fprintf(stderr, "Error: El wildcard '#' solo debe aparecer al final del tópico.\n");
                free(topic_copy);
                return;
            }

            Queue *queue = createQueue();
            enqueue(queue, current_node);

            // Use a BFS in order tu subscribe the user in all the adjacent topics
            while (!isEmpty(queue)) {
                TreeNode *node = dequeue(queue);

                if (node->users == NULL) {
                    node->users = (int *)malloc(sizeof(int));
                } else {
                    node->users = (int *)realloc(node->users, (node->num_users + 1) * sizeof(int));
                }
                node->users[node->num_users] = user;
                node->num_users++;

                for (int i = 0; i < node->num_children; i++) {
                    enqueue(queue, node->children[i]);
                }
            }

            freeQueue(queue);
            break;        
        } 

        // Search the index of the child associated to the topic
        int child_index = -1;
        for (int i = 0; i < current_node->num_children; i++) {
            if (strcmp(current_node->children[i]->name, token) == 0) {
                child_index = i;
                break;
            }
        }

        //If it is not a recursively call of a "+" wildcard, creates the topic and subscribe the user in this one.
        if (wildcard_flag == 0){
            if (child_index == -1) {
                current_node->children = (TreeNode **)realloc(current_node->children, (current_node->num_children + 1) * sizeof(TreeNode *));
                current_node->children[current_node->num_children] = createTreeNode(token);
                current_node->num_children++;
                child_index = current_node->num_children - 1;
            }
        } else {
            if (child_index == -1) {
                return;
            }
        }

        // Change the current node for the child node 
        current_node = current_node->children[child_index];

        // Switch to the next topic
        token = strtok(NULL, "/");    
    }

    // Check duplicates users in the required topic
    for (int i = 0; i < current_node->num_users; i++) {
        if (current_node->users[i] == user) {
            free(topic_copy);
            return;
        }
    }

    // Add the user in the required topic
    if (current_node->users == NULL) {
        current_node->users = (int *)malloc(sizeof(int));
    } else {
        current_node->users = (int *)realloc(current_node->users, (current_node->num_users + 1) * sizeof(int));
    }
    current_node->users[current_node->num_users] = user;
    current_node->num_users++;

    free(topic_copy);
}

//This function handles PUB requests for the tree
void publish(TreeNode *root, const char *topic, const char *message) {
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
    
    MQTT_Packet packet = create_publish_packet(encodeMessageToUTF8(topic), encodeMessageToUTF8(current_node->messages[current_node->num_messages - 1]));
    if (current_node->users != NULL) {
        for (int i = 0; i < current_node->num_users; i++) {
            send_publish_to_client(current_node->users[i], packet);
        }
        free_packet(&packet);
    }

    free(topic_copy);
}