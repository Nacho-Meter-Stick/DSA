/**
 * In this file, the user may be referred to as 'you'. 
 * The creator of the file, and the functions in the file may be refered to as 'we', 'us', 'our', etc.
 * Hopefully you get it, and if you don't, you will.
 * 
 * ---------- ALGORITHM ------------
 *  This file implements Dijkstra's algorithm with modifications.
 *  We use an array-based Min-heap as a priority queue, and a hashmap to store the indexes of nodes that are in the Min-heap.
 *  Use of memory-sense reveals that the swapping operations done by the minheap are faster done on pointers than actual nodes.
 *  Then we move the actual nodes to be stored in the hashmap, and every swap operation done by the minheap 
 *   must also change the index member variable of the node struct.
 *  This means when Dijkstra needs the minimum element, the minheap can give it a pointer, 
 *   and Dijkstra can drop that pointer without worry of losing nodes.
 *  Futhermore, all nodes that end up in the hashmap stay in the hashmap. Note that every node still has a parent-pointer.
 *  The implications of this lazy tactic include that after Dijkstra is finished, the hashmap's nodes will have formed a parent-pointer tree / in-tree,
 *   that completely represents the distance to all nodes along with the paths to all nodes.
 *   This is the ultimate solution, and it being accessible via hashmap makes it nigh impossible for any other format to beat.
 *  Thus, this "HashInTree" is what Dijkstra_1source_to_all_else() returns.
 * 
 * -------------- API --------------
 * We store void pointers to data values and weight values. 
 * The types underneath these void pointers are user-defined.
 * Dijkstra.h will have no knowledge of these types.
 * 
 * Any processing of these types are done through user-defined functions, namely:
 *   - const size_t (*hashfunc)(const size_t tableSize, const void *data)
 *     - The user knows best how to hash their own data.
 *     - hashfunc must return an index less than tableSize, and tableSize is likely not a prime number.
 *     - the hashfunction need not be perfect. If there are collisions, we will execute linear probing.
 *   - const bool (*isEqual)(const void *data1, const void *data2)
 *   - bool (*isBetterThan)(const void *weight1, const void *weight2)
 *     - To clarify, weights are costs that add and must be minimized. 
 *        There is probably a way to spoof what the weights mean to be multiplicative, 
 *         or to use 2 different metrics that increase in different ways. 
 *        As long as it forms a total ordering, and the cost of a path strictly increases as more nodes are appended, 
 *         and the increase is associative, Dijkstra should work just fine.
 *        And this is generally why the file gives so much freedom in what the weights even are.
 *     - isBetterThan() should return true if weight1 is more favorable than weight2.
 *   - void (*free_data_and_weight)(Data_and_weight data_and_weight)
 *     - This is a function that only HashInTree_free() needs.
 *       We will not tell you to free a NULL case or anything of the sort.
 *
 * 
 * Now here comes the interesting part of the whole API.
 *  You may have a graph that is very big. 
 *  Maybe the connectivity of this graph is in the big numbers.
 *  Maybe you don't even have the full graph in memory, but you can 
 *   derive neighbors of nodes on the fly and how far away they are from said node.
 *  Unlike many Dijkstra libraries, we don't actually need the whole graph as input.
 *   Yes, we sort of make our own, but the connectivity of the in-tree is always 1, so we store far less pointers than the graph would have.
 *  Instead we will give you a node that our minheap determined was the minimum element unsettled.
 *  We are refering to the function you pass in as:
 *   void (*send_neighbors)(MinHeap *minheap, HashInTree *hashintree, MinHeapNode *min_node).
 *  Then, using this node, you are to give us the neighbors and their computed weights based on said node.
 *  However, if you don't know whether the neighbor you give us is new to us, 
 *   or whether said neighbor already has a more optimal path to it, 
 *   you would be needlessly doing a lot of mallocs, mostly for redundant nodes.
 *  So, send_neighbors() does more.
 *  Send_neighbors() will call HashInTree_find_seat() on void *data of the neighbor node. This void* data need not be malloced yet.
 *  HashInTree_find_seat() will then use the hashfunc and linear probing to find, either that same neighbor, 
 *   or the first open spot to put such a neighbor node in. It will return a pointer to this node, whichever it finds.
 *   If it returns an open spot, you will know because the data pointer in that node is NULL.
 *  The implementation also returns NULL if the HashInTree is full, but that should not happen as long as total_number_of_nodes was correct.
 *  send_neighbors() then is responsible for writing in the data, weight, and parent-pointer of the neighbor into the node if the neighbor is new,
 *   and updating that information if the neighbor was to get a more optimal weight this time around.
 *  If it happens that you are mallocing these pointers in any of the cases, you will of course be responsible for freeing them.
 *  
 *  send_neighbors() will lastly call upon MinHeap_update_key() to update the minheap with the neighbor MinHeapNode* if the neighbor is new or has gotten better.
 *  send_neighbors() must repeat this process for all neighbors that it finds of the current minimum node.
 * 
 * 
 * With everything explained above, Dijkstra will work just fine. 
 * With the HashInTree that Dijkstra returns, 
 *  you will also be able to index any node by (void *data) name, using HashInTree_get_node_by_data().
 *  HashInTree_get_node_by_data() will return NULL if it cannot find the node you speak of. (Maybe your graph isn't 1 connected component)
 * This node has its distance from the source node as (void *weight).
 * You may then de-reference up the in-tree for the path from the source node in reverse order.
 * Of course, many paths will be converging at nodes closer to the source, so it is best not to modify the HashInTree.
 * Lastly, you are responsible for calling HashInTree_free() when you are done with it.
 * 
 * This large amount of tight cooperation between our functions is to provide you with as much control as possible, 
 * while avoiding the dangers of implementing co-routines in C.
 * This file places trust in the user because the user is programming in C in the first place.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

///////////////////////////// FOR THE USER TO INTERFACE WITH ////////////////////////////
typedef struct MinHeapNode {
    void *data;
    void *weight;
    struct MinHeapNode* parent;
    size_t heapIndex;
} MinHeapNode; 
typedef struct HashInTree {
    size_t length;
    size_t size;
    MinHeapNode *nodes;
} HashInTree;
typedef struct MinHeap {
    MinHeapNode** heap; // Array of pointers to MinHeapNodes stored in the HashInTree.
    size_t size;
    size_t capacity;
} MinHeap;

MinHeapNode *HashInTree_find_seat(
    const HashInTree *hashintree,
    const void *data,
    const size_t (*hashfunc)(const size_t tableSize, const void *data),
    const bool (*isEqual)(const void *data1, const void *data2)
);
MinHeapNode *HashInTree_get_node_by_data(
    const HashInTree *map, 
    const void *data, 
    const size_t (*hashfunc)(const size_t tableSize, const void *data),
    const bool (*isEqual)(const void *data1, const void *data2)
);
void HashInTree_free(
    HashInTree *map, 
    void (*free_data_and_weight)(void *data, void *weight)
);
void MinHeap_update_key(
    MinHeap* minheap, 
    MinHeapNode *node,
    const bool NodeIsNew,
    bool (*isBetterThan)(const void *weight1, const void *weight2)
);
HashInTree *Dijkstra_1source_to_all_else(
    void *sourceData,
    void *sourceWeight,
    const size_t total_number_of_nodes,
    const size_t (*hashfunc)(const size_t tableSize, const void *data),
    const bool (*isEqual)(const void *data1, const void *data2),
    bool (*isBetterThan)(const void *weight1, const void *weight2),
    void (*send_neighbors)(MinHeap *minheap, HashInTree *hashintree, MinHeapNode *min_node)
);
///////////////////////////////////// INTERNALS //////////////////////////////////////
/*
 USER--------------------------------------------------------------
  |  |                              |                             |
  |  Dijkstra_1source_to_all_else() HashInTree_get_node_by_data() HashInTree_free()
  |   |-> MinHeap_create()                                |              |
  |   |-> HashInTree_create()                             |              |
  |   |                                                   |              |
  |-->|-> HashInTree_find_seat()---------|-> hashfunc() <-|              |
  \-->|-> MinHeap_update_key()           \-> isEqual()  <-/              |
      |    \-> MinHeap_bubble_up()                                       |
      |                    >---------------> isBetterThan()              |
      |-> MinHeap_pluck_min()                                            |
      |------------------------------------> send_neighbors()            |
      |                                                                  /
      \-> MinHeap_free()                     free_data_and_weight() <---/
      
*/

