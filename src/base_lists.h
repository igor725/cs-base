#ifndef BASE_LISTS_H
#define BASE_LISTS_H
#include <core.h>
#include <types/list.h>

typedef struct {
	cs_bool alerted;
	cs_str filename;
	AListField *head;
} BList;

cs_bool Base_LoadList(BList *list);
cs_bool Base_SaveList(BList *list);
cs_bool Base_CheckList(BList *list, cs_str name);
cs_bool Base_AddList(BList *list, cs_str name);
cs_bool Base_RemoveList(BList *list, cs_str name);
void Base_EmptyList(BList *list);
#endif
