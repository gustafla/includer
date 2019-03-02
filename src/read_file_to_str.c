#include "read_file_to_str.h"
#include <stdlib.h>
#include <stdio.h>

size_t read_file_to_str(char **dst, char const *filename) {
    FILE *file = fopen(filename, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		size_t len = ftell(file);
		if (len) {
			*dst = (char*)malloc(len+1);
			fseek(file, 0, SEEK_SET);

			size_t read = fread(*dst, sizeof(char), len, file);

			(*dst)[len] = '\0';

			fclose(file);

			if (read == len) {
				return len;
			} else {
				fprintf(stderr, "Failed to read file %s\n", filename);
			}
		}
	} else {
		fprintf(stderr, "Cannot open file %s\n", filename);
	}
	return 0;
}
