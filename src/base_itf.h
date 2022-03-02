#ifndef CSBASEITF_H
#define CSBASEITF_H
#include <core.h>

#define BASE_ITF_NAME "BaseController_v1"

typedef struct _BaseItf {
	cs_bool (*isIsBanList)(cs_str name);
	cs_bool (*isInOPList)(cs_str name);

	cs_bool (*addToBanList)(cs_str name);
	cs_bool (*addToOPList)(cs_str name);

	cs_bool (*removeFromBanList)(cs_str name);
	cs_bool (*removeFromOPList)(cs_str name);
} BaseItf;

extern BaseItf BaseController;
#endif
