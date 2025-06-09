#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "KdTree.h"
#include "list.h"
#include "utils.h"
#include "gasStationProtocol.h"

#define AUTO_SAVE_INTERVAL 15

static pthread_mutex_t listLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t dieselTreeLock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t alcoholTreeLock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t gasTreeLock = PTHREAD_RWLOCK_INITIALIZER;

pthread_rwlock_t *getTreeLockByFuelType(int fuelType)
{
    switch (fuelType)
    {
    case 0:
        return &dieselTreeLock;
    case 1:
        return &alcoholTreeLock;
    case 2:
        return &gasTreeLock;
    default:
        return NULL;
    }
}

void lockTreeForRead(int fuelType)
{
    pthread_rwlock_t *lock = getTreeLockByFuelType(fuelType);
    if (lock)
        pthread_rwlock_rdlock(lock);
}

void lockTreeForWrite(int fuelType)
{
    pthread_rwlock_t *lock = getTreeLockByFuelType(fuelType);
    if (lock)
        pthread_rwlock_wrlock(lock);
}

void unlockTree(int fuelType)
{
    pthread_rwlock_t *lock = getTreeLockByFuelType(fuelType);
    if (lock)
        pthread_rwlock_unlock(lock);
}

// Automaticamente salva os dados no arquivo de tempos em tempos
void *autoSaveLoop(void *arg)
{
    DataNode **stationsList = (DataNode **)arg;
    while (1)
    {
        sleep(AUTO_SAVE_INTERVAL);
        pthread_mutex_lock(&listLock);
        saveDataToFile("data.txt", *stationsList);
        pthread_mutex_unlock(&listLock);
        printf("[I] [AutoSave] Dados salvos automaticamente.\n");
    }
    return NULL;
}

void getRequest(int sockfd, struct sockaddr *clientAddress, RequestMessage *msg)
{
    socklen_t clientAddressLength = sizeof(*clientAddress);
    ACKNAK response;

    while (1)
    {
        if (recvfrom(sockfd, msg, sizeof(RequestMessage), 0, clientAddress, &clientAddressLength) < 0)
        {
            perror("[W] Erro ao receber");
            printf("[I] Tentando novamente...\n");
            continue;
        }
        response.id = msg->id;
        if (msg->error)
        {
            printf("[W] Mensagem corrompida\n");
            printf("[I] Enviando NAK...\n");
            response.messageType = 'N';
            sendto(sockfd, &response, sizeof(ACKNAK), 0, clientAddress, clientAddressLength);
            continue;
        }
        printf("[i] Mensagem recebida\n");
        printf("[I] Enviando ACK...\n");
        response.messageType = 'A';
        sendto(sockfd, &response, sizeof(ACKNAK), 0, clientAddress, clientAddressLength);
        return;
    }
    return;
}

void processDataRequest(RequestMessage *msg, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, DataNode **stationsDataList)
{
    pthread_mutex_lock(&listLock);
    lockTreeForWrite(msg->fuelType);
    *stationsDataList = updateOrInsertData(*stationsDataList, dieselTree, alcoholTree, gasTree, msg->x, msg->y, msg->fuelType, msg->price_radius);
    unlockTree(msg->fuelType);
    pthread_mutex_unlock(&listLock);
    printf("[I] Dados armazenados com sucesso.\n");
    return;
}

void processSearchRequest(RequestMessage *msg, ResponseMessage *response, KdTree *dieselTree, KdTree *alcoholTree, KdTree *gasTree)
{
    KdTree *bestStation = NULL;
    double center[2] = {msg->x * 110.574, msg->y * 111.320 * cos(msg->x * M_PI / 180.0)};
    int radius = msg->price_radius;
    response->id = msg->id;
    if (msg->fuelType >= 0 && msg->fuelType <= 2)
    {
        lockTreeForRead(msg->fuelType);
        switch (msg->fuelType)
        {
        case 0:
            bestStation = findMinWithinRadius(dieselTree, center, radius, 0);
            break;
        case 1:
            bestStation = findMinWithinRadius(alcoholTree, center, radius, 0);
            break;
        case 2:
            bestStation = findMinWithinRadius(gasTree, center, radius, 0);
            break;
        default:
            printf("[W] Tipo de combustivel indisponivel\n");
            break;
        }
        unlockTree(msg->fuelType);
    }
    if (!bestStation)
    {
        printf("[I] Nenhum posto encontrado em um raio de %d (km) a partir de %lf %lf para o combustivel %d\n",
               msg->price_radius,
               msg->x,
               msg->y,
               msg->fuelType);
        response->minPrice = -1;
        response->x = 0;
        response->y = 0;
        return;
    }
    response->minPrice = bestStation->price;
    response->x = bestStation->point[0] / 110.574;
    response->y = bestStation->point[1] / (111.320 * cos(msg->x * M_PI / 180.0));
    return;
}

