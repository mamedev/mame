/***************************************************************************

  genesis.c

  Machine file to handle emulation of the Sega Mega Drive & Genesis.

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
#include "imageutl.h"
#include "cpu/m68000/m68000.h"

#include "imagedev/cartslot.h"
#include "../includes/megadriv.h"
#include "md_pcb.h"


#define MAX_MD_CART_SIZE 0x500000

/* where a fresh copy of rom is stashed for reset and banking setup */
#define VIRGIN_COPY_GEN 0xd00000

enum t_cart_type
{
	STANDARD = 0,
	SSF2,					/* Super Street Fighter 2 */
	LIONK3,					/* Lion King 3 */
	SKINGKONG,				/* Super King Kong 99 */
	SDK99,					/* Super Donkey Kong 99 */
	REDCLIFF,				/* Romance of the Three Kingdoms - Battle of Red Cliffs, already decoded from .mdx format */
	REDCL_EN,				/* The encoded version... */
	RADICA,					/* Radica TV games.. these probably should be a seperate driver since they are a seperate 'console' */
	KOF99,					/* King of Fighters '99 */
	SOULBLAD,				/* Soul Blade */
	MJLOVER,				/* Mahjong Lover */
	SQUIRRELK,				/* Squirrel King */
	SMOUSE,					/* Smart Mouse */
	SMB,					/* Super Mario Bros. */
	SMB2,					/* Super Mario Bros. 2 */
	KAIJU,					/* Pokemon Stadium */
	CHINFIGHT3,				/* Chinese Fighters 3 */
	LIONK2,					/* Lion King 2 */
	BUGSLIFE,				/* A Bug's Life */
	ELFWOR,					/* Elf Wor */
	ROCKMANX3,				/* Rockman X3 */
	SBUBBOB,				/* Super Bubble Bobble */
	KOF98,					/* King of Fighters '98 */
	REALTEC,				/* Whac a Critter/Mallet legend, Defend the Earth, Funnyworld/Ballonboy */
	SUP19IN1,				/* Super 19 in 1 */
	SUP15IN1,				/* Super 15 in 1 */
	NBA_JAM,				/* NBA Jam */
	NBA_JAM_TE,				/* NBA Jam TE / NFL Quarterback Club */
	NFL_QB_96,				/* NFL Quarterback Club '96 */
	C_SLAM,					/* College Slam / Frank Thomas Big Hurt Baseball */
	EA_NHLPA,				/* NHLPA Hockey 93 / Rings of Power */
	WBOY_V,					/* Wonder Boy V / Evander Holyfield's Boxing / Greatest Heavyweights of the Ring / Sports Talk Baseball / Megaman */
	CODE_MASTERS			/* Micro Machines 2 / Military / 96 / Brian Lara Cricket */
};


static enum t_cart_type cart_type;


static UINT16 squirrel_king_extra;
static UINT16 lion2_prot1_data, lion2_prot2_data;
static UINT16 realtec_bank_addr, realtec_bank_size, realtec_old_bank_addr;
static UINT16 g_l3alt_pdat;
static UINT16 g_l3alt_pcmd;

static int genesis_last_loaded_image_length;

UINT16 *genesis_sram;
static int genesis_sram_start, genesis_sram_end;
static int genesis_sram_active, genesis_sram_readonly;
static int sram_handlers_installed;

static int has_serial_eeprom;


/*************************************
 *
 *  Handlers for custom mappers
 *
 *************************************/

static WRITE16_HANDLER( genesis_ssf2_bank_w )
{
	static int lastoffset = -1,lastdata = -1;
	UINT8 *ROM = space->machine->region("maincpu")->base();

	if ((lastoffset != offset) || (lastdata != data))
	{
		lastoffset = offset; lastdata = data;
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

#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( g_l3alt_pdat_w )
{
	g_l3alt_pdat = data;
}

static WRITE16_HANDLER( g_l3alt_pcmd_w )
{
	g_l3alt_pcmd = data;
}
#endif

static READ16_HANDLER( g_l3alt_prot_r )
{
	int retdata = 0;

	offset &= 0x07;

	switch (offset)
	{

		case 2:

			switch (g_l3alt_pcmd)
			{
				case 1:
					retdata = g_l3alt_pdat >> 1;
					break;

				case 2:
					retdata = g_l3alt_pdat >> 4;
					retdata |= (g_l3alt_pdat & 0x0f) << 4;
					break;

				default:
					/* printf("unk prot case %d\n", g_l3alt_pcmd); */
					retdata =  (((g_l3alt_pdat >> 7) & 1) << 0);
					retdata |= (((g_l3alt_pdat >> 6) & 1) << 1);
					retdata |= (((g_l3alt_pdat >> 5) & 1) << 2);
					retdata |= (((g_l3alt_pdat >> 4) & 1) << 3);
					retdata |= (((g_l3alt_pdat >> 3) & 1) << 4);
					retdata |= (((g_l3alt_pdat >> 2) & 1) << 5);
					retdata |= (((g_l3alt_pdat >> 1) & 1) << 6);
					retdata |= (((g_l3alt_pdat >> 0) & 1) << 7);
					break;
			}
			break;

		default:

			printf("protection read, unknown offset\n");
			break;
	}

/*  printf("%06x: g_l3alt_pdat_w %04x g_l3alt_pcmd_w %04x return %04x\n", activecpu_get_pc(), g_l3alt_pdat, g_l3alt_pcmd, retdata); */

	return retdata;
}

static WRITE16_HANDLER( g_l3alt_prot_w )
{
	offset &= 0x7;

	switch (offset)
	{
		case 0x0:
			g_l3alt_pdat = data;
			break;
		case 0x1:
			g_l3alt_pcmd = data;
			break;
		default:
			printf("protection write, unknown offst\n");
			break;
	}
}

static WRITE16_HANDLER( g_l3alt_bank_w )
{
	offset &= 0x7;

	switch (offset)
	{
		case 0:
		{
		UINT8 *ROM = space->machine->region("maincpu")->base();
		/* printf("%06x data %04x\n",activecpu_get_pc(), data); */
		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN + (data & 0xffff) * 0x8000], 0x8000);
		}
		break;

		default:
		printf("unk bank w\n");
		break;

	}

}


