// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Hazeltine Esprit terminals.

Espirit: R6502P, R6531, HD46505RP, MC6850P, 16.5888

Espirit3: 2x R6551AP, HD46850P (6850), R6502BP, R6545-1AP, R6522AP, RO-3-9333B, 1.8432, 17.9712


************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/input_merger.h"
#include "machine/6850acia.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

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

	void esprit(machine_config &config);
	void esprit3(machine_config &config);

	void init_init();

private:
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	void mem3_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	optional_device<palette_device> m_palette;
};

void esprit_state::mem_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0058, 0x0058).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0059, 0x0059).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0078, 0x0079).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0080, 0x00ff).mirror(0x100).ram();
	//map(0x0780, 0x078e).rw("rrioc", FUNC(r6531_device::io_r), FUNC(r6531_device::io_w));
	map(0x2000, 0x27ff).ram().share("videoram");
	map(0x7000, 0x7fff).rom().region("roms", 0);
}

void esprit_state::mem3_map(address_map &map)
{
	map(0x0000, 0x202f).ram();
	map(0x2030, 0x3fff).ram().share("videoram"); // it might start at 3000
	map(0x81c0, 0x81c0).rw("crtc", FUNC(r6545_1_device::status_r), FUNC(r6545_1_device::address_w));
	map(0x81c1, 0x81c1).rw("crtc", FUNC(r6545_1_device::register_r), FUNC(r6545_1_device::register_w));
	map(0x93c0, 0x93c1).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x95c0, 0x95c3).rw("acia1", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x99c0, 0x99c3).rw("acia2", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xb1c0, 0xb1cf).m("via", FUNC(via6522_device::map));
	map(0xe000, 0xffff).rom().region("roms", 0);
}

static INPUT_PORTS_START( esprit )
INPUT_PORTS_END

MC6845_UPDATE_ROW(esprit_state::crtc_update_row)
{
	rgb_t const *const pens = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		uint16_t gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0x1ff : 0);

		/* Display a scanline of a character (9 pixels) */
		*p++ = pens[BIT(gfx, 8)];
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

MC6845_ON_UPDATE_ADDR_CHANGED(esprit_state::crtc_update_addr)
{
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

static GFXDECODE_START( gfx_esprit )
	GFXDECODE_ENTRY( "chargen", 0x0000, esprit_charlayout, 0, 1 )
GFXDECODE_END

void esprit_state::init_init()
{
	// chargen is incomplete, copy the first half into the vacant second half
	for (u16 i = 0; i < 0x800; i++)
		m_p_chargen[0x800 | i] = m_p_chargen[i];
}


void esprit_state::esprit(machine_config &config)
{
	M6502(config, m_maincpu, 16.5888_MHz_XTAL / 18); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &esprit_state::mem_map);

	//R6531(config, "rrioc", 16.5888_MHz_XTAL / 18);

	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.irq_handler().set_inputline(m_maincpu, m6502_device::NMI_LINE);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(16.5888_MHz_XTAL, 900, 0, 720, 307, 0, 288);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_esprit);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* Devices */
	mc6845_device &crtc(MC6845(config, "crtc", 16.5888_MHz_XTAL / 9));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(9);
	crtc.set_update_row_callback(FUNC(esprit_state::crtc_update_row));
}

void esprit_state::esprit3(machine_config &config)
{
	M6502(config, m_maincpu, 17.9712_MHz_XTAL / 18); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &esprit_state::mem3_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	mos6551_device &acia1(MOS6551(config, "acia1", 17.9712_MHz_XTAL / 18));
	acia1.set_xtal(1.8432_MHz_XTAL);
	acia1.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));

	mos6551_device &acia2(MOS6551(config, "acia2", 17.9712_MHz_XTAL / 18));
	acia2.set_xtal(1.8432_MHz_XTAL);
	acia2.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	via6522_device &via(MOS6522(config, "via", 17.9712_MHz_XTAL / 18));
	via.irq_handler().set_inputline(m_maincpu, m6502_device::NMI_LINE);
	via.writepb_handler().set("acia", FUNC(acia6850_device::write_rxc)).bit(7);
	via.writepb_handler().append("acia", FUNC(acia6850_device::write_txc)).bit(7);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(17.9712_MHz_XTAL, 936, 0, 720, 320, 0, 288);
	screen.set_screen_update("crtc", FUNC(r6545_1_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_esprit);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	r6545_1_device &crtc(R6545_1(config, "crtc", 17.9712_MHz_XTAL / 9));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(9);
	crtc.set_update_row_callback(FUNC(esprit_state::crtc_update_row));
	crtc.set_on_update_addr_change_callback(FUNC(esprit_state::crtc_update_addr));
	crtc.out_hsync_callback().set("via", FUNC(via6522_device::write_pb6)).invert();
}

ROM_START( esprit )
	// Esprit
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "hazeltine_esprit.u19",    0x0000, 0x1000, CRC(6fdec792) SHA1(a1d1d68c8793e7e15ab5cd17682c299dff3985cb) )
	ROM_REGION( 0x1000, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "hazeltine_esprit.u26",    0x0000, 0x0804, CRC(93f45f13) SHA1(1f493b44124c348759469e24fdfa8b7c52fe6fac) )
	ROM_REGION( 0x0800, "rrioc", 0 )
	ROM_LOAD( "r3198-11.u20",            0x0000, 0x0800, NO_DUMP ) // internal ROM apparently unused
ROM_END

ROM_START( esprit3 )
	// Esprit III
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD( "hazeltine_espritiii.u5",  0x0000, 0x2000, CRC(fd63dad1) SHA1(b2a3e7db8480b28cab2b2834ad89fb6257f13cba) )
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "hazeltine_espritiii.u19", 0x0000, 0x1000, CRC(33e4a8ef) SHA1(e19c84a3c5f94812928ea84bab3ede7970dd5e72) )
ROM_END

} // anonymous namespace


COMP( 1981, esprit,  0,      0, esprit,  esprit, esprit_state, init_init,  "Hazeltine", "Esprit",     MACHINE_IS_SKELETON )
COMP( 1981, esprit3, esprit, 0, esprit3, esprit, esprit_state, empty_init, "Hazeltine", "Esprit III", MACHINE_IS_SKELETON )
