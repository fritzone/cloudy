#include "list.h"
#include "memhandl.h"

#include <stdio.h>
#include <stdlib.h>

// Compare function for integers
int compareInts(const void *a, const void *b)
{
    return (*(int*)a - *(int*)b);
}

struct LinkedList* createLinkedList()
{
  LinkedList *newList = (LinkedList*)calloc(sizeof(struct LinkedList), 1);
  if (newList == NULL)
  {
      perror("Memory allocation error");
      exit(1);
  }
  newList->head = newList->currentSelected = newList->displayStart = newList->previousSelected = NULL;
  return newList;
}

void insertAtBeginning(struct LinkedList *list, void *data)
{
  Node *newNode = allocate<Node>();
  if (newNode == NULL)
  {
      perror("Memory allocation error");
      exit(1);
  }
  newNode->data = data;
  newNode->next = list->head;
  list->head = newNode;
  list->displayStart = list->head;
  list->currentSelected = list->head;
  list->count ++;
}

void insertAtEnd(struct LinkedList *list, void *data)
{
  Node *newNode = allocate<Node>();
  if (newNode == NULL)
  {
    perror("Memory allocation error");
    exit(1);
  }
  newNode->data = data;
  newNode->next = NULL;
  if (list->head == NULL)
  {
    list->head = list->displayStart = list->currentSelected = newNode;
  }
  else
  {
    struct Node *current = list->head;
    while (current->next != NULL)
    {
      current = current->next;
    }
    current->next = newNode;
  }
  list->count ++;
}

void freeLinkedList(struct LinkedList *list, void (*destructor)(void*) )
{
    struct Node *current = list->head;
    while (current != NULL)
    {
        struct Node *temp = current;
        current = current->next;
        if(destructor)
        {
          destructor(temp);
        }
        free(temp);
    }
    
    list->displayStart = NULL;
    list->displayEnd = NULL;
    list->currentSelected = NULL;
    list->previousSelected = NULL;

    free(list);
}

// Function to sort the linked list using a comparison function
void sortLinkedList(struct LinkedList *list, int (*compare)(const void *, const void *)) {
    if (list->head == NULL) {
        return; // Nothing to sort
    }

    int length = 0;
    struct Node *current = list->head;

    // Count the number of nodes in the list
    while (current != NULL) {
        length++;
        current = current->next;
    }

    // Convert the linked list to an array of void* pointers
    void **array = (void **)malloc(length * sizeof(void *));
    if (array == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    current = list->head;
    for (int i = 0; i < length; i++) {
        array[i] = current->data;
        current = current->next;
    }

    // Sort the array using the provided comparison function
    qsort(array, length, sizeof(void *), compare);

    // Update the linked list with the sorted data
    current = list->head;
    for (int i = 0; i < length; i++) {
        current->data = array[i];
        current = current->next;
    }

    free(array);
}

int distance(Node* node1, Node* node2)
{
  int dist = 0;
  Node* q = node1;
  while(q && q != node2)
  {
    dist ++;
    q = q->next;
  }
  return dist;
}

