#include "process_includes.h"
#include "read_file_to_str.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool cmp_is_not_quotation(char c) {
	return c != '\"';
}

int seek_line(char **c, bool(*cmp)(char)) {
	while (cmp(**c)) {
		if (**c == '\0' || **c == '\n') {
			return EXIT_FAILURE;
		}
		(*c)++;
	}
	return EXIT_SUCCESS;
}

int local_include(
		char *q,
		char **lastpoint,
		char **buffer,
		size_t *bufsize) {

	int rc = EXIT_FAILURE;

	// find end of filename
	char *f = q + 1;
	seek_line(&f, cmp_is_not_quotation);

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

#define KEYWORD "#include"
#define KW_LEN sizeof(KEYWORD)

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
			strncpy(compare, i, KW_LEN-1);

			// only process #include
			if (strncmp(compare, KEYWORD, KW_LEN-1) == 0) {
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

					if (local_include(q, &lastpoint, dst, &bufsize)) {
						fprintf(stderr, "Can't process line %d\n", linen);
						return 0;
					}
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
