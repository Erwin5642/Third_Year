#ifndef KDTREE_H
#define KDTREE_H

typedef struct KdTree {
    double point[2];                 // Coordenadas (x, y)
    int price;                    // Preco associada
    struct KdTree *left;          // Filho da esquerda na arvore Kd
    struct KdTree *right;         // Filho da direita na arvore Kd
    struct KdTree *minPrice;      // Ponteiro com o menor preco na atual subarvore
} KdTree;

// Declaracao de funcoes
KdTree* createKdTreeNode(double xCoord, double yCoord, int nodePrice);
KdTree* insertKdTreeNode(KdTree* root, double point[2], int price, unsigned currentDim);
KdTree* findMinWithinRadius(KdTree* root, double center[2], int radius, unsigned currentDim);
KdTree* freeTree(KdTree* root);

#endif
