#ifndef SEND_PACKETS_TO_CLIENT_H
#define SEND_PACKETS_TO_CLIENT_H

#include <stdlib.h>
#include "packet.h"

void send_publish_to_client(int connfd, MQTT_Packet packet);
void send_connack_to_client(int connfd, MQTT_Packet packet);
void send_suback_to_client(int connfd, MQTT_Packet packet);

#endif /* SEND_PACKETS_TO_CLIENT_H */