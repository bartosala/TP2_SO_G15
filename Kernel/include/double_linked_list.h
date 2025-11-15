#ifndef DOUBLE_LINKED_LIST_ADT_H
#define DOUBLE_LINKED_LIST_ADT_H

#include <stddef.h>

typedef struct Node {
	void *data;
	struct Node *next;
	struct Node *prev;
} Node;

typedef struct DoubleLinkedListCDT {
	Node *first;
	Node *last;
	int size;
} DoubleLinkedListCDT;

typedef struct DoubleLinkedListCDT *DoubleLinkedListADT;

DoubleLinkedListADT createDoubleLinkedList();
int insertFirst(DoubleLinkedListADT list, void *data);
int insertLast(DoubleLinkedListADT list, void *data);
int removeFirst(DoubleLinkedListADT list);
int removeLast(DoubleLinkedListADT list);
int removeElement(DoubleLinkedListADT list, void *data);
void *getFirst(DoubleLinkedListADT list);
void *getLast(DoubleLinkedListADT list);
int getSize(DoubleLinkedListADT list);
int isEmpty(DoubleLinkedListADT list);
int freeList(DoubleLinkedListADT list);

#endif
