#ifndef SERVER_H
#define SERVER_H

#include "../include/packet.h"

#define MAX 80
#define SA struct sockaddr

void disconnect_client(int connfd);
void handle_subscribe_packet(MQTT_Packet packet);
void handle_publish_packet(MQTT_Packet packet);
void handle_connect_packet(MQTT_Packet packet);
void identify_packet(MQTT_Packet packet, int connfd);
MQTT_Packet receive_packet_from_client(int connfd);
void *process_connection(void *arg);

#endif /* SERVER_H */
