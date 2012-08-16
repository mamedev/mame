/***************************************************************************

  megadriv.c

  Machine file to handle emulation of the Sega Mega Drive & Genesis in MESS.

    i2c games mapping table:

    game name                         |   SDA_IN   |  SDA_OUT   |     SCL    |  SIZE_MASK     | PAGE_MASK |
    ----------------------------------|------------|------------|------------|----------------|-----------|
    NBA Jam                           | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x00ff (24C02) |   0x03    | xx
    NBA Jam TE                        | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    | xx
    NBA Jam TE (32x)                  | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    |
    NFL Quarterback Club              | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    | xx
    NFL Quarterback Club 96           | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x07ff (24C16) |   0x07    | xx
    College Slam                      | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x1fff (24C64) |   0x07    | xx
    Frank Thomas Big Hurt Baseball    | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x1fff (24C64) |   0x07    | xx
    NHLPA Hockey 93                   | 0x200001-7 | 0x200001-7 | 0x200001-6 | 0x007f (24C01) |   0x03    | xx
    Rings of Power                    | 0x200001-7 | 0x200001-7 | 0x200001-6 | 0x007f (24C01) |   0x03    | xx
    Evander Holyfield's Boxing        | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
    Greatest Heavyweights of the Ring | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
    Wonder Boy V                      | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
    Sports Talk Baseball              | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
    Megaman - the Wily Wars           | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx **
    Micro Machines 2                  | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |
    Micro Machines Military           | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |
    Micro Machines 96                 | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x07ff (24C16) |   0x0f    |
    Brian Lara Cricket 96             | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x1fff (24C64) |   0x??*   |
    ----------------------------------|------------|------------|------------|----------------|-----------|

    * Notes: check these
    ** original Rockman Mega World (J) set uses normal backup RAM

    2008-09: Moved here cart code and custom mapper handlers. Hopefully,
        it will make painless the future merging with HazeMD
    2008-10: Fixed SRAM, matching as much as possible HazeMD. Some game is
        not detected, however. E.g. Sonic 3. Also some games seem false
        positives (e.g. Wonderboy 5: according to its headers it should
        have 1byte of SRAM). Better detection routines would be welcome.
    2009-06: Changed SRAM code, fixing more games, including Sonic 3. The
        false positives seem only some of the games using EEPROM. Todo
        list in mess/drivers/genesis.c includes EEPROM emulation.

***************************************************************************/


#include "emu.h"
#include "formats/imageutl.h"
#include "cpu/m68000/m68000.h"

#include "imagedev/cartslot.h"
#include "includes/megadriv.h"


/* PCB */
enum
{
    SEGA_STD = 0,

    SEGA_SRAM, SEGA_FRAM,

	CM_JCART, CM_JCART_SEPROM,   /* Codemasters PCB (J-Carts and SEPROM) */

	SSF2,                        /* Super Street Fighter 2 */
	GAME_KANDUME,                /* Game no Kandume Otokuyou */
	BEGGAR,                      /* Xin Qigai Wangzi uses different sram start address and has no valid header */

	// EEPROM
	SEGA_EEPROM,                 /* Wonder Boy V / Evander Holyfield's Boxing / Greatest Heavyweights of the Ring / Sports Talk Baseball / Megaman */
	NBA_JAM,                     /* NBA Jam */
	NBA_JAM_TE,                  /* NBA Jam TE / NFL Quarterback Club */
	NFL_QB_96,                   /* NFL Quarterback Club '96 */
	C_SLAM,                      /* College Slam / Frank Thomas Big Hurt Baseball */
	EA_NHLPA,                    /* NHLPA Hockey 93 / Rings of Power */
	CODE_MASTERS,                /* Micro Machines 2 / Military / 96 / Brian Lara Cricket 96 */

	MC_SUP19IN1,                 /* Super 19 in 1 */
	MC_SUP15IN1,                 /* Super 15 in 1 */
	MC_12IN1,                    /* 12 in 1 and a few more multicarts */
	BUGSLIFE,                    /* A Bug's Life */
	CHINFIGHT3,                  /* Chinese Fighters 3 */
	ELFWOR,                      /* Linghuan Daoshi Super Magician */
	KOF98,                       /* King of Fighters '98 */
	KOF99,                       /* King of Fighters '99 */
	LIONK2,                      /* Lion King 2 */
	LIONK3,                      /* Lion King 3 */
	SKINGKONG,                   /* Super King Kong 99 */
	SDK99,                       /* Super Donkey Kong 99 */
	MJLOVER,                     /* Mahjong Lover */
	MULAN,                       /* Hua Mu Lan - Mulan */
	POKEMON,                     /* Pocket Monster */
	POKEMON2,                    /* Pocket Monster 2 */
	KAIJU,                       /* Pokemon Stadium */
	RADICA,                      /* Radica TV games.. these probably should be a separate driver since they are a separate 'console' */
	REALTEC,                     /* Whac a Critter/Mallet legend, Defend the Earth, Funnyworld/Ballonboy */
	REDCLIFF,                    /* Romance of the Three Kingdoms - Battle of Red Cliffs, already decoded from .mdx format */
	REDCL_EN,                    /* The encoded version... */
	ROCKMANX3,                   /* Rockman X3 */
	SMB,                         /* Super Mario Bros. */
	SMB2,                        /* Super Mario Bros. 2 */
	SMOUSE,                      /* Smart Mouse */
	SOULBLAD,                    /* Soul Blade */
	SQUIRRELK,                   /* Squirrel King */
	SBUBBOB,                     /* Super Bubble Bobble */
	TOPFIGHTER,                  /* Top Fighter 2000 MK VIII */
	PSOLAR						 /* Pier Solar */
};

typedef struct _md_pcb  md_pcb;
struct _md_pcb
{
	const char              *pcb_name;
	int                     pcb_id;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const md_pcb pcb_list[] =
{
	{"SEGA-EEPROM", SEGA_EEPROM},
	{"SEGA-SRAM", SEGA_SRAM},
	{"SEGA-FRAM", SEGA_FRAM},

	{"CM-JCART", CM_JCART},
	{"CM-JCART-SEPROM", CM_JCART_SEPROM},
	{"CM-SEPROM", CODE_MASTERS},

	{"SSF2", SSF2},
	{"GAMEKANDUME", GAME_KANDUME},
	{"BEGGAR", BEGGAR},

	{"NBAJAM", NBA_JAM},
	{"NBAJAMTE", NBA_JAM_TE},
	{"NFLQB96", NFL_QB_96},
	{"CSLAM", C_SLAM},
	{"NHLPA", EA_NHLPA},

	{"LIONK3", LIONK3},
	{"SDK99", SDK99},
	{"SKINGKONG", SKINGKONG},
	{"REDCLIFF", REDCL_EN},
	{"RADICA", RADICA},
	{"KOF98", KOF98},
	{"KOF99", KOF99},
	{"SOULBLAD", SOULBLAD},
	{"MJLOVER", MJLOVER},
	{"SQUIRRELK", SQUIRRELK},
	{"SMOUSE", SMOUSE},
	{"SMB", SMB},
	{"SMB2", SMB2},
	{"KAIJU", KAIJU},
	{"CHINFIGHT3", CHINFIGHT3},
	{"LIONK2", LIONK2},
	{"BUGSLIFE", BUGSLIFE},
	{"ELFWOR", ELFWOR},
	{"ROCKMANX3", ROCKMANX3},
	{"SBUBBOB", SBUBBOB},
	{"REALTEC", REALTEC},
	{"MC_SUP19IN1", MC_SUP19IN1},
	{"MC_SUP15IN1", MC_SUP15IN1},
	{"MC_12IN1", MC_12IN1},
	{"TOPFIGHTER", TOPFIGHTER},
	{"POKEMON", POKEMON},
	{"POKEMON2", POKEMON2},
	{"MULAN", MULAN},
	{"PSOLAR", PSOLAR}
};

static int md_get_pcb_id(const char *pcb)
{
	int	i;

	for (i = 0; i < ARRAY_LENGTH(pcb_list); i++)
	{
		if (!mame_stricmp(pcb_list[i].pcb_name, pcb))
			return pcb_list[i].pcb_id;
	}

	return SEGA_STD;
}

#define MAX_MD_CART_SIZE 0x800000

/* where a fresh copy of rom is stashed for reset and banking setup */
#define VIRGIN_COPY_GEN 0xd00000


/*************************************
 *
 *  Handlers for custom mappers
 *
 *************************************/

/*************************************
 *  Super Street Fighter II
 *************************************/
static WRITE16_HANDLER( genesis_ssf2_bank_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	UINT8 *ROM = state->memregion("maincpu")->base();

	if ((state->m_md_cart.ssf2_lastoff != offset) || (state->m_md_cart.ssf2_lastdata != data))
	{
		state->m_md_cart.ssf2_lastoff = offset;
		state->m_md_cart.ssf2_lastdata = data;
		switch (offset << 1)
		{
			case 0x00: /* write protect register */ // this is not a write protect, but seems to do nothing useful but reset bank0 after the checksum test (red screen otherwise)
				if (data == 2)
				{
					memcpy(ROM + 0x000000, ROM + 0x400000 + (((data & 0xf) - 2) * 0x080000), 0x080000);
				}
				break;
			case 0x02: /* 0x080000 - 0x0FFFFF */
				memcpy(ROM + 0x080000, ROM + 0x400000 + ((data & 0xf) * 0x080000), 0x080000);
				break;
			case 0x04: /* 0x100000 - 0x17FFFF */
				memcpy(ROM + 0x100000, ROM + 0x400000 + ((data & 0xf) * 0x080000), 0x080000);
				break;
			case 0x06: /* 0x180000 - 0x1FFFFF */
				memcpy(ROM + 0x180000, ROM + 0x400000 + ((data & 0xf) * 0x080000), 0x080000);
				break;
			case 0x08: /* 0x200000 - 0x27FFFF */
				memcpy(ROM + 0x200000, ROM + 0x400000 + ((data & 0xf) * 0x080000), 0x080000);
				break;
			case 0x0a: /* 0x280000 - 0x2FFFFF */
				memcpy(ROM + 0x280000, ROM + 0x400000 + ((data & 0xf) * 0x080000), 0x080000);
				break;
			case 0x0c: /* 0x300000 - 0x37FFFF */
				memcpy(ROM + 0x300000, ROM + 0x400000 + ((data & 0xf) * 0x080000), 0x080000);
				break;
			case 0x0e: /* 0x380000 - 0x3FFFFF */
				memcpy(ROM + 0x380000, ROM + 0x400000 + ((data & 0xf) * 0x080000), 0x080000);
				break;
		}
	}
}

/*************************************
 *  Lion King 3, Super King Kong 99
 *  & Super Donkey Kong 99
 *************************************/
#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( l3alt_pdat_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.l3alt_pdat = data;
}

