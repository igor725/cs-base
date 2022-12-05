#ifndef CSBASEFUNCS_H
#define CSBASEFUNCS_H
#include "core.h"

void Base_Rcon(void);
void Base_Config(void);
void Base_DayNight(void);
void Base_DayNightUninit(void);
void Base_OnStop(void *);
void Base_OnSpawn(void *);
void Base_OnDisconnect(void *);
void Base_OnMessage(void *);
void Base_PreCommand(void *);
void Base_OnBlockPlace(void *);
cs_bool Base_OnHandshake(void *);
void Base_OnWorldStatus(void *);
void Base_Heartbeat(void);
void Base_HeartbeatStop(void);
void Base_AutoSave(void);
void Base_AutoSaveUninit(void);

cs_bool Base_RegisterCommands(void);
void Base_UnregisterCommands(void);
#endif
