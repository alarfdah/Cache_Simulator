/**
 * Date: 01/20/2019
 * Class: CS5541
 * Assignment: Assignment 3 - Cachelab
 * Author: Ahmed Radwan
 * Email: ahmedabdelwaha.radwan@wmich.edu
 */
#ifndef SET_H
#define SET_H
#include "line.h"

typedef struct struct_queue_set {
	line_t *front;
	line_t *rear;
	int size;
	unsigned int capacity;
} queue_set_t;


line_t* new_node(int valid, int tag);
queue_set_t *create_queue();
void enqueue(queue_set_t *q, int valid, int tag);
void *dequeue(queue_set_t *q);
void remove_item_by_valid(queue_set_t *q, int valid);
void remove_item_by_tag(queue_set_t *q, int tag);


#endif
