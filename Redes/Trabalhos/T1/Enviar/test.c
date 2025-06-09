#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include "gasStationProtocol.h"

#define NUM_CLIENTS 10

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
    int client_id;
} ThreadArgs;

void makeRequest(int sockfd, struct sockaddr *serverAddress, RequestMessage *msg)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);
    ACKNAK response;
    static int idCounter = 0;
    msg->id = __sync_fetch_and_add(&idCounter, 1); // thread-safe increment
    while (1)
    {
        msg->error = simulateError();
        if (sendto(sockfd, msg, sizeof(RequestMessage), 0, serverAddress, serverAdressLength) < 0)
        {
            perror("[W] Erro ao enviar");
            continue;
        }
        if (recvfrom(sockfd, &response, sizeof(ACKNAK), 0, serverAddress, &serverAdressLength) < 0)
        {
            perror("[W] Erro ao receber");
            continue;
        }

        if (response.id == msg->id)
        {
            if (response.messageType == 'A')
            {
                printf("[Client %d] ACK recebido para mensagem %d\n", msg->id, msg->id);
                return;
            }
            if (response.messageType == 'N')
            {
                printf("[Client %d] NAK recebido para mensagem %d. Reenviando...\n", msg->id, msg->id);
            }
        }
    }
}

void *client_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    int sockfd;
    struct sockaddr_in serverAddress;

    srand(time(NULL) ^ args->client_id);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("[E] Erro ao criar socket");
        free(arg);
        return NULL;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(args->port);
    inet_pton(AF_INET, args->ip, &serverAddress.sin_addr);

    RequestMessage msg;
    msg.messageType = (rand() % 2 == 0) ? 'R' : 'P'; // 'R' ou 'P'
    msg.fuelType = rand() % 5;
    msg.price_radius = rand() % 100;
    msg.x = (rand() % 1000) / 10.0;
    msg.y = (rand() % 1000) / 10.0;

    printf("[Client %d] Enviando: %c %d %d %.2f %.2f\n", args->client_id, msg.messageType, msg.fuelType, msg.price_radius, msg.x, msg.y);
    makeRequest(sockfd, (struct sockaddr *)&serverAddress, &msg);

    close(sockfd);
    free(arg);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Uso: %s <ip_do_servidor> <porta>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    pthread_t threads[NUM_CLIENTS];
    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        strncpy(args->ip, ip, INET_ADDRSTRLEN);
        args->port = port;
        args->client_id = i;
        if (pthread_create(&threads[i], NULL, client_thread, args) != 0)
        {
            perror("[E] Erro ao criar thread");
            free(args);
        }
    }

    for (int i = 0; i < NUM_CLIENTS; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
