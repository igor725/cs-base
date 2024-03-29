#include <core.h>
#include <str.h>
#include <log.h>
#include <cserror.h>
#include <config.h>
#include <platform.h>
#include <server.h>
#include <command.h>

extern CStore *Base_ConfigStore;
Socket Rcon_Socket;
cs_str Rcon_Password;

typedef struct {
	cs_int32 size, id, cmd;
	char payload[MAX_CMD_OUT];
} RPacket;

typedef struct {
	Socket fd;
	cs_bool authed, error;
	struct sockaddr_in addr;
	RPacket packet;
} RClient;

INL static void makeitsafe(cs_char *str) {
	if(*str == '\0') return;
	while(*str != '\0') {
		if(*str < ' ' || *str > '~')
			*str = '_';
		str++;
	}
}

INL static void setrconpayload(RPacket *packet, cs_str string) {
	cs_int32 strsize = (cs_int32)String_Copy(packet->payload, MAX_CMD_OUT - 2, string);
	packet->size = max(10, strsize + 9);
}

INL static void rconcommandhandler(RClient *client, cs_char *str) {
	if(*str == '/') ++str;

	cs_char ret[MAX_CMD_OUT] = {0};
	cs_char *args = str;

	while(1) {
		++args;
		if(*args == '\0') {
			args = NULL;
			break;
		} else if(*args == 32) {
			*args++ = '\0';
			break;
		}
	}

	Log_Info("RCON user %08X executed the command: %s.", client->addr.sin_addr.s_addr, str);
	Command *cmd = Command_GetByName(str);
	if(cmd) {
		if(cmd->flags & CMDF_CLIENT) {
			setrconpayload(&client->packet, "This command can't be called from console.");
			return;
		}

		CommandCallData ccdata = {
			.command = cmd,
			.args = (cs_str)args,
			.caller = NULL,
			.out = ret
		};

		if(cmd->func(&ccdata))
			setrconpayload(&client->packet, ret);
	} else {
		setrconpayload(&client->packet, "Unknown command.");
	}
}

static cs_bool handlerconpacket(RClient *client) {
	if(client->authed == false && client->packet.cmd != 0x03) return false;
	switch (client->packet.cmd) {
		case 0x02:
			client->packet.size = 10;
			client->packet.cmd = 0x00;
			makeitsafe(client->packet.payload);
			rconcommandhandler(client, client->packet.payload);
			break;
		case 0x03:
			if(String_Compare(client->packet.payload, Rcon_Password)) {
				client->authed = true;
				client->packet.size = 10;
				client->packet.cmd = 0x02;
				*(cs_uint16 *)client->packet.payload = 0x0000;
			} else {
				client->error = true;
				client->packet.size = 10;
				client->packet.id = -1;
				client->packet.cmd = 0x02;
				*(cs_uint16 *)client->packet.payload = 0x0000;
			}
			break;

		default: return false;
	}

	return Socket_Send(client->fd, (cs_char *)&client->packet, client->packet.size + 4) == client->packet.size + 4;
}

INL static cs_bool handlerconclient(RClient *client) {
	if(Socket_Receive(client->fd, (cs_char *)&client->packet.size, 4, MSG_WAITALL) == 4) {
		if(client->packet.size < 10 || client->packet.size > MAX_CMD_OUT) return false;
		if(Socket_Receive(client->fd, (cs_char *)&client->packet + 4, client->packet.size, MSG_WAITALL) == client->packet.size)
			return handlerconpacket(client);
	}
	return false;
}

THREAD_FUNC(RconClient) {
	RClient *rclient = (RClient *)param;
	while(!rclient->error && handlerconclient(rclient));
	Socket_Shutdown(rclient->fd, SD_SEND);
	Memory_Free((void *)rclient);
	return 0;
}

THREAD_FUNC(RconAccept) {
	(void)param;
	while(true) {
		RClient *rclient = (RClient *)Memory_TryAlloc(1, sizeof(RClient));
		if(!rclient) {
			Log_Error("Can't allocate memory for RClient structure.");
			Socket_Close(Rcon_Socket);
			Thread_Sleep(500);
			continue;
		}
		rclient->fd = Socket_Accept(Rcon_Socket, &rclient->addr);
		if(rclient->fd != INVALID_SOCKET)
			Thread_Create(RconClient, (void *)rclient, true);
		else {
			Memory_Free((void *)rclient);
			break;
		}
	}
	return 0;
}

void Base_Rcon(void) {
	if(Config_GetBoolByKey(Base_ConfigStore, "rcon-enabled")) {
		Rcon_Password = Config_GetStrByKey(Base_ConfigStore, "rcon-password");
		if(String_Length(Rcon_Password) < 8) {
			Log_Warn("Can't start RCON server: \"rcon-password\" must be at least 8 characters long.");
			return;
		} else Rcon_Password = String_AllocCopy(Rcon_Password);
		cs_str ip = Config_GetStrByKey(Server_Config, "server-ip");
		cs_uint16 port = (cs_uint16)Config_GetIntByKey(Base_ConfigStore, "rcon-port");
		if((Rcon_Socket = Socket_New()) == INVALID_SOCKET) {
			Error_PrintSys(false);
			return;
		}
		struct sockaddr_in ssa;
		if(Socket_SetAddr(&ssa, ip, port) > 0 && Socket_Bind(Rcon_Socket, &ssa)) {
			Log_Info("RCON server started on %s:%d", ip, port);
			Thread_Create(RconAccept, NULL, true);
			return;
		}

		Socket_Close(Rcon_Socket);
		Log_Error("Failed to start RCON server.");
	}
}
