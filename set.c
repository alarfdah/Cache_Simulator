/**
 * Date: 01/20/2019
 * Class: CS5541
 * Assignment: Assignment 3 - Cachelab
 * Author: Ahmed Radwan
 * Email: ahmedabdelwaha.radwan@wmich.edu
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "set.h"
#include "line.h"


// A utility function to create a new linked list node.
line_t *new_node(int valid, int tag) {
    line_t *line = (line_t*)malloc(sizeof(line_t));
    line->valid = valid;
		line->tag = tag;
    line->next = NULL;
    return line;
}

// A utility function to create an empty queue
queue_set_t *create_queue() {
    queue_set_t *q = (queue_set_t *)malloc(sizeof(queue_set_t));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enqueue(queue_set_t *q, int valid, int tag) {
    // Create a new LL node
    line_t *temp = new_node(valid, tag);

    // If queue is empty, then new node is front and rear both
    if (q->front == NULL) {
       q->front = q->rear = temp;
			 q->size++;
       return;
    }

    // Add the new node at the end of queue and change rear
		temp->next = q->front;
		q->front = temp;

		q->size++;
}

// Function to remove a key from given queue q
void *dequeue(queue_set_t *q) {
    // If queue is empty, return NULL.
    if (q->rear == NULL)
       return NULL;

		line_t *temp = q->front;
		while (temp->next != q->rear) {
			temp = temp->next;
		}
		q->rear = temp;
		temp->next = NULL;

    // If front becomes NULL, then change rear also as NULL
    if (q->rear == NULL)
       q->front = NULL;

		q->size--;
}

void remove_item_by_valid(queue_set_t *q, int valid) {
	if (q == NULL) {
		printf("remove by valid: queue is null!\n");
		return;
	}
	if (q->front->valid == 0) {
		q->front = q->front->next;
		if (q->front == NULL) {
			q->rear = NULL;
		}
    q->size --;
		return;
	}

	line_t *temp = q->front;
	while (temp->next != NULL) {
		if (temp->next->valid == 0) {
			temp->next = temp->next->next;
			if (temp->next == NULL) {
				q->rear = temp;
			}
			q->size --;
			return;
		}
		temp = temp->next;
	}

}

void remove_item_by_tag(queue_set_t *q, int tag) {
	if (q == NULL) {
		printf("remove by valid: queue is null!\n");
		return;
	}
	if (q->front->valid == 1 && q->front->tag == tag) {
		q->front = q->front->next;
		if (q->front == NULL) {
			q->rear = NULL;
		}
		q->size --;
		return;
	}

	line_t *temp = q->front;
	while (temp->next != NULL) {
		if (temp->next != NULL && temp->next->valid == 1 && temp->next->tag == tag) {
			temp->next = temp->next->next;
			if (temp->next == NULL) {
				q->rear = temp;
			}
			q->size--;
			return;
		}
		temp = temp->next;
	}

}
