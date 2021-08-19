#include <core.h>
#include <log.h>
#include <config.h>
#include <heartbeat.h>

extern CStore *Base_ConfigStore;
EXP Heartbeat *Base_HeartbeatStruct;
Heartbeat *Base_HeartbeatStruct = NULL;

void Base_Heartbeat(void) {
	if(Config_GetBoolByKey(Base_ConfigStore, "heartbeat-cc-enabled")) {
		Base_HeartbeatStruct = (Heartbeat *)Memory_Alloc(1, sizeof(Heartbeat));
		Base_HeartbeatStruct->templ = "/server/heartbeat/?name=%s&port=%d&users=%d&max=%d&salt=%s&public=%s&web=true&software=%s";
		Base_HeartbeatStruct->playURL = "http://www.classicube.net/server/play/";
		Base_HeartbeatStruct->domain = "classicube.net";
		Base_HeartbeatStruct->secretfile = "secret.txt";
		Base_HeartbeatStruct->isSecure = true;

		Base_HeartbeatStruct->delay = Config_GetInt8ByKey(Base_ConfigStore, "heartbeat-delay") * 1000;
		Base_HeartbeatStruct->isPublic = Config_GetBoolByKey(Base_ConfigStore, "heartbeat-public");

		if(Heartbeat_Add(Base_HeartbeatStruct))
			Heartbeat_AddKeyChecker(Heartbeat_VanillaKeyChecker);
		else
			Log_Info("Failed to start ClassiCube heartbeat!");
	}
}