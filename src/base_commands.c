#include <core.h>
#include <command.h>
#include <server.h>
#include <plugin.h>
#include <lang.h>
#include <generators.h>
LGroup *Base_Lang;
cs_str *Base_OSName;

COMMAND_FUNC(Info) {
	COMMAND_PRINTF(
		Lang_Get(Base_Lang, 2),
		GIT_COMMIT_SHA,
		PLUGIN_API_NUM,
    Base_OSName
	);
}

COMMAND_FUNC(MakeOp) {
	COMMAND_SETUSAGE("/makeop <playername>");

	cs_char clientname[64];
	if(COMMAND_GETARG(clientname, 64, 0)) {
		Client *tg = Client_GetByName(clientname);
		if(tg) {
			PlayerData *pd = tg->playerData;
			cs_str name = pd->name;
			pd->isOP ^= 1;
			COMMAND_PRINTF("Player %s %s", name, pd->isOP ? "opped" : "deopped");
		} else {
			COMMAND_PRINT(Lang_Get(Lang_CmdGrp, 2));
		}
	}
	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(Uptime) {
	cs_uint64 msec, d, h, m, s, ms;
	msec = Time_GetMSec() - Server_StartTime;
	d = msec / 86400000;
	h = (msec % 86400000) / 3600000;
	m = (msec / 60000) % 60000;
	s = (msec / 1000) % 60;
	ms = msec % 1000;
	COMMAND_PRINTF(
		"Server uptime: %03d:%02d:%02d:%02d.%03d",
		d, h, m, s, ms
	);
}

COMMAND_FUNC(CFG) {
	COMMAND_SETUSAGE("/cfg <set/get/print> [key] [value]");
	cs_char subcommand[8], key[MAX_CFG_LEN], value[MAX_CFG_LEN];

	if(COMMAND_GETARG(subcommand, 8, 0)) {
		if(String_CaselessCompare(subcommand, "set")) {
			if(!COMMAND_GETARG(key, MAX_CFG_LEN, 1)) {
				COMMAND_PRINTUSAGE;
			}
			CEntry *ent = Config_GetEntry(Server_Config, key);
			if(!ent) {
				COMMAND_PRINT("This entry not found in \"server.cfg\" store.");
			}
			if(!COMMAND_GETARG(value, MAX_CFG_LEN, 2)) {
				COMMAND_PRINTUSAGE;
			}

			switch (ent->type) {
				case CFG_TINT32:
					Config_SetInt32(ent, String_ToInt(value));
					break;
				case CFG_TINT16:
					Config_SetInt16(ent, (cs_int16)String_ToInt(value));
					break;
				case CFG_TINT8:
					Config_SetInt8(ent, (cs_int8)String_ToInt(value));
					break;
				case CFG_TBOOL:
					Config_SetBool(ent, String_CaselessCompare(value, "True"));
					break;
				case CFG_TSTR:
					Config_SetStr(ent, value);
					break;
				case CFG_TINVALID:
					COMMAND_PRINT("Can't detect entry type.");
			}
			COMMAND_PRINT("Entry value changed successfully.");
		} else if(String_CaselessCompare(subcommand, "get")) {
			if(!COMMAND_GETARG(key, MAX_CFG_LEN, 1)) {
				COMMAND_PRINTUSAGE;
			}

			CEntry *ent = Config_GetEntry(Server_Config, key);
			if(ent) {
				if(!Config_ToStr(ent, value, MAX_CFG_LEN)) {
					COMMAND_PRINT("Can't detect entry type.");
				}
				COMMAND_PRINTF("%s = %s (%s)", key, value, Config_TypeName(ent->type));
			}
			COMMAND_PRINT("This entry not found in \"server.cfg\" store.");
		} else if(String_CaselessCompare(subcommand, "print")) {
			CEntry *ent = Server_Config->firstCfgEntry;
			COMMAND_APPEND("Server config entries:")

			while(ent) {
				if(Config_ToStr(ent, value, MAX_CFG_LEN)) {
					cs_str type = Config_TypeName(ent->type);
					COMMAND_APPENDF(key, MAX_CFG_LEN, "\r\n%s = %s (%s)", ent->key, value, type)
				}
				ent = ent->next;
			}

			return true;
		}
	}

	COMMAND_PRINTUSAGE;
}

#define PLUGIN_NAME \
if(!COMMAND_GETARG(name, 64, 1)) { \
	COMMAND_PRINTUSAGE; \
} \
cs_str lc = String_LastChar(name, '.'); \
if(!lc || !String_CaselessCompare(lc, "." DLIB_EXT)) { \
	String_Append(name, 64, "." DLIB_EXT); \
}

COMMAND_FUNC(Plugins) {
	COMMAND_SETUSAGE("/plugins <load/unload/print> [pluginName]");
	cs_char subcommand[8], name[64];
	Plugin *plugin;

	if(COMMAND_GETARG(subcommand, 8, 0)) {
		if(String_CaselessCompare(subcommand, "load")) {
			PLUGIN_NAME
			if(!Plugin_Get(name)) {
				if(Plugin_LoadDll(name)) {
					COMMAND_PRINTF(Lang_Get(Base_Lang, 4), name);
				} else {
					COMMAND_PRINT(Lang_Get(Base_Lang, 9));
				}
			}
			COMMAND_PRINTF(Lang_Get(Base_Lang, 6), name);
		} else if(String_CaselessCompare(subcommand, "unload")) {
			PLUGIN_NAME
			plugin = Plugin_Get(name);
			if(!plugin) {
				COMMAND_PRINTF(Lang_Get(Base_Lang, 5), name);
			}
			if(Plugin_UnloadDll(plugin)) {
				COMMAND_PRINTF(Lang_Get(Base_Lang, 7), name);
			} else {
				COMMAND_PRINTF(Lang_Get(Base_Lang, 8), name);
			}
		} else if(String_CaselessCompare(subcommand, "list")) {
			cs_int32 idx = 1;
			cs_char pluginfo[64];
			COMMAND_APPEND(Lang_Get(Base_Lang, 10));

			for(cs_int32 i = 0; i < MAX_PLUGINS; i++) {
				plugin = Plugins_List[i];
				if(plugin) {
					if(idx > 10) {
						COMMAND_APPEND(Lang_Get(Base_Lang, 12));
						break;
					}
					COMMAND_APPENDF(
						pluginfo, 64,
						Lang_Get(Base_Lang, 11), idx++,
						plugin->name, plugin->version
					);
				}
			}

			return true;
		}
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(Stop) {
	(void)ccdata;
	Server_Active = false;
	return false;
}

COMMAND_FUNC(Kick) {
	COMMAND_SETUSAGE(Lang_Get(Base_Lang, 13));

	cs_char playername[64];
	if(COMMAND_GETARG(playername, 64, 0)) {
		Client *tg = Client_GetByName(playername);
		if(tg) {
			cs_str reason = String_FromArgument(ccdata->args, 1);
			Client_Kick(tg, reason);
			COMMAND_PRINTF(Lang_Get(Base_Lang, 14), playername);
		} else {
			COMMAND_PRINTF(Lang_Get(Lang_CmdGrp, 2));
		}
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(SetModel) {
	COMMAND_SETUSAGE("/model <modelname/blockid>");

	cs_char modelname[64];
	if(COMMAND_GETARG(modelname, 64, 0)) {
		if(!Client_SetModelStr(ccdata->caller, modelname)) {
			COMMAND_PRINT("Invalid model name.");
		}
		COMMAND_PRINT("Model changed successfully.");
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(ChgWorld) {
	COMMAND_SETUSAGE("/chgworld <worldname>");

	cs_char worldname[64];
	COMMAND_ARG2WN(worldname, 0)
	World *world = World_GetByName(worldname);
	if(world) {
		if(Client_IsInWorld(ccdata->caller, world)) {
			COMMAND_PRINT("You already in this world.");
		}
		if(Client_ChangeWorld(ccdata->caller, world)) return false;
	}
	COMMAND_PRINT("World not found.");
}

COMMAND_FUNC(GenWorld) {
	COMMAND_SETUSAGE("/genworld <name> <x> <y> <z>");

	cs_char worldname[64], x[6], y[6], z[6];
	if(COMMAND_GETARG(x, 6, 1) &&
	COMMAND_GETARG(y, 6, 2) &&
	COMMAND_GETARG(z, 6, 3)) {
		cs_int16 _x = (cs_int16)String_ToInt(x),
		_y = (cs_int16)String_ToInt(y),
		_z = (cs_int16)String_ToInt(z);

		if(_x > 0 && _y > 0 && _z > 0) {
			COMMAND_ARG2WN(worldname, 0)
			World *tmp = World_Create(worldname);
			SVec vec;
			Vec_Set(vec, _x, _y, _z);
			World_SetDimensions(tmp, &vec);
			World_AllocBlockArray(tmp);
			Generator_Flat(tmp);

			if(World_Add(tmp)) {
				COMMAND_PRINTF("World \"%s\" created.", worldname);
			} else {
				World_Free(tmp);
				COMMAND_PRINT("Worlds limit exceed.");
			}
		}
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(UnlWorld) {
	COMMAND_SETUSAGE("/unlworld <worldname>");

	cs_char worldname[64];
	COMMAND_ARG2WN(worldname, 0)
	World *tmp = World_GetByName(worldname);
	if(tmp) {
		if(tmp->id == 0) {
			COMMAND_PRINT("Can't unload world with id 0.");
		}
		for(ClientID i = 0; i < MAX_CLIENTS; i++) {
			Client *c = Clients_List[i];
			if(c && Client_IsInWorld(c, tmp)) Client_ChangeWorld(c, Worlds_List[0]);
		}
		if(World_Save(tmp, true)) {
			COMMAND_PRINT("World unloaded.");
		} else {
			COMMAND_PRINT("Can't start world saving process, try again later.");
		}
	}
	COMMAND_PRINT("World not found.");
}

COMMAND_FUNC(SavWorld) {
	COMMAND_SETUSAGE("/savworld <worldname>");

	cs_char worldname[64];
	COMMAND_ARG2WN(worldname, 0);
	World *tmp = World_GetByName(worldname);
	if(tmp) {
		if(World_Save(tmp, false)) {
			COMMAND_PRINT("World saving scheduled.");
		} else {
			COMMAND_PRINT("Can't start world saving process, try again later.");
		}
	}
	COMMAND_PRINT("World not found.");
}

void Base_Commands(void) {
  COMMAND_ADD(Info, CMDF_OP);
  COMMAND_ADD(MakeOp, CMDF_OP);
  COMMAND_ADD(Uptime, CMDF_NONE);
  COMMAND_ADD(CFG, CMDF_OP);
  COMMAND_ADD(Plugins, CMDF_OP);
  COMMAND_ADD(Stop, CMDF_OP);
  COMMAND_ADD(Kick, CMDF_OP);
  COMMAND_ADD(SetModel, CMDF_OP | CMDF_CLIENT);
  COMMAND_ADD(ChgWorld, CMDF_OP | CMDF_CLIENT);
  COMMAND_ADD(GenWorld, CMDF_OP);
  COMMAND_ADD(UnlWorld, CMDF_OP);
  COMMAND_ADD(SavWorld, CMDF_OP);
}
