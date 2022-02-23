#include <core.h>
#include <config.h>
#include <event.h>
#include <str.h>
#include <plugin.h>

void Base_TNT(void);
void Base_Rcon(void);
void Base_Chat(void);
void Base_Config(void);
void Base_DayNight(void);
void Base_OnStop(void *);
void Base_OnBlockPlace(void *);
void Base_OnHandshake(void *);
void Base_OnWorldUnloaded(void *);
void Base_Commands(void);
void Base_Heartbeat(void);
void Base_AutoSave(void);

CStore *Base_ConfigStore;
cs_char Base_OSName[64];
Plugin_SetVersion(1);

EventRegBunch events[] = {
	{'v', EVT_ONHANDSHAKEDONE, (void *)Base_OnHandshake},
	{'v', EVT_ONWORLDUNLOADED, (void *)Base_OnWorldUnloaded},
	{'v', EVT_ONBLOCKPLACE, (void *)Base_OnBlockPlace},
	{'v', EVT_ONSTOP, (void *)Base_OnStop},
	{0, 0, NULL}
};

cs_bool Plugin_Load(void) {
#if defined(CORE_USE_WINDOWS)
	String_Copy(Base_OSName, 64, "Windows");
#elif defined(CORE_USE_UNIX)
	cs_file uname = File_ProcOpen("uname -osm", "r");
	if(uname) {
		File_ReadLine(uname, Base_OSName, 64);
		File_ProcClose(uname);
	} else
		String_Copy(Base_OSName, 64, "Unix-like");
#endif

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
		Config_DestroyStore(Base_ConfigStore);
		return true;
	}
	return false;
}
