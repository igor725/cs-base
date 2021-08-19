#include <core.h>
#include <lang.h>
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

LGroup *Base_Lang;
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

  Base_Lang = Lang_NewGroup(20);

  // Notifications
  Lang_Set(Base_Lang, 0, "Player %s connected with %s.");
  Lang_Set(Base_Lang, 1, "Player %s disconnected.");

  // "Info" command
	Lang_Set(Base_Lang, 2, "CServer/%s with PluginAPI v%03d runned on %s");

  // "Plugin" command
  Lang_Set(Base_Lang, 3, "/plugins <load/unload/print> [pluginName]");
  Lang_Set(Base_Lang, 4, "Plugin \"%s\" loaded.");
  Lang_Set(Base_Lang, 5, "Plugin \"%s\" not loaded.");
  Lang_Set(Base_Lang, 6, "Plugin \"%s\" is already loaded.");
  Lang_Set(Base_Lang, 7, "Plugin \"%s\" successfully unloaded.");
  Lang_Set(Base_Lang, 8, "Plugin \"%s\" cannot be unloaded.");
  Lang_Set(Base_Lang, 9, "Plugin_Init() == false, something went wrong.");
  Lang_Set(Base_Lang, 10, "Loaded plugins list:");
  Lang_Set(Base_Lang, 11, "\r\n  %d.%s v%d");

  // "Kick" command
  Lang_Set(Base_Lang, 13, "Player %s kicked");

  // "Ban" command
  Lang_Set(Base_Lang, 14, "/kick <player> [reason]");
  Lang_Set(Base_Lang, 15, "Player %s banned.");
  Lang_Set(Base_Lang, 16, "Player %s unbanned.");
  Lang_Set(Base_Lang, 17, "You are banned!");
  Lang_Set(Base_Lang, 18, "Failed to add player to banlist.");
  Lang_Set(Base_Lang, 19, "Failed to remove player from banlist.");

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

cs_bool Plugin_Unload(void) {return false;}
