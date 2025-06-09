#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Carrega de um arquivo os dados sobre postos de gasolina para uma lista encadeada e para arvores KD para cada respectivo tipo de combustivel
DataNode *loadDataFromFile(const char *filename, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return NULL;

    DataNode *head = NULL;
    double x, y;
    int d, a, g;
    while (fscanf(file, "%lf %lf %d %d %d", &x, &y, &d, &a, &g) != EOF)
    {
        head = insertListNode(head, x, y, d, a, g);
        double point[2] = {x * 110.574, y * 111.320 * cos(x * M_PI / 180.0)};
        if (d != -1)
            *dieselTree = insertKdTreeNode(*dieselTree, point, d, 0);
        if (a != -1)
            *alcoholTree = insertKdTreeNode(*alcoholTree, point, a, 0);
        if (g != -1)
            *gasTree = insertKdTreeNode(*gasTree, point, g, 0);
    }
    fclose(file);
    return head;
}

// Atualiza ou insere dados sobre um posto de gasolina na lista encadeada e para arvores KD para cada respectivo tipo de combustivel
DataNode *updateOrInsertData(DataNode *head, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, double x, double y, int fuelType, int price)
{
    DataNode *current = head;
    double point[2] = {x * 110.574, y * 111.320 * cos(x * M_PI / 180.0)};
    while (current)
    {
        // Se encontrar o posto (dado uma margem de erro) apenas atualiza os seus valores
        if (fabs(current->x - x) < 0.0001 && fabs(current->y - y) < 0.0001)
        {
            if (fuelType == 0)
            {
                current->diesel = price;
                *dieselTree = insertKdTreeNode(*dieselTree, point, price, 0);
            }
            else if (fuelType == 1)
            {
                current->alcohol = price;
                *alcoholTree = insertKdTreeNode(*alcoholTree, point, price, 0);
            }
            else if (fuelType == 2)
            {
                current->gas = price;
                *gasTree = insertKdTreeNode(*gasTree, point, price, 0);
            }
            return head;
        }
        current = current->next;
    }
    //Se o posto não estava presente na lista, cria um novo no para ele
    if (fuelType == 0)
    {
        *dieselTree = insertKdTreeNode(*dieselTree, point, price, 0);
    }
    else if (fuelType == 1)
    {
        *alcoholTree = insertKdTreeNode(*alcoholTree, point, price, 0);
    }
    else if (fuelType == 2)
    {
        *gasTree = insertKdTreeNode(*gasTree, point, price, 0);
    }
    return insertListNode(head, x, y, (fuelType == 0) ? price : -1, (fuelType == 1) ? price : -1, (fuelType == 2) ? price : -1);
}

// Escreve os dados dos postos de gasolina da lista encadeada para um arquivo
void saveDataToFile(const char *filename, DataNode *head)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }

    DataNode *current = head;
    while (current)
    {
        fprintf(file, "%lf %lf %d %d %d\n", current->x, current->y, current->diesel, current->alcohol, current->gas);
        current = current->next;
    }
    fclose(file);
}