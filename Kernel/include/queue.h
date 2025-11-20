#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue *QueueADT;

/*
 * createQueue
 * @return: a new QueueADT instance or NULL on failure
 */
QueueADT createQueue();

/*
 * freeQueue
 * Frees internal resources for `queue`.
 */
void freeQueue(QueueADT queue);

/*
 * enqueue
 * @param queue: queue to enqueue into
 * @param data: pointer to data
 * @return: 0 on success, -1 on failure
 */
int enqueue(QueueADT queue, void *data);

/*
 * dequeue
 * Removes and returns the next element from the queue.
 * @return: pointer to data or NULL if empty
 */
void *dequeue(QueueADT queue);

/*
 * queueSize
 * @return: number of elements in the queue
 */
int queueSize(QueueADT queue);

/*
 * isQueueEmpty
 * @return: 1 if empty, 0 otherwise
 */
int isQueueEmpty(QueueADT queue);

/*
 * peekQueue
 * @return: pointer to data at front without removing, or NULL if empty
 */
void *peekQueue(QueueADT queue);

/*
 * clearQueue
 * Empties the queue and frees element wrappers (not the data itself)
 */
void clearQueue(QueueADT queue);

/*
 * containsQueue
 * @param compare: function to compare stored data with `data` (returns 0 on match)
 * @return: pointer to found data, or NULL if not found
 */
void *containsQueue(QueueADT queue, void *data, int (*compare)(void *, void *));

/*
 * remove
 * Removes the first element matching `data` using `compare` and returns it.
 * @return: pointer to removed data or NULL if not found
 */
void *remove(QueueADT queue, void *data, int (*compare)(void *, void *));

/*
 * dumpQueue
 * Returns an array of pointers to the queue elements (caller frees array).
 * @return: NULL-terminated array of pointers or NULL on failure
 */
void **dumpQueue(QueueADT queue);

#endif /* QUEUE_H */