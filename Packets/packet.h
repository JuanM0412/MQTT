#include <stdlib.h>

typedef struct {
    u_int8_t fixed_header;
    u_int8_t remaining_length;
    u_int8_t* variable_header;
    u_int8_t* payload;
} MQTT_Packet;

MQTT_Packet create_connect_package(MQTT_Packet *packet);
MQTT_Packet create_publish_packet(const char* topic, const char* message);
MQTT_Packet create_disconnect_packet();
void free_packet(MQTT_Packet *packet);
