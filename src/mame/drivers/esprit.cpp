// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Hazeltine Esprit terminals.

Espirit: R6502P, R6531, HD46505RP, MC6850P, 16.5888

Espirit3: 2x R6551AP, HD46850P (6850), R6502BP, R6545-1AP, R6522AP, 1.8432, 17.9712


************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "screen.h"

class esprit_state : public driver_device
{
public:
	esprit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_palette(*this, "palette")
	{ }

	MC6845_UPDATE_ROW(crtc_update_row);
	DECLARE_DRIVER_INIT(init);

	void esprit(machine_config &config);
	void esprit3(machine_config &config);
	void mem3_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	optional_device<palette_device> m_palette;
};

ADDRESS_MAP_START(esprit_state::mem_map)
	ADDRESS_MAP_GLOBAL_MASK (0x7fff)
	AM_RANGE(0x0058,0x0058) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x0059,0x0059) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x0080,0x1fff) AM_RAM
	AM_RANGE(0x2000,0x6fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x7000,0x7fff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(esprit_state::mem3_map)
	AM_RANGE(0x0000,0x202f) AM_RAM
	AM_RANGE(0x2030,0x3fff) AM_RAM AM_SHARE("videoram") // it might start at 3000
	AM_RANGE(0x81c0,0x81c0) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x81c1,0x81c1) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xe000,0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( esprit )
INPUT_PORTS_END

MC6845_UPDATE_ROW( esprit_state::crtc_update_row )
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
static const gfx_layout esprit_charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( esprit )
	GFXDECODE_ENTRY( "chargen", 0x0000, esprit_charlayout, 0, 1 )
GFXDECODE_END

DRIVER_INIT_MEMBER( esprit_state, init )
{
	// chargen is incomplete, copy the first half into the vacant second half
	for (u16 i = 0; i < 0x800; i++)
		m_p_chargen[0x800 | i] = m_p_chargen[i];
}


MACHINE_CONFIG_START(esprit_state::esprit)
	MCFG_CPU_ADD("maincpu", M6502, 1000000) // no idea of clock
	MCFG_CPU_PROGRAM_MAP(mem_map)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not correct
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", esprit)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1000000) // clk unknown
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(esprit_state, crtc_update_row)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(esprit_state::esprit3)
	esprit(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mem3_map)
MACHINE_CONFIG_END

ROM_START( esprit )
	// Esprit
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "hazeltine_esprit.u19",    0x0000, 0x1000, CRC(6fdec792) SHA1(a1d1d68c8793e7e15ab5cd17682c299dff3985cb) )
	ROM_REGION( 0x1000, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "hazeltine_esprit.u26",    0x0000, 0x0804, CRC(93f45f13) SHA1(1f493b44124c348759469e24fdfa8b7c52fe6fac) )
ROM_END

ROM_START( esprit3 )
	// Esprit III
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD( "hazeltine_espritiii.u5",  0x0000, 0x2000, CRC(fd63dad1) SHA1(b2a3e7db8480b28cab2b2834ad89fb6257f13cba) )
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "hazeltine_espritiii.u19", 0x0000, 0x1000, CRC(33e4a8ef) SHA1(e19c84a3c5f94812928ea84bab3ede7970dd5e72) )
ROM_END

COMP( 1981, esprit,  0,         0, esprit,  esprit, esprit_state, init, "Hazeltine", "Esprit", MACHINE_IS_SKELETON )
COMP( 1981, esprit3, esprit,    0, esprit3, esprit, esprit_state, 0,    "Hazeltine", "Esprit III", MACHINE_IS_SKELETON )
