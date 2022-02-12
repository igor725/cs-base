#include <core.h>
#include <log.h>
#include <config.h>
#include <world.h>
#include <timer.h>
#include <assoc.h>
#include <client.h>

extern CStore *Base_ConfigStore;
AssocType DayNightType;
const cs_int32 DayLength = 360,
NightLength = 480;

static struct _ColorPreset {
	Color3 diffuse, ambient,
	fog, cloud, sky;
} presets[] = {
	{
		{0xC0, 0xC0, 0xC0},
		{0x80, 0x70, 0x70},
		{0xFF, 0x92, 0x00},
		{0xC0, 0x90, 0x90},
		{0x10, 0x10, 0x80}
	},
	{
		{0xFF, 0xFF, 0xFF},
		{0x9B, 0x9B, 0x9B},
		{0xB9, 0xEC, 0xFF},
		{0xFF, 0xFF, 0xFF},
		{0x99, 0xCC, 0xFF}
	},
	{
		{0x80, 0x90, 0xA0},
		{0x68, 0x68, 0x70},
		{0x10, 0x10, 0x20},
		{0x40, 0x40, 0x40},
		{0x0A, 0x0A, 0x18}
	}
};

static void installColor(World *world, struct _ColorPreset *preset) {
	World_Lock(world, 0);
	World_SetEnvColor(world, WORLD_COLOR_DIFFUSE, &preset->diffuse);
	World_SetEnvColor(world, WORLD_COLOR_AMBIENT, &preset->ambient);
	World_SetEnvColor(world, WORLD_COLOR_FOG, &preset->fog);
	World_SetEnvColor(world, WORLD_COLOR_CLOUD, &preset->cloud);
	World_SetEnvColor(world, WORLD_COLOR_SKY, &preset->sky);
	World_FinishEnvUpdate(world);
	World_Unlock(world);
}

static void UpdateWorldTime(World *world) {
	cs_int32 *timer = Assoc_GetPtr(world, DayNightType);
	if(!timer) timer = Assoc_AllocFor(world, DayNightType, 1, sizeof(cs_int32));

	if(*timer == 0) {
		installColor(world, &presets[0]);
	} else if(*timer == NightLength / 4) {
		installColor(world, &presets[1]);
	} else if(*timer == NightLength / 4 + DayLength) {
		installColor(world, &presets[0]);
	} else if(*timer == NightLength / 4 + DayLength + DayLength / 4) {
		installColor(world, &presets[2]);
	} else if(*timer == NightLength + DayLength + DayLength / 4)
		*timer = -1;

	(*timer)++;
}

TIMER_FUNC(DNCycle) {
	(void)ticks; (void)left;
	if(Config_GetBool((CEntry *)ud)) {
		AListField *tmp;
		List_Iter(tmp, World_Head) {
			World *world = (World *)AList_GetValue(tmp).ptr;
			UpdateWorldTime(world);
		}
	}
}

void Base_OnWorldRemoved(void *param) {
	Assoc_Remove(param, DayNightType);
}

void Base_DayNight(void) {
	CEntry *ent = Config_GetEntry(Base_ConfigStore, "time-cycle");
	DayNightType = Assoc_NewType(ASSOC_BIND_WORLD);
	if(DayNightType >= 0) Timer_Add(-1, 1000, DNCycle, ent);
}
