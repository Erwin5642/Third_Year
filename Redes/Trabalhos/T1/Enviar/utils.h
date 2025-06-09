#ifndef UTILS_H
#define UTILS_H

#include "list.h"
#include "KdTree.h"

DataNode *loadDataFromFile(const char *filename, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree);
DataNode *updateOrInsertData(DataNode *head, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, double x, double y, int fuelType, int price);
void saveDataToFile(const char *filename, DataNode *head);

#endif