// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/************************************************************************************************************************

    stv.cpp

    ST-V hardware
    This file contains all game specific overrides

    TODO:
    - clean this up!
    - Properly emulate the protection chips, used by several games

    (per-game issues)
    - stress: accesses the Sound Memory Expansion Area (0x05a80000-0x05afffff), unknown purpose;

    - smleague / finlarch: it randomly hangs / crashes, it works if you use a ridiculous set_maximum_quantum() number,
      might need strict SH-2 synching or it's actually a m68k comms issue.

    - groovef: ugly back screen color, caused by incorrect usage of the Color Calculation function.

    - myfairld: Apparently this game gives a black screen (either test mode and in-game mode), but let it wait for about
      10 seconds and the game will load everything. This is because of a hellishly slow m68k sub-routine located at 54c2.
      Likely to not be a bug but an in-game design issue.

    - danchih / danchiq: currently hangs randomly (regression).

    - vfremix: when you play as Akira, there is a problem with third match: game doesn't upload all textures
      and tiles and doesn't enable display, although gameplay is normal - wait a while to get back
      to title screen after losing a match (fixed?)

    - vfremix: various problems with SCU DSP: Jeffry causes a black screen hang. Akira's kick sometimes
      sends the opponent out of the ring from whatever position.

    - critcrsh: has a 2 digits 7-seg LED. The current implementation works in test mode, but during gameplay it's stuck
      on the day's best score, while according to reports it should update the number of critters you have crushed.
      At the end of the round it outputs the number of crushed critters. Also needs ticket dispenser hookup (redemption
      is disabled by the game by default, but can be turned on in test menu).
      Reference video: https://www.youtube.com/watch?v=O9PyIKdSFnU

************************************************************************************************************************/

#include "emu.h"
#include "includes/stv.h"
#include "audio/rax.h"

#include "cpu/m68000/m68000.h"
#include "cpu/scudsp/scudsp.h"
#include "cpu/sh/sh2.h"
#include "machine/smpc.h"
#include "machine/stvcd.h"
#include "sound/scsp.h"

#include "softlist.h"
#include "speaker.h"

#include "critcrsh.lh"
#include "segabill.lh"
#include "segabillv.lh"

#define FIRST_SPEEDUP_SLOT  (2)         // in case we remove/alter the BIOS speedups later


/*
Sega 315-5649 IO IC, functional same as 315-5338A, also used in Model 2/3, integrated into 315-6146 'MIE' MCU, etc
I/O overview:                   Connector (as described in service manual)
    PORT-A  1st player inputs   JAMMA (56P)
    PORT-B  2nd player inputs   JAMMA (56P)
    PORT-C  system input        JAMMA (56P)
    PORT-D  system output       JAMMA (56P) + CN25 (JST NH 5P) RESERVED OUTPUT 4bit. (?)
    PORT-E  I/O 1               CN32 (JST NH 9P) EXTENSION I/O 8bit.
    PORT-F  I/O 2               CN21 (JST NH 11P) EXTENSION I/O 8bit.
    PORT-G  I/O 3               CN20 (JST HN 10P) EXTENSION INPUT 8bit. (?)
    PORT-AD AD-Stick inputs?    CN19 (JST NH 12P) A/D INPUT 8Ch.
    SERIAL COM                  CN18 (JST NH 6P) SERIAL COMMUNICATION
    ???                         CN22 (JST NH 3P) MIDI OUT
    ???                         CN27 (JST NH 3P) LIGHT GUN TRIGGER
    ???                         CN24 (JST NH 4P) AUDIO LINE OUTPUT

offsets:
    0x0001 PORT-A (P1)
    0x0003 PORT-B (P2)
    0x0005 PORT-C (SYSTEM)
    0x0007 PORT-D (OUTPUT)
    0x0009 PORT-E (P3)
    0x000b PORT-F (P4 / Extra 6B layout)
    0x000d PORT-G
    0x000f unused
    0x0011 PORT_DIRECTION (each bit configure above IO ports, 1 - input, 0 - output)
    ---x ---- joystick/mahjong panel select
    ---- x--- used in danchih (different mux scheme for the hanafuda panel?)
    0x0013 RS422 TXD1
    0x0015 RS422 TXD2 SERIAL COM Tx
    0x0017 RS422 RXD1
    0x0019 RS422 RXD2 SERIAL COM Rx
    0x001b RS422 FLAG
    0x001d MODE (bit 7 - set PORT-G to counter-mode, bits 0-5 - RS422 satellite mode and node#)  <Technical Bowling>
    0x001f PORT-AD (8ch, write: bits 0-2 - set channel, read: channel data with autoinc channel number)
*/

uint8_t stv_state::stv_ioga_r(offs_t offset)
{
	uint8_t res;

	res = 0xff;
	if(offset & 0x10 && !machine().side_effects_disabled())
		logerror("Reading from mirror %08x?\n",offset * 2 + 1);

	offset &= 0x0f; // mirror?

	switch((offset * 2) + 1)
	{
		case 0x0d:
			if (m_ioga_mode & 0x80) // PORT-G in counter mode
			{
				uint8_t const sel = (m_ioga_portg >> 1) & 0x03;
				res = ((m_ioga_counters[sel]->read() - m_ioga_count[sel]) >> ((~m_ioga_portg & 1) * 8) & 0xff);
				m_ioga_portg = (m_ioga_portg & 0xf8) | ((m_ioga_portg + 1) & 0x07); // counter# is auto-incremented on read
				break;
			}
			[[fallthrough]];
		case 0x01: // P1
		case 0x03: // P2
		case 0x05: // SYSTEM
		case 0x09: // P3
		case 0x0b: // P4
			res = m_ioga_ports[offset]->read();
			break;
		case 0x07: res = m_system_output; break; // port D, read-backs value written
		case 0x1b: res = 0; break; // Serial COM READ status
		case 0x1d: res = m_ioga_mode; break;
	}

	return res;
}

void stv_state::stv_ioga_w(offs_t offset, uint8_t data)
{
	if(offset & 0x10 && !machine().side_effects_disabled())
		logerror("Writing to mirror %08x %02x?\n",offset * 2 + 1,data);

	offset &= 0x0f; // mirror?

	switch(offset * 2 + 1)
	{
		case 0x07:
//          if (data != m_system_output)
//              logerror("OUT %02x\n", data);
			m_system_output = data;
			/*Why does the BIOS test these as ACTIVE HIGH? A program bug?*/
			machine().bookkeeping().coin_counter_w(0,~data & 0x01);
			machine().bookkeeping().coin_counter_w(1,~data & 0x02);
			machine().bookkeeping().coin_lockout_w(0,~data & 0x04);
			machine().bookkeeping().coin_lockout_w(1,~data & 0x08);
			break;
		case 0x09:
			m_billboard->write(data);
			break;
		case 0x0d:
			if(!BIT(data, 7)) // when bit 7==0 reset counters
			{
				for(unsigned i = 0; m_ioga_counters.size() > i; ++i)
					m_ioga_count[i] = m_ioga_counters[i]->read();
			}
			m_ioga_portg = data;
			break;
		case 0x1d:
			m_ioga_mode = data;
			break;
	}
}

uint8_t stv_state::critcrsh_ioga_r(offs_t offset)
{
	const char *const lgnames[] = { "LIGHTX", "LIGHTY" };

	uint8_t res = 0xff;

	switch(offset * 2 + 1)
	{
		case 0x01:
		case 0x03:
			res = ioport(lgnames[offset])->read();
			res = bitswap<8>(res, 2, 3, 0, 1, 6, 7, 5, 4) & 0xf3;
			res |= (ioport("PORTC")->read() & 0x10) ? 0x0 : 0x4; // x/y hit latch actually
			break;
		default:
			res = stv_ioga_r(offset);
			break;
	}

	return res;
}

void stv_state::critcrsh_ioga_w(offs_t offset, uint8_t data)
{
	static uint8_t const bcd2hex[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x49, 0x78, 0x00 }; // TODO: chip type unknown
	switch (offset * 2 + 1)
	{
		case 0x0b:
			m_cc_digits[0] = bcd2hex[(data & 0xf0) >> 4];
			m_cc_digits[1] = bcd2hex[data & 0x0f];
			break;
		default:
			stv_ioga_w(offset, data);
			break;
	}
}

uint8_t stv_state::magzun_ioga_r(offs_t offset)
{
	uint8_t res;

	res = 0xff;

	// 0x4a 0x40 0x47

	switch(offset * 2 + 1)
	{
		case 0x17:
			res = 0;
			break;
		case 0x19:
			res = 0;
			break;
		default: res = stv_ioga_r(offset); break;
	}

	return res;
}

void stv_state::magzun_ioga_w(offs_t offset, uint8_t data)
{
	switch(offset * 2 + 1)
	{
		case 0x13: m_serial_tx = (data << 8) | (m_serial_tx & 0xff); break;
		case 0x15: m_serial_tx = (data & 0xff) | (m_serial_tx & 0xff00); break;
		default: stv_ioga_w(offset,data); break;
	}
}

uint8_t stv_state::stvmp_ioga_r(offs_t offset)
{
	uint8_t res = 0xff;

	switch(offset * 2 + 1)
	{
		case 0x01:
		case 0x03:
			if(m_port_sel & 0x10) // joystick select <<< this is obviously wrong, this bit only selects PORTE direction
			{
				res = stv_ioga_r(offset);
			}
			else // mahjong panel select
			{
				for(unsigned i = 0; m_ioga_mahjong[i].size() > i; i++)
				{
					if(BIT(m_mux_data, i))
						res &= m_ioga_mahjong[offset][i]->read();
				}
			}
			break;
		default: res = stv_ioga_r(offset); break;
	}

	return res;
}

void stv_state::stvmp_ioga_w(offs_t offset, uint8_t data)
{
	switch(offset * 2 + 1)
	{
		case 0x09: m_mux_data = data ^ 0xff; break;
		case 0x11: m_port_sel = data; break;
		default:   stv_ioga_w(offset,data); break;
	}
}

void stv_state::hop_ioga_w(offs_t offset, uint8_t data)
{
	if ((offset * 2 + 1) == 7)
		m_hopper->motor_w(data & 0x80);
	stv_ioga_w(offset, data);
}


// ST-V hookup for 315-5881 encryption/compression chip

/*
 Known ST-V Games using this kind of protection

 Astra Superstars (text layer gfx transfer)
 Elandoree (gfx transfer of textures)
 Final Fight Revenge (boot vectors etc.)
 Radiant Silvergun (game start protection)
 Steep Slope Sliders (gfx transfer of character portraits)
 Tecmo World Cup '98 (Tecmo logo, player movement)

*/

/*************************************
*
* Common Handlers
*
*************************************/

uint32_t stv_state::common_prot_r(offs_t offset)
{
	uint32_t *ROM = (uint32_t *)memregion("abus")->base();

	if(m_abus_protenable & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			uint8_t* base;
			uint16_t res = m_cryptdevice->do_decrypt(base);
			uint16_t res2 = m_cryptdevice->do_decrypt(base);
			res = ((res & 0xff00) >> 8) | ((res & 0x00ff) << 8);
			res2 = ((res2 & 0xff00) >> 8) | ((res2 & 0x00ff) << 8);

			return res2 | (res << 16);
		}
		return m_a_bus[offset];
	}
	else
	{
		if(m_a_bus[offset] != 0) return m_a_bus[offset];
		else return ROM[(0x02fffff0/4)+offset];
	}
}


uint16_t stv_state::crypt_read_callback(uint32_t addr)
{
	uint16_t dat= m_maincpu->space().read_word((0x02000000+2*addr));
	return ((dat&0xff00)>>8)|((dat&0x00ff)<<8);
}

void stv_state::common_prot_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_a_bus[offset]);

	if (offset == 0)
	{
		COMBINE_DATA(&m_abus_protenable);
	}
	else if(offset == 2)
	{
		if (ACCESSING_BITS_16_31) m_cryptdevice->set_addr_low(data >> 16);
		if (ACCESSING_BITS_0_15) m_cryptdevice->set_addr_high(data&0xffff);

	}
	else if(offset == 3)
	{
		COMBINE_DATA(&m_abus_protkey);

		m_cryptdevice->set_subkey(m_abus_protkey>>16);
	}
}

void stv_state::install_common_protection()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4fffff0, 0x4ffffff, read32sm_delegate(*this, FUNC(stv_state::common_prot_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4fffff0, 0x4ffffff, write32s_delegate(*this, FUNC(stv_state::common_prot_w)));
}

void stv_state::stv_register_protection_savestates()
{
	save_item(NAME(m_a_bus));
}


/*

06013AE8: MOV.L   @($D4,PC),R5
06013AEA: MOV.L   @($D8,PC),R0
06013AEC: MOV.W   @R5,R5
06013AEE: MOV.L   @R0,R0
06013AF0: AND     R10,R5
06013AF2: TST     R0,R0
06013AF4: BTS     $06013B00
06013AF6: EXTS.W  R5,R5
06013B00: EXTS.W  R5,R5
06013B02: TST     R5,R5
06013B04: BF      $06013B0A
06013B06: TST     R4,R4
06013B08: BT      $06013AE8

   (loops for 375868 instructions)

*/

void stv_state::install_stvbios_speedups( void )
{
	// flushes 0 & 1 on both CPUs are for the BIOS speedups
	m_maincpu->sh2drc_add_pcflush(0x60154b2);
	m_maincpu->sh2drc_add_pcflush(0x6013aee);

	m_slave->sh2drc_add_pcflush(0x60154b2);
	m_slave->sh2drc_add_pcflush(0x6013aee);
}

void stv_state::init_stv()
{
	/* amount of time to boost interleave for on MINIT / SINIT, needed for communication to work */
	m_minit_boost = 400;
	m_sinit_boost = 400;
	m_minit_boost_timeslice = attotime::zero;
	m_sinit_boost_timeslice = attotime::zero;

//  m_scu_regs = std::make_unique<uint32_t[]>(0x100/4);
	m_backupram = std::make_unique<uint8_t[]>(0x8000);
	memset(m_backupram.get(), 0, sizeof(uint8_t) * 0x8000);

	install_stvbios_speedups();

	// do strict overwrite verification - maruchan and rsgun crash after coinup without this.
	// cottonbm needs strict PCREL
	// todo: test what games need this and don't turn it on for them...
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);
	m_slave->sh2drc_set_options(SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);

	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0007ffff, 1, &m_rom[0]);
	m_maincpu->sh2drc_add_fastram(0x00200000, 0x002fffff, 0, &m_workram_l[0]);
	m_maincpu->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_workram_h[0]);
	m_slave->sh2drc_add_fastram(0x00000000, 0x0007ffff, 1, &m_rom[0]);
	m_slave->sh2drc_add_fastram(0x00200000, 0x002fffff, 0, &m_workram_l[0]);
	m_slave->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_workram_h[0]);

	m_vdp2.pal = 0;
}

/*
    - if pc==604bf20 && 608e832 <- 1 (HWEF)
    - if pc==604bfbe && 608e832 <- 2 (HREF)
    - if pc==604c006 && 60ff3b7 <- 0x40 (they tries to read-back 0x40?)

    TODO: game doesn't work if not in debugger?
*/

uint32 stv_state::magzun_hef_hack_r()
{
	if(m_maincpu->pc()==0x604bf20) return 0x00000001; //HWEF

	if(m_maincpu->pc()==0x604bfbe) return 0x00000002; //HREF

	return m_workram_h[0x08e830/4];
}

uint32_t stv_state::magzun_rx_hack_r()
{
	if(m_maincpu->pc()==0x604c006) return 0x40;

	return m_workram_h[0x0ff3b4/4];
}

void stv_state::init_magzun()
{
	m_maincpu->sh2drc_add_pcflush(0x604bf20);
	m_maincpu->sh2drc_add_pcflush(0x604bfbe);
	m_maincpu->sh2drc_add_pcflush(0x604c006);

	init_stv();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x608e830, 0x608e833, read32smo_delegate(*this, FUNC(stv_state::magzun_hef_hack_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x60ff3b4, 0x60ff3b7, read32smo_delegate(*this, FUNC(stv_state::magzun_rx_hack_r)));

	/* Program ROM patches, don't understand how to avoid these two checks ... */
	{
		uint32_t *ROM = (uint32_t *)memregion("cart")->base();

		ROM[0x90054/4] = 0x00e00001; // END error

		ROM[0x34f4/4] = 0x00000009; // Time Out sub-routine
	}
}


void stv_state::init_shienryu()
{
	// master
	m_maincpu->sh2drc_add_pcflush(0x60041c6);
	// slave
	m_slave->sh2drc_add_pcflush(0x600440e);

	init_stv();
}

void stv_state::init_prikura()
{
/*
 06018640: MOV.B   @R14,R0  // 60b9228
 06018642: TST     R0,R0
 06018644: BF      $06018640

    (loops for 263473 instructions)
*/

	// master
	m_maincpu->sh2drc_add_pcflush(0x6018640);
	// slave
	m_slave->sh2drc_add_pcflush(0x6018c6e);

	init_stv();

	m_minit_boost = m_sinit_boost = 0;
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_hanagumi()
{
/*
    06013E1E: NOP
    0601015E: MOV.L   @($6C,PC),R3
    06010160: MOV.L   @R3,R0  (6094188)
    06010162: TST     R0,R0
    06010164: BT      $0601015A
    0601015A: JSR     R14
    0601015C: NOP
    06013E20: MOV.L   @($34,PC),R3
    06013E22: MOV.B   @($00,R3),R0
    06013E24: TST     R0,R0
    06013E26: BT      $06013E1C
    06013E1C: RTS
    06013E1E: NOP

   (loops for 288688 instructions)
*/
	m_maincpu->sh2drc_add_pcflush(0x6010160);

	init_stv();
}




/* these idle loops might change if the interrupts change / are fixed because i don't really think these are vblank waits... */

/* puyosun

CPU0: Aids Screen

06021CF0: MOV.B   @($13,GBR),R0 (60ffc13)
06021CF2: CMP/PZ  R0
06021CF4: BT      $06021CF0
   (loops for xxx instructions)

   this is still very slow .. but i don't think it can be sped up further


*/

void stv_state::init_puyosun()
{
	m_maincpu->sh2drc_add_pcflush(0x6021cf0);

	m_slave->sh2drc_add_pcflush(0x60236fe);

	init_stv();

	m_minit_boost = m_sinit_boost = 0;
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

/* mausuke

CPU0 Data East Logo:
060461A0: MOV.B   @($13,GBR),R0  (60ffc13)
060461A2: CMP/PZ  R0
060461A4: BT      $060461A0
   (loops for 232602 instructions)

*/

void stv_state::init_mausuke()
{
	m_maincpu->sh2drc_add_pcflush(0x60461A0);

	init_stv();

	m_minit_boost = m_sinit_boost = 0;
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_cottonbm()
{
//  m_maincpu->sh2drc_add_pcflush(0x6030ee2);
//  m_slave->sh2drc_add_pcflush(0x6032b52);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(10);
}

void stv_state::init_cotton2()
{
	m_maincpu->sh2drc_add_pcflush(0x6031c7a);
	m_slave->sh2drc_add_pcflush(0x60338ea);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_dnmtdeka()
{
	// install all 3 speedups on both master and slave
	m_maincpu->sh2drc_add_pcflush(0x6027c90);
	m_maincpu->sh2drc_add_pcflush(0xd04);
	m_maincpu->sh2drc_add_pcflush(0x60051f2);

	m_slave->sh2drc_add_pcflush(0x6027c90);
	m_slave->sh2drc_add_pcflush(0xd04);
	m_slave->sh2drc_add_pcflush(0x60051f2);

	init_stv();
}

void stv_state::init_diehard()
{
	// install all 3 speedups on both master and slave
	m_maincpu->sh2drc_add_pcflush(0x6027c98);
	m_maincpu->sh2drc_add_pcflush(0xd04);
	m_maincpu->sh2drc_add_pcflush(0x60051f2);

	m_slave->sh2drc_add_pcflush(0x6027c98);
	m_slave->sh2drc_add_pcflush(0xd04);
	m_slave->sh2drc_add_pcflush(0x60051f2);

	init_stv();
}

void stv_state::init_fhboxers()
{
	m_maincpu->sh2drc_add_pcflush(0x60041c2);
	m_maincpu->sh2drc_add_pcflush(0x600bb0a);
	m_maincpu->sh2drc_add_pcflush(0x600b31e);

	init_stv();

//  m_instadma_hack = 1;
}

void stv_state::init_groovef()
{
	m_maincpu->sh2drc_add_pcflush(0x6005e7c);
	m_maincpu->sh2drc_add_pcflush(0x6005e86);
	m_maincpu->sh2drc_add_pcflush(0x60a4970);

	m_slave->sh2drc_add_pcflush(0x60060c2);

	init_stv();

	m_minit_boost = m_sinit_boost = 0;
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_danchih()
{
	m_maincpu->sh2drc_add_pcflush(0x6028b28);
	m_maincpu->sh2drc_add_pcflush(0x6028c8e);
	m_slave->sh2drc_add_pcflush(0x602ae26);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(5);
}

void stv_state::init_danchiq()
{
	m_maincpu->sh2drc_add_pcflush(0x6028b28);
	m_maincpu->sh2drc_add_pcflush(0x6028c8e);
	m_slave->sh2drc_add_pcflush(0x602ae26);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(5);
}

void stv_state::init_astrass()
{
	m_maincpu->sh2drc_add_pcflush(0x60011ba);
	m_maincpu->sh2drc_add_pcflush(0x605b9da);

	install_common_protection();

	init_stv();
}

void stv_state::init_thunt()
{
	m_maincpu->sh2drc_add_pcflush(0x602A024);
	m_maincpu->sh2drc_add_pcflush(0x6013EEA);
	m_slave->sh2drc_add_pcflush(0x602AAF8);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(1);
}

void stv_state::init_sandor()
{
	m_maincpu->sh2drc_add_pcflush(0x602a0f8);
	m_maincpu->sh2drc_add_pcflush(0x6013fbe);
	m_slave->sh2drc_add_pcflush(0x602abcc);

	init_stv();
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(1);
}

void stv_state::init_grdforce()
{
	m_maincpu->sh2drc_add_pcflush(0x6041e32);
	m_slave->sh2drc_add_pcflush(0x6043aa2);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_batmanfr()
{
	m_maincpu->sh2drc_add_pcflush(0x60121c0);
	m_slave->sh2drc_add_pcflush(0x60125bc);

	init_stv();

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x04800000, 0x04800003, write32s_delegate(*this, FUNC(stv_state::batmanfr_sound_comms_w)));
	m_slave->space(AS_PROGRAM).install_write_handler(0x04800000, 0x04800003, write32s_delegate(*this, FUNC(stv_state::batmanfr_sound_comms_w)));

	m_minit_boost = m_sinit_boost = 0;
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_colmns97()
{
	m_slave->sh2drc_add_pcflush(0x60298a2);

	init_stv();

	m_minit_boost = m_sinit_boost = 0;
}

void stv_state::init_winterht()
{
	m_maincpu->sh2drc_add_pcflush(0x6098aea);
	m_slave->sh2drc_add_pcflush(0x609ae4e);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(2);
}

void stv_state::init_seabass()
{
	m_maincpu->sh2drc_add_pcflush(0x602cbfa);
	m_slave->sh2drc_add_pcflush(0x60321ee);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(5);
}

void stv_state::init_vfremix()
{
	m_maincpu->sh2drc_add_pcflush(0x602c30c);
	m_slave->sh2drc_add_pcflush(0x604c332);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(20);
}

void stv_state::init_sss()
{
	m_maincpu->sh2drc_add_pcflush(0x6026398);
	m_slave->sh2drc_add_pcflush(0x6028cd6);

	install_common_protection();

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_othellos()
{
	m_maincpu->sh2drc_add_pcflush(0x602bcbe);
	m_slave->sh2drc_add_pcflush(0x602d92e);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_sasissu()
{
	m_slave->sh2drc_add_pcflush(0x60710be);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(2);
}

void stv_state::init_gaxeduel()
{
//  m_maincpu->sh2drc_add_pcflush(0x6012ee4);

	init_stv();
}

void stv_state::init_suikoenb()
{
	m_maincpu->sh2drc_add_pcflush(0x6013f7a);

	init_stv();
}


void stv_state::init_sokyugrt()
{
	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_znpwfv()
{
	m_maincpu->sh2drc_add_pcflush(0x6012ec2);
	m_slave->sh2drc_add_pcflush(0x60175a6);

	init_stv();
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_nsec(500);
}

void stv_state::init_twcup98()
{
	m_maincpu->sh2drc_add_pcflush(0x605edde);
	m_slave->sh2drc_add_pcflush(0x6062bca);

	init_stv();
	install_common_protection();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(5);
}

void stv_state::init_smleague()
{
	m_maincpu->sh2drc_add_pcflush(0x6063bf4);
	m_slave->sh2drc_add_pcflush(0x6062bca);

	init_stv();

	/* tight sync to avoid dead locks */
	m_minit_boost = m_sinit_boost = 5000;
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(5000);
}

void stv_state::init_finlarch()
{
	m_maincpu->sh2drc_add_pcflush(0x6064d60);

	init_stv();

	/* tight sync to avoid dead locks */
	m_minit_boost = m_sinit_boost = 5000;
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(5000);
}

void stv_state::init_maruchan()
{
	m_maincpu->sh2drc_add_pcflush(0x601ba46);
	m_slave->sh2drc_add_pcflush(0x601ba46);

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(50);
}

void stv_state::init_pblbeach()
{
	m_maincpu->sh2drc_add_pcflush(0x605eb78);

	init_stv();
}

void stv_state::init_shanhigw()
{
	m_maincpu->sh2drc_add_pcflush(0x6020c5c);

	init_stv();
}

void stv_state::init_elandore()
{
	m_maincpu->sh2drc_add_pcflush(0x604eac0);
	m_slave->sh2drc_add_pcflush(0x605340a);

	install_common_protection();

	init_stv();
	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(0);
}

void stv_state::init_rsgun()
{
	m_maincpu->sh2drc_add_pcflush(0x6034d04);
	m_slave->sh2drc_add_pcflush(0x6036152);

	install_common_protection();

	init_stv();

	m_minit_boost_timeslice = m_sinit_boost_timeslice = attotime::from_usec(20);
}

void stv_state::init_ffreveng()
{
	install_common_protection();
	init_stv();
}


uint32_t stv_state::decathlt_prot_r(offs_t offset, uint32_t mem_mask)
{
	// needs to be a way to indicate if device is enabled and fall through to cartridge data if not?
	if (m_newprotection_element)
	{
		m_newprotection_element = false;
		m_5838crypt->debug_helper(m_protbankval);
	}

	uint32 ret = 0;
	if (mem_mask & 0xffff0000) ret |= (m_5838crypt->data_r()<<16);
	if (mem_mask & 0x0000ffff) ret |= m_5838crypt->data_r();
	return ret;
}

void stv_state::decathlt_prot_srcaddr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int offs = offset * 4;

	m_protbankval = (offs & 0x1800000)>>23;
	m_protbank->set_entry(m_protbankval); // if the protection device is accessed at this address data is fetched from 0x02000000
	m_newprotection_element = true;

	if ((offs & 0x7fffff) == 0x7FFFF0)
	{
		m_5838crypt->srcaddr_w(offs, data, mem_mask);
	}
	else if ((offs & 0x7fffff) == 0x7FFFF4)
	{
		m_5838crypt->data_w(offs, data, mem_mask);
	}
}

void stv_state::init_decathlt()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2000000, 0x37fffff, write32s_delegate(*this, FUNC(stv_state::decathlt_prot_srcaddr_w))); // set compressed data source address, write data

	// really needs installing over the whole range, with fallbacks to read rom if device is disabled or isn't accessed on given address
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x27ffff8, 0x27ffffb, read32s_delegate(*this, FUNC(stv_state::decathlt_prot_r))); // read decompressed data
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2fffff8, 0x2fffffb, read32s_delegate(*this, FUNC(stv_state::decathlt_prot_r))); //  ^
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x37ffff8, 0x37ffffb, read32s_delegate(*this, FUNC(stv_state::decathlt_prot_r))); //  ^

	m_protbank->configure_entry(0, memregion("cart")->base() + 0x0000000);
	m_protbank->configure_entry(1, memregion("cart")->base() + 0x0800000);
	m_protbank->configure_entry(2, memregion("cart")->base() + 0x1000000);
	//m_protbank->configure_entry(3, memregion("cart")->base() + 0x1800000);

	m_protbank->set_entry(0);

	m_newprotection_element = false;

	init_stv();
}

void stv_state::init_decathlt_nokey()
{
	init_decathlt();
	m_5838crypt->set_hack_mode(sega_315_5838_comp_device::HACK_MODE_NO_KEY);
}

void stv_state::init_nameclv3()
{
	m_maincpu->sh2drc_add_pcflush(0x601eb4c);
	m_slave->sh2drc_add_pcflush(0x602b80e);

	init_stv();
}

void stv_state::init_stv_us()
{
	init_stv();
	m_smpc_hle->set_region_code(4); // 4 - USA, configured via jumpers, should be added to input ports as DIPSW ?
}

void stv_state::stv_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().mirror(0x20000000).region("bios", 0); // bios
	map(0x00100000, 0x0010007f).rw(m_smpc_hle, FUNC(smpc_hle_device::read), FUNC(smpc_hle_device::write));
	map(0x00180000, 0x0018ffff).rw(FUNC(stv_state::saturn_backupram_r), FUNC(stv_state::saturn_backupram_w)).share("share1");
	map(0x00200000, 0x002fffff).ram().mirror(0x20100000).share("workram_l");
	map(0x00400000, 0x0040003f).rw(FUNC(stv_state::stv_ioga_r), FUNC(stv_state::stv_ioga_w)).umask32(0x00ff00ff);
	map(0x01000000, 0x017fffff).w(FUNC(stv_state::minit_w));
	map(0x01800000, 0x01ffffff).w(FUNC(stv_state::sinit_w));
	map(0x02000000, 0x04ffffff).rom().mirror(0x20000000).region("abus", 0); // cartridge
	/* Sound */
	map(0x05a00000, 0x05afffff).rw(FUNC(stv_state::saturn_soundram_r), FUNC(stv_state::saturn_soundram_w));
	map(0x05b00000, 0x05b00fff).rw("scsp", FUNC(scsp_device::read), FUNC(scsp_device::write));
	/* VDP1 */
	map(0x05c00000, 0x05c7ffff).rw(FUNC(stv_state::saturn_vdp1_vram_r), FUNC(stv_state::saturn_vdp1_vram_w));
	map(0x05c80000, 0x05cbffff).rw(FUNC(stv_state::saturn_vdp1_framebuffer0_r), FUNC(stv_state::saturn_vdp1_framebuffer0_w));
	map(0x05d00000, 0x05d0001f).rw(FUNC(stv_state::saturn_vdp1_regs_r), FUNC(stv_state::saturn_vdp1_regs_w));
	map(0x05e00000, 0x05e7ffff).mirror(0x80000).rw(FUNC(stv_state::saturn_vdp2_vram_r), FUNC(stv_state::saturn_vdp2_vram_w));
	map(0x05f00000, 0x05f7ffff).rw(FUNC(stv_state::saturn_vdp2_cram_r), FUNC(stv_state::saturn_vdp2_cram_w));
	map(0x05f80000, 0x05fbffff).rw(FUNC(stv_state::saturn_vdp2_regs_r), FUNC(stv_state::saturn_vdp2_regs_w));
	map(0x05fe0000, 0x05fe00cf).m(m_scu, FUNC(sega_scu_device::regs_map)); //rw(FUNC(stv_state::saturn_scu_r), FUNC(stv_state::saturn_scu_w));
	map(0x06000000, 0x060fffff).ram().mirror(0x21f00000).share("workram_h");
	map(0x60000000, 0x600003ff).nopw();
	map(0xc0000000, 0xc00007ff).ram(); // cache RAM
}

void stv_state::critcrsh_mem(address_map &map)
{
	stv_mem(map);
	map(0x00400000, 0x0040003f).rw(FUNC(stv_state::critcrsh_ioga_r), FUNC(stv_state::critcrsh_ioga_w)).umask32(0x00ff00ff);
}

void stv_state::magzun_mem(address_map &map)
{
	stv_mem(map);
	map(0x00400000, 0x0040003f).rw(FUNC(stv_state::magzun_ioga_r), FUNC(stv_state::magzun_ioga_w)).umask32(0x00ff00ff);
}

void stv_state::stvmp_mem(address_map &map)
{
	stv_mem(map);
	map(0x00400000, 0x0040003f).rw(FUNC(stv_state::stvmp_ioga_r), FUNC(stv_state::stvmp_ioga_w)).umask32(0x00ff00ff);
}

void stv_state::hopper_mem(address_map &map)
{
	stv_mem(map);
	map(0x00400000, 0x0040003f).rw(FUNC(stv_state::stv_ioga_r), FUNC(stv_state::hop_ioga_w)).umask32(0x00ff00ff);
}

void stv_state::stvcd_mem(address_map &map)
{
	stv_mem(map);
	map(0x05800000, 0x0589ffff).rw("stvcd", FUNC(stvcd_device::stvcd_r), FUNC(stvcd_device::stvcd_w));
}

void stv_state::sound_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("sound_ram");
	map(0x100000, 0x100fff).rw("scsp", FUNC(scsp_device::read), FUNC(scsp_device::write));
}

void stv_state::scsp_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("sound_ram");
}



/********************************************
 *
 * SMPC outputs
 *
 *******************************************/

void stv_state::stv_select_game(int gameno)
{
	if (m_prev_gamebank_select != gameno)
	{
		if (m_cart_reg[gameno] && m_cart_reg[gameno]->base())
			memcpy(memregion("abus")->base(), m_cart_reg[gameno]->base(), 0x3000000);
		else
			memset(memregion("abus")->base(), 0x00, 0x3000000); // TODO: 1-filled?

		m_prev_gamebank_select = gameno;
	}
}

uint8_t stv_state::pdr1_input_r()
{
	return (m_pdr[0]->read() & 0x40) | 0x3f;
}


uint8_t stv_state::pdr2_input_r()
{
	return (m_pdr[1]->read() & ~0x19) | 0x18 | (m_eeprom->do_read() << 0);
}


void stv_state::pdr1_output_w(uint8_t data)
{
	m_eeprom->clk_write((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write((data >> 4) & 1);
	m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);

	stv_select_game(data & 3);
}

void stv_state::pdr2_output_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
	m_en_68k = ((data & 0x10) >> 4) ^ 1;

	if(data & 8)
		logerror("PDR2: data 0x8 active!\n");
}


void stv_state::stv(machine_config &config)
{
	/* basic machine hardware */
	SH2(config, m_maincpu, MASTER_CLOCK_352/2); // 28.6364 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &stv_state::stv_mem);
	m_maincpu->set_is_slave(0);
	TIMER(config, "scantimer").configure_scanline(FUNC(stv_state::saturn_scanline), "screen", 0, 1);

	SH2(config, m_slave, MASTER_CLOCK_352/2); // 28.6364 MHz
	m_slave->set_addrmap(AS_PROGRAM, &stv_state::stv_mem);
	m_slave->set_is_slave(1);
	TIMER(config, "slave_scantimer").configure_scanline(FUNC(stv_state::saturn_slave_scanline), "screen", 0, 1);

	M68000(config, m_audiocpu, 11289600); //11.2896 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &stv_state::sound_mem);

	SEGA_SCU(config, m_scu, 0);
	m_scu->set_hostcpu(m_maincpu);

	SMPC_HLE(config, m_smpc_hle, XTAL(4'000'000));
	m_smpc_hle->set_screen_tag("screen");
	m_smpc_hle->set_region_code(1); // 1 - Japan, configured via jumpers, should be added to input ports as DIPSW ?
	m_smpc_hle->pdr1_in_handler().set(FUNC(stv_state::pdr1_input_r));
	m_smpc_hle->pdr2_in_handler().set(FUNC(stv_state::pdr2_input_r));
	m_smpc_hle->pdr1_out_handler().set(FUNC(stv_state::pdr1_output_w));
	m_smpc_hle->pdr2_out_handler().set(FUNC(stv_state::pdr2_output_w));
	m_smpc_hle->master_reset_handler().set(FUNC(saturn_state::master_sh2_reset_w));
	m_smpc_hle->master_nmi_handler().set(FUNC(saturn_state::master_sh2_nmi_w));
	m_smpc_hle->slave_reset_handler().set(FUNC(saturn_state::slave_sh2_reset_w));
//  m_smpc_hle->sound_reset_handler().set(FUNC(saturn_state::sound_68k_reset_w)); // ST-V games controls reset line via PDR2
	m_smpc_hle->system_reset_handler().set(FUNC(saturn_state::system_reset_w));
	m_smpc_hle->system_halt_handler().set(FUNC(saturn_state::system_halt_w));
	m_smpc_hle->dot_select_handler().set(FUNC(saturn_state::dot_select_w));
	m_smpc_hle->interrupt_handler().set(m_scu, FUNC(sega_scu_device::smpc_irq_w));

	EEPROM_93C46_16BIT(config, "eeprom"); /* Actually AK93C45F */

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	m_screen->set_raw(MASTER_CLOCK_320/8, 427, 0, 352, 263, 0, 224);
	m_screen->set_screen_update(FUNC(stv_state::screen_update_stv_vdp2));
	PALETTE(config, m_palette).set_entries(2048+(2048*2)); //standard palette + extra memory for rgb brightness.

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_stv);

	MCFG_VIDEO_START_OVERRIDE(stv_state,stv_vdp2)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SCSP(config, m_scsp, 22579200); // TODO : Unknown clock, divider
	m_scsp->set_addrmap(0, &stv_state::scsp_mem);
	m_scsp->irq_cb().set(FUNC(saturn_state::scsp_irq));
	m_scsp->main_irq_cb().set(m_scu, FUNC(sega_scu_device::sound_req_w));
	m_scsp->add_route(0, "lspeaker", 1.0);
	m_scsp->add_route(1, "rspeaker", 1.0);

	SEGA_BILLBOARD(config, m_billboard, 0);

	config.set_default_layout(layout_segabill);
}

