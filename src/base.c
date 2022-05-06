#include <core.h>
#include <config.h>
#include <event.h>
#include <str.h>
#include <client.h>
#include <command.h>
#include <plugin.h>
#include "base_lists.h"
#include "base_itf.h"
#include "base_funcs.h"

BList Base_Operators = {
	.alerted = false,
	.filename = "ops.txt",
	.head = NULL
};

BList Base_Bans = {
	.alerted = false,
	.filename = "bans.txt",
	.head = NULL
};

CStore *Base_ConfigStore;
cs_char Base_OSName[64];

Plugin_SetVersion(1);

Plugin_DeclareInterfaces {
	PLUGIN_IFACE_ADD(BASE_ITF_NAME, BaseController)

	PLUGIN_IFACE_END
};

/*
 * Перенести куда-нибудь это
 * чудо. Может быть в base_commands.
*/

static cs_bool Base_Whisper(onMessage *a) {
	if(*a->message == '@') {
		cs_char buffer[512];
		cs_size messagestart = 0;
		if((messagestart = String_GetArgument(a->message + 1, buffer, 32, 0)) > 0) {
			Client *target = Client_GetByName(buffer);
			if(target) {
				cs_char *text = a->message + messagestart;
				if(String_Length(text) < 3) {
					Client_Chat(a->client, MESSAGE_TYPE_CHAT, "&eNo message entered");
					return false;
				}
				text += 2;
				if(String_FormatBuf(buffer, 512, "&e[<] &7%s: &f%s", Client_GetName(target), text))
					Client_Chat(a->client, MESSAGE_TYPE_CHAT, buffer);
				if(String_FormatBuf(buffer, 512, "&9[>] &7%s: &f%s", Client_GetName(a->client), text))
					Client_Chat(target, MESSAGE_TYPE_CHAT, buffer);
			} else
				Client_Chat(a->client, MESSAGE_TYPE_CHAT, "&ePlayer not found");
		}

		return false;
	}

	return true;
}

Event_DeclareBunch (events) {
	EVENT_BUNCH_ADD('v', EVT_ONSTOP, Base_OnStop)
	EVENT_BUNCH_ADD('b', EVT_ONHANDSHAKEDONE, Base_OnHandshake)
	EVENT_BUNCH_ADD('v', EVT_ONDISCONNECT, Base_OnDisconnect)
	EVENT_BUNCH_ADD('v', EVT_ONSPAWN, Base_OnSpawn)
	EVENT_BUNCH_ADD('b', EVT_ONMESSAGE, Base_Whisper)
	EVENT_BUNCH_ADD('v', EVT_ONBLOCKPLACE, Base_OnBlockPlace)
	EVENT_BUNCH_ADD('v', EVT_ONWORLDUNLOADED, Base_OnWorldUnloaded)

	EVENT_BUNCH_END
};

cs_bool Plugin_Load(void) {
#	if defined(CORE_USE_WINDOWS)
		String_Copy(Base_OSName, 64, "Windows ");
		String_Append(Base_OSName, 64, sizeof(cs_size) == 8 ? "64" : "32");
#	elif defined(CORE_USE_UNIX)
		cs_file uname = File_ProcOpen("uname -osm", "r");
		if(uname) {
			File_ReadLine(uname, Base_OSName, 64);
			File_ProcClose(uname);
		} else
			String_Copy(Base_OSName, 64, "Unix-like");
#	endif

	Base_Config();
	Base_Rcon();
	Base_DayNight();
	Base_Heartbeat();
	Base_AutoSave();
	Base_TNT();

	return Base_RegisterCommands() &&
	Event_RegisterBunch(events);
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	Base_EmptyList(&Base_Operators);
	Base_EmptyList(&Base_Bans);
	Event_UnregisterBunch(events);
	Base_UnregisterCommands();
	Base_DayNightUninit();
	Base_AutoSaveUninit();
	Base_HeartbeatStop();
	if(Base_ConfigStore)
		Config_DestroyStore(Base_ConfigStore);

	return true;
}
