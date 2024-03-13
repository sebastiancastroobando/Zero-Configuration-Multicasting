#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

// Function to create a new queue
Queue* createQueue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->data = (char**)malloc(capacity * sizeof(char*));
    queue->capacity = capacity;
    queue->front = queue->rear = -1;    // Initially queue is empty
    return queue;
}

// Check if the queue is full
int isFull(Queue* queue) {
    return queue->rear == queue->capacity - 1;
}

// Check if the queue is empty
int isEmpty(Queue* queue) {
    return queue->front == -1;
}

// Function to insert an element (char*) into the queue
void enqueue(Queue* queue, char* item) {
    if (isFull(queue)) {
        printf("Queue overflow\n");
        return;
    }
    queue->rear = queue->rear + 1;
    queue->data[queue->rear] = item;
    if (queue->front == -1) {
        queue->front = 0;
    }
}

// Function to remove an element (char*) from the queue
char* dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue underflow\n");
        return NULL;
    }
    char* item = queue->data[queue->front];
    queue->front = queue->front + 1;
    if (queue->front > queue->rear) {
        queue->front = queue->rear = -1;    // Reset if only one element left
    }
    return item;
}

// Free the memory allocated to the queue
void deleteQueue(Queue* queue) {
    if (queue != NULL) {
        free(queue->data);
        free(queue);
    }
}

// Example usage
/*int main() {
    printf("pre-create\n");
    Queue* queue = createQueue(100);
    printf("queue created\n");
    enqueue(queue, "Hello");
    printf("enque1\n");
    enqueue(queue, "World");
    printf("enque2\n");
    enqueue(queue, "!");
    printf("enque3\n");

    char* item = dequeue(queue);
    printf("Dequeued item: %s\n", item); // Prints: Hello

    item = dequeue(queue);
    printf("Dequeued item: %s\n", item); // Prints: World
    
    item = dequeue(queue);
    printf("Dequeued item: %s\n", item); // Prints: !

    item = dequeue(queue); // should error

    deleteQueue(queue);
    return 0;
}*/

