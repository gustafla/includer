#include "read_file_to_str.h"
#include "process_includes.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	for (int i=1; i<argc; i++) {
		char *sourcecode = NULL;
		size_t len = read_file_to_str(&sourcecode, argv[i]);
		if (len) {
			char *processed = NULL;
			len = process_includes(&processed, sourcecode);

			if (len) {
				printf("%s", processed);
			} else {
				fprintf(stderr, "Failed to process includes for %s\n", argv[i]);
			}
			free(processed);
		}
		free(sourcecode);
	}

	return EXIT_SUCCESS;
}
