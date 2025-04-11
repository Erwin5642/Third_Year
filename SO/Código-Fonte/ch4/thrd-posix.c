/**
  * Um programa pthread que ilustra como
  * criar um thread simples e mostra parte da API pthread
 * Esse programa implementa a fun��o de soma onde
 * a opera��o de adi��o � executada como um thread separado.
 *
  * Para a maioria dos usu�rios do Unix/Linux/OS X
 * gcc thrd.c -lpthread
 *
  * Usu�rios do Solaris devem inserir
 * gcc thrd.c -lpthreads
 *
 * Figure 4.9
 *
 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013
 */

#include <pthread.h>
#include <stdio.h>

int sum; /* esse dado � compartilhado pelo(s) thread(s) */

void *runner(void *param); /* o thread */

int main(int argc, char *argv[])
{
pthread_t tid; /* o identificador do thread */
pthread_attr_t attr; /* configura��o de atributos do thread */
if (argc != 2) {
	fprintf(stderr,"usage: a.out <integer value>\n");
	/*exit(1);*/
	return -1;
}

if (atoi(argv[1]) < 0) {
	fprintf(stderr,"Argument %d must be non-negative\n",atoi(argv[1]));
	/*exit(1);*/
	return -1;
}

/* captura os atributos padr�o */
pthread_attr_init(&attr);

/* cria o thread */
pthread_create(&tid,&attr,runner,argv[1]);

/* agora espera o thread terminar */
pthread_join(tid,NULL);

printf("sum = %d\n",sum);
}

/**
  * O thread assumir� o controle nessa fun��o
 */
void *runner(void *param) 
{
int i, upper = atoi(param);
sum = 0;

	if (upper > 0) {
		for (i = 1; i <= upper; i++)
			sum += i;
	}

	pthread_exit(0);
}
