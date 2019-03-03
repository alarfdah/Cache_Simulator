/**
 * Date: 01/20/2019
 * Class: CS5541
 * Assignment: Assignment 3 - Cachelab
 * Author: Ahmed Radwan
 * Email: ahmedabdelwaha.radwan@wmich.edu
 */
#ifndef CACHE_H
#define CACHE_H

#include "set.h"
typedef struct struct_cache {
    queue_set_t **sets;
} cache_t;
#endif
