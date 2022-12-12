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
#if PLUGIN_API_NUM > 1
Plugin_SetURL("https://github.com/igor725/cs-base");
#endif

extern BaseItf BaseController;

Plugin_DeclareInterfaces {
	PLUGIN_IFACE_ADD(CSBASE_ITF_NAME, BaseController),

	PLUGIN_IFACE_END
};

Event_DeclareBunch (events) {
	EVENT_BUNCH_ADD('v', EVT_ONSTOP, Base_OnStop),
	EVENT_BUNCH_ADD('b', EVT_ONHANDSHAKEDONE, Base_OnHandshake),
	EVENT_BUNCH_ADD('v', EVT_ONDISCONNECT, Base_OnDisconnect),
	EVENT_BUNCH_ADD('v', EVT_PRECOMMAND, Base_PreCommand),
	EVENT_BUNCH_ADD('v', EVT_ONSPAWN, Base_OnSpawn),
	EVENT_BUNCH_ADD('b', EVT_ONMESSAGE, Base_OnMessage),
	EVENT_BUNCH_ADD('v', EVT_ONBLOCKPLACE, Base_OnBlockPlace),
	EVENT_BUNCH_ADD('v', EVT_ONWORLDSTATUSCHANGE, Base_OnWorldStatus),

	EVENT_BUNCH_END
};

cs_bool Plugin_Load(void) {
#	if defined(CORE_USE_WINDOWS)
		String_Copy(Base_OSName, 64, "Windows ");
		String_Append(Base_OSName, 64, sizeof(cs_size) == 8 ? "64" : "32");
#	elif defined(CORE_USE_UNIX)
#		ifdef CORE_USE_DARWIN
#			define UARGS "sm"
#		else
#			define UARGS "osm"
#		endif
		cs_file uname = File_ProcOpen("uname -" UARGS, "r");
		if(uname) {
			File_ReadLine(uname, Base_OSName, 64);
			File_ProcClose(uname);
		}

		if(*Base_OSName == '\0')
			String_Copy(Base_OSName, 64, "Unix-like");
#	endif

	Base_Config();
	Base_Rcon();
	Base_DayNight();
	Base_Heartbeat();
	Base_AutoSave();

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