static WRITE16_HANDLER( l3alt_pcmd_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.l3alt_pcmd = data;
}
#endif

static READ16_HANDLER( l3alt_prot_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	int retdata = 0;

	offset &= 0x07;

	switch (offset)
	{

		case 2:

			switch (state->m_md_cart.l3alt_pcmd)
			{
				case 1:
					retdata = state->m_md_cart.l3alt_pdat >> 1;
					break;

				case 2:
					retdata = state->m_md_cart.l3alt_pdat >> 4;
					retdata |= (state->m_md_cart.l3alt_pdat & 0x0f) << 4;
					break;

				default:
					/* printf("unk prot case %d\n", l3alt_pcmd); */
					retdata =  (BIT(state->m_md_cart.l3alt_pdat, 7) << 0);
					retdata |= (BIT(state->m_md_cart.l3alt_pdat, 6) << 1);
					retdata |= (BIT(state->m_md_cart.l3alt_pdat, 5) << 2);
					retdata |= (BIT(state->m_md_cart.l3alt_pdat, 4) << 3);
					retdata |= (BIT(state->m_md_cart.l3alt_pdat, 3) << 4);
					retdata |= (BIT(state->m_md_cart.l3alt_pdat, 2) << 5);
					retdata |= (BIT(state->m_md_cart.l3alt_pdat, 1) << 6);
					retdata |= (BIT(state->m_md_cart.l3alt_pdat, 0) << 7);
					break;
			}
			break;

		default:
			logerror("protection read, unknown offset\n");
			break;
	}

/*  printf("%06x: l3alt_pdat_w %04x l3alt_pcmd_w %04x return %04x\n", activecpu_get_pc(), l3alt_pdat, l3alt_pcmd, retdata); */

	return retdata;
}

static WRITE16_HANDLER( l3alt_prot_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	offset &= 0x7;

	switch (offset)
	{
		case 0x0:
			state->m_md_cart.l3alt_pdat = data;
			break;
		case 0x1:
			state->m_md_cart.l3alt_pcmd = data;
			break;
		default:
			logerror("protection write, unknown offst\n");
			break;
	}
}

static WRITE16_HANDLER( l3alt_bank_w )
{
	offset &= 0x7;

	switch (offset)
	{
		case 0:
		{
			UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();
			/* printf("%06x data %04x\n",activecpu_get_pc(), data); */
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN + (data & 0xffff) * 0x8000], 0x8000);
		}
		break;

		default:
		logerror("unk bank w\n");
		break;

	}

}

/*************************************
 *  Whac a Critter/Mallet Legend,
 *  Defend the Earth, Funnyworld/Ballonboy
 *************************************/
static WRITE16_HANDLER( realtec_402000_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.realtec_bank_addr = 0;
	state->m_md_cart.realtec_bank_size = (data >> 8) & 0x1f;
}

static WRITE16_HANDLER( realtec_400000_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	int bankdata = (data >> 9) & 0x7;

	UINT8 *ROM = state->memregion("maincpu")->base();

	state->m_md_cart.realtec_old_bank_addr = state->m_md_cart.realtec_bank_addr;
	state->m_md_cart.realtec_bank_addr = (state->m_md_cart.realtec_bank_addr & 0x7) | bankdata << 3;

	memcpy(ROM, ROM + (state->m_md_cart.realtec_bank_addr * 0x20000) + 0x400000, state->m_md_cart.realtec_bank_size * 0x20000);
	memcpy(ROM + state->m_md_cart.realtec_bank_size * 0x20000, ROM + (state->m_md_cart.realtec_bank_addr * 0x20000) + 0x400000, state->m_md_cart.realtec_bank_size * 0x20000);
}

static WRITE16_HANDLER( realtec_404000_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	int bankdata = (data >> 8) & 0x3;
	UINT8 *ROM = state->memregion("maincpu")->base();

	state->m_md_cart.realtec_old_bank_addr = state->m_md_cart.realtec_bank_addr;
	state->m_md_cart.realtec_bank_addr = (state->m_md_cart.realtec_bank_addr & 0xf8) | bankdata;

	if (state->m_md_cart.realtec_old_bank_addr != state->m_md_cart.realtec_bank_addr)
	{
		memcpy(ROM, ROM + (state->m_md_cart.realtec_bank_addr * 0x20000)+ 0x400000, state->m_md_cart.realtec_bank_size * 0x20000);
		memcpy(ROM + state->m_md_cart.realtec_bank_size * 0x20000, ROM + (state->m_md_cart.realtec_bank_addr * 0x20000) + 0x400000, state->m_md_cart.realtec_bank_size * 0x20000);
	}
}

/*************************************
 *  Chinese Fighters 3
 *************************************/
static WRITE16_HANDLER( chifi3_bank_w )
{
	UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();

	if (data == 0xf100) // *hit player
	{
		int x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(ROM + x, ROM + 0x410000, 0x10000);
		}
	}
	else if (data == 0xd700) // title screen..
	{
		int x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(ROM + x, ROM + 0x470000, 0x10000);
		}
	}
	else if (data == 0xd300) // character hits floor
	{
		int x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(ROM + x, ROM + 0x430000, 0x10000);
		}
	}
	else if (data == 0x0000)
	{
		int x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(ROM + x, ROM + 0x400000 + x, 0x10000);
		}
	}
	else
	{
		logerror("%06x chifi3, bankw? %04x %04x\n", cpu_get_pc(&space->device()), offset, data);
	}

}

