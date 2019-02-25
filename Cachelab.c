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
#include "Cachelab.h"

#define LINE_SIZE 512

void check_arguments(char **argv, int argc) {
	if (argc < 9 || argc > 11) {
		printf("Usage: ./Cachelab [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
		exit(-1);
	}
}

void declarations(cache **lcache, int sets, int lines) {
	int i;

	// Create cache
	(*lcache) = malloc(sizeof(cache));
	
	// Initialize sets
	(*lcache)->sets = malloc(sets * sizeof(set));

	// Initialize lines
	for (i = 0; i < sets; i++) {
		(*lcache)->sets[i].lines = malloc(lines * sizeof(line));
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
				printf("-s %d\n", (*sets));
			break;
			case 'E':
				(*lines) = atoi(optarg);
				// Check
				printf("-E %d\n", (*lines));
			break;
			case 'b':
				(*bytes) = atoi(optarg);
				// Check
				printf("-b %d\n", (*bytes));
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


void read_and_process(FILE *fp) {
	char *str = NULL;
	char *ptr;

	printf("Hello\n");
	while (fgets(str, (LINE_SIZE), fp) != NULL) {
		ptr = strtok(str, " ");
		printf("%s\n", ptr);

		ptr = strtok(NULL, " ");
		printf("%s\n", ptr);

		ptr = strtok(NULL, ",");
		printf("%s\n", ptr);

		ptr = strtok(NULL, "\n");
		printf("%s\n", ptr);
 
	}

}

int main(int argc, char **argv) {
	// Cache
	cache *lcache;

	// File
	FILE *fp;
	char *fname;

	// Input
	int num_sets;
	int num_lines;
	int num_bytes;
	// int flag_help;
	// int flag_verbose;

	// Stats
	// int hits;
	// int misses;
	// int evictions;


	// Check number of arguments
	check_arguments(argv, argc);

	// Parse command line argumments
	parse_command_line_argument(&fname, argv, &num_sets, &num_lines, &num_bytes, argc);

	// Declarations
	declarations(&lcache, num_sets, num_lines);

	// Open trace file
	open_trace_file(&fp, fname);

	// Read trace file
	read_and_process(fp);

	// Close trace file
	fclose(fp);


	return 0;
}