static WRITE16_HANDLER( realtec_402000_w )
{
	realtec_bank_addr = 0;
	realtec_bank_size = (data >> 8) & 0x1f;
}

static WRITE16_HANDLER( realtec_400000_w )
{
	int bankdata = (data >> 9) & 0x7;

	UINT8 *ROM = space->machine->region("maincpu")->base();

	realtec_old_bank_addr = realtec_bank_addr;
	realtec_bank_addr = (realtec_bank_addr & 0x7) | bankdata << 3;

	memcpy(ROM, ROM + (realtec_bank_addr * 0x20000) + 0x400000, realtec_bank_size * 0x20000);
	memcpy(ROM + realtec_bank_size * 0x20000, ROM + (realtec_bank_addr * 0x20000) + 0x400000, realtec_bank_size * 0x20000);
}

static WRITE16_HANDLER( realtec_404000_w )
{
	int bankdata = (data >> 8) & 0x3;
	UINT8 *ROM = space->machine->region("maincpu")->base();

	realtec_old_bank_addr = realtec_bank_addr;
	realtec_bank_addr = (realtec_bank_addr & 0xf8) | bankdata;

	if (realtec_old_bank_addr != realtec_bank_addr)
	{
		memcpy(ROM, ROM + (realtec_bank_addr * 0x20000)+ 0x400000, realtec_bank_size * 0x20000);
		memcpy(ROM + realtec_bank_size * 0x20000, ROM + (realtec_bank_addr * 0x20000) + 0x400000, realtec_bank_size * 0x20000);
	}
}

static WRITE16_HANDLER( g_chifi3_bank_w )
{
	UINT8 *ROM = space->machine->region("maincpu")->base();

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
		logerror("%06x chifi3, bankw? %04x %04x\n", cpu_get_pc(space->cpu), offset, data);
	}

}

