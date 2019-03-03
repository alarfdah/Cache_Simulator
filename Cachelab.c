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

void help_menu() {
	printf("Usage: ./Cachelab [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
	printf("Options:\n");
	printf("\t-h\t\tPrint this help message.\n");
	printf("\t-v\t\tOptional verbose flag.\n");
	printf("\t-s <num>\tNumber of set index bits.\n");
	printf("\t-E <num>\tNumber of lines per set.\n");
	printf("\t-b <num>\tNumber of block offset bits.\n");
	printf("\t-t <file>\tTrace file.\n");
	printf("\n");
	printf("Examples:\n");
	printf("\tlinux> ./Cachelab -s 4 -E 1 -b 4 -t ./traces/yi.trace\n");
	printf("\tlinux> ./Cachelab -v -s 6 -E 3 -b 2 -t ./traces/yi.trace\n");
	exit(0);
}

void check_arguments(char **argv, int argc) {
	if (argc < 9 || argc > 11) {
		help_menu();
		exit(0);
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
 int *e, int *b, int *h, int *v, int argc) {
	int option;
	int fname_len;
	int s_exists = 0;
	int e_exists = 0;
	int b_exists = 0;
	int t_exists = 0;

	// Parse command line arguments
	while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (option) {
			case 'h':
				(*h) = 1;
			break;
			case 'v':
				(*v) = 1;
			break;
			case 's':
				(*s) = atoi(optarg);
				s_exists = 1;
			break;
			case 'E':
				(*e) = atoi(optarg);
				e_exists = 1;
			break;
			case 'b':
				(*b) = atoi(optarg);
				b_exists = 1;
			break;
			case 't':
				fname_len = strlen(optarg);
				(*fname) = malloc(fname_len + 1);
				strncpy((*fname), optarg, fname_len);
				(*fname)[fname_len] = '\0';
				t_exists = 1;
			break;
		}
	}

	// Check that non-optional flags are set
	if (!s_exists || !e_exists || !b_exists || !t_exists) {
		help_menu();
	}
}

void print_stats(stats *cache_stats, int v) {
	// Print stats
	printf("hits: %d ", cache_stats->hits);
	printf("misses: %d ", cache_stats->total_misses);
	printf("evictions: %d", cache_stats->evictions);

	// Verbose
	if (v) {
		printf(" cold misses: %d", cache_stats->cold_misses);
	}
	printf("\n");
}

void process(char *str, cache_t **lcache, int s,
	 int e, int b, int v, stats *cache_stats) {

	line_t *temp;
	char *end_ptr;
	unsigned long byte_addr;
	unsigned int set_bits;
	unsigned int tag_bits;
	unsigned int byte_offset_bits;
	int n_tag_bits;

	// Compute byte address
	byte_addr = strtol(str, &end_ptr, 16);

	// Compute byte offset bits
	byte_offset_bits = (ULONG_MAX >> ((ADDRESS_SIZE) - b));

	// Compute tag bits
	tag_bits = byte_addr >> s >> b;
	n_tag_bits = (ADDRESS_SIZE) - s - b;

	// Get the set bits from byte_addr
	set_bits = byte_addr & ((ULONG_MAX >> n_tag_bits) - byte_offset_bits);
	set_bits >>= b;

	// Cache Hit
	temp = (*lcache)->sets[set_bits]->front;
	while (temp != NULL) {
		if (temp->valid == 1 && temp->tag == tag_bits) {
			// Remove item we just accessed
			remove_item_by_tag((*lcache)->sets[set_bits], tag_bits);

			// Put it back at the front of the queue
			enqueue((*lcache)->sets[set_bits], 1, tag_bits);

			// Increment cache hits
			cache_stats->hits++;

			// Verbose
			if (v) {
				printf("hit ");
			}
			return;
		}
		temp = temp->next;
	}

	// Cache Miss
	cache_stats->total_misses++;

	// Look for lines with no valid bits (cold miss)
	temp = (*lcache)->sets[set_bits]->front;
	while (temp != NULL && temp->valid != 0) {
		temp = temp->next;
	}

	// Cold Miss
	if (temp != NULL && temp->valid == 0) {
		// Remove item with non-valid bit
		remove_item_by_valid((*lcache)->sets[set_bits], 0);

		// Enqueue item with valid bit
		enqueue((*lcache)->sets[set_bits], 1, tag_bits);

		// Increment cold misses
		cache_stats->cold_misses++;

		// Verbose
		if (v) {
			printf("miss ");
		}
	// Miss Eviction
	} else {
		// Remove least recently used item (at the rear of queue)
		dequeue((*lcache)->sets[set_bits]);
		// Enqueue new item
		enqueue((*lcache)->sets[set_bits], 1, tag_bits);
		// Increment miss evictions
		cache_stats->evictions++;

		// Verbose
		if (v) {
			printf("miss eviction ");
		}
	}
}

void read_and_process(FILE *fp, cache_t **lcache, int s,
	 int e, int b, int v, stats *cache_stats) {
	char buf[LINE_SIZE];
	char *ptr;
	char *addr;
	char *mode;
	int len;

	// Grab a line
	while (fgets(buf, (LINE_SIZE), fp) != NULL) {
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

		// Process
		if (strncmp(mode, "I", 1) != 0) {
			process(addr, lcache, s, e, b, v, cache_stats);
		}
		if (strncmp(mode, "M", 1) == 0) {
			process(addr, lcache, s, e, b, v, cache_stats);
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
	int h = 0;
	int v = 0;

	// Stats
	stats *cache_stats = malloc(sizeof(stats));

	// Check number of arguments
	check_arguments(argv, argc);

	// Parse command line argumments
	parse_command_line_argument(&fname, argv, &s, &e, &b, &h, &v, argc);

	// Help menu
	if (h) {
		help_menu();
	}

	// Declarations
	declarations(&lcache, s, e);

	// Open trace file
	open_trace_file(&fp, fname);

	// Read trace file
	read_and_process(fp, &lcache, s, e, b, v, cache_stats);

	// Close trace file
	fclose(fp);

	// Print stats
	print_stats(cache_stats, v);


	return 0;
}
