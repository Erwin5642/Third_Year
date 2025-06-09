#include <stdlib.h>
#include <stdio.h>
#include "KdTree.h"
#include <math.h>

// Calcula o quadrado da distância entre dois pontos
double squaredDistance(double pointA[2], double pointB[2]) {
    return (pointB[0] - pointA[0]) * (pointB[0] - pointA[0]) + (pointB[1] - pointA[1]) * (pointB[1] - pointA[1]);
}

// Aloca memória para um no da arvore Kd com as coordenadas e preco no parametro
struct KdTree *createKdTreeNode(double  xCoord, double yCoord, int nodePrice) {
    struct KdTree *newNode = malloc(sizeof(struct KdTree));
    newNode->point[0] = xCoord;
    newNode->point[1] = yCoord;
    newNode->price = nodePrice;
    newNode->left = newNode->right = NULL;
    newNode->minPrice = newNode;
    return newNode;
}

// Libera a memoria ocupada por uma arvore Kd
struct KdTree *freeTree(struct KdTree *root) {
    if (!root) return NULL;
    root->left = freeTree(root->left);
    root->right = freeTree(root->right);
    free(root);
    return NULL;
}

// Encontra o no com o menor preco entre tres opcoes
struct KdTree *minPriceNode(struct KdTree *nodeA, struct KdTree *nodeB, struct KdTree *nodeC) {
    struct KdTree *minNode = NULL;
    if (nodeA) minNode = nodeA;
    if (nodeB && (!minNode || nodeB->price < minNode->price)) minNode = nodeB;
    if (nodeC && (!minNode || nodeC->price < minNode->price)) minNode = nodeC;
    return minNode;
}

// Insere um novo no na arvore Kd
struct KdTree *insertKdTreeNode(struct KdTree *root, double point[2], int price, unsigned currentDim) {
    if (!root) {
        return createKdTreeNode(point[0], point[1], price);
    }
    if(fabs(root->point[0] - point[0]) < 0.0001 && fabs(root->point[1] - point[1]) < 0.0001){
        root->price = price;
        if(root->price <= root->minPrice->price){
            root->minPrice = root;
        }
        return root;
    }
    else if (root->point[currentDim] > point[currentDim]) {
        root->left = insertKdTreeNode(root->left, point, price, (currentDim + 1) % 2);
    } else{
        root->right = insertKdTreeNode(root->right, point, price, (currentDim + 1) % 2);
    }

    root->minPrice = minPriceNode(
        root->minPrice,
        root->left ? root->left->minPrice : NULL,
        root->right ? root->right->minPrice : NULL
    );

    return root;
}

// Encontra o na arovre Kd o no com o menor preco dentro de uma regiao ciruclar
struct KdTree *findMinWithinRadius(struct KdTree *root, double center[2], int radius, unsigned currentDim) {
    if (!root) return NULL;

    double  minDistance = squaredDistance(root->minPrice->point, center);

    if (minDistance <= radius * radius) {
        return root->minPrice;
    }

    struct KdTree *leftMin = NULL, *rightMin = NULL, *midMin = NULL;
    double  distance = squaredDistance(root->point, center);

    if (distance <= radius * radius) {
        midMin = root;
    }

    if (root->point[currentDim] >= center[currentDim] - radius && root->left) {
        leftMin = findMinWithinRadius(root->left, center, radius, (currentDim + 1) % 2);
    }

    if (root->point[currentDim] <= center[currentDim] + radius && root->right) {
        rightMin = findMinWithinRadius(root->right, center, radius, (currentDim + 1) % 2);
    }

    return minPriceNode(midMin, leftMin, rightMin);
}
