// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
/******************************************************************************

    Game Driver for Video System Mahjong series.

    Taisen Idol-Mahjong Final Romance 2 (Japan)
    (c)1995 Video System Co.,Ltd.

    Taisen Mahjong Final Romance R (Japan)
    (c)1995 Video System Co.,Ltd.

    Taisen Mahjong Final Romance 4 (Japan)
    (c)1998 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/28 -
    Special thanks to Uki.

******************************************************************************/

#include "emu.h"
#include "includes/fromanc2.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"
#include "rendlay.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Interrupts and memory handlers
 *
 *************************************/

void fromanc2_state::sndcmd_w(uint16_t data)
{
	m_soundlatch->write((data >> 8) & 0xff);   // 1P (LEFT)
	m_soundlatch2->write(data & 0xff);         // 2P (RIGHT)

	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	m_sndcpu_nmi_flag = 0;
}

void fromanc2_state::portselect_w(uint16_t data)
{
	m_portselect = data;
}

uint16_t fromanc2_state::keymatrix_r()
{
	uint16_t ret;

	switch (m_portselect)
	{
		case 0x01:  ret = ioport("KEY0")->read(); break;
		case 0x02:  ret = ioport("KEY1")->read(); break;
		case 0x04:  ret = ioport("KEY2")->read(); break;
		case 0x08:  ret = ioport("KEY3")->read(); break;
		default:    ret = 0xffff;
			logerror("PC:%08X unknown %02X\n", m_maincpu->pc(), m_portselect);
			break;
	}

	return ret;
}

READ_LINE_MEMBER(fromanc2_state::subcpu_int_r)
{
	return m_subcpu_int_flag & 0x01;
}

READ_LINE_MEMBER(fromanc2_state::sndcpu_nmi_r)
{
	return m_sndcpu_nmi_flag & 0x01;
}

READ_LINE_MEMBER(fromanc2_state::subcpu_nmi_r)
{
	return m_subcpu_nmi_flag & 0x01;
}

void fromanc2_state::fromancr_gfxbank_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	fromancr_gfxbank_w(data & 0xfff8);
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->di_write(data & 0x01);
		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void fromanc2_state::subcpu_w(uint16_t data)
{
	m_datalatch1 = data;

	m_subcpu->set_input_line(0, HOLD_LINE);
	m_subcpu_int_flag = 0;
}

uint16_t fromanc2_state::subcpu_r()
{
	m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	m_subcpu_nmi_flag = 0;

	return (m_datalatch_2h << 8) | m_datalatch_2l;
}

uint8_t fromanc2_state::maincpu_r_l()
{
	return m_datalatch1 & 0x00ff;
}

uint8_t fromanc2_state::maincpu_r_h()
{
	m_subcpu_int_flag = 1;

	return (m_datalatch1 & 0xff00) >> 8;
}

void fromanc2_state::maincpu_w_l(uint8_t data)
{
	m_datalatch_2l = data;
}

void fromanc2_state::maincpu_w_h(uint8_t data)
{
	m_datalatch_2h = data;
}

void fromanc2_state::subcpu_nmi_clr(uint8_t data)
{
	m_subcpu_nmi_flag = 1;
}

uint8_t fromanc2_state::sndcpu_nmi_clr()
{
	m_sndcpu_nmi_flag = 1;

	return 0xff;
}