static READ16_HANDLER( g_chifi3_prot_r )
{
	UINT32 retdat;

	/* not 100% correct, there may be some relationship between the reads here
    and the writes made at the start of the game.. */

	/*
    04dc10 chifi3, prot_r? 2800
    04cefa chifi3, prot_r? 65262
    */

	if (cpu_get_pc(space->cpu) == 0x01782) // makes 'VS' screen appear
	{
		retdat = cpu_get_reg(space->cpu, M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(space->cpu) == 0x1c24) // background gfx etc.
	{
		retdat = cpu_get_reg(space->cpu, M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(space->cpu) == 0x10c4a) // unknown
	{
		return space->machine->rand();
	}
	else if (cpu_get_pc(space->cpu) == 0x10c50) // unknown
	{
		return space->machine->rand();
	}
	else if (cpu_get_pc(space->cpu) == 0x10c52) // relates to the game speed..
	{
		retdat = cpu_get_reg(space->cpu, M68K_D4) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(space->cpu) == 0x061ae)
	{
		retdat = cpu_get_reg(space->cpu, M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else if (cpu_get_pc(space->cpu) == 0x061b0)
	{
		retdat = cpu_get_reg(space->cpu, M68K_D3) & 0xff;
		retdat <<= 8;
		return retdat;
	}
	else
	{
		logerror("%06x chifi3, prot_r? %04x\n", cpu_get_pc(space->cpu), offset);
	}

	return 0;
}

static WRITE16_HANDLER( s19in1_bank )
{
	UINT8 *ROM = space->machine->region("maincpu")->base();
	memcpy(ROM + 0x000000, ROM + 0x400000 + ((offset << 1) * 0x10000), 0x80000);
}

// Kaiju? (Pokemon Stadium) handler from HazeMD
static WRITE16_HANDLER( g_kaiju_bank_w )
{
	UINT8 *ROM = space->machine->region("maincpu")->base();
	memcpy(ROM + 0x000000, ROM + 0x400000 + (data & 0x7f) * 0x8000, 0x8000);
}

// Soulblade handler from HazeMD
static READ16_HANDLER( soulb_0x400006_r )
{
	return 0xf000;
}

static READ16_HANDLER( soulb_0x400002_r )
{
	return 0x9800;
}

static READ16_HANDLER( soulb_0x400004_r )
{
	return 0xc900;
}

// Mahjong Lover handler from HazeMD
static READ16_HANDLER( mjlovr_prot_1_r )
{
	return 0x9000;
}

static READ16_HANDLER( mjlovr_prot_2_r )
{
	return 0xd300;
}

// Super Mario Bros handler from HazeMD
static READ16_HANDLER( smbro_prot_r )
{
	return 0xc;
}


// Smart Mouse handler from HazeMD
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

// Super Bubble Bobble MD handler from HazeMD
static READ16_HANDLER( sbub_extra1_r )
{
	return 0x5500;
}

static READ16_HANDLER( sbub_extra2_r )
{
	return 0x0f00;
}

// KOF99 handler from HazeMD
static READ16_HANDLER( kof99_0xA13002_r )
{
	// write 02 to a13002.. shift right 1?
	return 0x01;
}

static READ16_HANDLER( kof99_00A1303E_r )
{
	// write 3e to a1303e.. shift right 1?
	return 0x1f;
}

static READ16_HANDLER( kof99_0xA13000_r )
{
	// no write, startup check, chinese message if != 0
	return 0x0;
}

// Radica handler from HazeMD

static READ16_HANDLER( radica_bank_select )
{
	int bank = offset&0x3f;
	UINT8 *ROM = space->machine->region("maincpu")->base();
	memcpy(ROM, ROM +  (bank * 0x10000) + 0x400000, 0x400000);
	return 0;
}

// ROTK Red Cliff handler from HazeMD

static READ16_HANDLER( redclif_prot_r )
{
	return -0x56 << 8;
}

static READ16_HANDLER( redclif_prot2_r )
{
	return 0x55 << 8;
}

// Squirrel King handler from HazeMD, this does not give screen garbage like HazeMD compile If you reset it twice
static READ16_HANDLER( squirrel_king_extra_r )
{
	return squirrel_king_extra;

}
static WRITE16_HANDLER( squirrel_king_extra_w )
{
	squirrel_king_extra = data;
}

// Lion King 2 handler from HazeMD
static READ16_HANDLER( lion2_prot1_r )
{
	return lion2_prot1_data;
}

static READ16_HANDLER( lion2_prot2_r )
{
	return lion2_prot2_data;
}

static WRITE16_HANDLER ( lion2_prot1_w )
{
	lion2_prot1_data = data;
}

static WRITE16_HANDLER ( lion2_prot2_w )
{
	lion2_prot2_data = data;
}

// Rockman X3 handler from HazeMD
static READ16_HANDLER( rx3_extra_r )
{
	return 0xc;
}

// King of Fighters 98 handler from HazeMD
static READ16_HANDLER( g_kof98_aa_r )
{
	return 0xaa00;
}

static READ16_HANDLER( g_kof98_0a_r )
{
	return 0x0a00;
}

static READ16_HANDLER( g_kof98_00_r )
{
	return 0x0000;
}

// Super Mario Bros 2 handler from HazeMD
static READ16_HANDLER( smb2_extra_r )
{
	return 0xa;
}

// Bug's Life handler from HazeMD
static READ16_HANDLER( bugl_extra_r )
{
	return 0x28;
}

// Elf Wor handler from HazeMD (DFJustin says the title is 'Linghuan Daoshi Super Magician')
static READ16_HANDLER( elfwor_0x400000_r )
{
	return 0x5500;
}

static READ16_HANDLER( elfwor_0x400002_r )
{
	return 0x0f00;
}

static READ16_HANDLER( elfwor_0x400004_r )
{
	return 0xc900;
}

static READ16_HANDLER( elfwor_0x400006_r )
{
	return 0x1800;
}

static WRITE16_HANDLER( genesis_TMSS_bank_w )
{
	/* this probably should do more, like make Genesis V2 'die' if the SEGA string is not written promptly */
}



/*************************************
 *
 *  Handlers for SRAM
 *
 *************************************/

static void alloc_sram(running_machine *machine)
{
	genesis_sram = auto_alloc_array(machine, UINT16, (genesis_sram_end - genesis_sram_start + 1) / sizeof(UINT16));
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine->device("cart"));
	image->battery_load(genesis_sram, genesis_sram_end - genesis_sram_start + 1, 0xff); // Dino Dini's Soccer needs backup RAM to be 1fill
	memcpy(megadriv_backupram, genesis_sram, genesis_sram_end - genesis_sram_start + 1);
}

static READ16_HANDLER( genesis_sram_read )
{
	UINT8 *ROM;
	int rom_offset;

	if (genesis_sram_active)
	{
		if (genesis_sram == NULL)
			alloc_sram(space->machine);
		return genesis_sram[offset];
	}
	else
	{
		ROM = space->machine->region("maincpu")->base();
		rom_offset = genesis_sram_start + (offset << 1);

		return (UINT16) ROM[rom_offset] | (ROM[rom_offset + 1] << 8);
	}
}

static WRITE16_HANDLER( genesis_sram_write )
{
	if (genesis_sram_active)
	{
		if (genesis_sram == NULL)
			alloc_sram(space->machine);
		if (!genesis_sram_readonly)
			genesis_sram[offset] = data;
	}
}

static void install_sram_rw_handlers(running_machine *machine)
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), genesis_sram_start & 0x3fffff, genesis_sram_end & 0x3fffff, 0, 0, genesis_sram_read);
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), genesis_sram_start & 0x3fffff, genesis_sram_end & 0x3fffff, 0, 0, genesis_sram_write);
	sram_handlers_installed = 1;
}

static WRITE16_HANDLER( genesis_sram_toggle )
{

	/* unsure if this is actually supposed to toggle or just switch on?
    * Yet to encounter game that utilizes
    */
	genesis_sram_active = (data & 1) ? 1 : 0;
	genesis_sram_readonly = (data & 2) ? 1 : 0;

	if (genesis_sram_active && !sram_handlers_installed)
		install_sram_rw_handlers (space->machine);
}

/*
I2c bare minimum handlings (TODO: supports proper device interfacing and parameter change!)
*/

static UINT8 i2c_mem,i2c_clk;

static READ16_HANDLER( nba_jam_eeprom_r )
{
//  i2c_mem = (i2cmem_sda_read(space->machine->device("i2cmem")) & 1);

	return i2c_mem & 1;
}

