#include "read_file_to_str.h"
#include "process_includes.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int rc = EXIT_SUCCESS;

	for (int i=1; i<argc; i++) {
		char *source = NULL;
		read_result_t result = read_file_to_str(&source, NULL, argv[i]);
		fprintf(stderr, read_get_status_message(result), argv[i]);
		if (result == READ_OK) {
			char *processed = NULL;
			if (process_includes(&processed, NULL, source) == INCLUDE_OK) {
				printf("%s", processed);
			} else {
				fprintf(stderr, "Failed to process %s\n", argv[i]);
				rc = EXIT_FAILURE;
			}
			free(processed);
		} else {
			rc = EXIT_FAILURE;
		}
		free(source);
	}

	return rc;
}
