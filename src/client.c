#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/encode.h"
#include "../include/decode.h"
#include "../include/packet.h"
#include "../include/utils.h"
#include "../include/send_packets_to_server.h"

#define MAX 360
FILE *log_file = NULL;
char serverIP[MAX];
#define SA struct sockaddr

void *send_packet(void *arg) {
    int sockfd = *((int*)arg);
    printf("\n  ** Send packet **\n");
    while (1) {
        int option;
        printf("Enter the number of your option: \n1. Publish \n2. Subscribe \n3. Disconnect \n");
        scanf("%d", &option);
        
        // Limpia el búfer de entrada
        while (getchar() != '\n');

        if (option == 1) {
            printf("\n  ** Publish **\n");
            char topic[100];
            char message[100];

            printf("Enter the topic: ");
            scanf("%s", topic);
            printf("Enter the message: ");
            scanf("%s", message);

            MQTT_Packet packet = create_publish_packet(topic, encodeMessageToUTF8(message));
            send_publish_to_server(sockfd, packet);
            logger_server("Publish packet sent to server", sockfd);
        } else if (option == 2) {
            printf("\n  ** Subscribe **\n");
            char topic[100];
            printf("Enter the topic to subscribe to (or type 'done' to finish): ");
            scanf("%s", topic);

            const char *topics[4]; // Declaración de un array de 4 punteros a const char
            int count = 0;

            while (strcmp(topic, "done") != 0 && count < 3) {
                topics[count] = strdup(topic); // strdup asigna memoria y copia la cadena
                count++;
                if (count >= 3) break; // Evitar desbordamiento del array
                printf("Enter the next topic (or type 'done' to finish): ");
                scanf("%s", topic);
            }
            topics[count] = NULL; // Agrega un NULL al final del array de topics

            MQTT_Packet packet = create_subscribe_packet(topics);
            send_subscribe_to_server(sockfd, packet);
            logger_server("Subscribe packet sent to server", sockfd);
        } else if (option == 3) {
            printf("\n  ** Disconnect **\n");
            logger_server("Disconnect packet sent to server", sockfd);
            close(sockfd);
            exit(0);
        } else {
            printf("Invalid option, try again.\n");
        }
    }
    return NULL;
}

void *receive_packet(void *arg) {
    int sockfd = *((int*)arg);
    while (1) {
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
            size_t topic_length = (buffer[offset++] << 8) | buffer[offset++];
            char *topic = malloc(topic_length);
            received_packet.variable_header = malloc(4 + topic_length);

            received_packet.variable_header[0] = topic_length >> 8;
            received_packet.variable_header[1] = topic_length & 0xFF;
            
            memcpy(&received_packet.variable_header[2], buffer + offset, topic_length);
            memcpy(topic, buffer + offset, topic_length);
            offset += topic_length;

            received_packet.variable_header[topic_length + 2] = (buffer[offset++] >> 8) & 0xFF;
            received_packet.variable_header[topic_length + 3] = buffer[offset++] & 0xFF;

            payload_length = received_packet.remaining_length - (topic_length + 4);
            received_packet.payload = malloc(payload_length);
            memcpy(received_packet.payload, buffer + offset, payload_length);
            printf("El mensaje %s fue publicado en %s\n", received_packet.payload, topic);
            free(topic);
        } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_CONNACK) {
            received_packet.variable_header = malloc(2);
            received_packet.variable_header[0] = buffer[offset++];
            received_packet.variable_header[1] = buffer[offset];

            if (received_packet.variable_header[1] == 0x00) {
                printf("Connected to MQTT broker\n");
            }
        } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_SUBACK) {
            received_packet.variable_header = malloc(2);
            payload_length = received_packet.remaining_length - 2;

            received_packet.variable_header[0] = buffer[offset++];
            received_packet.variable_header[1] = buffer[offset++];

            received_packet.payload = malloc(payload_length);
            memcpy(received_packet.payload, buffer + offset, payload_length);
        }

        free_packet(&received_packet);
    }
}

int main(int argc, char *argv[]) {
    int sockfd, connfd; 
    struct sockaddr_in servaddr; 
    char log_path[MAX];
    int port;

    if (argc == 4) {
        strcpy(serverIP, argv[1]);
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
    servaddr.sin_addr.s_addr = inet_addr(serverIP);
    servaddr.sin_port = htons(port);

    // Connect ent socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("connected to the server..\n");

    log_file = fopen(log_path, "a");

    MQTT_Packet packet = create_connect_packet(1, encodeMessageToUTF8("J01"), encodeMessageToUTF8("Juan123"), encodeMessageToUTF8("12345678"));
    send_connect_to_server(sockfd, packet);
    logger_server("Connect packet sent to the server", sockfd);

    pthread_t send_tid, receive_tid;
    pthread_create(&receive_tid, NULL, receive_packet, &sockfd);
    pthread_create(&send_tid, NULL, send_packet, &sockfd);

    pthread_join(send_tid, NULL);
    pthread_join(receive_tid, NULL);

    // Close the socket
    fclose(log_file);
    close(sockfd);
  
    return 0;
}
