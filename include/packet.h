#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>

#define MQTT_FIXED_HEADER_CONNECT 0x10
#define MQTT_FIXED_HEADER_CONNACK 0x20
#define MQTT_FIXED_HEADER_PUBLISH 0x30
#define MQTT_FIXED_HEADER_SUBSCRIBE 0x82
#define MQTT_FIXED_HEADER_SUBACK 0x90
#define MQTT_FIXED_HEADER_DISCONNECT 0xE0

typedef struct {
    u_int8_t fixed_header;
    u_int8_t remaining_length;
    u_int8_t* variable_header;
    u_int8_t* payload;
} MQTT_Packet;

MQTT_Packet create_connect_packet(u_int16_t keep_alive, const char* client_id, const char* username, const char* password);
MQTT_Packet create_connack_packet(u_int8_t return_code);
MQTT_Packet create_publish_packet(const char* topic, const char* message);
MQTT_Packet create_subscribe_packet(const char** topics_to_subscribe);
MQTT_Packet create_suback_packet(unsigned int packet_id, int num_topics);
MQTT_Packet create_disconnect_packet();
void free_packet(MQTT_Packet *packet);

#endif /* PACKET_H */