// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***************************************************************************

Namco System II

  namcos2.cpp

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6805/m6805.h"
#include "namcos2.h"
#include "machine/nvram.h"


uint16_t namcos2_state::namcos2_finallap_prot_r(offs_t offset)
{
	static const uint16_t table0[8] = { 0x0000,0x0040,0x0440,0x2440,0x2480,0xa080,0x8081,0x8041 };
	static const uint16_t table1[8] = { 0x0040,0x0060,0x0060,0x0860,0x0864,0x08e4,0x08e5,0x08a5 };
	uint16_t data;

	switch( offset )
	{
	case 0:
		data = 0x0101;
		break;

	case 1:
		data = 0x3e55;
		break;

	case 2:
		data = table1[m_finallap_prot_count&7];
		data = (data&0xff00)>>8;
		break;

	case 3:
		data = table1[m_finallap_prot_count&7];
		m_finallap_prot_count++;
		data = data&0x00ff;
		break;

	case 0x3fffc/2:
		data = table0[m_finallap_prot_count&7];
		data = data&0xff00;
		break;

	case 0x3fffe/2:
		data = table0[m_finallap_prot_count&7];
		m_finallap_prot_count++;
		data = (data&0x00ff)<<8;
		break;

	default:
		data = 0;
	}
	return data;
}

/*************************************************************/
/* Perform basic machine initialisation                      */
/*************************************************************/


// S2 copy

void namcos2_state::machine_start()
{
	m_eeprom = std::make_unique<uint8_t[]>(0x2000);
	subdevice<nvram_device>("nvram")->set_base(m_eeprom.get(), 0x2000);

	uint32_t max = memregion("audiocpu")->bytes() / 0x4000;
	for (int i = 0; i < 0x10; i++)
		m_audiobank->configure_entry(i, memregion("audiocpu")->base() + (i % max) * 0x4000);

	save_pointer(NAME(m_eeprom), 0x2000);
	save_item(NAME(m_sendval));
}

void gollygho_state::machine_start()
{
	namcos2_state::machine_start();

	// resolve outputs
	m_out_digit.resolve();
	m_out_diorama.resolve();
	m_out_gun_recoil.resolve();
}

void namcos2_state::machine_reset()
{
	/* Initialise the bank select in the sound CPU */
	m_audiobank->set_entry(0); /* Page in bank 0 */

	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE );

	/* Place CPU2 & CPU3 into the reset condition */
	reset_all_subcpus(ASSERT_LINE);
}


void namcos2_state::reset_all_subcpus(int state)
{
	m_slave->set_input_line(INPUT_LINE_RESET, state);
	if (m_c68)
	{
		m_c68->ext_reset(state);
	}
	else if (m_c65)
	{
		m_c65->ext_reset(state);
	}
	else
	{
		logerror("no MCU to reset?\n");
	}
}

