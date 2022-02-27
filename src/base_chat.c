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
	char message[128];

	if(Client_IsFirstSpawn(client)) {
		cs_str name = Client_GetName(client);
		cs_str appname = Client_GetAppName(client);
		if(String_FormatBuf(message, 128, "&e%s has joined (%s)", name, appname)) {
			Client_Chat(Broadcast, MESSAGE_TYPE_CHAT, message);
			Log_Info(message);
		}
	}
}

static void ondisconnectfunc(void *param) {
	if(!Server_Active) return;
	if(!Config_GetBoolByKey(Base_ConfigStore, "connect-notifications")) return;
	Client *client = (Client *)param;
	if(!Client_CheckState(client, PLAYER_STATE_INGAME)) return;
	cs_char message[128];
	if(String_FormatBuf(message, 128, "&e%s left (%s)", Client_GetName(client), Client_GetDisconnectReason(client))) {
		Client_Chat(Broadcast, MESSAGE_TYPE_CHAT, message);
		Log_Info(message);
	}
}

void Base_Chat(void) {
	Event_RegisterVoid(EVT_ONSPAWN, onspawnfunc);
	Event_RegisterVoid(EVT_ONDISCONNECT, ondisconnectfunc);
}
