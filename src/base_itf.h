#ifndef CSBASEITF_H
#define CSBASEITF_H
#include <core.h>
#include <types/config.h>

#define BASE_ITF_VERSION 1
#define BASE_ITF_NAME "BaseController_v1"

typedef struct _BaseItf {
	cs_bool (*isBanned)(cs_str name);
	cs_bool (*isOperator)(cs_str name);

	cs_bool (*banUser)(cs_str name);
	cs_bool (*opUser)(cs_str name);

	cs_bool (*unbanUser)(cs_str name);
	cs_bool (*deopUser)(cs_str name);

	CStore *(*getConfig)(void);
} BaseItf;
#endif
