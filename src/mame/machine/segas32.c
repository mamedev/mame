/* Sega System 32 Protection related functions */

#include "emu.h"
#include "includes/segas32.h"


/******************************************************************************
 ******************************************************************************
  Golden Axe 2 (Revenge of Death Adder)
 ******************************************************************************
 ******************************************************************************/

#define xxxx 0x00

static const UINT8 ga2_v25_opcode_table[256] = {
     xxxx,xxxx,0xEA,xxxx,xxxx,0x8B,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xFA,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x49,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,0xE8,xxxx,xxxx,0x75,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,0x8D,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBF,xxxx,0x88,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBC,
     xxxx,xxxx,xxxx,0x8A,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x83,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB8,0x26,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xEB,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB2,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,0xC3,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB9,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,0x8E,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBE,xxxx,xxxx,xxxx,xxxx
};

#undef xxxx

static void nec_v25_cpu_decrypt(running_machine *machine)
{
	int i;
	address_space *space = cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_PROGRAM);
	UINT8 *rom = memory_region(machine, "mcu");
	UINT8* decrypted = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8* temp = auto_alloc_array(machine, UINT8, 0x100000);

	// set CPU3 opcode base
	space->set_decrypted_region(0x00000, 0x0ffff, decrypted);
	space->set_decrypted_region(0xf0000, 0xfffff, decrypted);

	// make copy of ROM so original can be overwritten
	memcpy(temp, rom, 0x10000);

	for(i = 0; i < 0x10000; i++)
	{
		int j = BITSWAP16(i, 14, 11, 15, 12, 13, 4, 3, 7, 5, 10, 2, 8, 9, 6, 1, 0);

		// normal ROM data with address swap undone
		rom[i] = temp[j];

		// decryped opcodes with address swap undone
		decrypted[i] = ga2_v25_opcode_table[ temp[j] ];
	}

	auto_free(machine, temp);
}

void decrypt_ga2_protrom(running_machine *machine)
{
	nec_v25_cpu_decrypt(machine);
}


WRITE16_HANDLER( ga2_dpram_w )
{
	/* does it ever actually write.. */
}

READ16_HANDLER( ga2_dpram_r )
{
	return (ga2_dpram[offset])|(ga2_dpram[offset+1]<<8);
}


#if 0 // simulation
READ16_HANDLER(ga2_sprite_protection_r)
{
	static const UINT16 prot[16] =
	{
		0x0a, 0,
		0xc5, 0,
		0x11, 0,
		0x11, 0,
		0x18, 0,
		0x18, 0,
		0x1f, 0,
		0xc6, 0,
	};

	return prot[offset];
}

READ16_HANDLER(ga2_wakeup_protection_r)
{
	static const char prot[] =
		"wake up! GOLDEN AXE The Revenge of Death-Adder! ";
	return prot[offset];
}
#endif

/******************************************************************************
 ******************************************************************************
  Sonic Arcade protection
 ******************************************************************************
 ******************************************************************************/


// This code duplicates the actions of the protection device used in SegaSonic
// arcade revision C, allowing the game to run correctly.
#define CLEARED_LEVELS			0xE5C4
#define CURRENT_LEVEL			0xF06E
#define CURRENT_LEVEL_STATUS		0xF0BC
#define LEVEL_ORDER_ARRAY		0x263A

WRITE16_HANDLER(sonic_level_load_protection)
{
	UINT16 level;
//Perform write
	system32_workram[CLEARED_LEVELS / 2] = (data & mem_mask) | (system32_workram[CLEARED_LEVELS / 2] & ~mem_mask);

//Refresh current level
		if (system32_workram[CLEARED_LEVELS / 2] == 0)
		{
			level = 0x0007;
		}
		else
		{
			const UINT8 *ROM = memory_region(space->machine, "maincpu");
			level =  *((ROM + LEVEL_ORDER_ARRAY) + (system32_workram[CLEARED_LEVELS / 2] * 2) - 1);
			level |= *((ROM + LEVEL_ORDER_ARRAY) + (system32_workram[CLEARED_LEVELS / 2] * 2) - 2) << 8;
		}
		system32_workram[CURRENT_LEVEL / 2] = level;

//Reset level status
		system32_workram[CURRENT_LEVEL_STATUS / 2] = 0x0000;
		system32_workram[(CURRENT_LEVEL_STATUS + 2) / 2] = 0x0000;
}


/******************************************************************************
 ******************************************************************************
  Burning Rival
 ******************************************************************************
 ******************************************************************************/


// the protection board on many system32 games has full dma/bus access
// and can write things into work RAM.  we simulate that here for burning rival.
READ16_HANDLER(brival_protection_r)
{
	if (mem_mask == 0xffff)	// only trap on word-wide reads
	{
		switch (offset)
		{
			case 0:
			case 2:
			case 3:
				return 0;
		}
	}

	return system32_workram[0xba00/2 + offset];
}

