#include <core.h>
#include <config.h>
#include <platform.h>
CStore *Base_ConfigStore;

void Base_Config(void) {
  Base_ConfigStore = Config_NewStore("base.cfg");
  CEntry *ent;

  ent = Config_NewEntry(Base_ConfigStore, "rcon-enabled", CFG_TBOOL);
  Config_SetComment(ent, "Enable or disable rcon protocol.");
  Config_SetDefaultBool(ent, false);

  ent = Config_NewEntry(Base_ConfigStore, "rcon-port", CFG_TINT16);
  Config_SetComment(ent, "Use specified port to accept rcon clients. [1-65535]");
  Config_SetDefaultInt16(ent, 25575);

  ent = Config_NewEntry(Base_ConfigStore, "rcon-password", CFG_TSTR);
  Config_SetComment(ent, "Rcon authorization password.");
  Config_SetDefaultStr(ent, "");

  ent = Config_NewEntry(Base_ConfigStore, "connect-notifications", CFG_TBOOL);
  Config_SetComment(ent, "Enable or disable player (dis-)connection notifications.");
  Config_SetDefaultBool(ent, true);

  Base_ConfigStore->modified = true;
  if(!Config_Load(Base_ConfigStore)) {
    Config_PrintError(Base_ConfigStore);
    Process_Exit(1);
  }
  Config_Save(Base_ConfigStore);
}
