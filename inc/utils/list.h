#ifndef _LIST_H_
#define _LIST_H_

/**
 * An element in the linkedlist, the data holds the actual data
 */
struct Node {
    void *data;
    struct Node *next;
};

/**
 * The actual linked list
 */
struct LinkedList {

  // the head of the list
  struct Node *head;
  
  // where the displaying of the list elements starts
  struct Node* displayStart;

  Node* displayEnd;

  // the currently selected item
  struct Node* currentSelected;

  // the one which was selected before this
  struct Node* previousSelected;

  int count;
  double size;

};

/**
 * Will create a linked list
 */
struct LinkedList* createLinkedList();

/**
 * Will insert a new element in the given list at the beginning
 */
void insertAtBeginning(struct LinkedList *list, void *data);

/**
 * Will insert the given data at the of the list
 */
void insertAtEnd(struct LinkedList *list, void *data) ;

/**
 * Free the linked list
 * The function pointer goes to free the allocated elements in the list
 */
void freeLinkedList(struct LinkedList *list, void (*freeer)(void*));

/*
 * Will calculate the distance between the two nodes in the list
 */
int distance(Node* node1, Node* node2);

#endif

