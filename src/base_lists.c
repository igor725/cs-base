#include <core.h>
#include <str.h>
#include <platform.h>
#include "base_lists.h"

cs_bool Base_LoadList(BList *list) {
	list->alerted = false;
	cs_file fp = File_Open(list->filename, "r");
	cs_char buffer[64];

	if(fp) {
		while(File_ReadLine(fp, buffer, 64) > 0)
			AList_AddField(&list->head, (cs_char *)String_AllocCopy(buffer));

		File_Close(fp);
	}

	return true;
}

static cs_bool SaveIterator(AListField *ptr, AListField **head, void *ud) {
	(void)head;
	cs_str name = AList_GetValue(ptr).str;
	return File_WriteFormat((cs_file)ud, "%s\n", name) > 0;
}

cs_bool Base_SaveList(BList *list) {
	if(!list->alerted) return true;
	cs_file fp = File_Open(list->filename, "w");
	cs_bool success = false;

	if(fp) {
		success = AList_Iter(&list->head, fp, SaveIterator);
		File_Close(fp);
	}

	return success;
}

static cs_bool ListChecker(AListField *ptr, AListField **head, void *ud) {
	(void)head;
	return !String_CaselessCompare((AList_GetValue(ptr).str), (cs_str)ud);
}

cs_bool Base_CheckList(BList *list, cs_str name) {
	return !AList_Iter(&list->head, (void *)name, ListChecker);
}

cs_bool Base_AddList(BList *list, cs_str name) {
	if(Base_CheckList(list, name)) return false;
	list->alerted = true;
	return AList_AddField(&list->head, (cs_char *)String_AllocCopy(name)) != NULL;
}

static cs_bool ListRemover(AListField *ptr, AListField **head, void *ud) {
	if(String_CaselessCompare((AList_GetValue(ptr).str), (cs_str)ud)) {
		AList_Remove(head, ptr);
		return false;
	}

	return true;
}

cs_bool Base_RemoveList(BList *list, cs_str name) {
	if(!AList_Iter(&list->head, (void *)name, ListRemover)) {
		list->alerted = true;
		return true;
	}

	return false;
}
