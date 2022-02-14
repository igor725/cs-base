#include <core.h>
#include <event.h>
#include <config.h>
#include <block.h>

extern CStore *Base_ConfigStore;
CEntry *tntent = NULL;

void Base_OnBlockPlace(void *param) {
	onBlockPlace *a = (onBlockPlace *)param;

	if(a->id == BLOCK_TNT) {
		a->id = BLOCK_AIR;
		if(Config_GetBool(tntent) && !Client_IsOP(a->client))
			return;
		
		World *world = Client_GetWorld(a->client);
		BulkBlockUpdate upd = {
			.world = world,
			.autosend = true
		};

		World_Lock(world, 0);
		
		for(ClientID i = 0; i < MAX_CLIENTS; i++) {
			Client *client = Clients_List[i];
			if(client && Client_IsInWorld(client, world))
				Client_SetNoFlush(client, true);
		}

		SVec pos;
		for(pos.x = a->pos.x - 10; pos.x < a->pos.x + 10; pos.x++) {
			for(pos.y = a->pos.y - 10; pos.y < a->pos.y + 10; pos.y++) {
				for(pos.z = a->pos.z - 10; pos.z < a->pos.z + 10; pos.z++) {
					cs_uint32 offset = World_GetOffset(world, &pos);
					Block_BulkUpdateAdd(&upd, offset, BLOCK_AIR);
					World_SetBlockO(world, offset, BLOCK_AIR);
				}
			}
		}

		for(ClientID i = 0; i < MAX_CLIENTS; i++) {
			Client *client = Clients_List[i];
			if(client && Client_IsInWorld(client, world))
				Client_SetNoFlush(client, false);
		}

		Block_BulkUpdateSend(&upd);
		World_Unlock(world);
	}
}

void Base_TNT(void) {
	tntent = Config_GetEntry(Base_ConfigStore, "tnt-deny");
}
