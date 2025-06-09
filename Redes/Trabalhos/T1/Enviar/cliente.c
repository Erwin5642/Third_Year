#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "gasStationProtocol.h"

void makeRequest(int sockfd, struct sockaddr *serverAddress, RequestMessage *msg)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);
    ACKNAK response;
    static int id = 0;
    msg->id = id++;
    while (1)
    {
        msg->error = simulateError();
        if (sendto(sockfd, msg, sizeof(RequestMessage), 0, serverAddress, serverAdressLength) < 0)
        {
            perror("[W] Erro ao enviar");
            printf("[I] Tentando novamente...\n");
            continue;
        }
        if (recvfrom(sockfd, &response, sizeof(ACKNAK), 0, serverAddress, &serverAdressLength) < 0)
        {
            perror("[W] Erro ao receber");
            printf("[I] Tentando novamente...\n");
            continue;
        }

        if (response.id == msg->id)
        {
            if (response.messageType == 'A')
            {
                printf("[I] Mensagem %d recebida pelo server com sucesso\n", msg->id);
                return;
            }
            if (response.messageType == 'N')
            {
                printf("[W] Mensagem %d recebida pelo server com erro\n", msg->id);
                printf("[I] Tentando novamente...\n");
            }
        }
    }
    return;
}

void getResponse(int sockfd, struct sockaddr *serverAddress, ResponseMessage *msg)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);

    while (recvfrom(sockfd, msg, sizeof(ResponseMessage), 0, serverAddress, &serverAdressLength) < 0)
    {
        perror("[W] Erro ao receber");
        printf("[I] Tentando novamente...\n");
    }
    return;
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    if (argc != 3)
    {
        printf("[E] Numero de parametros incorreto\n\tUso: <endereco/nome> <porta>\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    if ((port < 0) || (port > 65535))
    {
        printf("[E] Porta invalida\n");
        exit(EXIT_FAILURE);
    }
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("[E] Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(port);
    serverAddress.sin_family = AF_INET;

    RequestMessage sendBuffer;
    ResponseMessage receiveBuffer;
    int sendMore = 0;
    char inputLine[128];
    while(1){
        printf("Insira a mensagem de requisicao(<tipo_mensagem> <tipo_combustivel> <preco/raio(km)> <x> <y>): ");
        if (fgets(inputLine, sizeof(inputLine), stdin) == NULL)
        {
            perror("[W] Erro ao ler entrada\n");
            continue;
        }

        if (sscanf(inputLine, "%c %d %d %lf %lf", &(sendBuffer.messageType),
                   &(sendBuffer.fuelType),
                   &(sendBuffer.price_radius),
                   &(sendBuffer.x),
                   &(sendBuffer.y)) != 5)
        {
            printf("[W] Entrada invalida. Tente novamente.\n");
            continue;
        }
        printf("[I] Enviando mensagem para o servidor...\n");
        makeRequest(sockfd, (struct sockaddr *)&serverAddress, &sendBuffer);
        printf("[I] Mensagem %d enviada ao servidor: %c %d %d %lf %lf\n", sendBuffer.id, sendBuffer.messageType, sendBuffer.fuelType, sendBuffer.price_radius, sendBuffer.x, sendBuffer.y);
        if (sendBuffer.messageType == 'P')
        {
            getResponse(sockfd, (struct sockaddr *)&serverAddress, &receiveBuffer);
            printf("[I] Resposta da mensagem %d: %d %lf %lf\n", receiveBuffer.id, receiveBuffer.minPrice, receiveBuffer.x, receiveBuffer.y);
            if (receiveBuffer.minPrice == -1)
            {
                printf("[I] Nenhum posto encontrado em um raio de %d (km) a partir de %lf %lf para o combustivel %d\n",
                       sendBuffer.price_radius,
                       sendBuffer.x,
                       sendBuffer.y,
                       sendBuffer.fuelType);
            }
            else
            {
                printf("[I] Melhor posto encontrado em um raio de %d a partir de %lf %lf para o combustivel %d: %d %lf %lf\n",
                       sendBuffer.price_radius,
                       sendBuffer.x,
                       sendBuffer.y,
                       sendBuffer.fuelType,
                       receiveBuffer.minPrice,
                       receiveBuffer.x,
                       receiveBuffer.y);
            }
        }
        printf("\nDeseja fazer mais solicitacoes?\nNao: 0\nSim: 1\n");
        fgets(inputLine, sizeof(inputLine), stdin);
        sscanf(inputLine, "%d", &sendMore);
        if(!sendMore){
            break;
        }
    }

    close(sockfd);
    return 0;
}