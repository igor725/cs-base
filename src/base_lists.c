#include <core.h>
#include <str.h>
#include <platform.h>
#include "base_lists.h"

cs_bool Base_LoadList(BList *list) {
  list->alerted = false;
  cs_file fp = File_Open(list->filename, "r");
  if(!fp) return false;

  cs_char buffer[64];
  while(File_ReadLine(fp, buffer, 64))
    AList_AddField(&list->head, (cs_char *)String_AllocCopy(buffer));

  File_Close(fp);
  return true;
}

cs_bool Base_SaveList(BList *list) {
  if(!list->alerted) return true;
  cs_file fp = File_Open(list->filename, "w");
  if(!fp) return false;

  cs_bool success = true;
  AListField *ptr;
  List_Iter(ptr, list->head) {
    cs_str name = ptr->value.str;
    cs_size len = String_Length(ptr->value.str);
    if(File_WriteFormat(fp, "%s\n", name) != len) {
      success = false;
      break;
    }
  }

  File_Close(fp);
  return success;
}

cs_bool Base_CheckList(BList *list, cs_str name) {
  AListField *ptr;
  List_Iter(ptr, list->head) {
    if(String_CaselessCompare(ptr->value.str, name))
      return true;
  }
  return false;
}

cs_bool Base_AddList(BList *list, cs_str name) {
  if(Base_CheckList(list, name)) return true;
  list->alerted = true;
  return AList_AddField(&list->head, (cs_char *)String_AllocCopy(name)) != NULL;
}

cs_bool Base_RemoveList(BList *list, cs_str name) {
  AListField *ptr;
  List_Iter(ptr, list->head) {
    if(String_CaselessCompare(ptr->value.str, name)) {
      AList_Remove(&list->head, ptr);
      list->alerted = true;
      return true;
    }
  }
  return false;
}