void fromanc2_state::subcpu_rombank_w(uint8_t data)
{
	// Change ROM BANK
	membank("bank1")->set_entry(data & 0x03);

	// Change RAM BANK
	membank("bank2")->set_entry((data & 0x0c) >> 2);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void fromanc2_state::fromanc2_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                 // MAIN ROM

	map(0x802000, 0x802fff).nopr();                             // ???
	map(0x800000, 0x803fff).w(FUNC(fromanc2_state::fromanc2_videoram_0_w));        // VRAM 0, 1 (1P)
	map(0x880000, 0x883fff).w(FUNC(fromanc2_state::fromanc2_videoram_1_w));        // VRAM 2, 3 (1P)
	map(0x900000, 0x903fff).w(FUNC(fromanc2_state::fromanc2_videoram_2_w));        // VRAM 0, 1 (2P)
	map(0x980000, 0x983fff).w(FUNC(fromanc2_state::fromanc2_videoram_3_w));        // VRAM 2, 3 (2P)

	map(0xa00000, 0xa00fff).ram().w(m_lpalette, FUNC(palette_device::write16)).share("lpalette"); // PALETTE (1P)
	map(0xa80000, 0xa80fff).ram().w(m_rpalette, FUNC(palette_device::write16)).share("rpalette"); // PALETTE (2P)

	map(0xd00000, 0xd00023).w(FUNC(fromanc2_state::fromanc2_gfxreg_0_w));          // SCROLL REG (1P/2P)
	map(0xd00100, 0xd00123).w(FUNC(fromanc2_state::fromanc2_gfxreg_2_w));          // SCROLL REG (1P/2P)
	map(0xd00200, 0xd00223).w(FUNC(fromanc2_state::fromanc2_gfxreg_1_w));          // SCROLL REG (1P/2P)
	map(0xd00300, 0xd00323).w(FUNC(fromanc2_state::fromanc2_gfxreg_3_w));          // SCROLL REG (1P/2P)

	map(0xd00400, 0xd00413).nopw();                            // ???
	map(0xd00500, 0xd00513).nopw();                            // ???

	map(0xd01000, 0xd01001).w(FUNC(fromanc2_state::sndcmd_w));                     // SOUND REQ (1P/2P)
	map(0xd01100, 0xd01101).portr("SYSTEM");
	map(0xd01200, 0xd01201).w(FUNC(fromanc2_state::subcpu_w));                     // SUB CPU WRITE
	map(0xd01300, 0xd01301).r(FUNC(fromanc2_state::subcpu_r));                      // SUB CPU READ
	map(0xd01400, 0xd01401).w(FUNC(fromanc2_state::fromanc2_gfxbank_0_w));         // GFXBANK (1P)
	map(0xd01500, 0xd01501).w(FUNC(fromanc2_state::fromanc2_gfxbank_1_w));         // GFXBANK (2P)
	map(0xd01600, 0xd01601).portw("EEPROMOUT");             // EEPROM DATA
	map(0xd01800, 0xd01801).r(FUNC(fromanc2_state::keymatrix_r));                   // INPUT KEY MATRIX
	map(0xd01a00, 0xd01a01).w(FUNC(fromanc2_state::portselect_w));                 // PORT SELECT (1P/2P)

	map(0xd80000, 0xd8ffff).ram();                                 // WORK RAM
}

void fromanc2_state::fromancr_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                 // MAIN ROM

	map(0x800000, 0x803fff).w(FUNC(fromanc2_state::fromancr_videoram_0_w));        // VRAM BG (1P/2P)
	map(0x880000, 0x883fff).w(FUNC(fromanc2_state::fromancr_videoram_1_w));        // VRAM FG (1P/2P)
	map(0x900000, 0x903fff).w(FUNC(fromanc2_state::fromancr_videoram_2_w));        // VRAM TEXT (1P/2P)
	map(0x980000, 0x983fff).nopw();                            // VRAM Unused ?

	map(0xa00000, 0xa00fff).ram().w(m_lpalette, FUNC(palette_device::write16)).share("lpalette"); // PALETTE (1P)
	map(0xa80000, 0xa80fff).ram().w(m_rpalette, FUNC(palette_device::write16)).share("rpalette"); // PALETTE (2P)

	map(0xd00000, 0xd00023).w(FUNC(fromanc2_state::fromancr_gfxreg_1_w));          // SCROLL REG (1P/2P)
	map(0xd00200, 0xd002ff).nopw();                            // ?
	map(0xd00400, 0xd00413).nopw();                            // ???
	map(0xd00500, 0xd00513).nopw();                            // ???
	map(0xd01000, 0xd01001).w(FUNC(fromanc2_state::sndcmd_w));                     // SOUND REQ (1P/2P)
	map(0xd00100, 0xd00123).w(FUNC(fromanc2_state::fromancr_gfxreg_0_w));          // SCROLL REG (1P/2P)
	map(0xd01100, 0xd01101).portr("SYSTEM");
	map(0xd01200, 0xd01201).w(FUNC(fromanc2_state::subcpu_w));                     // SUB CPU WRITE
	map(0xd01300, 0xd01301).r(FUNC(fromanc2_state::subcpu_r));                      // SUB CPU READ
	map(0xd01400, 0xd01401).nopw();                            // COIN COUNTER ?
	map(0xd01600, 0xd01601).w(FUNC(fromanc2_state::fromancr_gfxbank_eeprom_w));    // EEPROM DATA, GFXBANK (1P/2P)
	map(0xd01800, 0xd01801).r(FUNC(fromanc2_state::keymatrix_r));                   // INPUT KEY MATRIX
	map(0xd01a00, 0xd01a01).w(FUNC(fromanc2_state::portselect_w));                 // PORT SELECT (1P/2P)

	map(0xd80000, 0xd8ffff).ram();                                 // WORK RAM
}

