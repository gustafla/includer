#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

size_t process_includes(char **dst, char *sourcecode);

#define KEYWORD "#include"
#define KW_LEN (sizeof(KEYWORD) - 1)

int process_local_include(
		char *q,
		int linen,
		char **lastpoint,
		char **buffer,
		size_t *bufsize) {

	int rc = EXIT_FAILURE;

	// find length of filename
	char *f = q;
	while (*(++f) != '\"') {
		if (*f == '\0') {
			fprintf(stderr, "line %d: Unexpected EOF\n", linen);
			return EXIT_FAILURE;
		} else if (*f == '\n') {
			fprintf(stderr, "line %d: Unexpected EOL\n", linen);
			return EXIT_FAILURE;
		}
	}

	// allocate and set up a filename
	size_t len = f - (q + 1);
	char *filename = (char*)calloc(sizeof(char), len + 1);
	strncpy(filename, q + 1, len);

	// read the file that needs to be included
	char *str = NULL;
	len = read_file_to_str(&str, filename);

	if (len) {
		// recursively process its includes
		fprintf(stderr, "Processing %s\n", filename);
		char *processed = NULL;
		len = process_includes(&processed, str);

		if (len) {
			fprintf(stderr, "Finished %s\n", filename);

			// append to output buffer
			*buffer = (char*)realloc(*buffer, *bufsize + len);
			memcpy(*buffer + *bufsize, processed, len);
			*bufsize += len;

			// mark last place before which did processing
			while (*f != '\n' && *f != '\0') f++;
			*lastpoint = f + 1;

			rc = EXIT_SUCCESS;
		}
		free(processed);
	}
	free(str);
	free(filename);
	return rc;
}

size_t process_includes(char **dst, char *sourcecode) {
	int linen = 1;
	bool newline = true;
	size_t bufsize = 0;
	char *lastpoint = sourcecode;

	for (char *i = sourcecode; *i; i++) {
		if (*i == '\n') {
			linen++;
			newline = true;
			continue;
		} else if (newline && *i == '#') {
			fprintf(stderr, "Preprocessor directive at line %d\n", linen);
			char compare[KW_LEN];
			strncpy(compare, i, KW_LEN);

			// only process #include
			if (strncmp(compare, KEYWORD, KW_LEN) == 0) {
				fprintf(stderr, "...which was an include\n");
				// seek whitespace until filename quotation
				char *q = i + KW_LEN;
				while (*q == ' ' || *q == '\t') q++;

				// only process local includes
				if (*q == '\"') {
					fprintf(stderr, "...which was a local include\n");

					// copy input before include directive to output
					size_t len = (size_t)(i - lastpoint);
					*dst = (char*)realloc(*dst, bufsize + len);
					memcpy(*dst + bufsize, lastpoint, len);
					bufsize += len;

					if (process_local_include(q, linen,&lastpoint, dst,
								&bufsize)) return 0;
				}
			}
		}
		newline = false;
	}

	// append trailing input to output
	size_t len = strlen(lastpoint);
	*dst = (char*)realloc(*dst, bufsize + len + 1); // +1 for null term
	memcpy(*dst+bufsize, lastpoint, len);
	bufsize += len;
	(*dst)[bufsize] = '\0';

	return bufsize;
}

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
