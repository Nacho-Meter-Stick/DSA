#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct node {   // represents one node in a Linked List
  void         *data;   // pointer to data associated with this node
  struct node  *next;   // pointer to next node in List
  struct node  *prev;   // pointer to previous node in List
} Node;

typedef struct {   // represents a Linked List
  Node  *header;   // pointer to the "dummy header node" of the Linked List
  int   size;      // number of nodes in the Linked List
} LinkedList;


// function proto-types
void createList ( LinkedList *someList );
void freeList( LinkedList *someList );
void addEnd( LinkedList *someList, void *newData ); // this prototype was missing
void *pop( LinkedList *someList, int position );
void outputList( LinkedList *someList );
void addFirst( LinkedList *someList, void *newElement );
void *removeFirst( LinkedList *someList );
void swap( LinkedList *someList, unsigned int i, unsigned int j );
void *change( LinkedList *someList, int position, void *newElement );

// Initially the List is empty
// We must create memory for the "dummy header node"
void createList ( LinkedList *someList )
{
  someList->size = 0; 

  someList->header = (Node *)malloc ( sizeof (Node) );

  someList->header->data = NULL;
  someList->header->next = someList->header;
  someList->header->prev = someList->header;
}
void freeList(LinkedList *someList) {
    while (someList->size) {
        free(removeFirst(someList));
    } free(someList->header);
}

// add the new data element to the end of the List
void addEnd( LinkedList *someList, void *newData )
{
  //Node *lastNode = someList->header->prev;

  Node *newNode = (Node *)malloc ( sizeof ( Node ) );
 
  newNode->data = newData;          // set the fields of the new Node
  newNode->next = someList->header;
  newNode->prev = someList->header->prev;

  someList->header->prev->next = newNode;  // splice-in the newNode
  someList->header->prev = newNode;        // into the List

  someList->size++;
}

// Input the dummy header, the desired index, 
//   and the length of the list.
// Returns a pointer to the node at the desired index.
// This function exists to abstract away 1 optimization,
//   and that is to move backwards if backwards is closer.
// This function does assume that 0<=i<size.
Node *get_node_at(Node *header, long int i, unsigned int size) {
  if (size-1-i < i) {
      while (i++ < size) header = header->prev;
  } else {
      while (i-- >= 0) header = header->next;
  } return header;
}

// remove the item in the given positionn in the List, and
// return that data
void *pop(LinkedList *someList, int position) {
  if (position < 0 || someList->size <= position) exit(2);

  // walk down the list until we reach the node to be removed
  Node *badNode = get_node_at(someList->header, position, someList->size);
  void *removedData = badNode->data;

  badNode->prev->next = badNode->next;   // splice-out the Node
  badNode->next->prev = badNode->prev; 
 
  someList->size--;

  free(badNode);   // free-up the memory of the deleted Node

  return ( removedData );
}


// output the contents of the List
// it is assumed that the elements in the list are strings
void outputList(LinkedList *someList) {
  if (someList == NULL) return;

  if (someList->size == 0)  {
     printf("[]\n");
     return;
  }

  printf("[");
  Node *temp = someList->header->next;
  for (int num = 0; num < someList->size; num++) {
     printf("%s%s", (char *) temp->data, (num < someList->size-1) ? " " : "");
     temp =  temp->next;
  } printf("]\n");
}


// add the newElement to position 0 in the List
// that is, the newElement precedes all other elements in the list
void addFirst(LinkedList *someList, void *newElement) {
    Node *newNode = (Node *)malloc(sizeof(Node));

    newNode->data = newElement;
    newNode->prev = someList->header;
    newNode->next = someList->header->next;

    someList->header->next->prev = newNode;
    someList->header->next = newNode;

    someList->size++;
}


// remove the first element in the list, and return a pointer
// to that data item.
// if the list is empty, exit the program with status 2
void *removeFirst(LinkedList *someList) {
    if (someList->size == 0) exit(2);
    Node *badNode = someList->header->next;
    someList->header->next = badNode->next;
    badNode->next->prev = someList->header;

    void *ret = badNode->data;
    free(badNode);
    someList->size--;
    return ret;
}

// swap the two elements in the list as indicated by their indices
// if one or both of the positions/indices specified in the input
// are not on the list, exit the program with status 2
void swap(LinkedList *someList, unsigned int i, unsigned int j) {
    if(someList->size <= i || someList->size <= j) {
        exit(2);
    }
    // Move the node pointers into place
    Node *p1 = get_node_at(someList->header, i, someList->size);
    Node *p2 = get_node_at(someList->header, j, someList->size);
    
    // Swap their data pointers (compiler does not like xor-swap here)
    void *temp = p1->data;
    p1->data = p2->data;
    p2->data = temp;
}

// modify the data in the given position in the List to
// be the newElement.  Return a pointer to the data that
// was over-written and deleted from the list
// if the position is illegal then exit the program with status 2
void *change(LinkedList *someList, int position, void *newElement) {
    if (position < 0 || someList->size <= position) exit(2);
    Node *p = get_node_at(someList->header, position, someList->size);
    void *oldData = p->data;
    p->data = newElement;
    return oldData;
}
