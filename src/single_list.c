#define  _CRT_SECURE_NO_WARNINGS

#include "single_list.h"

#include <stdlib.h>
#include <string.h>

single_list_string *single_list_string_new() {
	single_list_string *list = (single_list_string*)malloc(sizeof(single_list_string));
	list->head = NULL;
	list->current = NULL;
	list->length = 0;
	return list;
}

void single_list_string_append(single_list_string *list, const char *data) {
	if (list->head == NULL) {
		list->head = (single_list_node_string*)malloc(sizeof(single_list_node_string));
		list->current = list->head;
		strcpy(list->head->data, data);
		list->head->next = NULL;
	}
	else {
		list->current->next = (single_list_node_string*)malloc(sizeof(single_list_node_string));
		list->current = list->current->next;
		strcpy(list->current->data, data);
		list->current->next = NULL;
	}
	list->length++;
}

int single_list_string_contains(single_list_string *list, const char *data) {
	single_list_node_string *node = list->head;
	int count = 0;
	while (node != NULL) {
		if (strcmp(node->data, data) == 0) {
			return count;
		}
		node = node->next;
		count++;
	}
	return -1;
}
