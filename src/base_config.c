#include <core.h>
#include <log.h>
#include <str.h>
#include <list.h>
#include <config.h>
#include <client.h>
#include <platform.h>
#include <strstor.h>
#include <types/event.h>
#include "base_lists.h"

#define BASECFG "base"

extern CStore *Base_ConfigStore;
extern BList Base_Operators, Base_Bans;

void Base_Config(void) {
	if(!Base_LoadList(&Base_Operators))
		Log_Error("Failed to load ops.txt.");
	if(!Base_LoadList(&Base_Bans))
		Log_Error("Failed to load bans.txt.");

	Base_ConfigStore = Config_NewStore(BASECFG);

	CEntry *ent;
	ent = Config_NewEntry(Base_ConfigStore, "time-cycle", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Enable day/night cycle");
	Config_SetDefaultBool(ent, false);

	ent = Config_NewEntry(Base_ConfigStore, "autosave-delay", CONFIG_TYPE_STR);
	Config_SetComment(ent, "Worlds autosave delay (Accepts values like 15m, 16h, 1d, anything else means autosaving is disabled)");
	Config_SetDefaultStr(ent, "15m");

	ent = Config_NewEntry(Base_ConfigStore, "tnt-deny-place", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "If enabled, TNT cannot be placed");
	Config_SetDefaultBool(ent, false);

	ent = Config_NewEntry(Base_ConfigStore, "tnt-deny-blow", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "If enabled, TNT does not explode");
	Config_SetDefaultBool(ent, true);

	ent = Config_NewEntry(Base_ConfigStore, "rcon-enabled", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Enable or disable rcon protocol");
	Config_SetDefaultBool(ent, false);

	ent = Config_NewEntry(Base_ConfigStore, "rcon-port", CONFIG_TYPE_INT16);
	Config_SetComment(ent, "Use specified port to accept rcon clients [1-65535]");
	Config_SetDefaultInt16(ent, 25575);

	ent = Config_NewEntry(Base_ConfigStore, "rcon-password", CONFIG_TYPE_STR);
	Config_SetComment(ent, "Rcon authorization password");
	Config_SetDefaultStr(ent, "");

	ent = Config_NewEntry(Base_ConfigStore, "connect-notifications", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Enable or disable player (dis-)connection notifications");
	Config_SetDefaultBool(ent, true);

	ent = Config_NewEntry(Base_ConfigStore, "heartbeat-cc-enabled", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Enable ClassiCube heartbeat");
	Config_SetDefaultBool(ent, false);

	ent = Config_NewEntry(Base_ConfigStore, "heartbeat-delay", CONFIG_TYPE_INT8);
	Config_SetComment(ent, "Heartbeat request delay [1-60]");
	Config_SetLimit(ent, 1, 60);
	Config_SetDefaultInt8(ent, 10);

	ent = Config_NewEntry(Base_ConfigStore, "heartbeat-public", CONFIG_TYPE_BOOL);
	Config_SetComment(ent, "Show server in the ClassiCube server list");
	Config_SetDefaultBool(ent, false);

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

	Config_Save(Base_ConfigStore, false);
}

cs_bool Base_IsOP(cs_str name) {
	return Base_CheckList(&Base_Operators, name);
}

cs_bool Base_AddOP(cs_str name) {
	return Base_AddList(&Base_Operators, name);
}

cs_bool Base_RemoveOP(cs_str name) {
	return Base_RemoveList(&Base_Operators, name);
}

cs_bool Base_IsBanned(cs_str name) {
	return Base_CheckList(&Base_Bans, name);
}

cs_bool Base_AddBan(cs_str name) {
	return Base_AddList(&Base_Bans, name);
}

cs_bool Base_RemoveBan(cs_str name) {
	return Base_RemoveList(&Base_Bans, name);
}

cs_bool Base_OnHandshake(onHandshakeDone *a) {
	if(Base_IsBanned(Client_GetName(a->client))) {
		Client_Kick(a->client, "You are banned!");
		return false;
	} else if(Base_IsOP(Client_GetName(a->client)))
		Client_SetOP(a->client, true);

	return true;
}

CStore *Base_GetConfig(void) {
	return Base_ConfigStore;
}

void Base_OnStop(void) {
	if(!Base_SaveList(&Base_Operators))
		Log_Error("Failed to save ops.txt.");
	if(!Base_SaveList(&Base_Bans))
		Log_Error("Failed to save bans.txt.");
}
