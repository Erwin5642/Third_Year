#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<arpa/inet.h>

void funcao_filhos(int num_filhos);
int main(int argc, char **argv)
{
	int nfilhos = 1;
	int pid;
	int x;
	
	if(argc > 1)
	{
		nfilhos = atoi(argv[1]);
	}

	//função para criar n filhos.
	for(x = 0; x < nfilhos; x++)
	{
		if((pid = fork()) == 0)
		{
			funcao_filhos(x + 1);
			exit(0);
		}
	}

	//depois de criar os filhos, o processo pai espera por eles 
	wait(NULL);
	return 0;
}

//Função filho

void funcao_filhos(int num_filhos)
{
	int sock;
	struct sockaddr_in sAddr;
	char buffer[25];

	//socket cliente e bind da porta local
	memset((void *) &sAddr, 0, sizeof(struct sockaddr_in));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;
	sAddr.sin_port = 0;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

	//conectar ao servidor que estiver rodando localmente.
	sAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	sAddr.sin_port = htons(1972);

	if(connect (sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0)
	{
		perror("cliente");
		return;
	}

	//enviar alguma coisa ao servidor.
	snprintf(buffer, 128, "dados do cliente #%i.", num_filhos);
	printf("filho #%i mandou %i caracteres\n", num_filhos, send(sock, buffer, strlen(buffer), 0));
	printf("filho #%i recebeu %i caracteres\n", num_filhos, recv(sock, buffer, 25, 0));

	close(sock);
}