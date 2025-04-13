#include <stdio.h>
#include <time.h>
#include "Dijkstra.h"

#define NUM_NEIGHBORS 200
#define NUM_NODES 100000
#define MAX_WEIGHT 100
#define NEIGHBOR_GAP 3

typedef struct real_data_and_weight {
    int data;
    int weight;
} real_data_and_weight;

real_data_and_weight source = {
    0, 0
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
    if (data == &source.data) return;
    free(weight);
    free(data);
}
void writeIn_neighbor(MinHeap *minheap, HashInTree *hashintree, MinHeapNode *current_node, real_data_and_weight neighbor) {
    MinHeapNode *neighborNode = HashInTree_find_seat(hashintree, &neighbor.data, hashfunc, isEqual);
    // neighbor.weight is really the weight of immediate edge connecting current_node and neighbor
    int new_neighbor_weight = *(int*)(current_node->weight) + neighbor.weight; 

    if (neighborNode == NULL) exit(2);
    if (neighborNode->data == NULL) {
        neighborNode->data = malloc(sizeof(int));
        neighborNode->weight = malloc(sizeof(int));
        *(int*)(neighborNode->data) = neighbor.data;
        *(int*)(neighborNode->weight) = new_neighbor_weight;
        neighborNode->parent = current_node;
        MinHeap_update_key(minheap, neighborNode, true, isBetterThan);
    } else if (isBetterThan(&new_neighbor_weight, neighborNode->weight)) {
        *(int*)(neighborNode->weight) = new_neighbor_weight;
        neighborNode->parent = current_node;
        MinHeap_update_key(minheap, neighborNode, false, isBetterThan);
    }
}
void send_neighbors(MinHeap *minheap, HashInTree *hashintree, MinHeapNode *min_node) {
    for (int i = 1; i <= NUM_NEIGHBORS; i++) {
        real_data_and_weight neighbor = {
            .data = (*(int*)(min_node->data) + i*NEIGHBOR_GAP)%NUM_NODES,
            .weight = (i*NEIGHBOR_GAP)%(MAX_WEIGHT-1) + 1
        };
        writeIn_neighbor(minheap, hashintree, min_node, neighbor);
    }
}

int main(void) {
    HashInTree *solution = Dijkstra_1source_to_all_else(
        &source.data, &source.weight, 
        NUM_NODES, hashfunc, isEqual, isBetterThan, send_neighbors
    );
    for (int i = 0; i < NUM_NODES; i++) {
        MinHeapNode *node = HashInTree_get_node_by_data(solution, &i, hashfunc, isEqual);
        /*
        printf("(%d: %d)", *(int*)(node->data), *(int*)(node->weight));
        while (node->parent) {
            node = node->parent;
            printf(" <- (%d: %d)", *(int*)(node->data), *(int*)(node->weight));
        } printf("\n");
         */
    }
    HashInTree_free(solution, free_data_and_weight);
    return 0;
}