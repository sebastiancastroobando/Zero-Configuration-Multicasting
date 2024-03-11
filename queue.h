#ifndef QUEUE_H
#define QUEUE_H

// Define the Queue structure
typedef struct queue {
  char** data;  // Pointer to an array of character pointers (strings)
  int capacity;  // Maximum number of elements the queue can hold
  int front;    // Index of the front element in the queue
  int rear;     // Index of the element after the rear element (one past the last element)
} Queue;

// Function prototypes for the queue library
Queue* createQueue(int capacity);
int isFull(Queue* queue);
int isEmpty(Queue* queue);
void enqueue(Queue* queue, char* item);
char* dequeue(Queue* queue);
void deleteQueue(Queue* queue);

#endif // QUEUE_H

