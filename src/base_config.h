#ifndef CSLUACONFIG_H
#define CSLUACONFIG_H
#include <core.h>
#include <types/config.h>

cs_bool Base_IsOP(cs_str name);
cs_bool Base_AddOP(cs_str name);
cs_bool Base_RemoveOP(cs_str name);
cs_bool Base_IsBanned(cs_str name);
cs_bool Base_AddBan(cs_str name);
cs_bool Base_RemoveBan(cs_str name);
CStore *Base_GetConfig(void);
#endif