static READ16_HANDLER( chifi3_prot_r )
{
	UINT32 retdat;

	/* not 100% correct, there may be some relationship between the reads here
    and the writes made at the start of the game.. */

	/*
    04dc10 chifi3, prot_r? 2800
    04cefa chifi3, prot_r? 65262
    */

	if (cpu_get_pc(&space->device()) == 0x01782) // makes 'VS' screen appear
	{
		retdat = cpu_get_reg(&space->device(), M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(&space->device()) == 0x1c24) // background gfx etc.
	{
		retdat = cpu_get_reg(&space->device(), M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(&space->device()) == 0x10c4a) // unknown
	{
		return space->machine().rand();
	}
	else if (cpu_get_pc(&space->device()) == 0x10c50) // unknown
	{
		return space->machine().rand();
	}
	else if (cpu_get_pc(&space->device()) == 0x10c52) // relates to the game speed..
	{
		retdat = cpu_get_reg(&space->device(), M68K_D4) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(&space->device()) == 0x061ae)
	{
		retdat = cpu_get_reg(&space->device(), M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(&space->device()) == 0x061b0)
	{
		retdat = cpu_get_reg(&space->device(), M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else
	{
		logerror("%06x chifi3, prot_r? %04x\n", cpu_get_pc(&space->device()), offset);
	}

	return 0;
}

/*************************************
 *  Super 19 in 1 & Super 15 in 1
 *************************************/
static WRITE16_HANDLER( s19in1_bank )
{
	UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();
	memcpy(ROM + 0x000000, ROM + 0x400000 + ((offset << 1) * 0x10000), 0x80000);
}

/*************************************
 *  Kaiju? (Pokemon Stadium)
 *************************************/
static WRITE16_HANDLER( kaiju_bank_w )
{
	UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();
	memcpy(ROM + 0x000000, ROM + 0x400000 + (data & 0x7f) * 0x8000, 0x8000);
}

/*************************************
 *  Soulblade
 *************************************/
static READ16_HANDLER( soulb_400006_r )
{
//  printf("%06x soulb_400006_r\n",cpu_get_pc(&space->device()));
	return 0xf000;
}

static READ16_HANDLER( soulb_400002_r )
{
//  printf("%06x soulb_400002_r\n",cpu_get_pc(&space->device()));
	return 0x9800;
}

static READ16_HANDLER( soulb_400004_r )
{
//  return 0x9800;
//  printf("%06x soulb_400004_r\n",cpu_get_pc(&space->device()));
//
	return 0xc900;
//aa
//c9
}

/*************************************
 *  Mahjong Lover
 *************************************/
static READ16_HANDLER( mjlovr_prot_1_r )
{
	return 0x9000;
}

static READ16_HANDLER( mjlovr_prot_2_r )
{
	return 0xd300;
}

/*************************************
 *  Super Mario Bros
 *************************************/
static READ16_HANDLER( smbro_prot_r )
{
	return 0xc;
}


/*************************************
 *  Smart Mouse
 *************************************/
static READ16_HANDLER( smous_prot_r )
{
	switch (offset)
	{
		case 0: return 0x5500;
		case 1: return 0x0f00;
		case 2: return 0xaa00;
		case 3: return 0xf000;
	}

	return -1;
}

/*************************************
 *  Super Bubble Bobble MD
 *************************************/
static READ16_HANDLER( sbub_extra1_r )
{
	return 0x5500;
}

static READ16_HANDLER( sbub_extra2_r )
{
	return 0x0f00;
}

/*************************************
 *  King of Fighters 99
 *************************************/
static READ16_HANDLER( kof99_A13002_r )
{
	// write 02 to a13002.. shift right 1?
	return 0x01;
}

static READ16_HANDLER( kof99_00A1303E_r )
{
	// write 3e to a1303e.. shift right 1?
	return 0x1f;
}

static READ16_HANDLER( kof99_A13000_r )
{
	// no write, startup check, chinese message if != 0
	return 0x0;
}

/*************************************
 *  Radica controllers
 *************************************/
static READ16_HANDLER( radica_bank_select )
{
	int bank = offset & 0x3f;
	UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();
	memcpy(ROM, ROM + 0x400000 + (bank * 0x10000), 0x400000);
	return 0;
}

/*************************************
 *  ROTK Red Cliff
 *************************************/
static READ16_HANDLER( redclif_prot_r )
{
	return -0x56 << 8;
}

static READ16_HANDLER( redclif_prot2_r )
{
	return 0x55 << 8;
}

/*************************************
 *  Squirrel King
 *************************************/
static READ16_HANDLER( squirrel_king_extra_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	return state->m_md_cart.squirrel_king_extra;
}

static WRITE16_HANDLER( squirrel_king_extra_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.squirrel_king_extra = data;
}

/*************************************
 *  Lion King 2
 *************************************/
static READ16_HANDLER( lion2_prot1_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	return state->m_md_cart.lion2_prot1_data;
}

static READ16_HANDLER( lion2_prot2_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	return state->m_md_cart.lion2_prot2_data;
}

static WRITE16_HANDLER ( lion2_prot1_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.lion2_prot1_data = data;
}

static WRITE16_HANDLER ( lion2_prot2_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.lion2_prot2_data = data;
}

/*************************************
 *  Rockman X3
 *************************************/
static READ16_HANDLER( rx3_extra_r )
{
	return 0xc;
}

/*************************************
 *  King of Fighters 98
 *************************************/
static READ16_HANDLER( kof98_aa_r )
{
	return 0xaa00;
}

static READ16_HANDLER( kof98_0a_r )
{
	return 0x0a00;
}

static READ16_HANDLER( kof98_00_r )
{
	return 0x0000;
}

/*************************************
 *  Super Mario Bros 2
 *************************************/
static READ16_HANDLER( smb2_extra_r )
{
	return 0xa;
}

/*************************************
 *  A Bug's Life
 *************************************/
static READ16_HANDLER( bugl_extra_r )
{
	return 0x28;
}

/*************************************
 *  Linghuan Daoshi Super Magician
 *  (internal game name Elf Wor)
 *************************************/
static READ16_HANDLER( elfwor_400000_r )
{
	return 0x5500;
}

static READ16_HANDLER( elfwor_400002_r )
{
	return 0x0f00;
}

static READ16_HANDLER( elfwor_400004_r )
{
	return 0xc900;
}

static READ16_HANDLER( elfwor_400006_r )
{
	return 0x1800;
}

/*************************************
 *  Top Fighter - there is more to
 *  this.. No display at startup etc..
 *************************************/
static READ16_HANDLER( topfig_6BD294_r ) /* colours on title screen */
{
	static int x = -1;

	/*
     cpu #0 (PC=00177192): unmapped program memory word write to 006BD240 = 00A8 & 00FF
     cpu #0 (PC=0017719A): unmapped program memory word write to 006BD2D2 = 0098 & 00FF
     cpu #0 (PC=001771A2): unmapped program memory word read from 006BD294 & 00FF
     */

	if (cpu_get_pc(&space->device())==0x1771a2) return 0x50;
	else
	{
		x++;
		logerror("%06x topfig_6BD294_r %04x\n",cpu_get_pc(&space->device()), x);
		return x;
	}
}

static READ16_HANDLER( topfig_6F5344_r ) // after char select
{
	static int x = -1;

	if (cpu_get_pc(&space->device())==0x4C94E)
	{
		return cpu_get_reg(space->machine().device("maincpu"), (M68K_D0)) & 0xff;
	}
	else
	{
		x++;
		logerror("%06x topfig_6F5344_r %04x\n",cpu_get_pc(&space->device()), x);
		return x;
	}
}

static WRITE16_HANDLER( topfig_bank_w )
{
	UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();
	if (data == 0x002a)
	{
		memcpy(ROM + 0x060000, ROM + 0x570000, 0x8000); // == 0x2e*0x8000?!
		//  printf("%06x offset %06x, data %04x\n",cpu_get_pc(&space->device()), offset, data);

	}
	else if (data==0x0035) // characters ingame
	{
		memcpy(ROM + 0x020000, ROM + 0x5a8000, 0x8000); // == 0x35*0x8000
	}
	else if (data==0x000f) // special moves
	{
		memcpy(ROM + 0x058000, ROM + 0x478000, 0x8000); // == 0xf*0x8000
	}
	else if (data==0x0000)
	{
		memcpy(ROM + 0x060000, ROM + 0x460000, 0x8000);
		memcpy(ROM + 0x020000, ROM + 0x420000, 0x8000);
		memcpy(ROM + 0x058000, ROM + 0x458000, 0x8000);
		//  printf("%06x offset %06x, data %04x\n",cpu_get_pc(&space->device()), offset, data);
	}
	else
	{
		logerror("%06x offset %06x, data %04x\n", cpu_get_pc(&space->device()), offset, data);
	}

}

static READ16_HANDLER( topfig_645B44_r )
{
	//cpu #0 (PC=0004DE00): unmapped program memory word write to 00689B80 = 004A & 00FF
	//cpu #0 (PC=0004DE08): unmapped program memory word write to 00 = 00B5 & 00FF
	//cpu #0 (PC=0004DE0C): unmapped program memory word read from 00645B44 & 00FF

	return 0x9f;//0x25;
}

//cpu #0 (PC=0004CBAE): unmapped program memory word read from 006A35D4 & 00FF -- wants regD7



/*************************************
 *  12 in 1 and other multigame carts
 *************************************/
static WRITE16_HANDLER( mc_12in1_bank_w )
{
	UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();
	logerror("offset %06x", offset << 17);
	memcpy(ROM + 0x000000, ROM + VIRGIN_COPY_GEN + ((offset & 0x3f) << 17), 0x100000);
}



/*************************************
 *  Stub for TMSS bios bank
 *************************************/
static WRITE16_HANDLER( genesis_TMSS_bank_w )
{
	/* this probably should do more, like make Genesis V2 'die' if the SEGA string is not written promptly */
}

/*************************************
 *  Pier Solar banking
 *************************************/

static WRITE16_HANDLER( psolar_bank_w )
{
	UINT8 *ROM = space->machine().root_device().memregion("maincpu")->base();
	logerror("switch bank %02x, page %02x\n",offset, data);
	memcpy(&ROM[0x280000 + (0x80000 * offset)], &ROM[VIRGIN_COPY_GEN + (0x80000 * (data&0x0f))], 0x80000);
}

static WRITE16_HANDLER( psolar_unk_w )
{
	logerror("A13001 write %02x\n", data);
}

static int psolar_rdcnt = 0;

static READ16_HANDLER( psolar_hack_r )
{
	// ugly hack until we don't know much about game protection
	// first 3 reads from 15e6 return 0x00000010, then normal 0x00018010 value for crc check
	UINT16 res;
	if (psolar_rdcnt < 6) {
		psolar_rdcnt++;
		if (offset)
			res = 0x10;
		else
			res = 0x00;
	} else {
		if (offset)
			res = 0x8010;
		else
			res = 0x0001;
	}
	logerror("read 0x15e6 %d\n",psolar_rdcnt);
	return res;
}
/*************************************
 *
 *  Handlers for SRAM & EEPROM
 *
 *************************************/

/*************************************
 *  SRAM
 *************************************/
static READ16_HANDLER( genesis_sram_read )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	UINT8 *ROM;
	int rom_offset;

	if (state->m_md_cart.sram_active)
		return state->m_md_cart.sram[offset];
	else
	{
		ROM = state->memregion("maincpu")->base();
		rom_offset = state->m_md_cart.sram_start + (offset << 1);

		return (UINT16) ROM[rom_offset] | (ROM[rom_offset + 1] << 8);
	}
}

static WRITE16_HANDLER( genesis_sram_write )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	if (state->m_md_cart.sram_active)
	{
		if (!state->m_md_cart.sram_readonly)
			state->m_md_cart.sram[offset] = data;
	}
}

static void install_sram_rw_handlers(running_machine &machine, bool mask_addr)
{
	md_cons_state *state = machine.driver_data<md_cons_state>();
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("cart"));
	// if sram_start/end come from the cart header, they might be incorrect! from softlist, they are fine.
	UINT32 mask = mask_addr ? 0x3fffff : 0xffffff;

	mame_printf_debug("Allocing %d bytes for sram\n", state->m_md_cart.sram_end - state->m_md_cart.sram_start + 1);
	state->m_md_cart.sram = auto_alloc_array(machine, UINT16, (state->m_md_cart.sram_end - state->m_md_cart.sram_start + 1) / sizeof(UINT16));
	image->battery_load(state->m_md_cart.sram, state->m_md_cart.sram_end - state->m_md_cart.sram_start + 1, 0xff); // Dino Dini's Soccer needs backup RAM to be 1fill
	memcpy(megadriv_backupram, state->m_md_cart.sram, state->m_md_cart.sram_end - state->m_md_cart.sram_start + 1);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(state->m_md_cart.sram_start & mask, state->m_md_cart.sram_end & mask, FUNC(genesis_sram_read));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(state->m_md_cart.sram_start & mask, state->m_md_cart.sram_end & mask, FUNC(genesis_sram_write));
	state->m_md_cart.sram_handlers_installed = 1;
}

static WRITE16_HANDLER( genesis_sram_toggle )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();

	/* unsure if this is actually supposed to toggle or just switch on?
    Yet to encounter game that utilizes */
	state->m_md_cart.sram_active = (data & 1) ? 1 : 0;
	state->m_md_cart.sram_readonly = (data & 2) ? 1 : 0;

	if (state->m_md_cart.sram_active && !state->m_md_cart.sram_handlers_installed)
		install_sram_rw_handlers(space->machine(), TRUE);
}

static READ16_HANDLER( sega_6658a_reg_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	return state->m_md_cart.sram_active;
}

static WRITE16_HANDLER( sega_6658a_reg_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	if (data == 1)
		state->m_md_cart.sram_active = 1;
	if (data == 0)
		state->m_md_cart.sram_active = 0;
}

/*************************************
 *  I2C EEPROM
 *  bare minimum handlings
 *  TODO: supports proper device interfacing and parameter change!
 *************************************/

static READ16_HANDLER( nba_jam_eeprom_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
//  state->m_md_cart.i2c_mem = (i2cmem_sda_read(space->machine().device("i2cmem")) & 1);
	return state->m_md_cart.i2c_mem & 1;
}

static WRITE16_HANDLER( nba_jam_eeprom_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.i2c_clk = (data & 0x0002) >> 1;
	state->m_md_cart.i2c_mem = (data & 0x0001);

//  i2cmem_sda_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_clk);
//  i2cmem_scl_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_mem);
}

static READ16_HANDLER( nba_jam_te_eeprom_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
//  state->m_md_cart.i2c_mem = (i2cmem_sda_read(space->machine().device("i2cmem")) & 1);
	return (state->m_md_cart.i2c_mem & 1);
}

static WRITE16_HANDLER( nba_jam_te_eeprom_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.i2c_clk = ((data & 0x0100) >> 8);
	state->m_md_cart.i2c_mem = data & 0x0001;

//  i2cmem_sda_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_clk);
//  i2cmem_scl_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_mem);
}

static READ16_HANDLER( ea_nhlpa_eeprom_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
//  state->m_md_cart.i2c_mem = (i2cmem_sda_read(space->machine().device("i2cmem")) & 1);
	return (state->m_md_cart.i2c_mem & 1) << 7;
}

static WRITE16_HANDLER( ea_nhlpa_eeprom_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.i2c_clk = ((data & 0x0040) >> 6);
	state->m_md_cart.i2c_mem = ((data & 0x0080) >> 7);

//  i2cmem_sda_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_clk);
//  i2cmem_scl_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_mem);
}

/* TODO: identical as NBA Jam, used as kludge */
static READ16_HANDLER( wboy_v_eeprom_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
//  state->m_md_cart.i2c_mem = (i2cmem_sda_read(space->machine().device("i2cmem")) & 1);
	return ~state->m_md_cart.i2c_mem & 1;
}

static WRITE16_HANDLER( wboy_v_eeprom_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.i2c_clk = (data & 0x0002) >> 1;
	state->m_md_cart.i2c_mem = (data & 0x0001);

//  i2cmem_sda_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_clk);
//  i2cmem_scl_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_mem);
}

static READ16_HANDLER( codemasters_eeprom_r )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
//  state->m_md_cart.i2c_mem = (i2cmem_sda_read(space->machine().device("i2cmem")) & 1);
	return (state->m_md_cart.i2c_mem & 1) << 7;
}

