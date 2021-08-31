#include <core.h>
#include <log.h>
#include <str.h>
#include <list.h>
#include <config.h>
#include <client.h>
#include <platform.h>
#include "base_lists.h"

extern CStore *Base_ConfigStore;
BList operators = {
  .alerted = false,
  .filename = "ops.txt",
  .head = NULL
};

BList bans = {
  .alerted = false,
  .filename = "bans.txt",
  .head = NULL
};

void Base_Config(void) {
  if(!Base_LoadList(&operators))
    Log_Error("Failed to load ops.txt.");
  if(!Base_LoadList(&bans))
    Log_Error("Failed to load bans.txt.");

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

  ent = Config_NewEntry(Base_ConfigStore, "heartbeat-cc-enabled", CFG_TBOOL);
  Config_SetComment(ent, "Enable ClassiCube heartbeat.");
  Config_SetDefaultBool(ent, false);

  ent = Config_NewEntry(Base_ConfigStore, "heartbeat-delay", CFG_TINT8);
  Config_SetComment(ent, "Heartbeat request delay. [1-60]");
  Config_SetLimit(ent, 1, 60);
  Config_SetDefaultInt8(ent, 10);

  ent = Config_NewEntry(Base_ConfigStore, "heartbeat-public", CFG_TBOOL);
  Config_SetComment(ent, "Show server in the ClassiCube server list.");
  Config_SetDefaultBool(ent, false);

  Base_ConfigStore->modified = true;
  if(!Config_Load(Base_ConfigStore)) {
    Config_PrintError(Base_ConfigStore);
    Process_Exit(1);
  }

  Config_Save(Base_ConfigStore);
}

cs_bool Base_AddOP(cs_str name) {
  return Base_AddList(&operators, name);
}

cs_bool Base_RemoveOP(cs_str name) {
  return Base_RemoveList(&operators, name);
}

cs_bool Base_AddBan(cs_str name) {
  return Base_AddList(&bans, name);
}

cs_bool Base_RemoveBan(cs_str name) {
  return Base_RemoveList(&bans, name);
}


void Base_OnSpawn(Client *cl) {
  if(cl->playerData->firstSpawn && !cl->playerData->isOP)
    cl->playerData->isOP = Base_CheckList(&operators, Client_GetName(cl));
}

void Base_OnHandshake(Client *cl) {
  if(Base_CheckList(&bans, Client_GetName(cl)))
    Client_Kick(cl, "You are banned!");
}

void Base_OnStop(void) {
  if(!Base_SaveList(&operators))
    Log_Error("Failed to save ops.txt.");
  if(!Base_SaveList(&bans))
    Log_Error("Failed to save bans.txt.");
}
