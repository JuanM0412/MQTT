#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/encode.h"
#include "../include/decode.h"
#include "../include/packet.h"
#include "../include/send_packets_to_server.h"

#define MAX 360
#define SA struct sockaddr

void print_mqtt_packet(MQTT_Packet packet) {
    printf("Handle... \n");
    printf("Fixed Header: 0x%02X\n", packet.fixed_header);
    printf("Remaining Length: %d\n", packet.remaining_length);
    printf("Variable Header: 0x%02X 0x%02X\n", packet.variable_header[0], packet.variable_header[1]);
    printf("Payload:\n");
    
    for (int i = 0; i < packet.remaining_length; ) {
        // Leer longitud del tema
        int topic_length = (packet.payload[i] << 8) | packet.payload[i + 1];
        printf("Topic Length: %d\n", topic_length);

        // Leer el tema
        printf("Topic: ");
        char *topic = malloc(topic_length);
        for (int j = 0; j < topic_length; j++) {
            topic[j] = packet.payload[i + 2 + j];
        }
        printf("Topic: %s\n", topic);
        printf("\n");

        // Mover al siguiente tema
        i += 2 + topic_length;
    }
}

void print_connect_packet(const MQTT_Packet *packet) {
    printf("Fixed Header: 0x%02X\n", packet->fixed_header);
    printf("Remaining Length: %d\n", packet->remaining_length);

    printf("Variable Header: ");
    for (int i = 0; i < packet->remaining_length - 2; i++) { // Resta 2 para los bytes del packet_id
        printf("%c ", packet->variable_header[i]);
    }
    printf("\n");

    printf("Payload: ");
    for (int i = 0; i < packet->remaining_length - 2; i++) { // Resta 2 para los bytes del packet_id
        printf("%c ", packet->payload[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int sockfd, connfd; 
    struct sockaddr_in servaddr; 
    char ip[MAX], log_path[MAX];
    int port;

    if (argc == 4) {
        strcpy(ip, argv[1]);
        port = atoi(argv[2]);
        strcpy(log_path, argv[3]);
    } else
        return 1;

    // Socket creation and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // Assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    // Connect ent socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("connected to the server..\n");

    MQTT_Packet packet = create_connect_packet(1, encodeMessageToUTF8("J01"), encodeMessageToUTF8("Juan123"), encodeMessageToUTF8("12345678"));
    print_connect_packet(&packet);
    // send_connect_to_server(sockfd, packet);
    // MQTT_Packet packet = create_publish_packet(encodeMessageToUTF8("EAFIT/Bloque/18/Salon/301/Temperatura"), encodeMessageToUTF8("29"));
    // send_publish_to_server(sockfd, packet);

    const char *topics[] = {
        encodeMessageToUTF8("EAFIT/Bloque/18/Salon/301/Humedad"),
        encodeMessageToUTF8("EAFIT/Bloque/18/Salon/301/Temperatura"),
        NULL
    };

    // MQTT_Packet packet = create_subscribe_packet(topics);
    // print_mqtt_packet(packet);
    // send_mqtt_packet(sockfd, packet);
    // printf("ENVIADO");

    while (1) {
        printf("receive_packet_from_server\n");
        MQTT_Packet received_packet;
        unsigned char buffer[MAX];

        ssize_t bytes_received = read(sockfd, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            exit(EXIT_FAILURE);
        }

        size_t offset = 0;
        size_t payload_length = 0;

        received_packet.fixed_header = buffer[offset++];
        received_packet.remaining_length = buffer[offset++];
        
        if (received_packet.fixed_header == MQTT_FIXED_HEADER_PUBLISH) {
            printf("MQTT_FIXED_HEADER_PUBLISH\n");
            size_t topic_length = (buffer[offset++] << 8) | buffer[offset++];
            received_packet.variable_header = malloc(4 + topic_length);
            printf("Malloc var\n");

            received_packet.variable_header[0] = topic_length >> 8;
            received_packet.variable_header[1] = topic_length & 0xFF;
            
            memcpy(&received_packet.variable_header[2], buffer + offset, topic_length);
            offset += topic_length;
            printf("Var(1)\n");

            received_packet.variable_header[topic_length + 2] = (buffer[offset++] >> 8) & 0xFF;
            received_packet.variable_header[topic_length + 3] = buffer[offset++] & 0xFF;

            printf("Malloc var(2)\n");
            payload_length = received_packet.remaining_length - (topic_length + 4);
            received_packet.payload = malloc(payload_length);
            printf("Pay(1)\n");
            memcpy(received_packet.payload, buffer + offset, payload_length);
            printf("Pay(2)\n");

            printf("Variable Header: ");
            for (int i = 0; i < topic_length + 4; i++) { // Resta 2 para los bytes del received_packet_id
                printf("%c ", received_packet.variable_header[i]);
            }
            printf("\n");

            printf("Payload: ");
            for (int i = 0; i < payload_length; i++) {
                printf("%c ", received_packet.payload[i]);
            }
        } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_CONNACK) {
            received_packet.variable_header = malloc(2);
            received_packet.variable_header[0] = buffer[offset++];
            received_packet.variable_header[1] = buffer[offset];

            if (received_packet.variable_header[1] == 0x00) {
                printf("Connected to MQTT broker");
            }
        } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_SUBACK) {
            received_packet.variable_header = malloc(2);
            payload_length = received_packet.remaining_length - 2;

            received_packet.variable_header[0] = buffer[offset++];
            printf("received_packet.variable_header[0]: %02X\n", received_packet.variable_header[0]);
            received_packet.variable_header[1] = buffer[offset++];
            printf("received_packet.variable_header[1]: %02X\n", received_packet.variable_header[1]);

            received_packet.payload = malloc(payload_length);
            memcpy(received_packet.payload, buffer + offset, payload_length);
            printf("Payload: ");
            for (int i = 0; i < payload_length; i++) {
                printf("%02X ", received_packet.payload[i]);
            }
        }

        free_packet(&received_packet);
    }

    // Close the socket
    close(sockfd);
  
    free_packet(&packet);
  
    return 0;
}