static WRITE16_HANDLER( codemasters_eeprom_w )
{
	md_cons_state *state = space->machine().driver_data<md_cons_state>();
	state->m_md_cart.i2c_clk = (data & 0x0200) >> 9;
	state->m_md_cart.i2c_mem = (data & 0x0100) >> 8;

//  i2cmem_sda_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_clk);
//  i2cmem_scl_write(space->machine().device("i2cmem"), state->m_md_cart.i2c_mem);
}

/* ST M95320 32Kbit serial EEPROM implementation */

#define M95320_SIZE 0x1000
typedef enum
{
	IDLE = 0,
	CMD_WRSR,
	CMD_RDSR,
	CMD_READ,
	CMD_WRITE,
	READING,
	WRITING
} STMSTATE;

class stm95_device
{
public:
	stm95_device() : stm_state(IDLE), stream_pos(0) {};
	UINT8	*eeprom_data;
	void	set_cs_line(int);
	void	set_halt_line(int state) {}; // not implemented
	void	set_si_line(int);
	void	set_sck_line(int state);
	int		get_so_line(void);
protected:
	int		latch;
	int		reset_line;
	int		sck_line;
	int		WEL;

	STMSTATE	stm_state;
	int		stream_pos;
	int		stream_data;
	int		eeprom_addr;
};

void stm95_device::set_cs_line(int state)
{
	reset_line = state;
	if (reset_line != CLEAR_LINE)
	{
		stream_pos = 0;
		stm_state = IDLE;
	}
}

void stm95_device::set_si_line(int state)
{
	latch = state;
}

int stm95_device::get_so_line(void)
{
	if (stm_state == READING || stm_state == CMD_RDSR)
		return (stream_data >> 8) & 1;
	else
		return 0;
}

void stm95_device::set_sck_line(int state)
{
	if (reset_line == CLEAR_LINE)
	{
		if (state == ASSERT_LINE && sck_line == CLEAR_LINE)
		{
			switch (stm_state)
			{
				case IDLE:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 8)
					{
						stream_pos = 0;
						//printf("STM95 EEPROM: got cmd %02X\n", stream_data&0xff);
						switch(stream_data & 0xff)
						{
							case 0x01:	// write status register
								if (WEL != 0)
									stm_state = CMD_WRSR;
								WEL = 0;
								break;
							case 0x02:	// write
								if (WEL != 0)
									stm_state = CMD_WRITE;
								stream_data = 0;
								WEL = 0;
								break;
							case 0x03:	// read
								stm_state = CMD_READ;
								stream_data = 0;
								break;
							case 0x04:	// write disable
								WEL = 0;
								break;
							case 0x05:	// read status register
								stm_state = CMD_RDSR;
								stream_data = WEL<<1;
								break;
							case 0x06:	// write enable
								WEL = 1;
								break;
							default:
								logerror("STM95 EEPROM: unknown cmd %02X\n", stream_data&0xff);
						}
					}
					break;
				case CMD_WRSR:
					stream_pos++;		// just skip, don't care block protection
					if (stream_pos == 8)
					{
						stm_state = IDLE;
						stream_pos = 0;
					}
					break;
				case CMD_RDSR:
					stream_data = stream_data<<1;
					stream_pos++;
					if (stream_pos == 8)
					{
						stm_state = IDLE;
						stream_pos = 0;
					}
					break;
				case CMD_READ:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 16)
					{
						eeprom_addr = stream_data & (M95320_SIZE - 1);
						stream_data = eeprom_data[eeprom_addr];
						stm_state = READING;
						stream_pos = 0;
					}
					break;
				case READING:
					stream_data = stream_data<<1;
					stream_pos++;
					if (stream_pos == 8)
					{
						if (++eeprom_addr == M95320_SIZE)
							eeprom_addr = 0;
						stream_data |= eeprom_data[eeprom_addr];
						stream_pos = 0;
					}
					break;
				case CMD_WRITE:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 16)
					{
						eeprom_addr = stream_data & (M95320_SIZE - 1);
						stm_state = WRITING;
						stream_pos = 0;
					}
					break;
				case WRITING:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 8)
					{
						eeprom_data[eeprom_addr] = stream_data;
						if (++eeprom_addr == M95320_SIZE)
							eeprom_addr = 0;
						stream_pos = 0;
					}
					break;
			}
		}
	}
	sck_line = state;
}

stm95_device STM95;

static READ16_HANDLER( psolar_eeprom_r )
{
	return STM95.get_so_line() & 1;
}

static WRITE16_HANDLER( psolar_eeprom_w )
{
	STM95.set_si_line(data & 0x01);
	STM95.set_cs_line((data & 0x08)?ASSERT_LINE:CLEAR_LINE);
	STM95.set_halt_line((data & 0x04)?ASSERT_LINE:CLEAR_LINE);
	STM95.set_sck_line((data & 0x02)?ASSERT_LINE:CLEAR_LINE);
}

/*************************************
 *
 *  Machine driver reset
 *
 *************************************/

