#include <core.h>
#include <config.h>
#include <client.h>
#include <event.h>
#include <server.h>
#include <log.h>
#include <str.h>

extern CStore *Base_ConfigStore;

static void onspawnfunc(void *param) {
	if(!Config_GetBoolByKey(Base_ConfigStore, "connect-notifications")) return;
	Client *client = ((onSpawn *)param)->client;
	char message[154];

	if(Client_IsFirstSpawn(client)) {
		cs_str name = Client_GetName(client);
		cs_str appname = Client_GetAppName(client);
		if(String_FormatBuf(message, 154, "&ePlayer %s connected with %s.", name, appname)) {
			Client_Chat(Broadcast, MESSAGE_TYPE_CHAT, message);
			Log_Info(message + 2);
		}
	}
}

static void ondisconnectfunc(void *param) {
	if(!Server_Active) return;
	if(!Config_GetBoolByKey(Base_ConfigStore, "connect-notifications")) return;
	Client *client = (Client *)param;
	if(Client_IsFirstSpawn(client)) return;
	cs_char message[88];
	if(String_FormatBuf(message, 88, "&ePlayer %s disconnected.", Client_GetName(client))) {
		Client_Chat(Broadcast, MESSAGE_TYPE_CHAT, message);
		Log_Info(message + 2);
	}
}

void Base_Chat(void) {
	Event_RegisterVoid(EVT_ONSPAWN, onspawnfunc);
	Event_RegisterVoid(EVT_ONDISCONNECT, ondisconnectfunc);
}