HashInTree *HashInTree_create(const size_t size);
MinHeap* MinHeap_create(const size_t size);
void MinHeap_bubble_up(
    MinHeap* minheap, 
    size_t curr_index, 
    bool (*isBetterThan)(const void *weight1, const void *weight2)
);
MinHeapNode* MinHeap_pluck_min(
    MinHeap* minheap,
    bool (*isBetterThan)(const void *weight1, const void *weight2)
);
void MinHeap_free(MinHeap* minheap);




HashInTree *HashInTree_create(const size_t size){
    HashInTree* hashintree = (HashInTree*)malloc(sizeof(HashInTree));
    hashintree->length = 0;
    hashintree->size = size;

    hashintree->nodes = (MinHeapNode*)malloc(size*sizeof(MinHeapNode));
    for (size_t i = 0; i < size; i++) {
        hashintree->nodes[i].data = NULL;
    } return hashintree;
}
MinHeapNode *HashInTree_find_seat(
    const HashInTree *hashintree,
    const void *data,
    const size_t (*hashfunc)(const size_t tableSize, const void *data),
    const bool (*isEqual)(const void *data1, const void *data2)
) {
    size_t hash = hashfunc(hashintree->size, data);
    size_t index = hash;
    do {
        if (hashintree->nodes[index].data == NULL || isEqual(data, hashintree->nodes[index].data)) {
            return &(hashintree->nodes[index]);
        }
    } while ((index = (index+1)%(hashintree->size)) != hash);
    return NULL;
}
MinHeapNode *HashInTree_get_node_by_data(
    const HashInTree *hashintree, 
    const void *data, 
    const size_t (*hashfunc)(const size_t tableSize, const void *data),
    const bool (*isEqual)(const void *data1, const void *data2)
) {
    size_t hash = hashfunc(hashintree->size, data);
    size_t index = hash;
    do {
        if (hashintree->nodes[index].data == NULL) {
            return NULL;
        } else if (isEqual(data, hashintree->nodes[index].data)) {
            return &(hashintree->nodes[index]);
        }
    } while ((index = (index == hashintree->size-1) ? 0 : index + 1) != hash);
    return NULL;
}

