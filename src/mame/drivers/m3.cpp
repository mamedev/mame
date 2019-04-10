// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-20 Skeleton

LSI M3

From disassembly: chips: Z80, 6845, 8251, 2x 8255, Z80CTC


************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

class m3_state : public driver_device
{
public:
	m3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_palette(*this, "palette")
	{ }

	void m3(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	required_device<palette_device> m_palette;
};

void m3_state::mem_map(address_map &map)
{
	map(0x0000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().share("videoram");
	map(0xf000, 0xffff).rom().region("roms", 0);
}

void m3_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x84, 0x84).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x85, 0x85).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

static INPUT_PORTS_START( m3 )
INPUT_PORTS_END

MC6845_UPDATE_ROW( m3_state::crtc_update_row )
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();
	uint8_t chr,gfx,inv;
	uint16_t mem,x;
	uint32_t *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		if (BIT(chr, 7))
		{
			inv ^= 0xff;
			chr &= 0x7f;
		}

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character (8 pixels) */
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
static const gfx_layout charlayout =
{
	7, 16,                  /* 7 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_f4disp )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

void m3_state::machine_reset()
{
	m_maincpu->set_pc(0xf000);
}

void m3_state::m3(machine_config &config)
{
	Z80(config, m_maincpu, 2'000'000); // no idea of clock.
	m_maincpu->set_addrmap(AS_PROGRAM, &m3_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &m3_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not correct
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_f4disp);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* Devices */
	mc6845_device &crtc(MC6845(config, "crtc", 2'000'000)); // clk unknown
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(7);
	crtc.set_update_row_callback(FUNC(m3_state::crtc_update_row), this);
}

ROM_START( m3 )
	ROM_REGION( 0x3000, "roms", 0 )
	ROM_LOAD( "bootstrap_prom,034.bin",    0x0000, 0x0800, CRC(7fdb051e) SHA1(7aa24d4f44b6a0c8f7f647667f4997432c186cac) )
	ROM_LOAD( "monitor_prom_v1.7_2015-12-09.bin", 0x0800, 0x0800, CRC(85b5c541) SHA1(92b4ec87a4d0d8c0f7b49eec0c5457f237de0a01) )

	ROM_REGION( 0x0800, "chargen", 0 ) // bit 7 set on every byte - bad?
	ROM_LOAD( "6845crt_font_prom,033.bin", 0x0000, 0x0800, CRC(cc29f664) SHA1(4197530d9455d665fd4773f95bb6394f6b056dec) )

	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "keyboard_prom,032.bin",     0x0000, 0x0800, CRC(21548355) SHA1(ee4ce4af9c78474263dd58e0f19e79e5b00926fa) )
ROM_END

COMP( 19??, m3, 0, 0, m3, m3, m3_state, empty_init, "LSI", "M3", MACHINE_IS_SKELETON )
