#include <core.h>
#include "base_config.h"
#include "base_lists.h"
#include "base_itf.h"

extern BList Base_Operators, Base_Bans;

BaseItf BaseController = {
	.isIsBanList = Base_IsBanned,
	.isInOPList = Base_IsOP,

	.addToBanList = Base_AddBan,
	.addToOPList = Base_AddOP,

	.removeFromBanList = Base_RemoveBan,
	.removeFromOPList = Base_RemoveOP
};