void namcos2_state::sound_reset_w(uint8_t data)
{
	if (data & 0x01)
	{
		/* Resume execution */
		m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_maincpu->yield();
	}
	else
	{
		/* Suspend execution */
		m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}

void namcos2_state::system_reset_w(uint8_t data)
{
	reset_all_subcpus(data & 1 ? CLEAR_LINE : ASSERT_LINE);

	if (data & 0x01)
		m_maincpu->yield();
}

/*************************************************************/
/* EEPROM Load/Save and read/write handling                  */
/*************************************************************/

void namcos2_state::eeprom_w(offs_t offset, uint8_t data)
{
	m_eeprom[offset] = data;
}

uint8_t namcos2_state::eeprom_r(offs_t offset)
{
	return m_eeprom[offset];
}

/*************************************************************/
/* 68000 Shared protection/random key generator              */
/*************************************************************

Custom chip ID numbers:

Game        Year    ID (dec)    ID (hex)
--------    ----    ---         -----
finallap    1987
assault     1988    unused
metlhawk    1988
ordyne      1988    176         $00b0
mirninja    1988    177         $00b1
phelios     1988    178         $00b2   readme says 179
dirtfoxj    1989    180         $00b4
fourtrax    1989
valkyrie    1989
finehour    1989    188         $00bc
burnforc    1989    189         $00bd
marvland    1989    190         $00be
kyukaidk    1990    191         $00bf
dsaber      1990    192         $00c0
finalap2    1990    318         $013e
rthun2      1990    319         $013f
gollygho    1990                $0143
cosmogng    1991    330         $014a
sgunner2    1991    346         $015a   ID out of order; gfx board is not standard
finalap3    1992    318         $013e   same as finalap2
suzuka8h    1992
sws92       1992    331         $014b
sws92g      1992    332         $014c
suzuk8h2    1993
sws93       1993    334         $014e
 *************************************************************/

uint16_t namcos2_state::namcos2_68k_key_r(offs_t offset)
{
	switch (m_gametype)
	{
	case NAMCOS2_ORDYNE:
		switch(offset)
		{
		case 2: return 0x1001;
		case 3: return 0x1;
		case 4: return 0x110;
		case 5: return 0x10;
		case 6: return 0xB0;
		case 7: return 0xB0;
		}
		break;

	case NAMCOS2_STEEL_GUNNER_2:
		switch(offset)
		{
			case 4: return 0x15a;
		}
		break;

	case NAMCOS2_MIRAI_NINJA:
		switch(offset)
		{
		case 7: return 0xB1;
		}
		break;

	case NAMCOS2_PHELIOS:
		switch(offset)
		{
		case 0: return 0xF0;
		case 1: return 0xFF0;
		case 2: return 0xB2;
		case 3: return 0xB2;
		case 4: return 0xF;
		case 5: return 0xF00F;
		case 7: return 0xB2;
		}
		break;

	case NAMCOS2_DIRT_FOX_JP:
		switch(offset)
		{
		case 1: return 0xB4;
		}
		break;

	case NAMCOS2_FINEST_HOUR:
		switch(offset)
		{
		case 7: return 0xBC;
		}
		break;

	case NAMCOS2_BURNING_FORCE:
		switch(offset)
		{
		case 1: return 0xBD;
		}
		break;

	case NAMCOS2_MARVEL_LAND:
		switch(offset)
		{
		case 0: return 0x10;
		case 1: return 0x110;
		case 4: return 0xBE;
		case 6: return 0x1001;
		case 7: return (m_sendval==1)?0xBE:1;
		}
		break;

	case NAMCOS2_DRAGON_SABER:
		switch(offset)
		{
		case 2: return 0xC0;
		}
		break;

	case NAMCOS2_ROLLING_THUNDER_2:
		switch(offset)
		{
		case 4:
			if( m_sendval == 1 ){
				m_sendval = 0;
				return 0x13F;
			}
			break;
		case 7:
			if( m_sendval == 1 ){
				m_sendval = 0;
				return 0x13F;
			}
			break;
		case 2: return 0;
		}
		break;

	case NAMCOS2_COSMO_GANG:
		switch(offset)
		{
		case 3: return 0x14A;
		}
		break;

	case NAMCOS2_SUPER_WSTADIUM:
		switch(offset)
		{
	//  case 3: return 0x142;
		case 4: return 0x142;
		default: return machine().rand();
		}

	case NAMCOS2_SUPER_WSTADIUM_92:
		switch(offset)
		{
		case 3: return 0x14B;
		}
		break;

	case NAMCOS2_SUPER_WSTADIUM_92T:
		switch(offset)
		{
		case 3: return 0x14C;
		}
		break;

	case NAMCOS2_SUPER_WSTADIUM_93:
		switch(offset)
		{
		case 3: return 0x14E;
		}
		break;

	case NAMCOS2_SUZUKA_8_HOURS_2:
		switch(offset)
		{
		case 3: return 0x14D;
		case 2: return 0;
		}
		break;

	case NAMCOS2_GOLLY_GHOST:
		switch(offset)
		{
		case 0: return 2;
		case 1: return 2;
		case 2: return 0;
		case 4: return 0x143;
		}
		break;


	case NAMCOS2_BUBBLE_TROUBLE:
		switch(offset)
		{
		case 0: return 2; // not verified
		case 1: return 2; // not verified
		case 2: return 0; // not verified
		case 4: return 0x141;
		}
		break;
	}

	return machine().rand()&0xffff;
}

void namcos2_state::namcos2_68k_key_w(offs_t offset, uint16_t data)
{
	if( m_gametype == NAMCOS2_MARVEL_LAND && offset == 5 )
	{
		if (data == 0x615E) m_sendval = 1;
	}
	if( m_gametype == NAMCOS2_ROLLING_THUNDER_2 && offset == 4 )
	{
		if (data == 0x13EC) m_sendval = 1;
	}
	if( m_gametype == NAMCOS2_ROLLING_THUNDER_2 && offset == 7 )
	{
		if (data == 0x13EC) m_sendval = 1;
	}
	if( m_gametype == NAMCOS2_MARVEL_LAND && offset == 6 )
	{
		if (data == 0x1001) m_sendval = 0;
	}
}

/*************************************************************/
/* 68000 Interrupt/IO Handlers - CUSTOM 148 - NOT SHARED     */
/*************************************************************/

#define NO_OF_LINES     256
#define FRAME_TIME      (1.0/60.0)
#define LINE_LENGTH     (FRAME_TIME/NO_OF_LINES)



/**************************************************************/
/*  Sound sub-system                                          */
/**************************************************************/

void namcos2_state::sound_bankselect_w(uint8_t data)
{
	m_audiobank->set_entry(data>>4);
}

uint16_t namcos2_state::c140_rom_r(offs_t offset)
{
	/*
	    Verified from schematics:
	    MD0-MD3 : Connected in 3N "voice0" D0-D3 or D4-D7, Nibble changeable with 74LS157
	    MD4-MD11 : Connected in 3M "voice1" or 3L "voice2" D0-D7
	    MA0-MA18 : Connected in Address bus of ROMs
	    MA19 : Connected in 74LS157 Select Pin
	    MA20 : Connected in 74LS157 Strobe(Enable) Pin
	    MA21 : ROM select in MD4-MD11 area
	*/
	if (m_c140_region != nullptr)
	{
		bool romsel = BIT(offset, 21);
		bool lsb_en = BIT(~offset, 20);
		bool lsb_swap = BIT(~offset, 19);
		offset &= 0x7ffff;
		u16 ret = m_c140_region[(romsel << 19) | offset] & 0xff00; // voice1 or voice2
		if (lsb_en)
		{
			u8 lsb = m_c140_region[offset] & 0xff; // voice0
			if (lsb_swap)
				lsb <<= 4; // D0-D3

			ret |= (lsb & 0xf0);
		}
		return ret;
	}
	return 0;
}
