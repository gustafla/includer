#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500

#include "read_file_to_str.h"
#include "process_includes.h"
#include "arraylist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char *get_directory(char *path) {
	char *end = NULL, *c = path;
	while (c) {
		c = strchr(c+1, '/');
		if (c) {
			end = c;
		}
	}

	if (!end) return NULL;

	size_t len = end - path;

	char *buf = (char*)malloc(sizeof(char) * (len + 1));
	memcpy(buf, path, len);
	buf[len] = '\0';
	
	return buf;
}

#define USAGE "Usage: %s [-Ipath] file\n"

int main(int argc, char *argv[]) {
	int rc = EXIT_SUCCESS;

	// allocate paths list
	arraylist_t *paths = arraylist_init(8);
	if (!paths) return EXIT_FAILURE;

	// parse arguments
	int opt;
	while ((opt = getopt(argc, argv, "I:")) != -1) {
		switch (opt) {
			case 'I':
				arraylist_add(paths, (void*)strdup(optarg));
				break;
			default:
				fprintf(stderr, USAGE, argv[0]);
				rc = EXIT_FAILURE;
				goto cleanup;
		}
	}

	// if got the main source file as argument, process it
	if (argv[optind]) {
		// record path of the file
		char *main_path = get_directory(argv[optind]);
		if (main_path) {
			arraylist_add(paths, (void*)main_path);
		} else {
			arraylist_add(paths, (void*)strdup("."));
		}

		char *source = NULL;
		read_result_t result = read_file_to_str(&source, NULL, argv[optind]);
		fprintf(stderr, read_get_status_message(result), argv[optind]);
		if (result == READ_OK) {
			char *output = NULL;
			if (process_includes(&output, NULL, source, paths) == INCLUDE_OK) {
				printf("%s", output);
			} else {
				fprintf(stderr, "Failed to process %s\n", argv[optind]);
				rc = EXIT_FAILURE;
			}
			free(output);
		} else {
			rc = EXIT_FAILURE;
		}
		free(source);
	} else {
		fprintf(stderr, USAGE, argv[0]);
		rc = EXIT_FAILURE;
	}

cleanup:
	for (size_t i = 0; i < paths->size; i++) {
		free(paths->list[i]);
	}
	arraylist_free(paths);

	return rc;
}
