#include <stdlib.h>

typedef struct {
    u_int8_t fixed_header;
    u_int8_t remaining_length;
    u_int8_t* variable_header;
    u_int8_t* payload;
} MQTT_Packet;

MQTT_Packet create_connect_packet(u_int16_t keep_alive, const char* client_id);
MQTT_Packet create_publish_packet(const char* topic, const char* message);
MQTT_Packet create_subscribe_packet(const char* topics_to_subscribe);
MQTT_Packet create_disconnect_packet();
void free_packet(MQTT_Packet *packet);
