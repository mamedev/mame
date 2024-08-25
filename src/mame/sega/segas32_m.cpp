// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* Sega System 32 Protection related functions */

#include "emu.h"
#include "segas32.h"


/******************************************************************************
 ******************************************************************************
  Golden Axe 2 (Revenge of Death Adder)
 ******************************************************************************
 ******************************************************************************/

#define xxxx 0x00

const uint8_t segas32_v25_state::ga2_opcode_table[256] = {
		xxxx,xxxx,0xEA,xxxx,xxxx,0x8B,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xFA,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x3B,xxxx,0x49,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,0xE8,xxxx,xxxx,0x75,xxxx,xxxx,xxxx,xxxx,0x3A,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,0x8D,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBF,xxxx,0x88,xxxx,
		xxxx,xxxx,xxxx,0x81,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		0x02,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBC,
		xxxx,xxxx,xxxx,0x8A,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x83,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB8,0x26,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB5,xxxx,0xEB,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB2,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,0xC3,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB9,0xBB,xxxx,0x43,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,0x8E,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBE,xxxx,0x80,xxxx,xxxx
};

#undef xxxx

void segas32_v25_state::decrypt_protrom()
{
	uint8_t *rom = memregion("mcu")->base();
	std::vector<uint8_t> temp(0x100000);

	// make copy of ROM so original can be overwritten
	memcpy(&temp[0], rom, 0x10000);

	// unscramble the address lines
	for (int i = 0; i < 0x10000; i++)
		rom[i] = temp[bitswap<16>(i, 14, 11, 15, 12, 13, 4, 3, 7, 5, 10, 2, 8, 9, 6, 1, 0)];
}


#if 0 // simulation
uint16_t segas32_state::ga2_sprite_protection_r(offs_t offset)
{
	static const uint16_t prot[16] =
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

uint16_t segas32_state::ga2_wakeup_protection_r(offs_t offset)
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
#define CLEARED_LEVELS          0xE5C4
#define CURRENT_LEVEL           0xF06E
#define CURRENT_LEVEL_STATUS    0xF0BC
#define LEVEL_ORDER_ARRAY       0x263A

void segas32_state::sonic_level_load_protection(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t level;

	// Perform write
	COMBINE_DATA(&m_system32_workram[CLEARED_LEVELS / 2]);

	// Refresh current level
	if (m_system32_workram[CLEARED_LEVELS / 2] == 0)
	{
		level = 0x0007;
	}
	else
	{
		const uint8_t *ROM = m_maincpu_region->base();
		level =  *((ROM + LEVEL_ORDER_ARRAY) + (m_system32_workram[CLEARED_LEVELS / 2] * 2) - 1);
		level |= *((ROM + LEVEL_ORDER_ARRAY) + (m_system32_workram[CLEARED_LEVELS / 2] * 2) - 2) << 8;
	}
	m_system32_workram[CURRENT_LEVEL / 2] = level;

	// Reset level status
	m_system32_workram[CURRENT_LEVEL_STATUS / 2] = 0x0000;
	m_system32_workram[(CURRENT_LEVEL_STATUS + 2) / 2] = 0x0000;
}


/******************************************************************************
 ******************************************************************************
  Burning Rival
 ******************************************************************************
 ******************************************************************************/


// the protection board on many system32 games has full dma/bus access
// and can write things into work RAM.  we simulate that here for burning rival.
uint16_t segas32_state::brival_protection_r(offs_t offset, uint16_t mem_mask)
{
	if (mem_mask == 0xffff) // only trap on word-wide reads
	{
		switch (offset)
		{
			case 0:
			case 2:
			case 3:
				return 0;
		}
	}

	return m_system32_workram[0xba00/2 + offset];
}

void segas32_state::brival_protection_w(offs_t offset, uint16_t data)
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
	uint8_t *ROM = m_maincpu_region->base();

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

	memcpy(&m_system32_protram[protAddress[curProtType][1]], ret, 16);
}


/******************************************************************************
 ******************************************************************************
  Dark Edge
 ******************************************************************************
 ******************************************************************************/

void segas32_state::darkedge_fd1149_vblank()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.write_word(0x20f072, 0);
	space.write_word(0x20f082, 0);

	if (space.read_byte(0x20a12c) != 0)
	{
		space.write_byte(0x20a12c, space.read_byte(0x20a12c)-1);

		if (space.read_byte(0x20a12c) == 0)
			space.write_byte(0x20a12e, 1);
	}
}

