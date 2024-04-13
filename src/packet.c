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
#include "../include/packet.h"

unsigned int get_packet_id() {
    static unsigned int packet_id = 0;
    return ++packet_id;
}

MQTT_Packet create_connect_packet(u_int16_t keep_alive, const char* client_id, const char* username, const char* password) {
    // Calcular la longitud del cliente ID
    size_t client_id_length = strlen(client_id);
    size_t username_length = strlen(username);
    size_t password_length = strlen(password);
    size_t offset = 0;
    size_t payload_length = client_id_length + username_length + password_length + 6;

    // Asignar memoria para el paquete
    MQTT_Packet connect_packet;
    connect_packet.variable_header = malloc(10);
    connect_packet.payload = malloc(payload_length);

    // Rellenar el encabezado fijo y la longitud restante
    connect_packet.fixed_header = MQTT_FIXED_HEADER_CONNECT; // CONNECT
    connect_packet.remaining_length = 10 + payload_length;

    // Rellenar el encabezado variable
    // Protocol Name (MQTT)
    connect_packet.variable_header[0] = 0x00;
    connect_packet.variable_header[1] = 0x04;
    connect_packet.variable_header[2] = 'M';
    connect_packet.variable_header[3] = 'Q';
    connect_packet.variable_header[4] = 'T';
    connect_packet.variable_header[5] = 'T';

    // Protocol Level (4)
    connect_packet.variable_header[6] = 0x04;

    // Connect Flags (Clean Session)
    connect_packet.variable_header[7] = 0x02;

    // Keep Alive
    connect_packet.variable_header[8] = keep_alive >> 8;
    connect_packet.variable_header[9] = keep_alive & 0xFF;

    connect_packet.payload[offset++] = client_id_length >> 8;
    connect_packet.payload[offset++] = client_id_length & 0xFF;
    memcpy(&connect_packet.payload[offset], client_id, client_id_length);
    offset += client_id_length;

    connect_packet.payload[offset++] = username_length >> 8;
    connect_packet.payload[offset++] = username_length & 0xFF;
    memcpy(&connect_packet.payload[offset], username, username_length);
    offset += username_length;

    connect_packet.payload[offset++] = password_length >> 8;
    connect_packet.payload[offset++] = password_length & 0xFF;
    memcpy(&connect_packet.payload[offset], password, password_length);
    offset += username_length;

    return connect_packet;
}

MQTT_Packet create_connack_packet(u_int8_t return_code) {
    MQTT_Packet connack_packet;

    connack_packet.fixed_header = MQTT_FIXED_HEADER_CONNACK;
    connack_packet.remaining_length = 0x02;

    connack_packet.variable_header = malloc(2);
    connack_packet.variable_header[0] = 0x00;
    connack_packet.variable_header[1] = return_code;

    return connack_packet;
}

MQTT_Packet create_publish_packet(const char* topic, const char* message) {
    size_t topic_length = strlen(topic);
    size_t message_length = strlen(message);
    unsigned int packet_id = get_packet_id();

    // Calcular la longitud total del paquete PUBLISH
    size_t packet_length = 4 + topic_length + message_length; // 2 bytes para el Length del encabezado variable y 2 para el id

    // Asignar memoria para el paquete
    MQTT_Packet publish_packet;
    publish_packet.variable_header = malloc(5 + topic_length);
    publish_packet.payload = NULL;

    // Rellenar el encabezado fijo y la longitud restante
    publish_packet.fixed_header = MQTT_FIXED_HEADER_PUBLISH;
    publish_packet.remaining_length = packet_length;  // restar 2 para el Length del encabezado variable

    // Rellenar el encabezado variable
    // Topic Name
    publish_packet.variable_header[0] = topic_length >> 8;
    publish_packet.variable_header[1] = topic_length & 0xFF;
    memcpy(&publish_packet.variable_header[2], topic, topic_length);
    publish_packet.variable_header[topic_length + 2] = (packet_id >> 8) & 0xFF;
    publish_packet.variable_header[topic_length + 3] = packet_id & 0xFF;

    // Payload (mensaje)
    publish_packet.payload = malloc(message_length);
    memcpy(publish_packet.payload, message, message_length);
    
    return publish_packet;
}

MQTT_Packet create_subscribe_packet(const char** topics_to_subscribe) {
    MQTT_Packet subscribe_packet;
    int num_topics = 0;
    int payload_length = 0;

    // Calculate the number of topics and total payload length
    while (topics_to_subscribe[num_topics] != NULL) {
        payload_length += strlen(topics_to_subscribe[num_topics]);
        num_topics++;
    }

    // Calculate total packet size
    size_t packet_size = 2 + payload_length + (num_topics * 3); // 2 bytes for each topic length

    // Allocate memory for the packet
    subscribe_packet.variable_header = malloc(2);
    subscribe_packet.payload = malloc(packet_size);

    if (subscribe_packet.variable_header == NULL || subscribe_packet.payload == NULL) {
        exit(EXIT_FAILURE);
    }

    // Fill in the fixed header of the packet
    subscribe_packet.fixed_header = MQTT_FIXED_HEADER_SUBSCRIBE; // MQTT subscribe packet
    subscribe_packet.remaining_length = packet_size; // -2 because we don't include fixed_header and remaining_length in size

    // Message ID (any valid value)
    int packet_id = get_packet_id();
    subscribe_packet.variable_header[0] = packet_id >> 8;
    subscribe_packet.variable_header[1] = packet_id & 0xFF;

    // Copy topics into the payload
    int offset = 0;
    for (int i = 0; i < num_topics; i++) {
        const char *topic = topics_to_subscribe[i];
        size_t topic_length = strlen(topic);

        // Copy topic length (MSB and LSB)
        subscribe_packet.payload[offset++] = (topic_length >> 8) & 0xFF;
        subscribe_packet.payload[offset++] = topic_length & 0xFF;

        // Copy topic
        memcpy(subscribe_packet.payload + offset, topic, topic_length);
        offset += topic_length;
        subscribe_packet.payload[offset++] = 0x00;
    }

    return subscribe_packet;
}

MQTT_Packet create_suback_packet(unsigned int packet_id, int num_topics) {
    MQTT_Packet suback_packet;

    suback_packet.fixed_header = MQTT_FIXED_HEADER_SUBACK;
    suback_packet.remaining_length = 2 + num_topics;
    suback_packet.variable_header = malloc(2);

    suback_packet.variable_header[0] = packet_id >> 8;
    suback_packet.variable_header[1] = packet_id & 0xFF;

    suback_packet.payload = malloc(num_topics);
    
    return suback_packet;
}

MQTT_Packet create_disconnect_packet() {
    // Asignar memoria para el paquete
    MQTT_Packet disconnect_packet;
    disconnect_packet.variable_header = NULL;
    disconnect_packet.payload = NULL;

    // Rellenar el encabezado fijo y la longitud restante
    disconnect_packet.fixed_header = MQTT_FIXED_HEADER_DISCONNECT; // DISCONNECT
    disconnect_packet.remaining_length = 0x00;

    return disconnect_packet;
}

void free_packet(MQTT_Packet* packet) {
    if (packet->variable_header != NULL) {
        free(packet->variable_header);
    }
    if (packet->payload != NULL) {
        free(packet->payload);
    }
}