static WRITE16_HANDLER( nba_jam_eeprom_w )
{
	i2c_clk = (data & 0x0002) >> 1;
	i2c_mem = (data & 0x0001);

//  i2cmem_sda_write(space->machine->device("i2cmem"), i2c_clk);
//  i2cmem_scl_write(space->machine->device("i2cmem"), i2c_mem);
}

static READ16_HANDLER( nba_jam_te_eeprom_r )
{
//  i2c_mem = (i2cmem_sda_read(space->machine->device("i2cmem")) & 1);

	return (i2c_mem & 1);
}

static WRITE16_HANDLER( nba_jam_te_eeprom_w )
{
	i2c_clk = ((data & 0x0100) >> 8);
	i2c_mem = data & 0x0001;

//  i2cmem_sda_write(space->machine->device("i2cmem"), i2c_clk);
//  i2cmem_scl_write(space->machine->device("i2cmem"), i2c_mem);
}

static READ16_HANDLER( ea_nhlpa_eeprom_r )
{
//  i2c_mem = (i2cmem_sda_read(space->machine->device("i2cmem")) & 1);

	return (i2c_mem & 1) << 7;
}

static WRITE16_HANDLER( ea_nhlpa_eeprom_w )
{
	i2c_clk = ((data & 0x0040) >> 6);
	i2c_mem = ((data & 0x0080) >> 7);

//  i2cmem_sda_write(space->machine->device("i2cmem"), i2c_clk);
//  i2cmem_scl_write(space->machine->device("i2cmem"), i2c_mem);
}

/* TODO: identical as NBA Jam, used as kludge */
static READ16_HANDLER( wboy_v_eeprom_r )
{
//  i2c_mem = (i2cmem_sda_read(space->machine->device("i2cmem")) & 1);

	return ~i2c_mem & 1;
}

static WRITE16_HANDLER( wboy_v_eeprom_w )
{
	i2c_clk = (data & 0x0002) >> 1;
	i2c_mem = (data & 0x0001);

//  i2cmem_sda_write(space->machine->device("i2cmem"), i2c_clk);
//  i2cmem_scl_write(space->machine->device("i2cmem"), i2c_mem);
}

static READ16_HANDLER( codemasters_eeprom_r )
{
//  i2c_mem = (i2cmem_sda_read(space->machine->device("i2cmem")) & 1);

	return (i2c_mem & 1) << 7;
}

static WRITE16_HANDLER( codemasters_eeprom_w )
{
	i2c_clk = (data & 0x0200) >> 9;
	i2c_mem = (data & 0x0100) >> 8;

//  i2cmem_sda_write(space->machine->device("i2cmem"), i2c_clk);
//  i2cmem_scl_write(space->machine->device("i2cmem"), i2c_mem);
}

/*************************************
 *
 *  Machine driver reset
 *
 *************************************/


