// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        EC-65

        16/07/2009 Initial driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/g65816/g65816.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/6850acia.h"
#include "machine/keyboard.h"

#define PIA6821_TAG "pia6821"
#define ACIA6850_TAG "acia6850"
#define ACIA6551_TAG "acia6551"
#define VIA6522_0_TAG "via6522_0"
#define VIA6522_1_TAG "via6522_1"
#define MC6845_TAG "mc6845"
#define KEYBOARD_TAG "keyboard"

class ec65_state : public driver_device
{
public:
	ec65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_via_0(*this, VIA6522_0_TAG),
		m_via_1(*this, VIA6522_1_TAG),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	MC6845_UPDATE_ROW(crtc_update_row);
	UINT8 *m_p_chargen;
	required_device<via6522_device> m_via_0;
	required_device<via6522_device> m_via_1;
	required_shared_ptr<UINT8> m_p_videoram;
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};

class ec65k_state : public driver_device
{
public:
	ec65k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}
};

static ADDRESS_MAP_START(ec65_mem, AS_PROGRAM, 8, ec65_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE(PIA6821_TAG, pia6821_device, read, write)
	AM_RANGE(0xe010, 0xe010) AM_DEVREADWRITE(ACIA6850_TAG, acia6850_device, status_r, control_w)
	AM_RANGE(0xe011, 0xe011) AM_DEVREADWRITE(ACIA6850_TAG, acia6850_device, data_r, data_w)
	AM_RANGE(0xe100, 0xe10f) AM_DEVREADWRITE(VIA6522_0_TAG, via6522_device, read, write)
	AM_RANGE(0xe110, 0xe11f) AM_DEVREADWRITE(VIA6522_1_TAG, via6522_device, read, write)
	AM_RANGE(0xe130, 0xe133) AM_DEVREADWRITE(ACIA6551_TAG,  mos6551_device, read, write)
	AM_RANGE(0xe140, 0xe140) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0xe141, 0xe141) AM_DEVREADWRITE(MC6845_TAG, mc6845_device, register_r , register_w)
	AM_RANGE(0xe400, 0xe7ff) AM_RAM // 1KB on-board RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ec65k_mem, AS_PROGRAM, 8, ec65_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ec65 )
INPUT_PORTS_END

WRITE8_MEMBER( ec65_state::kbd_put )
{
	if (data)
	{
		m_via_0->write_pa0((data>>0)&1);
		m_via_0->write_pa1((data>>1)&1);
		m_via_0->write_pa2((data>>2)&1);
		m_via_0->write_pa3((data>>3)&1);
		m_via_0->write_pa4((data>>4)&1);
		m_via_0->write_pa5((data>>5)&1);
		m_via_0->write_pa6((data>>6)&1);
		m_via_0->write_pa7((data>>7)&1);
		m_via_0->write_ca1(1);
		m_via_0->write_ca1(0);
	}
}

void ec65_state::machine_reset()
{
	m_via_1->write_pb0(1);
	m_via_1->write_pb1(1);
	m_via_1->write_pb2(1);
	m_via_1->write_pb3(1);
	m_via_1->write_pb4(1);
	m_via_1->write_pb5(1);
	m_via_1->write_pb6(1);
	m_via_1->write_pb7(1);
}

void ec65_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

MC6845_UPDATE_ROW( ec65_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx,inv;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<4) | (ra & 0x0f)] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

/* F4 Character Displayer */
static const gfx_layout ec65_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( ec65 )
	GFXDECODE_ENTRY( "chargen", 0x0000, ec65_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( ec65, ec65_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502, XTAL_4MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ec65_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 200 - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ec65)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, "screen", XTAL_16MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8) /*?*/
	MCFG_MC6845_UPDATE_ROW_CB(ec65_state, crtc_update_row)

	/* devices */
	MCFG_DEVICE_ADD(PIA6821_TAG, PIA6821, 0)

	MCFG_DEVICE_ADD(ACIA6850_TAG, ACIA6850, 0)

	MCFG_DEVICE_ADD(VIA6522_0_TAG, VIA6522, XTAL_4MHz / 4)

	MCFG_DEVICE_ADD(VIA6522_1_TAG, VIA6522, XTAL_4MHz / 4)

	MCFG_DEVICE_ADD(ACIA6551_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)

	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(ec65_state, kbd_put))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ec65k, ec65k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",G65816, XTAL_4MHz) // can use 4,2 or 1 MHz
	MCFG_CPU_PROGRAM_MAP(ec65k_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 200 - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ec65)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, "screen", XTAL_16MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8) /*?*/
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ec65 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ec65.ic6", 0xf000, 0x1000, CRC(acd928ed) SHA1(e02a688a057ff77294717cf7b887425fed0b1153))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.ic19", 0x0000, 0x1000, CRC(9b56a28d) SHA1(41c04fd9fb542c50287bc0e366358a61fc4b0cd4)) // Located on VDU card
ROM_END

ROM_START( ec65k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ec65k.ic19",  0xf000, 0x1000, CRC(5e5a890a) SHA1(daa006f2179fd156833e11c73b37881cafe5dede))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.ic19", 0x0000, 0x1000, CRC(9b56a28d) SHA1(41c04fd9fb542c50287bc0e366358a61fc4b0cd4)) // Located on VDU card
ROM_END
/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1985, ec65,   0,       0,      ec65,      ec65, driver_device,    0,    "Elektor Electronics", "EC-65", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1985, ec65k,  ec65,    0,      ec65k,     ec65, driver_device,    0,    "Elektor Electronics", "EC-65K", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
