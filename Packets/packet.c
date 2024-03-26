#include <stdlib.h>
#include "packet.h"
#include <string.h>
#include <sys/types.h>

#define MQTT_FIXED_HEADER_PUBLISH 0x30
#define MAX_TOPIC_LENGTH 256  // Tamaño máximo del tema
#define MAX_MESSAGE_LENGTH 1024  // Tamaño máximo del mensaje

MQTT_Packet create_connect_packet(u_int16_t keep_alive, const char* client_id) {
    // Calcular la longitud del cliente ID
    size_t client_id_length = strlen(client_id);

    // Longitud total del paquete CONNECT
    size_t packet_length = 10 + client_id_length;

    // Asignar memoria para el paquete
    MQTT_Packet connect_packet;
    connect_packet.variable_header = malloc(packet_length);
    connect_packet.payload = NULL;

    // Rellenar el encabezado fijo y la longitud restante
    connect_packet.fixed_header = 0x10; // CONNECT
    connect_packet.remaining_length = packet_length;

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
    connect_packet.variable_header[8] = keep_alive >> 8; // MSB
    connect_packet.variable_header[9] = keep_alive & 0xFF; // LSB

    // Client ID Length
    connect_packet.variable_header[10] = client_id_length >> 8; // MSB
    connect_packet.variable_header[11] = client_id_length & 0xFF; // LSB

    // Client ID
    memcpy(&connect_packet.variable_header[12], client_id, client_id_length);

    return connect_packet;
}

MQTT_Packet create_publish_packet(const char* topic, const char* message) {
    size_t topic_length = strlen(topic);
    size_t message_length = strlen(message);

    // Codificar el tema y el mensaje a UTF-8
    unsigned char *encoded_topic = encodeMessageToUTF8(topic);
    unsigned char *encoded_message = encodeMessageToUTF8(message);

    // Calcular la longitud total del paquete PUBLISH
    size_t packet_length = 2 + topic_length + message_length; // 2 bytes para el Length del encabezado variable

    // Asignar memoria para el paquete
    MQTT_Packet publish_packet;
    publish_packet.variable_header = malloc(packet_length);
    publish_packet.payload = NULL;

    // Rellenar el encabezado fijo y la longitud restante
    publish_packet.fixed_header = MQTT_FIXED_HEADER_PUBLISH;
    publish_packet.remaining_length = packet_length - 2;  // restar 2 para el Length del encabezado variable

    // Rellenar el encabezado variable
    // Topic Name
    publish_packet.variable_header[0] = topic_length >> 8; // MSB
    publish_packet.variable_header[1] = topic_length & 0xFF; // LSB
    memcpy(&publish_packet.variable_header[2], encoded_topic, topic_length);

    // Payload (mensaje)
    publish_packet.payload = malloc(message_length);
    memcpy(publish_packet.payload, encoded_message, message_length);

    // Liberar la memoria de los buffers de codificación UTF-8
    free(encoded_topic);
    free(encoded_message);

    return publish_packet;
}

MQTT_Packet create_disconnect_packet() {
    // Asignar memoria para el paquete
    MQTT_Packet disconnect_packet;
    disconnect_packet.variable_header = NULL;
    disconnect_packet.payload = NULL;

    // Rellenar el encabezado fijo y la longitud restante
    disconnect_packet.fixed_header = 0xE0; // DISCONNECT
    disconnect_packet.remaining_length = 0x00;

    return disconnect_packet;
}

void free_packet(MQTT_Packet* packet) {
    if (packet->variable_header != NULL) {
        free(packet->variable_header);
        packet->variable_header = NULL;
    }
    if (packet->payload != NULL) {
        free(packet->payload);
        packet->payload = NULL;
    }
}