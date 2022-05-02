#ifndef CSBASEFUNCS_H
#define CSBASEFUNCS_H
#include "core.h"

void Base_TNT(void);
void Base_Rcon(void);
void Base_Chat(void);
void Base_Config(void);
void Base_DayNight(void);
void Base_DayNightUninit(void);
void Base_OnStop(void *);
void Base_OnBlockPlace(void *);
cs_bool Base_OnHandshake(void *);
void Base_OnWorldUnloaded(void *);
void Base_Heartbeat(void);
void Base_AutoSave(void);

cs_bool Base_RegisterCommands(void);
void Base_UnregisterCommands(void);
#endif
