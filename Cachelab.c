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
#include "Cachelab.h"

#define LINE_SIZE 128
#define ADDRESS_SIZE 64

void check_arguments(char **argv, int argc) {
	if (argc < 9 || argc > 11) {
		printf("Usage: ./Cachelab [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
		exit(-1);
	}
}

void declarations(cache **lcache, int n_sets, int n_lines, int n_bytes) {
	int i;
	int j;

	// Create cache
	(*lcache) = malloc(sizeof(cache));

	// Initialize sets
	(*lcache)->sets = malloc(n_sets * sizeof(set *));

	// Initialize lines
	for (i = 0; i < n_sets; i++) {
		(*lcache)->sets[i] = malloc(sizeof(set));
		(*lcache)->sets[i]->lines = malloc(n_lines * sizeof(line *));
		for (j = 0; j < n_lines; j++) {
			(*lcache)->sets[i]->lines[j] = malloc(sizeof(line));
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

void parse_command_line_argument(char **fname, char **argv, int *sets,
 int *lines, int *bytes, int argc) {
	int option;
	int fname_len;

	// Parse command line arguments
	while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (option) {
			case 's':
				(*sets) = atoi(optarg);
				// Check
				printf("-s %d ", (*sets));
			break;
			case 'E':
				(*lines) = atoi(optarg);
				// Check
				printf("-E %d ", (*lines));
			break;
			case 'b':
				(*bytes) = atoi(optarg);
				// Check
				printf("-b %d ", (*bytes));
			break;
			case 't':
				fname_len = strlen(optarg);
				(*fname) = malloc(fname_len + 1);
				strncpy((*fname), optarg, fname_len);
				(*fname)[fname_len] = '\0';
				// Check
				printf("-t %s\n", (*fname));
			break;
		}
	}
}

void process_load(char *str, cache **lcache, int n_sets,
	 int n_lines, int n_bytes, stats *cache_stats) {
	char *end_ptr;
	unsigned long byte_addr;
	unsigned int set_bits;
	unsigned int tag_bits;
	unsigned int byte_offset_bits;
	int n_tag_bits;
	int i;


	// Compute byte address
	byte_addr = strtol(str, &end_ptr, 16);

	// Compute byte offset bits
	byte_offset_bits = (ULONG_MAX >> ((ADDRESS_SIZE) - n_bytes));

	// Compute tag bits
	tag_bits = byte_addr >> n_sets >> n_bytes;
	n_tag_bits = (ADDRESS_SIZE) - n_sets - n_bytes;

	// Get the set from byte_addr
	set_bits = byte_addr && ((ULONG_MAX >> n_tag_bits) - byte_offset_bits);

	// Find out if cache hit or miss
	for (i = 0; i < n_lines; i++) {
		if ((*lcache)->sets[set_bits]->lines[i]->valid != 0
				|| (*lcache)->sets[set_bits]->lines[i]->tag == tag_bits) {
						cache_stats->hits += 1;
						return;
		}
	}

	cache_stats->misses += 1;

}

void read_and_process(FILE *fp, cache **lcache, int sets,
	 int lines, int bytes, stats *cache_stats) {
	char buf[LINE_SIZE];
	char *ptr;

	// Grab a line
	while (fgets(buf, (LINE_SIZE), fp) != NULL) {
		ptr = strtok(buf, " ");
		printf("%s ", ptr);

		if (strncmp(ptr, "I", 1) == 0) {
			ptr = strtok(NULL, " ,");
		} else if (strncmp(ptr, "L", 1) == 0) {
			process_load(ptr, lcache, sets, lines, bytes, cache_stats);
			ptr = strtok(NULL, ",");
		} else if (strncmp(ptr, "M", 1) == 0) {
			process_modify(ptr, lcache, sets, lines, bytes, cache_stats);
			ptr = strtok(NULL, ",");
		} else if (strncmp(ptr, "S", 1) == 0) {
			process_save(ptr, lcache, sets, lines, bytes, cache_stats);
			ptr = strtok(NULL, ",");
		}
		printf("%s,", ptr);

		ptr = strtok(NULL, "\n");
		printf("%s\n", ptr);

	}

}

int main(int argc, char **argv) {
	// Cache
	cache *lcache;

	// File
	FILE *fp = NULL;
	char *fname;

	// Input
	int n_sets;
	int n_lines;
	int n_bytes;
	// int flag_help;
	// int flag_verbose;

	// Stats
	stats *cache_stats = malloc(sizeof(stats));


	// Check number of arguments
	check_arguments(argv, argc);

	// Parse command line argumments
	parse_command_line_argument(&fname, argv, &n_sets, &n_lines, &n_bytes, argc);

	// Declarations
	declarations(&lcache, n_sets, n_lines, n_bytes);

	// Open trace file
	open_trace_file(&fp, fname);

	// Read trace file
	read_and_process(fp, &lcache, n_sets, n_lines, n_bytes, cache_stats);

	// Close trace file
	fclose(fp);


	return 0;
}
