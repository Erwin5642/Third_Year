/**
  * Exemplo que ilustra os sem�foros n�o nomeados do POSIX
 *
 * Compila��o (no Linux):
 *
 *	gcc -lpthread posix-sem.c
 *
 * Esse exemplo inclui a verifica��o de erros apropriada
 * que n�o � abordada no texto.
 *
 * � bom destacar que o OS X n�o d� suporte a sem�foros n�o 
  * nomeados, logo, esse programa n�o funcionar� nesses sistemas.
 *
  * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * John Wiley & Sons - 2013.
 */

#include <unistd.h>     
#include <sys/types.h>  
#include <errno.h>      
#include <stdio.h>      
#include <stdlib.h>     
#include <pthread.h>    
#include <string.h>     
#include <semaphore.h>  

int main(void)
{
sem_t sem;

    if (sem_init(&sem,0,1) == -1)
              printf("%s\n",strerror(errno));

	if (sem_wait(&sem) != 0)
              printf("%s\n",strerror(errno));

        printf("*** Critical Section *** \n");

	if (sem_post(&sem) != 0)
              printf("%s\n",strerror(errno));

        printf("*** Non-Critical Section *** \n");

	if (sem_destroy(&sem) != 0)
              printf("%s\n",strerror(errno));

        return 0;
}
