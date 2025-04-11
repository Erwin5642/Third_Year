/**
  * Programa de interface de shell simples.
 *
  * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <unistd.h>



#define MAX_LINE		80 /* 80 caracteres por linha, por comando */

int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* a linha de comando (de 80 caracteres) tem um m�ximo de 40 argumentos */
    int should_run = 1;
	
	int i, upper;
		
    while (should_run){   
        printf("osh>");
        fflush(stdout);
        
        /**
         * Ap�s a leitura da entrada do usu�rio, as etapas s�o:
         * (1) criar um processo filho
         * (2) o processo filho chamar� execvp()
         * (3) se o comando incluir &, o pai chamar� wait()
         */
    }
    
	return 0;
}