WRITE16_HANDLER(brival_protection_w)
{
	static const int protAddress[6][2] =
	{
		{ 0x109517, 0x00/2 },
		{ 0x109597, 0x10/2 },
		{ 0x109597, 0x20/2 },
		{ 0x109597, 0x30/2 },
		{ 0x109597, 0x40/2 },
		{ 0x109617, 0x50/2 },
	};
	char ret[32];
	int curProtType;
	UINT8 *ROM = memory_region(space->machine, "maincpu");

	switch (offset)
	{
		case 0x800/2:
			curProtType = 0;
			break;
		case 0x802/2:
			curProtType = 1;
			break;
		case 0x804/2:
			curProtType = 2;
			break;
		case 0x806/2:
			curProtType = 3;
			break;
		case 0x808/2:
			curProtType = 4;
			break;
		case 0x80a/2:
			curProtType = 5;
			break;
		default:
			if (offset >= 0xa00/2 && offset < 0xc00/2)
				return;
			logerror("brival_protection_w: UNKNOWN WRITE: offset %x value %x\n", offset, data);
			return;
	}

	memcpy(ret, &ROM[protAddress[curProtType][0]], 16);
	ret[16] = '\0';

	memcpy(&system32_protram[protAddress[curProtType][1]], ret, 16);
}


/******************************************************************************
 ******************************************************************************
  Dark Edge
 ******************************************************************************
 ******************************************************************************/

void darkedge_fd1149_vblank(running_device *device)
{
	address_space *space = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);

	space->write_word(0x20f072, 0);
	space->write_word(0x20f082, 0);

	if( space->read_byte(0x20a12c) != 0 )
	{
		space->write_byte(0x20a12c, space->read_byte(0x20a12c)-1 );

		if( space->read_byte(0x20a12c) == 0 )
			space->write_byte(0x20a12e, 1);
	}
}


WRITE16_HANDLER( darkedge_protection_w )
{
	logerror("%06x:darkedge_prot_w(%06X) = %04X & %04X\n",
		cpu_get_pc(space->cpu), 0xa00000 + 2*offset, data, mem_mask);
}


READ16_HANDLER( darkedge_protection_r )
{
	logerror("%06x:darkedge_prot_r(%06X) & %04X\n",
		cpu_get_pc(space->cpu), 0xa00000 + 2*offset, mem_mask);
	return 0xffff;
}



/******************************************************************************
 ******************************************************************************
  DBZ VRVS
 ******************************************************************************
 ******************************************************************************/

WRITE16_HANDLER( dbzvrvs_protection_w )
{
	space->write_word( 0x2080c8, space->read_word( 0x200044 ) );
}


READ16_HANDLER( dbzvrvs_protection_r )
{
	return 0xffff;
}



/******************************************************************************
 ******************************************************************************
  Arabian Fight
 ******************************************************************************
 ******************************************************************************/


// protection ram is 8-bits wide and only occupies every other address
READ16_HANDLER(arabfgt_protection_r)
{
	int PC = cpu_get_pc(space->cpu);
	int cmpVal;

	if (PC == 0xfe0325 || PC == 0xfe01e5 || PC == 0xfe035e || PC == 0xfe03cc)
	{
		cmpVal = cpu_get_reg(space->cpu, 1);

		// R0 always contains the value the protection is supposed to return (!)
		return cmpVal;
	}
	else
	{
		popmessage("UNKONWN ARF PROTECTION READ PC=%x\n", PC);
	}

	return 0;
}

WRITE16_HANDLER(arabfgt_protection_w)
{
}

READ16_HANDLER(arf_wakeup_protection_r)
{
	static const char prot[] =
		"wake up! ARF!                                   ";
	return prot[offset];
}

/******************************************************************************
 ******************************************************************************
  The J.League 1994 (Japan)
 ******************************************************************************
 ******************************************************************************/
WRITE16_HANDLER( jleague_protection_w )
{
	COMBINE_DATA( &system32_workram[0xf700/2 + offset ] );

	switch( offset )
	{
		// Map team browser selection to opponent browser selection
		// using same lookup table that V60 uses for sound sample mapping.
		case 0:
			space->write_byte( 0x20f708, space->read_word( 0x7bbc0 + data*2 ) );
			break;

		// move on to team browser
		case 4/2:
			space->write_byte( 0x200016, data & 0xff );
			break;

		default:
			break;
	}
}

/******************************************************************************
 ******************************************************************************
  Air Rescue
 ******************************************************************************
 ******************************************************************************/
/*
    protection
    a00000 - a00002 dsp i/o
    a00004 - dsp int/ack

    dsp uses its p0/p1 for address select
    dsp.sr = ???0 read a00000 into dsp.a
    dsp.sr = ???1 read a00002 into dsp.b
    dsp.sr = ???2 write dsp.b in a00000
    dsp.sr = ???3 write dsp.a in a00002

    Use of p0/p1 means there's no other way for dsp to communicate with V60, unless it shares RAM.
    99.99% of the dsp code is unused because the V60 ROM is hardcoded as part of a twin set,
    maybe the standalone board was for dev only? nop the 3 bytes at 0x06023A for standalone. (centred intro text)
*/
static UINT16 arescue_dsp_io[6] = {0,0,0,0,0,0};

READ16_HANDLER( arescue_dsp_r )
{
	if( offset == 4/2 )
	{
		switch( arescue_dsp_io[0] )
		{
			case 0:
			case 1:
			case 2:
				break;

			case 3:
				arescue_dsp_io[0] = 0x8000;
				arescue_dsp_io[2/2] = 0x0001;
				break;

			case 6:
				arescue_dsp_io[0] = 4 * arescue_dsp_io[2/2];
				break;

			default:
				logerror("Unhandled DSP cmd %04x (%04x).\n", arescue_dsp_io[0], arescue_dsp_io[1] );
				break;
		}
	}

	return arescue_dsp_io[offset];
}

WRITE16_HANDLER( arescue_dsp_w )
{
	COMBINE_DATA(&arescue_dsp_io[offset]);
}