void processRequest(RequestMessage msg, ResponseMessage *response, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, DataNode **stationsDataList)
{
    if (msg.messageType == 'D')
    {
        processDataRequest(&msg, dieselTree, alcoholTree, gasTree, stationsDataList);
    }
    else if (msg.messageType == 'P')
    {
        processSearchRequest(&msg, response, *dieselTree, *alcoholTree, *gasTree);
    }
    return;
}

void sendResponse(int sockfd, struct sockaddr *clientAddress, ResponseMessage *msg)
{
    socklen_t len = sizeof(*clientAddress);
    sendto(sockfd, msg, sizeof(ResponseMessage), 0, clientAddress, len);
    printf("[I] Resposta enviada para o cliente\n");
}

typedef struct{
    int sockfd;
    struct sockaddr_in clientAddress;
    RequestMessage msg;
    ResponseMessage response;
    KdTree **dieselTree, **alcoholTree, **gasTree;
    DataNode **stationsDataList;
}DispatchArgs;

void *dispatchRequest(void *args){
    DispatchArgs *data = (DispatchArgs *)args;
    processRequest(data->msg, &(data->response), data->dieselTree, data->alcoholTree, data->gasTree, data->stationsDataList);
    if ((data->msg).messageType == 'P')    
    {
        sendResponse(data->sockfd, (struct sockaddr *)&(data->clientAddress), &(data->response));
    }
    free(args);
    pthread_exit(0);
}


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Erro: Numero de parametros incorreto\n\tUso: <porta>\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    if ((port < 0) || (port > 65535))
    {
        printf("Erro: Porta invalida\n");
        exit(EXIT_FAILURE);
    }

    // ############# Inicialização da base de dados #################

    // Arvores KD para cada categoria de combustivel
    KdTree *dieselTree = NULL, *alcoholTree = NULL, *gasTree = NULL;
    DataNode *stationsDataList;
    stationsDataList = loadDataFromFile("data.txt", &dieselTree, &alcoholTree, &gasTree);
    printf("[I] Base de dados inicializada...\n");
    // ########## Inicialização do servidor ##########

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    socklen_t serverAdrressLength;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAdrressLength = sizeof(serverAddress);

    if (bind(sockfd, (struct sockaddr *)&serverAddress, serverAdrressLength) < 0)
    {
        perror("Erro ao fazer bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("[I] Server estabelecido...\n");
    struct sockaddr_in clientAddress;
    RequestMessage receiveBuffer;

    pthread_t autoSaver;
    if (pthread_create(&autoSaver, NULL, autoSaveLoop, &stationsDataList) != 0)
    {
        perror("[E] Erro ao criar thread de salvamento automático");
        exit(EXIT_FAILURE);
    }

    pthread_detach(autoSaver);

    pthread_t worker;
    while (1)
    {
        DispatchArgs *args = malloc(sizeof(DispatchArgs));
        if(!args){
            perror("[W] Erro ao alocar argumentos para threads");
            continue;
        }
        printf("[I] Esperando solicitacao...\n");
        getRequest(sockfd, (struct sockaddr *)&clientAddress, &receiveBuffer);
        args->sockfd = sockfd;
        args->clientAddress = clientAddress;
        args->alcoholTree = &alcoholTree;
        args->dieselTree = &dieselTree;
        args->gasTree = &gasTree;
        args->stationsDataList = &stationsDataList;
        args->msg = receiveBuffer;
        
        if (pthread_create(&worker, NULL, dispatchRequest, args) != 0) {
            perror("pthread_create");
            free(args);
            continue;
        }   
        pthread_detach(worker);
    }

    // ########## Liberando recursos do servidor ##########

    stationsDataList = freeList(stationsDataList);
    close(sockfd);
    dieselTree = freeTree(dieselTree);
    alcoholTree = freeTree(alcoholTree);
    gasTree = freeTree(gasTree);

    return 0;
}