void segas32_state::darkedge_protection_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s:darkedge_prot_w(%06X) = %04X & %04X\n",
		machine().describe_context(), 0xa00000 + 2*offset, data, mem_mask);
}


uint16_t segas32_state::darkedge_protection_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s:darkedge_prot_r(%06X) & %04X\n",
		machine().describe_context(), 0xa00000 + 2*offset, mem_mask);
	return 0xffff;
}

/******************************************************************************
 ******************************************************************************
  F1 Super Lap
 ******************************************************************************
 ******************************************************************************/

void segas32_state::f1lap_fd1149_vblank()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.write_byte(0x20f7c6, 0);

	// needed to start a game
	uint8_t val = space.read_byte(0x20ee81);
	if (val == 0xff)
		space.write_byte(0x20ee81,0);
}



/******************************************************************************
 ******************************************************************************
  DBZ VRVS
 ******************************************************************************
 ******************************************************************************/

void segas32_state::dbzvrvs_protection_w(address_space &space, uint16_t data)
{
	space.write_word(0x2080c8, space.read_word(0x200044));
}


uint16_t segas32_state::dbzvrvs_protection_r()
{
	return 0xffff;
}



/******************************************************************************
 ******************************************************************************
  Arabian Fight
 ******************************************************************************
 ******************************************************************************/

#define xxxx 0x00

const uint8_t segas32_v25_state::arf_opcode_table[256] = {
		xxxx,xxxx,0x43,xxxx,xxxx,xxxx,0x83,xxxx,xxxx,xxxx,0xEA,xxxx,xxxx,0xBC,0x73,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		0x3A,xxxx,xxxx,0xBE,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x80,xxxx,
		xxxx,0xB5,xxxx,xxxx,xxxx,xxxx,xxxx,0x26,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xE8,0x8D,xxxx,0x8B,xxxx,
		xxxx,xxxx,xxxx,0xFA,xxxx,0x8A,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBA,0x88,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBB,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x75,xxxx,0xBF,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x03,0x3B,0x8E,0x74,xxxx,xxxx,0x81,xxxx,
		xxxx,xxxx,xxxx,0xC3,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB9,0xB2,xxxx,xxxx,xxxx,xxxx,0x49,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xEB,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
		xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x02,0xB8
};

#undef xxxx

#if 0 // old simulation
uint16_t segas32_state::arabfgt_protection_r()
{
	int PC = m_maincpu->pc();
	int cmpVal;

	if (PC == 0xfe0325 || PC == 0xfe01e5 || PC == 0xfe035e || PC == 0xfe03cc)
	{
		cmpVal = m_maincpu->state_int(V60_R0);

		// R0 always contains the value the protection is supposed to return (!)
		return cmpVal;
	}
	else
	{
		popmessage("UNKONWN ARF PROTECTION READ PC=%x\n", PC);
	}

	return 0;
}

void segas32_state::arabfgt_protection_w(uint16_t data)
{
}

uint16_t segas32_state::arf_wakeup_protection_r(offs_t offset)
{
	static const char prot[] =
		"wake up! ARF!                                   ";
	return prot[offset];
}
#endif

/******************************************************************************
 ******************************************************************************
  The J.League 1994 (Japan)
 ******************************************************************************
 ******************************************************************************/
void segas32_state::jleague_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_system32_workram[0xf700/2 + offset]);

	switch (offset)
	{
		// Map team browser selection to opponent browser selection
		// using same lookup table that V60 uses for sound sample mapping.
		case 0:
			space.write_byte(0x20f708, space.read_word(0x7bbc0 + data*2));
			break;

		// move on to team browser
		case 4/2:
			space.write_byte(0x200016, data & 0xff);
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

uint16_t segas32_state::arescue_dsp_r(offs_t offset)
{
	if (offset == 4/2)
	{
		switch (m_arescue_dsp_io[0])
		{
			case 0:
			case 1:
			case 2:
				break;

			case 3:
				m_arescue_dsp_io[0] = 0x8000;
				m_arescue_dsp_io[2/2] = 0x0001;
				break;

			case 6:
				m_arescue_dsp_io[0] = 4 * m_arescue_dsp_io[2/2];
				break;

			default:
				logerror("Unhandled DSP cmd %04x (%04x).\n", m_arescue_dsp_io[0], m_arescue_dsp_io[1]);
				break;
		}
	}

	return m_arescue_dsp_io[offset];
}

void segas32_state::arescue_dsp_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_arescue_dsp_io[offset]);
}
