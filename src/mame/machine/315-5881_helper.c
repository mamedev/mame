
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
	// name              key              gameid #    year     chip label     platform
	{ "twcup98",         0x05200913 }, // 25209801    1998     317-5039-COM   ST-V
	{ "astrass",         0x052e2901 }, // 25349801    1998     317-5040-COM   ST-V      (yes, the 317-5040-COM chip was reused for 3 different games and on both Naomi and ST-V!)
	{ "rsgun",           0x05272d01 }, //             1998     317-5041-COM   ST-V
	{ "sss",             0x052b6901 }, //             1998     317-5042-COM   ST-V
	{ "elandore",        0x05226d41 }, //             1998     317-5043-COM   ST-V
	{ "ffreveng",        0x0524ac01 }, //             1998     317-5049-COM   ST-V

	{ "dybbnao",         0x080e6ae1 }, // 840-0001    1998     317-0246-JPN   Naomi
	{ "crzytaxi",        0x080d2f45 }, // 840-0002    1999     317-0248-COM   Naomi
	{ "zombrvn",         0x08012b41 }, // 840-0003    1999     317-0249-COM   Naomi
	{ "ringout",         0x080b1e40 }, // 840-0004    1999     317-0250-COM   Naomi
	{ "alpilota",        0x08070e41 }, // 840-0005    1999     317-0251-COM   Naomi
	{ "alpiltdx",        0x08070e41 }, // 834-?????   1999     317-0251-COM   Naomi
	{ "ggram2",          0x08074a61 }, // 840-0007    1999     317-0253-JPN   Naomi
	{ "f355",            0x080e8f84 }, // 834-13842   1999     317-0254-COM   Naomi
	{ "tduno",           0x08028ea5 }, // 840-0008    1999     317-0255-JPN   Naomi
	{ "toyfight",        0x0802ca85 }, // 840-0011    1999     317-0257-COM   Naomi
	{ "vs2_2k",          0x08088b08 }, // 840-0010    1999     317-0258-COM   Naomi
	{ "smlg99",          0x08048a01 }, // 840-0012    1999     317-0259-COM   Naomi
	{ "derbyoc",         0x080fee35 }, // 840-0016    1999     317-0262-JPN   Naomi
	{ "vtennis",         0x0803eb15 }, // 840-0015    1999     317-0263-COM   Naomi
	{ "jambo",           0x080fab95 }, // 840-0013    1999     317-0264-COM   Naomi
	{ "mazan",           0x080fea94 }, // 25869812    2002     317-0266-COM   Naomi
	{ "mazana",          0x080fea94 }, // 25869812    2002     317-0266-COM   Naomi
	{ "otrigger",        0x080fea94 }, // 840-0017    1999     317-0266-COM   Naomi
	{ "f355twin",        0x0806efd4 }, // 834-13950   1999     317-0267-COM   Naomi
	{ "sgtetris",        0x0808ae51 }, // 840-0018    1999     317-0268-COM   Naomi
	{ "dybb99",          0x0804ae71 }, // 840-0019    1999     317-0269-JPN   Naomi
	{ "samba",           0x080a8b5d }, // 840-0020    1999     317-0270-COM   Naomi
	{ "sambap",          0x080a8b5d }, // 840-0020    1999     317-0270-COM   Naomi
	{ "virnbao",         0x08068b58 }, // 840-0021    2000     317-0271-COM   Naomi
	{ "virnbap",         0x08068b58 }, // 840-0021    2000     317-0271-COM   Naomi
	{ "18wheelr",        0x0807cf54 }, // 840-0023    2000     317-0273-COM   Naomi
	{ "18wheels",        0x0807cf54 }, // 840-0036    2000     317-0273-COM   Naomi
	{ "18wheelu",        0x0807cf54 }, // 840-0037    2000     317-0273-COM   Naomi
	{ "marstv",          0x080b8ef5 }, // 840-0025    1999     317-0274-JPN   Naomi
	{ "vonot",           0x08010715 }, // 840-0028    2000     317-0279-COM   Naomi
	{ "sstrkfgt",        0x08132303 }, // 840-0035    2000     317-0281-COM   Naomi
	{ "sstrkfgta",       0x08132303 }, // 840-0035    2000     317-0281-COM   Naomi
	{ "wwfroyal",        0x081627c3 }, // 840-0040    2000     317-0285-COM   Naomi
	{ "slasho",          0x081a66ca }, // 840-0041    2000     317-0286-COM   Naomi
	{ "f355twn2",        0x081666c6 }, // 840-0042    2001     317-0287-COM   Naomi
	{ "crackndj",        0x081c2347 }, // 840-0043    2000     317-0288-COM   Naomi
	{ "csmash",          0x08103347 }, // 840-0044    2000     317-0289-COM   Naomi
	{ "csmasho",         0x08103347 }, // 840-0044    2000     317-0289-COM   Naomi
	{ "alienfnt",        0x08174343 }, // 840-0048    2001     317-0293-COM   Naomi
	{ "alienfnta",       0x08174343 }, // 840-0048    2001     317-0293-COM   Naomi
	{ "samba2k",         0x081702cf }, // 840-0047    2000     317-0295-COM   Naomi
	{ "wldrider",        0x0ce7a703 }, // 840-0046    2001     317-0301-COM   Naomi 2
	{ "vstrik3c",        0x0cee834a }, // 840-0061    2001     317-0310-COM   Naomi 2
	{ "vstrik3cb",       0x0cee834a }, // 840-0061    2001     317-0310-COM   Naomi 2
	{ "crackdj2",        0x08428247 }, // 840-0068    2001     317-0311-COM   Naomi
	{ "clubkrtc",        0x0ce7d742 }, // 840-0062    2001     317-0313-COM   Naomi 2
	{ "clubkrtd",        0x0ce7d742 }, // 840-0062    2001     317-0313-COM   Naomi 2
	{ "clubkrte",        0x0ce7d742 }, // 840-0062    2001     317-0313-COM   Naomi 2
	{ "inunoos",         0x094bc3e3 }, // 840-0073    2001     317-0316-JPN   Naomi
	{ "vf4cart",         0x0eef2f96 }, // 840-0080    2002     317-0324-COM   Naomi 2
	{ "toukon4",         0x052e2901 }, // 25349801    2000     317-5040-COM   Naomi
	{ "wldkicks",        0x052e2901 }, // 25209801    2000     317-5040-COM   Naomi
	{ "wldkicksa",       0x052e2901 }, // 25209801    2000     317-5040-COM   Naomi
	{ "wldkicksb",       0x052e2901 }, // 25209801    2000     317-5040-COM   Naomi
	{ "pstone",          0x000e69c1 }, // 841-0001    1999     317-5046-COM   Naomi
	{ "suchie3",         0x000368e1 }, // 841-0002    1999     317-5047-JPN   Naomi
	{ "doa2",            0x0008ad01 }, // 841-0003    1999     317-5048-COM   Naomi
	{ "doa2m",           0x0008ad01 }, // 841-0003    1999     317-5048-COM   Naomi
	{ "shangril",        -1         }, // 841-0004    1999     317-5050-JPN   Naomi     seems not used by game
	{ "spawn",           0x00078d01 }, // 841-0005    1999     317-5051-COM   Naomi
	{ "puyoda",          0x000acd40 }, // 841-0006    1999     317-5052-COM   Naomi
	{ "pstone2",         0x000b8dc0 }, // 841-0008    2000     317-5054-COM   Naomi
	{ "capsnk",          0x00000000 }, // 841-0011    2000     317-5059-COM   Naomi
	{ "capsnka",         0x00000000 }, // 841-0011    2000     317-5059-COM   Naomi
	{ "capsnkb",         0x00000000 }, // 841-0011    2000     317-5059-COM   Naomi
	{ "cspike",          0x000e2010 }, // 841-0012    2000     317-5060-COM   Naomi
	{ "ggx",             0x00076110 }, // 841-0013    2000     317-5063-COM   Naomi
	{ "gwing2",          0x000b25d0 }, // 841-0014    2000     317-5064-COM   Naomi
	{ "pjustic",         0x000725d0 }, // 841-0015    2000     317-5065-COM   Naomi
	{ "deathcox",        0x000b64d0 }, // 841-0016    2000     317-5066-COM   Naomi
	{ "ninjaslt",        0x000ca510 }, // 25469801    2000     317-5068-COM   Naomi
	{ "ninjaslt1",       0x000ca510 }, // 25469801    2000     317-5068-COM   Naomi
	{ "ninjaslt2",       0x000ca510 }, // 25469801    2000     317-5068-COM   Naomi
	{ "ninjaslt4",       0x000ca510 }, // 25469801    2000     317-5068-COM   Naomi
	{ "gundmct",         0x000e8010 }, // 841-0017    2001     317-5070-COM   Naomi
	{ "hmgeo",           0x00038510 }, // HMG016007   2001     317-5071-COM   Naomi
	{ "zerogu2",         0x0007c010 }, // 841-0020    2001     317-5073-COM   Naomi
	{ "gunsur2",         0x000680d0 }, // 25709801    2001     317-5075-COM   Naomi
	{ "gunsur2e",        0x000680d0 }, // 25709801    2001     317-5075-COM   Naomi

	{ "podrace",         0x0903dad5 }, // 834-14002   2001     317-0277-COM   Hikaru
	{ "airtrix",         0x091b02c7 }, // 834-14149   2000     317-0294-COM   Hikaru
	{ "pharrier",        0x0912c68a }, // 834-14144   2001     317-0297-COM   Hikaru

	{ "dynamcop",        0x0c2a4a93 }, //             1998     317-0236-COM   Model 2
	{ "dyndeka2",        0x0c2a4a93 }, //             1998     317-0236-COM   Model 2
	{ "dynamcopb",       0x0c2a4a93 }, //             1998     317-0236-COM   Model 2
	{ "dyndeka2b",       0x0c2a4a93 }, //             1998     317-0236-COM   Model 2
	{ "dynamcopc",       0x0c2a4a93 }, //             1998     317-0236-COM   Model 2
	{ "zerogun",         0x042c0d13 }, //             1997     317-5038-COM   Model 2
	{ "zerogunj",        0x042c0d13 }, //             1997     317-5038-COM   Model 2
	{ "zeroguna",        0x042c0d13 }, //             1997     317-5038-COM   Model 2
	{ "zerogunaj",       0x042c0d13 }, //             1997     317-5038-COM   Model 2
	{ "pltkids",         0x042e2dc1 }, //             1998     317-5044-COM   Model 2
	{ "pltkidsa",        0x042e2dc1 }, //             1998     317-5044-COM   Model 2

	{ "von2",            0x092a0e97 }, //             ????     317-0234-COM   Model 3
	{ "von254g",         0x092a0e97 }, //             ????     317-0234-COM   Model 3
	{ "fvipers2",        0x09260e96 }, //             ????     317-0235-COM   Model 3
	{ "vs298",           0x09234e96 }, //             ????     317-0237-COM   Model 3
	{ "dirtdvls",        0x09290f17 }, //             ????     317-0238-COM   Model 3
	{ "dirtdvlsa",       0x09290f17 }, //             ????     317-0238-COM   Model 3
	{ "daytona2",        0x09250e16 }, //             ????     317-0239-COM   Model 3
	{ "spikeout",        0x092f2b04 }, //             ????     317-0240-COM   Model 3
	{ "swtrilgy",        0x11272a01 }, //             ????     317-0241-COM   Model 3
	{ "swtrilgya",       0x11272a01 }, //             ????     317-0241-COM   Model 3
	{ "oceanhun",        0x092b6a01 }, //             ????     317-0242-COM   Model 3
	{ "magtruck",        0x09266e45 }, //             ????     317-0243-COM   Model 3
	{ "lamachin",        0x092a2bc5 }, //             ????     317-0244-COM   Model 3
	{ "vs299",           0x09222ac8 }, //             ????     317-0245-COM   Model 3
	{ "vs2v991",         0x09222ac8 }, //             ????     317-0245-COM   Model 3
	{ "vs299b",          0x09222ac8 }, //             ????     317-0245-COM   Model 3
	{ "vs299a",          0x09222ac8 }, //             ????     317-0245-COM   Model 3
	{ "spikeofe",        0x09236fc8 }, //             ????     317-0247-COM   Model 3
	{ "eca",             0x0923aa91 }, //             ????     317-0265-COM   Model 3
	{ "ecax",            0x0923aa91 }, //             ????     317-0265-COM   Model 3
	{ "ecap",            0x0923aa91 }, //             ????     317-0265-COM   Model 3
	{ "dayto2pe",        -1         }, //             ????     317-5045-COM   Model 3
	
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
