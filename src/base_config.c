#include <core.h>
#include <log.h>
#include <str.h>
#include <list.h>
#include <config.h>
#include <client.h>
#include <platform.h>
CStore *Base_ConfigStore;
AListField *Base_OPsList;
cs_bool Base_OPsListAlerted = false;

static cs_bool LoadOPsList(cs_str filename) {
  cs_file fp = File_Open(filename, "r");
  if(!fp) return false;

  cs_char buffer[64];
  while(File_ReadLine(fp, buffer, 64))
    AList_AddField(&Base_OPsList, (cs_char *)String_AllocCopy(buffer));

  File_Close(fp);
  return true;
}

static cs_bool SaveOPsList(cs_str filename) {
  cs_file fp = File_Open(filename, "w");
  if(!fp) return false;

  cs_bool success = true;
  AListField *ptr;
  List_Iter(ptr, Base_OPsList) {
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

static cs_bool checkOP(cs_str name) {
  AListField *ptr;
  List_Iter(ptr, Base_OPsList) {
    if(String_CaselessCompare(ptr->value.str, name))
      return true;
  }
  return false;
}

cs_bool Base_AddOP(cs_str name) {
  if(checkOP(name)) return true;
  Base_OPsListAlerted = true;
  return AList_AddField(&Base_OPsList, (cs_char *)String_AllocCopy(name)) != NULL;
}

cs_bool Base_RemoveOP(cs_str name) {
  AListField *ptr;
  List_Iter(ptr, Base_OPsList) {
    if(String_CaselessCompare(ptr->value.str, name)) {
      AList_Remove(&Base_OPsList, ptr);
      Base_OPsListAlerted = true;
      return true;
    }
  }
  return false;
}

void Base_Config(void) {
  if(!LoadOPsList("ops.txt"))
    Log_Error("Failed to load ops.txt.");
  Base_ConfigStore = Config_NewStore("base.cfg");
  CEntry *ent;

  ent = Config_NewEntry(Base_ConfigStore, "rcon-enabled", CFG_TBOOL);
  Config_SetComment(ent, "Enable or disable rcon protocol.");
  Config_SetDefaultBool(ent, false);

  ent = Config_NewEntry(Base_ConfigStore, "rcon-port", CFG_TINT16);
  Config_SetComment(ent, "Use specified port to accept rcon clients. [1-65535]");
  Config_SetDefaultInt16(ent, 25575);

  ent = Config_NewEntry(Base_ConfigStore, "rcon-password", CFG_TSTR);
  Config_SetComment(ent, "Rcon authorization password.");
  Config_SetDefaultStr(ent, "");

  ent = Config_NewEntry(Base_ConfigStore, "connect-notifications", CFG_TBOOL);
  Config_SetComment(ent, "Enable or disable player (dis-)connection notifications.");
  Config_SetDefaultBool(ent, true);

  Base_ConfigStore->modified = true;
  if(!Config_Load(Base_ConfigStore)) {
    Config_PrintError(Base_ConfigStore);
    Process_Exit(1);
  }

  Config_Save(Base_ConfigStore);
}

void Base_OnSpawn(Client *cl) {
  if(cl->playerData->firstSpawn)
    cl->playerData->isOP = checkOP(Client_GetName(cl));
}

void Base_OnStop(void) {
  if(Base_OPsListAlerted)
    if(!SaveOPsList("ops.txt"))
      Log_Error("Failed to save ops.txt.");
}