static void setup_megadriv_custom_mappers(running_machine &machine)
{
	md_cons_state *state = machine.driver_data<md_cons_state>();
	UINT32 mirroraddr;
	UINT8 *ROM = state->memregion("maincpu")->base();

	switch (state->m_md_cart.type)
	{
		case CM_JCART:
		case CM_JCART_SEPROM:
			/* Codemasters PCB (J-Carts) */
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x38fffe, 0x38ffff, FUNC(jcart_ctrl_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x38fffe, 0x38ffff, FUNC(jcart_ctrl_w));
			break;

		case SSF2:
			memcpy(&ROM[0x800000], &ROM[VIRGIN_COPY_GEN + 0x400000], 0x100000);
			memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x400000);
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x400000);

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa130f0, 0xa130ff, FUNC(genesis_ssf2_bank_w));
			break;

		case LIONK3:
		case SKINGKONG:
			state->m_md_cart.l3alt_pdat = state->m_md_cart.l3alt_pcmd = 0;
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x200000); /* default rom */
			memcpy(&ROM[0x200000], &ROM[VIRGIN_COPY_GEN], 0x200000); /* default rom */

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x600000, 0x6fffff, FUNC(l3alt_prot_r), FUNC(l3alt_prot_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x700000, 0x7fffff, FUNC(l3alt_bank_w));
			break;

		case SDK99:
			state->m_md_cart.l3alt_pdat = state->m_md_cart.l3alt_pcmd = 0;

			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x300000); /* default rom */
			memcpy(&ROM[0x300000], &ROM[VIRGIN_COPY_GEN], 0x100000); /* default rom */

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x600000, 0x6fffff, FUNC(l3alt_prot_r), FUNC(l3alt_prot_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x700000, 0x7fffff, FUNC(l3alt_bank_w));
			break;

		case REDCLIFF:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400001, FUNC(redclif_prot2_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400004, 0x400005, FUNC(redclif_prot_r));
			break;

		case REDCL_EN:
			for (int x = VIRGIN_COPY_GEN; x < VIRGIN_COPY_GEN + 0x200005; x++)
				ROM[x] ^= 0x40;
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN + 4], 0x200000); /* default rom */

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400001, FUNC(redclif_prot2_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400004, 0x400005, FUNC(redclif_prot_r));
			break;

		case RADICA:
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x400000);
			memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x400000); // keep a copy for later banking.. making use of huge ROM_REGION allocated to genesis driver
			memcpy(&ROM[0x800000], &ROM[VIRGIN_COPY_GEN], 0x400000); // wraparound banking (from hazemd code)

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa13000, 0xa1307f, FUNC(radica_bank_select));
			break;

		case KOF99:
			//memcpy(&ROM[0x000000],&ROM[VIRGIN_COPY_GEN],0x300000);
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa13000, 0xa13001, FUNC(kof99_A13000_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa13002, 0xa13003, FUNC(kof99_A13002_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa1303e, 0xa1303f, FUNC(kof99_00A1303E_r));
			break;

		case SOULBLAD:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400002, 0x400003, FUNC(soulb_400002_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400004, 0x400005, FUNC(soulb_400004_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400006, 0x400007, FUNC(soulb_400006_r));
			break;

		case MJLOVER:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400001, FUNC(mjlovr_prot_1_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x401000, 0x401001, FUNC(mjlovr_prot_2_r));
			break;

		case SQUIRRELK:
			state->m_md_cart.squirrel_king_extra = 0;
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400007, FUNC(squirrel_king_extra_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x400000, 0x400007, FUNC(squirrel_king_extra_w));
			break;

		case SMOUSE:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400007, FUNC(smous_prot_r));
			break;

		case SMB:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa13000, 0xa13001, FUNC(smbro_prot_r));
			break;

		case SMB2:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa13000, 0xa13001, FUNC(smb2_extra_r));
			break;

		case KAIJU:
			memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x200000);
			memcpy(&ROM[0x600000], &ROM[VIRGIN_COPY_GEN], 0x200000);
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x200000);

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x700000, 0x7fffff, FUNC(kaiju_bank_w));
			break;

		case CHINFIGHT3:
			memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x200000);
			memcpy(&ROM[0x600000], &ROM[VIRGIN_COPY_GEN], 0x200000);
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x200000);

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x4fffff, FUNC(chifi3_prot_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x600000, 0x6fffff, FUNC(chifi3_bank_w));
			break;

		case LIONK2:
			state->m_md_cart.lion2_prot1_data = state->m_md_cart.lion2_prot2_data = 0;

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400002, 0x400003, FUNC(lion2_prot1_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400006, 0x400007, FUNC(lion2_prot2_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x400000, 0x400001, FUNC(lion2_prot1_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x400004, 0x400005, FUNC(lion2_prot2_w));
			break;

		case BUGSLIFE:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa13000, 0xa13001, FUNC(bugl_extra_r));
			break;

		case ELFWOR:
			/* It return (0x55 @ 0x400000 OR 0xc9 @ 0x400004) AND (0x0f @ 0x400002 OR 0x18 @ 0x400006). It is probably best to add handlers for all 4 addresses. */
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400001, FUNC(elfwor_400000_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400002, 0x400003, FUNC(elfwor_400002_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400004, 0x400005, FUNC(elfwor_400004_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400006, 0x400007, FUNC(elfwor_400006_r));
			break;

		case ROCKMANX3:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa13000, 0xa13001, FUNC(rx3_extra_r));
			break;

		case SBUBBOB:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400000, 0x400001, FUNC(sbub_extra1_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x400002, 0x400003, FUNC(sbub_extra2_r));
			break;

		case KOF98:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x480000, 0x480001, FUNC(kof98_aa_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4800e0, 0x4800e1, FUNC(kof98_aa_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4824a0, 0x4824a1, FUNC(kof98_aa_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x488880, 0x488881, FUNC(kof98_aa_r));

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4a8820, 0x4a8821, FUNC(kof98_0a_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f8820, 0x4f8821, FUNC(kof98_00_r));
			break;

		case REALTEC:
			/* Realtec mapper!*/
			state->m_md_cart.realtec_bank_addr = state->m_md_cart.realtec_bank_size = 0;
			state->m_md_cart.realtec_old_bank_addr = -1;

			memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x80000);

			for (mirroraddr = 0; mirroraddr < 0x400000; mirroraddr += 0x2000)
				memcpy(ROM + mirroraddr, ROM + VIRGIN_COPY_GEN + 0x7e000, 0x002000); /* copy last 8kb across the whole rom region */

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x400000, 0x400001, FUNC(realtec_400000_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x402000, 0x402001, FUNC(realtec_402000_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x404000, 0x404001, FUNC(realtec_404000_w));
			break;

		case MC_SUP19IN1:
			memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x400000); // allow hard reset to menu
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa13000, 0xa13039, FUNC(s19in1_bank));
			break;

		case MC_SUP15IN1:
			memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x200000); // allow hard reset to menu
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa13000, 0xa13039, FUNC(s19in1_bank));
			break;

		case MC_12IN1:
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x400000);  /* default rom */
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa13000, 0xa1303f, FUNC(mc_12in1_bank_w));
			break;

		case TOPFIGHTER:
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x400000);  /* default rom */

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x6f5344, 0x6f5345, FUNC(topfig_6F5344_r) );
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x6bd294, 0x6bd295, FUNC(topfig_6BD294_r) );
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x645b44, 0x645b45, FUNC(topfig_645B44_r) );

			/* readd */
			//machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x689b80, 0x689b81, FUNC(MWA16_NOP));
			//machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x6d8b02, 0x6d8b03, FUNC(MWA16_NOP));

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x700000, 0x7fffff, FUNC(topfig_bank_w) );
			break;
		case PSOLAR:
			memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x400000);
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa13002, 0xa13007, FUNC(psolar_bank_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa13000, 0xa13001, FUNC(psolar_unk_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x0015e6, 0x0015e9, FUNC(psolar_hack_r));
			psolar_rdcnt = 0;
			break;
	}

	/* games whose protection gets patched out */

	if (state->m_md_cart.type == POKEMON)
	{
		/*todo: emulate protection instead
         0DD19E:47F8
         0DD1A0:FFF0
         0DD1A2:4E63
         0DD46E:4EF8
         0DD470:0300
         0DD49C:6002
         */

		/*

         you need to return 1 @ 0xa13002 and 0???1f @ 0xa1303E (it does word reads).

         */
		UINT16 *ROM16 = (UINT16 *)machine.root_device().memregion("maincpu")->base();

		ROM16[0x0dd19e/2] = 0x47F8;
		ROM16[0x0dd1a0/2] = 0xFFF0;
		ROM16[0x0dd1a2/2] = 0x4E63;
		ROM16[0x0dd46e/2] = 0x4EF8;
		ROM16[0x0dd470/2] = 0x0300;
		ROM16[0x0dd49c/2] = 0x6002;
	}

	if (state->m_md_cart.type == POKEMON2)
	{
		/*todo: emulate protection instead
         006036:E000
         002540:6026
         001ED0:6026
         002476:6022

         */
		UINT16 *ROM16 = (UINT16 *)machine.root_device().memregion("maincpu")->base();

		ROM16[0x06036/2] = 0xE000;
		ROM16[0x02540/2] = 0x6026;
		ROM16[0x01ED0/2] = 0x6026;
		ROM16[0x02476/2] = 0x6022;

		ROM16[0x7E300/2] = 0x60FE;
	}

	if (state->m_md_cart.type == MULAN)
	{
		/*todo: emulate protection instead
         006036:E000
         +more?

         */
		//  ROM[0x01ED0/2] = 0xE000;
		//  ROM[0x02540/2] = 0xE000;

		UINT16 *ROM16 = (UINT16 *)machine.root_device().memregion("maincpu")->base();

		ROM16[0x06036/2] = 0xE000;
	}

	/* install NOP handler for TMSS */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa14000, 0xa14003, FUNC(genesis_TMSS_bank_w));
}

