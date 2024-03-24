#include <stdlib.h>
#include <stdio.h>
#include "Packets/packet.c"

int main() {
    // Crear un paquete PUBLISH
    MQTT_Packet publish_packet = create_publish_packet("mi/topico", "Hola, MQTT!");

    // Acceder al encabezado variable del paquete PUBLISH
    u_int8_t topic_length_msb = publish_packet.variable_header[0];
    u_int8_t topic_length_lsb = publish_packet.variable_header[1];

    // Calcular la longitud total del tema
    size_t topic_length = (topic_length_msb << 8) | topic_length_lsb;

    printf("Longitud del tema: %zu\n", topic_length);

    // Imprimir el payload del paquete PUBLISH
    printf("Payload: %s\n", publish_packet.payload);

    // Liberar la memoria utilizada por el paquete PUBLISH
    free_packet(&publish_packet);

    return 0;
}