static void setup_megadriv_custom_mappers(running_machine *machine)
{
	static int relocate = VIRGIN_COPY_GEN;
	unsigned char *ROM;
	UINT32 mirroraddr;

	/* SRAM data */
	sram_handlers_installed = 0;
	genesis_sram_readonly = 0;
	genesis_sram_active = 0;

	ROM = machine->region("maincpu")->base();

	if (cart_type == SSF2)
	{
		memcpy(&ROM[0x800000], &ROM[VIRGIN_COPY_GEN + 0x400000], 0x100000);
		memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x400000);
		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x400000);

		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa130f0, 0xa130ff, 0, 0, genesis_ssf2_bank_w);
	}

	if (cart_type == LIONK3 || cart_type == SKINGKONG)
	{
		g_l3alt_pdat = g_l3alt_pcmd = 0;
		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x200000); /* default rom */
		memcpy(&ROM[0x200000], &ROM[VIRGIN_COPY_GEN], 0x200000); /* default rom */

		memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x600000, 0x6fffff, 0, 0, g_l3alt_prot_r, g_l3alt_prot_w);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x700000, 0x7fffff, 0, 0, g_l3alt_bank_w);
	}

	if (cart_type == SDK99)
	{
		g_l3alt_pdat = g_l3alt_pcmd = 0;

		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x300000); /* default rom */
		memcpy(&ROM[0x300000], &ROM[VIRGIN_COPY_GEN], 0x100000); /* default rom */

		memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x600000, 0x6fffff, 0, 0, g_l3alt_prot_r, g_l3alt_prot_w);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x700000, 0x7fffff, 0, 0, g_l3alt_bank_w);
	}

	if (cart_type == REDCLIFF)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, redclif_prot2_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400004, 0x400005, 0, 0, redclif_prot_r);
	}

	if (cart_type == REDCL_EN)
	{
		int x;

		for (x = VIRGIN_COPY_GEN; x < VIRGIN_COPY_GEN + 0x200005; x++)
		{
			ROM[x] ^= 0x40;
		}

		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN + 4], 0x200000); /* default rom */

		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, redclif_prot2_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400004, 0x400005, 0, 0, redclif_prot_r);
	}

	if (cart_type == RADICA)
	{
		memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x400000); // keep a copy for later banking.. making use of huge ROM_REGION allocated to genesis driver
		memcpy(&ROM[0x800000], &ROM[VIRGIN_COPY_GEN], 0x400000); // wraparound banking (from hazemd code)
		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x400000);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa1307f, 0, 0, radica_bank_select);
	}

	if (cart_type == KOF99)
	{
		//memcpy(&ROM[0x000000],&ROM[VIRGIN_COPY_GEN],0x300000);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa13001, 0, 0, kof99_0xA13000_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13002, 0xa13003, 0, 0, kof99_0xA13002_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa1303e, 0xa1303f, 0, 0, kof99_00A1303E_r);
	}

	if (cart_type == SOULBLAD)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400002, 0x400003, 0, 0, soulb_0x400002_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400004, 0x400005, 0, 0, soulb_0x400004_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400006, 0x400007, 0, 0, soulb_0x400006_r);
	}

	if (cart_type == MJLOVER)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, mjlovr_prot_1_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x401000, 0x401001, 0, 0, mjlovr_prot_2_r);
	}

	if (cart_type == SQUIRRELK)
	{
		squirrel_king_extra = 0;
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400007, 0, 0, squirrel_king_extra_r);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400007, 0, 0, squirrel_king_extra_w);
	}

	if (cart_type == SMOUSE)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400007, 0, 0, smous_prot_r);
	}

	if (cart_type == SMB)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa13001, 0, 0, smbro_prot_r);
	}

	if (cart_type == SMB2)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa13001, 0, 0, smb2_extra_r);
	}

	if (cart_type == KAIJU)
	{
		memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x200000);
		memcpy(&ROM[0x600000], &ROM[VIRGIN_COPY_GEN], 0x200000);
		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x200000);

		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x700000, 0x7fffff, 0, 0, g_kaiju_bank_w);
	}

	if (cart_type == CHINFIGHT3)
	{
		memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x200000);
		memcpy(&ROM[0x600000], &ROM[VIRGIN_COPY_GEN], 0x200000);
		memcpy(&ROM[0x000000], &ROM[VIRGIN_COPY_GEN], 0x200000);

		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x4fffff, 0, 0, g_chifi3_prot_r);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x600000, 0x6fffff, 0, 0, g_chifi3_bank_w);
	}

	if (cart_type == LIONK2)
	{
		lion2_prot1_data = lion2_prot2_data = 0;

		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400002, 0x400003, 0, 0, lion2_prot1_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400006, 0x400007, 0, 0, lion2_prot2_r);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, lion2_prot1_w);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400004, 0x400005, 0, 0, lion2_prot2_w);
	}

	if (cart_type == BUGSLIFE)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa13001, 0, 0, bugl_extra_r);
	}

	if (cart_type == ELFWOR)
	{
	/* It return (0x55 @ 0x400000 OR 0xc9 @ 0x400004) AND (0x0f @ 0x400002 OR 0x18 @ 0x400006). It is probably best to add handlers for all 4 addresses. */

		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, elfwor_0x400000_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400002, 0x400003, 0, 0, elfwor_0x400002_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400004, 0x400005, 0, 0, elfwor_0x400004_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400006, 0x400007, 0, 0, elfwor_0x400006_r);
	}

	if (cart_type == ROCKMANX3)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa13001, 0, 0, rx3_extra_r);
	}

	if (cart_type == SBUBBOB)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, sbub_extra1_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400002, 0x400003, 0, 0, sbub_extra2_r);
	}

	if (cart_type == KOF98)
	{
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x480000, 0x480001, 0, 0, g_kof98_aa_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4800e0, 0x4800e1, 0, 0, g_kof98_aa_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4824a0, 0x4824a1, 0, 0, g_kof98_aa_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x488880, 0x488881, 0, 0, g_kof98_aa_r);

		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4a8820, 0x4a8821, 0, 0, g_kof98_0a_r);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4f8820, 0x4f8821, 0, 0, g_kof98_00_r);
	}

	if (cart_type == REALTEC)
	{
		/* Realtec mapper!*/
		realtec_bank_addr = realtec_bank_size = 0;
		realtec_old_bank_addr = -1;

		memcpy(&ROM[0x400000], &ROM[relocate], 0x80000);

		for (mirroraddr = 0; mirroraddr < 0x400000; mirroraddr += 0x2000)
		{
			memcpy(ROM + mirroraddr, ROM + relocate + 0x7e000, 0x002000); /* copy last 8kb across the whole rom region */
		}

		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, realtec_400000_w);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x402000, 0x402001, 0, 0, realtec_402000_w);
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x404000, 0x404001, 0, 0, realtec_404000_w);
	}

	if (cart_type == SUP19IN1)
	{
		memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x400000); // allow hard reset to menu
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa13039, 0, 0, s19in1_bank);
	}

	if (cart_type == SUP15IN1)
	{
		memcpy(&ROM[0x400000], &ROM[VIRGIN_COPY_GEN], 0x200000); // allow hard reset to menu
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa13000, 0xa13039, 0, 0, s19in1_bank);
	}

	/* Disable SRAM handlers if it's a game using EEPROM to save
    progresses. Currently EEPROM is not emulated. */
	if (!has_serial_eeprom)
	{
		/* Info from DGen: If SRAM does not overlap main ROM, set it active by
        default since a few games can't manage to properly switch it on/off. */
		if (genesis_last_loaded_image_length <= genesis_sram_start)
			genesis_sram_active = 1;

		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa130f0, 0xa130f1, 0, 0, genesis_sram_toggle);

		/* Sonic 1 included in Sonic Classics doesn't have SRAM and
        does lots of ROM access at this range, then only install read/
        write handlers if SRAM is active to not slow down emulation. */
		if (genesis_sram_active)
			install_sram_rw_handlers (machine);
	}

	/* install i2c handlers */
	if(cart_type == NBA_JAM)
		memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, nba_jam_eeprom_r,nba_jam_eeprom_w);

	if(cart_type == WBOY_V)
		memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, wboy_v_eeprom_r,wboy_v_eeprom_w);

	if(cart_type == NBA_JAM_TE || cart_type == NFL_QB_96 || cart_type == C_SLAM) // same handling but different sizes
		memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, nba_jam_te_eeprom_r,nba_jam_te_eeprom_w);

	if(cart_type == EA_NHLPA)
		memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200001, 0, 0, ea_nhlpa_eeprom_r,ea_nhlpa_eeprom_w);

	if(cart_type == CODE_MASTERS)
	{
		memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x300000, 0x300001, 0, 0, codemasters_eeprom_w);
		memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x380000, 0x380001, 0, 0, codemasters_eeprom_r);

		// TODO: J-Cart device
	}

	/* install NOP handler for TMSS */
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa14000, 0xa14003, 0, 0, genesis_TMSS_bank_w);
}


