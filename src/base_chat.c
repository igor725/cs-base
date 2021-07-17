#include <core.h>
#include <client.h>
#include <event.h>
#include <log.h>
#include <lang.h>
LGroup *Base_Lang;

static void onspawnfunc(void *param) {
  Client *client = (Client *)param;
  PlayerData *pd = client->playerData;

  if(pd->firstSpawn) {
    cs_str name = Client_GetName(client);
    cs_str appname = Client_GetAppName(client);
    Log_Info(Lang_Get(Base_Lang, 0), name, appname);
    pd->firstSpawn = false;
  }
}

static void ondisconnectfunc(void *param) {
  Log_Info(Lang_Get(Base_Lang, 1), Client_GetName((Client *)param));
}

void Base_Chat(void) {
  Event_RegisterVoid(EVT_ONSPAWN, onspawnfunc);
  Event_RegisterVoid(EVT_ONDISCONNECT, ondisconnectfunc);
}
