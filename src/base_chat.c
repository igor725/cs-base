#include <core.h>
#include <config.h>
#include <client.h>
#include <event.h>
#include <server.h>
#include <log.h>
#include <str.h>

extern CStore *Base_ConfigStore;

void Base_OnSpawn(onSpawn *obj) {
	if(!Config_GetBoolByKey(Base_ConfigStore, "connect-notifications")) return;
	char message[128];

	if(Client_IsFirstSpawn(obj->client)) {
		cs_str name = Client_GetName(obj->client);
		cs_str appname = Client_GetAppName(obj->client);
		if(String_FormatBuf(message, 128, "&e%s has joined (%s)", name, appname)) {
			Client_Chat(CLIENT_BROADCAST, MESSAGE_TYPE_CHAT, message);
			Log_Info(message);
		}
	}
}

void Base_OnDisconnect(Client *obj) {
	if(!Server_Active) return;
	if(Client_IsBot(obj)) return;
	if(Client_GetState(obj) < CLIENT_STATE_INGAME) return;
	if(!Config_GetBoolByKey(Base_ConfigStore, "connect-notifications")) return;
	cs_char message[128];
	if(String_FormatBuf(message, 128, "&e%s left (%s)", Client_GetName(obj), Client_GetDisconnectReason(obj))) {
		Client_Chat(CLIENT_BROADCAST, MESSAGE_TYPE_CHAT, message);
		Log_Info(message);
	}
}

cs_bool Base_OnMessage(onMessage *obj) {
	cs_char buffer[512];

	if(*obj->message == '@') {
		cs_size messagestart = 0;
		if((messagestart = String_GetArgument(obj->message + 1, buffer, 32, 0)) > 0) {
			Client *target = Client_GetByName(buffer);
			if(target) {
				cs_char *text = obj->message + messagestart;
				if(String_Length(text) < 3) {
					Client_Chat(obj->client, MESSAGE_TYPE_CHAT, "&eNo message entered");
					return false;
				}
				text += 2;
				if(String_FormatBuf(buffer, 512, "&e[<] &7%s: &f%s", Client_GetName(target), text))
					Client_Chat(obj->client, MESSAGE_TYPE_CHAT, buffer);
				if(String_FormatBuf(buffer, 512, "&9[>] &7%s: &f%s", Client_GetName(obj->client), text))
					Client_Chat(target, MESSAGE_TYPE_CHAT, buffer);
			} else
				Client_Chat(obj->client, MESSAGE_TYPE_CHAT, "&ePlayer not found");
		}

		return false;
	} else {
		// TODO: Придумать более адекватное решение
		cs_int32 namend = String_FormatBuf(buffer, 512, "<%s&f>: ", Client_GetName(obj->client));
		if(namend > 0) {
			cs_int32 i = 0, m = 512 - namend;
			for(; obj->message[i] && i < m; i++) // TODO: Конвертилка CP437 в UTF-8
				buffer[namend + i] = obj->message[i] < ' ' || obj->message[i] > '~' ? '?' : obj->message[i];
			buffer[namend + i] = '\0';
			Log_Chat(buffer);
		}
	}

	return true;
}

void Base_PreCommand(preCommand *obj) {
	if(obj->caller) {
		cs_str callername = Client_GetName(obj->caller);
		cs_str cmdname = obj->command->name;
		if(obj->args)
			Log_Warn("%s&f executed command: /%s %s", callername, cmdname, obj->args);
		else
			Log_Warn("%s&f executed command: /%s", callername, cmdname);
	}
}
