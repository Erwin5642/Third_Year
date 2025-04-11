/**
  * Exemplo de regi�o paralela do OpenMP
 * 
  * Para compilar, insira:
 *
 *	gcc -fopenmp openmp.c
 *
  * Voc� deve ver a mensagem "I am a parallel region" para cada
 * n�cleo de processamento de seu sistema.
 *
  * Para quem estiver usando uma m�quina virtual, certifique-se de configurar o n�mero de
 * n�cleos de processamento com um valor > 1 para ver a execu��o simult�nea da regi�o paralela.
 */

#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	/* c�digo sequencial */

	#pragma omp parallel
	{
		printf("I am a parallel region\n");
	}

	/* c�digo sequencial */

	return 0;
}
