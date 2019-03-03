/**
 * Date: 01/20/2019
 * Class: CS5541
 * Assignment: Assignment 3 - Cachelab
 * Author: Ahmed Radwan
 * Email: ahmedabdelwaha.radwan@wmich.edu
 */
#ifndef LINE_H
#define LINE_H

typedef struct struct_line {
    unsigned int tag;
    int valid;
    struct struct_line *next;
} line_t;
#endif
