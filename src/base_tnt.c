#include <core.h>
#include <event.h>
#include <config.h>
#include <block.h>
#include <client.h>
#include <world.h>

extern CStore *Base_ConfigStore;
CEntry *tntpl = NULL, *tntbl = NULL;

void Base_OnBlockPlace(void *param) {
	onBlockPlace *a = (onBlockPlace *)param;

	if(a->id == BLOCK_TNT) {
		if(!tntpl || !tntbl) {
			tntpl = Config_GetEntry(Base_ConfigStore, "tnt-deny-place");
			tntbl = Config_GetEntry(Base_ConfigStore, "tnt-deny-blow");
			if(!tntpl || !tntbl) return;
		}
		if(Config_GetBool(tntpl)) {
			a->id = BLOCK_AIR;
			return;
		} else if(Config_GetBool(tntbl))
			return;

		World *world = Client_GetWorld(a->client);
		BulkBlockUpdate upd = {
			.world = world,
			.autosend = true
		};

		a->id = BLOCK_AIR;
		World_Lock(world, 0);

		SVec pos;
		for(pos.x = a->pos.x - 10; pos.x < a->pos.x + 10; pos.x++) {
			for(pos.y = a->pos.y - 10; pos.y < a->pos.y + 10; pos.y++) {
				for(pos.z = a->pos.z - 10; pos.z < a->pos.z + 10; pos.z++) {
					cs_uint32 offset = World_GetOffset(world, &pos);
					BlockID id = World_GetBlockO(world, offset);
					if(id != BLOCK_BEDROCK && (id < BLOCK_WATER || id > BLOCK_LAVA_STILL)) {
						Block_BulkUpdateAdd(&upd, offset, BLOCK_AIR);
						World_SetBlockO(world, offset, BLOCK_AIR);
					}
				}
			}
		}

		Block_BulkUpdateSend(&upd);
		World_Unlock(world);
	}
}
