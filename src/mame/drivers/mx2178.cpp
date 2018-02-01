// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************************************

Skeleton driver for Memorex 2178

Chips: Z80A, N8X305N, 2x B8452A, 4x D4016C-3, 2x HD468A50P, HD46505SP-1
Crystal: 18.8696MHz
There is a large piezo-beeper.

TODO:
- Connect up the beeper
- Unknown memory i/o C000, 4000
- Need schematic / tech manual
- Doesn't seem to be any dips, looks like all settings and modes are controlled by keystrokes.
- 8X305 is a 16-bit bipolar processor which appears to use four external PROMs (undumped). It
  would communicate with the Z80 via a common 8-bit I/O bus. No idea what it is used for here,
  but in another system it acts as the floppy disk controller.
- Debug trick: set pc=809 to see the test menu.
- It shows the status line but keystrokes are ignored. After 20 minutes of inactivity, the screen
  goes blank. Pressing a key will restore it.

***************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "cpu/8x300/8x300.h" // device = N8X300
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "video/mc6845.h"
#include "screen.h"


class mx2178_state : public driver_device
{
public:
	mx2178_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	MC6845_UPDATE_ROW(crtc_update_row);

	void mx2178(machine_config &config);
	void mx2178_io(address_map &map);
	void mx2178_mem(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_p_videoram;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

ADDRESS_MAP_START(mx2178_state::mx2178_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(mx2178_state::mx2178_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("acia1", acia6850_device, read, write)
	AM_RANGE(0xa0, 0xa1) AM_DEVREADWRITE("acia2", acia6850_device, read, write)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( mx2178 )
INPUT_PORTS_END

MC6845_UPDATE_ROW( mx2178_state::crtc_update_row )
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();
	uint8_t chr,gfx;
	uint16_t mem,x;
	uint32_t *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0xff : 0);

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7)];
		*p++ = pens[BIT(gfx, 6)];
		*p++ = pens[BIT(gfx, 5)];
		*p++ = pens[BIT(gfx, 4)];
		*p++ = pens[BIT(gfx, 3)];
		*p++ = pens[BIT(gfx, 2)];
		*p++ = pens[BIT(gfx, 1)];
		*p++ = pens[BIT(gfx, 0)];
	}
}

/* F4 Character Displayer */
static const gfx_layout mx2178_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 },
	8*16                    /* every char takes 8 bytes */
};

static GFXDECODE_START( mx2178 )
	GFXDECODE_ENTRY( "chargen", 0x0000, mx2178_charlayout, 0, 1 )
GFXDECODE_END

void mx2178_state::machine_reset()
{
}

MACHINE_CONFIG_START(mx2178_state::mx2178)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(18'869'600) / 5) // guess
	MCFG_CPU_PROGRAM_MAP(mx2178_mem)
	MCFG_CPU_IO_MAP(mx2178_io)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not correct
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mx2178)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL(18'869'600) / 8) // clk unknown
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(mx2178_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, XTAL(18'869'600) / 30)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("acia1", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("acia1", acia6850_device, write_rxc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("acia2", acia6850_device, write_rxc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("acia2", acia6850_device, write_rxc))

	MCFG_DEVICE_ADD("acia1", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232a", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia1", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia1", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia2", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232b", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia2", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia2", acia6850_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mx2178 )
	ROM_REGION(0x2000, "roms", 0) // MBM2764-25
	ROM_LOAD( "96274139.u9", 0x000000, 0x002000, CRC(eb471a27) SHA1(433abefd1a72653d0bf35bcaaeccf9943b96260b) )

	ROM_REGION(0x800, "proms", 0) // MB7122E - actual mapping not known
	ROMX_LOAD( "96270350.q2", 0x0000, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "96270368.r2", 0x0000, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "96270376.s2", 0x0001, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "96270384.t2", 0x0001, 0x0400, NO_DUMP, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )

	ROM_REGION(0x1000, "chargen", 0) // D2732A-3
	ROM_LOAD( "96273883.c7", 0x000000, 0x001000, CRC(8311fadd) SHA1(573bbad23e893ad9374edc929642dc1cba3452d2) )
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE         INIT  COMPANY    FULLNAME        FLAGS
COMP( 1984, mx2178, 0,      0,       mx2178,    mx2178, mx2178_state, 0,    "Memorex", "Memorex 2178", MACHINE_IS_SKELETON )
