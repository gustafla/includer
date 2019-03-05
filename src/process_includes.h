#ifndef PROCESS_INCLUDES_H
#define PROCESS_INCLUDES_H

#include <stddef.h>

typedef enum {
	INCLUDE_OK,
	INCLUDE_ERR
} include_result_t;

include_result_t process_includes(char **dst, size_t *len, char *source);

#endif // PROCESS_INCLUDES_H
