#ifndef READ_FILE_TO_STR_H
#define READ_FILE_TO_STR_H

#include <stddef.h>

typedef enum {
	READ_OK, // success
	READ_ERR_OPEN_FILE, // couldn't open the file
	READ_ERR_MEMORY, // couldn't allocate memory
	READ_ERR_READ // reading failure
} read_result_t;

read_result_t read_file_to_str(char **dst, size_t *len, char const *filename);
char const *read_get_status_message(read_result_t result);

#endif // READ_FILE_TO_STR_H
