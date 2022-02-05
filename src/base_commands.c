#include <core.h>
#include <config.h>
#include <command.h>
#include <server.h>
#include <plugin.h>
#include <generators.h>

extern cs_char Base_OSName[64];
cs_bool Base_AddOP(cs_str name);
cs_bool Base_RemoveOP(cs_str name);
cs_bool Base_AddBan(cs_str name);
cs_bool Base_RemoveBan(cs_str name);

COMMAND_FUNC(Info) {
	COMMAND_PRINTF(
		"CServer/%s with PluginAPI v%03d runned on %s",
		Server_Version, PLUGIN_API_NUM, Base_OSName
	);
}

COMMAND_FUNC(MakeOp) {
	COMMAND_SETUSAGE("/makeop <playername>");
	cs_char clientname[64];
	if(COMMAND_GETARG(clientname, 64, 0)) {
		if(Base_AddOP(clientname)) {
			Client *client = Client_GetByName(clientname);
			if(client) Client_SetOP(client, true);
			COMMAND_PRINTF("Player %s added to the OPs list.", clientname);
		} else {
			COMMAND_PRINT("Failed to add player to OPs list.");
		}
	}
	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(DeOp) {
	COMMAND_SETUSAGE("/deop <playername>");
	cs_char clientname[64];
	if(COMMAND_GETARG(clientname, 64, 0)) {
		if(Base_RemoveOP(clientname)) {
			Client *client = Client_GetByName(clientname);
			if(client) Client_SetOP(client, false);
			COMMAND_PRINTF("Player %s removed from the OPs list.", clientname);
		} else {
			COMMAND_PRINT("Failed to remove player from OPs list.");
		}
	}
	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(Ban) {
	COMMAND_SETUSAGE("/ban <playername>");
	cs_char clientname[64];
	if(COMMAND_GETARG(clientname, 64, 0)) {
		if(Base_AddBan(clientname)) {
			Client *client = Client_GetByName(clientname);
			if(client) Client_Kick(client, "You are banned!");
			COMMAND_PRINTF("Player %s banned.", clientname);
		} else {
			COMMAND_PRINT("Cannot add this player to the banlist.");
		}
	}
	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(UnBan) {
	COMMAND_SETUSAGE("/unban <playername>");
	cs_char clientname[64];
	if(COMMAND_GETARG(clientname, 64, 0)) {
		if(Base_RemoveBan(clientname)) {
			COMMAND_PRINTF("Player %s unbanned.", clientname);
		} else {
			COMMAND_PRINT("Failed to remove player from banlist.");
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
	cs_char subcommand[8], key[CFG_MAX_LEN], value[CFG_MAX_LEN];

	if(COMMAND_GETARG(subcommand, 8, 0)) {
		if(String_CaselessCompare(subcommand, "set")) {
			if(!COMMAND_GETARG(key, CFG_MAX_LEN, 1)) {
				COMMAND_PRINTUSAGE;
			}
			CEntry *ent = Config_GetEntry(Server_Config, key);
			if(!ent) {
				COMMAND_PRINT("This entry not found in \"server.cfg\" store.");
			}
			if(!COMMAND_GETARG(value, CFG_MAX_LEN, 2)) {
				COMMAND_PRINTUSAGE;
			}

			switch (Config_GetEntryType(ent)) {
				case CONFIG_TYPE_INT32:
					Config_SetInt32(ent, String_ToInt(value));
					break;
				case CONFIG_TYPE_INT16:
					Config_SetInt16(ent, (cs_int16)String_ToInt(value));
					break;
				case CONFIG_TYPE_INT8:
					Config_SetInt8(ent, (cs_int8)String_ToInt(value));
					break;
				case CONFIG_TYPE_BOOL:
					Config_SetBool(ent, String_CaselessCompare(value, "True"));
					break;
				case CONFIG_TYPE_STR:
					Config_SetStr(ent, value);
					break;
				default:
					COMMAND_PRINT("Can't detect entry type.");
			}
			COMMAND_PRINT("Entry value changed successfully.");
		} else if(String_CaselessCompare(subcommand, "get")) {
			if(!COMMAND_GETARG(key, CFG_MAX_LEN, 1)) {
				COMMAND_PRINTUSAGE;
			}

			CEntry *ent = Config_GetEntry(Server_Config, key);
			if(ent) {
				if(Config_ToStr(ent, value, CFG_MAX_LEN)) {
					COMMAND_PRINTF("%s = %s (%s)", key, value, Config_GetEntryTypeName(ent));
				} else {
					COMMAND_PRINT("Config_ToStr() == 0??");
				}
			}
			COMMAND_PRINT("This entry not found in \"server.cfg\" store.");
		} else if(String_CaselessCompare(subcommand, "print")) {
			CEntry *ent = Server_Config->firstCfgEntry;
			COMMAND_APPEND("Server config entries:")

			while(ent) {
				if(Config_ToStr(ent, value, CFG_MAX_LEN)) {
					COMMAND_APPENDF(key, CFG_MAX_LEN, "\r\n%s = %s (%s)",
					Config_GetEntryKey(ent), value, Config_GetEntryTypeName(ent));
				} else {
					COMMAND_APPENDF(key, CFG_MAX_LEN, "\r\n%s - Config_ToStr() == 0??");
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
	COMMAND_SETUSAGE("/plugins <load/unload/list> [pluginName]");
	cs_char subcommand[8], name[64];
	Plugin *plugin;

	if(COMMAND_GETARG(subcommand, 8, 0)) {
		if(String_CaselessCompare(subcommand, "load")) {
			PLUGIN_NAME
			if(!Plugin_Get(name)) {
				if(Plugin_LoadDll(name)) {
					COMMAND_PRINTF("Plugin \"%s\" loaded.", name);
				} else {
					COMMAND_PRINT("Something went wrong.");
				}
			}
			COMMAND_PRINTF("Plugin \"%s\" is already loaded.", name);
		} else if(String_CaselessCompare(subcommand, "unload")) {
			PLUGIN_NAME
			plugin = Plugin_Get(name);
			if(!plugin) {
				COMMAND_PRINTF("Plugin \"%s\" not loaded.", name);
			}
			if(Plugin_UnloadDll(plugin, false)) {
				COMMAND_PRINTF("Plugin \"%s\" successfully unloaded.", name);
			} else {
				COMMAND_PRINTF("Plugin \"%s\" cannot be unloaded.", name);
			}
		} else if(String_CaselessCompare(subcommand, "list")) {
			cs_int32 idx = 1;
			cs_char pluginfo[64];
			COMMAND_APPEND("Loaded plugins list:");

			for(cs_int32 i = 0; i < MAX_PLUGINS; i++) {
				plugin = Plugins_List[i];
				if(plugin) {
					COMMAND_APPENDF(
						pluginfo, 64,
						"\r\n  %d.%s v%d", idx++,
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
	COMMAND_SETUSAGE("/kick <player> [reason]");

	cs_char playername[64];
	if(COMMAND_GETARG(playername, 64, 0)) {
		Client *tg = Client_GetByName(playername);
		if(tg) {
			cs_str reason = String_FromArgument(ccdata->args, 1);
			Client_Kick(tg, reason);
			COMMAND_PRINTF("Player %s kicked.", playername);
		} else {
			COMMAND_PRINTF("Player not found.");
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
		Client_Update(ccdata->caller);
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
	COMMAND_SETUSAGE("/genworld <name> <x> <y> <z> [generator]");

	cs_char worldname[64], genname[64], x[6], y[6], z[6];
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
			cs_bool success;
			if(COMMAND_GETARG(genname, 64, 4))
				success = Generators_Use(tmp, genname, NULL);
			else
				success = Generators_Use(tmp, "flat", NULL);

			if(success) {
				World_Add(tmp);
				COMMAND_PRINTF("World \"%s\" created.", worldname);
			} else {
				World_Free(tmp);
				COMMAND_PRINT("Worlds creation error.");
			}
		}
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(UnlWorld) {
	COMMAND_SETUSAGE("/unlworld <worldname>");

	cs_char worldname[64];
	COMMAND_ARG2WN(worldname, 0)
	World *tmp = World_GetByName(worldname),
	*mainw = (World *)AList_GetValue(World_Head).ptr;
	if(tmp) {
		if(mainw == tmp) {
			COMMAND_PRINT("Can't unload main world.");
		}
		for(ClientID i = 0; i < MAX_CLIENTS; i++) {
			Client *client = Clients_List[i];
			if(client && Client_IsInWorld(client, tmp))
				Client_ChangeWorld(client, mainw);
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
	COMMAND_ADD(Info, CMDF_NONE, "Prints server software info");
	COMMAND_ADD(MakeOp, CMDF_OP, "Grant operator status to a player");
	COMMAND_ADD(DeOp, CMDF_OP, "Revokes operator status from a player");
	COMMAND_ADD(Ban, CMDF_OP, "Adds player to banlist");
	COMMAND_ADD(UnBan, CMDF_OP, "Removes player from banlist");
	COMMAND_ADD(Uptime, CMDF_NONE, "Prints server uptime");
	COMMAND_ADD(CFG, CMDF_OP, "Server config manager");
	COMMAND_ADD(Plugins, CMDF_OP, "Server plugin manager");
	COMMAND_ADD(Stop, CMDF_OP, "Stops a server");
	COMMAND_ADD(Kick, CMDF_OP, "Kicks a player off a server");
	COMMAND_ADD(SetModel, CMDF_OP | CMDF_CLIENT, "Sets player model");
	COMMAND_ADD(ChgWorld, CMDF_OP | CMDF_CLIENT, "Teleports you to another world");
	COMMAND_ADD(GenWorld, CMDF_OP, "Generates new world");
	COMMAND_ADD(UnlWorld, CMDF_OP, "Unloads specified world");
	COMMAND_ADD(SavWorld, CMDF_OP, "Forces world save process");
}
