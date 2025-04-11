/**
  * Ilustra o mapeamento de arquivos para a mem�ria no Windows 
 *
  * C�digo do consumidor.
 *
 * Figure 9.25
 *
 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013.
 */

#include <stdio.h>
#include <windows.h>

int main(int argc, char *argv[]) {
HANDLE hMapFile;
LPVOID lpMapAddress;

hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,          // permiss�o de leitura/grava��o
                           FALSE,                        // N�o herda o nome
                           TEXT("SharedObject")); // do objeto do mapeamento

if (hMapFile == NULL)
{
   printf("Could not open file mapping object (%d).\n", GetLastError());
   return -1;
}

lpMapAddress = MapViewOfFile(hMapFile,            // handle do objeto do mapeamento
                             FILE_MAP_ALL_ACCESS, // permiss�o de leitura/grava��o
                             0,                  // tamanho m�ximo do objeto
                             0,                   // tamanho de hFile
                             0);                  // mapeia o arquivo inteiro

if (lpMapAddress == NULL)
{
   printf("Could not map view of file (%d).\n", GetLastError());
   return -1;
}

printf("%s\n",lpMapAddress);

UnmapViewOfFile(lpMapAddress);
CloseHandle(hMapFile);

}
