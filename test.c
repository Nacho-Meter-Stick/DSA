#include <stdio.h>
#include "Dijkstra.h"

typedef struct real_data_and_weight {
    int data;
    int weight;
} real_data_and_weight;

const real_data_and_weight NULL_CASE = {
    -1, -1
};
real_data_and_weight source = {
    0, 0
};

real_data_and_weight Graph[9][3]  = {
    {{1, 3}, {3, 2}, {8, 4}},
    {{0, 3}, {7, 4}, NULL_CASE},
    {{3, 6}, {7, 2}, {5, 1}},
    {{0, 2}, {2, 6}, {4, 1}},
    {{3, 1}, {8, 8}, NULL_CASE},
    {{2, 1}, {6, 8}, NULL_CASE},
    {{5, 8}, NULL_CASE, NULL_CASE},
    {{1, 4}, {2, 2}, NULL_CASE},
    {{0, 4}, {4, 8}, NULL_CASE}
};

const size_t hashfunc(const size_t tableSize, const void *data) {
    return *(int*)data % tableSize;
}
const bool isEqual(const void *data1, const void *data2) {
    return (*(int*)data1 == *(int*)data2);
}
bool isBetterThan(const void *weight1, const void *weight2) {
    return (*(int*)weight1 < *(int*)weight2);
}
void free_data_and_weight(void *data, void *weight) {
    if (data != &source.data) {
        free(weight);
    }
}
void writeIn_neighbor(MinHeap *minheap, HashInTree *hashintree, MinHeapNode *current_node, real_data_and_weight *neighbor) {
    MinHeapNode *canvas = HashInTree_find_seat(hashintree, &(neighbor->data), hashfunc, isEqual);
    int new_neighbor_weight = *(int*)(current_node->weight) + neighbor->weight;

    if (canvas == NULL) exit(2);
    if (canvas->data == NULL) {
        canvas->data = &neighbor->data;
        canvas->weight = (void *)malloc(sizeof(int));

        *(int*)(canvas->weight) = new_neighbor_weight;
        canvas->parent = current_node;
        MinHeap_update_key(minheap, canvas, true, isBetterThan);
    } else if (isBetterThan(&new_neighbor_weight, canvas->weight)) {
        *(int*)(canvas->weight) = new_neighbor_weight;
        canvas->parent = current_node;
        MinHeap_update_key(minheap, canvas, false, isBetterThan);
    }
}
void send_neighbors(MinHeap *minheap, HashInTree *hashintree, MinHeapNode *min_node) {
    for (int i = 0; i < 3; i++) {
        real_data_and_weight *neighbor = &(Graph[*(int*)(min_node->data)][i]);
        if (neighbor->data == NULL_CASE.data) return;
        writeIn_neighbor(minheap, hashintree, min_node, neighbor);
    }
}
int main(void) {
    HashInTree *solution = Dijkstra_1source_to_all_else(
        &source.data, &source.weight, 
        9, hashfunc, isEqual, isBetterThan, send_neighbors
    );
    for (int i = 0; i < 9; i++) {
        MinHeapNode *node = HashInTree_get_node_by_data(solution, &i, hashfunc, isEqual);
        printf("(%d: %d)", *(int*)(node->data), *(int*)(node->weight));
        while (node->parent) {
            node = node->parent;
            printf(" <- (%d: %d)", *(int*)(node->data), *(int*)(node->weight));
        } printf("\n");
    }
    HashInTree_free(solution, free_data_and_weight);
    return 0;
}