#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#include <pthread.h> // Para utilizar hilos (threads)

#define MAX 80 
#define CONFIG_FILE "config.txt" // Nombre del archivo de configuración
#define SA struct sockaddr 

// Función diseñada para el chat entre cliente y servidor. 
void *func(void *arg) 
{ 
    int connfd = *((int*)arg);
	char buff[MAX]; 
	int n; 
	// Bucle infinito para el chat 
	for (;;) { 
		bzero(buff, MAX); 

		// Leer el mensaje del cliente y copiarlo en el buffer 
		read(connfd, buff, sizeof(buff)); 
		// Imprimir el buffer que contiene el contenido del cliente 
		printf("From client: %s\t To client : ", buff); 
		bzero(buff, MAX); 
		n = 0; 
		// Copiar el mensaje del servidor en el buffer 
		while ((buff[n++] = getchar()) != '\n') 
			; 

		// y enviar ese buffer al cliente 
		write(connfd, buff, sizeof(buff)); 

		// Si el mensaje contiene "Exit" entonces el servidor sale y el chat termina. 
		if (strncmp("exit", buff, 4) == 0) { 
			printf("Server Exit...\n"); 
			break; 
		} 
	} 
	close(connfd); // Cerrar la conexión con este cliente
	return NULL;
} 

// Función principal 
int main() 
{ 
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// Leer IP y puerto desde el archivo de configuración
    FILE *config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL)
    {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    char ip[MAX];
    int port;
    if (fscanf(config_file, "%s%d", ip, &port) != 2)
    {
        perror("Error reading config file");
        exit(EXIT_FAILURE);
    }
    fclose(config_file);

	// Crear el socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// Asignar IP y PORT
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(ip); 
	servaddr.sin_port = htons(port); 

	// Enlazar el socket al IP y PORT
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Escuchar las conexiones entrantes
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 

    len = sizeof(cli); 

	// Bucle para aceptar conexiones entrantes
	while (1) {
		// Aceptar el paquete de datos del cliente
		connfd = accept(sockfd, (SA*)&cli, &len); 
		if (connfd < 0) { 
			printf("server accept failed...\n"); 
			exit(0); 
		} 
		else
			printf("server accept the client...\n"); 
		
		pthread_t tid; // Identificador del hilo
		pthread_create(&tid, NULL, func, &connfd); // Crear un hilo para manejar esta conexión
	}

	close(sockfd); // Cerrar el socket principal
	return 0;
}
