#ifndef SERVER_H
#define SERVER_H

#include "../include/packet.h"

#define TreeNode struct TreeNode
#define MAX 360
#define SA struct sockaddr

typedef struct {
    TreeNode *tree;
    pthread_mutex_t mutex;
} Tree;

Tree* get_tree();
void handle_tree(int request, const char *topic, const char *message);
void disconnect_client(int connfd);
void handle_subscribe_packet(MQTT_Packet packet, int connfd);
void handle_publish_packet(MQTT_Packet packet, int connfd);
void handle_connect_packet(MQTT_Packet packet, int connfd);
void identify_packet(MQTT_Packet packet, int connfd);
MQTT_Packet receive_packet_from_client(int connfd);
void *process_connection(void *arg);

#endif /* SERVER_H */
