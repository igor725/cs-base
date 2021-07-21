#include <core.h>
#include <str.h>
#include <log.h>
#include <config.h>
CStore *Base_ConfigStore;

void Base_Rcon(void) {
  if(Config_GetBoolByKey(Base_ConfigStore, "rcon-enabled")) {
    if(!String_Length(Config_GetStrByKey(Base_ConfigStore, "rcon-password"))) {
      Log_Warn("Can't start RCON server: \"rcon-password\" cannot be empty!");
      return;
    }
    Log_Info("RCON server is not implemented yet, sorry((");
  }
}
