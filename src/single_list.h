#ifndef SINGLE_LIST_H
#define SINGLE_LIST_H

#include <stdlib.h>

struct single_list_node_string_t {
	char data[33];
	struct single_list_node_string_t *next;
};

struct single_list_string_t {
	struct single_list_node_string_t *head;
	struct single_list_node_string_t *current;
	size_t length;
};

typedef struct single_list_node_string_t single_list_node_string;
typedef struct single_list_string_t single_list_string;

single_list_string *single_list_string_new();
void single_list_string_append(single_list_string *list, const char *data);
int single_list_string_contains(single_list_string *list, const char *data);

#endif
