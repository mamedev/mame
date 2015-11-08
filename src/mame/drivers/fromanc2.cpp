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
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/2610intf.h"
#include "includes/fromanc2.h"
#include "rendlay.h"


/*************************************
 *
 *  Interrupts and memory handlers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(fromanc2_state::fromanc2_interrupt)
{
	device.execute().set_input_line(1, HOLD_LINE);
}


WRITE16_MEMBER(fromanc2_state::fromanc2_sndcmd_w)
{
	soundlatch_byte_w(space, offset, (data >> 8) & 0xff);   // 1P (LEFT)
	soundlatch2_byte_w(space, offset, data & 0xff);         // 2P (RIGHT)

	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	m_sndcpu_nmi_flag = 0;
}

WRITE16_MEMBER(fromanc2_state::fromanc2_portselect_w)
{
	m_portselect = data;
}

READ16_MEMBER(fromanc2_state::fromanc2_keymatrix_r)
{
	UINT16 ret;

	switch (m_portselect)
	{
		case 0x01:  ret = ioport("KEY0")->read(); break;
		case 0x02:  ret = ioport("KEY1")->read(); break;
		case 0x04:  ret = ioport("KEY2")->read(); break;
		case 0x08:  ret = ioport("KEY3")->read(); break;
		default:    ret = 0xffff;
			logerror("PC:%08X unknown %02X\n", space.device().safe_pc(), m_portselect);
			break;
	}

	return ret;
}

CUSTOM_INPUT_MEMBER(fromanc2_state::subcpu_int_r)
{
	return m_subcpu_int_flag & 0x01;
}

CUSTOM_INPUT_MEMBER(fromanc2_state::sndcpu_nmi_r)
{
	return m_sndcpu_nmi_flag & 0x01;
}

CUSTOM_INPUT_MEMBER(fromanc2_state::subcpu_nmi_r)
{
	return m_subcpu_nmi_flag & 0x01;
}

WRITE16_MEMBER(fromanc2_state::fromancr_gfxbank_eeprom_w)
{
	fromancr_gfxbank_w(data & 0xfff8);
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->di_write(data & 0x01);
		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE16_MEMBER(fromanc2_state::fromanc2_subcpu_w)
{
	m_datalatch1 = data;

	m_subcpu->set_input_line(0, HOLD_LINE);
	m_subcpu_int_flag = 0;
}

READ16_MEMBER(fromanc2_state::fromanc2_subcpu_r)
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	m_subcpu_nmi_flag = 0;

	return (m_datalatch_2h << 8) | m_datalatch_2l;
}

READ8_MEMBER(fromanc2_state::fromanc2_maincpu_r_l)
{
	return m_datalatch1 & 0x00ff;
}

READ8_MEMBER(fromanc2_state::fromanc2_maincpu_r_h)
{
	m_subcpu_int_flag = 1;

	return (m_datalatch1 & 0xff00) >> 8;
}

WRITE8_MEMBER(fromanc2_state::fromanc2_maincpu_w_l)
{
	m_datalatch_2l = data;
}

WRITE8_MEMBER(fromanc2_state::fromanc2_maincpu_w_h)
{
	m_datalatch_2h = data;
}

WRITE8_MEMBER(fromanc2_state::fromanc2_subcpu_nmi_clr)
{
	m_subcpu_nmi_flag = 1;
}

READ8_MEMBER(fromanc2_state::fromanc2_sndcpu_nmi_clr)
{
	m_sndcpu_nmi_flag = 1;

	return 0xff;
}

WRITE8_MEMBER(fromanc2_state::fromanc2_subcpu_rombank_w)
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

static ADDRESS_MAP_START( fromanc2_main_map, AS_PROGRAM, 16, fromanc2_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                 // MAIN ROM

	AM_RANGE(0x802000, 0x802fff) AM_READNOP                             // ???
	AM_RANGE(0x800000, 0x803fff) AM_WRITE(fromanc2_videoram_0_w)        // VRAM 0, 1 (1P)
	AM_RANGE(0x880000, 0x883fff) AM_WRITE(fromanc2_videoram_1_w)        // VRAM 2, 3 (1P)
	AM_RANGE(0x900000, 0x903fff) AM_WRITE(fromanc2_videoram_2_w)        // VRAM 0, 1 (2P)
	AM_RANGE(0x980000, 0x983fff) AM_WRITE(fromanc2_videoram_3_w)        // VRAM 2, 3 (2P)

	AM_RANGE(0xa00000, 0xa00fff) AM_RAM_DEVWRITE("lpalette", palette_device, write) AM_SHARE("lpalette") // PALETTE (1P)
	AM_RANGE(0xa80000, 0xa80fff) AM_RAM_DEVWRITE("rpalette", palette_device, write) AM_SHARE("rpalette") // PALETTE (2P)

	AM_RANGE(0xd00000, 0xd00023) AM_WRITE(fromanc2_gfxreg_0_w)          // SCROLL REG (1P/2P)
	AM_RANGE(0xd00100, 0xd00123) AM_WRITE(fromanc2_gfxreg_2_w)          // SCROLL REG (1P/2P)
	AM_RANGE(0xd00200, 0xd00223) AM_WRITE(fromanc2_gfxreg_1_w)          // SCROLL REG (1P/2P)
	AM_RANGE(0xd00300, 0xd00323) AM_WRITE(fromanc2_gfxreg_3_w)          // SCROLL REG (1P/2P)

	AM_RANGE(0xd00400, 0xd00413) AM_WRITENOP                            // ???
	AM_RANGE(0xd00500, 0xd00513) AM_WRITENOP                            // ???

	AM_RANGE(0xd01000, 0xd01001) AM_WRITE(fromanc2_sndcmd_w)            // SOUND REQ (1P/2P)
	AM_RANGE(0xd01100, 0xd01101) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd01200, 0xd01201) AM_WRITE(fromanc2_subcpu_w)            // SUB CPU WRITE
	AM_RANGE(0xd01300, 0xd01301) AM_READ(fromanc2_subcpu_r  )           // SUB CPU READ
	AM_RANGE(0xd01400, 0xd01401) AM_WRITE(fromanc2_gfxbank_0_w)         // GFXBANK (1P)
	AM_RANGE(0xd01500, 0xd01501) AM_WRITE(fromanc2_gfxbank_1_w)         // GFXBANK (2P)
	AM_RANGE(0xd01600, 0xd01601) AM_WRITE_PORT("EEPROMOUT")             // EEPROM DATA
	AM_RANGE(0xd01800, 0xd01801) AM_READ(fromanc2_keymatrix_r)          // INPUT KEY MATRIX
	AM_RANGE(0xd01a00, 0xd01a01) AM_WRITE(fromanc2_portselect_w)        // PORT SELECT (1P/2P)

	AM_RANGE(0xd80000, 0xd8ffff) AM_RAM                                 // WORK RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fromancr_main_map, AS_PROGRAM, 16, fromanc2_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                 // MAIN ROM

	AM_RANGE(0x800000, 0x803fff) AM_WRITE(fromancr_videoram_0_w)        // VRAM BG (1P/2P)
	AM_RANGE(0x880000, 0x883fff) AM_WRITE(fromancr_videoram_1_w)        // VRAM FG (1P/2P)
	AM_RANGE(0x900000, 0x903fff) AM_WRITE(fromancr_videoram_2_w)        // VRAM TEXT (1P/2P)
	AM_RANGE(0x980000, 0x983fff) AM_WRITENOP                            // VRAM Unused ?

	AM_RANGE(0xa00000, 0xa00fff) AM_RAM_DEVWRITE("lpalette", palette_device, write) AM_SHARE("lpalette") // PALETTE (1P)
	AM_RANGE(0xa80000, 0xa80fff) AM_RAM_DEVWRITE("rpalette", palette_device, write) AM_SHARE("rpalette") // PALETTE (2P)

	AM_RANGE(0xd00000, 0xd00023) AM_WRITE(fromancr_gfxreg_1_w)          // SCROLL REG (1P/2P)
	AM_RANGE(0xd00200, 0xd002ff) AM_WRITENOP                            // ?
	AM_RANGE(0xd00400, 0xd00413) AM_WRITENOP                            // ???
	AM_RANGE(0xd00500, 0xd00513) AM_WRITENOP                            // ???
	AM_RANGE(0xd01000, 0xd01001) AM_WRITE(fromanc2_sndcmd_w)            // SOUND REQ (1P/2P)
	AM_RANGE(0xd00100, 0xd00123) AM_WRITE(fromancr_gfxreg_0_w)          // SCROLL REG (1P/2P)
	AM_RANGE(0xd01100, 0xd01101) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd01200, 0xd01201) AM_WRITE(fromanc2_subcpu_w)            // SUB CPU WRITE
	AM_RANGE(0xd01300, 0xd01301) AM_READ(fromanc2_subcpu_r)             // SUB CPU READ
	AM_RANGE(0xd01400, 0xd01401) AM_WRITENOP                            // COIN COUNTER ?
	AM_RANGE(0xd01600, 0xd01601) AM_WRITE(fromancr_gfxbank_eeprom_w)    // EEPROM DATA, GFXBANK (1P/2P)
	AM_RANGE(0xd01800, 0xd01801) AM_READ(fromanc2_keymatrix_r)          // INPUT KEY MATRIX
	AM_RANGE(0xd01a00, 0xd01a01) AM_WRITE(fromanc2_portselect_w)        // PORT SELECT (1P/2P)

	AM_RANGE(0xd80000, 0xd8ffff) AM_RAM                                 // WORK RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fromanc4_main_map, AS_PROGRAM, 16, fromanc2_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                             // MAIN ROM
	AM_RANGE(0x400000, 0x7fffff) AM_ROM                             // DATA ROM

	AM_RANGE(0x800000, 0x81ffff) AM_RAM                             // WORK RAM

	AM_RANGE(0xd00000, 0xd00001) AM_WRITE(fromanc2_portselect_w)    // PORT SELECT (1P/2P)

	AM_RANGE(0xd10000, 0xd10001) AM_WRITENOP                        // ?
	AM_RANGE(0xd30000, 0xd30001) AM_WRITENOP                        // ?
	AM_RANGE(0xd50000, 0xd50001) AM_WRITE_PORT("EEPROMOUT")         // EEPROM DATA

	AM_RANGE(0xd70000, 0xd70001) AM_WRITE(fromanc2_sndcmd_w)        // SOUND REQ (1P/2P)

	AM_RANGE(0xd80000, 0xd8ffff) AM_WRITE(fromanc4_videoram_0_w)    // VRAM FG (1P/2P)
	AM_RANGE(0xd90000, 0xd9ffff) AM_WRITE(fromanc4_videoram_1_w)    // VRAM BG (1P/2P)
	AM_RANGE(0xda0000, 0xdaffff) AM_WRITE(fromanc4_videoram_2_w)    // VRAM TEXT (1P/2P)

	AM_RANGE(0xdb0000, 0xdb0fff) AM_RAM_DEVWRITE("lpalette", palette_device, write) AM_SHARE("lpalette") // PALETTE (1P)
	AM_RANGE(0xdc0000, 0xdc0fff) AM_RAM_DEVWRITE("rpalette", palette_device, write) AM_SHARE("rpalette") // PALETTE (2P)

	AM_RANGE(0xd10000, 0xd10001) AM_READ(fromanc2_keymatrix_r)      // INPUT KEY MATRIX
	AM_RANGE(0xd20000, 0xd20001) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xe00000, 0xe0001d) AM_WRITE(fromanc4_gfxreg_0_w)      // SCROLL, GFXBANK (1P/2P)
	AM_RANGE(0xe10000, 0xe1001d) AM_WRITE(fromanc4_gfxreg_1_w)      // SCROLL, GFXBANK (1P/2P)
	AM_RANGE(0xe20000, 0xe2001d) AM_WRITE(fromanc4_gfxreg_2_w)      // SCROLL, GFXBANK (1P/2P)

	AM_RANGE(0xe30000, 0xe30013) AM_WRITENOP                        // ???
	AM_RANGE(0xe40000, 0xe40013) AM_WRITENOP                        // ???

	AM_RANGE(0xe50000, 0xe50009) AM_WRITENOP                        // EXT-COMM PORT ?
	AM_RANGE(0xe5000c, 0xe5000d) AM_READNOP                         // EXT-COMM PORT ?
ADDRESS_MAP_END


static ADDRESS_MAP_START( fromanc2_sub_map, AS_PROGRAM, 8, fromanc2_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM                                 // ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")                    // ROM(BANK)
	AM_RANGE(0x8000, 0xbfff) AM_RAM                                 // RAM(WORK)
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank2")                    // RAM(BANK)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fromanc2_sub_io_map, AS_IO, 8, fromanc2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(fromanc2_subcpu_rombank_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(fromanc2_maincpu_r_l, fromanc2_maincpu_w_l) // to/from MAIN CPU
	AM_RANGE(0x04, 0x04) AM_READWRITE(fromanc2_maincpu_r_h, fromanc2_maincpu_w_h) // to/from MAIN CPU
	AM_RANGE(0x06, 0x06) AM_WRITE(fromanc2_subcpu_nmi_clr)
ADDRESS_MAP_END


static ADDRESS_MAP_START( fromanc2_sound_map, AS_PROGRAM, 8, fromanc2_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fromanc2_sound_io_map, AS_IO, 8, fromanc2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r) AM_WRITENOP     // snd cmd (1P) / ?
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch2_byte_r)                // snd cmd (2P)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0x0c, 0x0c) AM_READ(fromanc2_sndcpu_nmi_clr)
ADDRESS_MAP_END


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
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, fromanc2_state,subcpu_int_r, NULL) // SUBCPU INT FLAG
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, fromanc2_state,sndcpu_nmi_r, NULL) // SNDCPU NMI FLAG
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, fromanc2_state,subcpu_nmi_r, NULL) // SUBCPU NMI FLAG
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
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
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, fromanc2_state,sndcpu_nmi_r, NULL) // SNDCPU NMI FLAG
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
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

static GFXDECODE_START( fromanc2 )
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

static GFXDECODE_START( fromancr )
	GFXDECODE_ENTRY( "gfx1", 0, fromancr_tilelayout, 512, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, fromancr_tilelayout, 256, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, fromancr_tilelayout,   0, 1 )
GFXDECODE_END


/*************************************
 *
 *  Sound interface
 *
 *************************************/

