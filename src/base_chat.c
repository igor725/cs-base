#include <core.h>
#include <config.h>
#include <client.h>
#include <event.h>
#include <server.h>
#include <log.h>
#include <lang.h>
#include <str.h>

extern LGroup *Base_Lang;
extern CStore *Base_ConfigStore;

static void onspawnfunc(void *param) {
  if(!Config_GetBoolByKey(Base_ConfigStore, "connect-notifications")) return;
  Client *client = (Client *)param;
  PlayerData *pd = client->playerData;
  char message[154];

  if(pd->firstSpawn) {
    cs_str name = Client_GetName(client);
    cs_str appname = Client_GetAppName(client);
    if(String_FormatBuf(message, 154, Lang_Get(Base_Lang, 0), name, appname)) {
      Client_Chat(Broadcast, MT_CHAT, message);
      Log_Info(message + 2);
    }
  }
}

static void ondisconnectfunc(void *param) {
  if(!Server_Active) return;
  if(!Config_GetBoolByKey(Base_ConfigStore, "connect-notifications")) return;
  Client *client = (Client *)param;
  if(client->playerData->firstSpawn) return;
  cs_char message[88];
  if(String_FormatBuf(message, 88, Lang_Get(Base_Lang, 1), Client_GetName(client))) {
    Client_Chat(Broadcast, MT_CHAT, message);
    Log_Info(message + 2);
  }
}

void Base_Chat(void) {
  Event_RegisterVoid(EVT_ONSPAWN, onspawnfunc);
  Event_RegisterVoid(EVT_ONDISCONNECT, ondisconnectfunc);
}
