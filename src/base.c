#include <core.h>
#include <config.h>
#include <event.h>
#include <str.h>

void Base_Rcon(void);
void Base_Chat(void);
void Base_Config(void);
void Base_OnStop(void *);
void Base_OnSpawn(void *);
void Base_OnHandshake(void *);
void Base_Commands(void);
void Base_Heartbeat(void);

CStore *Base_ConfigStore;
cs_char Base_OSName[64];
Plugin_SetVersion(1)

cs_bool Plugin_Load(void) {
#if defined(WINDOWS)
  String_Copy(Base_OSName, 64, "Windows");
#elif defined(UNIX)
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
  Base_Commands();
  Base_Heartbeat();
  Event_RegisterVoid(EVT_ONHANDSHAKEDONE, Base_OnHandshake);
  Event_RegisterVoid(EVT_ONSPAWN, Base_OnSpawn);
  Event_RegisterVoid(EVT_ONSTOP, Base_OnStop);
  return true;
}

cs_bool Plugin_Unload(cs_bool force) {
  if(force) {
    EVENT_UNREGISTER(EVT_ONHANDSHAKEDONE, Base_OnHandshake);
    EVENT_UNREGISTER(EVT_ONSPAWN, Base_OnSpawn);
    EVENT_UNREGISTER(EVT_ONSTOP, Base_OnStop);
    return true;
  }
  return false;
}
