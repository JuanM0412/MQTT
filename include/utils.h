#include <arpa/inet.h>
#define MAX 360

extern FILE *log_file;
extern char serverIP[MAX];
extern char clientIP[INET_ADDRSTRLEN];

void logger_server(char *message, int socket);
void logger_client(char *message, int socket);