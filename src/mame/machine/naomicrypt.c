
/* Naomi cartridge encryption

 see naomim1.c, naomim2.c and naomim4.c for implementation details

 The keys here are totally arbitrary and have nothing to do with the real keys.

 Atomiswave is significantly different and not listed here.
 Naomi GD-ROMs use DES encryption, and the keys are stored as part of the PIC dumps instead.

 the Naomi schemes are clearly related to CPS2
 the ST-V scheme could also be related to this.

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
// M2
	{ "wldkicks",        0x00ae2901 }, // 25209801    2000
	{ "toukon4",         0x012e2901 }, // 25349801    2000
	{ "ninjaslt",        0x000ca510 }, // 25469801    2000
	{ "ninjaslt4",       0x000ca510 }, // 25469801    2000
	{ "gunsur2e",        0x000680d0 }, // 25709801    2001
	{ "mazan",           0x000fea94 }, // 25869812    2002
	{ "f355twin",        0x0006efd4 }, // 834-13950   1999
	{ "alpiltdx",        0x00070e41 }, // 834-?????   1999
	{ "f355twn2",        0x001666c6 }, // 834-?????   2001
	{ "crzytaxi",        0x000d2f45 }, // 840-0002    1999
	{ "zombrvn",         0x00012b41 }, // 840-0003    1999
	{ "ringout",         0x000b1e40 }, // 840-0004    1999
	{ "alpilota",        0x00070e41 }, // 840-0005    1999
	{ "ggram2",          0x00074a61 }, // 840-0007    1999
	{ "vs2_2k",          0x00088b08 }, // 840-0010    1999
	{ "toyfight",        0x0002ca85 }, // 840-0011    1999
	{ "smlg99",          0x00048a01 }, // 840-0012    1999
	{ "jambo",           0x000fab95 }, // 840-0013    1999
	{ "vtennis",         0x0003eb15 }, // 840-0015    1999
	{ "derbyoc",         0x000fee35 }, // 840-0016    1999
	{ "otrigger",        0x000fea94 }, // 840-0017    1999
	{ "sgtetris",        0x0008ae51 }, // 840-0018    1999
	{ "dybb99",          0x00048a01 }, // 840-0019    1999
	{ "samba",           0x000a8b5d }, // 840-0020    1999
	{ "sambap",          0x000a8b5d }, // 840-0020    1999
	{ "virnbao",         0x00068b58 }, // 840-0021    2000
	{ "18wheelr",        0x0007cf54 }, // 840-0023    2000
	{ "marstv",          0x000b8ef5 }, // 840-0025    1999
	{ "vonot",           0x00010715 }, // 840-0028    2000
	{ "sstrkfgt",        0x00132303 }, // 840-0035    2000
	{ "18wheels",        0x0007cf54 }, // 840-0036    2000
	{ "wwfroyal",        0x001627c3 }, // 840-0040    2000
	{ "slasho",          0x001a66ca }, // 840-0041    2000
	{ "crackndj",        0x001c2347 }, // 840-0043    2000
	{ "csmash",          0x00103347 }, // 840-0044    2000
	{ "csmasho",         0x00103347 }, // 840-0044    2000
	{ "samba2k",         0x001702cf }, // 840-0047    2000
	{ "alienfnt",        0x00174343 }, // 840-0048    2001
	{ "alienfnta",       0x00174343 }, // 840-0048    2001
	{ "crackdj2",        0x00428247 }, // 840-0068    2001
	{ "vf4cart",         0x02ef2f96 }, // 840-0080    2002
	{ "pstone",          0x000e69c1 }, // 841-0001    1999
	{ "suchie3",         0x000368e1 }, // 841-0002    1999
	{ "doa2",            0x0008ad01 }, // 841-0003    1999
	{ "doa2m",           0x0008ad01 }, // 841-0003    1999
	{ "spawn",           0x00078d01 }, // 841-0005    1999
	{ "puyoda",          0x000acd40 }, // 841-0006    1999
	{ "pstone2",         0x000b8dc0 }, // 841-0008    2000
	{ "capsnk",          0x00000000 }, // 841-0011    2000
	{ "capsnka",         0x00000000 }, // 841-0011    2000
	{ "capsnkb",         0x00000000 }, // 841-0011    2000
	{ "cspike",          0x000e2010 }, // 841-0012    2000
	{ "ggx",             0x00076110 }, // 841-0013    2000
	{ "gwing2",          0x000b25d0 }, // 841-0014    2000
	{ "pjustic",         0x000725d0 }, // 841-0015    2000
	{ "deathcox",        0x000b64d0 }, // 841-0016    2000
	{ "gundmct",         0x000e8010 }, // 841-0017    2001
	{ "zerogu2",         0x0007c010 }, // 841-0020    2001
	{ "hmgeo",           0x00038510 }, // HMG016007   2001
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
