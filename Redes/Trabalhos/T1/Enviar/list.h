#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct DataNode
{
    double x, y;
    int diesel, alcohol, gas; //-1 para entradas inexistentes
    struct DataNode *next;
} DataNode;

DataNode *insertListNode(DataNode *head, double x, double y, int diesel, int alcohol, int gas);
DataNode *freeList(DataNode *head);

#endif