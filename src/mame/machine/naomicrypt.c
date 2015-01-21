
/* Sega 'M1' type encryption ( using an Actel A54SX32 )

 see naomim1.c for Naomi hokup details

 The keys here are totally arbitrary and have nothing to do with the real keys.

 used by
 Naomi
 Hikaru



 */


#include "emu.h"
#include "naomicrypt.h"

struct game_keys
{
	const char *name;             /* game driver name */
	const UINT32 key;

};

static const struct game_keys keys_table[] =
{
	// name             key              gameid #         year
// M1
	{ "tduno2",          0x2f6f0f8d }, // 840-0022    2000
	{ "qmegamis",        0x96489bcd }, // 840-0030    2000
	{ "gram2000",        0x3f5c807f }, // 840-0039    2000
	{ "vtenis2c",        0x43472d2d }, // 840-0084    2001
	{ "shootopl",        0xa77cf3a0 }, // 840-0098    2002
	{ "vf4evoct",        0xcdb05b1e }, // 840-0106    2002
	{ "shootpl",         0xcde98d9d }, // 840-0128    2002
	{ "shootplm",        0xcde98d9d }, // 840-0136    2002
	{ "kick4csh",        0xc9570882 }, // 840-0140    2004
	{ "mtkob2",          0x3892fb3a }, // 840-0150    2003
	{ "mvsc2",           0x7c6e8bc1 }, // 841-0007-02 2000
//	sgnascar  (Hikaru)
	{ NULL, 0 }    // end of table
};


UINT32 get_naomi_key(running_machine &machine)
{
	const char *gamename = machine.system().name;
	const struct game_keys *k = &keys_table[0];

	while (k->name)
	{
		if (strcmp(k->name, gamename) == 0)
		{
			// we have a proper key so return it
			return k->key;
		}
		++k;
	}

	printf("get_naomi_key : KEY NOT FOUND\n");

	return 0;
}
