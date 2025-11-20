#include "../include/double_linked_list.h"
#include "../include/memoryManager.h"
#include <stddef.h>
#include <stdint.h>

DoubleLinkedListADT createDoubleLinkedList()
{
	void *mem = allocMemory(sizeof(DoubleLinkedListCDT));
	DoubleLinkedListADT list = (DoubleLinkedListADT)mem;
	if (!list)
		return NULL;
	list->first = NULL;
	list->last = NULL;
	list->size = 0;
	return list;
}

static Node *createNode(void *data)
{
	void *mem = allocMemory(sizeof(Node));
	Node *n = (Node *)mem;
	if (!n)
		return NULL;
	n->data = data;
	n->next = NULL;
	n->prev = NULL;
	return n;
}

int insertFirst(DoubleLinkedListADT list, void *data)
{
	if (!list)
		return -1;
	Node *n = createNode(data);
	if (!n)
		return -1;
	n->next = list->first;
	if (list->first)
		list->first->prev = n;
	list->first = n;
	if (!list->last)
		list->last = n;
	list->size++;
	return 0;
}

int insertLast(DoubleLinkedListADT list, void *data)
{
	if (!list)
		return -1;
	Node *n = createNode(data);
	if (!n)
		return -1;
	n->prev = list->last;
	if (list->last)
		list->last->next = n;
	list->last = n;
	if (!list->first)
		list->first = n;
	list->size++;
	return 0;
}

int removeFirst(DoubleLinkedListADT list)
{
	if (!list || !list->first)
		return -1;
	Node *n = list->first;
	list->first = n->next;
	if (list->first)
		list->first->prev = NULL;
	else
		list->last = NULL;
	freeMemory(n);
	list->size--;
	return 0;
}

int removeLast(DoubleLinkedListADT list)
{
	if (!list || !list->last)
		return -1;
	Node *n = list->last;
	list->last = n->prev;
	if (list->last)
		list->last->next = NULL;
	else
		list->first = NULL;
	freeMemory(n);
	list->size--;
	return 0;
}

int removeElement(DoubleLinkedListADT list, void *data)
{
	if (!list)
		return -1;
	Node *it = list->first;
	while (it) {
		if (it->data == data) {
			if (it->prev)
				it->prev->next = it->next;
			else
				list->first = it->next;
			if (it->next)
				it->next->prev = it->prev;
			else
				list->last = it->prev;
			freeMemory(it);
			list->size--;
			return 0;
		}
		it = it->next;
	}
	return -1;
}

void *getFirst(DoubleLinkedListADT list)
{
	if (!list || !list->first)
		return NULL;
	return list->first->data;
}

void *getLast(DoubleLinkedListADT list)
{
	if (!list || !list->last)
		return NULL;
	return list->last->data;
}

int getSize(DoubleLinkedListADT list)
{
	if (!list)
		return 0;
	return list->size;
}

int isEmpty(DoubleLinkedListADT list)
{
	if (!list)
		return 1;
	return list->size == 0;
}

int freeList(DoubleLinkedListADT list)
{
	if (!list)
		return -1;
	Node *it = list->first;
	while (it) {
		Node *next = it->next;
		freeMemory(it);
		it = next;
	}
	freeMemory(list);
	return 0;
}
