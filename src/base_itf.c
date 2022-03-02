#include <core.h>
#include "base_lists.h"
#include "base_itf.h"

extern BList Base_Operators, Base_Bans;

static cs_bool IsInBan(cs_str name) {
	return Base_CheckList(&Base_Bans, name);
}

static cs_bool IsInOP(cs_str name) {
	return Base_CheckList(&Base_Operators, name);
}

static cs_bool AddToBan(cs_str name) {
	return Base_AddList(&Base_Bans, name);
}

static cs_bool AddToOP(cs_str name) {
	return Base_AddList(&Base_Operators, name);
}

static cs_bool RemoveFromBan(cs_str name) {
	return Base_RemoveList(&Base_Bans, name);
}

static cs_bool RemoveFromOP(cs_str name) {
	return Base_RemoveList(&Base_Operators, name);
}

BaseItf BaseController = {
	.isIsBanList = IsInBan,
	.isInOPList = IsInOP,
	
	.addToBanList = AddToBan,
	.addToOPList = AddToOP,

	.removeFromBanList = RemoveFromBan,
	.removeFromOPList = RemoveFromOP
};