void stv_state::stv_5881(machine_config &config)
{
	stv(config);
	SEGA315_5881_CRYPT(config, m_cryptdevice, 0);
	m_cryptdevice->set_read_cb(FUNC(stv_state::crypt_read_callback));
}

void stv_state::critcrsh(machine_config &config)
{
	stv(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &stv_state::critcrsh_mem);
	m_slave->set_addrmap(AS_PROGRAM, &stv_state::critcrsh_mem);
}

void stv_state::magzun(machine_config &config)
{
	stv(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &stv_state::magzun_mem);
	m_slave->set_addrmap(AS_PROGRAM, &stv_state::magzun_mem);
}

void stv_state::stvmp(machine_config &config)
{
	stv(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &stv_state::stvmp_mem);
	m_slave->set_addrmap(AS_PROGRAM, &stv_state::stvmp_mem);
}

void stv_state::stvcd(machine_config &config)
{
	stv(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &stv_state::stvcd_mem);
	m_slave->set_addrmap(AS_PROGRAM, &stv_state::stvcd_mem);

	stvcd_device &stvcd(STVCD(config, "stvcd", 0));
	stvcd.add_route(0, "scsp", 1.0, 0);
	stvcd.add_route(1, "scsp", 1.0, 1);
}


void stv_state::sega5838_map(address_map &map)
{
	map(0x000000, 0x7fffff).bankr("protbank");
}

void stv_state::stv_5838(machine_config &config)
{
	stv(config);

	SEGA315_5838_COMP(config, m_5838crypt, 0);
	m_5838crypt->set_addrmap(0, &stv_state::sega5838_map);
}


/*
    Batman Forever has an extra ADSP, used for sound.
    Similar if not the same as Magic the Gathering, probably needs merging.
*/

void stv_state::batmanfr_sound_comms_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// FIXME
	if(ACCESSING_BITS_16_31)
		m_rax->data_w(data >> 16);
	if(ACCESSING_BITS_0_15)
		logerror("Warning: write %04x & %08x to lo-word sound communication area\n",data,mem_mask);
}


void stv_state::batmanfr(machine_config &config)
{
	stv(config);
	ACCLAIM_RAX(config, "rax", 0);
}

void stv_state::shienryu(machine_config &config)
{
	stv(config);
	config.set_default_layout(layout_segabillv);
}

#define STV_CARTSLOT_ADD(_tag, _load) \
	GENERIC_CARTSLOT(config, _tag, generic_plain_slot, "stv_cart").set_device_load(FUNC(stv_state::_load));

void stv_state::stv_cartslot(machine_config &config)
{
	STV_CARTSLOT_ADD("stv_slot1", stv_cart1);
	STV_CARTSLOT_ADD("stv_slot2", stv_cart2);
	STV_CARTSLOT_ADD("stv_slot3", stv_cart3);
	STV_CARTSLOT_ADD("stv_slot4", stv_cart4);

	SOFTWARE_LIST(config, "cart_list").set_original("stv");
}

void stv_state::stv_slot(machine_config &config)
{
	stv(config);
	stv_cartslot(config);
}

void stv_state::hopper(machine_config &config)
{
	stv(config);
	HOPPER(config, m_hopper, attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);

	m_maincpu->set_addrmap(AS_PROGRAM, &stv_state::hopper_mem);
	m_slave->set_addrmap(AS_PROGRAM, &stv_state::hopper_mem);
}

void stv_state::machine_reset()
{
	m_scsp_last_line = 0;

	// don't let the slave cpu and the 68k go anywhere
	m_slave->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	std::string region_tag;
	if (m_cart1)
		m_cart_reg[0] = memregion(region_tag.assign(m_cart1->tag()).append(GENERIC_ROM_REGION_TAG));
	else
		m_cart_reg[0] = memregion("cart");
	if (m_cart2)
		m_cart_reg[1] = memregion(region_tag.assign(m_cart2->tag()).append(GENERIC_ROM_REGION_TAG));
	else
		m_cart_reg[1] = nullptr;
	if (m_cart3)
		m_cart_reg[2] = memregion(region_tag.assign(m_cart3->tag()).append(GENERIC_ROM_REGION_TAG));
	else
		m_cart_reg[2] = nullptr;
	if (m_cart4)
		m_cart_reg[3] = memregion(region_tag.assign(m_cart4->tag()).append(GENERIC_ROM_REGION_TAG));
	else
		m_cart_reg[3] = nullptr;


	m_en_68k = 0;

	m_port_sel = m_mux_data = 0;

	m_maincpu->set_unscaled_clock(MASTER_CLOCK_320/2);
	m_slave->set_unscaled_clock(MASTER_CLOCK_320/2);

	m_prev_gamebank_select = 0xff;

	m_vdp2.old_crmd = -1;
	m_vdp2.old_tvmd = -1;
}

image_init_result stv_state::load_cart(device_image_interface &image, generic_slot_device *slot)
{
	uint8_t *ROM;
	uint32_t size = slot->common_get_size("rom");

	if (!image.loaded_through_softlist())
		return image_init_result::FAIL;

	slot->rom_alloc(size, GENERIC_ROM32_WIDTH, ENDIANNESS_BIG);
	ROM = slot->get_rom_base();
	memcpy(ROM, image.get_software_region("rom"), size);

	/* fix endianess */
	{
		uint8_t j[4];

		for (int i = 0; i < size; i += 4)
		{
			j[0] = ROM[i + 0];
			j[1] = ROM[i + 1];
			j[2] = ROM[i + 2];
			j[3] = ROM[i + 3];
			ROM[i + 0] = j[3];
			ROM[i + 1] = j[2];
			ROM[i + 2] = j[1];
			ROM[i + 3] = j[0];
		}
	}

	return image_init_result::PASS;
}


void stv_state::machine_start()
{
	m_cc_digits.resolve();

	// save states
//  save_pointer(NAME(m_scu_regs), 0x100/4);
	save_item(NAME(m_en_68k));
	save_item(NAME(m_prev_gamebank_select));
	save_item(NAME(m_port_sel));
	save_item(NAME(m_mux_data));
	save_item(NAME(m_scsp_last_line));
	save_item(NAME(m_vdp2.odd));

	stv_register_protection_savestates();

	m_audiocpu->set_reset_callback(*this, FUNC(stv_state::m68k_reset_callback));
}


#define STV_PLAYER_INPUTS(_n_, _b1_, _b2_, _b3_,_b4_)                       \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_##_b1_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_##_b2_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_##_b3_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_##_b4_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(_n_)     \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(_n_)       \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_)    \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(_n_)

static INPUT_PORTS_START( stv )
	PORT_START("PDR1")
	PORT_DIPNAME( 0x40, 0x40, "PDR1" ) // P1 Gun Trigger
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PDR2")
	PORT_DIPNAME( 0x01, 0x01, "PDR2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) // test mode mirror
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) // service button mirror
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) // P2 Gun Trigger
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PORTA")
	STV_PLAYER_INPUTS(1, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("PORTB")
	STV_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1P Push Switch (Multi Cart Select)") PORT_CODE(KEYCODE_7) // selects game in multi-cart mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2P Push Switch (Pause)") PORT_CODE(KEYCODE_8) // for games that supports it, most of them needs to be enabled in game assignment first

	PORT_START("PORTE")
	STV_PLAYER_INPUTS(3, BUTTON1, BUTTON2, BUTTON3, START)

	PORT_START("PORTF")
	STV_PLAYER_INPUTS(4, BUTTON1, BUTTON2, BUTTON3, START)

	PORT_START("PORTG")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PORTG.0")
	PORT_BIT( 0x0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PORTG.1")
	PORT_BIT( 0x0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PORTG.2")
	PORT_BIT( 0x0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PORTG.3")
	PORT_BIT( 0x0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( batmanfr )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Jump")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Punch")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Kick")

	PORT_MODIFY("PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Jump")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Punch")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Kick")

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( stv6b )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Extra button layout, used by Power Instinct 3, Suikoenbu, Elan Doree, Golden Axe Duel & Astra SuperStars */
	PORT_MODIFY("PORTF")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( critcrsh )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hammer Hit")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX") /* mask default type                     sens delta min max */
	PORT_BIT( 0x3f, 0x00, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,0x3f) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("LIGHTY")
	PORT_BIT( 0x3f, 0x02, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x2,0x2e) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)
INPUT_PORTS_END

