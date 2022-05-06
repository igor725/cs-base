#include <core.h>
#include <log.h>
#include <config.h>
#include <heartbeat.h>

extern CStore *Base_ConfigStore;
Heartbeat *CCHbeat = NULL;

void Base_Heartbeat(void) {
	if(Config_GetBoolByKey(Base_ConfigStore, "heartbeat-cc-enabled")) {
		CCHbeat = Heartbeat_New();
		if(CCHbeat) {
			Heartbeat_SetDomain(CCHbeat, "classicube.net");
			Heartbeat_SetRequestPath(CCHbeat, "/server/heartbeat/");
			Heartbeat_SetPlayURL(CCHbeat, "http://www.classicube.net/server/play/");
			Heartbeat_SetPublic(CCHbeat, Config_GetBoolByKey(Base_ConfigStore, "heartbeat-public"));
			Heartbeat_SetDelay(CCHbeat, Config_GetInt8ByKey(Base_ConfigStore, "heartbeat-delay") * 1000);
			if(Heartbeat_Run(CCHbeat)) return;
		}

		Log_Warn("Failed to start ClassiCube heartbeat!");
	}
}

void Base_HeartbeatStop(void) {
	if(CCHbeat) {
		Heartbeat_Close(CCHbeat);
		CCHbeat = NULL;
	}
}
