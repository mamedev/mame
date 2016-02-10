// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

    icatel - Brazilian public payphone
    manufactured by icatel http://www.icatel.com.br/

    Partial schematics (drawn based on PCB inspection) available at:
    https://github.com/garoa/Icatel/blob/master/doc/icatel.pdf

    Driver by Felipe Sanches <juca@members.fsf.org>

    Changelog:

    2014 DEC 14 [Felipe Sanches]:
    * Initial driver skeleton

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/hd44780.h"
//#include "sound/speaker.h"
#include "rendlay.h"
#include "debugger.h"

class icatel_state : public driver_device
{
public:
	icatel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
	{ }

	DECLARE_READ8_MEMBER(magic_string);

	DECLARE_READ8_MEMBER(ioport_r);
	DECLARE_WRITE8_MEMBER(ioport_w);

	DECLARE_READ8_MEMBER(cn8_extension_r);
	DECLARE_WRITE8_MEMBER(cn8_extension_w);

	DECLARE_READ8_MEMBER(modem_r);
	DECLARE_WRITE8_MEMBER(modem_w);

	DECLARE_READ8_MEMBER(ci8_r);
	DECLARE_WRITE8_MEMBER(ci8_w);

	DECLARE_READ8_MEMBER(ci15_r);
	DECLARE_WRITE8_MEMBER(ci15_w);

	DECLARE_READ8_MEMBER(ci16_r);
	DECLARE_WRITE8_MEMBER(ci16_w);

	DECLARE_DRIVER_INIT(icatel);
	DECLARE_PALETTE_INIT(icatel);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

static ADDRESS_MAP_START(i80c31_prg, AS_PROGRAM, 8, icatel_state)
	AM_RANGE(0x0000, 0x7FFF) AM_MIRROR(0x8000) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(i80c31_io, AS_IO, 8, icatel_state)
	AM_RANGE(0x0000,0x3FFF) AM_RAM
	AM_RANGE(0x8000,0x8002) AM_RAM /* HACK! */
	AM_RANGE(0x8040,0x8040) AM_MIRROR(0x3F1E) AM_DEVWRITE("hd44780", hd44780_device, control_write) // not sure yet. CI12 (73LS273)
	AM_RANGE(0x8041,0x8041) AM_MIRROR(0x3F1E) AM_DEVWRITE("hd44780", hd44780_device, data_write) // not sure yet.  CI12
	AM_RANGE(0x8060,0x8060) AM_MIRROR(0x3F1F) AM_READWRITE(ci8_r, ci8_w)
	AM_RANGE(0x8080,0x8080) AM_MIRROR(0x3F1F) AM_READWRITE(ci16_r, ci16_w) // card reader (?)
	AM_RANGE(0x80C0,0x80C0) AM_MIRROR(0x3F1F) AM_READWRITE(ci15_r, ci15_w) // 74LS244 (tristate buffer)
	AM_RANGE(0xC000,0xCFFF) AM_READWRITE(cn8_extension_r, cn8_extension_w)
	AM_RANGE(0xE000,0xEFFF) AM_READWRITE(modem_r, modem_w)
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(ioport_r, ioport_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(i80c31_data, AS_DATA, 8, icatel_state)
//  AM_RANGE(0x0056,0x005A) AM_READ(magic_string) /* This is a hack! */
ADDRESS_MAP_END

DRIVER_INIT_MEMBER( icatel_state, icatel )
{
}

void icatel_state::machine_start()
{
}

void icatel_state::machine_reset()
{
}

READ8_MEMBER(icatel_state::magic_string)
{
//  logerror("read: magic_string, offset=%04X\n", offset);
	char mstr[] = "TP-OK";
	return mstr[offset%5];
}

READ8_MEMBER(icatel_state::ioport_r)
{
	switch (offset%4)
	{
		case 0: return 0xff;
		case 1: return 0x7f;
		case 2: return 0xff;
		case 3: return 0xff;
	}
	return 0;
}

WRITE8_MEMBER(icatel_state::ioport_w)
{
}

//----------------------------------------

READ8_MEMBER(icatel_state::cn8_extension_r)
{
	/* TODO: Implement-me! */
	logerror("read: cn8_extension\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::cn8_extension_w)
{
	/* TODO: Implement-me! */
	logerror("write: cn8_extension [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::modem_r)
{
	/* TODO: Implement-me! */
	logerror("read: modem\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::modem_w)
{
	/* TODO: Implement-me! */
	logerror("write: modem [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::ci8_r)
{
	/* TODO: Implement-me! */
	logerror("read: ci8\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::ci8_w)
{
	/* TODO: Implement-me! */
	logerror("write: ci8 [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::ci15_r)
{
	/* TODO: Implement-me! */
//  debugger_break(machine());
//  logerror("read: ci15\n");
	return (1 << 3) | (1 << 0);
}

WRITE8_MEMBER(icatel_state::ci15_w)
{
	/* TODO: Implement-me! */
	logerror("write: ci15 [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::ci16_r)
{
	/* TODO: Implement-me! */
	// seems to be the card reader.
	logerror("read: ci16\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::ci16_w)
{
	/* TODO: Implement-me! */
	// seems to be the card reader.
	logerror("write: ci16 [%02x]\n", data);
}

//----------------------------------------

PALETTE_INIT_MEMBER(icatel_state, icatel)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static const gfx_layout prot_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( icatel )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, prot_charlayout, 0, 1 )
GFXDECODE_END

static HD44780_PIXEL_UPDATE(icatel_pixel_update)
{
	if ( pos < 16 && line==0 )
	{
		bitmap.pix16(y, pos*6 + x) = state;
	}

	if ( pos >= 64 && pos < 80 && line==0 )
	{
		bitmap.pix16(y+9,(pos-64)*6 + x) = state;
	}
}

static MACHINE_CONFIG_START( icatel, icatel_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C31, XTAL_2_097152MHz)
	MCFG_CPU_PROGRAM_MAP(i80c31_prg)
	MCFG_CPU_DATA_MAP(i80c31_data)
	MCFG_CPU_IO_MAP(i80c31_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(6*16, 9*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16-1, 0, 9*2-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(icatel_state, icatel)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", icatel)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)
	MCFG_HD44780_PIXEL_UPDATE_CB(icatel_pixel_update)
MACHINE_CONFIG_END

ROM_START( icatel )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "icatel_tpci_em._4_v16.05.ci14",  0x00000, 0x8000, CRC(d310586e) SHA1(21736ad5a06cf9695f8cc5ff2dc2d19b101504f5) )
ROM_END

/*    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT     CLASS         INIT    COMPANY  FULLNAME                       FLAGS */
COMP( 1995, icatel,   0,      0,      icatel,     0,        icatel_state, icatel, "Icatel", "TPCI (Brazilian public payphone)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
/*The hardware was clearly manufactured in 1995. There's no evindence of the actual date of the firmware.*/