void fromanc2_state::fromanc4_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // MAIN ROM
	map(0x400000, 0x7fffff).rom();                             // DATA ROM

	map(0x800000, 0x81ffff).ram();                             // WORK RAM

	map(0xd00000, 0xd00001).w(FUNC(fromanc2_state::portselect_w));             // PORT SELECT (1P/2P)

	map(0xd10000, 0xd10001).nopw();                        // ?
	map(0xd30000, 0xd30001).nopw();                        // ?
	map(0xd50000, 0xd50001).portw("EEPROMOUT");         // EEPROM DATA

	map(0xd70000, 0xd70001).w(FUNC(fromanc2_state::sndcmd_w));                 // SOUND REQ (1P/2P)

	map(0xd80000, 0xd8ffff).w(FUNC(fromanc2_state::fromanc4_videoram_0_w));    // VRAM FG (1P/2P)
	map(0xd90000, 0xd9ffff).w(FUNC(fromanc2_state::fromanc4_videoram_1_w));    // VRAM BG (1P/2P)
	map(0xda0000, 0xdaffff).w(FUNC(fromanc2_state::fromanc4_videoram_2_w));    // VRAM TEXT (1P/2P)

	map(0xdb0000, 0xdb0fff).ram().w(m_lpalette, FUNC(palette_device::write16)).share("lpalette"); // PALETTE (1P)
	map(0xdc0000, 0xdc0fff).ram().w(m_rpalette, FUNC(palette_device::write16)).share("rpalette"); // PALETTE (2P)

	map(0xd10000, 0xd10001).r(FUNC(fromanc2_state::keymatrix_r));               // INPUT KEY MATRIX
	map(0xd20000, 0xd20001).portr("SYSTEM");

	map(0xe00000, 0xe0001d).w(FUNC(fromanc2_state::fromanc4_gfxreg_0_w));      // SCROLL, GFXBANK (1P/2P)
	map(0xe10000, 0xe1001d).w(FUNC(fromanc2_state::fromanc4_gfxreg_1_w));      // SCROLL, GFXBANK (1P/2P)
	map(0xe20000, 0xe2001d).w(FUNC(fromanc2_state::fromanc4_gfxreg_2_w));      // SCROLL, GFXBANK (1P/2P)

	map(0xe30000, 0xe30013).nopw();                        // ???
	map(0xe40000, 0xe40013).nopw();                        // ???

	map(0xe50000, 0xe5000f).rw(m_uart, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask16(0x00ff).cswidth(16); // EXT-COMM PORT ?
}


void fromanc2_state::fromanc2_sub_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();                                 // ROM
	map(0x4000, 0x7fff).bankr("bank1");                    // ROM(BANK)
	map(0x8000, 0xbfff).ram();                                 // RAM(WORK)
	map(0xc000, 0xffff).bankrw("bank2");                    // RAM(BANK)
}

void fromanc2_state::fromanc2_sub_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(fromanc2_state::subcpu_rombank_w));
	map(0x02, 0x02).rw(FUNC(fromanc2_state::maincpu_r_l), FUNC(fromanc2_state::maincpu_w_l)); // to/from MAIN CPU
	map(0x04, 0x04).rw(FUNC(fromanc2_state::maincpu_r_h), FUNC(fromanc2_state::maincpu_w_h)); // to/from MAIN CPU
	map(0x06, 0x06).w(FUNC(fromanc2_state::subcpu_nmi_clr));
}


void fromanc2_state::fromanc2_sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xffff).ram();
}

