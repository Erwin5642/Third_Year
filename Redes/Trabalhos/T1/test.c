#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include "gasStationProtocol.h"

#define NUM_CLIENTS 1000

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
    int client_id;
} ThreadArgs;

// Função para enviar uma requisição e aguardar ACK/NAK
void makeRequest(int sockfd, struct sockaddr *serverAddress, RequestMessage *msg)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);
    AckNakMessage response;
    static int idCounter = 0;

    msg->requestId = __sync_fetch_and_add(&idCounter, 1); // ID único e thread-safe

    while (1)
    {
        msg->errorFlag = simulateError(); // Recalcular erro a cada tentativa

        if (sendto(sockfd, msg, sizeof(RequestMessage), 0, serverAddress, serverAdressLength) < 0)
        {
            perror("[W] Erro ao enviar");
            continue;
        }

        if (recvfrom(sockfd, &response, sizeof(AckNakMessage), 0, serverAddress, &serverAdressLength) < 0)
        {
            perror("[W] Erro ao receber");
            continue;
        }

        if (response.requestId == msg->requestId)
        {
            if (response.messageType == 'A')
            {
                printf("[Client %d] ACK recebido para mensagem %d\n", msg->requestId, msg->requestId);
                fflush(stdout);
                return;
            }
            if (response.messageType == 'N')
            {
                printf("[Client %d] NAK recebido para mensagem %d. Reenviando...\n", msg->requestId, msg->requestId);
                fflush(stdout);
            }
        }
        else
        {
            printf("[Client %d] Resposta com ID incorreto (esperado %d, recebido %d)\n", msg->requestId, msg->requestId, response.requestId);
            fflush(stdout);
        }
    }
}

// Thread de cliente
void *client_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    int sockfd;
    struct sockaddr_in serverAddress;

    srand(time(NULL) + args->client_id * 1234); // Semente única

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("[E] Erro ao criar socket");
        free(arg);
        return NULL;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(args->port);
    if (inet_pton(AF_INET, args->ip, &serverAddress.sin_addr) <= 0)
    {
        perror("[E] Endereço IP inválido");
        close(sockfd);
        free(arg);
        return NULL;
    }

    // Gerar mensagem aleatória
    RequestMessage msg;
    msg.messageType = (rand() % 2 == 0) ? 'D' : 'P'; // 'D' de dado ou 'P' de pesquisa
    msg.fuelType = rand() % 3;
    msg.priceOrRadius = rand() % 100;
    msg.coordinates[0] = (rand() % 1000) / 10.0;
    msg.coordinates[1] = (rand() % 1000) / 10.0;
    msg.errorFlag = simulateError();

    printf("[Client %d] Enviando: %c %d %d %.2f %.2f (erro=%d)\n",
           args->client_id, msg.messageType, msg.fuelType,
           msg.priceOrRadius, msg.coordinates[0], msg.coordinates[1], msg.errorFlag);
    fflush(stdout);

    makeRequest(sockfd, (struct sockaddr *)&serverAddress, &msg);

    close(sockfd);
    free(arg);
    return NULL;
}

// Programa principal
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Uso: %s <ip_do_servidor> <porta>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    pthread_t threads[NUM_CLIENTS];

    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        if (!args)
        {
            perror("[E] Falha ao alocar memória");
            continue;
        }

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

    return EXIT_SUCCESS;
}
