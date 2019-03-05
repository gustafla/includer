#include "process_includes.h"
#include "read_file_to_str.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "uthash.h"

#define MACRO_NAME_LEN 64

struct name {
	char name[MACRO_NAME_LEN];
	UT_hash_handle hh;
} *macro_name, *name_tmp, *macro_names;

bool cmp_is_not_quotation(char c) {
	return c != '\"';
}

bool cmp_is_alnum_or_underscore(char c) {
	return isalnum(c) || c == '_';
}

bool cmp_is_not_eol(char c) {
	return c != '\n';
}

bool cmp_is_space_or_tab(char c) {
	return c == ' ' || c == '\t';
}

include_result_t seek_line(char **c, bool(*cmp)(char)) {
	while (cmp(**c)) {
		if (**c == '\0' || **c == '\n') {
			return INCLUDE_ERR;
		}
		(*c)++;
	}
	return INCLUDE_OK;
}

void append(char **buffer, char *source, size_t *bufsize, size_t len) {
	*buffer = (char*)realloc(*buffer, *bufsize + len);
	memcpy(*buffer + *bufsize, source, len);
	*bufsize += len;
}

include_result_t process(char **dst, size_t *len, char *source);

include_result_t local_include(
		char *q,
		char **lastpoint,
		char **buffer,
		size_t *bufsize) {

	include_result_t rc = INCLUDE_ERR;

	// find end of filename
	char *f = q + 1;
	if (seek_line(&f, cmp_is_not_quotation) == INCLUDE_ERR) {
		return INCLUDE_ERR;
	}

	// allocate and set up a filename buffer
	size_t filename_len = f - (q + 1);
	char *filename = (char*)calloc(sizeof(char), filename_len + 1);
	strncpy(filename, q + 1, filename_len);

	// read the file that needs to be included
	char *str = NULL;
	read_result_t result_read = read_file_to_str(&str, NULL, filename);
	fprintf(stderr, read_get_status_message(result_read), filename);

	if (result_read == READ_OK) {
		// recursively process its includes
		fprintf(stderr, "Processing %s\n", filename);
		char *processed = NULL;
		size_t processed_len = 0;
		if (process(&processed, &processed_len, str) == INCLUDE_OK) {
			fprintf(stderr, "Finished %s\n", filename);

			// append to output buffer
			append(buffer, processed, bufsize, processed_len);

			// mark last place before which did processing
			while (*f != '\n' && *f != '\0') f++;
			*lastpoint = f + 1;

			rc = INCLUDE_OK;
		}
		free(processed);
	}
	free(str);
	free(filename);
	return rc;
}

size_t min(size_t a, size_t b) {
	if (a < b) {
		return a;
	}
	return b;
}

include_result_t get_macro_name(char *dst, char *c) {
	// seek for macro name start
	if (seek_line(&c, cmp_is_space_or_tab) == INCLUDE_ERR) {
		return INCLUDE_ERR;
	}
	char *macro = c;

	// seek for macro name end
	if (seek_line(&c, cmp_is_alnum_or_underscore) == INCLUDE_ERR) {
		return INCLUDE_ERR;
	}
	size_t macro_len = min(c - macro, MACRO_NAME_LEN - 1);

	// write to output and terminate
	memcpy(dst, macro, macro_len);
	dst[macro_len] = '\0';

	return INCLUDE_OK;
}

#define KW_INCLUDE "#include"
#define KW_INCLUDE_LEN (sizeof(KW_INCLUDE) - 1)
#define KW_DEFINE "#define"
#define KW_DEFINE_LEN (sizeof(KW_DEFINE) - 1)
#define KW_ENDIF "#endif"
#define KW_ENDIF_LEN (sizeof(KW_ENDIF) - 1)
#define KW_IF "#if"
#define KW_IF_LEN (sizeof(KW_IF) - 1)
#define NDEF "ndef"
#define NDEF_LEN (sizeof(NDEF) - 1)