static void setup_megadriv_sram(device_image_interface &image)
{
	running_machine &machine = image.device().machine();
	md_cons_state *state = machine.driver_data<md_cons_state>();
	UINT8 *ROM = state->memregion("maincpu")->base();
	megadriv_backupram = NULL;
	state->m_md_cart.sram = NULL;
	state->m_md_cart.sram_start = state->m_md_cart.sram_end = 0;
	state->m_md_cart.has_serial_eeprom = 0;
	state->m_md_cart.sram_detected = 0;
	state->m_md_cart.sram_handlers_installed = 0;
	state->m_md_cart.sram_readonly = 0;
	state->m_md_cart.sram_active = 0;

	/* install SRAM & i2c handlers for the specific type of cart */
	switch (state->m_md_cart.type)
	{
		// These types only come from xml
		// TODO: Eventually megadriv_backupram should point to the allocated cart:sram region!
		// For now, we only use the region as a placeholder to carry size info...
		case SEGA_SRAM:
			state->m_md_cart.sram_start = 0x200000;
			state->m_md_cart.sram_end = state->m_md_cart.sram_start + image.get_software_region_length("sram") - 1;
			state->m_md_cart.sram_detected = 1;
			megadriv_backupram = (UINT16*) (ROM + state->m_md_cart.sram_start);
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa130f0, 0xa130f1, FUNC(genesis_sram_toggle));
			if (state->m_md_cart.last_loaded_image_length <= state->m_md_cart.sram_start)
			{
				state->m_md_cart.sram_active = 1;
				install_sram_rw_handlers(machine, FALSE);
			}
			break;

		case BEGGAR:
			state->m_md_cart.sram_start = 0x400000;
			state->m_md_cart.sram_end = state->m_md_cart.sram_start + 0xffff;
			state->m_md_cart.sram_detected = 1;
			state->m_md_cart.sram_active = 1;
			megadriv_backupram = (UINT16*) (ROM + state->m_md_cart.sram_start);
			install_sram_rw_handlers(machine, FALSE);
			break;

		case SEGA_FRAM:
			state->m_md_cart.sram_start = 0x200000;
			state->m_md_cart.sram_end = state->m_md_cart.sram_start + image.get_software_region_length("fram") - 1;
			state->m_md_cart.sram_detected = 1;
			megadriv_backupram = (UINT16*) (ROM + state->m_md_cart.sram_start);
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa130f0, 0xa130f1, FUNC(sega_6658a_reg_r));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa130f0, 0xa130f1, FUNC(sega_6658a_reg_w));
			install_sram_rw_handlers(machine, FALSE);
			break;

		// These types might come either from xml or from old-styele loading
		case SEGA_EEPROM:
			state->m_md_cart.has_serial_eeprom = 1;
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x200000, 0x200001, FUNC(wboy_v_eeprom_r), FUNC(wboy_v_eeprom_w));
			break;

		case NBA_JAM:
			state->m_md_cart.has_serial_eeprom = 1;
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x200000, 0x200001, FUNC(nba_jam_eeprom_r), FUNC(nba_jam_eeprom_w));
			break;

		case NBA_JAM_TE:
		case NFL_QB_96:
		case C_SLAM: // same handling but different sizes
			state->m_md_cart.has_serial_eeprom = 1;
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x200000, 0x200001, FUNC(nba_jam_te_eeprom_r), FUNC(nba_jam_te_eeprom_w));
			break;

		case EA_NHLPA:
			state->m_md_cart.has_serial_eeprom = 1;
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x200000, 0x200001, FUNC(ea_nhlpa_eeprom_r), FUNC(ea_nhlpa_eeprom_w));
			break;

		case CODE_MASTERS:
		case CM_JCART_SEPROM:
			state->m_md_cart.has_serial_eeprom = 1;
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x300000, 0x300001, FUNC(codemasters_eeprom_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x380000, 0x380001, FUNC(codemasters_eeprom_r));
			break;
		case PSOLAR:
			state->m_md_cart.sram_start = 0x800000;
			state->m_md_cart.sram_end = state->m_md_cart.sram_start + M95320_SIZE - 1;
			state->m_md_cart.sram = auto_alloc_array(machine, UINT16, M95320_SIZE / sizeof(UINT16));
			image.battery_load(state->m_md_cart.sram, M95320_SIZE, 0xff);
			STM95.eeprom_data = (UINT8*)state->m_md_cart.sram;

			state->m_md_cart.has_serial_eeprom = 1;
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa13008, 0xa13009, FUNC(psolar_eeprom_w));
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xa1300a, 0xa1300b, FUNC(psolar_eeprom_r));
			break;
	}

	// If the cart is not of a special type, we check the header. Unfortunately, there are ROMs without correct info in the header,
	// hence we do the mapping anyway, but we activate SRAM later only if the game will access it.
	if (!state->m_md_cart.sram_detected && !state->m_md_cart.has_serial_eeprom)
	{
		/* check if cart has battery save */
		if (ROM[0x1b1] == 'R' && ROM[0x1b0] == 'A')
		{
			/* SRAM info found in header */
			state->m_md_cart.sram_start = (ROM[0x1b5] << 24 | ROM[0x1b4] << 16 | ROM[0x1b7] << 8 | ROM[0x1b6]);
			state->m_md_cart.sram_end = (ROM[0x1b9] << 24 | ROM[0x1b8] << 16 | ROM[0x1bb] << 8 | ROM[0x1ba]);

			if ((state->m_md_cart.sram_start > state->m_md_cart.sram_end) || ((state->m_md_cart.sram_end - state->m_md_cart.sram_start) >= 0x10000))	// we assume at most 64k of SRAM (HazeMD uses at most 64k). is this correct?
				state->m_md_cart.sram_end = state->m_md_cart.sram_start + 0x0FFFF;

			/* for some games using serial EEPROM, difference between SRAM
             end to start is 0 or 1. Currently EEPROM is not emulated. */
			if ((state->m_md_cart.sram_end - state->m_md_cart.sram_start) < 2)
				state->m_md_cart.has_serial_eeprom = 1;
			else
				state->m_md_cart.sram_detected = 1;
		}
		else
		{
			/* set default SRAM positions, with size = 64k */
			state->m_md_cart.sram_start = 0x200000;
			state->m_md_cart.sram_end = state->m_md_cart.sram_start + 0xffff;
		}

		if (state->m_md_cart.sram_start & 1)
			state->m_md_cart.sram_start -= 1;

		if (!(state->m_md_cart.sram_end & 1))
			state->m_md_cart.sram_end += 1;

		/* calculate backup RAM location */
		megadriv_backupram = (UINT16*) (ROM + (state->m_md_cart.sram_start & 0x3fffff));

		if (state->m_md_cart.sram_detected)
			logerror("SRAM detected from header: starting location %X - SRAM Length %X\n", state->m_md_cart.sram_start, state->m_md_cart.sram_end - state->m_md_cart.sram_start + 1);

		/* Enable SRAM handlers only if the game does not use EEPROM. */
		if (!state->m_md_cart.has_serial_eeprom)
		{
			/* Info from DGen: If SRAM does not overlap main ROM, set it active by
             default since a few games can't manage to properly switch it on/off. */
			if (state->m_md_cart.last_loaded_image_length <= state->m_md_cart.sram_start)
				state->m_md_cart.sram_active = 1;

			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xa130f0, 0xa130f1, FUNC(genesis_sram_toggle));
			//printf("res: start %x, end %x, det %d, active %d\n", state->m_md_cart.sram_start, state->m_md_cart.sram_end, state->m_md_cart.sram_detected, state->m_md_cart.sram_active);

			/* Sonic 1 included in Sonic Classics doesn't have SRAM and
             does lots of ROM access at this range, then only install read/
             write handlers if SRAM is active to not slow down emulation. */
			if (state->m_md_cart.sram_active)
				install_sram_rw_handlers(machine, TRUE);
		}
	}
}

/*************************************
 *
 *  CART HANDLING
 *
 *************************************/


/*************************************
 *  Helper function: Memcmp for both
 *  endianness
 *************************************/

static int allendianmemcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *realbuf;

#ifdef LSB_FIRST
	unsigned char flippedbuf[64];
	unsigned int i;

	if ((n & 1) || (n > 63)) return -1 ; // don't want odd sized buffers or too large a compare
	for (i = 0; i < n; i++) flippedbuf[i] = *((char *)s2 + (i ^ 1));
	realbuf = flippedbuf;

#else

	realbuf = (unsigned char *)s2;

#endif
	return memcmp(s1,realbuf,n);
}


/*************************************
 *  Helper function: Detect SMD file
 *************************************/

/* code taken directly from GoodGEN by Cowering */
static int genesis_is_funky_SMD(unsigned char *buf,unsigned int len)
{
	/* aq quiz */
	if (!strncmp("UZ(-01  ", (const char *) &buf[0xf0], 8))
		return 1;

    /* Phelios USA redump */
	/* target earth */
	/* klax (namcot) */
	if (buf[0x2080] == ' ' && buf[0x0080] == 'S' && buf[0x2081] == 'E' && buf[0x0081] == 'G')
		return 1;

    /* jap baseball 94 */
	if (!strncmp("OL R-AEAL", (const char *) &buf[0xf0], 9))
		return 1;

    /* devilish Mahjong Tower */
    if (!strncmp("optrEtranet", (const char *) &buf[0xf3], 11))
		return 1;

	/* golden axe 2 beta */
	if (buf[0x0100] == 0x3c && buf[0x0101] == 0 && buf[0x0102] == 0 && buf[0x0103] == 0x3c)
		return 1;

    /* omega race */
	if (!strncmp("OEARC   ", (const char *) &buf[0x90], 8))
		return 1;

    /* budokan beta */
	if ((len >= 0x6708+8) && !strncmp(" NTEBDKN", (const char *) &buf[0x6708], 8))
		return 1;

    /* cdx pro 1.8 bios */
	if (!strncmp("so fCXP", (const char *) &buf[0x2c0], 7))
		return 1;

    /* ishido (hacked) */
	if (!strncmp("sio-Wyo ", (const char *) &buf[0x0090], 8))
		return 1;

    /* onslaught */
	if (!strncmp("SS  CAL ", (const char *) &buf[0x0088], 8))
		return 1;

    /* tram terror pirate */
	if ((len >= 0x3648 + 8) && !strncmp("SG NEPIE", (const char *) &buf[0x3648], 8))
		return 1;

    /* breath of fire 3 chinese */
	if (buf[0x0007] == 0x1c && buf[0x0008] == 0x0a && buf[0x0009] == 0xb8 && buf[0x000a] == 0x0a)
		return 1;

    /*tetris pirate */
	if ((len >= 0x1cbe + 5) && !strncmp("@TTI>", (const char *) &buf[0x1cbe], 5))
		return 1;

	return 0;
}

/* code taken directly from GoodGEN by Cowering */
static int genesis_is_SMD(unsigned char *buf,unsigned int len)
{
	if (buf[0x2080] == 'S' && buf[0x80] == 'E' && buf[0x2081] == 'G' && buf[0x81] == 'A')
		return 1;
	return genesis_is_funky_SMD(buf,len);
}

/*************************************
 *  Loading a cart image *not* from
 *  softlist
 *************************************/

