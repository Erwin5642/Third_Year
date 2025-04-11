#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>

int *numberArray;
void *sumSubarray(void *param);
void *safeAllocate(size_t size);
int totalSum;

struct inibini{
    int begin;
    int end;
};

int main(int argc, char **argv){
    if(argc != 3){
        printf("Numero de argumentos incorreto. Formato correto: <arquivo> <numero de threads>\n");
        exit(EXIT_FAILURE);
    }
    int threadsNumber = atoi(argv[2]);
    if(threadsNumber <= 0){
        printf("O numero de threads deve ser positivo\n");
        exit(EXIT_FAILURE);
    }

    FILE *file;
    if ((file = fopen(argv[1], "rb")) == NULL) {
        printf("Nao foi possivel abrir o arquivo\n");
        exit(EXIT_FAILURE);
    }
    
    struct stat fileStat;
    if (fstat(fileno(file), &fileStat) == -1) {
        printf("Erro ao obter tamanho do arquivo\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t size = fileStat.st_size;
    size_t numNumbers = size / sizeof(int);
    if ((numberArray = (int*) malloc(size)) == NULL) {
        printf("Erro ao alocar memoria\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(numberArray, sizeof(int), numNumbers, file) != numNumbers) {
        printf("Erro ao ler arquivo\n");
        free(numberArray);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);
    
    printf("Somando os numeros do arquivo %s com %d threads\n", argv[1], threadsNumber);
    pthread_t *threads = malloc(threadsNumber * sizeof(pthread_t));
    struct inibini *subArrayBounds = malloc(threadsNumber * sizeof(struct inibini));
    pthread_attr_t threads_attr;
    
    totalSum = 0;
    pthread_attr_init(&threads_attr);
    size_t subArraySize = numNumbers / threadsNumber;
    printf("Size: %ld\n", numNumbers);
    for(int i = 0; i < threadsNumber; i++){
        subArrayBounds[i].begin = i * subArraySize;
        subArrayBounds[i].end = (i + 1) * subArraySize - 1;
        if (i == threadsNumber - 1) { 
            subArrayBounds[i].end += numNumbers % threadsNumber; 
        }
        printf("Bounds: %d - %d\n", subArrayBounds[i].begin, subArrayBounds[i].end);
        pthread_create(&threads[i], &threads_attr, sumSubarray, &subArrayBounds[i]);
    }

    for(int i = 0; i < threadsNumber; i++){
        pthread_join(threads[i], NULL);
    }

    printf("sum = %d\n", totalSum);

    return 0;
}


void *sumSubarray(void *params){
    struct inibini *bounds = params;
    for(int i = bounds->begin; i <= bounds->end; i++){
        totalSum += numberArray[i];
    }
    pthread_exit(0);
}