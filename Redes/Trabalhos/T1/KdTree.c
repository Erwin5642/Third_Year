#include <stdlib.h>
#include <stdio.h>
#include "KdTree.h"
#include <math.h>

// Calcula o quadrado da distância entre dois pontos
double getSquaredDistance(double pointA[2], double pointB[2]) {
    double dx = pointB[0] - pointA[0];
    double dy = pointB[1] - pointA[1];
    return dx * dx + dy * dy;
}

// Aloca memória para um no da arvore Kd com as coordenadas e preco como parametro
KdTree *createKdTreeNode(double x, double y, int fuelPrice) {
    KdTree *node = malloc(sizeof(KdTree));
    *node = (KdTree){
        .coordinates[0] = x,
        .coordinates[1] = y,
        .fuelPrice = fuelPrice,
        .left = NULL,
        .right = NULL,
        .minPrice = node
    };
    return node;
}

// Libera a memoria ocupada por uma arvore Kd
KdTree *freeTree(KdTree *root) {
    if (!root) return NULL;
    root->left = freeTree(root->left);
    root->right = freeTree(root->right);
    free(root);
    return NULL;
}

// Encontra o no com o menor preco entre tres opcoes
KdTree *getMinPriceNode(KdTree *a, KdTree *b, KdTree *c) {
    KdTree *minNode = NULL;
    if (a) minNode = a;
    if (b && (!minNode || b->fuelPrice < minNode->fuelPrice)) minNode = b;
    if (c && (!minNode || c->fuelPrice < minNode->fuelPrice)) minNode = c;
    return minNode;
}

// Insere um novo no na arvore Kd baseado em coordenadas 2D
KdTree *insertKdTreeNode(KdTree *root, double coordinates[2], int fuelPrice, unsigned currentDim) {
    if (!root) return createKdTreeNode(coordinates[0], coordinates[1], fuelPrice);
    
    if (fabs(root->coordinates[0] - coordinates[0]) < 1e-4 &&
        fabs(root->coordinates[1] - coordinates[1]) < 1e-4) {
        root->fuelPrice = fuelPrice;
        if (fuelPrice <= root->minPrice->fuelPrice) {
            root->minPrice = root;
        }
        return root;
    }
    
    if (root->coordinates[currentDim] > coordinates[currentDim]) {
        root->left = insertKdTreeNode(root->left, coordinates, fuelPrice, (currentDim + 1) % 2);
    } else{
        root->right = insertKdTreeNode(root->right, coordinates, fuelPrice, (currentDim + 1) % 2);
    }

    root->minPrice = getMinPriceNode(
        root->minPrice,
        root->left ? root->left->minPrice : NULL,
        root->right ? root->right->minPrice : NULL);

    return root;
}

// Encontra o na arovre Kd o no com o menor preco dentro de uma regiao ciruclar
KdTree *findMinPriceInRadius(KdTree *root, double center[2], int radius, unsigned currentDim) {
    if (!root) return NULL;

    double radiusSquared = radius * radius;
    double minDistance = getSquaredDistance(root->minPrice->coordinates, center);

    if (minDistance <= radiusSquared) return root->minPrice;


    KdTree *best = NULL;
    double distanceToNode = getSquaredDistance(root->coordinates, center);
    if (distanceToNode <= radiusSquared) {
        best = root;
    }
    
    if (root->coordinates[currentDim] >= center[currentDim] - radius && root->left) {
        struct KdTree *leftBest = findMinPriceInRadius(root->left, center, radius, (currentDim + 1) % 2);
        best = getMinPriceNode(best, leftBest, NULL);
    }

    if (root->coordinates[currentDim] <= center[currentDim] + radius && root->right) {
        struct KdTree *rightBest = findMinPriceInRadius(root->right, center, radius, (currentDim + 1) % 2);
        best = getMinPriceNode(best, rightBest, NULL);
    }

    return best;
}
