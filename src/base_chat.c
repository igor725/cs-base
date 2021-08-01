#include <core.h>
#include <config.h>
#include <client.h>
#include <event.h>
#include <log.h>
#include <lang.h>

extern LGroup *Base_Lang;
extern CStore *Base_ConfigStore;

static void onspawnfunc(void *param) {
  Client *client = (Client *)param;
  PlayerData *pd = client->playerData;

  if(pd->firstSpawn) {
    cs_str name = Client_GetName(client);
    cs_str appname = Client_GetAppName(client);
    if(Config_GetBoolByKey(Base_ConfigStore, "connect-notifications"))
      Log_Info(Lang_Get(Base_Lang, 0), name, appname);
  }
}

static void ondisconnectfunc(void *param) {
  if(Config_GetBoolByKey(Base_ConfigStore, "connect-notifications"))
    Log_Info(Lang_Get(Base_Lang, 1), Client_GetName((Client *)param));
}

void Base_Chat(void) {
  Event_RegisterVoid(EVT_ONSPAWN, onspawnfunc);
  Event_RegisterVoid(EVT_ONDISCONNECT, ondisconnectfunc);
}
