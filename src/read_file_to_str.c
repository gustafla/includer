#include "read_file_to_str.h"
#include <stdio.h>
#include <stdlib.h>

#define BLOCKSIZE 4096

/// Allocates **dst and reads file contents to it, null terminating it.
/// Returns EXIT_FAILURE on error, and otherwise EXIT_SUCCESS (0).
read_result_t read_file_to_str(char **dst, size_t *len, char const *filename) {
	read_result_t rc = READ_OK;

	// allow for NULL len parameter
	size_t internal_len = 0;
	if (!len) len = &internal_len;

    FILE *file = fopen(filename, "rb");
	if (file) {
		for (int i=1; !feof(file); i++) {
			// allocate memory for next read, first time add one byte for null
			*dst = (char*)realloc(*dst, *len + BLOCKSIZE * i + (i == 1));
			if (!*dst) {
				rc = READ_ERR_MEMORY;
				goto cleanup;
			}

			// read from file to buffer
			*len += fread(*dst + *len, sizeof(char), BLOCKSIZE * i, file);
			if (ferror(file)) {
				rc = READ_ERR_READ;
				goto cleanup;
			}
		}

		// null terminate
		(*dst)[*len] = '\0';
	} else {
		rc = READ_ERR_OPEN_FILE;
	}

cleanup:
	fclose(file);
	return rc;
}

char const *read_get_status_message(read_result_t result) {
	switch (result) {
		case READ_OK: return "File %s read\n";
		case READ_ERR_OPEN_FILE: return "Failed to open file %s\n";
		case READ_ERR_MEMORY: return "Failed to allocate a buffer for %s\n";
		case READ_ERR_READ: return "An error occured while reading file %s\n";
	}
	return "Reading file %s failed for an unknown reason\n";
}
