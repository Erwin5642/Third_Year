#include "list.h"

DataNode *insertListNode(DataNode *head, double x, double y, int diesel, int alcohol, int gas)
{
    DataNode *newData = malloc(sizeof(DataNode));
    newData->x = x;
    newData->y = y;
    newData->diesel = diesel;
    newData->alcohol = alcohol;
    newData->gas = gas;
    newData->next = head;
    return newData;
}

DataNode *freeList(DataNode *head)
{
    DataNode *aux;
    while (head)
    {
        aux = head->next;
        free(head);
        head = aux;
    }
    return NULL;
}