include_result_t process(char **dst, size_t *len, char *source) {
	int linen = 1, skip = 0;
	bool newline = true;
	char *lastpoint = source;

	// allow for NULL len parameter
	size_t local_len = 0;
	if (!len) len = &local_len;

	for (char *i = source; *i; i++) {
		if (*i == '\n') {
			linen++;
			newline = true;
			continue;
		} else if (newline && *i == '#') {
			fprintf(stderr, "Preprocessor directive at line %d\n", linen);
			if (strncmp(i, KW_IF, KW_IF_LEN) == 0) {
				fprintf(stderr, "...which was an if\n");

				if (strncmp(i + KW_IF_LEN, NDEF, NDEF_LEN) == 0) {
					fprintf(stderr, "...of type ifndef\n");

					// initialize a name compare buffer
					char compare[MACRO_NAME_LEN];
					char *c = i + KW_IF_LEN + NDEF_LEN + 1;
					if (get_macro_name(compare, c) == INCLUDE_ERR) {
						fprintf(stderr, "Can't make sense of line %d\n", linen);
						return INCLUDE_ERR;
					}

					// check if defined
					HASH_FIND_STR(macro_names, compare, macro_name);
					if (macro_name) {
						append(dst, lastpoint, len, (size_t)(i - lastpoint));
						skip++;
					}
				} else {
					if (skip) skip++;
				}
			} else if (strncmp(i, KW_ENDIF, KW_ENDIF_LEN) == 0) {
				fprintf(stderr, "...which was an endif\n");

				if (skip) {
					skip--;
					if (!skip) {
						if (seek_line(&i, cmp_is_not_eol) == INCLUDE_ERR) {
							fprintf(stderr, "Line %d ended early\n", linen);
							return INCLUDE_ERR;
						}
						lastpoint = i + 1;
					}
				}
			} else if (!skip && strncmp(i, KW_DEFINE, KW_DEFINE_LEN) == 0) {
				fprintf(stderr, "...which was a define\n");
	
				macro_name = (struct name*)malloc(sizeof(struct name));
				char *c = i + KW_DEFINE_LEN + 1;
				if (get_macro_name(macro_name->name, c) == INCLUDE_ERR) {
					free(macro_name);
					fprintf(stderr, "Can't get name from line %d\n", linen);
					return INCLUDE_ERR;
				}
				HASH_ADD_STR(macro_names, name, macro_name);
			} else if (!skip && strncmp(i, KW_INCLUDE, KW_INCLUDE_LEN) == 0) {
				fprintf(stderr, "...which was an include\n");

				// seek whitespace until filename quotation
				char *q = i + sizeof(KW_INCLUDE) - 1;
				while (*q == ' ' || *q == '\t') q++;

				// only process local includes
				if (*q == '\"') {
					fprintf(stderr, "...which was a local include\n");

					// copy input before include directive to output
					append(dst, lastpoint, len, (size_t)(i - lastpoint));

					if (local_include(q, &lastpoint, dst, len) == INCLUDE_ERR) {
						fprintf(stderr, "Include at line %d failed\n", linen);
						return INCLUDE_ERR;
					}
				}
			}
		}
		newline = false;
	}

	// append trailing input to output and terminate
	size_t len_suffix = strlen(lastpoint);
	*dst = (char*)realloc(*dst, *len + len_suffix + 1);
	memcpy(*dst + *len, lastpoint, len_suffix);
	*len += len_suffix;
	(*dst)[*len] = '\0';

	return INCLUDE_OK;
}

include_result_t process_includes(char **dst, size_t *len, char *source) {
	macro_names = NULL;

	include_result_t result = process(dst, len, source);

	HASH_ITER(hh, macro_names, macro_name, name_tmp) {
		HASH_DEL(macro_names, macro_name);
		free(macro_name);
	}

	return result;
}
