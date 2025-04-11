/**
  * Ilustra o mapeamento de arquivos para a mem�ria no Win32
 *
  * C�digo do produtor.
 *
 * Figure 9.25
 *
 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013.
 */

#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
HANDLE hFile, hMapFile;
LPVOID mapAddress;

	// primeiro cria/abre o arquivo
	hFile = CreateFile("temp.txt",
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
   		fprintf(stderr,"Could not open file temp.txt (%d).\n",GetLastError());
   		return -1;
	}

	// agora obt�m um mapeamento para ele

	hMapFile = CreateFileMapping(hFile,
									NULL,
									PAGE_READWRITE,
									0,
									0,
									TEXT("SharedObject"));

	if (hMapFile == NULL) {
		fprintf(stderr,"Could not create mapping (%d).\n", GetLastError());
   		return -1;
	}

	// em seguida, estabelece uma visualiza��o mapeada do arquivo

	mapAddress = MapViewOfFile(hMapFile,FILE_MAP_ALL_ACCESS,0,0,0);

	if(mapAddress == NULL) {
		printf("Could not map view of file (%d).\n", GetLastError());
		return -1;
	}

	// grava na mem�ria compartilhada

	sprintf((char *)mapAddress,"%s","Shared memory message");

	while (1);
	// remove o mapeamento do arquivo
	UnmapViewOfFile(mapAddress);

	// fecha todos os handles
	CloseHandle(hMapFile);
	CloseHandle(hFile);
}