void fromanc2_state::fromanc2_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw();     // snd cmd (1P) / ?
	map(0x04, 0x04).r(m_soundlatch2, FUNC(generic_latch_8_device::read));                // snd cmd (2P)
	map(0x08, 0x0b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x0c, 0x0c).r(FUNC(fromanc2_state::sndcpu_nmi_clr));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( fromanc2 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(fromanc2_state, subcpu_int_r) // SUBCPU INT FLAG
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(fromanc2_state, sndcpu_nmi_r) // SNDCPU NMI FLAG
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(fromanc2_state, subcpu_nmi_r) // SUBCPU NMI FLAG
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( "Service Mode (1P)" ) PORT_CODE(KEYCODE_F2) // TEST (1P)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( "Service Mode (2P)" ) PORT_CODE(KEYCODE_F2) // TEST (2P)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0xf8ff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( fromanc4 )
	PORT_INCLUDE( fromanc2 )

	PORT_MODIFY("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x0001, IP_ACTIVE_LOW)  // TEST (1P)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(fromanc2_state, sndcpu_nmi_r) // SNDCPU NMI FLAG
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("EEPROMOUT")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout fromanc2_tilelayout =
{
	8, 8,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( gfx_fromanc2 )
	GFXDECODE_ENTRY( "gfx1", 0, fromanc2_tilelayout,   0, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, fromanc2_tilelayout, 256, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, fromanc2_tilelayout, 512, 4 )
	GFXDECODE_ENTRY( "gfx4", 0, fromanc2_tilelayout, 768, 4 )
GFXDECODE_END

static const gfx_layout fromancr_tilelayout =
{
	8, 8,
	RGN_FRAC(1, 1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXDECODE_START( gfx_fromancr )
	GFXDECODE_ENTRY( "gfx1", 0, fromancr_tilelayout, 512, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, fromancr_tilelayout, 256, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, fromancr_tilelayout,   0, 1 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_START_MEMBER(fromanc2_state,fromanc4)
{
	save_item(NAME(m_portselect));
	save_item(NAME(m_sndcpu_nmi_flag));
	save_item(NAME(m_datalatch1));
	save_item(NAME(m_datalatch_2h));
	save_item(NAME(m_datalatch_2l));

	/* video-related elements are saved in video_start */
}

MACHINE_START_MEMBER(fromanc2_state,fromanc2)
{
	m_bankedram = std::make_unique<uint8_t[]>(0x4000 * 3);

	membank("bank1")->configure_entries(0, 4, memregion("sub")->base(), 0x4000);
	membank("bank2")->configure_entry(0, memregion("sub")->base() + 0x08000);
	membank("bank2")->configure_entries(1, 3, m_bankedram.get(), 0x4000);

	MACHINE_START_CALL_MEMBER(fromanc4);

	save_item(NAME(m_subcpu_int_flag));
	save_item(NAME(m_subcpu_nmi_flag));
	save_pointer(NAME(m_bankedram), 0x4000 * 3);
}

void fromanc2_state::machine_reset()
{
	m_portselect = 0;
	m_datalatch1 = 0;
	m_datalatch_2h = 0;
	m_datalatch_2l = 0;
}

void fromanc2_state::fromanc2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32000000/2);      /* 16.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromanc2_main_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(fromanc2_state::irq1_line_hold));

	Z80(config, m_audiocpu, 32000000/4);        /* 8.00 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromanc2_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &fromanc2_state::fromanc2_sound_io_map);

	Z80(config, m_subcpu, 32000000/4);     /* 8.00 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromanc2_sub_map);
	m_subcpu->set_addrmap(AS_IO, &fromanc2_state::fromanc2_sub_io_map);

	MCFG_MACHINE_START_OVERRIDE(fromanc2_state,fromanc2)

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_lpalette, gfx_fromanc2);

	PALETTE(config, m_lpalette).set_format(palette_device::GRBx_555, 2048);
	PALETTE(config, m_rpalette).set_format(palette_device::GRBx_555, 2048);

	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(512, 512);
	lscreen.set_visarea(0, 352-1, 0, 240-1);
	lscreen.set_screen_update(FUNC(fromanc2_state::screen_update_left));
	lscreen.set_palette(m_lpalette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(512, 512);
	rscreen.set_visarea(0, 352-1, 0, 240-1);
	rscreen.set_screen_update(FUNC(fromanc2_state::screen_update_right));
	rscreen.set_palette(m_rpalette);

	MCFG_VIDEO_START_OVERRIDE(fromanc2_state,fromanc2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.75);
	ymsnd.add_route(2, "mono", 0.75);
}

void fromanc2_state::fromancr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32000000/2);      /* 16.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromancr_main_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(fromanc2_state::irq1_line_hold));

	Z80(config, m_audiocpu, 32000000/4);        /* 8.00 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromanc2_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &fromanc2_state::fromanc2_sound_io_map);

	Z80(config, m_subcpu, 32000000/4);     /* 8.00 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromanc2_sub_map);
	m_subcpu->set_addrmap(AS_IO, &fromanc2_state::fromanc2_sub_io_map);

	MCFG_MACHINE_START_OVERRIDE(fromanc2_state,fromanc2)

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_lpalette, gfx_fromancr);

	PALETTE(config, m_lpalette).set_format(palette_device::xGRB_555, 2048);
	PALETTE(config, m_rpalette).set_format(palette_device::xGRB_555, 2048);

	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(512, 512);
	lscreen.set_visarea(0, 352-1, 0, 240-1);
	lscreen.set_screen_update(FUNC(fromanc2_state::screen_update_left));
	lscreen.set_palette(m_lpalette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(512, 512);
	rscreen.set_visarea(0, 352-1, 0, 240-1);
	rscreen.set_screen_update(FUNC(fromanc2_state::screen_update_right));
	rscreen.set_palette(m_rpalette);

	MCFG_VIDEO_START_OVERRIDE(fromanc2_state,fromancr)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.75);
	ymsnd.add_route(2, "mono", 0.75);
}

void fromanc2_state::fromanc4(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);      /* 16.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromanc4_main_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(fromanc2_state::irq1_line_hold));

	Z80(config, m_audiocpu, XTAL(32'000'000)/4);        /* 8.00 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &fromanc2_state::fromanc2_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &fromanc2_state::fromanc2_sound_io_map);

	MCFG_MACHINE_START_OVERRIDE(fromanc2_state,fromanc4)

	EEPROM_93C46_16BIT(config, m_eeprom);

	NS16550(config, m_uart, 2000000); // actual type is TL16C550CFN; clock unknown
	m_uart->out_int_callback().set_inputline("maincpu", M68K_IRQ_2);
	//m_uart->out_tx_callback().set("link", FUNC(rs232_port_device::write_txd));
	//m_uart->out_rts_callback().set("link", FUNC(rs232_port_device::write_rts));

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_lpalette, gfx_fromancr);

	PALETTE(config, m_lpalette).set_format(palette_device::xRGB_555, 2048);
	PALETTE(config, m_rpalette).set_format(palette_device::xRGB_555, 2048);

	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(512, 512);
	lscreen.set_visarea(0, 352-1, 0, 240-1);
	lscreen.set_screen_update(FUNC(fromanc2_state::screen_update_left));
	lscreen.set_palette(m_lpalette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(512, 512);
	rscreen.set_visarea(0, 352-1, 0, 240-1);
	rscreen.set_screen_update(FUNC(fromanc2_state::screen_update_right));
	rscreen.set_palette(m_rpalette);

	MCFG_VIDEO_START_OVERRIDE(fromanc2_state,fromanc4)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.75);
	ymsnd.add_route(2, "mono", 0.75);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( fromanc2 )
	ROM_REGION( 0x0080000, "maincpu", 0 )   // MAIN CPU
	ROM_LOAD16_WORD_SWAP( "9.ic23", 0x000000, 0x080000, CRC(1b8b2570) SHA1(efcaa9af0737ab280c1e49ad69dcd4e05f6102c8) )

	ROM_REGION( 0x0010000, "audiocpu", 0 )  // SOUND CPU
	ROM_LOAD( "5-ic85.bin",  0x00000, 0x10000, CRC(d8f19aa3) SHA1(f980c2a021fa1995bc18b6427b361506ca8d9bf2) )

	ROM_REGION( 0x0010000, "sub", 0 )   // SUB CPU
	ROM_LOAD( "3-ic1.bin",   0x00000, 0x10000, CRC(6d02090e) SHA1(08a538f3a578adbf83718e5e592c457b2ad841a6) )

	ROM_REGION( 0x0480000, "gfx1", 0 )  // LAYER4 DATA
	ROM_LOAD( "124-121.bin", 0x000000, 0x200000, CRC(0b62c9c5) SHA1(1e82398a34fb69bf2a82ef1af79dcc6a50ee53e9) )
	ROM_LOAD( "125-122.bin", 0x200000, 0x200000, CRC(1d6dc86e) SHA1(31804465fd9a7c8a20a4bc2217a70bda7963e0ae) )
	ROM_LOAD( "126-123.bin", 0x400000, 0x080000, CRC(9c0f7abc) SHA1(0b69d72e50e64bf02fed4a11cdf10db547953074) )

	ROM_REGION( 0x0480000, "gfx2", 0 )  // LAYER3 DATA
	ROM_LOAD( "35-47.bin",   0x000000, 0x200000, CRC(97ff0ad6) SHA1(eefa13ef07d6f665a641464089345f1e0ffa7b56) )
	ROM_LOAD( "161-164.bin", 0x200000, 0x200000, CRC(eedbc4d1) SHA1(2f882c5a2a0311bc1fca7b8569621ffee8cdbc82) )
	ROM_LOAD( "162-165.bin", 0x400000, 0x080000, CRC(9b546e59) SHA1(69a2fad9aa87fd07e59fed2fb19c5533a9176bb5) )

	ROM_REGION( 0x0200000, "gfx3", 0 )  // LAYER2 DATA
	ROM_LOAD( "36-48.bin",   0x000000, 0x200000, CRC(c8ee7f40) SHA1(3f043e4d93dd20f0bfb56b6345d8d60c884547db) )

	ROM_REGION( 0x0100000, "gfx4", 0 )  // LAYER1 DATA
	ROM_LOAD( "40-52.bin",   0x000000, 0x100000, CRC(dbb5062d) SHA1(d1be4d675b36ea6ebd602d5c990adcf3c029485e) )

	ROM_REGION( 0x0400000, "ymsnd", 0 ) // SOUND DATA
	ROM_LOAD( "ic96.bin",    0x000000, 0x200000, CRC(2f1b394c) SHA1(d95dd8231d7873328f2253eaa27374c79d87e21b) )
	ROM_LOAD( "ic97.bin",    0x200000, 0x200000, CRC(1d1377fc) SHA1(0dae5dfcbcf4ed6662522e9404fcac0236dce04d) )
ROM_END

ROM_START( fromanc2o )
	ROM_REGION( 0x0080000, "maincpu", 0 )   // MAIN CPU
	ROM_LOAD16_WORD_SWAP( "4-ic23.bin", 0x000000, 0x080000, CRC(96c90f9e) SHA1(c233e91d6967ef05cf14923273be84b17fce200f) )

	ROM_REGION( 0x0010000, "audiocpu", 0 )  // SOUND CPU
	ROM_LOAD( "5-ic85.bin",  0x00000, 0x10000, CRC(d8f19aa3) SHA1(f980c2a021fa1995bc18b6427b361506ca8d9bf2) )

	ROM_REGION( 0x0010000, "sub", 0 )   // SUB CPU
	ROM_LOAD( "3-ic1.bin",   0x00000, 0x10000, CRC(6d02090e) SHA1(08a538f3a578adbf83718e5e592c457b2ad841a6) )

	ROM_REGION( 0x0480000, "gfx1", 0 )  // LAYER4 DATA
	ROM_LOAD( "124-121.bin", 0x000000, 0x200000, CRC(0b62c9c5) SHA1(1e82398a34fb69bf2a82ef1af79dcc6a50ee53e9) )
	ROM_LOAD( "125-122.bin", 0x200000, 0x200000, CRC(1d6dc86e) SHA1(31804465fd9a7c8a20a4bc2217a70bda7963e0ae) )
	ROM_LOAD( "126-123.bin", 0x400000, 0x080000, CRC(9c0f7abc) SHA1(0b69d72e50e64bf02fed4a11cdf10db547953074) )

	ROM_REGION( 0x0480000, "gfx2", 0 )  // LAYER3 DATA
	ROM_LOAD( "35-47.bin",   0x000000, 0x200000, CRC(97ff0ad6) SHA1(eefa13ef07d6f665a641464089345f1e0ffa7b56) )
	ROM_LOAD( "161-164.bin", 0x200000, 0x200000, CRC(eedbc4d1) SHA1(2f882c5a2a0311bc1fca7b8569621ffee8cdbc82) )
	ROM_LOAD( "162-165.bin", 0x400000, 0x080000, CRC(9b546e59) SHA1(69a2fad9aa87fd07e59fed2fb19c5533a9176bb5) )

	ROM_REGION( 0x0200000, "gfx3", 0 )  // LAYER2 DATA
	ROM_LOAD( "36-48.bin",   0x000000, 0x200000, CRC(c8ee7f40) SHA1(3f043e4d93dd20f0bfb56b6345d8d60c884547db) )

	ROM_REGION( 0x0100000, "gfx4", 0 )  // LAYER1 DATA
	ROM_LOAD( "40-52.bin",   0x000000, 0x100000, CRC(dbb5062d) SHA1(d1be4d675b36ea6ebd602d5c990adcf3c029485e) )

	ROM_REGION( 0x0400000, "ymsnd", 0 ) // SOUND DATA
	ROM_LOAD( "ic96.bin",    0x000000, 0x200000, CRC(2f1b394c) SHA1(d95dd8231d7873328f2253eaa27374c79d87e21b) )
	ROM_LOAD( "ic97.bin",    0x200000, 0x200000, CRC(1d1377fc) SHA1(0dae5dfcbcf4ed6662522e9404fcac0236dce04d) )
ROM_END

ROM_START( fromancr )
	ROM_REGION( 0x0080000, "maincpu", 0 )   // MAIN CPU
	ROM_LOAD16_WORD_SWAP( "2-ic20.bin", 0x000000, 0x080000, CRC(378eeb9c) SHA1(c1cfc7440590a229b3cdc1114428a473fea15b63) )

	ROM_REGION( 0x0010000, "audiocpu", 0 )  // SOUND CPU
	ROM_LOAD( "5-ic73.bin",  0x0000000, 0x010000, CRC(3e4727fe) SHA1(816c0c2cd2e349900fb9cd63cbced4c621017f37) )

	ROM_REGION( 0x0010000, "sub", 0 )   // SUB CPU
	ROM_LOAD( "4-ic1.bin",   0x0000000, 0x010000, CRC(6d02090e) SHA1(08a538f3a578adbf83718e5e592c457b2ad841a6) )

	ROM_REGION( 0x0800000, "gfx1", 0 )  // BG DATA
	ROM_LOAD( "ic1-3.bin",   0x0000000, 0x400000, CRC(70ad9094) SHA1(534f10478a929e9e0cc4e01573a68474fe696099) )
	ROM_LOAD( "ic2-4.bin",   0x0400000, 0x400000, CRC(c6c6e8f7) SHA1(315e4e8ae9d1e3d68f4b2cff723d78652dc74e57) )

	ROM_REGION( 0x2400000, "gfx2", 0 )  // FG DATA
	ROM_LOAD( "ic28-13.bin", 0x0000000, 0x400000, CRC(7d7f9f63) SHA1(fe7b7a6bd9610d953f109b5ff8e38aab1c4ffac1) )
	ROM_LOAD( "ic29-14.bin", 0x0400000, 0x400000, CRC(8ec65f31) SHA1(9b63b18d5ad8f7ec37fa950b21d547fec559d5fa) )
	ROM_LOAD( "ic31-16.bin", 0x0800000, 0x400000, CRC(e4859534) SHA1(91fbbe0ab8119a954d76d33134290a7f7640e4ba) )
	ROM_LOAD( "ic32-17.bin", 0x0c00000, 0x400000, CRC(20d767da) SHA1(477d86538e95583238c50e11acee3ed9ed17b75a) )
	ROM_LOAD( "ic34-19.bin", 0x1000000, 0x400000, CRC(d62a383f) SHA1(0b11a97fa11a0b9657219d70a2ba26843b37d285) )
	ROM_LOAD( "ic35-20.bin", 0x1400000, 0x400000, CRC(4e697f38) SHA1(66b2e9ecedfcf878defb31528611574c1711e831) )
	ROM_LOAD( "ic37-22.bin", 0x1800000, 0x400000, CRC(6302bf5f) SHA1(bac8bead71e25e060bc75abd428dce97e5d51ef2) )
	ROM_LOAD( "ic38-23.bin", 0x1c00000, 0x400000, CRC(c6cffa53) SHA1(41a1c31d921fa92aa285e0a874565e929dba80dc) )
	ROM_LOAD( "ic40-25.bin", 0x2000000, 0x400000, CRC(af60bd0e) SHA1(0dc3a2e9b06626b3891b60368c3ef4d7ce1bdc6a) )

	ROM_REGION( 0x0200000, "gfx3", 0 )  // TEXT DATA
	ROM_LOAD( "ic28-29.bin", 0x0000000, 0x200000, CRC(f5e262aa) SHA1(35464d059f4814832bf5cb3bede4b8a600bc8a84) )

	ROM_REGION( 0x0400000, "ymsnd", 0 ) // SOUND DATA
	ROM_LOAD( "ic81.bin",    0x0000000, 0x200000, CRC(8ab6e343) SHA1(5ae28e6944edb0a4b8d0071ce48e348b6e927ca9) )
	ROM_LOAD( "ic82.bin",    0x0200000, 0x200000, CRC(f57daaf8) SHA1(720eadf771c89d8749317b632bbc5e8ff1f6f520) )
ROM_END

ROM_START( fromanc4 )
	ROM_REGION( 0x0800000, "maincpu", 0 )   // MAIN CPU + DATA
	ROM_LOAD16_WORD_SWAP( "ic18.bin",    0x0000000, 0x080000, CRC(46a47839) SHA1(f1ba47b193e7e4b1c0fe8d67a76a9c452989885c) )
	ROM_LOAD16_WORD_SWAP( "em33-m00.19", 0x0400000, 0x400000, CRC(6442534b) SHA1(a504d5cdd569ad4301f9917247531d4fdb807c76) )

	ROM_REGION( 0x0020000, "audiocpu", 0 )  // SOUND CPU
	ROM_LOAD( "ic79.bin", 0x0000000, 0x020000, CRC(c9587c09) SHA1(e04ee8c3f8519c2b2d3c2bdade1e142974b7fcb1) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  // BG DATA
	ROM_LOAD16_WORD_SWAP( "em33-c00.59", 0x0000000, 0x400000, CRC(7192bbad) SHA1(d9212860a516106c64e348c78e03091ee766ab23) )
	ROM_LOAD16_WORD_SWAP( "em33-c01.60", 0x0400000, 0x400000, CRC(d75af19a) SHA1(3a9c4ccf1f832d0302fe115d336e33e006910a8a) )
	ROM_LOAD16_WORD_SWAP( "em33-c02.61", 0x0800000, 0x400000, CRC(4f4d2735) SHA1(d0b59c8ed285ec9120a89b0198e414e33567729a) )
	ROM_LOAD16_WORD_SWAP( "em33-c03.62", 0x0c00000, 0x400000, CRC(7ece6ad5) SHA1(c506fc4ea68abf57009d524a17ca487f9c568abd) )

	ROM_REGION( 0x3000000, "gfx2", 0 )  // FG DATA
	ROM_LOAD16_WORD_SWAP( "em33-b00.38", 0x0000000, 0x400000, CRC(10b8f90d) SHA1(68b8f197c7be70082f61016824098c1ae3a76b38) )
	ROM_LOAD16_WORD_SWAP( "em33-b01.39", 0x0400000, 0x400000, CRC(3b3ea291) SHA1(bb80070a19bb1a1febda612ef260f895a8b65ce2) )
	ROM_LOAD16_WORD_SWAP( "em33-b02.40", 0x0800000, 0x400000, CRC(de88f95b) SHA1(d84a1896a1ef3d9b7fa7de23771168e17c7a450a) )
	ROM_LOAD16_WORD_SWAP( "em33-b03.41", 0x0c00000, 0x400000, CRC(35c1b398) SHA1(b2141cdd3b7f9e2cbfb0a048c440979b59149be5) )
	ROM_LOAD16_WORD_SWAP( "em33-b04.42", 0x1000000, 0x400000, CRC(84b8d5db) SHA1(5999a12c24c01ee8673c2c0a9193c8800a490e6f) )
	ROM_LOAD16_WORD_SWAP( "em33-b05.43", 0x1400000, 0x400000, CRC(b822b57c) SHA1(b50f3b73239a688101027f1c4247fed5ae59b064) )
	ROM_LOAD16_WORD_SWAP( "em33-b06.44", 0x1800000, 0x400000, CRC(8f1b2b19) SHA1(1e08908758fed104d114fecc9977a4a0eb93fe9b) )
	ROM_LOAD16_WORD_SWAP( "em33-b07.45", 0x1c00000, 0x400000, CRC(dd4ddcb7) SHA1(0145afa70c1a6f59eec65cf4d8572f2c00cd04a5) )
	ROM_LOAD16_WORD_SWAP( "em33-b08.46", 0x2000000, 0x400000, CRC(3d8ce018) SHA1(43c3cb4d6c26a8209fc290fcac56297fe66209e3) )
	ROM_LOAD16_WORD_SWAP( "em33-b09.47", 0x2400000, 0x400000, CRC(4ad79143) SHA1(9240ee46fff8f4a400a2bddaedb9acd258f37e1d) )
	ROM_LOAD16_WORD_SWAP( "em33-b10.48", 0x2800000, 0x400000, CRC(d6ab74b2) SHA1(1dbff7e997869a00922f6471afbd76d383ec0e2c) )
	ROM_LOAD16_WORD_SWAP( "em33-b11.49", 0x2c00000, 0x400000, CRC(4aa206b1) SHA1(afee0d8fc02e4f673ecccb9786c6d502dea5cb70) )

	ROM_REGION( 0x0400000, "gfx3", 0 )  // TEXT DATA
	ROM_LOAD16_WORD_SWAP( "em33-a00.37", 0x0000000, 0x400000, CRC(a3bd4a34) SHA1(78bd5298e83f89c738c18105c8bc809fa6a35206) )

	ROM_REGION( 0x0800000, "ymsnd", 0 ) // SOUND DATA
	ROM_LOAD16_WORD_SWAP( "em33-p00.88", 0x0000000, 0x400000, CRC(1c6418d2) SHA1(c66d6b35f342fcbeca5414dbb2ac038d8a2ec2c4) )
	ROM_LOAD16_WORD_SWAP( "em33-p01.89", 0x0400000, 0x400000, CRC(615b4e6e) SHA1(a031773ed27de2263e32422a3d11118bdcb2c197) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void fromanc2_state::init_fromanc2()
{
	m_subcpu_nmi_flag = 1;
	m_subcpu_int_flag = 1;
	m_sndcpu_nmi_flag = 1;
}

void fromanc2_state::init_fromanc4()
{
	m_sndcpu_nmi_flag = 1;
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1995, fromanc2,  0,        fromanc2, fromanc2, fromanc2_state, init_fromanc2, ROT0, "Video System Co.", "Taisen Idol-Mahjong Final Romance 2 (Japan, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, fromanc2o, fromanc2, fromanc2, fromanc2, fromanc2_state, init_fromanc2, ROT0, "Video System Co.", "Taisen Idol-Mahjong Final Romance 2 (Japan, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, fromancr,  0,        fromancr, fromanc2, fromanc2_state, init_fromanc2, ROT0, "Video System Co.", "Taisen Mahjong Final Romance R (Japan)",      MACHINE_SUPPORTS_SAVE )
GAME( 1998, fromanc4,  0,        fromanc4, fromanc4, fromanc2_state, init_fromanc4, ROT0, "Video System Co.", "Taisen Mahjong Final Romance 4 (Japan)",      MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
