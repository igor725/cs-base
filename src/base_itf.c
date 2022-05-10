#include "base_config.h"
#include "base_lists.h"
#include "base_itf.h"

extern BList Base_Operators, Base_Bans;
extern CStore *Base_ConfigStore;

BaseItf BaseController = {
	.isBanned = Base_IsBanned,
	.isOperator = Base_IsOP,

	.banUser = Base_AddBan,
	.opUser = Base_AddOP,

	.unbanUser = Base_RemoveBan,
	.deopUser = Base_RemoveOP,

	.getConfig = Base_GetConfig
};
