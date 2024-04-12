#ifndef HANDLE_PACKET_H
#define HANDLE_PACKET_H

#include <stdlib.h>
#include "packet.h"

void send_publish_to_client(int connfd, MQTT_Packet packet);
void send_connack_to_client(int connfd, MQTT_Packet packet);
void send_suback_to_client(int connfd, MQTT_Packet packet);

#endif /* HANDLE_PACKET_H */