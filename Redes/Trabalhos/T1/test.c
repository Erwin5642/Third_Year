#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "gasStationProtocol.h"

#define NUM_CLIENTS 10
#define REQUEST_PER_CLIENT 100

void sendRequest(int sockfd, struct sockaddr *serverAddress, RequestMessage *request)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);
    AckNakMessage ackNakResponse;
    static int requestId = 0;
    request->requestId = requestId++;
    while (1)
    {
        request->errorFlag = simulateError();
        if (sendto(sockfd, request, sizeof(RequestMessage), 0, serverAddress, serverAdressLength) < 0)
        {
            logMessage(LOG_WARN, "[CLIENT] Erro ao enviar dados");
        }
        else if (recvfrom(sockfd, &ackNakResponse, sizeof(AckNakMessage), 0, serverAddress, &serverAdressLength) < 0)
        {
            logMessage(LOG_WARN, "[CLIENT] Erro ao receber dados");
        }
        else if (ackNakResponse.requestId == request->requestId)
        {
            if (ackNakResponse.messageType == 'A')
            {
                logMessage(LOG_SUCCESS, "[MSG id=%d] Mensagem recebida pelo server com sucesso", request->requestId);
                return;
            }
            if (ackNakResponse.messageType == 'N')
            {
                logMessage(LOG_WARN, "[MSG id=%d] Mensagem recebida pelo server com erro", request->requestId);
            }
        }
        logMessage(LOG_INFO, "[MSG id=%d] Enviando novamente a mensangem", request->requestId);
    }
    return;
}

void getResponse(int sockfd, struct sockaddr *serverAddress, ResponseMessage *response)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);

    while (recvfrom(sockfd, response, sizeof(ResponseMessage), 0, serverAddress, &serverAdressLength) < 0)
    {
        logMessage(LOG_WARN, "[CLIENT] Erro ao receber dados");
        logMessage(LOG_INFO, "[CLIENT] Esperando novamente uma mensangem");
    }
    return;
}

void printSearchResult(const RequestMessage *request, const ResponseMessage *response)
{
    if (response->minPrice == -1)
    {
        logMessage(LOG_INFO, "[MSG id=%d] Nenhum posto encontrado num raio de %d km a partir de (%lf, %lf) para combustível tipo %d",
                   request->requestId, request->priceOrRadius, request->coordinates[0], request->coordinates[1], request->fuelType);
    }
    else
    {
        logMessage(LOG_INFO, "[MSG id=%d] Melhor posto encontrado para combustível tipo %d num raio de %d km:\n\tPreço: %d\n\tLocalização: (%lf, %lf)",
                   request->requestId, request->fuelType, request->priceOrRadius, response->minPrice, response->coordinates[0], response->coordinates[1]);
    }
}

typedef struct
{
    int sockfd;
    struct sockaddr_in serverAddr;
} TestArgs;

void *threadTest(void *args)
{
    TestArgs test = *(TestArgs *)args;
    RequestMessage request;
    ResponseMessage response;
    struct sockaddr_in serverAddr = test.serverAddr;
    int sockfd = test.sockfd;

    for (int i = 0; i < REQUEST_PER_CLIENT; i++)
    {
        request.messageType = (rand() % 2 == 0) ? 'D' : 'P'; // 'D' de dado ou 'P' de pesquisa
        request.fuelType = rand() % 3;
        request.priceOrRadius = rand() % 100;
        request.coordinates[0] = (rand() % 1000) / 10.0;
        request.coordinates[1] = (rand() % 1000) / 10.0;
        request.errorFlag = simulateError();

        logMessage(LOG_INFO, "[MSG id=%d] Mensagem enviada: %c %d %d %lf %lf",
                   request.requestId, request.messageType, request.fuelType,
                   request.priceOrRadius, request.coordinates[0], request.coordinates[1]);

        sendRequest(sockfd, (struct sockaddr *)&serverAddr, &request);

        if (request.messageType == 'P')
        {
            getResponse(sockfd, (struct sockaddr *)&serverAddr, &response);
            printSearchResult(&request, &response);
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc != 3)
    {
        logMessage(LOG_ERROR, "[USO] Uso: %s <endereco_ip> <porta>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    if (port < 0 || port > 65535)
    {
        logMessage(LOG_ERROR, "[USO] Porta inválida: %d", port);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        logMessage(LOG_ERROR, "[SOCKET] Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    RequestMessage request;
    ResponseMessage response;
    pthread_t threads[NUM_CLIENTS];
    TestArgs args;
    args.sockfd = sockfd;
    args.serverAddr = serverAddr;
    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        if (pthread_create(&(threads[i]), NULL, threadTest, &args) != 0)
        {
            logMessage(LOG_WARN, "[THREAD] Falha ao criar thread para cliente");
        }
    }

    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    close(sockfd);
    return 0;
}
