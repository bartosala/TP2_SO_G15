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

/*
 * createDoubleLinkedList
 * @return: newly allocated DoubleLinkedListADT, or NULL on failure
 */
DoubleLinkedListADT createDoubleLinkedList();

/*
 * insertFirst
 * @param list: list to insert into
 * @param data: pointer to data to store in the new node
 * @return: 0 on success, -1 on failure
 */
int insertFirst(DoubleLinkedListADT list, void *data);

/*
 * insertLast
 * @param list: list to insert into
 * @param data: pointer to data to store in the new node
 * @return: 0 on success, -1 on failure
 */
int insertLast(DoubleLinkedListADT list, void *data);

/*
 * removeFirst
 * @param list: list to remove from
 * @return: 0 on success, -1 if list empty or error
 */
int removeFirst(DoubleLinkedListADT list);

/*
 * removeLast
 * @param list: list to remove from
 * @return: 0 on success, -1 if list empty or error
 */
int removeLast(DoubleLinkedListADT list);

/*
 * removeElement
 * @param list: list to remove from
 * @param data: data pointer to match and remove (match by pointer)
 * @return: 0 on success, -1 if element not found or error
 */
int removeElement(DoubleLinkedListADT list, void *data);

/*
 * getFirst
 * @param list: source list
 * @return: pointer to the first element's data, or NULL if empty
 */
void *getFirst(DoubleLinkedListADT list);

/*
 * getLast
 * @param list: source list
 * @return: pointer to the last element's data, or NULL if empty
 */
void *getLast(DoubleLinkedListADT list);

/*
 * getSize
 * @param list: source list
 * @return: number of elements in the list
 */
int getSize(DoubleLinkedListADT list);

/*
 * isEmpty
 * @param list: source list
 * @return: 1 if empty, 0 otherwise
 */
int isEmpty(DoubleLinkedListADT list);

/*
 * freeList
 * @param list: list to free
 * @return: 0 on success, -1 on failure
 */
int freeList(DoubleLinkedListADT list);

#endif
