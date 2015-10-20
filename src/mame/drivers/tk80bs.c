// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Robbbert
/***************************************************************************


NEC TK80BS
**********
TK-80BS (c) 1980 NEC

Preliminary driver by Angelo Salese
Various additions by Robbbert

The TK80BS (Basic Station) has a plugin keyboard, BASIC in rom,
and connected to a tv.

TODO:
    - (try to) dump proper roms, the whole driver is based off fake roms;
    - bios 0 BASIC doesn't seem to work properly; (It does if you type NEW first)
    - bios 1 does not boot up because it runs off into the weeds
    - bios 2 also does that, somehow it starts up anyway, but no commands work


****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"

#define KEYBOARD_TAG "keyboard"

class tk80bs_state : public driver_device
{
public:
	tk80bs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	DECLARE_READ8_MEMBER(ppi_custom_r);
	DECLARE_WRITE8_MEMBER(ppi_custom_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_READ8_MEMBER(port_b_r);
	UINT32 screen_update_tk80bs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_shared_ptr<UINT8> m_p_videoram;
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


UINT32 tk80bs_state::screen_update_tk80bs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;

	count = 0;

	for(y=0;y<16;y++)
	{
		for(x=0;x<32;x++)
		{
			int tile = m_p_videoram[count++];

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile, 0, 0, 0, x*8, y*8);
		}
	}

	return 0;
}

/* A0 and A1 are swapped at the 8255 chip */
READ8_MEMBER( tk80bs_state::ppi_custom_r )
{
	switch(offset)
	{
		case 1:
			return m_ppi->read(space, 2);
		case 2:
			return m_ppi->read(space, 1);
		default:
			return m_ppi->read(space, offset);
	}
}

WRITE8_MEMBER( tk80bs_state::ppi_custom_w )
{
	switch(offset)
	{
		case 1:
			m_ppi->write(space, 2, data);
			break;
		case 2:
			m_ppi->write(space, 1, data);
			break;
		default:
			m_ppi->write(space, offset, data);
	}
}

static ADDRESS_MAP_START(tk80bs_mem, AS_PROGRAM, 8, tk80bs_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
//  AM_RANGE(0x0c00, 0x7bff) AM_ROM // ext
	AM_RANGE(0x7df8, 0x7df9) AM_NOP // i8251 sio
	AM_RANGE(0x7dfc, 0x7dff) AM_READWRITE(ppi_custom_r,ppi_custom_w)
	AM_RANGE(0x7e00, 0x7fff) AM_RAM AM_SHARE("videoram") // video ram
	AM_RANGE(0x8000, 0xcfff) AM_RAM // RAM
	AM_RANGE(0xd000, 0xefff) AM_ROM // BASIC
	AM_RANGE(0xf000, 0xffff) AM_ROM // BSMON
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( tk80bs )
INPUT_PORTS_END

READ8_MEMBER( tk80bs_state::port_a_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}


READ8_MEMBER( tk80bs_state::port_b_r )
{
	if (m_term_data)
	{
		m_ppi->pc4_w(0); // send a strobe pulse
		return 0x20;
	}
	else
		return 0;
}

WRITE8_MEMBER( tk80bs_state::kbd_put )
{
	data &= 0x7f;
	if (data > 0x5f) data-=0x20;
	m_term_data = data;
}

/* F4 Character Displayer */
static const gfx_layout tk80bs_charlayout =
{
	8, 8,
	512,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( tk80bs )
	GFXDECODE_ENTRY( "chargen", 0x0000, tk80bs_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( tk80bs, tk80bs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_1MHz) //unknown clock
	MCFG_CPU_PROGRAM_MAP(tk80bs_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 128-1)
	MCFG_SCREEN_UPDATE_DRIVER(tk80bs_state, screen_update_tk80bs)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tk80bs)

	/* Devices */
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(tk80bs_state, port_a_r))
	MCFG_I8255_IN_PORTB_CB(READ8(tk80bs_state, port_b_r))

	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(tk80bs_state, kbd_put))
MACHINE_CONFIG_END


ROM_START( tk80bs )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	/* all of these aren't taken from an original machine*/
	ROM_SYSTEM_BIOS(0, "psedo", "Pseudo LEVEL 1")
	ROMX_LOAD( "tk80.dummy", 0x0000, 0x0800, BAD_DUMP CRC(553b25ca) SHA1(939350d7fa56ce567ddf393c9f4b9db6ebc18a2c), ROM_BIOS(1))
	ROMX_LOAD( "ext.l1",     0x0c00, 0x6e46, BAD_DUMP CRC(d05ed3ff) SHA1(8544aa2cb58df9edf221f5be2cdafa248dd33828), ROM_BIOS(1))
	ROMX_LOAD( "lv1basic.l1",0xe000, 0x09a2, BAD_DUMP CRC(3ff67a71) SHA1(528c9331740637e853c099e1739ecdea6dd200bc), ROM_BIOS(1))
	ROMX_LOAD( "bsmon.l1",   0xf000, 0x0db0, BAD_DUMP CRC(5daa599b) SHA1(7e6ec5bfb3eea114f7ee9ef589a89246b8533b2f), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "psedo10", "Pseudo LEVEL 2 1.0")
	ROMX_LOAD( "tk80.dummy", 0x0000, 0x0800, BAD_DUMP CRC(553b25ca) SHA1(939350d7fa56ce567ddf393c9f4b9db6ebc18a2c), ROM_BIOS(2))
	ROMX_LOAD( "ext.10",     0x0c00, 0x3dc2, BAD_DUMP CRC(3c64d488) SHA1(919180d5b34b981ab3dd8b2885d3c0933203f355), ROM_BIOS(2))
	ROMX_LOAD( "lv2basic.10",0xd000, 0x2000, BAD_DUMP CRC(594fe70e) SHA1(5854c1be5fa78c1bfee365379495f14bc23e15e7), ROM_BIOS(2))
	ROMX_LOAD( "bsmon.10",   0xf000, 0x0daf, BAD_DUMP CRC(d0047983) SHA1(79e2b5dc47b574b55375cbafffff144744093ec1), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(2, "psedo11", "Pseudo LEVEL 2 1.1")
	ROMX_LOAD( "tk80.dummy", 0x0000, 0x0800, BAD_DUMP CRC(553b25ca) SHA1(939350d7fa56ce567ddf393c9f4b9db6ebc18a2c), ROM_BIOS(3))
	ROMX_LOAD( "ext.11",     0x0c00, 0x3dd4, BAD_DUMP CRC(bd5c5169) SHA1(2ad70828348372328b76bac0fa93d3f6f17ade34), ROM_BIOS(3))
	ROMX_LOAD( "lv2basic.11",0xd000, 0x2000, BAD_DUMP CRC(3df9a3bd) SHA1(9539409c876bce27d630fe47d07a4316d2ce09cb), ROM_BIOS(3))
	ROMX_LOAD( "bsmon.11",   0xf000, 0x0ff6, BAD_DUMP CRC(fca7a609) SHA1(7c7eb5e5e4cf1e0021383bdfc192b88262aba6f5), ROM_BIOS(3))

	ROM_REGION( 0x1000, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "font.rom",    0x0000, 0x1000, BAD_DUMP CRC(94d95199) SHA1(9fe741eab866b0c520d4108bccc6277172fa190c))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT     CLASS         INIT     COMPANY                 FULLNAME       FLAGS */
COMP( 1980, tk80bs,   tk80,   0,       tk80bs,    tk80bs,   driver_device, 0, "Nippon Electronic Company", "TK-80BS", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
