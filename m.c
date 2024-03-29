#include "encode/Encode.c"
#include "encode/Decode.c"
#include "Packets/packet.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Debes definir estas constantes en algún lugar
#define MQTT_FIXED_HEADER_PUBLISH 0x30

int main() {
    MQTT_Packet publish_packet = create_publish_packet("EAFIT/Sede/Poblado/Bloque/33/Salon/301/humedad", "33%");

    // Concatenar encabezado variable y payload en un solo búfer
    size_t total_length = publish_packet.remaining_length + strlen(publish_packet.payload);
    unsigned char *serialized_packet = malloc(total_length);
    memcpy(serialized_packet, publish_packet.variable_header, publish_packet.remaining_length);
    memcpy(serialized_packet + publish_packet.remaining_length, publish_packet.payload, strlen(publish_packet.payload));

    // Imprimir el paquete serializado
    printf("Serialized packet: ");
    for (int i = 0; i < total_length; ++i) {
        printf("%02X ", serialized_packet[i]);
    }
    printf("\n");

    // Decodificar el tópico codificado en UTF-8
    unsigned int topic_length = (publish_packet.variable_header[0] << 8) | publish_packet.variable_header[1];
    char *decoded_topic = decodeUTF8(serialized_packet + 2); // Ignoremos el encabezado variable
    printf("Decoded topic: %s\n", decoded_topic);

    // Decodificar el mensaje codificado en UTF-8 (ignorando el encabezado variable)
    char *decoded_message = decodeUTF8(serialized_packet + publish_packet.remaining_length); // Ignoremos el encabezado variable
    printf("Decoded message: %s\n", decoded_message);

    // Liberar la memoria utilizada por el paquete PUBLISH y los mensajes decodificados
    free_packet(&publish_packet);
    free(serialized_packet);
    free(decoded_topic);
    free(decoded_message);

    return 0;
}