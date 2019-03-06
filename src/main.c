#define _POSIX_C_SOURCE 2

#include "read_file_to_str.h"
#include "process_includes.h"
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

char *copy_str(char *str) {
	size_t len = strlen(str) + 1;
	char *buf = (char*)malloc(sizeof(char) * len);
	memcpy(buf, str, len);
	return buf;
}

int main(int argc, char *argv[]) {
	int rc = EXIT_SUCCESS;

	char **paths = (char**)malloc(sizeof(char*));
	size_t paths_count = 1;

	int opt;
	while ((opt = getopt(argc, argv, "I:")) != -1) {
		switch (opt) {
			case 'I':
				paths = (char**)realloc(paths, sizeof(char*) * (++paths_count));
				paths[paths_count - 1] = copy_str(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [-Ipath] file\n", argv[0]);
				rc = EXIT_FAILURE;
				goto cleanup;
		}
	}

	paths[0] = get_directory(argv[optind]);

	char *source = NULL;
	read_result_t result = read_file_to_str(&source, NULL, argv[optind]);
	fprintf(stderr, read_get_status_message(result), argv[optind]);
	if (result == READ_OK) {
		char *processed = NULL;
		if (process_includes(&processed, NULL, source) == INCLUDE_OK) {
			printf("%s", processed);
		} else {
			fprintf(stderr, "Failed to process %s\n", argv[optind]);
			rc = EXIT_FAILURE;
		}
		free(processed);
	} else {
		rc = EXIT_FAILURE;
	}
	free(source);

cleanup:
	for (size_t i=0; i<paths_count; i++) {
		free(paths[i]);
	}

	return rc;
}
