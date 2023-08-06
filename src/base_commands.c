#include <core.h>
#include <list.h>
#include <client.h>
#include <command.h>
#include <server.h>
#include <plugin.h>
#include <pager.h>
#include <generators.h>
#include <world.h>

extern cs_char Base_OSName[64];
cs_bool Base_AddOP(cs_str name);
cs_bool Base_RemoveOP(cs_str name);
cs_bool Base_AddBan(cs_str name);
cs_bool Base_RemoveBan(cs_str name);

COMMAND_FUNC(Info) {
	ServerInfo si;
	if(Server_GetInfo(&si, sizeof(si))) {
		COMMAND_PRINTF(
			"&aInstallation info:\r\n"
			"  &1OS&f: %s\r\n"
			"  &9Software&f: %s/%s (%s)\r\n"
			"  &bBase plugin version&f: %03d (%s)",
			Base_OSName,
			si.coreName, si.coreGitTag,
			(si.coreFlags & SERVERINFO_FLAG_DEBUG) ? "DBG" : "REL",
			Plugin_Version, GIT_COMMIT_TAG
		);
	}

	COMMAND_PRINT("Failed to get the server info");
}

COMMAND_FUNC(MakeOp) {
	COMMAND_SETUSAGE("/makeop <playername>");
	cs_char clientname[64];
	if(COMMAND_GETARG(clientname, 64, 0)) {
		if(Base_AddOP(clientname)) {
			Client *client = Client_GetByName(clientname);
			if(client) Client_SetOP(client, true);
			COMMAND_PRINTF("Player %s added to the OPs list.", clientname);
		}

		COMMAND_PRINT("Failed to add player to OPs list.");
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
		}

		COMMAND_PRINT("Failed to remove player from OPs list.");
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
		}

		COMMAND_PRINT("Cannot add this player to the banlist.");
	}
	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(UnBan) {
	COMMAND_SETUSAGE("/unban <playername>");
	cs_char clientname[64];
	if(COMMAND_GETARG(clientname, 64, 0)) {
		if(Base_RemoveBan(clientname))
			COMMAND_PRINTF("Player %s unbanned.", clientname);

		COMMAND_PRINT("Failed to remove player from banlist.");
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(Uptime) {
	cs_uint64 diff = Time_GetMSec() - Server_StartTime;
	cs_uint32 d = (diff / (1000 * 60 * 60 * 24)) % 365,
	h = (diff / (1000 * 60 * 60)) % 24,
	m = (diff / (1000 * 60)) % 60,
	s = (diff / 1000) % 60,
	ms = diff % 1000;

	COMMAND_PRINTF(
		"Server uptime: &b%03u:%02u:%02u:%02u.%03u",
		d, h, m, s, ms
	);
}

COMMAND_FUNC(Kick) {
	COMMAND_SETUSAGE("/kick <player> [reason]");

	cs_char playername[64];
	if(COMMAND_GETARG(playername, 64, 0)) {
		Client *tg = Client_GetByName(playername);
		if(tg) {
			Client_Kick(tg, String_FromArgument(ccdata->args, 1));
			COMMAND_PRINTF("Player %s kicked.", playername);
		}

		COMMAND_PRINT("Player not found.");
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(SetModel) {
	COMMAND_SETUSAGE("/model <modelname/blockid>");

	cs_char modelname[64];
	if(COMMAND_GETARG(modelname, 64, 0)) {
		if(!Client_SetModelStr(ccdata->caller, modelname))
			COMMAND_PRINT("Invalid model name.");
		Client_Update(ccdata->caller);
		COMMAND_PRINT("Model changed successfully.");
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(SetWeather) {
	COMMAND_SETUSAGE("/setweather <sunny/rain/snow>");

	World *world = Client_GetWorld(ccdata->caller);
	if(!world) COMMAND_PRINT("Something went wrong");

	cs_char weathername[64];
	if(COMMAND_GETARG(weathername, 64, 0)) {
		EWeather weather = WORLD_WEATHER_SUN;
		if(String_CaselessCompare(weathername, "snow"))
			weather = WORLD_WEATHER_SNOW;
		else if(String_CaselessCompare(weathername, "rain"))
			weather = WORLD_WEATHER_RAIN;
		World_SetWeather(world, weather);
		World_FinishEnvUpdate(world);
		COMMAND_PRINT("Weather changed");
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(World) {
	COMMAND_SETUSAGE("/world <create/save/load/unload/generate/list> ...");

	cs_char subcmd[64], worldname[64];
	if(COMMAND_GETARG(subcmd, 64, 0)) {
		World *world = NULL,
		*mainw = World_Main;
		cs_int32 argoffset = 2;

		if(COMMAND_GETARG(worldname, 64, 1))
			world = World_GetByName(worldname);

		if(String_CaselessCompare(subcmd, "create")) {
			if(world)
				COMMAND_PRINT("World with that name already exists");
			cs_char xarg[6], yarg[6], zarg[6];
			if(COMMAND_GETARG(xarg, 6, argoffset++) &&
			COMMAND_GETARG(yarg, 6, argoffset++) &&
			COMMAND_GETARG(zarg, 6, argoffset++)) {
				SVec dims = {
					.x = (cs_int16)String_ToInt(xarg),
					.y = (cs_int16)String_ToInt(yarg),
					.z = (cs_int16)String_ToInt(zarg)
				};

				if(Vec_HaveZero(dims) || Vec_IsNegative(dims))
					COMMAND_PRINT("Invalid world dimensions");

				World *tmp = World_Create(worldname);
				if(tmp) {
					World_SetDimensions(tmp, &dims);
					World_AllocBlockArray(tmp);
					World_Add(tmp);
					COMMAND_PRINTF("World \"%s\" created.", worldname);
				}

				COMMAND_PRINT("Unexpected error");
			}

			COMMAND_PRINT("Invalid dimensions, use /world create <name> <x> <y> <z>");
		} else if(String_CaselessCompare(subcmd, "load")) {
			if(world) COMMAND_PRINT("This world is already loaded");

			World *tmp = World_Create(worldname);
			if(World_Load(tmp)) {
				World_Lock(tmp, 0);
				World_Unlock(tmp);
				World_Add(tmp);
				COMMAND_PRINT("World loaded successfully");
			}

			World_Free(tmp);
			COMMAND_PRINT("Failed to load world");
		} else {
			if(!world) {
				argoffset = 1;
				if(ccdata->caller) {
					world = Client_GetWorld(ccdata->caller);
				}
			}

			if(String_CaselessCompare(subcmd, "list")) {
				COMMAND_APPEND("&aList of loaded worlds:");

				AListField *tmp;
				SVec dims;
				cs_int32 startPage = 1;
				if(COMMAND_GETARG(subcmd, 64, 1))
					startPage = String_ToInt(subcmd);
				Pager pager = Pager_Init(startPage, PAGER_DEFAULT_PAGELEN);

				List_Iter(tmp, World_Head) {
					Pager_Step(pager);

					world = AList_GetValue(tmp).ptr;
					cs_str wname = World_GetName(world);
					World_GetDimensions(world, &dims);
					COMMAND_APPENDF(worldname, 64, "\r\n  &3%.12s&f: &d%dx%dx%d&f with &9%d&f player(-s)",
						wname, dims.x, dims.y, dims.z, World_CountPlayers(world)
					);
				}

				if(Pager_IsDirty(pager))
					COMMAND_APPENDF(worldname, 64, "\r\nPage %d/%d shown",
						Pager_CurrentPage(pager), Pager_CountPages(pager)
					);

				return true;
			}

			if(!world) COMMAND_PRINTUSAGE;

			if(String_CaselessCompare(subcmd, "save")) {
				if(World_Save(world))
					COMMAND_PRINT("World saving scheduled");

				COMMAND_PRINT("This world is busy, try again later");
			} else if(String_CaselessCompare(subcmd, "unload")) {
				if(world == mainw)
					COMMAND_PRINT("Cannot unload main world");

				for(ClientID i = 0; i < MAX_CLIENTS; i++) {
					Client *client = Clients_List[i];
					if(client && Client_IsInWorld(client, world))
						Client_ChangeWorld(client, mainw);
				}
				if(World_Save(world)) {
					if(World_Remove(world))
						COMMAND_PRINT("World unloaded.");

					COMMAND_PRINT("Unexpected error");
				}

				COMMAND_PRINT("This world is busy, try again later");
			} else if(String_CaselessCompare(subcmd, "generate")) {
				cs_char genname[64], genseed[12];
				cs_int32 numseed = GENERATOR_SEED_FROM_TIME;

				if(COMMAND_GETARG(genname, 64, argoffset++)) {
					GeneratorRoutine gr = Generators_Get(genname);
					if(!gr) COMMAND_PRINT("Unknown generator name");

					if(COMMAND_GETARG(genseed, 12, argoffset))
						numseed = String_ToInt(genseed);

					World_Lock(world, 0);
					World_CleanBlockArray(world);
					if(gr(world, numseed) == false) {
						World_Unlock(world);
						COMMAND_PRINT("Generator failed");
					}
					World_Unlock(world);
					for(ClientID i = 0; i < MAX_CLIENTS; i++) {
						Client *client = Clients_List[i];
						if(client && Client_IsInWorld(client, world))
							Client_ChangeWorld(client, world);
					}
					COMMAND_PRINT("World generation done");
				}
			}
		}
	}

	COMMAND_PRINTUSAGE;
}

COMMAND_FUNC(GoTo) {
	COMMAND_SETUSAGE("/goto <worldname>");

	cs_char worldname[64];
	if(COMMAND_GETARG(worldname, 60, 0)) {
		World *world = World_GetByName(worldname);
		if(world) {
			if(Client_IsInWorld(ccdata->caller, world))
				COMMAND_PRINT("You already in this world.");

			if(Client_ChangeWorld(ccdata->caller, world))
				COMMAND_PRINTF("Teleported to \"%s\"", worldname);
		}

		COMMAND_PRINT("World not found.");
	}

	COMMAND_PRINTUSAGE;
}

Command_DeclareBunch(Commands) {
	COMMAND_BUNCH_ADD(Info, CMDF_NONE, "Prints server software info"),
	COMMAND_BUNCH_ADD(MakeOp, CMDF_OP, "Grant operator status to a player"),
	COMMAND_BUNCH_ADD(DeOp, CMDF_OP, "Revokes operator status from a player"),
	COMMAND_BUNCH_ADD(Ban, CMDF_OP, "Adds player to banlist"),
	COMMAND_BUNCH_ADD(UnBan, CMDF_OP, "Removes player from banlist"),
	COMMAND_BUNCH_ADD(Uptime, CMDF_NONE, "Prints server uptime"),
	COMMAND_BUNCH_ADD(Kick, CMDF_OP, "Kicks a player off a server"),
	COMMAND_BUNCH_ADD(SetModel, CMDF_OP | CMDF_CLIENT, "Sets player model"),
	COMMAND_BUNCH_ADD(SetWeather, CMDF_OP | CMDF_CLIENT, "Sets weather in current world"),
	COMMAND_BUNCH_ADD(GoTo, CMDF_OP | CMDF_CLIENT, "Teleports you to specified world"),
	COMMAND_BUNCH_ADD(World, CMDF_OP, "Worlds management"),

	COMMAND_BUNCH_END
};

cs_bool Base_RegisterCommands(void) {
	return Command_RegisterBunch(Commands);
}

void Base_UnregisterCommands(void) {
	Command_UnregisterBunch(Commands);
}
