#ifndef SEND_PACKETS_TO_SERVER_H
#define SEND_PACKETS_TO_SERVER_H

#include "packet.h"

void send_connect_to_server(int sockfd, MQTT_Packet packet);
void send_publish_to_server(int sockfd, MQTT_Packet packet);
int send_subscribe_to_server(int sockfd, MQTT_Packet packet);

#endif /* SEND_PACKETS_TO_SERVER_H */