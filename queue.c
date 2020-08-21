/* derived from tutorials point code
*  https://www.tutorialspoint.com/data_structures_algorithms/queue_program_in_c.htm */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#ifdef __APPLE__
    const int QUEUE_SIZE = 6;
    const int NUMBER_OF_QUEUES = 100;
#endif

#ifdef __linux__
    #define QUEUE_SIZE 6
    #define NUMBER_OF_QUEUES 100
#endif
struct queue {
    struct gt * intArray[QUEUE_SIZE];
    int front;
    int rear;
    int itemCount;
};

struct queue* priority_queues[NUMBER_OF_QUEUES];

void init();
void destroy();
bool isEmpty(int prio);
struct gt * peek (int prio);
bool isFull(int prio);
int size(int prio);
void enqueue(struct gt* data, int prio);
struct gt * dequeue(int prio);
void print_queue(int prio);

void init() {
    for (int i = 0; i < NUMBER_OF_QUEUES; i++) {
        priority_queues[i] = (struct queue *) malloc(sizeof(struct queue));
        priority_queues[i]->front = 0;
        priority_queues[i]->rear = -1;
        priority_queues[i]->itemCount = 0;
    }
}

void destroy() {
    for (int i = 0; i < NUMBER_OF_QUEUES; i++) {
        //struct queue *q = priority_queues[i];     //apparently this does NOT create a separate pointer to priority_queues[i]
        free((void *) priority_queues[i]);
    }
}

bool isEmpty(int prio) {
    return size(prio) == 0;
}

//need to check return value
//if NULL, the queue is empty
struct gt * peek(int prio) {
    if (!isEmpty(prio)) {
        return priority_queues[prio]->intArray[priority_queues[prio]->front];
    }
    return NULL;
}

bool isFull(int prio) {
    return size(prio) == QUEUE_SIZE;
}

int size(int prio) {
    return priority_queues[prio]->itemCount;
}

void enqueue(struct gt * data, int prio) {
    if(!isFull(prio)) {

        priority_queues[prio]->intArray[++priority_queues[prio]->rear] = data;

        if(priority_queues[prio]->rear == QUEUE_SIZE-1) {
            priority_queues[prio]->rear = -1;
        }
        priority_queues[prio]->itemCount++;
    } else {
        printf("cannot enqueue when queue is full\n");
    }
}

//need to check return value
//if NULL, queue was empty!
struct gt * dequeue(int prio) {
    if (!isEmpty(prio)) {
        struct gt * data = priority_queues[prio]->intArray[priority_queues[prio]->front++];

        if(priority_queues[prio]->front == QUEUE_SIZE) {
            priority_queues[prio]->front = 0;
        }

        priority_queues[prio]->itemCount--;
        return data;
    } else {
        printf("cannot dequeue when queue is empty\n");
        return NULL;
    }
}

////i is the position in the queue
////j is the index into the gttbl array
//void print_queue(int prio) {
//    int i= priority_queues[prio]->front;
//    int count = size(prio);
//
//    printf("queue %d: ", prio);
//
//    while (count > 0) {
//        //identify which gttbl thread is in intArray[i]
//        for (int j = 0; j < MaxGThreads; j++) {
//            if (&gttbl[j] == priority_queues[prio]->intArray[i]) {
//                printf("Thread gttbl[%d] is in position %d in queue %d", j, i, prio);
//            }
//        }
//        i = (i + 1) % QUEUE_SIZE;
//        count -= 1;
//    }
//    printf("\n");
//}

//int main() {
//    //printf("%d\n", sizeof(struct queue));
//    init();
//    /* insert 5 items */
//    enqueue(3, 5);
//    enqueue(5, 2);
//    enqueue(9, 7);
//    enqueue(1, 5);
//    enqueue(12, 2);
//    enqueue(15, 2);
//
//    if(isFull(5)){
//        printf("Queue with priority 5 is full!\n");
//    } else {
//        printf("queue with priority 5 is not full!\n");
//    }
//
//    for (int i = 0; i < NUMBER_OF_QUEUES; i++) {
//        if (!isEmpty(i)) {
//            print_queue(i);
//        }
//    }
//
//    // remove one item
//    int num = dequeue(2);
//
//    printf("Element removed from queue with priority 2: %d\n",num);
//
//    print_queue(2);
//
//    // insert more items
//    enqueue(16, 2);
//    enqueue(17, 2);
//    enqueue(18, 2);
//    enqueue(19, 2);
//
//    print_queue(2);
//
//    // As queue is full, elements will not be inserted.
//    enqueue(20, 2);
//
//    //remove everything from queue with priority 2
//    printf("order of elements removed from queue 2: ");
//    while(!isEmpty(2)) {
//        num = dequeue(2);
//        printf("%d ",num);
//    }
//    printf("\n");
//    print_queue(2);     //should be empty
//
//
//    printf("Element at front of queue with priority 5: %d\n",peek(5));
//    num = dequeue(5);
//    printf("Element at front of queue with priority 5: %d\n", peek(5));
//
//    //try to peek at an empty queue
//    dequeue(5);
//    if (peek(5) == - 1) {
//        printf("tried to peek when queue is empty\n");
//    } else {
//        printf("Element at front of queue with priority 5: %d\n", peek(5));
//    }
//
//    print_queue(5);
//    destroy();          //reclaim memory associated with all queues
//
//    //should not work, not sure why it still works
//    //stack overflow: this is undefined behavior, you should not rely on it being there
//    for (int i = 0; i < NUMBER_OF_QUEUES; i++) {
//        if (!isEmpty(i)) {
//            print_queue(i);
//        }
//    }
//}