static int megadrive_load_nonlist(device_image_interface &image)
{
	md_cons_state *state = image.device().machine().driver_data<md_cons_state>();
	unsigned char *ROM, *rawROM, *tmpROMnew, *tmpROM, *secondhalf;
	int relocate, length, ptr, x;
#ifdef LSB_FIRST
	unsigned char fliptemp;
#endif

	// STEP 1: determine the file type (SMD? MD? BIN?)
	rawROM = state->memregion("maincpu")->base();
	ROM = rawROM /*+ 512 */;

	state->m_md_cart.last_loaded_image_length = -1;

	length = image.fread( rawROM + 0x2000, 0x800000);

	logerror("image length = 0x%x\n", length);

	/* is this a SMD file? */
	if (genesis_is_SMD(&rawROM[0x2200], (unsigned)length))
	{
//      printf("SMD!\n");
		tmpROMnew = ROM;
		tmpROM = ROM + 0x2000 + 512;

		for (ptr = 0; ptr < MAX_MD_CART_SIZE / (8192); ptr += 2)
		{
			for (x = 0; x < 8192; x++)
			{
				*tmpROMnew++ = *(tmpROM + ((ptr + 1) * 8192) + x);
				*tmpROMnew++ = *(tmpROM + ((ptr + 0) * 8192) + x);
			}
		}

#ifdef LSB_FIRST
		tmpROMnew = ROM;
		for (ptr = 0; ptr < length; ptr += 2)
		{
			fliptemp = tmpROMnew[ptr];
			tmpROMnew[ptr] = tmpROMnew[ptr + 1];
			tmpROMnew[ptr + 1] = fliptemp;
		}
#endif
		state->m_md_cart.last_loaded_image_length = length - 512;
		memcpy(&ROM[VIRGIN_COPY_GEN], &ROM[0x000000], MAX_MD_CART_SIZE);  /* store a copy of data */

		relocate = 0;

	}
	/* is this a MD file? */
	else if ((rawROM[0x2080] == 'E') && (rawROM[0x2081] == 'A') &&
			 (rawROM[0x2082] == 'M' || rawROM[0x2082] == 'G'))
	{
//      printf("MD!\n");
		tmpROMnew = global_alloc_array(unsigned char, length);
		secondhalf = &tmpROMnew[length >> 1];

		if (!tmpROMnew)
		{
			logerror("Memory allocation failed reading roms!\n");
			return IMAGE_INIT_FAIL;
		}

		memcpy(tmpROMnew, ROM + 0x2000, length);
		for (ptr = 0; ptr < length; ptr += 2)
		{

			ROM[ptr] = secondhalf[ptr >> 1];
			ROM[ptr + 1] = tmpROMnew[ptr >> 1];
		}
		global_free(tmpROMnew);

#ifdef LSB_FIRST
		for (ptr = 0; ptr < length; ptr += 2)
		{
			fliptemp = ROM[ptr];
			ROM[ptr] = ROM[ptr+1];
			ROM[ptr+1] = fliptemp;
		}
#endif
		state->m_md_cart.last_loaded_image_length = length;
		memcpy(&ROM[VIRGIN_COPY_GEN], &ROM[0x000000], MAX_MD_CART_SIZE);  /* store a copy of data */
		relocate = 0;

	}
	/* BIN it is, then */
	else
	{
		relocate = 0x2000;
		state->m_md_cart.last_loaded_image_length = length;

		for (ptr = 0; ptr < MAX_MD_CART_SIZE + relocate; ptr += 2)		/* mangle bytes for little endian machines */
		{
#ifdef LSB_FIRST
			int temp = ROM[relocate + ptr];

			ROM[ptr] = ROM[relocate + ptr + 1];
			ROM[ptr + 1] = temp;
#else
			ROM[ptr] = ROM[relocate + ptr];
			ROM[ptr + 1] = ROM[relocate + ptr + 1];
#endif
		}

		memcpy(&ROM[VIRGIN_COPY_GEN], &ROM[0x000000], MAX_MD_CART_SIZE);  /* store a copy of data */
	}

	// STEP 2: determine the cart type (to deal with pirate mappers & eeprom)

	/* Default cartridge type */
	state->m_md_cart.type = SEGA_STD;

	/* Detect carts which need additional handlers */
	{
		static const unsigned char smouse_sig[] = { 0x4d, 0xf9, 0x00, 0x40, 0x00, 0x02 },
		mjlover_sig[]	= { 0x13, 0xf9, 0x00, 0x40, 0x00, 0x00 }, // move.b  ($400000).l,($FFFF0C).l (partial)
		squir_sig[]		= { 0x26, 0x79, 0x00, 0xff, 0x00, 0xfa },
		bugsl_sig[]		= { 0x20, 0x12, 0x13, 0xc0, 0x00, 0xff },
		sbub_sig[]		= { 0x0c, 0x39, 0x00, 0x55, 0x00, 0x40 }, // cmpi.b  #$55,($400000).l
		lk3_sig[]		= { 0x0c, 0x01, 0x00, 0x30, 0x66, 0xe4 },
		sdk_sig[]		= { 0x48, 0xe7, 0xff, 0xfe, 0x52, 0x79 },
		redcliff_sig[]	= { 0x10, 0x39, 0x00, 0x40, 0x00, 0x04 }, // move.b  ($400004).l,d0
		redcl_en_sig[]	= { 0x50, 0x79, 0x40, 0x00, 0x40, 0x44 }, // move.b  ($400004).l,d0
		smb_sig[]		= { 0x20, 0x4d, 0x41, 0x52, 0x49, 0x4f },
		smb2_sig[]		= { 0x4e, 0xb9, 0x00, 0x0f, 0x25, 0x84 },
		kaiju_sig[]		= { 0x19, 0x7c, 0x00, 0x01, 0x00, 0x00 },
		chifi3_sig[]	= { 0xb6, 0x16, 0x66, 0x00, 0x00, 0x4a },
		lionk2_sig[]	= { 0x26, 0x79, 0x00, 0xff, 0x00, 0xf4 },
		rx3_sig[]		= { 0x66, 0x00, 0x00, 0x0e, 0x30, 0x3c },
		kof98_sig[]		= { 0x9b, 0xfc, 0x00, 0x00, 0x4a, 0x00 },
		s15in1_sig[]	= { 0x22, 0x3c, 0x00, 0xa1, 0x30, 0x00 },
		kof99_sig[]		= { 0x20, 0x3c, 0x30, 0x00, 0x00, 0xa1 }, // move.l  #$300000A1,d0
		radica_sig[]	= { 0x4e, 0xd0, 0x30, 0x39, 0x00, 0xa1 }, // jmp (a0) move.w ($a130xx),d0
		soulb_sig[]		= { 0x33, 0xfc, 0x00, 0x0c, 0x00, 0xff }, // move.w  #$C,($FF020A).l (what happens if check fails)
		s19in1_sig[]	= { 0x13, 0xc0, 0x00, 0xa1, 0x30, 0x38 },
		rockman_sig[]	= { 0xea, 0x80 };

		switch (state->m_md_cart.last_loaded_image_length)
		{
			case 0x80000:
			    if (!allendianmemcmp(&ROM[0x08c8], &smouse_sig[0], sizeof(smouse_sig)))
					state->m_md_cart.type = SMOUSE;

			    if (!allendianmemcmp((char *)&ROM[0x7e30e], "SEGA", 4) ||
					!allendianmemcmp((char *)&ROM[0x7e100], "SEGA", 4) ||
					!allendianmemcmp((char *)&ROM[0x7e1e6], "SEGA", 4))
					state->m_md_cart.type = REALTEC;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-50396", 10)) // NHLPA Hockey 93
					state->m_md_cart.type = EA_NHLPA;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM MK-1215", 10)) // Evander Holyfield
					state->m_md_cart.type = SEGA_EEPROM;

			    break;

			case 0xc0000:

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM G-4060 ", 8)) // Wonder Boy V
					state->m_md_cart.type = SEGA_EEPROM;

			    break;

			case 0x100000:
			    if (!allendianmemcmp(&ROM[0x01b24], &mjlover_sig[0], sizeof(mjlover_sig)))
					state->m_md_cart.type = MJLOVER;

			    if (!allendianmemcmp(&ROM[0x03b4], &squir_sig[0], sizeof(squir_sig)))
					state->m_md_cart.type = SQUIRRELK;

			    if (!allendianmemcmp(&ROM[0xee0d0], &bugsl_sig[0], sizeof(bugsl_sig)))
					state->m_md_cart.type = BUGSLIFE;

			    if (!allendianmemcmp((char *)&ROM[0x0172], "GAME : ELF WOR", 14))
					state->m_md_cart.type = ELFWOR;

			    if (!allendianmemcmp(&ROM[0x123e4], &sbub_sig[0], sizeof(sbub_sig)))
					state->m_md_cart.type = SBUBBOB;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-50176", 10)) // Rings of Power
					state->m_md_cart.type = EA_NHLPA;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "MK 00001211-00", 14)) // Sports Talk Baseball
					state->m_md_cart.type = SEGA_EEPROM;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-120096-", 12)) // Micro Machines 2
					state->m_md_cart.type = CODE_MASTERS;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-120146-", 12)) // Brian Lara Cricket 96 / Shane Wayne Cricket 96
					state->m_md_cart.type = CODE_MASTERS;

			    if (!allendianmemcmp((char *)&ROM[0x0190], "OJKRPTBVFCA     ", 0x10)) // Micro Machines '96 / Military TODO: better way to recognize these?
					state->m_md_cart.type = CODE_MASTERS;
			    break;

			case 0x200000:
			    if (!allendianmemcmp(&ROM[0x18c6], &lk3_sig[0], sizeof(lk3_sig)))
					state->m_md_cart.type = LIONK3;

			    if (!allendianmemcmp(&ROM[0x220], &sdk_sig[0], sizeof(sdk_sig)))
					state->m_md_cart.type = SKINGKONG;

			    if (!allendianmemcmp(&ROM[0xce560], &redcliff_sig[0], sizeof(redcliff_sig)))
					state->m_md_cart.type = REDCLIFF;

			    if (!allendianmemcmp(&ROM[0xc8cb0], &smb_sig[0], sizeof(smb_sig)))
					state->m_md_cart.type = SMB;

			    if (!allendianmemcmp(&ROM[0xf24d6], &smb2_sig[0], sizeof(smb2_sig)))
					state->m_md_cart.type = SMB2;

			    if (!allendianmemcmp(&ROM[0x674e], &kaiju_sig[0], sizeof(kaiju_sig)))
					state->m_md_cart.type = KAIJU;

			    if (!allendianmemcmp(&ROM[0x1780], &chifi3_sig[0], sizeof(chifi3_sig)))
					state->m_md_cart.type = CHINFIGHT3;

			    if (!allendianmemcmp(&ROM[0x03c2], &lionk2_sig[0], sizeof(lionk2_sig)))
					state->m_md_cart.type = LIONK2;

			    if (!allendianmemcmp(&ROM[0xc8b90], &rx3_sig[0], sizeof(rx3_sig)))
					state->m_md_cart.type = ROCKMANX3;

			    if (!allendianmemcmp(&ROM[0x56ae2], &kof98_sig[0], sizeof(kof98_sig)))
					state->m_md_cart.type = KOF98;

			    if (!allendianmemcmp(&ROM[0x17bb2], &s15in1_sig[0], sizeof(s15in1_sig)))
					state->m_md_cart.type = MC_SUP15IN1;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081326 ", 12)) // NBA Jam
					state->m_md_cart.type = NBA_JAM;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM MK-1228", 10)) // Greatest Heavyweight of the Ring
					state->m_md_cart.type = SEGA_EEPROM;

			    if ((!allendianmemcmp((char *)&ROM[0x0180], "GM T-12046", 10)) || // Mega Man
					(!allendianmemcmp((char *)&ROM[0x0180], "GM T-12053", 10) && !allendianmemcmp(&ROM[0x18e], &rockman_sig[0], sizeof(rockman_sig)))) // / Rock Man (EEPROM version)
					state->m_md_cart.type = SEGA_EEPROM;

			    break;

			case 0x200005:
			    if (!allendianmemcmp(&ROM[0xce564], &redcl_en_sig[0], sizeof(redcliff_sig)))
					state->m_md_cart.type = REDCL_EN;
			    break;

			case 0x300000:
			    if (!allendianmemcmp(&ROM[0x220], &sdk_sig[0], sizeof(sdk_sig)))
					state->m_md_cart.type = SDK99;

			    if (!allendianmemcmp(&ROM[0x1fd0d2], &kof99_sig[0], sizeof(kof99_sig)))
					state->m_md_cart.type = KOF99;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-81406", 10)) // NBA Jam TE
					state->m_md_cart.type = NBA_JAM_TE;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081276 ", 12)) // NFL Quarterback Club
					state->m_md_cart.type = NBA_JAM_TE;

			    break;

			case 0x400000:
			    if (!allendianmemcmp(&ROM[0x3c031c], &radica_sig[0], sizeof(radica_sig)) ||
					!allendianmemcmp(&ROM[0x3f031c], &radica_sig[0], sizeof(radica_sig))) // ssf+gng + radica vol1
					state->m_md_cart.type = RADICA;

			    if (!allendianmemcmp(&ROM[0x028460], &soulb_sig[0], sizeof(soulb_sig)))
					state->m_md_cart.type = SOULBLAD;

			    if (!allendianmemcmp(&ROM[0x1e700], &s19in1_sig[0], sizeof(s19in1_sig)))
					state->m_md_cart.type = MC_SUP19IN1;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081586-", 12)) // NFL Quarterback Club 96
					state->m_md_cart.type = NFL_QB_96;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081576 ", 12)) // College Slam
					state->m_md_cart.type = C_SLAM;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-81476", 10)) // Big Hurt Baseball
					state->m_md_cart.type = C_SLAM;

			    break;

			case 0x500000:
			    if (!allendianmemcmp((char *)&ROM[0x0120], "SUPER STREET FIGHTER2 ", 22))
					state->m_md_cart.type = SSF2;
			    break;
			case 0x800000:
			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-574023-", 12)) // Pier Solar
					state->m_md_cart.type = PSOLAR;
			    break;
			default:
			    break;
		}
	}

	return IMAGE_INIT_PASS;
}

