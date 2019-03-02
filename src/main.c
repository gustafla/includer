#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

size_t read_file_to_str(char **dst, char const *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    if (!len) {
        return 0; // Don't read and malloc an empty file
    }
    *dst = (char*)malloc(len+1);
    fseek(file, 0, SEEK_SET);

    size_t read = fread(*dst, sizeof(char), len, file);
    fclose(file);

    (*dst)[len] = '\0';

    if (read != len) {
        free(*dst);
        fprintf(stderr, "Failed to read file %s\n", filename);
        return 0;
    }
    return len;
}

#define KEYWORD "#include"
#define KW_LEN (sizeof(KEYWORD) - 1)

size_t process_includes(char **dst, char const *sourcecode) {
	int linen = 1;
	bool newline = true;
	char *buffer = NULL;
	size_t bufsize = 0;
	char const *lastpoint = sourcecode;

	for (char const *i = sourcecode; *i; i++) {
		if (*i == '\n') {
			linen++;
			newline = true;
			continue;
		} else if (newline && *i == '#') {
			fprintf(stderr, "Found some preprocessor directive at line %d\n", linen);
			char compare[KW_LEN];
			strncpy(compare, i, KW_LEN);

			// only process #include
			if (strncmp(compare, KEYWORD, KW_LEN) == 0) {
				fprintf(stderr, "...which was an include\n");
				// seek whitespace until filename quotation
				char const *q = i + KW_LEN;
				while (*q == ' ' || *q == '\t') q++;

				// only process local includes
				if (*q == '\"') {
					fprintf(stderr, "...which was a local include\n");
					// copy input before include directive to output
					size_t len = i - lastpoint;
					buffer = (char*)realloc(buffer, bufsize + len);
					memcpy(buffer + bufsize, lastpoint, len);
					bufsize += len;

					// find length of filename
					char const *f = q;
					while (*(++f) != '\"') {
						if (*f == '\0') {
							fprintf(stderr, "%d: Unexpected EOF\n", linen);
							goto fail;
						} else if (*f == '\n') {
							fprintf(stderr, "%d: Unexpected EOL\n", linen);
							goto fail;
						}
					}

					// allocate and set up a filename
					size_t const fn_len = f - (q + 1);
					char *filename = (char*)calloc(sizeof(char), fn_len + 1);
					strncpy(filename, q + 1, fn_len);

					// read the file that needs to be included
					char *str;
					size_t str_len = read_file_to_str(&str, filename);
					if (!str_len) goto fail;

					// append to output buffer
					buffer = (char*)realloc(buffer, bufsize + str_len);
					memcpy(buffer + bufsize, str, str_len);
					bufsize += str_len;

					// mark last place before which did processing
					while (*f != '\n' && *f != '\0') f++;
					lastpoint = f + 1;

					free(filename);
					free(str);
				}
			}
		}
		newline = false;
	}

	// append trailing input to output
	size_t len = strlen(lastpoint);
	buffer = (char*)realloc(buffer, bufsize + len + 1); // +1 for null term
	memcpy(buffer+bufsize, lastpoint, len);
	bufsize += len;
	buffer[bufsize] = '\0';

	*dst = buffer;
	return bufsize;

fail:
	free(buffer);
	return 0;
}

int main(int argc, char *argv[]) {
	for (int i=1; i<argc; i++) {
		char *sourcecode;
		size_t len = read_file_to_str(&sourcecode, argv[i]);
		if (!len) continue;

		char *processed;
		len = process_includes(&processed, sourcecode);

		if (len) {
			printf("%s", processed);
			free(processed);
		} else {
			fprintf(stderr, "Failed to process includes for %s\n", argv[i]);
		}

		free(sourcecode);
	}

	return EXIT_SUCCESS;
}
