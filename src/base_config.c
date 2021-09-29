#include <core.h>
#include <log.h>
#include <str.h>
#include <list.h>
#include <config.h>
#include <client.h>
#include <platform.h>
#include <strstor.h>
#include "base_lists.h"

#define BASECFG "base.cfg"

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

	Base_ConfigStore = Config_NewStore(BASECFG);
	CEntry *ent;

	ent = Config_NewEntry(Base_ConfigStore, "rcon-enabled", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Enable or disable rcon protocol.");
	Config_SetDefaultBool(ent, false);

	ent = Config_NewEntry(Base_ConfigStore, "rcon-port", CONFIG_TYPE_INT16);
	Config_SetComment(ent, "Use specified port to accept rcon clients. [1-65535]");
	Config_SetDefaultInt16(ent, 25575);

	ent = Config_NewEntry(Base_ConfigStore, "rcon-password", CONFIG_TYPE_STR);
	Config_SetComment(ent, "Rcon authorization password.");
	Config_SetDefaultStr(ent, "");

	ent = Config_NewEntry(Base_ConfigStore, "connect-notifications", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Enable or disable player (dis-)connection notifications.");
	Config_SetDefaultBool(ent, true);

	ent = Config_NewEntry(Base_ConfigStore, "heartbeat-cc-enabled", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Enable ClassiCube heartbeat.");
	Config_SetDefaultBool(ent, false);

	ent = Config_NewEntry(Base_ConfigStore, "heartbeat-delay", CONFIG_TYPE_INT8);
	Config_SetComment(ent, "Heartbeat request delay. [1-60]");
	Config_SetLimit(ent, 1, 60);
	Config_SetDefaultInt8(ent, 10);

	ent = Config_NewEntry(Base_ConfigStore, "heartbeat-public", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Show server in the ClassiCube server list.");
	Config_SetDefaultBool(ent, false);

	Base_ConfigStore->modified = true;
	if(!Config_Load(Base_ConfigStore)) {
		cs_int32 line = 0;
		ECExtra extra = CONFIG_EXTRA_NOINFO;
		ECError code = Config_PopError(Base_ConfigStore, &extra, &line);
		if(extra == CONFIG_EXTRA_IO_LINEASERROR) {
			if(line != ENOENT)
				Log_Error(Sstor_Get("SV_CFG_ERR2"), "open", BASECFG, Config_ErrorToString(code), extra);
		} else {
			cs_str scode = Config_ErrorToString(code), 
			sextra = Config_ExtraToString(extra);
			if(line > 0)
				Log_Error(Sstor_Get("SV_CFGL_ERR"), line, BASECFG, scode, sextra);
			else
				Log_Error(Sstor_Get("SV_CFG_ERR"), "parse", BASECFG, scode, sextra);
		}

		Config_ResetToDefault(Base_ConfigStore);
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


void Base_OnSpawn(Client *client) {
	if(Client_IsFirstSpawn(client) && !Client_IsOP(client))
		Client_SetOP(client, Base_CheckList(&operators, Client_GetName(client)));
}

void Base_OnHandshake(Client *client) {
	if(Base_CheckList(&bans, Client_GetName(client)))
		Client_Kick(client, "You are banned!");
}

void Base_OnStop(void) {
	if(!Base_SaveList(&operators))
		Log_Error("Failed to save ops.txt.");
	if(!Base_SaveList(&bans))
		Log_Error("Failed to save bans.txt.");
}
