#include <core.h>
#include <lang.h>
#include <config.h>
#include <event.h>

void Base_Rcon(void);
void Base_Chat(void);
void Base_Config(void);
void Base_OnStop(void *);
void Base_OnSpawn(void *);
void Base_Commands(void);
LGroup *Base_Lang;
CStore *Base_ConfigStore;
cs_str Base_OSName;
Plugin_SetVersion(1)

cs_bool Plugin_Load(void) {
  Base_OSName = // Todo use uname and wmic
  #if defined(WINDOWS)
  "Windows";
  #elif defined(UNIX)
  "Unix-like";
  #endif

  Base_Lang = Lang_NewGroup(20);
  // Notifications
  Lang_Set(Base_Lang, 0, "Player %s connected with %s.");
  Lang_Set(Base_Lang, 1, "Player %s disconnected.");

  // "Info" command
	Lang_Set(Base_Lang, 2, "CServer-%s with PluginAPI v%d runned on %s");

  // "Plugin" command
  Lang_Set(Base_Lang, 3, "/plugins <load/unload/print> [pluginName]");
  Lang_Set(Base_Lang, 4, "Plugin \"%s\" loaded.");
  Lang_Set(Base_Lang, 5, "Plugin \"%s\" not loaded.");
  Lang_Set(Base_Lang, 6, "Plugin \"%s\" is already loaded.");
  Lang_Set(Base_Lang, 7, "Plugin \"%s\" successfully unloaded.");
  Lang_Set(Base_Lang, 8, "Plugin \"%s\" cannot be unloaded.");
  Lang_Set(Base_Lang, 9, "Plugin_Init() == false, something went wrong.");
  Lang_Set(Base_Lang, 10, "Loaded plugins list:");
  Lang_Set(Base_Lang, 11, "\r\n%d.%s v%d");
  Lang_Set(Base_Lang, 12, "\r\n(Can't show full plugins list)");

  // "Kick" command
  Lang_Set(Base_Lang, 13, "/kick <player> [reason]");
  Lang_Set(Base_Lang, 14, "Player %s kicked");

  Base_Config();
  Base_Chat();
  Base_Rcon();
  Base_Commands();
  Event_RegisterVoid(EVT_ONSPAWN, Base_OnSpawn);
  Event_RegisterVoid(EVT_ONSTOP, Base_OnStop);
  return true;
}

cs_bool Plugin_Unload(void) {return false;}