/* Same as the regular one, but with an additional & optional mahjong panel */
static INPUT_PORTS_START( stvmp )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Mahjong panel/player 1 side */
	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L )

	PORT_START("P1_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Mahjong panel/player 2 side */
	PORT_START("P2_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)

	PORT_START("P2_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)

	PORT_START("P2_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)

	PORT_START("P2_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)

	PORT_START("P2_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/* Micronet layout, routes joystick port to the mux! */
static INPUT_PORTS_START( myfairld )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BET )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) /* F/F is there, but these two games are single player so it isn't connected */

	PORT_START("P2_KEY0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO_TYPE")
	PORT_CONFNAME( 0x01, 0x01, "I/O Device type" )
	PORT_CONFSETTING(    0x00, "Mahjong Panel" )
	PORT_CONFSETTING(    0x01, "Joystick" )
INPUT_PORTS_END

/* Micronet layout, routes joystick port to the mux! */
static INPUT_PORTS_START( vmahjong )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L )

	PORT_START("P1_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) /* F/F is there, but these two games are single player so it isn't connected */

	PORT_START("P2_KEY0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( patocar )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )   // hopper ?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3 )   // ??
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 )   // Door switch ?
	PORT_BIT( 0x0c, IP_ACTIVE_LOW,  IPT_BUTTON5 )   // ??
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN3 ) PORT_NAME("Medal")
	PORT_BIT( 0xd0, IP_ACTIVE_LOW,  IPT_UNUSED )

	// TODO: sense/delta values seems wrong
	PORT_MODIFY("PORTG.0")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(500) PORT_KEYDELTA(100) PORT_PLAYER(1)
	PORT_MODIFY("PORTG.1")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(500) PORT_KEYDELTA(100) PORT_PLAYER(1)

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1)    PORT_NAME("10Yen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2)    PORT_NAME("100Yen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )  PORT_NAME("Select Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Power Button")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( micrombc )
	PORT_INCLUDE( patocar )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Power Button")
	PORT_BIT( 0xdc, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW,  IPT_BUTTON5 ) PORT_NAME("Magnet Sensor")

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Shoot Button")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTG.0")
	PORT_BIT( 0x0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_MODIFY("PORTG.1")
	PORT_BIT( 0x0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

/* BIOS rev info found at 0x800 (byte swapped):

epr-17740  - Japan   STVB1.10J0950131  95/01/31 v1.10  - Found on a early board dated 02/1995
epr-17740a - Japan   STVB1.11J0950220  95/02/20 v1.11
epr-17741a - USA     STVB1.11U0950220  95/02/20 v1.11
epr-17951a - Japan   STVB1.13J0950425  95/04/25 v1.13 - There's also a 100% identical mask ROM version: mpr-17951a
epr-17952a - USA     STVB1.13U0950425  95/04/25 v1.13
epr-17953a - Taiwan  STVB1.13T0950425  95/04/25 v1.13
epr-17954a - Europe  STVB1.13E0950425  95/04/25 v1.13
epr-19854  - Taiwan  STVB1.14T0970515  97/05/15 v1.14

epr-18343  - ?????   STVB1.30S0950727  95/07/27 v1.30 - Special version used on Sports Fishing 2 (CD based)
epr-19730  - Japan   PCD11.13J0970217  97/02/17 v1.13 - Enhanced version for CD based units?
epr-20091  - Japan   PCS11.13J0970821  97/08/21 v1.13 - Enhanced version for ???

stv110.bin  - Debug  STVB1.10D0950113  95/01/13 v1.10
stv1061.bin - ST-V Dev Bios (1.061) - Sega 1994, Noted "ST-V Ver 1.061 94/11/25" on EPROM sticker, Coming from a S-TV SG5001A dev board

Regions found listed in the BIOS:
    Japan
    Taiwan and Philipines
    USA and Canada
    Brazil
    Korea
    Asia PAL area
    Europe
    Latin America

ROM_SYSTEM_BIOS( x, "saturn",      "Saturn bios" ) \
ROM_LOAD16_WORD_SWAP_BIOS( x, "saturn.bin", 0x000000, 0x080000, CRC(653ff2d8) SHA1(20994ae7ee177ddaf3a430b010c7620dca000fb4) ) - Saturn Eu Bios

*/

#define STV_BIOS \
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* SH2 code */ \
	ROM_SYSTEM_BIOS( 0,  "jp",    "EPR-20091 (Japan 97/08/21)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "epr-20091.ic8",   0x000000, 0x080000, CRC(59ed40f4) SHA1(eff0f54c70bce05ff3a289bf30b1027e1c8cd117) ) \
	ROM_SYSTEM_BIOS( 1,  "jp1",   "EPR-19730 (Japan 97/02/17)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1,  "epr-19730.ic8",   0x000000, 0x080000, CRC(d0e0889d) SHA1(fae53107c894e0c41c49e191dbe706c9cd6e50bd) ) \
	ROM_SYSTEM_BIOS( 2,  "jp2",   "EPR-17951A (Japan 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2,  "epr-17951a.ic8",  0x000000, 0x080000, CRC(2672f9d8) SHA1(63cf4a6432f6c87952f9cf3ab0f977aed2367303) ) \
	ROM_SYSTEM_BIOS( 3,  "jp3",   "EPR-17740A (Japan 95/02/20)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3,  "epr-17740a.ic8",  0x000000, 0x080000, CRC(3e23c81f) SHA1(f9b282fd27693e9891843597b2e1823da3d23c7b) ) \
	ROM_SYSTEM_BIOS( 4,  "jp4",   "EPR-17740 (Japan 95/01/31)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4,  "epr-17740.ic8",   0x000000, 0x080000, CRC(5c5aa63d) SHA1(06860d96923b81afbc21e0ad32ee19487d8ff6e7) ) \
	ROM_SYSTEM_BIOS( 5,  "euro",  "EPR-17954A (Europe 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5,  "epr-17954a.ic8",  0x000000, 0x080000, CRC(f7722da3) SHA1(af79cff317e5b57d49e463af16a9f616ed1eee08) ) \
	ROM_SYSTEM_BIOS( 6,  "us",    "EPR-17952A (USA 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6,  "epr-17952a.ic8",  0x000000, 0x080000, CRC(d1be2adf) SHA1(eaf1c3e5d602e1139d2090a78d7e19f04f916794) ) \
	ROM_SYSTEM_BIOS( 7,  "us1",   "EPR-17741a (USA 95/02/20)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7,  "epr-17741a.ic8",  0x000000, 0x080000, CRC(4166c663) SHA1(cc41d30de06160083d77e0dc5d69e61fec7fdcb5) ) \
	ROM_SYSTEM_BIOS( 8,  "tw",    "EPR-19854 (Taiwan 97/05/15)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8,  "epr-19854.ic8",   0x000000, 0x080000, CRC(e09d1f60) SHA1(b55cdcb45b2a5b0b35e352cf7625f0bd659084df) ) \
	ROM_SYSTEM_BIOS( 9,  "tw1",   "EPR-17953A (Taiwan 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 9,  "epr-17953a.ic8",  0x000000, 0x080000, CRC(a4c47570) SHA1(9efc73717ec8a13417e65c54344ded9fc25bf5ef) ) \
	ROM_SYSTEM_BIOS( 10, "tw2",   "EPR-17742A (Taiwan 95/02/20)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 10, "epr-17742a.ic8",  0x000000, 0x080000, CRC(02daf123) SHA1(23185beb1ce9c09b8719e57d1adb7b28c8141fd5) ) \
	ROM_SYSTEM_BIOS( 11,  "debug","Development (1.10, 95/01/13)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 11,  "stv110.bin",     0x000000, 0x080000, CRC(3dfeda92) SHA1(8eb33192a57df5f3a1dfb57263054867c6b2db6d) ) \
	ROM_SYSTEM_BIOS( 12, "dev",   "Development (1.061, 94/11/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 12, "stv1061.bin",     0x000000, 0x080000, CRC(728dbca3) SHA1(0ed2030177f0aa8285645c395ae9ad9f568ab1d6) ) \
	\
	ROM_REGION32_BE( 0x3000000, "abus", ROMREGION_ERASE00 ) /* SH2 code */

ROM_START( stvbios )
	STV_BIOS

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to multi cart mode
	ROM_LOAD( "stvbios.nv", 0x0000, 0x0080, CRC(15432ae2) SHA1(7d9364d546f3d3f839ec36be148076f8d26a65a6) )
ROM_END

/*

there appears to only be one main cartridge layout, just some having different positions populated if you use the ic named in
the test mode you have the following

some of the rom names were using something else and have been renamed to match test mode, old extension left in comments

( add 0x2000000 for real memory map location )

0x0000000 - 0x01fffff IC13 Header can be read from here .. IC13 roms are loaded on ODD bytes only
0x0200000 - 0x03fffff IC7  Header can also be read from here.  If No IC7 is present it seems to mirror IC13, but loaded normally (mausuke)
0x0400000 - 0x07fffff IC2
0x0800000 - 0x0bfffff IC3
0x0c00000 - 0x0ffffff IC4
0x1000000 - 0x13fffff IC5
0x1400000 - 0x17fffff IC6
0x1800000 - 0x1bfffff IC1
0x1c00000 - 0x1ffffff IC8
0x2000000 - 0x23fffff IC9
0x2400000 - 0x27fffff IC10
0x2800000 - 0x2bfffff IC11
0x2c00000 - 0x2ffffff IC12

*/

/*
country codes:
J = Japan
U = United States
E = Europe
T = Taiwan
B = Brazil
K = Korea
A = PAL Asia
L = Latin America
date codes:
(Original is yyyy/mm/dd,changed to reflect Capcom's one (yy/mm/dd))
Version codes:
V = Version(obviously ;)
(number before the dot) = game status (0=Sample,1=Master,2=Upgrade)
(number after the dot) = game version/release
There is also another internal code (called the "Product Number"),but AFAIK it's only used
by introdon in ST-V ("SG0000000"),and according to the manual it's even wrong! (SG is used
by Sega titles,and this is a Sunsoft game)It's likely to be a left-over...
*/

void stv_state::init_sanjeon()
{
	uint8_t *src = memregion("cart")->base();
	for (int x = 0; x < 0x3000000; x++)
	{
		src[x] = src[x]^0xff;

		src[x] = bitswap<8>(src[x],7,2,5,1,  3,6,4,0);
		src[x] = bitswap<8>(src[x],4,6,5,7,  3,2,1,0);
		src[x] = bitswap<8>(src[x],7,6,5,4,  2,3,1,0);
		src[x] = bitswap<8>(src[x],7,0,5,4,  3,2,1,6);
		src[x] = bitswap<8>(src[x],3,6,5,4,  7,2,1,0);
	}

	init_sasissu();
}


ROM_START( astrass )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20825.13",                0x0000001, 0x0100000, CRC(94a9ad8f) SHA1(861311c14cfa9f560752aa5b023c147a539cf135) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr20827.2",     0x0400000, 0x0400000, CRC(65cabbb3) SHA1(5e7cb090101dc42207a4084465e419f4311b6baf) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr20828.3",     0x0800000, 0x0400000, CRC(3934d44a) SHA1(969406b8bfac43b30f4d732702ca8cffeeefffb9) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr20829.4",     0x0c00000, 0x0400000, CRC(814308c3) SHA1(45c3f551690224c95acd156ae8f8397667927a04) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr20830.5",     0x1000000, 0x0400000, CRC(ff97fd19) SHA1(f37bcdce5f3f522527a44d59f1b8184ef290f829) ) // good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr20831.6",     0x1400000, 0x0400000, CRC(4408e6fb) SHA1(d4228cad8a1128e9426dac9ac62e9513a7a0117b) ) // good (was .16)
	ROM_LOAD16_WORD_SWAP( "mpr20826.1",     0x1800000, 0x0400000, CRC(bdc4b941) SHA1(c5e8b1b186324c2ccab617915f7bdbfe6897ca9f) ) // good (was .17)
	ROM_LOAD16_WORD_SWAP( "mpr20832.8",     0x1c00000, 0x0400000, CRC(af1b0985) SHA1(d7a0e4e0a8b0556915f924bdde8c3d14e5b3423e) ) // good (was .18s)
	ROM_LOAD16_WORD_SWAP( "mpr20833.9",     0x2000000, 0x0400000, CRC(cb6af231) SHA1(4a2e5d7c2fd6179c19cdefa84a03f9a34fbb9e70) ) // good (was .19s)

	// 610-0374-90   1998     317-5040-COM   ST-V      (yes, the 317-5040-COM chip was reused for 3 different games and on both Naomi and ST-V!)
	ROM_PARAMETER( ":315_5881:key", "052e2901" )
ROM_END

ROM_START( bakubaku )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr17969.13",               0x0000001, 0x0100000, CRC(bee327e5) SHA1(1d226db72d6ef68fd294f60659df7f882b25def6) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )

	ROM_LOAD16_WORD_SWAP( "mpr17970.2",    0x0400000, 0x0400000, CRC(bc4d6f91) SHA1(dcc241dcabea59325decfba3fd5e113c07958422) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17971.3",    0x0800000, 0x0400000, CRC(c780a3b3) SHA1(99587eea528a6413cacc3e4d3d1dbfff57b03dca) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17972.4",    0x0c00000, 0x0400000, CRC(8f29815a) SHA1(e86acd8096f2aee5f5e3ddfd3abb4f5c2b11df66) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17973.5",    0x1000000, 0x0400000, CRC(5f6e0e8b) SHA1(eeb5efb5216ab8b8fdee4656774bbd5a2a5b2d42) ) // good
ROM_END

ROM_START( colmns97 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19553.13",    0x000001, 0x100000, CRC(d4fb6a5e) SHA1(bd3cfb4f451b6c9612e42af5ddcbffa14f057329) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr19554.2",     0x400000, 0x400000, CRC(5a3ebcac) SHA1(46e3d1cf515a7ff8a8f97e5050b29dbbeb5060c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19555.3",     0x800000, 0x400000, CRC(74f6e6b8) SHA1(8080860550eb770e04447e344fb337748a249761) ) // good
ROM_END

ROM_START( cotton2 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20122.7",    0x0200000, 0x0200000, CRC(d616f78a) SHA1(8039dcdfdafb8327a19a1da46a67c0b3f7eee53a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20117.2",    0x0400000, 0x0400000, CRC(893656ea) SHA1(11e3160083ba018fbd588f07061a4e55c1efbebb) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20118.3",    0x0800000, 0x0400000, CRC(1b6a1d4c) SHA1(6b234d6b2d24df7f6d400a56698c0af2f78ce0e7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20119.4",    0x0c00000, 0x0400000, CRC(5a76e72b) SHA1(0a058627ddf78a0bcdaba328a58712419f24e33b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20120.5",    0x1000000, 0x0400000, CRC(7113dd7b) SHA1(f86add67c4e1349a9b9ebcd0145a30b1667df811) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20121.6",    0x1400000, 0x0400000, CRC(8c8fd521) SHA1(c715681330b5ed37a8506ac58ee2143baa721206) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20116.1",    0x1800000, 0x0400000, CRC(d30b0175) SHA1(2da5c3c02d68b8324948a8cdc93946d97fccdd8f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20123.8",    0x1c00000, 0x0400000, CRC(35f1b89f) SHA1(1d6007c380f817def734fc3030d4fe56df4a15be) ) // good
ROM_END

ROM_START( cottonbm )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21075.7",    0x0200000, 0x0200000, CRC(200b58ba) SHA1(6daad6d70a3a41172e8d9402af775c03e191232d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21070.2",    0x0400000, 0x0400000, CRC(56c0bf1d) SHA1(c2b564ce536c637bb723ed96683b27596e87ebe7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21071.3",    0x0800000, 0x0400000, CRC(2bb18df2) SHA1(e900adb94ad3f48be00a4ce33e915147dc6a8737) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21072.4",    0x0c00000, 0x0400000, CRC(7c7cb977) SHA1(376dfb8014050605b00b6545520bd544768f5828) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21073.5",    0x1000000, 0x0400000, CRC(f2e5a5b7) SHA1(9258d508ef6f6529efc4ad172fd29e69877a99eb) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21074.6",    0x1400000, 0x0400000, CRC(6a7e7a7b) SHA1(a0b1e7a85e623b59886b28797281df1d65b8a5aa) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21069.1",    0x1800000, 0x0400000, CRC(6a28e3c5) SHA1(60454b71db49b872e0cb89fae2259fed601588bd) ) // good
ROM_END

ROM_START( decathlt )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18967a.13",               0x0000001, 0x0100000, CRC(ac59c186) SHA1(7d4924d1e4c1b9257b58a690de988b3f6486e86f) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr18968.2",    0x0400000, 0x0400000, CRC(11a891de) SHA1(1a4fa8d7e07e1d8fdc8122ef8a5b93723c007cda) ) // good (was .1)
	ROM_LOAD16_WORD_SWAP( "mpr18969.3",    0x0800000, 0x0400000, CRC(199cc47d) SHA1(d78f7c6be7e9b43e208244c5c8722245f4c653e1) ) // good (was .2)
	ROM_LOAD16_WORD_SWAP( "mpr18970.4",    0x0c00000, 0x0400000, CRC(8b7a509e) SHA1(8f4d36a858231764ed09b26a1141d1f055eee092) ) // good (was .3)
	ROM_LOAD16_WORD_SWAP( "mpr18971.5",    0x1000000, 0x0400000, CRC(c87c443b) SHA1(f2fedb35c80e5c4855c7aebff88186397f4d51bc) ) // good (was .4)
	ROM_LOAD16_WORD_SWAP( "mpr18972.6",    0x1400000, 0x0400000, CRC(45c64fca) SHA1(ae2f678b9885426ce99b615b7f62a451f9ef83f9) ) // good (was .5)

#if 0
	// these are VDP2RAM dumps from the JPN Saturn version running in UME, saved from the debugger
	ROM_REGION32_BE( 0x3000000, "fake0", ROMREGION_ERASE00 )
	ROM_LOAD( "dec",    0x0000000, 0x80000,  CRC(5c8c4353) SHA1(18f317c8f0beac4ff3c9e56639b8e990e31f53ff) ) // on title screen
	ROM_REGION( 0x3000000, "fake1", ROMREGION_ERASE00 )
	ROM_LOAD( "dec2",    0x0000000, 0x80000, CRC(d1e08bc9) SHA1(3c5867de81a380bfc181d57214b86ce891b05f06) ) // on select char screen
#endif
ROM_END

ROM_START( decathlto )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18967.13",               0x0000001, 0x0100000, CRC(c0446674) SHA1(4917089d95613c9d2a936ed9fe3ebd22f461aa4f) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr18968.2",    0x0400000, 0x0400000, CRC(11a891de) SHA1(1a4fa8d7e07e1d8fdc8122ef8a5b93723c007cda) ) // good (was .1)
	ROM_LOAD16_WORD_SWAP( "mpr18969.3",    0x0800000, 0x0400000, CRC(199cc47d) SHA1(d78f7c6be7e9b43e208244c5c8722245f4c653e1) ) // good (was .2)
	ROM_LOAD16_WORD_SWAP( "mpr18970.4",    0x0c00000, 0x0400000, CRC(8b7a509e) SHA1(8f4d36a858231764ed09b26a1141d1f055eee092) ) // good (was .3)
	ROM_LOAD16_WORD_SWAP( "mpr18971.5",    0x1000000, 0x0400000, CRC(c87c443b) SHA1(f2fedb35c80e5c4855c7aebff88186397f4d51bc) ) // good (was .4)
	ROM_LOAD16_WORD_SWAP( "mpr18972.6",    0x1400000, 0x0400000, CRC(45c64fca) SHA1(ae2f678b9885426ce99b615b7f62a451f9ef83f9) ) // good (was .5)
ROM_END

ROM_START( diehard ) /* must use USA, Europe or Taiwan BIOS */
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19119.13",               0x0000001, 0x0100000, CRC(de5c4f7c) SHA1(35f670a15e9c86edbe2fe718470f5a75b5b096ac) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr19115.2",    0x0400000, 0x0400000, CRC(6fe06a30) SHA1(dedb90f800bae8fd9df1023eb5bec7fb6c9d0179) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19116.3",    0x0800000, 0x0400000, CRC(af9e627b) SHA1(a53921c3185a93ec95299bf1c29e744e2fa3b8c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19117.4",    0x0c00000, 0x0400000, CRC(74520ff1) SHA1(16c1acf878664b3bd866c9b94f3695ae892ac12f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19118.5",    0x1000000, 0x0400000, CRC(2c9702f0) SHA1(5c2c66de83f2ccbe97d3b1e8c7e65999e1fa2de1) ) // good
ROM_END

ROM_START( dnmtdeka )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19114.13",               0x0000001, 0x0100000, CRC(1fd22a5f) SHA1(c3d9653b12354a73a3e15f23a2ab7992ffb83e46) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr19115.2",    0x0400000, 0x0400000, CRC(6fe06a30) SHA1(dedb90f800bae8fd9df1023eb5bec7fb6c9d0179) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19116.3",    0x0800000, 0x0400000, CRC(af9e627b) SHA1(a53921c3185a93ec95299bf1c29e744e2fa3b8c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19117.4",    0x0c00000, 0x0400000, CRC(74520ff1) SHA1(16c1acf878664b3bd866c9b94f3695ae892ac12f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19118.5",    0x1000000, 0x0400000, CRC(2c9702f0) SHA1(5c2c66de83f2ccbe97d3b1e8c7e65999e1fa2de1) ) // good
ROM_END

ROM_START( ejihon )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18137.13",               0x0000001, 0x0080000, CRC(151aa9bc) SHA1(0959c60f31634816825acb57413838dcddb17d31) )
	ROM_RELOAD( 0x100001, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0080000 )
	ROM_LOAD16_WORD_SWAP( "mpr18138.2",    0x0400000, 0x0400000, CRC(f5567049) SHA1(6eb35e4b5fbda39cf7e8c42b6a568bd53a364d6d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18139.3",    0x0800000, 0x0400000, CRC(f36b4878) SHA1(e3f63c0046bd37b7ab02fb3865b8ebcf4cf68e75) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18140.4",    0x0c00000, 0x0400000, CRC(228850a0) SHA1(d83f7fa7df08407fa45a13661393679b88800805) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18141.5",    0x1000000, 0x0400000, CRC(b51eef36) SHA1(2745cba48dc410d6d31327b956886ec284b9eac3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18142.6",    0x1400000, 0x0400000, CRC(cf259541) SHA1(51e2c8d16506d6074f6511112ec4b6b44bed4886) ) // good
ROM_END

ROM_START( elandore )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21307.7",    0x0200000, 0x0200000, CRC(966ad472) SHA1(d6db41d1c40d08eb6bce8a8a2f491e7533daf670) ) // good (was .11s)
	ROM_LOAD16_WORD_SWAP( "mpr21301.2",    0x0400000, 0x0400000, CRC(1a23b0a0) SHA1(f9dbc7ba96dadfb00e5827622b557080449acd83) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr21302.3",    0x0800000, 0x0400000, CRC(1c91ca33) SHA1(ae11209088e3bf8fc4a92dca850d7303ce949b29) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr21303.4",    0x0c00000, 0x0400000, CRC(07b2350e) SHA1(f32f63fd8bec4e667f61da203d63be9a27798dfe) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr21304.5",    0x1000000, 0x0400000, CRC(cfea52ae) SHA1(4b6d27e0b2a95300ee9e07ebcdc4953d77c4efbe) ) // good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr21305.6",    0x1400000, 0x0400000, CRC(46cfc2a2) SHA1(8ca26bf8fa5ced040e815c125c13dd06d599e189) ) // good (was .16)
	ROM_LOAD16_WORD_SWAP( "mpr21306.1",    0x1800000, 0x0400000, CRC(87a5929c) SHA1(b259341d7b0e1fa98959bf52d23db5c308a8efdd) ) // good (was .17)
	ROM_LOAD16_WORD_SWAP( "mpr21308.8",    0x1c00000, 0x0400000, CRC(336ec1a4) SHA1(20d1fce050cf6132d284b91853a4dd5626372ef0) ) // good (was .18s)

	// 610-0374-126   1998     317-5043-COM   ST-V
	ROM_PARAMETER( ":315_5881:key", "05226d41" )
ROM_END

ROM_START( ffrevng10 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "opr21872.7",   0x0200000, 0x0200000, CRC(32d36fee) SHA1(441c4254ef2e9301e1006d69462a850ce339314b) ) // good (was .11s)
	ROM_LOAD16_WORD_SWAP( "mpr21873.2",   0x0400000, 0x0400000, CRC(dac5bd98) SHA1(6102035ce9eb2f83d7d9b20f989a151f45087c67) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr21874.3",   0x0800000, 0x0400000, CRC(0a7be2f1) SHA1(e2d13f36e54d1e2cb9d584db829c04a6ff65108c) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr21875.4",   0x0c00000, 0x0400000, CRC(ccb75029) SHA1(9611a08a2ad0e0e82137ded6205440a948a339a4) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr21876.5",   0x1000000, 0x0400000, CRC(bb92a7fc) SHA1(d9e0fab1104a46adeb0a0cfc0d070d4c63a28d55) ) // good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr21877.6",   0x1400000, 0x0400000, CRC(c22a4a75) SHA1(3276bc0628e71b432f21ba9a4f5ff7ccc8769cd9) ) // good (was .16)
	ROM_LOAD16_WORD_SWAP( "opr21878.1",   0x1800000, 0x0200000, CRC(2ea4a64d) SHA1(928a973dce5eba0a1628d61ba56a530de990a946) ) // good (was .17)

	// 610-0374-128   1998     317-5049-COM   ST-V
	ROM_PARAMETER( ":315_5881:key", "0524ac01" )
ROM_END

ROM_START( ffreveng )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE(      "ffr110.ic35",  0x0000001, 0x0100000, CRC(3ffea541) SHA1(715b070b1d574a99aeb12086de380b0b3aaa25a1) )

	// actual source ROM board have 21x 16Mbit FlashROMs instead
	ROM_LOAD16_WORD_SWAP( "opr21872.7",   0x0200000, 0x0200000, CRC(32d36fee) SHA1(441c4254ef2e9301e1006d69462a850ce339314b) ) // good (was .11s)
	ROM_LOAD16_WORD_SWAP( "mpr21873.2",   0x0400000, 0x0400000, CRC(dac5bd98) SHA1(6102035ce9eb2f83d7d9b20f989a151f45087c67) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr21874.3",   0x0800000, 0x0400000, CRC(0a7be2f1) SHA1(e2d13f36e54d1e2cb9d584db829c04a6ff65108c) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr21875.4",   0x0c00000, 0x0400000, CRC(ccb75029) SHA1(9611a08a2ad0e0e82137ded6205440a948a339a4) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr21876.5",   0x1000000, 0x0400000, CRC(bb92a7fc) SHA1(d9e0fab1104a46adeb0a0cfc0d070d4c63a28d55) ) // good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr21877.6",   0x1400000, 0x0400000, CRC(c22a4a75) SHA1(3276bc0628e71b432f21ba9a4f5ff7ccc8769cd9) ) // good (was .16)
	ROM_LOAD16_WORD_SWAP( "opr21878.1",   0x1800000, 0x0200000, CRC(2ea4a64d) SHA1(928a973dce5eba0a1628d61ba56a530de990a946) ) // good (was .17)

	// 610-0374-128   1998     317-5049-COM   ST-V
	ROM_PARAMETER( ":315_5881:key", "0524ac01" )
ROM_END

/* set system to 1 player to test rom */
ROM_START( fhboxers )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fr18541a.13",               0x0000001, 0x0100000, CRC(8c61a17c) SHA1(a8aef27b53482923a506f7daa4b7a38653b4d8a4) ) //(header is read from here, not ic7 even if both are populated on this board)

	ROM_LOAD16_WORD_SWAP( "mpr18538.7",    0x0200000, 0x0200000, CRC(7b5230c5) SHA1(70cebc3281580b43adf42c37318e12159c28a13d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18533.2",    0x0400000, 0x0400000, CRC(7181fe51) SHA1(646f95e1a5b64d721e961352cee6fd5adfd031ec) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18534.3",    0x0800000, 0x0400000, CRC(c87ef125) SHA1(c9ced130faf6dd9e626074b6519615654d8beb19) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18535.4",    0x0c00000, 0x0400000, CRC(929a64cf) SHA1(206dfc2a46befbcea974df1e27515c5759d88d00) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18536.5",    0x1000000, 0x0400000, CRC(51b9f64e) SHA1(bfbdfb73d24f26ce1cc5294c23a1712fb9631691) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18537.6",    0x1400000, 0x0400000, CRC(c364f6a7) SHA1(4db21bcf6ea3e75f9eb34f067b56a417589271c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18532.1",    0x1800000, 0x0400000, CRC(39528643) SHA1(e35f4c35c9eb13e1cdcc26cb2599bb846f2c1af7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18539.8",    0x1c00000, 0x0400000, CRC(62b3908c) SHA1(3f00e49beb0e5575cc4250a25c41f04dc91d6ed0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18540.9",    0x2000000, 0x0400000, CRC(4c2b59a4) SHA1(4d15503fcff0e9e0d1ed3bac724278102b506da0) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "fhboxers.nv", 0x0000, 0x0080, CRC(590fd6da) SHA1(1abe71c62c51a246d854f02cd4cb79fd0d427e88) )
ROM_END

/* set system to 1 player to test rom */
ROM_START( findlove )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20424.13",               0x0000001, 0x0100000, CRC(4e61fa46) SHA1(e34624d98cbdf2dd04d997167d3c4decd2f208f7) ) //(header is read from here, not ic7 even if both are populated on this board)

	ROM_LOAD16_WORD_SWAP( "mpr20431.7",    0x0200000, 0x0200000, CRC(ea656ced) SHA1(b2d6286081bd46a89d1284a2757b87d0bca1bbde) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20426.2",    0x0400000, 0x0400000, CRC(897d1747) SHA1(f3fb2c4ef8bc2c1658907e822f2ee2b88582afdd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20427.3",    0x0800000, 0x0400000, CRC(a488a694) SHA1(80ec81f32e4b5712a607208b2a45cfdf6d5e1849) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20428.4",    0x0c00000, 0x0400000, CRC(4353b3b6) SHA1(f5e56396b345ff65f57a23f391b77d401f1f58b5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20429.5",    0x1000000, 0x0400000, CRC(4f566486) SHA1(5b449288e33f02f2362ebbd515c87ea11cc02633) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20430.6",    0x1400000, 0x0400000, CRC(d1e11979) SHA1(14405997eefac22c42f0c86dca9411ba1dee9bf9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20425.1",    0x1800000, 0x0400000, CRC(67f104c4) SHA1(8e965d2ce554ba8d37254f6bf3931dff4bce1a43) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20432.8",    0x1c00000, 0x0400000, CRC(79fcdecd) SHA1(df8e7733a51e24196914fc66a024515ee1565599) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20433.9",    0x2000000, 0x0400000, CRC(82289f29) SHA1(fb6a1015621b1afa3913da162ae71ded6b674649) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20434.10",   0x2400000, 0x0400000, CRC(85c94afc) SHA1(dfc2f16614bc499747ea87567a21c86e7bddce45) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20435.11",   0x2800000, 0x0400000, CRC(263a2e48) SHA1(27ef4bf577d240e36dcb6e6a09b9c5f24e59ce8c) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20436.12",   0x2c00000, 0x0400000, CRC(e3823f49) SHA1(754d48635bd1d4fb01ff665bfe2a71593d92f688) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "findlove.nv", 0x0000, 0x0080, CRC(df2fa9f6) SHA1(f05450e1648eafd2ffb52e50769f07bdc75ffb95) )
ROM_END

ROM_START( finlarch )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "finlarch.13",               0x0000001, 0x0100000, CRC(4505fa9e) SHA1(96c6399146cf9c8f1d27a8fb6a265f937258004a) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr18257.2",    0x0400000, 0x0400000, CRC(137fdf55) SHA1(07a02fe531b3707e063498f5bc9749bd1b4cadb3) ) // good
	ROM_RELOAD(                            0x1400000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18258.3",    0x0800000, 0x0400000, CRC(f519c505) SHA1(5cad39314e46b98c24a71f1c2c10c682ef3bdcf3) ) // good
	ROM_RELOAD(                            0x1800000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18259.4",    0x0c00000, 0x0400000, CRC(5cabc775) SHA1(84383a4cbe3b1a9dcc6c140cff165425666dc780) ) // good
	ROM_RELOAD(                            0x1c00000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18260.5",    0x1000000, 0x0400000, CRC(f5b92082) SHA1(806ad85a187a23a5cf867f2f3dea7d8150065b8e) ) // good
	ROM_RELOAD(                            0x2000000, 0x0400000 )
ROM_END


ROM_START( gaxeduel )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr17766.13",               0x0000001, 0x0080000, CRC(a83fcd62) SHA1(4ce77ebaa0e93c6553ad8f7fb87cbdc32433402b) )
	ROM_RELOAD( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0080000 )
	ROM_LOAD16_WORD_SWAP( "mpr17768.2",    0x0400000, 0x0400000, CRC(d6808a7d) SHA1(83a97bbe1160cb45b3bdcbde8adc0d9bae5ded60) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17769.3",    0x0800000, 0x0400000, CRC(3471dd35) SHA1(24febddfe70984cebc0e6948ad718e0e6957fa82) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17770.4",    0x0c00000, 0x0400000, CRC(06978a00) SHA1(a8d1333a9f4322e28b23724937f595805315b136) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17771.5",    0x1000000, 0x0400000, CRC(aea2ea3b) SHA1(2fbe3e10d3f5a3b3099a7ed5b38b93b6e22e19b8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17772.6",    0x1400000, 0x0400000, CRC(b3dc0e75) SHA1(fbe2790c84466d186ea3e9d41edfcb7afaf54bea) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17767.1",    0x1800000, 0x0400000, CRC(9ba1e7b1) SHA1(f297c3697d2e8ba4476d672267163f91f371b362) ) // good
ROM_END

ROM_START( grdforce )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20844.7",    0x0200000, 0x0200000, CRC(283e7587) SHA1(477fabc27cfe149ad17757e31f10665dcf8c0860) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20839.2",    0x0400000, 0x0400000, CRC(facd4dd8) SHA1(2582894c98b31ab719f1865d4623dad6736dc877) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20840.3",    0x0800000, 0x0400000, CRC(fe0158e6) SHA1(73460effe69fb8f16dd952271542b7803471a599) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20841.4",    0x0c00000, 0x0400000, CRC(d87ac873) SHA1(35b8fa3862e09dca530e9597f983f5a22919cf08) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20842.5",    0x1000000, 0x0400000, CRC(baebc506) SHA1(f5f59f9263956d0c49c729729cf6db31dc861d3b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20843.6",    0x1400000, 0x0400000, CRC(263e49cc) SHA1(67979861ca2784b3ce39d87e7994e6e7351b40e5) ) // good
ROM_END

ROM_START( groovef )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19820.7",    0x0200000, 0x0100000, CRC(e93c4513) SHA1(f9636529224880c49bd2cc5572bd5bf41dbf911a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19815.2",    0x0400000, 0x0400000, CRC(1b9b14e6) SHA1(b1828c520cb108e2927a23273ebd2939dca52304) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19816.3",    0x0800000, 0x0400000, CRC(83f5731c) SHA1(2f645737f945c59a1a2fabf3b21a761be9e8c8a6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19817.4",    0x0c00000, 0x0400000, CRC(525bd6c7) SHA1(2db2501177fb0b44d0fad2054eddf356c4ea08f2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19818.5",    0x1000000, 0x0400000, CRC(66723ba8) SHA1(0a8379e46a8f8cab11befeadd9abdf59dba68e27) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19819.6",    0x1400000, 0x0400000, CRC(ee8c55f4) SHA1(f6d86b2c2ab43ec5baefb8ccc25e11af4d82712d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19814.1",    0x1800000, 0x0400000, CRC(8f20e9f7) SHA1(30ff5ad0427208e7265cb996e870c4dc0fbbf7d2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19821.8",    0x1c00000, 0x0400000, CRC(f69a76e6) SHA1(b7e41f34d8b787bf1b4d587e5d8bddb241c043a8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19822.9",    0x2000000, 0x0200000, CRC(5e8c4b5f) SHA1(1d146fbe3d0bfa68993135ba94ef18081ab65d31) ) // good
ROM_END

ROM_START( hanagumi )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20143.7",    0x0200000, 0x0100000, CRC(7bfc38d0) SHA1(66f223e7ff2b5456a6f4185b7ab36f9cd833351a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20138.2",    0x0400000, 0x0400000, CRC(fdcf1046) SHA1(cbb1f03879833c17feffdd6f5a4fbff06e1059a2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20139.3",    0x0800000, 0x0400000, CRC(7f0140e5) SHA1(f2f7de7620d66a596d552e1af491a0592ebc4e51) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20140.4",    0x0c00000, 0x0400000, CRC(2fa03852) SHA1(798ce008f6fc24a00f85298188c8d0d01933640d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20141.5",    0x1000000, 0x0400000, CRC(45d6d21b) SHA1(fe0f0b2195b74e79b8efb6a7c0b7bedca7194c48) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20142.6",    0x1400000, 0x0400000, CRC(e38561ec) SHA1(c04c400be033bc74a7bb2a60f6ae00853a2220d4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20137.1",    0x1800000, 0x0400000, CRC(181d2688) SHA1(950059f89eda30d8a5bce145421f507e226b8b3e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20144.8",    0x1c00000, 0x0400000, CRC(235b43f6) SHA1(e35d9bf15ac805513ab3edeca4f264647a2dc0b0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20145.9",    0x2000000, 0x0400000, CRC(aeaac7a1) SHA1(5c75ecce49a5c53dbb0b07e75f3a76e6db9976d0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20146.10",   0x2400000, 0x0400000, CRC(39bab9a2) SHA1(077132e6a03afd181ee9ca9ca4f7c9cbf418e57e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20147.11",   0x2800000, 0x0400000, CRC(294ab997) SHA1(aeba269ae7d056f07edecf96bc138231c66c3637) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20148.12",   0x2c00000, 0x0400000, CRC(5337ccb0) SHA1(a998bb116eb10c4044410f065c5ddeb845f9dab5) ) // good
ROM_END

ROM_START( introdon )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18937.13",               0x0000001, 0x0080000, CRC(1f40d766) SHA1(35d9751c1b23cfbf448f2a9e9cf3b121929368ae) )
	ROM_RELOAD( 0x0100001, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr18944.7",    0x0200000, 0x0100000, CRC(f7f75ce5) SHA1(0787ece9f89cc1847889adbf08ba5d3ccbc405de) ) // good
	ROM_RELOAD( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr18939.2",    0x0400000, 0x0400000, CRC(ef95a6e6) SHA1(3026c52ad542997d5b0e621b389c0e01240cb486) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18940.3",    0x0800000, 0x0400000, CRC(cabab4cd) SHA1(b251609573c4b0ccc933188f32226855b25fd9da) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18941.4",    0x0c00000, 0x0400000, CRC(f4a33a20) SHA1(bf0f33495fb5c9de4ae5036cedda65b3ece217e8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18942.5",    0x1000000, 0x0400000, CRC(8dd0a446) SHA1(a75e3552b0fb99e0b253c0906f62fabcf204b735) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18943.6",    0x1400000, 0x0400000, CRC(d8702a9e) SHA1(960dd3cb0b9eb1f18b8d0bc0da532b600d583ceb) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18938.1",    0x1800000, 0x0400000, CRC(580ecb83) SHA1(6c59f7da408b53f9fa7aa32c1b53328b5fd6334d) ) // good
ROM_END

/* set system to 1 player to test rom */
ROM_START( kiwames )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18737.13",               0x0000001, 0x0080000, CRC(cfad6c49) SHA1(fc69980a351ed13307706db506c79c774eabeb66) ) // bad
	ROM_RELOAD( 0x0100001, 0x0080000)

	ROM_LOAD16_WORD_SWAP( "mpr18738.2",    0x0400000, 0x0400000, CRC(4b3c175a) SHA1(b6d2438ae1d3d51950a7ed1eaadf2dae45c4e7b1) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18739.3",    0x0800000, 0x0400000, CRC(eb41fa67) SHA1(d12acebb1df9eafd17aff1841087f5017225e7e7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18740.4",    0x0c00000, 0x0200000, CRC(9ca7962f) SHA1(a09e0db2246b34ca7efa3165afbc5ba292a95398) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "kiwames.nv", 0x0000, 0x0080, CRC(c7002732) SHA1(57395d256a58ddcedd354ce1f2c458321be40505) )
ROM_END

ROM_START( maruchan )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20416.13",               0x0000001, 0x0100000, CRC(8bf0176d) SHA1(5bd468e2ffed042ee84e2ceb8712ff5883a1d824) ) // bad
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20417.2",    0x0400000, 0x0400000, CRC(636c2a08) SHA1(47986b71d68f6a1852e4e2b03ca7b6e48e83718b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20418.3",    0x0800000, 0x0400000, CRC(3f0d9e34) SHA1(2ec81e40ebf689d17b6421820bfb0a1280a8ef25) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20419.4",    0x0c00000, 0x0400000, CRC(ec969815) SHA1(b59782174051f5717b06f43e57dd8a2a6910d95f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20420.5",    0x1000000, 0x0400000, CRC(f2902c88) SHA1(df81e137e8aa4bd37e1d14fce4d593cfd14608f0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20421.6",    0x1400000, 0x0400000, CRC(cd0b477c) SHA1(5169cc47fae465b11bc50f5e8410d84c2b2eee42) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20422.1",    0x1800000, 0x0400000, CRC(66335049) SHA1(59f1968001d1e9fe30990a56309bae18033eee62) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20423.8",    0x1c00000, 0x0400000, CRC(2bd55832) SHA1(1a1a510f30882d4d726b594a6541a12c552fafb4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20443.9",    0x2000000, 0x0400000, CRC(8ac288f5) SHA1(0c08874e6ab2b07b17438721fb535434a626115f) ) // good
ROM_END

/* set system to 1 player to test rom */
ROM_START( myfairld )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21000.7",    0x0200000, 0x0200000, CRC(2581c560) SHA1(5fb64f0e09583d50dfea7ad613d45aad30b677a5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20995.2",    0x0400000, 0x0400000, CRC(1bb73f24) SHA1(8773654810de760c5dffbb561f43e259b074a61b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20996.3",    0x0800000, 0x0400000, CRC(993c3859) SHA1(93f95e3e080a08961784482607919c1ab3eeb5e5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20997.4",    0x0c00000, 0x0400000, CRC(f0bf64a4) SHA1(f51431f1a736bbc498fa0baa1f8570f89984d9f9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20998.5",    0x1000000, 0x0400000, CRC(d3b19786) SHA1(1933e57272cd68cc323922fa93a9af97dcef8450) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20999.6",    0x1400000, 0x0400000, CRC(82e31f25) SHA1(0cf74af14abb6ede21d19bc22041214232751594) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20994.1",    0x1800000, 0x0400000, CRC(a69243a0) SHA1(e5a1b6ec62bdd5b015ed6cf48f5a6aabaf4bd837) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21001.8",    0x1c00000, 0x0400000, CRC(95fbe549) SHA1(8cfb48f353b2849600373d66f293f103bca700df) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "myfairld.nv", 0x0000, 0x0080, CRC(c7cf3a5a) SHA1(5365f38047821305658f94395f03cf2c49c87576) )
ROM_END

ROM_START( othellos )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20967.7",    0x0200000, 0x0200000, CRC(efc05b97) SHA1(a533366c3aaba90dcac8f3654db9ad902efca258) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20963.2",    0x0400000, 0x0400000, CRC(2cc4f141) SHA1(8bd1998aff8615b34d119fab3637a08ed6e8e1e4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20964.3",    0x0800000, 0x0400000, CRC(5f5cda94) SHA1(616be219a2512e80c875eddf05137c23aedf6f65) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20965.4",    0x0c00000, 0x0400000, CRC(37044f3e) SHA1(cbc071554cfd8bb12a337c04b169de6c6309c3ab) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20966.5",    0x1000000, 0x0400000, CRC(b94b83de) SHA1(ba1b3135d0ad057f0786f94c9d06b5e347bedea8) ) // good
ROM_END

ROM_START( pblbeach )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18852.13",               0x0000001, 0x0080000, CRC(d12414ec) SHA1(0f42ec9e41983781b6892622b00398a102072aa7) ) // bad
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr18853.2",    0x0400000, 0x0400000, CRC(b9268c97) SHA1(8734e3f0e6b2849d173e3acc9d0308084a4e84fd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18854.3",    0x0800000, 0x0400000, CRC(3113c8bc) SHA1(4e4600646ddd1978988d27430ffdf0d1d405b804) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18855.4",    0x0c00000, 0x0400000, CRC(daf6ad0c) SHA1(2a14a6a42e4eb68abb7a427e43062dfde2d13c5c) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18856.5",    0x1000000, 0x0400000, CRC(214cef24) SHA1(f62b462170b377cff16bb6c6126cbba00b013a87) ) // good
ROM_END

ROM_START( prikura )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19337.7",    0x0200000, 0x0200000, CRC(76f69ff3) SHA1(5af2e1eb3288d70c2a1c71d0b6370125d65c7757) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19333.2",    0x0400000, 0x0400000, CRC(eb57a6a6) SHA1(cdacaa7a2fb1a343195e2ac5fd02eabf27f89ccd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19334.3",    0x0800000, 0x0400000, CRC(c9979981) SHA1(be491a4ac118d5025d6a6f2d9267a6d52f21d2b6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19335.4",    0x0c00000, 0x0400000, CRC(9e000140) SHA1(9b7dc3dc7f9dc048d2fcbc2b44ae79a631ceb381) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19336.5",    0x1000000, 0x0400000, CRC(2363fa4b) SHA1(f45e53352520be4ea313eeab87bcab83f479d5a8) ) // good
ROM_END

ROM_START( puyosun )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr19531.13",               0x0000001, 0x0080000, CRC(ac81024f) SHA1(b22c7c1798fade7ae992ff83b138dd23e6292d3f) ) // bad
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr19533.2",    0x0400000, 0x0400000, CRC(17ec54ba) SHA1(d4cdc86926519291cc78980ec513e1cfc677e76e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19534.3",    0x0800000, 0x0400000, CRC(820e4781) SHA1(7ea5626ad4e1929a5ec28a99ec12bc364df8f70d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19535.4",    0x0c00000, 0x0400000, CRC(94fadfa4) SHA1(a7d0727cf601e00f1ea31e6bf3e591349c3f6030) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19536.5",    0x1000000, 0x0400000, CRC(5765bc9c) SHA1(b217c292e7cc8ed73a39a3ae7009bc9dd031e376) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19537.6",    0x1400000, 0x0400000, CRC(8b736686) SHA1(aec347c0f3e5dd8646e85f68d71ca9acc3bf62c3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19532.1",    0x1800000, 0x0400000, CRC(985f0c9d) SHA1(de1ad42ef3cf3f4f071e9801696407be7ae29d21) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19538.8",    0x1c00000, 0x0400000, CRC(915a723e) SHA1(96480441a69d6aad3887ed6f46b0a6bebfb752aa) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19539.9",    0x2000000, 0x0400000, CRC(72a297e5) SHA1(679987e62118dd1bf7c074f4b88678e1a1187437) ) // good
ROM_END

ROM_START( rsgun )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20958.7",   0x0200000, 0x0200000, CRC(cbe5a449) SHA1(b4744ab71ccbadda1921ba43dd1148e57c0f84c5) ) // good (was .11s)
	ROM_LOAD16_WORD_SWAP( "mpr20959.2",   0x0400000, 0x0400000, CRC(a953330b) SHA1(965274a7297cb88e281fcbdd3ec5025c6463cc7b) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr20960.3",   0x0800000, 0x0400000, CRC(b5ab9053) SHA1(87c5d077eb1219c35fa65b4e11d5b62e826f5236) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr20961.4",   0x0c00000, 0x0400000, CRC(0e06295c) SHA1(0ec2842622f3e9dc5689abd58aeddc7e5603b97a) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr20962.5",   0x1000000, 0x0400000, CRC(f1e6c7fc) SHA1(0ba0972f1bc7c56f4e0589d3e363523cea988bb0) ) // good (was .15)

	// 610-0374-96   1998     317-5041-COM   ST-V
	ROM_PARAMETER( ":315_5881:key", "05272d01" )
ROM_END

ROM_START( sandor )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "sando-r.13",               0x0000001, 0x0100000, CRC(fe63a239) SHA1(01502d4494f968443581cd2c74f25967d41f775e) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr18635.8",   0x1c00000, 0x0400000, CRC(441e1368) SHA1(acb2a7e8d44c2203b8d3c7a7b70e20ffb120bebf) ) // good
	ROM_RELOAD(                           0x0400000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18636.9",   0x2000000, 0x0400000, CRC(fff1dd80) SHA1(36b8e1526a4370ae33fd4671850faf51c448bca4) ) // good
	ROM_RELOAD(                           0x0800000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18637.10",  0x2400000, 0x0400000, CRC(83aced0f) SHA1(6cd1702b9c2655dc4f56c666607c333f62b09fc0) ) // good
	ROM_RELOAD(                           0x0c00000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18638.11",  0x2800000, 0x0400000, CRC(caab531b) SHA1(a77bdcc27d183896c0ed576eeebcc1785d93669e) ) // good
	ROM_RELOAD(                           0x1000000, 0x0400000 )
ROM_END


/*
Treasure Hunt
Deniam (Licensed to Sega Enterprises, Ltd), 1997

PCB Number: LEX-0704

This is a non-Sega-manufactured STV cart which works with Japanese and USA bioses.
The cart is mostly the same as the Sega carts, containing not a lot except some ROMs
and logic chips.

On the top side, there are two 27C040 EPROMs and positions for 5 maskROMs, but only 4 of
them are populated. In between the two eproms is an unpopulated position for a TSOP40 flashROM.
On the bottom are locations for 5 maskROMs (none are populated) and also some logic ICs.
*/

ROM_START( thunt )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "th-ic7_2.stv",    0x0200000, 0x0080000, CRC(c4e993de) SHA1(7aa433bc2623cb19a09d4ef4c8233a2d29901020) )
	ROM_LOAD16_BYTE( "th-ic7_1.stv",    0x0200001, 0x0080000, CRC(1355cc18) SHA1(a9b731228a807b2b01f933fe0f7dcdbadaf89b7e) )

	ROM_LOAD16_WORD_SWAP( "th-e-2.ic2",   0x0400000, 0x0400000, CRC(47315694) SHA1(4a7cc195b98dbca146f5efe3b5b35be0f13b2f4a) )
	ROM_LOAD16_WORD_SWAP( "th-e-3.ic3",   0x0800000, 0x0400000, CRC(c9290b44) SHA1(4e51e667edf330cc20e600cf1500f332cc533e20) )
	ROM_LOAD16_WORD_SWAP( "th-e-4.ic4",   0x0c00000, 0x0400000, CRC(c672e40b) SHA1(4ba189de7b8fb5ca45fb15c89cdaf0f860831d52) )
	ROM_LOAD16_WORD_SWAP( "th-e-5.ic5",   0x1000000, 0x0400000, CRC(3914b805) SHA1(1331ce82ba0bdfc76fe3456a5252e69c00e2cf1f) )
ROM_END

ROM_START( thuntk )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "2.ic13_2",    0x0200000, 0x0080000, CRC(6cae2926) SHA1(e8d5745b4228de24672da5017cb3dab58344f59f) )
	ROM_LOAD16_BYTE( "1.ic13_1",    0x0200001, 0x0080000, CRC(460727c8) SHA1(da7171b65734264e10692e3408ac93beb374c65e) )

	ROM_LOAD( "bom210-10.ic2",   0x1c00000, 0x0400000, CRC(f59d0827) SHA1(2bed4b2c78e9b4e9332f576e1b264a6343f4cfff) )
	ROM_RELOAD(                  0x0400000, 0x0400000 )
	ROM_LOAD( "bom210-11.ic3",   0x2000000, 0x0400000, CRC(44e5a13e) SHA1(aee3c06662a1d083f3bd01292cf2694132f63533) )
	ROM_RELOAD(                  0x0800000, 0x0400000 )
	ROM_LOAD( "bom210-12.ic4",   0x2400000, 0x0400000, CRC(deabc701) SHA1(cb313ae9bf6f115682fc76647a999e12c98f6120) )
	ROM_RELOAD(                  0x0c00000, 0x0400000 )
	ROM_LOAD( "bom210-13.ic5",   0x2800000, 0x0400000, CRC(5ece1d5c) SHA1(6d88f71b485bf2b3c164fa22f1c7ecaba4b3f5b1) )
	ROM_RELOAD(                  0x1000000, 0x0400000 )
ROM_END


ROM_START( sanjeon )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "ic11",               0x0000001, 0x0200000, CRC(9abae8d4) SHA1(ddbe4c8fff8fa59d63e278e95f245145d2da8aeb) )

	ROM_LOAD( "ic13",    0x0400000, 0x0200000, CRC(f72c1d13) SHA1(a2b168d187034024b83fbbe2f5eec78816285da9) ) // ic2 good
	ROM_LOAD( "ic14",    0x0600000, 0x0200000, CRC(bcd72105) SHA1(fb88a1f2589fef5d9845027646259ec8e781771b) ) // ic2 good
	ROM_LOAD( "ic15",    0x0800000, 0x0200000, CRC(8c9c8352) SHA1(312e9e6435c6e9a4a19b72e234b087d4f5b069a5) ) // ic3 good
	ROM_LOAD( "ic16",    0x0a00000, 0x0200000, CRC(07e11512) SHA1(5fb5e93a167aae501bd799aba98f7dbc3762a795) ) // ic3 good
	ROM_LOAD( "ic17",    0x0c00000, 0x0200000, CRC(46b7b344) SHA1(a325b724e6346f585582bb2581d3063b1c8ddee9) ) // ic4 good
	ROM_LOAD( "ic18",    0x0e00000, 0x0200000, CRC(d48404e1) SHA1(e4927d17cb19f6825f41b41056b89de224cf97de) ) // ic4 good
	ROM_LOAD( "ic19",    0x1000000, 0x0200000, CRC(33d23bb9) SHA1(0179438a0b14ad7eb553a24d86fae778671fcc49) ) // ic5 good
	ROM_LOAD( "ic20",    0x1200000, 0x0200000, CRC(f8cc1038) SHA1(06bcb25eb5df77bd503b0692a96ec66c00976fe9) ) // ic5 good
	ROM_LOAD( "ic21",    0x1400000, 0x0200000, CRC(74ceb649) SHA1(91b7eb699b7167f890528149178644a74ef3ad17) ) // ic6 good
	ROM_LOAD( "ic22",    0x1600000, 0x0200000, CRC(85f31277) SHA1(277c28d44cabd9081a8151d6ec27d47b5606435a) ) // ic6 good
	ROM_LOAD( "ic12",    0x1800000, 0x0400000, CRC(d5ebc84e) SHA1(f990b793cdfadbbac69c680191660f4a2f282ba2) ) // ic1 good
ROM_END


ROM_START( sasissu )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20542.13",               0x0000001, 0x0100000, CRC(0e632db5) SHA1(9bc52794892eec22d381387d13a0388042e30714) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20544.2",    0x0400000, 0x0400000, CRC(661fff5e) SHA1(41f4ddda7adf004b52cc9a076606a60f31947d19) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20545.3",    0x0800000, 0x0400000, CRC(8e3a37be) SHA1(a3227cdc4f03bb088e7f9aed225b238da3283e01) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20546.4",    0x0c00000, 0x0400000, CRC(72020886) SHA1(e80bdeb11b726eb23f2283950d65d55e31a5672e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20547.5",    0x1000000, 0x0400000, CRC(8362e397) SHA1(71f13689a60572a04b91417a9a48adfd3bd0f5dc) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20548.6",    0x1400000, 0x0400000, CRC(e37534d9) SHA1(79988cbb1537ca99fdd0288a86564fe1f714d052) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20543.1",    0x1800000, 0x0400000, CRC(1f688cdf) SHA1(a90c1011119adb50e0d9d5cd3d7616a307b2d7e8) ) // good
ROM_END

/* set to 1 player to test */
ROM_START( seabass )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "seabassf.13",               0x0000001, 0x0100000, CRC(6d7c39cc) SHA1(d9d1663134420b75c65ee07d7d547254785f2f83) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20551.2",    0x0400000, 0x0400000, CRC(9a0c6dd8) SHA1(26600372cc673ce3678945f4b5dc4e3ab31643a4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20552.3",    0x0800000, 0x0400000, CRC(5f46b0aa) SHA1(1aa576b15971c0ffb4e08d4802246841b31b6f35) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20553.4",    0x0c00000, 0x0400000, CRC(c0f8a6b6) SHA1(2038b9231a950450267be0db24b31d8035db79ad) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20554.5",    0x1000000, 0x0400000, CRC(215fc1f9) SHA1(f042145622ba4bbbcce5f050a4c9eae42cb7adcd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20555.6",    0x1400000, 0x0400000, CRC(3f5186a9) SHA1(d613f307ab150a7eae358aa449206af05db5f9d7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20550.1",    0x1800000, 0x0400000, CRC(083e1ca8) SHA1(03944dd8fe86f305ca4bd2d71e2140e03798ffc9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20556.8",    0x1c00000, 0x0400000, CRC(1fd70c6c) SHA1(d9d2e362d13238216f4f7e10095fb8383bbd91e8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20557.9",    0x2000000, 0x0400000, CRC(3c9ba442) SHA1(2e5b795cf4cdc11ab3e4887b2f77c7147c6e3eec) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "seabass.nv", 0x0000, 0x0080, CRC(4e7c0944) SHA1(dbba78f6a7f3e7d12d5c4fff36117db82f5f9c01) )
ROM_END

ROM_START( shanhigw )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr18341.7",    0x0200000, 0x0200000, CRC(cc5e8646) SHA1(a733616c118140ff3887d30d595533f9a1beae06) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18340.2",    0x0400000, 0x0200000, CRC(8db23212) SHA1(85d604a5c6ab97188716dbcd77d365af12a238fe) ) // good
ROM_END

ROM_START( shienryu )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19631.7",    0x0200000, 0x0200000, CRC(3a4b1abc) SHA1(3b14b7fdebd4817da32ea374c15a38c695ffeff1) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19632.2",    0x0400000, 0x0400000, CRC(985fae46) SHA1(f953bde91805b97b60d2ab9270f9d2933e064d95) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19633.3",    0x0800000, 0x0400000, CRC(e2f0b037) SHA1(97861d09e10ce5d2b10bf5559574b3f489e28077) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-shienryu.bin", 0x0000, 0x0080, CRC(98db6925) SHA1(e78545e8f62d19f8e00197c62ff0e56f6c85e355) )
ROM_END

ROM_START( smleague ) /* only runs with the USA bios */
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18777.13",               0x0000001, 0x0080000, CRC(8d180866) SHA1(d47ebabab6e06400312d39f68cd818852e496b96) )
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr18778.8",    0x1c00000, 0x0400000, CRC(25e1300e) SHA1(64f3843f62cee34a47244ad5ee78fb2aa35289e3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18779.9",    0x2000000, 0x0400000, CRC(51e2fabd) SHA1(3aa361149af516f16d7d422596ee82014a183c2b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18780.10",   0x2400000, 0x0400000, CRC(8cd4dd74) SHA1(9ffec1280b3965d52f643894bdfecdd792028191) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18781.11",   0x2800000, 0x0400000, CRC(13ee41ae) SHA1(cdbaeac4c90b5ee84233c299612f7f28280a6ba6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18782.12",   0x2c00000, 0x0200000, CRC(9be2270a) SHA1(f2de5cd6b269f123305e30bed2b474019e4f05b8) ) // good
ROM_END

ROM_START( sokyugrt )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19188.13",               0x0000001, 0x0100000, CRC(45a27e32) SHA1(96e1bab8bdadf7071afac2a0a6dd8fd8989f12a6) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr19189.2",    0x0400000, 0x0400000, CRC(0b202a3e) SHA1(6691b5af2cacd6092ec03886b78c2565953fa297) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19190.3",    0x0800000, 0x0400000, CRC(1777ded8) SHA1(dd332ac79f0a6d82b6bde35b795b2845003dd1a5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19191.4",    0x0c00000, 0x0400000, CRC(ec6eb07b) SHA1(01fe4832ece8638ea6f4060099d9105fe8092c88) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19192.5",    0x1000000, 0x0200000, CRC(cb544a1e) SHA1(eb3ba9758487d0e8c4bbfc41453fe35b35cce3bf) ) // good
ROM_END

/* set to 1 player to test */
ROM_START( sss )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr21488.13",               0x0000001, 0x0080000, CRC(71c9def1) SHA1(a544a0b4046307172d2c1bf426ed24845f87d894) )
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr21489.2",    0x0400000, 0x0400000, CRC(4c85152b) SHA1(78f2f1c31718d5bf631d8813daf9a11ea2a0e451) ) // ic2 good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr21490.3",    0x0800000, 0x0400000, CRC(03da67f8) SHA1(02f9ba7549ca552291dc0ff1b631103015838bba) ) // ic3 good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr21491.4",    0x0c00000, 0x0400000, CRC(cf7ee784) SHA1(af823df2d60d8ef3d17628b95a04136b807ca095) ) // ic4 good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr21492.5",    0x1000000, 0x0400000, CRC(57753894) SHA1(5c51167c158443d02a53d724a5ceb73055876c06) ) // ic5 good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr21493.6",    0x1400000, 0x0400000, CRC(efb2d271) SHA1(a591e48206704fbda5fef3ce69ad279da1017ed6) ) // ic6 good (was .16)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "sss.nv", 0x0000, 0x0080, CRC(3473b2f3) SHA1(6480b4b321af8ee6e967710e74f2556c17bfca97) )

	// 610-0374-107   1998     317-5042-COM   ST-V
	ROM_PARAMETER( ":315_5881:key", "052b6901" )
ROM_END

ROM_START( suikoenb )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr17834.13",               0x0000001, 0x0100000, CRC(746ef686) SHA1(e31c317991a687662a8a2a45aed411001e5f1941) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr17836.2",    0x0400000, 0x0400000, CRC(55e9642d) SHA1(5198291cd1dce0398eb47760db2c19eae99273b0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17837.3",    0x0800000, 0x0400000, CRC(13d1e667) SHA1(cd513ceb33cc20032090113b61227638cf3b3998) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17838.4",    0x0c00000, 0x0400000, CRC(f9e70032) SHA1(8efdbcce01bdf77acfdb293545c59bf224a9c7d2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17839.5",    0x1000000, 0x0400000, CRC(1b2762c5) SHA1(5c7d5fc8a4705249a5b0ea64d51dc3dc95d723f5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17840.6",    0x1400000, 0x0400000, CRC(0fd4c857) SHA1(42caf22716e834d59e60d45c24f51d95734e63ae) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17835.1",    0x1800000, 0x0400000, CRC(77f5cb43) SHA1(a4f54bc08d73a56caee5b26bea06360568655bd7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17841.8",    0x1c00000, 0x0400000, CRC(f48beffc) SHA1(92f1730a206f4a0abf7fb0ee1210e083a464ad70) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17842.9",    0x2000000, 0x0400000, CRC(ac8deed7) SHA1(370eb2216b8080d3ddadbd32804db63c4ebac76f) ) // good
ROM_END

ROM_START( twcup98 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20819.24",    0x0000001, 0x0100000, CRC(d930dfc8) SHA1(f66cc955181720661a0334fe67fa5750ddf9758b) ) // tested as IC13
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20821.12",    0x0400000, 0x0400000, CRC(2d930d23) SHA1(5fcaf4257f3639cb3aa407d2936f616499a09d97) ) // tested as IC2
	ROM_LOAD16_WORD_SWAP( "mpr20822.13",    0x0800000, 0x0400000, CRC(8b33a5e2) SHA1(d5689ac8aad63509febe9aa4077351be09b2d8d4) ) // tested as IC3
	ROM_LOAD16_WORD_SWAP( "mpr20823.14",    0x0c00000, 0x0400000, CRC(6e6d4e95) SHA1(c387d03ba27580c62ac0bf780915fdf41552df6f) ) // tested as IC4
	ROM_LOAD16_WORD_SWAP( "mpr20824.15",    0x1000000, 0x0400000, CRC(4cf18a25) SHA1(310961a5f114fea8938a3f514dffd5231e910a5a) ) // tested as IC5

	// 610-0374-89   1998     317-5039-COM   ST-V
	ROM_PARAMETER( ":315_5881:key", "05200913" )
ROM_END

ROM_START( twsoc98 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr-20820.ic24",    0x0000001, 0x0100000, CRC(b10451f9) SHA1(180b5071eaa5d6e9903da3fe4845ee2b6a93ef22) ) // tested as IC13
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20821.12",    0x0400000, 0x0400000, CRC(2d930d23) SHA1(5fcaf4257f3639cb3aa407d2936f616499a09d97) ) // tested as IC2
	ROM_LOAD16_WORD_SWAP( "mpr20822.13",    0x0800000, 0x0400000, CRC(8b33a5e2) SHA1(d5689ac8aad63509febe9aa4077351be09b2d8d4) ) // tested as IC3
	ROM_LOAD16_WORD_SWAP( "mpr20823.14",    0x0c00000, 0x0400000, CRC(6e6d4e95) SHA1(c387d03ba27580c62ac0bf780915fdf41552df6f) ) // tested as IC4
	ROM_LOAD16_WORD_SWAP( "mpr20824.15",    0x1000000, 0x0400000, CRC(4cf18a25) SHA1(310961a5f114fea8938a3f514dffd5231e910a5a) ) // tested as IC5

	// 610-0374-91   1998     317-5039-COM   ST-V
	ROM_PARAMETER( ":315_5881:key", "05200913" )
ROM_END

ROM_START( vfkids )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr18914.13",               0x0000001, 0x0100000, CRC(cd35730a) SHA1(645b52b449766beb740ab8f99957f8f431351ceb) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr18916.4",    0x0c00000, 0x0400000, CRC(4aae3ddb) SHA1(b75479e73f1bce3f0c27fbd90820fa51eb1914a6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18917.5",    0x1000000, 0x0400000, CRC(edf6edc3) SHA1(478e958f4f10a8126a00c83feca4a55ad6c25503) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18918.6",    0x1400000, 0x0400000, CRC(d3a95036) SHA1(e300bbbb71fb06027dc539c9bbb12946770ffc95) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18915.1",    0x1800000, 0x0400000, CRC(09cc38e5) SHA1(4dfe0e2f21f746020ec557e62487aa7558cbc1fd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18919.8",    0x1c00000, 0x0400000, CRC(4ac700de) SHA1(b1a8501f1683de380dfa49c9cabbe28bd70a5b26) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18920.9",    0x2000000, 0x0400000, CRC(0106e36c) SHA1(f7c30dc9fedb9da079dd7d52fdecbeb8721c5dee) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18921.10",   0x2400000, 0x0400000, CRC(c23d51ad) SHA1(0169b7e2df84e8caa2b349843bd0673f6de2195f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18922.11",   0x2800000, 0x0400000, CRC(99d0ab90) SHA1(e9c82a826cc76ffbe2423913645cf5d5ba2506d6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18923.12",   0x2c00000, 0x0400000, CRC(30a41ae9) SHA1(78a3d88b5e6cf669b660460ac967daf408038883) ) // good
ROM_END

ROM_START( vfremix )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr17944.13",               0x0000001, 0x0080000, CRC(3304c175) SHA1(6d847efad73d361cac4d7fcb452ccf89efa13e24) ) // 27C040
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr17946.2",    0x0400000, 0x0400000, CRC(4cb245f7) SHA1(363d9936b27043b5858c956a45736ac05aefc54e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17947.3",    0x0800000, 0x0400000, CRC(fef4a9fb) SHA1(1b4bd095962db769da17d3644df10f62d041e914) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17948.4",    0x0c00000, 0x0400000, CRC(3e2b251a) SHA1(be6191c18727d7cbc6399fd4c1aaae59304af30c) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17949.5",    0x1000000, 0x0400000, CRC(b2ecea25) SHA1(320c0e7ce34e81e2fe6400cbeb2cb3ca74426cc8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17950.6",    0x1400000, 0x0400000, CRC(5b1f981d) SHA1(693b5744d210a2ac8b77e7c8c87f07ca859f8aed) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17945.1",    0x1800000, 0x0200000, CRC(03ede188) SHA1(849c7fab5b97e043fea3deb8df6cc195ccced0e0) ) // good
ROM_END

/* set to 1 player to test */
ROM_START( vmahjong )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19620.7",    0x0200000, 0x0200000, CRC(c98de7e5) SHA1(5346f884793bcb080aa01967e91b54ced4a9802f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19615.2",    0x0400000, 0x0400000, CRC(c62896da) SHA1(52a5b10ca8af31295d2d700349eca038c418b522) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19616.3",    0x0800000, 0x0400000, CRC(f62207c7) SHA1(87e60183365c6f7e62c7a0667f88df0c7f5457fd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19617.4",    0x0c00000, 0x0400000, CRC(ab667e19) SHA1(2608a567888fe052753d0679d9a831d7706dbc86) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19618.5",    0x1000000, 0x0400000, CRC(9782ceee) SHA1(405dd42706416e128b1e2fde225b5343e9330092) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19619.6",    0x1400000, 0x0400000, CRC(0b76866c) SHA1(10add2993dfe9daf757ec2ff8675390081a93c0a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19614.1",    0x1800000, 0x0400000, CRC(b83b3f03) SHA1(e5a5919ee74964633eaaf4af2fe04c38604ccf16) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19621.8",    0x1c00000, 0x0400000, CRC(f92616b3) SHA1(61a9dda92a86a02d027260e11b1bad3b0dda9f02) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "vmahjong.nv", 0x0000, 0x0080, CRC(4e6487f4) SHA1(d6d930ab5f21b8c4f42812d08b3ee90f2bc94081) )
ROM_END

ROM_START( winterht )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr20108.13",    0x0000001, 0x0100000, CRC(1ef9ced0) SHA1(abc90ce341cd17bb77349d611d6879389611f0bf) ) // bad
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20110.2",    0x0400000, 0x0400000, CRC(238ef832) SHA1(20fade5730ff8e249a1450c41bfdff6e133f4768) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20111.3",    0x0800000, 0x0400000, CRC(b0a86f69) SHA1(e66427f70413ad43fccc38423962c5eeda01094f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20112.4",    0x0c00000, 0x0400000, CRC(3ba2b49b) SHA1(5ad154a8b774075479d791e29cbaf221d47557fc) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20113.5",    0x1000000, 0x0400000, CRC(8c858b41) SHA1(d05d2980363c8440863fe2fdb39274de246bd4b9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20114.6",    0x1400000, 0x0400000, CRC(b723862c) SHA1(1e0a08669f16fc4cb647124e0c215233ccb98e5a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20109.1",    0x1800000, 0x0400000, CRC(c1a713b8) SHA1(a7fefa6e9a1e3aecff5ead41da6fd3aec2ef502a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20115.8",    0x1c00000, 0x0400000, CRC(dd01f2ad) SHA1(3bb48dc8670d9460fea2a67400ddb573472c2f4f) ) // good
ROM_END

ROM_START( znpwfv )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20398.13",    0x0000001, 0x0100000, CRC(3fb56a0b) SHA1(13c2fa2d94b106d39e46f71d15fbce3607a5965a) ) // good
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20400.2",    0x0400000, 0x0400000, CRC(1edfbe05) SHA1(b0edd3f3d57408101ae6eb0aec742afbb4d289ca) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20401.3",    0x0800000, 0x0400000, CRC(99e98937) SHA1(e1b4d12a0b4d0fe97a62fcc085e19cce77657c99) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20402.4",    0x0c00000, 0x0400000, CRC(4572aa60) SHA1(8b2d76ea8c6e2f472c6ee7c9b6ad6e80e6a1a85a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20403.5",    0x1000000, 0x0400000, CRC(26a8e13e) SHA1(07f5564b704598e3c3580d3d620ecc4f14549dbd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20404.6",    0x1400000, 0x0400000, CRC(0b70275d) SHA1(47b8672e19c698dc948760f7091f4c6280e728d0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20399.1",    0x1800000, 0x0400000, CRC(c178a96e) SHA1(65f4aa05187d48ba8ad4fe75ff6ffe1f8524831d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20405.8",    0x1c00000, 0x0400000, CRC(f53337b7) SHA1(09a21f81016ee54f10554ae1f790415d7436afe0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20406.9",    0x2000000, 0x0400000, CRC(b677c175) SHA1(d0de7b5a29928036df0bdfced5a8021c0999eb26) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20407.10",   0x2400000, 0x0400000, CRC(58356050) SHA1(f8fb5a14f4ec516093c785891b05d55ae345754e) ) // good
ROM_END

ROM_START( znpwfvt )
	STV_BIOS
	ROM_DEFAULT_BIOS( "tw" ) // only runs with Taiwanese BIOS ROMs

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20408.13",    0x0000001, 0x0100000, CRC(1d62fcf6) SHA1(3651261aa755da27b11462f2705311b7c639a687) ) // good
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20400.2",    0x0400000, 0x0400000, CRC(1edfbe05) SHA1(b0edd3f3d57408101ae6eb0aec742afbb4d289ca) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20401.3",    0x0800000, 0x0400000, CRC(99e98937) SHA1(e1b4d12a0b4d0fe97a62fcc085e19cce77657c99) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20402.4",    0x0c00000, 0x0400000, CRC(4572aa60) SHA1(8b2d76ea8c6e2f472c6ee7c9b6ad6e80e6a1a85a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20403.5",    0x1000000, 0x0400000, CRC(26a8e13e) SHA1(07f5564b704598e3c3580d3d620ecc4f14549dbd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20404.6",    0x1400000, 0x0400000, CRC(0b70275d) SHA1(47b8672e19c698dc948760f7091f4c6280e728d0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20399.1",    0x1800000, 0x0400000, CRC(c178a96e) SHA1(65f4aa05187d48ba8ad4fe75ff6ffe1f8524831d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20405.8",    0x1c00000, 0x0400000, CRC(f53337b7) SHA1(09a21f81016ee54f10554ae1f790415d7436afe0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20406.9",    0x2000000, 0x0400000, CRC(b677c175) SHA1(d0de7b5a29928036df0bdfced5a8021c0999eb26) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20407.10",   0x2400000, 0x0400000, CRC(58356050) SHA1(f8fb5a14f4ec516093c785891b05d55ae345754e) ) // good
ROM_END

ROM_START( danchih )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21974.7",    0x0200000, 0x0200000, CRC(e7472793) SHA1(11b7b11cf492eb9cf69b50e7cfac46a5b86849ac) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21970.2",    0x0400000, 0x0400000, CRC(34dd7f4d) SHA1(d5c45da94ec5b6584049caf09516f1ad4ba3adb5) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21971.3",    0x0800000, 0x0400000, CRC(8995158c) SHA1(fbbd171d67eebf43630d6054bc1b9132f6b38183) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21972.4",    0x0c00000, 0x0400000, CRC(68a39090) SHA1(cff1b909c4191660570012eb5e4cb6a7467bc79e) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21973.5",    0x1000000, 0x0400000, CRC(b0f23f14) SHA1(4e7076c29fd57bb3ef9af50a6104e39ecda94e06) )// good
ROM_END

ROM_START( danchiq )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(e216bfc8) SHA1(7a08fa32281e272dbf5e7daea50a1800cc225c1b) )//ic 7
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(b95aa5ac) SHA1(2766c5414643034a0f6d746050557516bd3753df) )//ic 2
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(df6ebd48) SHA1(fcccafbee1b8b952b07ed0e7e86219eed9cf4a93) )
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(cf6a2b76) SHA1(1f7522d446d57b78d099bae553133d5e7e54ff70) )//ic 3
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(0b6a9901) SHA1(b4c335199d3e49a9ae5d474b10130abc4718cdf9) )
	ROM_LOAD16_WORD_SWAP( "ic32",    0x0c00000, 0x0200000, CRC(0b4604f5) SHA1(547cba4a80baf126e87f87529aa933587643d359) )//ic 4
	ROM_LOAD16_WORD_SWAP( "ic34",    0x0e00000, 0x0200000, CRC(616e20fa) SHA1(45c175e79b5701db9726d157ff92eee368f4bbf9) )
	ROM_LOAD16_WORD_SWAP( "ic36",    0x1000000, 0x0200000, CRC(43474e08) SHA1(30b3ede287d5de93c6e0219bfd0a5d7ed5b6a958) )//ic 5
	ROM_LOAD16_WORD_SWAP( "ic23",    0x1200000, 0x0200000, CRC(d080eb71) SHA1(9c39b887697c8872f0cb655cff24282ad3b90e9b) )
	ROM_LOAD16_WORD_SWAP( "ic25",    0x1400000, 0x0200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )//(Untested)
ROM_END

ROM_START( mausuke )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE(             "ic13.bin",      0x0000001, 0x0100000, CRC(b456f4cd) SHA1(91cbe703ec7c1dd45eb3b05bdfeb06e3570599d1) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 ) // needs the rom mapped here to appear 'normal'
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )

	ROM_LOAD16_WORD_SWAP( "mcj-00.2",      0x0400000, 0x0200000, CRC(4eeacd6f) SHA1(104ca230f22cd11cc536b34abd482e54791b4d0f) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-01.3",      0x0800000, 0x0200000, CRC(365a494b) SHA1(29713dfc83a9ade63ebcc7994d14cd785c4500b9) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-02.4",      0x0c00000, 0x0200000, CRC(8b8e4931) SHA1(0c94e2ccb72902d7786d1101a3958504f7151077) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-03.5",      0x1000000, 0x0200000, CRC(9015a0e7) SHA1(8ba8a3723267e631169dc1e06620260fbccce4bd) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-04.6",      0x1400000, 0x0200000, CRC(9d1beaee) SHA1(c63b61378860319fff2e605c7b9afaa5f1bc4cd2) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-05.1",      0x1800000, 0x0200000, CRC(a7626a82) SHA1(c12a099132c5b9234a2de5674f3b8ba5fdd35289) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-06.8",      0x1c00000, 0x0200000, CRC(1ab8e90e) SHA1(8e22f03c1791a983eb330b2a9199e5349a0b1baa) )// good
ROM_END

/* acclaim game, not a standard cart ... */
ROM_START( batmanfr )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	/* Thanks to Runik to point this out*/
	ROM_LOAD16_BYTE( "350-mpa1.u19",    0x0000000, 0x0100000, CRC(2a5a8c3a) SHA1(374ec55a39ea909cc672e4a629422681d1f2da05) )
	ROM_RELOAD(                         0x0200000, 0x0100000 )
	ROM_LOAD16_BYTE( "350-mpa1.u16",    0x0000001, 0x0100000, CRC(735e23ab) SHA1(133e2284a07a611aed8ada2707248f392f4509aa) )
	ROM_RELOAD(                         0x0200001, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "gfx0.u1",    0x0400000, 0x0400000, CRC(a82d0b7e) SHA1(37a7a177634d51620b1b43e58732987df166c7e6) )
	ROM_LOAD16_WORD_SWAP( "gfx1.u3",    0x0800000, 0x0400000, CRC(a41e55d9) SHA1(b896d3a6c36d325c3cece699da54f340a4512703) )
	ROM_LOAD16_WORD_SWAP( "gfx2.u5",    0x0c00000, 0x0400000, CRC(4c1ebeb7) SHA1(cdd139652d9484ae5837a39c2fd48d0a8d966d43) )
	ROM_LOAD16_WORD_SWAP( "gfx3.u8",    0x1000000, 0x0400000, CRC(f679a3e7) SHA1(db11b033b8bbdd80b81e3bc098bd40ad3a8784f2) )
	ROM_LOAD16_WORD_SWAP( "gfx4.u12",   0x1400000, 0x0400000, CRC(52d95242) SHA1(b554a95933c2be4c72fb4226d3bc4775695da2c1) )
	ROM_LOAD16_WORD_SWAP( "gfx5.u15",   0x1800000, 0x0400000, CRC(e201f830) SHA1(5aa22fcc8f2e153d1abc3aa4050c594b3942ee67) )
	ROM_LOAD16_WORD_SWAP( "gfx6.u18",   0x1c00000, 0x0400000, CRC(c6b381a3) SHA1(46431f1e47c084a0bf85535d35af27471653b008) )

	ROM_REGION( 0x1000000, "rax", 0 )
	ROM_LOAD( "350snda1.u52",  0x000000, 0x080000, CRC(9027e7a0) SHA1(678df530838b078964a044ce734776f391654e6c) )
	ROM_LOAD( "snd0.u48",      0x400000, 0x200000, CRC(02b1927c) SHA1(08b21d8b31b0f15c59fb5bb7eaf425e6fe04f7b5) )
	ROM_LOAD( "snd1.u49",      0x600000, 0x200000, CRC(58b18eda) SHA1(7f3105fe04d9c0cdfd76e3323f623a4d0f7dad06) )
	ROM_LOAD( "snd2.u50",      0x800000, 0x200000, CRC(51d626d6) SHA1(0e68b79dcb653dcba48121ca2d4f692f90afa85e) )
	ROM_LOAD( "snd3.u51",      0xa00000, 0x200000, CRC(31af26ae) SHA1(2c9f4c078afec55964b5c2a4d00f5c43f2661a04) )
ROM_END

/*
Critter Crusher EXP
Sega, 1995.

This is a cart for Sega STV system. The game involves hitting 'critters' on
screen with a rubber/plastic hammer. The cab has a large LED display to show how many
'critters' were hit. The hammer positioning might work like a lightgun and the 'hitting'
may be like pulling the lightgun trigger?

The ROM cart appears to be a re-used Virtua Fighter Remix cart.
On top there is one 27C040 EPROM, EPR-18821 @ IC13
6 maskROMs, MPR-17945 to MPR-17950 (already dumped, known Virtua Fighter Remix ROMs)
On the other side of the PCB are 2 more maskROMs, MPR-18788 @ IC9 and MPR-18789 @ IC8

*/

ROM_START( critcrsh ) /* Must use Europe or Asia BIOS */
	STV_BIOS
	ROM_DEFAULT_BIOS( "euro" )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr-18821.ic13",  0x0000001, 0x0080000, CRC(9a6658e2) SHA1(16dbae3d9ab584713afcb403f89fe71049609245) )
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
//  ROM_LOAD16_WORD_SWAP( "mpr17946.2",    0x0400000, 0x0400000, CRC(4cb245f7) SHA1(363d9936b27043b5858c956a45736ac05aefc54e) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17947.3",    0x0800000, 0x0400000, CRC(fef4a9fb) SHA1(1b4bd095962db769da17d3644df10f62d041e914) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17948.4",    0x0c00000, 0x0400000, CRC(3e2b251a) SHA1(be6191c18727d7cbc6399fd4c1aaae59304af30c) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17949.5",    0x1000000, 0x0400000, CRC(b2ecea25) SHA1(320c0e7ce34e81e2fe6400cbeb2cb3ca74426cc8) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17950.6",    0x1400000, 0x0400000, CRC(5b1f981d) SHA1(693b5744d210a2ac8b77e7c8c87f07ca859f8aed) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17945.1",    0x1800000, 0x0200000, CRC(03ede188) SHA1(849c7fab5b97e043fea3deb8df6cc195ccced0e0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18789.ic8", 0x1c00000, 0x0400000, CRC(b388616f) SHA1(0b2c5a547c3a6a8fb9f4ca54336cf6dc9adb8c6a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18788.ic9", 0x2000000, 0x0400000, CRC(feae5867) SHA1(7d2e47d5ab18700a246d53fdb7872a905cdac55a) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "critcrsh.nv", 0x0000, 0x0080, CRC(3da9860e) SHA1(05b315aa71fcfc4e617266e1c5e4954eccbf7854) )
ROM_END

/*
Tatacot
Sega, 1995.

Japanese version of Critter Crusher. Like Critter Crusher this is also a conversion,
but this time from a Ejihon Tantei Jimusho cart.

*/

ROM_START( tatacot ) /* Must use Japan or Asia BIOS */
	STV_BIOS
	ROM_DEFAULT_BIOS( "jp" )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr-18790.ic13",  0x0000001, 0x0080000, CRC(d95155dc) SHA1(b08b75d15aad073eecf8b04fd2d718366bb6f6bb) )
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
//  ROM_LOAD16_WORD_SWAP( "mpr18138.2",    0x0400000, 0x0400000, CRC(f5567049) SHA1(6eb35e4b5fbda39cf7e8c42b6a568bd53a364d6d) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr18139.3",    0x0800000, 0x0400000, CRC(f36b4878) SHA1(e3f63c0046bd37b7ab02fb3865b8ebcf4cf68e75) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr18140.4",    0x0c00000, 0x0400000, CRC(228850a0) SHA1(d83f7fa7df08407fa45a13661393679b88800805) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr18141.5",    0x1000000, 0x0400000, CRC(b51eef36) SHA1(2745cba48dc410d6d31327b956886ec284b9eac3) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr18142.6",    0x1400000, 0x0400000, CRC(cf259541) SHA1(51e2c8d16506d6074f6511112ec4b6b44bed4886) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18789.ic8", 0x1c00000, 0x0400000, CRC(b388616f) SHA1(0b2c5a547c3a6a8fb9f4ca54336cf6dc9adb8c6a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18788.ic9", 0x2000000, 0x0400000, CRC(feae5867) SHA1(7d2e47d5ab18700a246d53fdb7872a905cdac55a) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "critcrsh.nv", 0x0000, 0x0080, CRC(3da9860e) SHA1(05b315aa71fcfc4e617266e1c5e4954eccbf7854) )
ROM_END

/*
Sports Fishing

There's the regular STV main board with dumped BIOS EPR-18343 and on top is a plug in board (837-11781) containing 2 smaller plug in modules.
One is the CD ROM controller (838-10834) with....
Hitachi HD6437097 (SH1 variant) (QFP144)
20MHz Xtal
Hitachi YGR019B (QFP168)
HM514260 4M DRAM

The other is an MPEG decoder board (837-10835 171-6814D PC BD MPEG) containing....
HM514260 4M DRAM
Hitachi HD814102F (QFP100)
Sega 315-5745 HD814101FE (QFP144)
a curious PLCC44 marked SEGA MPR-17610A-H. The MPR-xxxxx suggests it's a PLCC mask ROM, but type unknown????)
*/

ROM_START( sfish2 )
//  STV_BIOS // - sports fishing 2 uses its own bios

	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "epr18343.bin",   0x000000, 0x080000, CRC(48e2eecf) SHA1(a38bfbd5f279525e413b18b5ed3f37f6e9e31cdc) ) /* sport fishing 2 bios */

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr-18427.ic13",  0x0000001, 0x0100000, CRC(3f25bec8) SHA1(43a5342b882d5aec0f35a8777cb475659f43b1c4) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr-18273.ic2",    0x0400000, 0x0400000, CRC(6fec0193) SHA1(5bbda289a5ca58c5bf57307360b07f0bb98f7356) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18274.ic3",    0x0800000, 0x0400000, CRC(a6d76d23) SHA1(eee8c824eff4485d1b3af93a4fd5b21262eec803) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18275.ic4",    0x0c00000, 0x0200000, CRC(7691deca) SHA1(aabb6b098963caf51f66aefa0a97aed7eb86c308) ) // good

	DISK_REGION( "stvcd" )
	DISK_IMAGE_READONLY( "cdp-00428", 0, SHA1(166cb5518fa5e0ab15d40dade70fa8913089dcd2) )

	ROM_REGION32_BE( 0x3000000, "abus", ROMREGION_ERASE00 ) /* SH2 code */
ROM_END

ROM_START( sfish2j )
//  STV_BIOS // - sports fishing 2 uses its own bios

	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "epr18343.bin",   0x000000, 0x080000, CRC(48e2eecf) SHA1(a38bfbd5f279525e413b18b5ed3f37f6e9e31cdc) ) /* sport fishing 2 bios */

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18344.a",      0x0000001, 0x0100000, CRC(5a7de018) SHA1(88e0c2a9a9d4ebf699878c0aa9737af85f95ccf8) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr-18273.ic2",    0x0400000, 0x0400000, CRC(6fec0193) SHA1(5bbda289a5ca58c5bf57307360b07f0bb98f7356) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18274.ic3",    0x0800000, 0x0400000, CRC(a6d76d23) SHA1(eee8c824eff4485d1b3af93a4fd5b21262eec803) ) // good

	DISK_REGION( "stvcd" )
	DISK_IMAGE_READONLY( "cdp-00386b", 0, SHA1(2cb357a930bb7fa668949717ec6daaad2669d137) )

	ROM_REGION32_BE( 0x3000000, "abus", ROMREGION_ERASE00 ) /* SH2 code */
ROM_END


ROM_START( magzun )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "flash.ic13",               0x0000001, 0x0100000, CRC(e6f0aca0) SHA1(251d4d9c5a332d13af3a144c5eb9d8e7836bdd1b) ) // good
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr-19354.ic2",    0x0400000, 0x0400000, CRC(a23822e7) SHA1(10ca5d39dcaaf35b80168a08d8a18d77fba1d2ce) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19355.ic3",    0x0800000, 0x0400000, CRC(d70e5ebc) SHA1(2d560f6b6e693b2b91cf5ff5c4f0890cc2176f91) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19356.ic4",    0x0c00000, 0x0400000, CRC(3bc43fe9) SHA1(f72b0f3208e2f411f4c9cc76c317a605acd32a67) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19357.ic5",    0x1000000, 0x0400000, CRC(aa749370) SHA1(f09b0aa94fcc983419aaf2465dae5c9c64606158) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19358.ic6",    0x1400000, 0x0400000, CRC(0969f1ec) SHA1(25b9671f0975172f283458d46e160080dd1d19e9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19359.ic1",    0x1800000, 0x0400000, CRC(b0d06f9c) SHA1(19e04c9c3a0bea5950aba8e1975962fa37722f32) ) // good

	ROM_REGION(0x1000, "subboard", ROMREGION_ERASE00 )
	ROM_LOAD("microm", 0x0000, 0x1000, NO_DUMP ) // we are missing the driver ROM for sub-board (used to control the mic)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 3 players
	ROM_LOAD( "magzun.nv", 0x0000, 0x0080, CRC(42700321) SHA1(1f2ba760c410312539c8677223edcd1cda3b51d4) )
ROM_END

// The internal ROM checks on these Atlus Print Club / Name clubs aren't very good
//  they test the ROM space, but usually test multiple ROMs under one IC label in
//  the test, and that IC label doesn't even relate to the actual cartridge at all.
//  furthermore several sets use blank roms, and even test that they're present!


ROM_START( stress )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr-21300a.ic13",    0x0000001, 0x0100000, CRC(899d829e) SHA1(b6c6da92dc108353998b29c0659d288645541519) ) // good
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr-21290.ic2",    0x0400000, 0x0400000, CRC(a49d29f3) SHA1(8f6c26fd9e94a9e03dd0029026d205cf481fe151) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21291.ic3",    0x0800000, 0x0400000, CRC(9452ba20) SHA1(8a9ff546901715f99bb911616c74ae30ebd7c6d7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21292.ic4",    0x0c00000, 0x0400000, CRC(f60268e2) SHA1(5c2febb94553a941a68e9611617750a89c82e783) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21293.ic5",    0x1000000, 0x0400000, CRC(794946e0) SHA1(7881adb92fbf3efecb88f30834cdd1ed863b0b0e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21294.ic6",    0x1400000, 0x0400000, CRC(550843bb) SHA1(5e278a0ae60f7bee23b5f1ee10b8ce31effc9718) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21289.ic1",    0x1800000, 0x0400000, CRC(c2ee8bea) SHA1(5fa0e6b492cc272c33b01adbef9233e6c8098827) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21296.ic8",    0x1c00000, 0x0400000, CRC(b825c42a) SHA1(7c6f737b69b9283345f8fa23cb1d110b9b5f3d7e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21297.ic9",    0x2000000, 0x0400000, CRC(4bff7469) SHA1(8a36405a1a292d60b5918e604b460ed98740fae3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21298.ic10",   0x2400000, 0x0400000, CRC(68d07144) SHA1(5021f43d19105b484135ebddd3fa203233096c98) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21299.ic11",   0x2800000, 0x0400000, CRC(ecc521c6) SHA1(f7ed4dd1cbe179652fdfdde34929b41a1fdcf9e2) ) // good
ROM_END

/* the rom test for this is in 'each game test'  */
ROM_START( nclubv3 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(b4008ed0) SHA1(acb3784acad971eb5f4920760dc23a16330e7bad) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(4e894850) SHA1(eb7c3399505a45816701197a45062b9f34e5a3e1) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(5b6b023f) SHA1(cf17c5857d85d4326dfe2ce40cf96989f9f78ecd) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(b7beab03) SHA1(de703e461a2bdd87b0695bd2f16e4c97d11bcf92) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(a9f81069) SHA1(60d88c7c20178a00d6927c37069ab0c374ebf51e) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic32",    0x0c00000, 0x0200000, CRC(02708d66) SHA1(6881b0b05e55989953a16f6ba503ba891b849c07) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic34",    0x0e00000, 0x0200000, CRC(c79d0537) SHA1(9d595f718ff8f8ff7ca88100f35c589d5f9b4216) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic36",    0x1000000, 0x0200000, CRC(0c9df896) SHA1(4d8c18205e7aa90bfaa677ecff2b65128f2ad47c) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic23",    0x1200000, 0x0200000, CRC(bd922829) SHA1(4c6f988173e439a05a77da043d856e142b0da831) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic25",    0x1400000, 0x0200000, CRC(f77f9e24) SHA1(9a9636114e74c1fd7bd67db8005af02ef6a75ab1) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "nclubv3.nv", 0x0000, 0x0080, CRC(9122a9e9) SHA1(5318994905e005567709c41449547c545182bece) )
ROM_END

ROM_START( techbowl ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(5058db21) SHA1(eec908bbfb9ec0fdca0002e69f32c1c030086456) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(34090f6d) SHA1(b8bc344ab826d5c9584afb01dba1c720b8dbc74d) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(fb073352) SHA1(a5164aa5854ab3095f704ab73b6e4fb9ed0e0785) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(530e0ceb) SHA1(8d14eb9dbf253a4563587d256a15492384e7ca5c) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 Fill, OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "techbowl.nv", 0x0000, 0x0080, CRC(5bebc2b7) SHA1(e189e891e1753059fbaad4ce82ddf191d5e8176a) )
ROM_END

ROM_START( micrombc ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(8385bc45) SHA1(0bd60d7560cb2313d68470d0572850a7b8c501fd) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(84ecb42f) SHA1(005dee9a0912d4b1b7f5157bc3cde96548c1e348) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(869bc19c) SHA1(638125007d331beef567c976ea6f4e7a21acdb64) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(0c3db354) SHA1(c4d43da7cea1b4d5ca3ac545afde10344a4a385b) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic32",    0x0c00000, 0x0200000, CRC(62c10626) SHA1(58cb0ca0330fa7a62b277ab0ff84bff65b81bb23) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic34",    0x1000000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 Fill. OK
	ROM_LOAD16_WORD_SWAP( "ic36",    0x1200000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 Fill, OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "micrombc.nv", 0x0000, 0x0080, CRC(6e89815f) SHA1(4478f614fb61859f4ee7bf55462f737387887e6f) )
ROM_END

ROM_START( choroqhr ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22.bin",     0x0200000, 0x200000, CRC(22c58710) SHA1(6d8b849f6fcf6566193dcff6e2c7c857b7d0d9bf) )
	ROM_LOAD16_WORD_SWAP( "ic24.bin",     0x0400000, 0x200000, CRC(09b8a154) SHA1(cfd212c6fe6188b9c665650b21f2fd80cd65268f) )
	ROM_LOAD16_WORD_SWAP( "ic26.bin",     0x0600000, 0x200000, CRC(136ca5e9) SHA1(8697a415d0958e58f5cea5dcc767dd6a4cbdef5c) )
	ROM_LOAD16_WORD_SWAP( "ic28.bin",     0x0800000, 0x200000, CRC(3c949563) SHA1(ab2a9f9ec23071cc236dee945b436a9cd73efb92) )
	ROM_LOAD16_WORD_SWAP( "ic30.bin",     0x0a00000, 0x200000, CRC(7e93078d) SHA1(10fa99029a3e741ea0fddcf00ee07b5fd039bf19) )
	ROM_LOAD16_WORD_SWAP( "ic32.bin",     0x0c00000, 0x200000, CRC(86cdcbd8) SHA1(90060cde84c75fd4146ebf6b9101f04140408e88) )
	ROM_LOAD16_WORD_SWAP( "ic34.bin",     0x0e00000, 0x200000, CRC(be2ed0a0) SHA1(a9225ba6b78fa0119fc6484828f4d4cc6ea05d8f) )
	ROM_LOAD16_WORD_SWAP( "ic36.bin",     0x1000000, 0x200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // preconfigured to 1 player
	ROM_LOAD( "choroqhr.nv", 0x0000, 0x0080, CRC(6e89815f) SHA1(4478f614fb61859f4ee7bf55462f737387887e6f) )
ROM_END

ROM_START( pclub2 ) // set to 1p / runs with the USA bios
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclub2_ic22",    0x0200000, 0x0200000, CRC(d2ceade7) SHA1(a4300322e582f403d9207290f3900e1a72fcb9b9) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2_ic24",    0x0400000, 0x0200000, CRC(0e968c2d) SHA1(fbcc7533fcb6b87cd8255fc2d307ae618301ea64) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2_ic26",    0x0600000, 0x0200000, CRC(ab51da70) SHA1(85214aa805ffc9de59900dc0cd4e19e5ab756bf7) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2_ic28",    0x0800000, 0x0200000, CRC(3a654b2a) SHA1(7398e25836bfbdeab6350759f25c420c3b496172) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2_ic30",    0x0a00000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 Fill, OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2.nv", 0x0000, 0x0080, CRC(00d0f04e) SHA1(8b5a3e1c52e34443f83fd4a8948a00cacb5071d0) )
ROM_END



ROM_START( pclub2fc ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclub2fc_ic22",    0x0200000, 0x0200000, CRC(109c61a5) SHA1(bfb3e014e49064443ef290bc068ffcd459ae132d) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2fc_ic24",    0x0400000, 0x0200000, CRC(0ccc5538) SHA1(15133fc9c85f0a384d49841d874a0fe9a76057ce) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2fc_ic26",    0x0600000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 Fill, OK
	ROM_LOAD16_WORD_SWAP( "pclub2fc_ic28",    0x0800000, 0x0200000, CRC(ff9643ca) SHA1(3309f970f87324b06cc48add386019f769abcd89) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2fc_ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2fc.nv", 0x0000, 0x0080, CRC(c8082326) SHA1(d24ad66eb01a58d3a117a49606003522b6f1feba) )
ROM_END


ROM_START( pclub2pf ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclb2puf.ic22",    0x0200000, 0x0200000, CRC(a14282f2) SHA1(b96e70693d8e71b090e20efdd3aa6228e7289fa4) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclb2puf.ic24",    0x0400000, 0x0200000, CRC(4fb4dc74) SHA1(1f174512c9cd5420d7f935cbc6b5875836f6e825) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclb2puf.ic26",    0x0600000, 0x0200000, CRC(d20bbfb5) SHA1(5f2768e0e306bd0e3ed9b4e1d234aac8fd7155e6) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclb2puf.ic28",    0x0800000, 0x0200000, CRC(da658ae9) SHA1(24293c2b23b3009956fc05df5177a27415754301) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclb2puf.ic30",    0x0a00000, 0x0200000, CRC(cafc0e6b) SHA1(fa2ac54260336d5dd1ced7ccaf87115511ece1f8) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2pf.nv", 0x0000, 0x0080, CRC(447bb3bd) SHA1(9fefec09849bfa0c14b49e73ff13e2a538dff511) )
ROM_END

ROM_START( prc297wi ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "prc297wi_ic22",    0x0200000, 0x0200000, CRC(589f6705) SHA1(d10897ab26c3ecdd518087562207de131133646c) ) // OK - IC7
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic24",    0x0400000, 0x0200000, CRC(4bd706d1) SHA1(e3c52c63bb93d9fa836c300865423a226bf74586) ) // OK - IC2
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic26",    0x0600000, 0x0200000, CRC(417e182a) SHA1(4df04a390523e52e48efcc48891bc54452f351c9) ) // OK - IC2
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic28",    0x0800000, 0x0200000, CRC(73da594e) SHA1(936b0af4a32d5b93847bbf2ecfc8d334290059c0) ) // OK - IC3
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK - IC3
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic32",    0x0c00000, 0x0200000, CRC(20437e93) SHA1(dfd2026bec6b2f418cd1cbfa7266717211d013b6) ) // OK - IC4
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic34",    0x0e00000, 0x0200000, CRC(9639b003) SHA1(8f95b024ad19151e1e642d58aa785d14ae3a0661) ) // OK - IC4
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic36",    0x1000000, 0x0200000, CRC(dd1b57b6) SHA1(8450355ec6cdc9718f8579f8702f3900f686c3f8) ) // OK - IC5
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic23",    0x1200000, 0x0200000, CRC(e3d9d12b) SHA1(28ec3727774ef8a6a241238ad134a5adab8327fd) ) // OK - IC5
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic25",    0x1400000, 0x0200000, CRC(71238374) SHA1(0dc534628a98aba508bdef58f8b812908414ae48) ) // OK - IC6
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic27",    0x1600000, 0x0200000, CRC(7485a9a2) SHA1(17999a5192f185a27c08c2f05e19c65977b8f84e) ) // OK - IC6

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(9ba58358) SHA1(555ac21321b3051f7083cd72176ddc0fef2d4155) )
ROM_END

ROM_START( prc297wia ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclb297w_ic22_alt",    0x0200000, 0x0200000, CRC(1feb3bfe) SHA1(cb79908a13e32c3c00e5892d988088a902d6f874) ) // OK - IC7
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic24",    0x0400000, 0x0200000, CRC(4bd706d1) SHA1(e3c52c63bb93d9fa836c300865423a226bf74586) ) // OK - IC2
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic26",    0x0600000, 0x0200000, CRC(417e182a) SHA1(4df04a390523e52e48efcc48891bc54452f351c9) ) // OK - IC2
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic28",    0x0800000, 0x0200000, CRC(73da594e) SHA1(936b0af4a32d5b93847bbf2ecfc8d334290059c0) ) // OK - IC3
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK - IC3
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic32",    0x0c00000, 0x0200000, CRC(20437e93) SHA1(dfd2026bec6b2f418cd1cbfa7266717211d013b6) ) // OK - IC4
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic34",    0x0e00000, 0x0200000, CRC(9639b003) SHA1(8f95b024ad19151e1e642d58aa785d14ae3a0661) ) // OK - IC4
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic36",    0x1000000, 0x0200000, CRC(dd1b57b6) SHA1(8450355ec6cdc9718f8579f8702f3900f686c3f8) ) // OK - IC5
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic23",    0x1200000, 0x0200000, CRC(e3d9d12b) SHA1(28ec3727774ef8a6a241238ad134a5adab8327fd) ) // OK - IC5
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic25",    0x1400000, 0x0200000, CRC(71238374) SHA1(0dc534628a98aba508bdef58f8b812908414ae48) ) // OK - IC6
	ROM_LOAD16_WORD_SWAP( "prc297wi_ic27",    0x1600000, 0x0200000, CRC(7485a9a2) SHA1(17999a5192f185a27c08c2f05e19c65977b8f84e) ) // OK - IC6

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(9ba58358) SHA1(555ac21321b3051f7083cd72176ddc0fef2d4155) )
ROM_END


ROM_START( prc298sp ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "prc298sp_ic22",    0x0200000, 0x0200000, CRC(cb0ec98a) SHA1(efef536cb3bc71207936b26b87f04641baded10b) ) // OK? - tested as IC7?
	ROM_LOAD16_WORD_SWAP( "prc298sp_ic24",    0x0400000, 0x0200000, CRC(645e7e24) SHA1(7362b0c4b500639c20ec27002f543a0b4390eaa8) ) // OK - tested as IC2
	ROM_LOAD16_WORD_SWAP( "prc298sp_ic26",    0x0600000, 0x0200000, CRC(9d3ad85d) SHA1(71fe330594ab58be331aa5311472855be07cb44c) ) // OK - tested as IC2
	ROM_LOAD16_WORD_SWAP( "prc298sp_ic28",    0x0800000, 0x0200000, CRC(877e73cc) SHA1(dd9928a3fe0ed759611e1b7be8ea10b45084e392) ) // OK - tested as IC3
	ROM_LOAD16_WORD_SWAP( "prc298sp_ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK - tested as IC3
	ROM_LOAD16_WORD_SWAP( "prc298sp_ic32",    0x0c00000, 0x0200000, CRC(62c10626) SHA1(58cb0ca0330fa7a62b277ab0ff84bff65b81bb23) ) // OK - tested as IC4
	ROM_LOAD16_WORD_SWAP( "prc298sp_ic34",    0x0e00000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 fill. OK - tested as IC4
	ROM_LOAD16_WORD_SWAP( "prc298sp_ic36",    0x1000000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 fill, OK - tested as IC5

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "prc298sp.nv", 0x0000, 0x0080, CRC(a23dd0f2) SHA1(457282b5d40a17477b95330bba91e05c603f951e) )
ROM_END

ROM_START( prc298su ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclb298s_ic22",    0x0200000, 0x0200000, CRC(9720fe7a) SHA1(5b17eee0bd4574c0b2eb5bb64928d37bd87da23f) ) // OK - tested as IC7
	ROM_LOAD16_WORD_SWAP( "pclb298s_ic24",    0x0400000, 0x0200000, CRC(380496dc) SHA1(60a000cd71553cd23009ffd41b0208153b18d858) ) // OK - tested as IC2
	ROM_LOAD16_WORD_SWAP( "pclb298s_ic26",    0x0600000, 0x0200000, CRC(42622126) SHA1(cc312b1be51e919013ce55d4c1242a90676157e0) ) // OK - tested as IC2
	ROM_LOAD16_WORD_SWAP( "pclb298s_ic28",    0x0800000, 0x0200000, CRC(c03e861a) SHA1(e39e1d040651577d088ce88d354c74e96971efaa) ) // OK - tested as IC3
	ROM_LOAD16_WORD_SWAP( "pclb298s_ic30",    0x0a00000, 0x0200000, CRC(01844b12) SHA1(92d23e54cdfba8c0bdf3d87c52313334e2f903fa) ) // OK - tested as IC3

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "prc298su.nv", 0x0000, 0x0080, CRC(6b81636a) SHA1(c84de7c374c46f92985186834ab023986e2abbd8) )
ROM_END


ROM_START( pclub26w ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclbvol6w_ic22",    0x0200000, 0x0200000, CRC(72aa320c) SHA1(09bc30e8cb00a5a4014c44e468cc64f6c3425d92) )
	ROM_LOAD16_WORD_SWAP( "pclbvol6w_ic24",    0x0400000, 0x0200000, CRC(d98371e2) SHA1(813ac5f3c5b57d07cc319c73560bc0719ddcfe6b) )
	ROM_LOAD16_WORD_SWAP( "pclbvol6w_ic26",    0x0600000, 0x0200000, CRC(e6bbe3a5) SHA1(b2f642b8ca0779ad66cfbbadece40f4e3dc41fd1) )
	ROM_LOAD16_WORD_SWAP( "pclbvol6w_ic28",    0x0800000, 0x0200000, CRC(3c330c9b) SHA1(92f8e8d4f43db7c4ce431d17501492a7f8d8a867) )
	ROM_LOAD16_WORD_SWAP( "pclbvol6w_ic30",    0x0a00000, 0x0200000, CRC(67646090) SHA1(ed6402a22acafa0203c587b871edc547f0ec5277) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub26w.nv", 0x0000, 0x0080, CRC(448f770d) SHA1(5f966c511c4c8e9d5b2d257c41c2c88a453b4944) )
ROM_END

ROM_START( pclub26wa ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22.bin",    0x0200000, 0x0200000, CRC(a88e117d) SHA1(afbc862c558f710b2a87e1326c91881996008055) )
	ROM_LOAD16_WORD_SWAP( "ic24.bin",    0x0400000, 0x0200000, CRC(d98371e2) SHA1(813ac5f3c5b57d07cc319c73560bc0719ddcfe6b) )
	ROM_LOAD16_WORD_SWAP( "ic26.bin",    0x0600000, 0x0200000, CRC(d0412c8d) SHA1(4cb3c0ed3176bf86dee00649678a8e868819e92e) ) // this is actually blank after 0x199404 but otherwise matches the rom in the parent set, the rom check expects this and fails with the rom from the parent set.
	ROM_LOAD16_WORD_SWAP( "ic28.bin",    0x0800000, 0x0200000, CRC(3c330c9b) SHA1(92f8e8d4f43db7c4ce431d17501492a7f8d8a867) )
	ROM_LOAD16_WORD_SWAP( "ic30.bin",    0x0a00000, 0x0200000, CRC(00a0c702) SHA1(f2c4a7a51559f0ade96b8e6337cd1a1d61472de7) ) // same as the Warner Bros and Print Club Kome Kome Club versions?!

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub26w.nv", 0x0000, 0x0080, CRC(448f770d) SHA1(5f966c511c4c8e9d5b2d257c41c2c88a453b4944) )
ROM_END

ROM_START( pclub27s ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclub2v7.ic22",    0x0200000, 0x0200000, CRC(44c8ab27) SHA1(65e2705b2918da32ea40375707df4e148b311159) )
	ROM_LOAD16_WORD_SWAP( "pclub2v7.ic24",    0x0400000, 0x0200000, CRC(24818437) SHA1(5293d45b53680301abaf0b32a62596aaaa2552d6) )
	ROM_LOAD16_WORD_SWAP( "pclub2v7.ic26",    0x0600000, 0x0200000, CRC(076c1d44) SHA1(d597ed4524bb03eb0ef8ada08d49f3dc0fc8136d) )
	ROM_LOAD16_WORD_SWAP( "pclub2v7.ic28",    0x0800000, 0x0200000, CRC(ff9643ca) SHA1(3309f970f87324b06cc48add386019f769abcd89) )
	ROM_LOAD16_WORD_SWAP( "pclub2v7.ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub27s.nv", 0x0000, 0x0080, CRC(e58c7167) SHA1(d88b1648c5d86a90615a8c6a1bf87bc9e75dc320) )
ROM_END

ROM_START( prc28su ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "u22.bin",    0x0200000, 0x0200000, CRC(b78cf122) SHA1(10ecdbb04d1ce2c7f98e57260bbd6a09b8a5905d) ) // OK (tested as IC7)
	ROM_LOAD16_WORD_SWAP( "u24.bin",    0x0400000, 0x0200000, CRC(aca05d29) SHA1(68c487821a912aea867f2f5041f5d926f31e4513) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "u26.bin",    0x0600000, 0x0200000, CRC(5591f6e2) SHA1(95a28b0a4fc2aeb063902505a4f3613c33f9059a) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "u28.bin",    0x0800000, 0x0200000, CRC(0899889b) SHA1(e7a77350a2421bbd867681f4c256d3b0d473468d) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "u30.bin",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "u32.bin",    0x0c00000, 0x0200000, CRC(5dc1f4d7) SHA1(dbb04d4c45b3ecf5416f32e34d3cabd4945352fb) ) // OK
	ROM_LOAD16_WORD_SWAP( "u34.bin",    0x0e00000, 0x0200000, CRC(0222734a) SHA1(d65311234100f81aa40a6738f79c61e598da8665) ) // OK
	ROM_LOAD16_WORD_SWAP( "u36.bin",    0x1000000, 0x0200000, CRC(57c30e0f) SHA1(2286ba237edecc0cab1539b30b5f36843c7e276d) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(447bb3bd) SHA1(9fefec09849bfa0c14b49e73ff13e2a538dff511) )
ROM_END

// this is the first one to offer visual display of the frames in the Frame Count test menu?
ROM_START( prc29au ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22.bin",    0x0200000, 0x0200000, CRC(b7a9bfa4) SHA1(d756eb1232fb121ccce6d74feb86becc95335e11) ) // OK (tested as IC7)
	ROM_LOAD16_WORD_SWAP( "ic24.bin",    0x0400000, 0x0200000, BAD_DUMP CRC(91f36785) SHA1(1cdbb9236e119be37ad782b03e51ba401d0a1dd1) ) // BAD? (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "ic26.bin",    0x0600000, 0x0200000, BAD_DUMP CRC(4c1b8823) SHA1(8b9f79da973e0331c04da3e776e922aaf2c0e0b7) ) // BAD? (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "ic28.bin",    0x0800000, 0x0200000, CRC(594d200f) SHA1(9e490b4189348bfde44098dcfbcefb7f070f51fe) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "ic30.bin",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK (tested as IC3)
// these aren't tested by the ROM check but can be viewed as 'frame set 2' in the Each Game Test -> Frame Count -> Frame Count Display menu
// they match the ones found on pckobe99 except for the final rom, which might be a better dump of the one in the pckobe99 romset
	ROM_LOAD16_WORD_SWAP( "ic32.bin",    0x0c00000, 0x0200000, CRC(76f6efaa) SHA1(d861d178a37adbde6b86df84231b34b89c5fc1d8) ) // not tested?!
	ROM_LOAD16_WORD_SWAP( "ic34.bin",    0x0e00000, 0x0200000, CRC(894c63f9) SHA1(9956fd512cb715780616c4262d2b05474e3337ff) ) // not tested?!
	ROM_LOAD16_WORD_SWAP( "ic36.bin",    0x1000000, 0x0200000, CRC(524a1c4e) SHA1(182dcc9d4808595ec67b4f80a5bf6d25016bf2a7) ) // not tested?!

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(447bb3bd) SHA1(9fefec09849bfa0c14b49e73ff13e2a538dff511) )
ROM_END



ROM_START( prc2ksu ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	// NOTE: Game fails IC7 check but rom read consistently, might be okay, might not be - supposedly ST-V print carts are known to have bad ROM tests(?)
	ROM_LOAD16_WORD_SWAP( "ic22.bin",    0x0200000, 0x0200000, CRC(4b3de7df) SHA1(869c3840ac2eab263bb8b79ba1430e5789fa5758) ) // 'BAD' (tested as IC7) (but maybe OK)
	ROM_LOAD16_WORD_SWAP( "ic24.bin",    0x0400000, 0x0200000, CRC(02da49b7) SHA1(dd19bfd6c21f432b3af011ce43ba38d295c06c6d) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "ic26.bin",    0x0600000, 0x0200000, CRC(a431d614) SHA1(75de91d8eff5af7e1b668012a0613884ba660e21) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "ic28.bin",    0x0800000, 0x0200000, CRC(c0fba1a5) SHA1(5f98be1eed5f74e62fe5a3e33fdcf3b827dda1a7) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "ic30.bin",    0x0a00000, 0x0200000, CRC(0811d0e4) SHA1(b4fd9369c80f76141ae2fb38525b405b4c2f391a) ) // OK (tested as IC3)

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 ) // preconfigured to 1 player
	ROM_LOAD( "prc2ksu.nv", 0x0000, 0x0080, CRC(ee7ffdc5) SHA1(4008e37cae306c0202146c5dd79ca925b8d8edd5) )
ROM_END


ROM_START( pclub2pe ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclb2psi_ic22",    0x0200000, 0x0200000, CRC(caadc660) SHA1(f2e84bee96266bb03d8f9009249c17c27935f82e) )
	ROM_LOAD16_WORD_SWAP( "pclb2psi_ic24",    0x0400000, 0x0200000, CRC(ece82698) SHA1(b17b1ea8adc13c3722067c9854d1b7fdf3917090) )
	ROM_LOAD16_WORD_SWAP( "pclb2psi_ic26",    0x0600000, 0x0200000, CRC(c8a1e335) SHA1(a95ddfc41fdd9f720c11208f45ef5db4bee6cb97) )
	ROM_LOAD16_WORD_SWAP( "pclb2psi_ic28",    0x0800000, 0x0200000, CRC(52f09627) SHA1(e2ffc321bb0f2a650d0c0b39c3ec68226e1ca7f4) )
	ROM_LOAD16_WORD_SWAP( "pclb2psi_ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2pe.nv", 0x0000, 0x0080, CRC(447bb3bd) SHA1(9fefec09849bfa0c14b49e73ff13e2a538dff511))
ROM_END

ROM_START( pclub2wb ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclb2wb_ic22",    0x0200000, 0x0200000, CRC(12245be7) SHA1(4d6c2c9ca7fe73a9ec490157cdb01a6228dee7f8) )
	ROM_LOAD16_WORD_SWAP( "pclb2wb_ic24",    0x0400000, 0x0200000, CRC(e5d6e11e) SHA1(4af3c646747f76d99482c985f960df2519a85c23) )
	ROM_LOAD16_WORD_SWAP( "pclb2wb_ic26",    0x0600000, 0x0200000, CRC(7ee066f0) SHA1(a7c725ce8e621ed299474dd215174699e097db3f) )
	ROM_LOAD16_WORD_SWAP( "pclb2wb_ic28",    0x0800000, 0x0200000, CRC(9ed59513) SHA1(c8f5ed13be2a91f83c35a7929aaa5751d7843e6e) )
	ROM_LOAD16_WORD_SWAP( "pclb2wb_ic30",    0x0a00000, 0x0200000, CRC(00a0c702) SHA1(f2c4a7a51559f0ade96b8e6337cd1a1d61472de7) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2wb.nv", 0x0000, 0x0080, CRC(0d442eec) SHA1(54dd544e1496e3999d8111eb06abf805b610d77d) )
ROM_END



ROM_START( pclubyo2 ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclbyov2.ic22",    0x0200000, 0x0200000, CRC(719a4d27) SHA1(328dfb8debea02e8660e636e953982d381529945) )
	ROM_LOAD16_WORD_SWAP( "pclbyov2.ic24",    0x0400000, 0x0200000, CRC(790dc7b5) SHA1(829ead39930779617a9bef41d8615362ca86c4c7) )
	ROM_LOAD16_WORD_SWAP( "pclbyov2.ic26",    0x0600000, 0x0200000, CRC(12ae1606) SHA1(9534fb2dbf6fd2c258ba2716783cc5bab8bd8dc0) )
	ROM_LOAD16_WORD_SWAP( "pclbyov2.ic28",    0x0800000, 0x0200000, CRC(ff9643ca) SHA1(3309f970f87324b06cc48add386019f769abcd89) )
	ROM_LOAD16_WORD_SWAP( "pclbyov2.ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclubyo2.nv", 0x0000, 0x0080, CRC(2b26a8f7) SHA1(32f34096cac05a37c492ee389ed8e4c02694c268) )
ROM_END


ROM_START( prc298au ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "prc298au_ic22",    0x0200000, 0x0200000, CRC(21a995ce) SHA1(6ee1250becd76bef3aa8044a42e10c3830a609bd) ) // OK
	ROM_LOAD16_WORD_SWAP( "prc298au_ic24",    0x0400000, 0x0200000, CRC(94540f39) SHA1(cee9fff48d177e7502802d366339ed922c212871) ) // OK
	ROM_LOAD16_WORD_SWAP( "prc298au_ic26",    0x0600000, 0x0200000, CRC(8b22c41f) SHA1(371f8b35ed45f695f5ec0c8db2c4b62007bf4782) ) // OK
	ROM_LOAD16_WORD_SWAP( "prc298au_ic28",    0x0800000, 0x0200000, CRC(bf68cec0) SHA1(550138f5110661d69eaff44c0596914a2621c3df) ) // OK
	ROM_LOAD16_WORD_SWAP( "prc298au_ic30",    0x0a00000, 0x0200000, CRC(ae276c06) SHA1(98358860ae9bf7c405ba4f763c7a4bf309ce85e3) ) // OK
	ROM_LOAD16_WORD_SWAP( "prc298au_ic32",    0x0c00000, 0x0200000, CRC(a3fb81f5) SHA1(6c78c97635dd486d2a7c09bc0511267eae6082c4) ) // OK
	ROM_LOAD16_WORD_SWAP( "prc298au_ic34",    0x0e00000, 0x0200000, CRC(04200dc9) SHA1(e40b01d12ccf71e50da7fd0f3000158626e5a98d) ) // OK
	ROM_LOAD16_WORD_SWAP( "prc298au_ic36",    0x1000000, 0x0200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) // (blank! - not tested)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "prc298au.nv", 0x0000, 0x0080, CRC(b4440ff0) SHA1(bd3c83221ede11c68163df4b52a85856c83f865f) )
ROM_END


ROM_START( pclubor ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclubor_ic22",    0x0200000, 0x0200000, CRC(b25072f7) SHA1(baa674e3d277bee152773b3b0e522677bfedd65c) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubor_ic24",    0x0400000, 0x0200000, CRC(a4863a0e) SHA1(b092b638b6a8d06776f88aac7e47fff06b4e8221) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubor_ic26",    0x0600000, 0x0200000, CRC(7f55baf7) SHA1(344f08590484a5ece3a64fe3e2e84a7d386fb8c1) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubor_ic28",    0x0800000, 0x0200000, CRC(cafd2a7d) SHA1(ff5f6ac7f5745d21db4a9c80eaec6f23d6084f23) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubor_ic30",    0x0a00000, 0x0200000, CRC(ccf6f885) SHA1(a01279a06cbc0c724c5c03b985f979458a2a2e65) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubor_ic32",    0x0c00000, 0x0200000, CRC(62e6d1e1) SHA1(5ea2ed73c5c02efbc85cd047983a02947ae829ff) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubor_ic34",    0x0e00000, 0x0200000, CRC(19cdd167) SHA1(a474134f58101dbbff3a47001ca26a69b03ce830) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubor_ic36",    0x1000000, 0x0200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) // (blank! - not tested)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclubor.nv", 0x0000, 0x0080, CRC(3ad918c0) SHA1(70b63d78948e2fc26fd95f98b93ed86c6130ed8e) )
ROM_END



ROM_START( pclubol ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclubol_ic22",    0x0200000, 0x0200000, CRC(668b1049) SHA1(a88ab4fbab73f85eb4da8d4792e98fbbefeba5f8) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubol_ic24",    0x0400000, 0x0200000, CRC(35721f04) SHA1(9658c9526a3d1dde89e1b6fd986b3469011813ca) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubol_ic26",    0x0600000, 0x0200000, CRC(17b3cc4b) SHA1(226eba06dd7db14fd3d10a1ca687293f2c93db9d) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubol_ic28",    0x0800000, 0x0200000, CRC(7cfaa530) SHA1(c04742acb692a028021188deea4617c9955db38c) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubol_ic30",    0x0a00000, 0x0200000, CRC(e1dd7854) SHA1(190b85f4e25d5b98ea893033004a6ee698ade8c4) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubol_ic32",    0x0c00000, 0x0200000, CRC(f0a3ded7) SHA1(574417b9fb234ea75893dd22c65d2a3c4e3e4aa1) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubol_ic34",    0x0e00000, 0x0200000, CRC(53aa9821) SHA1(b866e0255297d48fbcab93bc044e198bdff8c6d3) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubol_ic36",    0x1000000, 0x0200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) // (blank! - not tested)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(a744ca03) SHA1(f82ca58469356fad8c0fc53d54e7a4714ba43dd3) )
ROM_END


ROM_START( pclub2v3 ) // set to 1p / runs with the USA bios
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclub2v3_ic22",    0x0200000, 0x0200000, BAD_DUMP CRC(f88347aa) SHA1(3e9ca105edbd6ce11ea4194eb1733785e87f92b2) ) // BAD
	ROM_LOAD16_WORD_SWAP( "pclub2v3_ic24",    0x0400000, 0x0200000, CRC(b5871198) SHA1(10d187eebcca5d70c5ae10d1a144685a96491126) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2v3_ic26",    0x0600000, 0x0200000, CRC(d97034ed) SHA1(a7a0f659eefd539b2a1fd70ef394eed30ea54c0c) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2v3_ic28",    0x0800000, 0x0200000, CRC(f1421506) SHA1(c384b695338144e5f051134bda73b059b678a7df) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclub2v3_ic30",    0x0a00000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // 00 Fill, OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2v3.nv", 0x0000, 0x0080, CRC(a8a2d30c) SHA1(bdde3d62ff21190a23698058ff66e476a75a09aa) )
ROM_END

ROM_START( pclubpok ) // set to 1p / runs with the USA bios
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclubpok_ic22",    0x0200000, 0x0200000, CRC(48ab8371) SHA1(1c2124afad6bc1f4de2619e6b915f78e91addf05) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubpok_ic24",    0x0400000, 0x0200000, CRC(9915faea) SHA1(b96f64a8cbb1b9496bb566d0469975fddb4fbe98) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubpok_ic26",    0x0600000, 0x0200000, CRC(054ad120) SHA1(987e812af099bb0aa5d43a4c10ae8345370a2606) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubpok_ic28",    0x0800000, 0x0200000, CRC(3a654b2a) SHA1(7398e25836bfbdeab6350759f25c420c3b496172) ) // OK
	ROM_LOAD16_WORD_SWAP( "pclubpok_ic30",    0x0a00000, 0x0200000, CRC(98747bef) SHA1(8d452507a9842a48cd1a7db8fde11a070a6f068b) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclubpok.nv", 0x0000, 0x0080, CRC(4ba3f21a) SHA1(898a393fb2fc1961b68b7d4f383d5447e6c010e8) )
ROM_END

ROM_START( pclub2kc ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclub2kc_ic22",    0x0200000, 0x0200000, CRC(9eb4cfd7) SHA1(d74b00541419b5cde75409315fdc620a7cb3ac6b) ) // OK (tested as IC7)
	ROM_LOAD16_WORD_SWAP( "pclub2kc_ic24",    0x0400000, 0x0200000, CRC(cf3b4080) SHA1(1963a45d391f8d591d3995231b26eba56c7896c6) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "pclub2kc_ic26",    0x0600000, 0x0200000, CRC(dbdcb1d7) SHA1(33bd8c9e351864cdd5f5be2aba62efda1c4a0f4f) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "pclub2kc_ic28",    0x0800000, 0x0200000, CRC(3c330c9b) SHA1(92f8e8d4f43db7c4ce431d17501492a7f8d8a867) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "pclub2kc_ic30",    0x0a00000, 0x0200000, CRC(00a0c702) SHA1(f2c4a7a51559f0ade96b8e6337cd1a1d61472de7) ) // OK (tested as IC3)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2kc.nv", 0x0000, 0x0080, CRC(064366fe) SHA1(b85489ae19ddc0fbd67b441a6967f10a6cd22d45) )
ROM_END

ROM_START( pclb2elk ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "pclb2elk_ic22",    0x0200000, 0x0200000, CRC(2faed82a) SHA1(035ef25dd974679e46a79e408ee284eca0310557) ) // OK (tested as IC7)
	ROM_LOAD16_WORD_SWAP( "pclb2elk_ic24",    0x0400000, 0x0200000, CRC(9cacfb7b) SHA1(1c68e1ba077e02ded0f388b4e9ad24998a5d8a48) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "pclb2elk_ic26",    0x0600000, 0x0200000, CRC(533a189e) SHA1(23a08a9ab02b21d6c75c770f4adc33c6dfe98a4e) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "pclb2elk_ic28",    0x0800000, 0x0200000, CRC(1f0c9113) SHA1(b65fc9012cb159c374674cde1fd2fb2cf192e7c5) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "pclb2elk_ic30",    0x0a00000, 0x0200000, CRC(0e188b8c) SHA1(8574f884fbeb2f428913c24e147ee8753305ce86) ) // OK (tested as IC3)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclb2elk.nv", 0x0000, 0x0080, CRC(54c7564f) SHA1(574dcc5e8fe4aac091fee1476347485ed660eddd) )
ROM_END

ROM_START( pckobe99 ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	// all ROM tests fail both in MAME and on real hardware, but ROMs read consistently.
	// there do seem to be bad values on some 0x40000 boundaries in places tho (obvious if you look at the blank fill area of ic36 for instance)
	// as a result I'm marking them as bad, we'll need a 2nd cartridge to verify tho.
	ROM_LOAD16_WORD_SWAP( "ic22.bin",    0x0200000, 0x0200000, BAD_DUMP CRC(670296eb) SHA1(18055dddf59edbf6d6a97bc81ba332e681f460ba) ) //
	ROM_LOAD16_WORD_SWAP( "ic24.bin",    0x0400000, 0x0200000, BAD_DUMP CRC(c2139f62) SHA1(9e7060d77571349b13e58a845fde0f75c29a1bac) ) //
	ROM_LOAD16_WORD_SWAP( "ic26.bin",    0x0600000, 0x0200000, BAD_DUMP CRC(17e3efd0) SHA1(f004bb27c1708ed5a78426aa26a51496470c173d) ) //
	ROM_LOAD16_WORD_SWAP( "ic28.bin",    0x0800000, 0x0200000, BAD_DUMP CRC(a52f99f6) SHA1(9d23f63a9515b468c93320458d12f54850e9e121) ) //
	ROM_LOAD16_WORD_SWAP( "ic30.bin",    0x0a00000, 0x0200000, BAD_DUMP CRC(f1b7e3d5) SHA1(2e1f881c0abe8bfb168478b3cbbd724c74c374c0) ) //
	ROM_LOAD16_WORD_SWAP( "ic32.bin",    0x0c00000, 0x0200000, BAD_DUMP CRC(76f6efaa) SHA1(d861d178a37adbde6b86df84231b34b89c5fc1d8) ) // not tested
	ROM_LOAD16_WORD_SWAP( "ic34.bin",    0x0e00000, 0x0200000, BAD_DUMP CRC(894c63f9) SHA1(9956fd512cb715780616c4262d2b05474e3337ff) ) // not tested
	ROM_LOAD16_WORD_SWAP( "ic36.bin",    0x1000000, 0x0200000, BAD_DUMP CRC(078694c3) SHA1(a37eb118338c0fc07aa75705dc6a7a1b866cb081) ) // not tested 2ND HALF = xx00

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(dbe305a9) SHA1(4b738104db8345385db3f622672dcc536bbfe67e) )
ROM_END



ROM_START( pclove )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	// note, 'IC2' in service mode (the test of IC24/IC26) fails once you map the protection device because it occupies the same memory address as the rom at IC26
	// this sometimes causes it to fail on real hardware too(!)
	ROM_LOAD16_WORD_SWAP( "pclblove.ic22",    0x0200000, 0x0200000, CRC(8cd25a0f) SHA1(c938d5f4f800db019abc2e17cce1e780e93f3d02) ) // OK (tested as IC7)
	ROM_LOAD16_WORD_SWAP( "pclblove.ic24",    0x0400000, 0x0200000, CRC(85583e2c) SHA1(7f407d1bce40317fc10433dafcd82ee41be05839) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "pclblove.ic26",    0x0600000, 0x0200000, CRC(7efcabcc) SHA1(b99a67ab2053c3be5ce37530b65f9693c2a4eef8) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "pclblove.ic28",    0x0800000, 0x0200000, CRC(a1336da7) SHA1(ba26810067a13968a54a8867025b8d8e96384ae7) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "pclblove.ic30",    0x0a00000, 0x0200000, CRC(ec5b5e28) SHA1(89bcddb52c176c86ad4bdb9f4f052be5b75bcd1b) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "pclblove.ic32",    0x0c00000, 0x0200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) // FF fill? (not tested either)

	// protection device used to decrypt some startup code

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclove.nv", 0x0000, 0x0080, CRC(3c78e3bd) SHA1(6d5fe8545f434b4cc1e8229549adb0a49ac45bd1) )
ROM_END

ROM_START( pclove2 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	// note, 'IC2' in service mode (the test of IC24/IC26) fails once you map the protection device because it occupies the same memory address as the rom at IC26
	// this sometimes causes it to fail on real hardware too(!)
	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(d7d968d6) SHA1(59916a453ba8a53af2138272e359c6d6ce11ea8c) ) // OK (tested as IC7)
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(9c9b7e57) SHA1(ae834a3648126ec2456d2cc5544f81b6dc2f5825) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(55eb859f) SHA1(4f25536787142f965d688d1758a45885b52ae52e) ) // OK (tested as IC2)
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(463604a6) SHA1(d8eb41676c750e01870241361ef04c8f22a0c4b4) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(ec5b5e28) SHA1(89bcddb52c176c86ad4bdb9f4f052be5b75bcd1b) ) // OK (tested as IC3)
	ROM_LOAD16_WORD_SWAP( "ic32",    0x0c00000, 0x0200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) // FF fill? (not tested either)

	// protection device used to decrypt some startup code

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclove2.nv", 0x0000, 0x0080, CRC(93b30600) SHA1(eadba12ec322911823a6873a343e4d9b4089ed93) )
ROM_END

ROM_START( pcpooh2 ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	// note, 'IC2' in service mode (the test of IC24/IC26) fails once you map the protection device because it occupies the same memory address as the rom at IC26
	// this sometimes causes it to fail on real hardware too(!)
	ROM_LOAD16_WORD_SWAP( "ic22.bin",    0x0200000, 0x0200000, CRC(2cb33332) SHA1(7c05035358c08b6327d9f0581bf7036e95001eae) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24.bin",    0x0400000, 0x0200000, CRC(3c6fc10f) SHA1(8edb4c583be7270c1a8c155663f47554064f5213) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26.bin",    0x0600000, 0x0200000, CRC(b891c7ab) SHA1(7948ac76cde2851dcba3752e84e1fc35affe0bc4) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28.bin",    0x0800000, 0x0200000, CRC(1a1c74cb) SHA1(0a35e5a5ccf42bda641d56cce2fd0d02953f3878) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30.bin",    0x0a00000, 0x0200000, CRC(b7b6fc61) SHA1(bb02b321d3e2ff120f1b84f008507f0cf6c36768) ) // OK

	// protection device is 317-0230

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(5aee29d0) SHA1(a1dd443cf2dd9d52ca45ea687eff2f88cae25b85) )
ROM_END

ROM_START( pcpooh3 ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	// note, 'IC2' in service mode (the test of IC24/IC26) fails once you map the protection device because it occupies the same memory address as the rom at IC26
	// this sometimes causes it to fail on real hardware too(!)
	ROM_LOAD16_WORD_SWAP( "ic22.bin",    0x0200000, 0x0200000, CRC(efc83c4f) SHA1(8cabb88d8d52beddcde6a62858f4666092ed179e) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24.bin",    0x0400000, 0x0200000, CRC(e8753d0d) SHA1(571fe30075ad63dc395402cfb1d4734b54017e77) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26.bin",    0x0600000, 0x0200000, CRC(daf1a0d4) SHA1(474fe24fb95ad3065a66e664f362692d86a8d563) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28.bin",    0x0800000, 0x0200000, CRC(3c0f040f) SHA1(18d3917a1c1f49a2b45b13b92ba7da37341d3d1f) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30.bin",    0x0a00000, 0x0200000, CRC(6006d785) SHA1(b6f4d1c26288c5ae48ad5ec785512a4b0fd3f8ef) ) // OK

	// protection device is 317-0230

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(e41d541b) SHA1(511fe00745787ee5dbd813125ddc3db921d6531e) )
ROM_END



// Name Club / Name Club vol.2
// have an unusual rom mapping compared to other games, the cartridge is a little different too, with a large PALCE16V8H-10 marked 315-6026
// For Name Club vol. 2, the protection device (317-0229 on both) is checked in the 'each game test' menu as 'RCDD2'
// For the service mode test the game just passes a large block of compressed data and checksums the result, it doesn't even look like it's
// passing 100% valid data, just an entire section of ROM, checking the result against a pre-calculated checksum.

// The device is accessed by the game when you choose to print, it looks like it's decompressing the full-size graphics for the printer rather
// than anything you see onscreen.  It makes quite extensive use of the device, with lots of different dictionaries, unlike Decathlete where
// there are only 2 that cover all the data.

ROM_START( nameclub )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(ac23c648) SHA1(4dd099a92ff162082eb24a61a277ca907b3f9892) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0600000, 0x0200000, CRC(a16902e3) SHA1(85c582cb0d02ef028a8ae32688c20a5b5aeeaae8) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0a00000, 0x0200000, CRC(a5eab3f3) SHA1(1b7263639bb8f4aa644cc46133988ef4d2b6c9de) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0e00000, 0x0200000, CRC(34ed677a) SHA1(ff2c4dd8fae33ac618f6e3e28ba71c4ecb4ca88f) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "nameclub.nv", 0x0000, 0x0080, CRC(680a64bc) SHA1(45194bbe4a7e67f0e44f858589881967884f63a6) )
ROM_END

ROM_START( nclubv2 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "nclubv2.ic22",    0x0200000, 0x0200000, CRC(7e81676d) SHA1(fc0f0dcdb4aaf71218d7c1dd0e4ddc5381e8b13b) ) // OK
	ROM_LOAD16_WORD_SWAP( "nclubv2.ic24",    0x0600000, 0x0200000, CRC(1b7637de) SHA1(43c3094f60a6582298a45bad923fef57e98c5b2b) ) // OK
	ROM_LOAD16_WORD_SWAP( "nclubv2.ic26",    0x0a00000, 0x0200000, CRC(630be99d) SHA1(ac7fbaae98b126fad5228b0ebffa91a0f0a94516) ) // OK
	ROM_LOAD16_WORD_SWAP( "nclubv2.ic28",    0x0e00000, 0x0200000, CRC(1a3ca5e2) SHA1(4d3aed51d29c54e71175d828f648c9feb813ac04) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "nclubv2.nv", 0x0000, 0x0080, CRC(96d55fa9) SHA1(b3c821d6cd4ed52d0e20565e12a06d8f81a08dbc) )
ROM_END

ROM_START( patocar )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22.bin",     0x0200000, 0x200000, CRC(b7e6d425) SHA1(b7496cb390fe50ee786815082415427056e4e3e1) )
	ROM_LOAD16_WORD_SWAP( "ic24.bin",     0x0400000, 0x200000, CRC(cbbb687e) SHA1(cfc87ae6124f9978bb2432b98d77f0da07d020b7) )
	ROM_LOAD16_WORD_SWAP( "ic26.bin",     0x0600000, 0x200000, CRC(91db9dbe) SHA1(8652fe45ce56633016403c75e8b3a7b77f279819) )
	ROM_LOAD16_WORD_SWAP( "ic28.bin",     0x0800000, 0x200000, CRC(bff0cd9c) SHA1(3c62aa2d7f71bd6fb147fdcd8d99cd7815f3047e) )
	ROM_LOAD16_WORD_SWAP( "ic30.bin",     0x0a00000, 0x200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )   // empty / FF filled

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "patocar.nv", 0x0000, 0x0080, CRC(d9873ee8) SHA1(e74747816bba6745afd718b0beec67a884c6a31c) )
ROM_END

ROM_START( sackids )
//  STV_BIOS

	// wants it's own specific bios, marked "CKBP1.13J0001024" at 0x800
	// PC=06004150 is where it compares this
	// it also looks like it has a specific I/O board for the lightpen
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "epr-20091.ic8",   0x000000, 0x080000, BAD_DUMP CRC(59ed40f4) SHA1(eff0f54c70bce05ff3a289bf30b1027e1c8cd117) )

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22.bin",     0x0200000, 0x200000, CRC(4d9d1870) SHA1(c702964af2767b0db4ca1d6c7d07356e675d5efd) )
	ROM_LOAD16_WORD_SWAP( "ic24.bin",     0x0400000, 0x200000, CRC(39fca3e5) SHA1(29be552f58b69f8f3f237ca14f13af3673559123) )
	ROM_LOAD16_WORD_SWAP( "ic26.bin",     0x0600000, 0x200000, CRC(f38c79b6) SHA1(a470a22ef3d735c9929f70ef5441547a07a480e8) )
	ROM_LOAD16_WORD_SWAP( "ic28.bin",     0x0800000, 0x200000, CRC(63d09f3c) SHA1(e470e5af52f9ee70bf160ff58a5cbafd7e674073) )
	ROM_LOAD16_WORD_SWAP( "ic30.bin",     0x0a00000, 0x200000, CRC(f89811ba) SHA1(8fa8b4b09430456bce63e45686640c7bcdde90e9) )
	ROM_LOAD16_WORD_SWAP( "ic32.bin",     0x0c00000, 0x200000, CRC(1db6c26b) SHA1(2e14b7b021bce145f989295fdc6effcd799f00a4) )
	ROM_LOAD16_WORD_SWAP( "ic34.bin",     0x0e00000, 0x200000, CRC(0f3622c8) SHA1(69337114d6902675018371101f0fba01902de54a) )
	ROM_LOAD16_WORD_SWAP( "ic36.bin",     0x1000000, 0x200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) // empty / FF filled

	ROM_REGION32_BE( 0x3000000, "abus", ROMREGION_ERASE00 ) /* SH2 code */
ROM_END

ROM_START( dfeverg )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) // SH2 code
	ROM_LOAD16_WORD_SWAP( "13",    0x0000000, 0x080000, CRC(ecd7ac4b) SHA1(9bb4ae5ac236192bb6e927a3b5b75f84264e36aa) )
	ROM_LOAD16_WORD_SWAP( "1",     0x0400000, 0x400000, CRC(d72f1640) SHA1(6a2ef0ca2b525d9ef0a817bb1d99275cf954242e) )
	ROM_LOAD16_WORD_SWAP( "2",     0x0800000, 0x400000, CRC(c2e8aee5) SHA1(f7e404be93f99c8206636dc3391f162fc83abe2f) )
	ROM_LOAD16_WORD_SWAP( "3",     0x0c00000, 0x400000, CRC(cb5b2744) SHA1(18d42bb6ced01e3e9fa1ed66c07bf91e34811510) )
	ROM_LOAD16_WORD_SWAP( "4",     0x1000000, 0x400000, CRC(7eca59b2) SHA1(e03d71e1e9a67dadb2b76dde87f01147afe9ce34) )
	ROM_LOAD16_WORD_SWAP( "5",     0x1400000, 0x400000, CRC(c3450f2b) SHA1(52dbfc8900d569debd92d2dd0d7fc25e133c5fbb) )
	ROM_LOAD16_WORD_SWAP( "6",     0x1800000, 0x400000, CRC(1ac57ed5) SHA1(a34fb91818b586813d5f7ff5885bc07211189751) )
	ROM_LOAD16_WORD_SWAP( "8",     0x1c00000, 0x400000, CRC(acc78f10) SHA1(9081d3006cc2f4daac1a8b99b729ac17bd6fed52) )
	ROM_LOAD16_WORD_SWAP( "9",     0x2000000, 0x400000, CRC(4ffbba8d) SHA1(526b5ba3b874c0ee7ce3fca7631bed1e0b84fea1) )
	ROM_LOAD16_WORD_SWAP( "10",    0x2400000, 0x400000, CRC(4b2a4397) SHA1(e8499e20939f26009f4678b85d3bfb5f02f4ddcb) )
	ROM_LOAD16_WORD_SWAP( "11",    0x2800000, 0x400000, CRC(58877b19) SHA1(fff50cc05ae39a3b8313756d606e27ae2e62af26) )
	// IC12 is tested but wasn't included in the dump. This causes missing backgrounds
	ROM_LOAD16_WORD_SWAP( "12",    0x2c00000, 0x400000, NO_DUMP )
ROM_END

ROM_START( skychal )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22.bin",     0x0200000, 0x200000, CRC(a12ccf64) SHA1(eb3ff0cdc10fa17e40af5bddedfa5f758c7a5623) )
	ROM_LOAD16_WORD_SWAP( "ic24.bin",     0x0400000, 0x200000, CRC(9a929dcf) SHA1(892e491fa33cc30cbbd24feb4ea6a63f9a9e1a62) )
	ROM_LOAD16_WORD_SWAP( "ic26.bin",     0x0600000, 0x200000, CRC(ed2183d3) SHA1(7c43171a57e070a295c191408bd1b8ffee053d9b) )
	ROM_LOAD16_WORD_SWAP( "ic28.bin",     0x0800000, 0x200000, CRC(e7401d68) SHA1(02f77439075b6367b46dc9ef7f67023b32a68526) )
	ROM_LOAD16_WORD_SWAP( "ic30.bin",     0x0a00000, 0x200000, CRC(950f7a2f) SHA1(1b4fd7b08eeb2fdf2cdfae7f8aa3240e8dce3b9a) )
	ROM_LOAD16_WORD_SWAP( "ic32.bin",     0x0c00000, 0x200000, CRC(a656212b) SHA1(b2bf325cb4cf67787c836a9ac1bb7231068ffd82) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "skychal.nv", 0x0000, 0x0080, CRC(a6515237) SHA1(5e50cc93eb60ed67cdca408b23b80d16a398df02) )
ROM_END

ROM_START( supgoal )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22.bin",     0x0200000, 0x200000, CRC(a686f7a2) SHA1(f4f9b63046d184864cfffe6ef8268a33b73298da) )
	ROM_LOAD16_WORD_SWAP( "ic24.bin",     0x0400000, 0x200000, CRC(56fbbeea) SHA1(2c622ccc20aed0df7c611a00986ce76b84c61d70) )
	ROM_LOAD16_WORD_SWAP( "ic26.bin",     0x0600000, 0x200000, CRC(64701c2b) SHA1(e9a426ce1882660d533b963899a8b1b6f41f85d4) )
	ROM_LOAD16_WORD_SWAP( "ic28.bin",     0x0800000, 0x200000, CRC(d9aebe8c) SHA1(cd19cf1227d151a015c4dc3aed14bc3b2f78ce07) )
	ROM_LOAD16_WORD_SWAP( "ic30.bin",     0x0a00000, 0x200000, CRC(26d4ade5) SHA1(6b958aa4db293c7af88a735323d19f6417d86048) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "supgoal.nv", 0x0000, 0x0080, CRC(63806aae) SHA1(b82f0995799e9259a1f071ea8b64a719f9e3c9e9) )
ROM_END

/*
Fantasy Zone ROM board
837-13556 REV.A
837-13247 (C) Sega
|----------------------------------------------------------|
|       ----CN3----                     ----CN4----        |
|                                                          |
| CN5 ADM202  IC11           IC10           IC9            |
|                                                          |
| 74F245N     IC12           IC8            IC7            |
|                                                          |
| 74F245N     IC13           IC6            IC5            |
|                                                          |
| J J J J J                  IC4            IC3            |
| P P P P P 315-5930 74F139N                               |
| 5 4 3 2 1                  IC2            IC1            |
| 74F245N 74F245N 74F245N                                  |
|        ----CN1----                    ----CN2----        |
|----------------------------------------------------------|
Notes:
 IC1-IC12 - MaskROMs (DIP42), 8Mb or 16Mb or 32Mb. Not all positions are populated
     IC13 - EPROM (DIP40), 27C4002
 315-5930 - GAL16V8D (DIP20)
  JP1-JP2 - JUMPERs, ROM size
                 JP2 JP1
             8M  2-3 2-3
             16M 2-3 1-2
             32M 1-2 1-2
  JP3-JP4 - JUMPERs
                    JP4 JP3
             MODE0  1-2 1-2
             MODE1  1-2 2-3
             MODE2  2-3 1-2
             MODE3  2-3 2-3
      JP5 - JUMPER, ADDRESS
             1-2 - 2000000h-39FFFFFh
             2-3 - 3A00000h-4FFFFFFh
CN1/2/3/4 - connectors joining to main board
*/
ROM_START( fanzonem )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "epr-21440a.ic13",  0x0000000, 0x0080000, CRC(28457d58) SHA1(1c7c3153eae1faa6c09595dc01906fe05702352d) ) // ST M27C4002-12F1
	ROM_LOAD16_WORD_SWAP( "mpr-21441.ic2",    0x0400000, 0x0400000, CRC(b69133a5) SHA1(63fb8b23cf216a84a2f2458b14a1648b22ae6256) )
	ROM_LOAD16_WORD_SWAP( "mpr-21445.ic1",    0x0800000, 0x0400000, CRC(c02ffbd3) SHA1(284fb6ff25b0621b7c1406a78308da522beaaaf7) )
	ROM_LOAD16_WORD_SWAP( "mpr-21442.ic4",    0x0c00000, 0x0400000, CRC(d4d3575f) SHA1(a3f50f2c932e5b9b9d7e0acf04e8aec41f252147) )
	ROM_LOAD16_WORD_SWAP( "mpr-21446.ic3",    0x1000000, 0x0400000, CRC(4831539e) SHA1(7a3b1c6d4d7e6652efd16fe95ed0471b67c1871e) )
	ROM_LOAD16_WORD_SWAP( "mpr-21443.ic6",    0x1400000, 0x0400000, CRC(cb1401c9) SHA1(a24836b84f8eb63079607b62dd6e72d76cca14ae) )
	ROM_LOAD16_WORD_SWAP( "mpr-21447.ic5",    0x1800000, 0x0400000, CRC(61e0d313) SHA1(595553d82e63aaadb1b19ccf761748677013454d) )
	ROM_LOAD16_WORD_SWAP( "mpr-21444.ic8",    0x1c00000, 0x0400000, CRC(a82ff33b) SHA1(9559ee4cf1ec487c3847df40d10aa2a4eaee97d2) )
ROM_END

ROM_START( yattrmnp ) // ROM board stickered 837-13598
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "cart", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "epr-21122.ic13",    0x0000000, 0x0080000, CRC(49f56e32) SHA1(7d8bdaaf3a4edd9df90becc3ec5e94a69bb29ffc) ) // ST M27C4002-12F1
	ROM_LOAD16_WORD_SWAP( "mpr-21125.ic02",    0x0400000, 0x0400000, CRC(40f5f119) SHA1(68fc634734ab05b54ff93256259969f16e26807d) )
	ROM_LOAD16_WORD_SWAP( "mpr-21130.ic03",    0x0800000, 0x0400000, CRC(84cb4e9c) SHA1(675464b0fdf80a3d6e39292e56528e906d388d3c) )
	ROM_LOAD16_WORD_SWAP( "mpr-21126.ic04",    0x0c00000, 0x0400000, CRC(36095649) SHA1(cd06471943380d80887e389e04008d78c58d14b3) )
	ROM_LOAD16_WORD_SWAP( "mpr-21131.ic05",    0x1000000, 0x0400000, CRC(b1c860d8) SHA1(474539f86369f10df5f0e51fd5b44febbc846f98) )
	ROM_LOAD16_WORD_SWAP( "mpr-21127.ic06",    0x1400000, 0x0400000, CRC(f55901d7) SHA1(ab80d82b6707929af9cc5615319bc5b336c13f4a) )
	ROM_LOAD16_WORD_SWAP( "mpr-21132.ic07",    0x1800000, 0x0400000, CRC(809b1b0e) SHA1(8404a03624ba735cc796a38a2b2b96bf9e37f5ee) )
	ROM_LOAD16_WORD_SWAP( "mpr-21128.ic08",    0x1c00000, 0x0400000, CRC(c2e2f72e) SHA1(299c60a74caf3ec6641b6369c823059ef2fbdc5e) )
	ROM_LOAD16_WORD_SWAP( "mpr-21133.ic09",    0x2000000, 0x0400000, CRC(2dc21956) SHA1(6fd7ab435127286a7b212e30b1f29a6f3fbd018b) )
	ROM_LOAD16_WORD_SWAP( "mpr-21129.ic10",    0x2400000, 0x0400000, CRC(997d67bd) SHA1(f588b4391982df4afe60fe8bf739cd510f98caa0) )
	ROM_LOAD16_WORD_SWAP( "mpr-21124.ic11",    0x2800000, 0x0400000, CRC(10f073ac) SHA1(8c16d8ad9c435d44a7b4dd327cd44ffd2f7e895c) )
	ROM_LOAD16_WORD_SWAP( "mpr-21123.ic12",    0x2c00000, 0x0400000, CRC(491b9166) SHA1(f1d5e5b7bef4fc99862b1b8178759e96ba4cb29c) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "315-5930.ic19", 0x000, 0x117, CRC(d1201563) SHA1(a133b07240c0a4eb8bae4b438d98fc6148fb7f4f) )
ROM_END

GAME( 1996, stvbios,   0,       stv_slot, stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "ST-V Bios", MACHINE_IS_BIOS_ROOT )

//GAME YEAR, NAME,     PARENT,  MACH,     INP,      STATE,       INIT,            MONITOR
/* Playable */
GAME( 1998, astrass,   stvbios, stv_5881, stv6b,    stv_state,   init_astrass,    ROT0,   "Sunsoft",                      "Astra SuperStars (J 980514 V1.002)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, bakubaku,  stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Baku Baku Animal (J 950407 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, batmanfr,  stvbios, batmanfr, batmanfr, stv_state,   init_batmanfr,   ROT0,   "Acclaim",                      "Batman Forever (JUE 960507 V1.000)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, colmns97,  stvbios, stv,      stv,      stv_state,   init_colmns97,   ROT0,   "Sega",                         "Columns '97 (JET 961209 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, cotton2,   stvbios, stv,      stv,      stv_state,   init_cotton2,    ROT0,   "Success",                      "Cotton 2 (JUET 970902 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, cottonbm,  stvbios, stv,      stv,      stv_state,   init_cottonbm,   ROT0,   "Success",                      "Cotton Boomerang (JUET 980709 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAMEL(1995, critcrsh,  stvbios, critcrsh, critcrsh, stv_state,   init_stv,        ROT0,   "Sega",                         "Critter Crusher (EA 951204 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS, layout_critcrsh )
GAMEL(1995, tatacot,   critcrsh,critcrsh, critcrsh, stv_state,   init_stv,        ROT0,   "Sega",                         "Tatacot (JA 951128 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS, layout_critcrsh )
GAME( 1999, danchih,   stvbios, stvmp,    stvmp,    stv_state,   init_danchih,    ROT0,   "Altron (Tecmo license)",       "Danchi de Hanafuda (J 990607 V1.400)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, danchiq,   stvbios, stv,      stv,      stv_state,   init_danchiq,    ROT0,   "Altron",                       "Danchi de Quiz: Okusan Yontaku Desuyo! (J 001128 V1.200)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, diehard,   stvbios, stv,      stv,      stv_state,   init_diehard,    ROT0,   "Sega",                         "Die Hard Arcade (UET 960515 V1.000)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND  )
GAME( 1996, dnmtdeka,  diehard, stv,      stv,      stv_state,   init_dnmtdeka,   ROT0,   "Sega",                         "Dynamite Deka (J 960515 V1.000)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND  )
GAME( 1995, ejihon,    stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Ejihon Tantei Jimusho (J 950613 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, elandore,  stvbios, stv_5881, stv6b,    stv_state,   init_elandore,   ROT0,   "Sai-Mate",                     "Touryuu Densetsu Elan-Doree / Elan Doree - Legend of Dragoon (JUET 980922 V1.006)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, ffrevng10, ffreveng,stv_5881, stv6b,    stv_state,   init_ffreveng,   ROT0,   "Capcom",                       "Final Fight Revenge / Final Revenge (JUET 990714 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, ffreveng,  stvbios, stv_5881, stv6b,    stv_state,   init_ffreveng,   ROT0,   "Capcom",                       "Final Fight Revenge / Final Revenge (JUET 990930 V1.100)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, fhboxers,  stvbios, stv,      stv,      stv_state,   init_fhboxers,   ROT0,   "Sega",                         "Funky Head Boxers (JUETBKAL 951218 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, findlove,  stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Daiki / FCF",                  "Zenkoku Seifuku Bishoujo Grand Prix Find Love (J 971212 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, gaxeduel,  stvbios, stv,      stv6b,    stv_state,   init_gaxeduel,   ROT0,   "Sega",                         "Golden Axe - The Duel (JUETL 950117 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS)
GAME( 1998, grdforce,  stvbios, stv,      stv,      stv_state,   init_grdforce,   ROT0,   "Success",                      "Guardian Force (JUET 980318 V0.105)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, groovef,   stvbios, stv,      stv6b,    stv_state,   init_groovef,    ROT0,   "Atlus",                        "Groove on Fight - Gouketsuji Ichizoku 3 (J 970416 V1.001)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, hanagumi,  stvbios, stv,      stv,      stv_state,   init_hanagumi,   ROT0,   "Sega",                         "Sakura Taisen - Hanagumi Taisen Columns (J 971007 V1.010)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, introdon,  stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sunsoft / Success",            "Karaoke Quiz Intro Don Don! (J 960213 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, kiwames,   stvbios, stvmp,    stvmp,    stv_state,   init_stv,        ROT0,   "Athena",                       "Pro Mahjong Kiwame S (J 951020 V1.208)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, maruchan,  stvbios, stv,      stv,      stv_state,   init_maruchan,   ROT0,   "Sega / Toyosuisan",            "Maru-Chan de Goo! (J 971216 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, mausuke,   stvbios, stv,      stv,      stv_state,   init_mausuke,    ROT0,   "Data East",                    "Mausuke no Ojama the World (J 960314 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, micrombc,  stvbios, hopper,   micrombc, stv_state,   init_stv,        ROT0,   "Sega",                         "Microman Battle Charge (J 990326 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, myfairld,  stvbios, stvmp,    myfairld, stv_state,   init_stv,        ROT0,   "Micronet",                     "Virtual Mahjong 2 - My Fair Lady (J 980608 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, othellos,  stvbios, stv,      stv,      stv_state,   init_othellos,   ROT0,   "Success",                      "Othello Shiyouyo (J 980423 V1.002)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, patocar,   stvbios, hopper,   patocar,  stv_state,   init_stv,        ROT0,   "Sega",                         "Hashire Patrol Car (J 990326 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, pblbeach,  stvbios, stv,      stv,      stv_state,   init_pblbeach,   ROT0,   "T&E Soft",                     "Pebble Beach - The Great Shot (JUE 950913 V0.990)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, prikura,   stvbios, stv,      stv,      stv_state,   init_prikura,    ROT0,   "Atlus",                        "Princess Clara Daisakusen (J 960910 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, puyosun,   stvbios, stv,      stv,      stv_state,   init_puyosun,    ROT0,   "Compile",                      "Puyo Puyo Sun (J 961115 V0.001)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, rsgun,     stvbios, stv_5881, stv,      stv_state,   init_rsgun,      ROT0,   "Treasure",                     "Radiant Silvergun (JUET 980523 V1.000)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, sasissu,   stvbios, stv,      stv,      stv_state,   init_sasissu,    ROT0,   "Sega",                         "Taisen Tanto-R Sashissu!! (J 980216 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, sanjeon,   sasissu, stv,      stv,      stv_state,   init_sanjeon,    ROT0,   "Sega / Deniam",                "DaeJeon! SanJeon SuJeon (AJTUE 990412 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, seabass,   stvbios, stv,      stv,      stv_state,   init_seabass,    ROT0,   "A wave inc. (Able license)",   "Sea Bass Fishing (JUET 971110 V0.001)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, shanhigw,  stvbios, stv,      stv,      stv_state,   init_shanhigw,   ROT0,   "Sunsoft / Activision",         "Shanghai - The Great Wall / Shanghai Triple Threat (JUE 950623 V1.005)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, shienryu,  stvbios, shienryu, stv,      stv_state,   init_shienryu,   ROT270, "Warashi",                      "Shienryu (JUET 961226 V1.000)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, sss,       stvbios, stv_5881, stv,      stv_state,   init_sss,        ROT0,   "Capcom / Cave / Victor Interactive Software",       "Steep Slope Sliders (JUET 981110 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // Also credited as Pack In Soft in ending screen
GAME( 1995, sandor,    stvbios, stv,      stv,      stv_state,   init_sandor,     ROT0,   "Sega",                         "Puzzle & Action: Sando-R (J 951114 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, thunt,     sandor,  stv,      stv,      stv_state,   init_thunt,      ROT0,   "Sega",                         "Puzzle & Action: Treasure Hunt (JUET 970901 V2.00E)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, thuntk,    sandor,  stv,      stv,      stv_state,   init_sandor,     ROT0,   "Sega / Deniam",                "Puzzle & Action: BoMulEul Chajara (JUET 970125 V2.00K)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, skychal,   stvbios, hopper,   patocar,  stv_state,   init_stv,        ROT0,   "Sega",                         "Sky Challenger (J 000406 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, smleague,  stvbios, stv,      stv,      stv_state,   init_smleague,   ROT0,   "Sega",                         "Super Major League (U 960108 V1.000)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, finlarch,  smleague,stv,      stv,      stv_state,   init_finlarch,   ROT0,   "Sega",                         "Final Arch (J 950714 V1.001)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, sokyugrt,  stvbios, stv,      stv,      stv_state,   init_sokyugrt,   ROT0,   "Raizing / Eighting",           "Soukyugurentai / Terra Diver (JUET 960821 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, suikoenb,  stvbios, stv,      stv6b,    stv_state,   init_suikoenb,   ROT0,   "Data East",                    "Suiko Enbu / Outlaws of the Lost Dynasty (JUETL 950314 V2.001)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, supgoal,   stvbios, hopper,   patocar,  stv_state,   init_stv,        ROT0,   "Sega",                         "Nerae! Super Goal (J 981218 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, techbowl,  stvbios, hopper,   patocar,  stv_state,   init_stv,        ROT0,   "Sega",                         "Technical Bowling (J 971212 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, vfkids,    stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Virtua Fighter Kids (JUET 960319 V0.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, vmahjong,  stvbios, stvmp,    vmahjong, stv_state,   init_stv,        ROT0,   "Micronet",                     "Virtual Mahjong (J 961214 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, winterht,  stvbios, stv,      stv,      stv_state,   init_winterht,   ROT0,   "Sega",                         "Winter Heat (JUET 971012 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, znpwfv,    stvbios, stv,      stv,      stv_state,   init_znpwfv,     ROT0,   "Sega",                         "Zen Nippon Pro-Wrestling Featuring Virtua (J 971123 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, znpwfvt,   znpwfv,  stv,      stv,      stv_state,   init_znpwfv,     ROT0,   "Sega",                         "Zen Nippon Pro-Wrestling Featuring Virtua (T 971123 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

/* Unemulated printer / camera devices */
// USA sets
GAME( 1997, pclub2,    stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 (U 970921 V1.000)", MACHINE_NOT_WORKING )
GAME( 1999, pclub2v3,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Vol. 3 (U 990310 V1.000)", MACHINE_NOT_WORKING ) // Hello Kitty themed
GAME( 1999, pclubpok,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club Pokemon B (U 991126 V1.000)", MACHINE_NOT_WORKING )
// Japan sets
GAME( 1999, pclub2fc,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Felix The Cat (Rev. A) (J 970415 V1.100)", MACHINE_NOT_WORKING )
GAME( 1998, pclub2pf,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Puffy (J V1.100)", MACHINE_NOT_WORKING ) // version info is blank
GAME( 1997, pclub2pe,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Pepsiman (J 970618 V1.100)", MACHINE_NOT_WORKING )
GAME( 1997, pclub2wb,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Warner Bros (J 970228 V1.000)", MACHINE_NOT_WORKING )

GAME( 1997, pclb2elk,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Earth Limited Kobe (Print Club Custom) (J 970808 V1.000)", MACHINE_NOT_WORKING )
GAME( 1997, pckobe99,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Kobe Luminaire '99 (Print Club Custom 3) (J 991203 V1.000)", MACHINE_NOT_WORKING )

GAME( 1997, pclub26w,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Vol. 6 Winter (J 961210 V1.000)", MACHINE_NOT_WORKING ) // internal string is 'PURIKURA2 97FUYU' (but in reality it seems to be an end of 96 Winter version)
GAME( 1997, pclub26wa, pclub26w,stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Vol. 6 Winter (J 970121 V1.200)", MACHINE_NOT_WORKING ) // ^
GAME( 1997, pclub27s,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Vol. 7 Spring (J 970313 V1.100)", MACHINE_NOT_WORKING )
GAME( 1997, prc28su,   stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Vol. 8 Summer (J 970616 V1.100)", MACHINE_NOT_WORKING ) // internal string 97SUMMER
GAME( 1997, prc29au,   stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 Vol. 9 Autumn (J V1.100)", MACHINE_NOT_WORKING ) // internal string 97AUTUMN, no date code! (all 0)
GAME( 1997, prc297wi,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 '97 Winter Ver (J 971017 V1.100, set 1)", MACHINE_NOT_WORKING ) // internal string is '97WINTER'
GAME( 1997, prc297wia, prc297wi,stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 '97 Winter Ver (J 971017 V1.100, set 2)", MACHINE_NOT_WORKING ) // different program revision, same date code, clearly didn't get updated properly
GAME( 1998, prc298sp,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 '98 Spring Ver (J 971017 V1.100)", MACHINE_NOT_WORKING ) // again, date doesn't appear to have bene updated, this should be early 98
GAME( 1998, prc298su,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 '98 Summer Ver (J 980603 V1.100)", MACHINE_NOT_WORKING ) //
GAME( 1998, prc298au,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 '98 Autumn Ver (J 980827 V1.000)", MACHINE_NOT_WORKING )
GAME( 2000, prc2ksu,   stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club 2 2000 Summer (J 000509 V1.000)", MACHINE_NOT_WORKING ) // internal string 2000_SUMMER


GAME( 1999, pclubor,   stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club Goukakenran (J 991104 V1.000)", MACHINE_NOT_WORKING )
GAME( 1999, pclubol,   stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club Olive (J 980717 V1.000)", MACHINE_NOT_WORKING )
GAME( 1997, pclub2kc,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club Kome Kome Club (J 970203 V1.000)", MACHINE_NOT_WORKING )
GAME( 1997, pclubyo2,  stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Atlus",                        "Print Club Yoshimoto V2 (J 970422 V1.100)", MACHINE_NOT_WORKING )


GAME( 1997, pclove,    stvbios, stv_5838, stv,      stvpc_state, init_decathlt_nokey,   ROT0,   "Atlus",                        "Print Club LoveLove (J 970421 V1.000)", MACHINE_NOT_WORKING ) // uses the same type of protection as decathlete!!
GAME( 1997, pclove2,   stvbios, stv_5838, stv,      stvpc_state, init_decathlt_nokey,   ROT0,   "Atlus",                        "Print Club LoveLove Ver 2 (J 970825 V1.000)", MACHINE_NOT_WORKING ) // ^
GAME( 1997, pcpooh2,   stvbios, stv_5838, stv,      stvpc_state, init_decathlt_nokey,   ROT0,   "Atlus",                        "Print Club Winnie-the-Pooh Vol. 2 (J 971218 V1.000)", MACHINE_NOT_WORKING ) // ^
GAME( 1998, pcpooh3,   stvbios, stv_5838, stv,      stvpc_state, init_decathlt_nokey,   ROT0,   "Atlus",                        "Print Club Winnie-the-Pooh Vol. 3 (J 980406 V1.000)", MACHINE_NOT_WORKING ) // ^

GAME( 1998, stress,    stvbios, stv,      stv,      stvpc_state, init_stv,        ROT0,   "Sega",                         "Stress Busters (J 981020 V1.000)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

GAME( 1996, nameclub,  stvbios, stv_5838, stv,      stvpc_state, init_decathlt_nokey,   ROT0,   "Sega",                         "Name Club (J 960315 V1.000)", MACHINE_NOT_WORKING ) // uses the same type of protection as decathlete!!
GAME( 1996, nclubv2,   stvbios, stv_5838, stv,      stvpc_state, init_decathlt_nokey,   ROT0,   "Sega",                         "Name Club Ver.2 (J 960315 V1.000)", MACHINE_NOT_WORKING ) // ^  (has the same datecode as nameclub, probably incorrect unless both were released today)
GAME( 1997, nclubv3,   stvbios, stv,      stv,      stvpc_state, init_nameclv3,         ROT0,   "Sega",                         "Name Club Ver.3 (J 970723 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // no protection





/* Doing something.. but not enough yet */
GAME( 1995, vfremix,   stvbios, stv,      stv,      stv_state,   init_vfremix,    ROT0,   "Sega",                         "Virtua Fighter Remix (JUETBKAL 950428 V1.000)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1996, decathlt,  stvbios, stv_5838, stv,      stv_state,   init_decathlt,   ROT0,   "Sega",                         "Decathlete (JUET 960709 V1.001)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1996, decathlto, decathlt,stv_5838, stv,      stv_state,   init_decathlt,   ROT0,   "Sega",                         "Decathlete (JUET 960424 V1.000)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1998, twcup98,   stvbios, stv_5881, stv,      stv_state,   init_twcup98,    ROT0,   "Tecmo",                        "Tecmo World Cup '98 (JUET 980410 V1.000)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // some situations with the GK result in the game stalling, maybe CPU core bug??
GAME( 1998, twsoc98,   twcup98, stv_5881, stv,      stv_state,   init_twcup98,    ROT0,   "Tecmo",                        "Tecmo World Soccer '98 (JUET 980410 V1.000)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // ^^ (check)

/* Gives I/O errors */
GAME( 1996, magzun,    stvbios, magzun,   stv,      stv_state,   init_magzun,     ROT0,   "Sega",                         "Magical Zunou Power (J 961031 V1.000)", MACHINE_NOT_WORKING | MACHINE_NODEVICE_MICROPHONE )
GAME( 1998, yattrmnp,  stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Yatterman Plus (J 981006 V1.000)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // needs emulation of the medal specific hardware
GAME( 1998, choroqhr,  stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega / Takara",                "Choro Q Hyper Racing 5 (J 981230 V1.000)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, fanzonem,  stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Fantasy Zone (medal game, REV.A) (J 990202 V1.000)", MACHINE_NOT_WORKING ) // require SH2's SCI serial port emulated, to communicate with coin/medal-related I/O board
GAME( 2000, sackids,   stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Soreyuke Anpanman Crayon Kids (J 001026 V1.000)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, dfeverg,   stvbios, stv,      stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Dancing Fever Gold (J 000821 V2.001)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

/* CD games */
GAME( 1995, sfish2,    0,       stvcd,    stv,      stv_state,   init_stv_us,     ROT0,   "Sega",                         "Sport Fishing 2 (UET 951106 V1.10e)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN )
GAME( 1995, sfish2j,   sfish2,  stvcd,    stv,      stv_state,   init_stv,        ROT0,   "Sega",                         "Sport Fishing 2 (J 951201 V1.100)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN )

/*
This is the known list of undumped ST-V games:
    Kiss Off (US version of mausuke)
    Baku Baku (US version of Baku Baku Animal,dunno if it exists for the ST-V)
    Sport Fishing
    Aroma Club
    Movie Club
    Name Club (other versions)
    Print Club 2 (other versions)
    Youen Denshi Mahjong Yuugi Gal Jan
    NBA Action (?)
    Quiz Omaeni Pipon Cho! (?)
Others may exist as well...
*/