WRITE_LINE_MEMBER(fromanc2_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


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
	m_bankedram = auto_alloc_array(machine(), UINT8, 0x4000 * 3);

	membank("bank1")->configure_entries(0, 4, memregion("sub")->base(), 0x4000);
	membank("bank2")->configure_entry(0, memregion("sub")->base() + 0x08000);
	membank("bank2")->configure_entries(1, 3, m_bankedram, 0x4000);

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

static MACHINE_CONFIG_START( fromanc2, fromanc2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,32000000/2)      /* 16.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromanc2_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", fromanc2_state,  fromanc2_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80,32000000/4)        /* 8.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromanc2_sound_map)
	MCFG_CPU_IO_MAP(fromanc2_sound_io_map)

	MCFG_CPU_ADD("sub", Z80,32000000/4)     /* 8.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromanc2_sub_map)
	MCFG_CPU_IO_MAP(fromanc2_sub_io_map)

	MCFG_MACHINE_START_OVERRIDE(fromanc2_state,fromanc2)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "lpalette", fromanc2)

	MCFG_PALETTE_ADD("lpalette", 2048)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)
	MCFG_PALETTE_ADD("rpalette", 2048)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromanc2_state, screen_update_fromanc2_left)
	MCFG_SCREEN_PALETTE("lpalette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromanc2_state, screen_update_fromanc2_right)
	MCFG_SCREEN_PALETTE("rpalette")

	MCFG_VIDEO_START_OVERRIDE(fromanc2_state,fromanc2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(fromanc2_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.75)
	MCFG_SOUND_ROUTE(2, "mono", 0.75)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fromancr, fromanc2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,32000000/2)      /* 16.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromancr_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", fromanc2_state,  fromanc2_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80,32000000/4)        /* 8.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromanc2_sound_map)
	MCFG_CPU_IO_MAP(fromanc2_sound_io_map)

	MCFG_CPU_ADD("sub", Z80,32000000/4)     /* 8.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromanc2_sub_map)
	MCFG_CPU_IO_MAP(fromanc2_sub_io_map)

	MCFG_MACHINE_START_OVERRIDE(fromanc2_state,fromanc2)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "lpalette", fromancr)

	MCFG_PALETTE_ADD("lpalette", 2048)
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)
	MCFG_PALETTE_ADD("rpalette", 2048)
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromanc2_state, screen_update_fromanc2_left)
	MCFG_SCREEN_PALETTE("lpalette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromanc2_state, screen_update_fromanc2_right)
	MCFG_SCREEN_PALETTE("rpalette")

	MCFG_VIDEO_START_OVERRIDE(fromanc2_state,fromancr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(fromanc2_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.75)
	MCFG_SOUND_ROUTE(2, "mono", 0.75)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fromanc4, fromanc2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,32000000/2)      /* 16.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromanc4_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", fromanc2_state,  fromanc2_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80,32000000/4)        /* 8.00 MHz */
	MCFG_CPU_PROGRAM_MAP(fromanc2_sound_map)
	MCFG_CPU_IO_MAP(fromanc2_sound_io_map)

	MCFG_MACHINE_START_OVERRIDE(fromanc2_state,fromanc4)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "lpalette", fromancr)

	MCFG_PALETTE_ADD("lpalette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_PALETTE_ADD("rpalette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromanc2_state, screen_update_fromanc2_left)
	MCFG_SCREEN_PALETTE("lpalette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(fromanc2_state, screen_update_fromanc2_right)
	MCFG_SCREEN_PALETTE("rpalette")

	MCFG_VIDEO_START_OVERRIDE(fromanc2_state,fromanc4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(fromanc2_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.75)
	MCFG_SOUND_ROUTE(2, "mono", 0.75)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( fromanc2 )
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

DRIVER_INIT_MEMBER(fromanc2_state,fromanc2)
{
	m_subcpu_nmi_flag = 1;
	m_subcpu_int_flag = 1;
	m_sndcpu_nmi_flag = 1;
}

DRIVER_INIT_MEMBER(fromanc2_state,fromanc4)
{
	m_sndcpu_nmi_flag = 1;
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1995, fromanc2, 0, fromanc2, fromanc2, fromanc2_state, fromanc2, ROT0, "Video System Co.", "Taisen Idol-Mahjong Final Romance 2 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, fromancr, 0, fromancr, fromanc2, fromanc2_state, fromanc2, ROT0, "Video System Co.", "Taisen Mahjong Final Romance R (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, fromanc4, 0, fromanc4, fromanc4, fromanc2_state, fromanc4, ROT0, "Video System Co.", "Taisen Mahjong Final Romance 4 (Japan)", MACHINE_SUPPORTS_SAVE )