MACHINE_RESET( md_mappers )
{
	setup_megadriv_custom_mappers(machine);
}


/*************************************
 *
 *  Cart handling
 *
 *************************************/


/******* Memcmp for both endianness *******/

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


/******* SMD files detection *******/

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


/******* Image loading *******/

static DEVICE_IMAGE_LOAD( genesis_cart )
{
	unsigned char *ROM, *rawROM, *tmpROMnew, *tmpROM, *secondhalf;
	int relocate, length, ptr, x, sram_detected;
#ifdef LSB_FIRST
	unsigned char fliptemp;
#endif

	genesis_sram = NULL;
	sram_detected = 0;
	genesis_sram_start = genesis_sram_end = 0;
	has_serial_eeprom = 0;
	cart_type = STANDARD;

	if (image.software_entry() == NULL)
	{
		rawROM = image.device().machine->region("maincpu")->base();
		ROM = rawROM /*+ 512 */;

		genesis_last_loaded_image_length = -1;

		length = image.fread( rawROM + 0x2000, 0x600000);

		logerror("image length = 0x%x\n", length);

		/* is this a SMD file? */
		if (genesis_is_SMD(&rawROM[0x2200], (unsigned)length))
		{
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
			genesis_last_loaded_image_length = length - 512;
			memcpy(&ROM[VIRGIN_COPY_GEN], &ROM[0x000000], MAX_MD_CART_SIZE);  /* store a copy of data for MACHINE_RESET processing */

			relocate = 0;

		}

		/* is this a MD file? */
		else if ((rawROM[0x2080] == 'E') && (rawROM[0x2081] == 'A') &&
				 (rawROM[0x2082] == 'M' || rawROM[0x2082] == 'G'))
		{
			tmpROMnew = (unsigned char *)osd_malloc(length);
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
			free(tmpROMnew);

#ifdef LSB_FIRST
			for (ptr = 0; ptr < length; ptr += 2)
			{
				fliptemp = ROM[ptr];
				ROM[ptr] = ROM[ptr+1];
				ROM[ptr+1] = fliptemp;
			}
#endif
			genesis_last_loaded_image_length = length;
			memcpy(&ROM[VIRGIN_COPY_GEN], &ROM[0x000000], MAX_MD_CART_SIZE);  /* store a copy of data for MACHINE_RESET processing */
			relocate = 0;

		}

		/* BIN it is, then */
		else
		{
			relocate = 0x2000;
			genesis_last_loaded_image_length = length;

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

			memcpy(&ROM[VIRGIN_COPY_GEN], &ROM[0x000000], MAX_MD_CART_SIZE);  /* store a copy of data for MACHINE_RESET processing */
		}

		/* Default cartridge type */
		cart_type = STANDARD;

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

			switch (genesis_last_loaded_image_length)
			{
			case 0x80000:
			    if (!allendianmemcmp(&ROM[0x08c8], &smouse_sig[0], sizeof(smouse_sig)))
				cart_type = SMOUSE;

			    if (!allendianmemcmp((char *)&ROM[0x7e30e], "SEGA", 4) ||
				!allendianmemcmp((char *)&ROM[0x7e100], "SEGA", 4) ||
				!allendianmemcmp((char *)&ROM[0x7e1e6], "SEGA", 4))
				cart_type = REALTEC;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-50396", 10)) // NHLPA Hockey 93
				cart_type = EA_NHLPA;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM MK-1215", 10)) // Evander Holyfield
				cart_type = WBOY_V;

			    break;

			case 0xc0000:

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM G-4060 ", 8)) // Wonder Boy V
				cart_type = WBOY_V;

			    break;

			case 0x100000:
			    if (!allendianmemcmp(&ROM[0x01b24], &mjlover_sig[0], sizeof(mjlover_sig)))
				cart_type = MJLOVER;

			    if (!allendianmemcmp(&ROM[0x03b4], &squir_sig[0], sizeof(squir_sig)))
				cart_type = SQUIRRELK;

			    if (!allendianmemcmp(&ROM[0xee0d0], &bugsl_sig[0], sizeof(bugsl_sig)))
				cart_type = BUGSLIFE;

			    if (!allendianmemcmp((char *)&ROM[0x0172], "GAME : ELF WOR", 14))
				cart_type = ELFWOR;

			    if (!allendianmemcmp(&ROM[0x123e4], &sbub_sig[0], sizeof(sbub_sig)))
				cart_type = SBUBBOB;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-50176", 10)) // Rings of Power
				cart_type = EA_NHLPA;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "MK 00001211-00", 14)) // Sports Talk Baseball
				cart_type = WBOY_V;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-120096-", 12)) // Micro Machines 2
				cart_type = CODE_MASTERS;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-120146-", 12)) // Brian Lara Cricket 96 / Shane Wayne Cricket 96
				cart_type = CODE_MASTERS;

			    if (!allendianmemcmp((char *)&ROM[0x0190], "OJKRPTBVFCA     ", 0x10)) // Micro Machines '96 / Military TODO: better way to recognize these?
				cart_type = CODE_MASTERS;
			    break;

			case 0x200000:
			    if (!allendianmemcmp(&ROM[0x18c6], &lk3_sig[0], sizeof(lk3_sig)))
				cart_type = LIONK3;

			    if (!allendianmemcmp(&ROM[0x220], &sdk_sig[0], sizeof(sdk_sig)))
				cart_type = SKINGKONG;

			    if (!allendianmemcmp(&ROM[0xce560], &redcliff_sig[0], sizeof(redcliff_sig)))
				cart_type = REDCLIFF;

			    if (!allendianmemcmp(&ROM[0xc8cb0], &smb_sig[0], sizeof(smb_sig)))
				cart_type = SMB;

			    if (!allendianmemcmp(&ROM[0xf24d6], &smb2_sig[0], sizeof(smb2_sig)))
				cart_type = SMB2;

			    if (!allendianmemcmp(&ROM[0x674e], &kaiju_sig[0], sizeof(kaiju_sig)))
				cart_type = KAIJU;

			    if (!allendianmemcmp(&ROM[0x1780], &chifi3_sig[0], sizeof(chifi3_sig)))
				cart_type = CHINFIGHT3;

			    if (!allendianmemcmp(&ROM[0x03c2], &lionk2_sig[0], sizeof(lionk2_sig)))
				cart_type = LIONK2;

			    if (!allendianmemcmp(&ROM[0xc8b90], &rx3_sig[0], sizeof(rx3_sig)))
				cart_type = ROCKMANX3;

			    if (!allendianmemcmp(&ROM[0x56ae2], &kof98_sig[0], sizeof(kof98_sig)))
				cart_type = KOF98;

			    if (!allendianmemcmp(&ROM[0x17bb2], &s15in1_sig[0], sizeof(s15in1_sig)))
				cart_type = SUP15IN1;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081326 ", 12)) // NBA Jam
				cart_type = NBA_JAM;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM MK-1228", 10)) // Greatest Heavyweight of the Ring
				cart_type = WBOY_V;

			    if ((!allendianmemcmp((char *)&ROM[0x0180], "GM T-12046", 10)) || // Mega Man
				(!allendianmemcmp((char *)&ROM[0x0180], "GM T-12053", 10) && !allendianmemcmp(&ROM[0x18e], &rockman_sig[0], sizeof(rockman_sig)))) // / Rock Man (EEPROM version)
				cart_type = WBOY_V;

			    break;

			case 0x200005:
			    if (!allendianmemcmp(&ROM[0xce564], &redcl_en_sig[0], sizeof(redcliff_sig)))
				cart_type = REDCL_EN;
			    break;

			case 0x300000:
			    if (!allendianmemcmp(&ROM[0x220], &sdk_sig[0], sizeof(sdk_sig)))
				cart_type = SDK99;

			    if (!allendianmemcmp(&ROM[0x1fd0d2], &kof99_sig[0], sizeof(kof99_sig)))
				cart_type = KOF99;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-81406", 10)) // NBA Jam TE
				cart_type = NBA_JAM_TE;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081276 ", 12)) // NFL Quarterback Club
				cart_type = NBA_JAM_TE;

			    break;

			case 0x400000:
			    if (!allendianmemcmp(&ROM[0x3c031c], &radica_sig[0], sizeof(radica_sig)) ||
				!allendianmemcmp(&ROM[0x3f031c], &radica_sig[0], sizeof(radica_sig))) // ssf+gng + radica vol1
				cart_type = RADICA;

			    if (!allendianmemcmp(&ROM[0x028460], &soulb_sig[0], sizeof(soulb_sig)))
				cart_type = SOULBLAD;

			    if (!allendianmemcmp(&ROM[0x1e700], &s19in1_sig[0], sizeof(s19in1_sig)))
				cart_type = SUP19IN1;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081586-", 12)) // NFL Quarterback Club 96
				cart_type = NFL_QB_96;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-081576 ", 12)) // College Slam
				cart_type = C_SLAM;

			    if (!allendianmemcmp((char *)&ROM[0x0180], "GM T-81476", 10)) // Big Hurt Baseball
				cart_type = C_SLAM;

			    break;

			case 0x500000:
			    if (!allendianmemcmp((char *)&ROM[0x0120], "SUPER STREET FIGHTER2 ", 22))
				cart_type = SSF2;
			    break;

			default:
			    break;
			}
		}

		logerror("cart type: %d\n", cart_type);

		genesis_sram = NULL;
		sram_detected = 0;
		genesis_sram_start = genesis_sram_end = 0;
		has_serial_eeprom = 0;

		/* check if cart has battery save */

		/* For games using SRAM, unfortunately there are ROMs without info
           about it in header. The solution adopted is do the mapping anyway,
           then active SRAM later if the game will access it. */

		if (ROM[0x1b1] == 'R' && ROM[0x1b0] == 'A')
		{
			/* SRAM info found in header */
			genesis_sram_start = (ROM[0x1b5] << 24 | ROM[0x1b4] << 16 | ROM[0x1b7] << 8 | ROM[0x1b6]);
			genesis_sram_end = (ROM[0x1b9] << 24 | ROM[0x1b8] << 16 | ROM[0x1bb] << 8 | ROM[0x1ba]);

			if ((genesis_sram_start > genesis_sram_end) || ((genesis_sram_end - genesis_sram_start) >= 0x10000))	// we assume at most 64k of SRAM (HazeMD uses at most 64k). is this correct?
			  genesis_sram_end = genesis_sram_start + 0x0FFFF;

			/* for some games using serial EEPROM, difference between SRAM
               end to start is 0 or 1. Currently EEPROM is not emulated. */
			if ((genesis_sram_end - genesis_sram_start) < 2)
			  has_serial_eeprom = 1;
			else
			  sram_detected = 1;
		}
		else
		{
			/* set default SRAM positions, with size = 64k */
			genesis_sram_start = 0x200000;
			genesis_sram_end = genesis_sram_start + 0xffff;
		}

		if (genesis_sram_start & 1)
		  genesis_sram_start -= 1;

		if (!(genesis_sram_end & 1))
		  genesis_sram_end += 1;

		/* calculate backup RAM location */
		megadriv_backupram = (UINT16*) (ROM + (genesis_sram_start & 0x3fffff));

		/* Until serial EEPROM is emulated, clears one byte at beginning of
           backup RAM, but only if the value is 0xFFFF, to not break MLBPA Sports
           Talk Baseball. With this hack some games at least run (NBA Jam, Evander
           Holyfield's Real Deal Boxing, Greatest Heavyweights of the Ring) */
		//if (has_serial_eeprom && megadriv_backupram[0] == 0xffff)
		//  megadriv_backupram[0] = 0xff00;

		if (sram_detected)
		  logerror("SRAM detected from header: starting location %X - SRAM Length %X\n", genesis_sram_start, genesis_sram_end - genesis_sram_start + 1);
	}
	else /* Handle pcb with the softlist info */
	{
		int		pcb_id;
		const char	*pcb_name;

		length = image.get_software_region_length("rom");
		ROM = image.device().machine->region("maincpu")->base();
		memcpy(ROM, image.get_software_region("rom"), length);

		genesis_last_loaded_image_length = length;
		megadriv_backupram = NULL;
		if ((pcb_name = image.get_feature("pcb")) == NULL)
		  pcb_id = STD_ROM;
		else
		  pcb_id = md_get_pcb_id(pcb_name);

		switch (pcb_id)
		{
			/* Sega PCB */
		  case SEGA_6584A:
			fatalerror("Need Serial EEPROM emulation");
			break;
		  case SEGA_5921:
		  case SEGA_6278A:
			genesis_sram_start = 0x200000;
			genesis_sram_end = genesis_sram_start + 0x3fff;
			sram_detected = 1;
			megadriv_backupram = (UINT16*) (ROM + (genesis_sram_start & 0x3fffff));
			break;
		  case SEGA_6658A:
			genesis_sram_start = 0x200000;
			genesis_sram_end = genesis_sram_start + 0x3ff;
			sram_detected = 1;
			megadriv_backupram = (UINT16*) (ROM + (genesis_sram_start & 0x3fffff));
			break;

			/* Codemasters PCB (J-Carts) */
		  case CM_JCART:
			memory_install_read16_handler(cputag_get_address_space(image.device().machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x38FFFE, 0x38FFFF, 0, 0, jcart_ctrl_r);
			memory_install_write16_handler(cputag_get_address_space(image.device().machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x38FFFE, 0x38FFFF, 0, 0, jcart_ctrl_w);
			break;
		  case CM_JCART_SEPROM:
			memory_install_read16_handler(cputag_get_address_space(image.device().machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x38FFFE, 0x38FFFF, 0, 0, jcart_ctrl_r);
			memory_install_write16_handler(cputag_get_address_space(image.device().machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x38FFFE, 0x38FFFF, 0, 0, jcart_ctrl_w);
			/* TODO add SEPROM part */
			break;
		}
	}

	return IMAGE_INIT_PASS;
}

/******* Image unloading (with SRAM saving) *******/

static DEVICE_IMAGE_UNLOAD( genesis_cart )
{
	/* Write out the battery file if necessary */
	if (genesis_sram != NULL)
	{
		image.battery_save(genesis_sram, genesis_sram_end - genesis_sram_start + 1);
		free(genesis_sram);
	}
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
		temp_copy = auto_alloc_array(image.device().machine, UINT8, length);
		image.fread( temp_copy, length);
	}
	else
	{
		length = image.get_software_region_length("rom");
		temp_copy = auto_alloc_array(image.device().machine, UINT8, length);
		memcpy(temp_copy, image.get_software_region("rom"), length);
	}

	/* Copy the cart image in the locations the driver expects */
	// Notice that, by using pick_integer, we are sure the code works on both LE and BE machines
	ROM16 = (UINT16 *) image.device().machine->region("gamecart")->base();
	for (i = 0; i < length; i += 2)
		ROM16[i / 2] = pick_integer_be(temp_copy, i, 2);

	ROM32 = (UINT32 *) image.device().machine->region("gamecart_sh2")->base();
	for (i = 0; i < length; i += 4)
		ROM32[i / 4] = pick_integer_be(temp_copy, i, 4);

	ROM16 = (UINT16 *) image.device().machine->region("maincpu")->base();
	for (i = 0x00; i < length; i += 2)
		ROM16[i / 2] = pick_integer_be(temp_copy, i, 2);

	auto_free(image.device().machine, temp_copy);

	cart_type = NBA_JAM_TE;

	return IMAGE_INIT_PASS;
}

/******* Cart getinfo *******/

MACHINE_CONFIG_FRAGMENT( genesis_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("smd,bin,md,gen")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("megadriv_cart")
	MCFG_CARTSLOT_LOAD(genesis_cart)
	MCFG_CARTSLOT_UNLOAD(genesis_cart)
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
	MCFG_CARTSLOT_UNLOAD(genesis_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","pico")
MACHINE_CONFIG_END