void HashInTree_free(
    HashInTree *hashintree, 
    void (*free_data_and_weight)(void *data, void *weight)
) {
    if (hashintree == NULL || hashintree->nodes == NULL) {
        free(hashintree);
        return;
    };

    for (size_t i = 0; i < hashintree->size; i++) {
        if (hashintree->nodes[i].data == NULL) continue;
        free_data_and_weight(
            hashintree->nodes[i].data,
            hashintree->nodes[i].weight
        );
    }
    free(hashintree->nodes);
    free(hashintree);
}

MinHeap* MinHeap_create(const size_t size) {
    MinHeap* minheap = (MinHeap*)malloc(sizeof(MinHeap));
    minheap->heap = (MinHeapNode**)calloc(size, sizeof(MinHeapNode*));
    minheap->size = 0;
    minheap->capacity = size;
    return minheap;
}
static inline size_t parent_index(const size_t i) {
    return ((i - 1) >> 1);
}
static inline size_t left_child_index(const size_t i) {
    return ((2 * i) + 1);
}
static inline size_t right_child_index(const size_t i) {
    return (2 * (i + 1));
}
void MinHeap_bubble_up(
    MinHeap* minheap, 
    size_t curr_index, 
    bool (*isBetterThan)(const void *weight1, const void *weight2)
) {
    MinHeapNode* this_node = minheap->heap[curr_index];
    while (curr_index > 0) {
        size_t parent_ind = parent_index(curr_index);
        if (isBetterThan(this_node->weight, minheap->heap[parent_ind]->weight)) {
            minheap->heap[parent_ind]->heapIndex = curr_index;
            minheap->heap[curr_index] = minheap->heap[parent_ind];
            curr_index = parent_ind;
        } else break;
    }
    this_node->heapIndex = curr_index;
    minheap->heap[curr_index] = this_node;
}
MinHeapNode* MinHeap_pluck_min(
    MinHeap* minheap,
    bool (*isBetterThan)(const void *weight1, const void *weight2)
) {
    if (minheap->size == 0) return NULL; 
    // save minnode
    MinHeapNode* minnode = minheap->heap[0];
    // move last into root
    MinHeapNode* curr_node = minheap->heap[--(minheap->size)];
    minheap->heap[minheap->size] = NULL;
    if (minheap->size == 0) return minnode;

    // bubble down
    size_t curr_index = 0;
    size_t left_child_ind = left_child_index(curr_index);
    size_t right_child_ind = right_child_index(curr_index);
    while (left_child_ind < minheap->size) {
        size_t smaller_child_ind = left_child_ind;
        if (right_child_ind < minheap->size && isBetterThan(minheap->heap[right_child_ind]->weight, minheap->heap[left_child_ind]->weight)) {
            smaller_child_ind = right_child_ind;
        }
        if (isBetterThan(minheap->heap[smaller_child_ind]->weight, curr_node->weight)) {
            minheap->heap[smaller_child_ind]->heapIndex = curr_index;
            minheap->heap[curr_index] = minheap->heap[smaller_child_ind];
            curr_index = smaller_child_ind;
            left_child_ind = left_child_index(curr_index);
            right_child_ind = right_child_index(curr_index);
        } else break;
    }
    minheap->heap[curr_index] = curr_node;
    curr_node->heapIndex = curr_index;
    return minnode;
}
void MinHeap_update_key(
    MinHeap* minheap, 
    MinHeapNode *node,
    const bool NodeIsNew,
    bool (*isBetterThan)(const void *weight1, const void *weight2)
) {
    if (NodeIsNew) { // If node was new, we need to insert
        if (minheap->size == minheap->capacity) exit(2);
        minheap->heap[minheap->size++] = node;
    }
    MinHeap_bubble_up(minheap, node->heapIndex, isBetterThan);
}
void MinHeap_free(MinHeap* minheap) {
    if (minheap != NULL) free(minheap->heap);
    free(minheap);
}

HashInTree *Dijkstra_1source_to_all_else(
    void *sourceData,
    void *sourceWeight,
    const size_t total_number_of_nodes,
    const size_t (*hashfunc)(const size_t tableSize, const void *data),
    const bool (*isEqual)(const void *data1, const void *data2),
    bool (*isBetterThan)(const void *weight1, const void *weight2),
    void (*send_neighbors)(MinHeap *minheap, HashInTree *hashintree, MinHeapNode *min_node)
) {
    // Init structures
    MinHeap *minheap = MinHeap_create(total_number_of_nodes);
    HashInTree *hashintree = HashInTree_create(2*total_number_of_nodes + 1);
    // Insert source node
    MinHeapNode *sourcenode = HashInTree_find_seat(hashintree, sourceData, hashfunc, isEqual);
    sourcenode->data = sourceData;
    sourcenode->weight = sourceWeight;
    sourcenode->parent = NULL;
    MinHeap_update_key(minheap, sourcenode, true, isBetterThan);
    // Do Dijkstra's algorithm
    MinHeapNode *min_node;
    while ((min_node = MinHeap_pluck_min(minheap, isBetterThan))) {
        send_neighbors(minheap, hashintree, min_node);
    } MinHeap_free(minheap);
    return hashintree;
}