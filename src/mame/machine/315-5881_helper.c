
/* Sega 315-5881 support file

  This encryption chip was used on various games running on
  ST-V
  Naomi
  Naomi 2
  Hikaru
  Model 2
  Model 3
  (more?)

  As the encryption isn't fully understood yet this file holds the current keys, these are subject to change.
 */


#include "emu.h"
#include "315-5881_helper.h"

struct game_keys
{
	const char *name;             /* game driver name */
	const INT64 key; // INT64 so we can use -1 for unknown keys

};

// chip numbers based on Guru's list at
// http://members.iinet.net.au/~lantra9jp1/gurudumps/sega-security/sega_security.html
// (todo , complete it )


static const struct game_keys keys_table[] =
{
	// name             key              gameid #     year     chip label     platform
	{ "twcup98",         0x05200913 }, // 25209801    1998     317-5039-COM   ST-V   (this is correct key, but not yet working with our code)
//	{ "twcup98",         -1         }, // 25209801    1998     317-5039-COM   ST-V
	{ "astrass",         0x052e2901 }, // 25349801    1998     317-5040-COM   ST-V   (yes, the 317-5040-COM chip was reused for 3 different games and on both Naomi and ST-V!)
	{ "wldkicks",        0x052e2901 }, // 25209801    2000     317-5040-COM   Naomi
	{ "toukon4",         0x052e2901 }, // 25349801    2000     317-5040-COM   Naomi
	{ "rsgun",           -1         }, //    	      1998     317-5041-COM   ST-V
	{ "sss",             -1         }, //    	      1998     317-5042-COM   ST-V
	{ "elandore",        -1         }, //    	      1998     317-5043-COM   ST-V
	{ "ffreveng",        -1         }, //    	      1998     317-5049-COM   ST-V
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
	{ "vf4cart",         0x06ef2f92 }, // 840-0080    2002
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

	{ "airtrix",         -1         }, //             ????  317-0294-COM  Hikaru
	{ "pharrier",        -1         }, //             ????  317-0297-COM  Hikaru
	{ "podrace",         -1         }, //             ????  317-0277-COM  Hikaru

	{ "vs298",           0x09234e96 }, //             ????  317-????-COM  Model 3
	{ "vs299",           0x09222ac8 }, //             ????  317-0245-COM  Model 3
	{ "swt",             0x11272a01 }, //             ????  317-0241-COM  Model 3

	{ "dynamcop",        0x0c2a4a93 }, //             ????  317-0236-COM  Model 2


	{ NULL, 0 }    // end of table
};


INT64 get_315_5881_key(running_machine &machine)
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
