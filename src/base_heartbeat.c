#include <core.h>
#include <log.h>
#include <config.h>
#include <heartbeat.h>

extern CStore *Base_ConfigStore;

void Base_Heartbeat(void) {
	if(Config_GetBoolByKey(Base_ConfigStore, "heartbeat-cc-enabled")) {
		Heartbeat *beat = Heartbeat_New();
		if(beat) {
			Heartbeat_SetDomain(beat, "classicube.net");
			Heartbeat_SetRequestPath(beat, "/server/heartbeat/");
			Heartbeat_SetPlayURL(beat, "http://www.classicube.net/server/play/");
			Heartbeat_SetPublic(beat, Config_GetBoolByKey(Base_ConfigStore, "heartbeat-public"));
			Heartbeat_SetDelay(beat, Config_GetInt8ByKey(Base_ConfigStore, "heartbeat-delay") * 1000);
			if(Heartbeat_Run(beat)) return;
		}

		Log_Warn("Failed to start ClassiCube heartbeat!");
	}
}
