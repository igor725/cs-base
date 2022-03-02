#include <core.h>
#include <config.h>
#include <event.h>
#include <str.h>
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

Event_DeclareBunch (events) {
	EVENT_BUNCH_ADD('v', EVT_ONHANDSHAKEDONE, Base_OnHandshake)
	EVENT_BUNCH_ADD('v', EVT_ONWORLDUNLOADED, Base_OnWorldUnloaded)
	EVENT_BUNCH_ADD('v', EVT_ONBLOCKPLACE, Base_OnBlockPlace)
	EVENT_BUNCH_ADD('v', EVT_ONSTOP, Base_OnStop)

	EVENT_BUNCH_END
};

cs_bool Plugin_Load(void) {
#	if defined(CORE_USE_WINDOWS)
		String_Copy(Base_OSName, 64, "Windows");
#	elif defined(CORE_USE_UNIX)
		cs_file uname = File_ProcOpen("uname -osm", "r");
		if(uname) {
			File_ReadLine(uname, Base_OSName, 64);
			File_ProcClose(uname);
		} else
			String_Copy(Base_OSName, 64, "Unix-like");
#	endif

	Base_Config();
	Base_Chat();
	Base_Rcon();
	Base_DayNight();
	Base_Commands();
	Base_Heartbeat();
	Base_AutoSave();
	Base_TNT();

	return Event_RegisterBunch(events);
}

cs_bool Plugin_Unload(cs_bool force) {
	if(force) {
		Event_UnregisterBunch(events);
		if(Base_ConfigStore)
			Config_DestroyStore(Base_ConfigStore);
		return true;
	}

	return false;
}
