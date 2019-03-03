/**
 * Date: 01/20/2019
 * Class: CS5541
 * Assignment: Assignment 3 - Cachelab
 * Author: Ahmed Radwan
 * Email: ahmedabdelwaha.radwan@wmich.edu
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <limits.h>
#include "cache.h"
#include "set.h"
#include "line.h"
#include "stats.h"

#define LINE_SIZE 128
#define ADDRESS_SIZE 64

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

void check_arguments(char **argv, int argc) {
	if (argc < 9 || argc > 11) {
		printf("Usage: ./Cachelab [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
		exit(-1);
	}
}

void declarations(cache_t **lcache, int s, int e) {
	int i;
	int j;
	int n_sets;

	n_sets = pow(2, s);

	// Create cache
	(*lcache) = malloc(sizeof(cache_t));

	// Malloc sets
	(*lcache)->sets = malloc(n_sets * sizeof(queue_set_t *));

	// Initialize set
	for (i = 0; i < n_sets; i++) {
		(*lcache)->sets[i] = create_queue();
		for (j = 0; j < e; j++) {
			enqueue((*lcache)->sets[i], 0, INT_MAX);
		}
	}
}

void open_trace_file(FILE **fp, char *fname) {
	(*fp) = fopen(fname, "r");
	if ((*fp) == NULL) {
		printf("%s does not exist.\n", fname);
		exit(-1);
	}
}

void parse_command_line_argument(char **fname, char **argv, int *s,
 int *e, int *b, int *v, int argc) {
	int option;
	int fname_len;

	// Parse command line arguments
	while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (option) {
			case 'v':
				(*v) = 1;
			break;
			case 's':
				(*s) = atoi(optarg);
			break;
			case 'E':
				(*e) = atoi(optarg);
			break;
			case 'b':
				(*b) = atoi(optarg);
			break;
			case 't':
				fname_len = strlen(optarg);
				(*fname) = malloc(fname_len + 1);
				strncpy((*fname), optarg, fname_len);
				(*fname)[fname_len] = '\0';
			break;
		}
	}
}

void print_stats(stats *cache_stats) {
	printf("hits: %d ", cache_stats->hits);
	printf("misses: %d ", cache_stats->total_misses);
	printf("evictions: %d\n", cache_stats->evictions);
}

void process_load(char *str, cache_t **lcache, int s,
	 int e, int b, int v, stats *cache_stats) {

	line_t *temp;
	char *end_ptr;
	unsigned long byte_addr;
	unsigned int set_bits;
	unsigned int tag_bits;
	unsigned int byte_offset_bits;
	int n_tag_bits;
	int n_sets;
	int i;

	// Calculate number of sets
	n_sets = pow(2, s);

	// Compute byte address
	// printf("string: %s\n", str);
	byte_addr = strtol(str, &end_ptr, 16);
	// printf("byte_addr: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(byte_addr));

	// Compute byte offset bits
	byte_offset_bits = (ULONG_MAX >> ((ADDRESS_SIZE) - b));
	// printf("offset_bits: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(byte_offset_bits));

	// Compute tag bits
	tag_bits = byte_addr >> s >> b;
	// printf("tag_bits: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(tag_bits));

	n_tag_bits = (ADDRESS_SIZE) - s - b;

	// Get the set from byte_addr
	set_bits = byte_addr & ((ULONG_MAX >> n_tag_bits) - byte_offset_bits);
	set_bits >>= b;
	// printf("set_bits: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(set_bits));

	// printf("n_lines = %d\n", n_lines);

	// Hit
	temp = (*lcache)->sets[set_bits]->front;
	while (temp != NULL) {
		if (temp->valid == 1 && temp->tag == tag_bits) {
			remove_item_by_tag((*lcache)->sets[set_bits], tag_bits);
			enqueue((*lcache)->sets[set_bits], 1, tag_bits);
			cache_stats->hits++;

			if (v) {
				printf("hit ");
			}
			return;
		}
		temp = temp->next;
	}
	
	// Miss
	cache_stats->total_misses++;

	// Look for lines with no valid bits (cold miss)
	temp = (*lcache)->sets[set_bits]->front;
	while (temp != NULL && temp->valid != 0) {
		temp = temp->next;
	}

	if (temp != NULL && temp->valid == 0) { // Cold miss
		remove_item_by_valid((*lcache)->sets[set_bits], 0);
		enqueue((*lcache)->sets[set_bits], 1, tag_bits);

		if (v) {
			printf("miss ");
		}
		cache_stats->cold_misses++;
	} else { // Eviction
		dequeue((*lcache)->sets[set_bits]);
		enqueue((*lcache)->sets[set_bits], 1, tag_bits);

		if (v) {
			printf("miss eviction ");
		}
		cache_stats->evictions++;
	}
}

void read_and_process(FILE *fp, cache_t **lcache, int s,
	 int e, int b, int v, stats *cache_stats) {
	char buf[LINE_SIZE];
	char *ptr;
	char *addr;
	char *mode;
	int len;

	int counter = 0;

	// Grab a line
	while (fgets(buf, (LINE_SIZE), fp) != NULL) {
		printf("%d.", counter++);
		// I, L, M, S
		ptr = strtok(buf, " ");

		// Verbose
		if (v) {
			printf("%s ", ptr);
		}
		len = strlen(ptr);
		mode = malloc(len + 1);
		strncpy(mode, ptr, len);

		// Address
		ptr = strtok(NULL, " ,");

		// Verbose
		if (v) {
			printf("%s,", ptr);
		}

		len = strlen(ptr);
		addr = malloc(len + 1);
		strncpy(addr, ptr, len);

		// Bytes
		ptr = strtok(NULL, "\n");

		// Verbose
		if (v) {
			printf("%s ", ptr);
		}

		if (strncmp(mode, "I", 1) != 0) {
			process_load(addr, lcache, s, e, b, v, cache_stats);
		}
		if (strncmp(mode, "M", 1) == 0) {
			process_load(addr, lcache, s, e, b, v, cache_stats);
		}

		// Verbose
		if (v) {
			printf("\n");
		}
	}

}

int main(int argc, char **argv) {
	// Cache
	cache_t *lcache;

	// File
	FILE *fp = NULL;
	char *fname;

	// Input
	int s;
	int e;
	int b;
	// int h;
	int v = 0;

	// Stats
	stats *cache_stats = malloc(sizeof(stats));


	// Check number of arguments
	check_arguments(argv, argc);

	// Parse command line argumments
	parse_command_line_argument(&fname, argv, &s, &e, &b, &v, argc);

	// Declarations
	declarations(&lcache, s, e);

	// Open trace file
	open_trace_file(&fp, fname);

	// Read trace file
	read_and_process(fp, &lcache, s, e, b, v, cache_stats);

	// Close trace file
	fclose(fp);

	// Print stats
	print_stats(cache_stats);


	return 0;
}