static DEVICE_IMAGE_DISPLAY_INFO(megadriv)
{
	const char *compatibility = image.get_feature("compatibility");
	const char *chips = image.get_feature("addon");
	int md_region = megadrive_region_export + (megadrive_region_pal << 1);

	if (chips)
	{
		if (!mame_stricmp(chips, "SVP") && (image.device().machine().device<cpu_device>("svp") == NULL))
		{
			mame_printf_warning("This software requires emulation of the SVP CPU to work.\n");
			mame_printf_warning("It might not work in the system you are using.\n");
		}
	}

	if (compatibility)
	{
		if (!mame_stricmp(compatibility, "PAL") && (md_region != 3))
		{
			mame_printf_warning("This software requires a PAL console to work.\n");
			mame_printf_warning("It might not work in the system you are using.\n");
		}
		if (!mame_stricmp(compatibility, "NTSC-J") && (md_region != 0))
		{
			mame_printf_warning("This software requires a JPN console to work.\n");
			mame_printf_warning("It might not work in the system you are using.\n");
		}
		if (!mame_stricmp(compatibility, "NTSC-U") && (md_region != 1))
		{
			mame_printf_warning("This software requires a USA console to work.\n");
			mame_printf_warning("It might not work in the system you are using.\n");
		}
		if (!mame_stricmp(compatibility, "EUR-JPN") && (md_region != 0 && md_region != 3))
		{
			mame_printf_warning("This software requires a PAL or a JPN console to work.\n");
			mame_printf_warning("It might not work in the system you are using.\n");
		}
	}
}


/*************************************
 *  Loading a cart image from softlist
 *************************************/

static int megadrive_load_list(device_image_interface &image)
{
	md_cons_state *state = image.device().machine().driver_data<md_cons_state>();
	UINT8 *ROM = state->memregion("maincpu")->base();
	UINT32 length = image.get_software_region_length("rom");
	const char	*pcb_name;
	memcpy(ROM, image.get_software_region("rom"), length);

	state->m_md_cart.last_loaded_image_length = length;

	if ((pcb_name = image.get_feature("pcb_type")) == NULL)
		state->m_md_cart.type = SEGA_STD;
	else
		state->m_md_cart.type = md_get_pcb_id(pcb_name);

	memcpy(&ROM[VIRGIN_COPY_GEN], &ROM[0x000000], MAX_MD_CART_SIZE);  /* store a copy of data */

	return IMAGE_INIT_PASS;
}

static DEVICE_IMAGE_LOAD( genesis_cart )
{
	md_cons_state *state = image.device().machine().driver_data<md_cons_state>();
	state->m_md_cart.type = SEGA_STD;
	int res;

	// STEP 1: load the file image and keep a copy for later banking
	// STEP 2: identify the cart type
	// The two steps are carried out differently if we are loading from a list or not
	if (image.software_entry() == NULL)
		res = megadrive_load_nonlist(image);
	else
		res = megadrive_load_list(image);

	logerror("cart type: %d\n", state->m_md_cart.type);

	if (res == IMAGE_INIT_PASS)
	{
		// STEP 3: install memory handlers for this type of cart
		setup_megadriv_custom_mappers(image.device().machine());

		// STEP 4: take care of SRAM.
		setup_megadriv_sram(image);

		return IMAGE_INIT_PASS;
	}
	else
		return IMAGE_INIT_FAIL;
}

/******* SRAM saving *******/

static void genesis_machine_stop(running_machine &machine)
{
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("cart"));
	md_cons_state *state = machine.driver_data<md_cons_state>();

	/* Write out the battery file if necessary */
	if (state->m_md_cart.sram != NULL)
	{
		mame_printf_debug("Saving sram\n");
		image->battery_save(state->m_md_cart.sram, state->m_md_cart.sram_end - state->m_md_cart.sram_start + 1);
	}
}

MACHINE_START( md_sram )
{
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(genesis_machine_stop), &machine));
}

/******* 32X image loading *******/

// FIXME: non-softlist loading should keep using ROM_CART_LOAD in the ROM definitions,
// once we better integrate softlist with the old loading procedures
static DEVICE_IMAGE_LOAD( _32x_cart )
{
	UINT32 length;
	UINT8 *temp_copy;
	UINT16 *ROM16;
	UINT32 *ROM32;
	int i;

	if (image.software_entry() == NULL)
	{
		length = image.length();
		temp_copy = auto_alloc_array(image.device().machine(), UINT8, length);
		image.fread( temp_copy, length);
	}
	else
	{
		length = image.get_software_region_length("rom");
		temp_copy = auto_alloc_array(image.device().machine(), UINT8, length);
		memcpy(temp_copy, image.get_software_region("rom"), length);
	}

	/* Copy the cart image in the locations the driver expects */
	// Notice that, by using pick_integer, we are sure the code works on both LE and BE machines
	ROM16 = (UINT16 *) image.device().machine().root_device().memregion("gamecart")->base();
	for (i = 0; i < length; i += 2)
		ROM16[i / 2] = pick_integer_be(temp_copy, i, 2);

	ROM32 = (UINT32 *) image.device().machine().root_device().memregion("gamecart_sh2")->base();
	for (i = 0; i < length; i += 4)
		ROM32[i / 4] = pick_integer_be(temp_copy, i, 4);

	ROM16 = (UINT16 *) image.device().machine().root_device().memregion("maincpu")->base();
	for (i = 0x00; i < length; i += 2)
		ROM16[i / 2] = pick_integer_be(temp_copy, i, 2);

	auto_free(image.device().machine(), temp_copy);

	return IMAGE_INIT_PASS;
}

/******* Cart getinfo *******/

MACHINE_CONFIG_FRAGMENT( genesis_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("smd,bin,md,gen")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("megadriv_cart")
	MCFG_CARTSLOT_LOAD(genesis_cart)
	MCFG_CARTSLOT_DISPLAY_INFO(megadriv)
	MCFG_SOFTWARE_LIST_ADD("cart_list","megadriv")
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( _32x_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("32x,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("_32x_cart")
	MCFG_CARTSLOT_LOAD(_32x_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","32x")
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( pico_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("pico_cart")
	MCFG_CARTSLOT_LOAD(genesis_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","pico")
MACHINE_CONFIG_END
