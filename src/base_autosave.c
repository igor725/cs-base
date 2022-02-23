#include <core.h>
#include <log.h>
#include <config.h>
#include <str.h>
#include <world.h>
#include <timer.h>
#include <list.h>

extern CStore *Base_ConfigStore;

TIMER_FUNC(AutoSave) {
	(void)left; (void)ticks; (void)ud;
	AListField *tmp;
	List_Iter(tmp, World_Head) {
		World *world = (World *)AList_GetValue(tmp).ptr;
		if(World_IsInMemory(world)) continue;
		// Думаю, не сильно интересно, произошла ли тут какая ошибка
		World_Save(world);
	}
}

static cs_uint32 ParseDelayValue(cs_str str) {
	cs_char value[6] = {0};
	cs_uint32 i = 0, delay = 0;

	do {
		if(i < 6)
			value[i++] = *str++;
		else return 0;
	} while(ISNUM(*str));

	delay = (cs_uint32)String_ToInt(value);
	switch(*str) {
		case 'm':
			delay *= 60000;
			break;

		case 'h':
			delay *= 3600000;
			break;

		case 'd':
			delay *= 86400000;
			break;

		default:
			return 0;
	}

	return delay;
}

void Base_AutoSave(void) {
	cs_str dvalue = Config_GetStrByKey(Base_ConfigStore, "autosave-delay");
	cs_uint32 delay = ParseDelayValue(dvalue);

	if(delay > 0)
		Timer_Add(-1, delay, AutoSave, NULL);
	else
		Log_Warn("Autosaving feature is disabled!");
}
