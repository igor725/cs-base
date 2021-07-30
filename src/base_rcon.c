#include <core.h>
#include <str.h>
#include <log.h>
#include <error.h>
#include <config.h>
#include <platform.h>
#include <server.h>
#include <command.h>
#include <lang.h>

CStore *Base_ConfigStore;
Socket Rcon_Socket;
cs_str Rcon_Password, Rcon_IP;
cs_uint16 Rcon_Port;

typedef struct {
  cs_int32 size;
  cs_int32 id;
  cs_int32 cmd;
  char payload[MAX_CMD_OUT];
} RPacket;

typedef struct {
  Socket fd;
  struct sockaddr_in addr;
  cs_bool authed, error;
  RPacket packet;
} RClient;

static void makeitsafe(cs_char *str) {
  if(*str == '\0') return;
  while(*str != '\0') {
    if(*str < ' ' || *str > '~')
      *str = '_';
    str++;
  }
}

static void setrconpayload(RPacket *packet, cs_str string) {
  cs_int32 strsize = (cs_int32)String_Copy(packet->payload, MAX_CMD_OUT - 2, string);
  packet->size = max(10, strsize + 9);
}

static void rconcommandhandler(RClient *client, cs_char *str) {
	if(*str == '/') ++str;

	cs_char ret[MAX_CMD_OUT];
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
			setrconpayload(&client->packet, Lang_Get(Lang_CmdGrp, 4));
			return;
		}

		CommandCallData ccdata;
		ccdata.args = (cs_str)args;
		ccdata.caller = NULL;
		ccdata.command = cmd;
		ccdata.out = ret;
		*ret = '\0';

		if(cmd->func(&ccdata))
      setrconpayload(&client->packet, ret);
	} else {
    setrconpayload(&client->packet, Lang_Get(Lang_CmdGrp, 3));
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
      if(String_CaselessCompare(client->packet.payload, Rcon_Password)) {
        client->packet.size = 10;
        client->packet.cmd = 0x02;
        *(cs_uint16 *)client->packet.payload = 0x0000;
        client->authed = true;
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

static cs_bool handlerconclient(RClient *client) {
  if(Socket_Receive(client->fd, (cs_char *)&client->packet.size, 4, 0) == 4) {
    if(client->packet.size < 10 || client->packet.size > MAX_CMD_OUT) return false;
    if(Socket_Receive(client->fd, (cs_char *)&client->packet + 4, client->packet.size, 0) == client->packet.size)
      return handlerconpacket(client);
  }

  return false;
}

THREAD_FUNC(Rcon_Client) {
  RClient *rclient = (RClient *)param;
  while(!rclient->error && handlerconclient(rclient));
  Socket_Shutdown(rclient->fd, SD_SEND);
  Memory_Free((void *)rclient);
  return 0;
}

THREAD_FUNC(Rcon_Accept) {
  (void)param;
  while(true) {
    RClient *rclient = (RClient *)Memory_Alloc(1, sizeof(RClient));
    rclient->error = false;
    rclient->authed = false;
    rclient->fd = Socket_Accept(Rcon_Socket, &rclient->addr);
    if(rclient->fd != INVALID_SOCKET)
      Thread_Create(Rcon_Client, (void *)rclient, true);
    else {
      Memory_Free((void *)rclient);
      break;
    }
  }
  return 0;
}

void Base_Rcon(void) {
  if(Config_GetBoolByKey(Base_ConfigStore, "rcon-enabled")) {
    if(!String_Length((Rcon_Password = Config_GetStrByKey(Base_ConfigStore, "rcon-password")))) {
      Log_Warn("Can't start RCON server: \"rcon-password\" cannot be empty!");
      return;
    }
    Rcon_IP = Config_GetStrByKey(Server_Config, "server-ip");
    Rcon_Port = Config_GetInt16ByKey(Base_ConfigStore, "rcon-port");
    Rcon_Socket = Socket_New();
    if(!Rcon_Socket) {
      Error_PrintSys(false);
    }
    struct sockaddr_in ssa;
    switch (Socket_SetAddr(&ssa, Rcon_IP, Rcon_Port)) {
      case 0:
        ERROR_PRINT(ET_SERVER, EC_INVALIDIP, false);
        break;
      case -1:
        Error_PrintSys(false);
        break;
    }
    if(!Socket_Bind(Rcon_Socket, &ssa)) {
      Error_PrintSys(false);
    } else {
      Log_Info("RCON server started on %s:%d", Rcon_IP, Rcon_Port);
      Thread_Create(Rcon_Accept, NULL, true);
    }
  }
}
