// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 207/100

    VT-100 compatible terminal

    Hardware:
    - HD68B09EP
    - 3x HM6116LP-4 (+ 3 empty sockets), 2x TC5517BPL-20
    - R6545-1AP CRTC
    - MC2681P DUART
    - MC68B50P ACIA
    - M58321 RTC
    - 19.7184 MHz XTAL, 3.6864 MHz XTAL

    TODO:
    - Redump ROM 207_100_2.bin

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/mc68681.h"
#include "machine/msm58321.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class informer_207_100_state : public driver_device
{
public:
	informer_207_100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_duart(*this, "duart"),
		m_acia(*this, "acia"),
		m_rtc(*this, "rtc"),
		m_ram(*this, "ram"),
		m_chargen(*this, "chargen")
	{ }

	void informer_207_100(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<r6545_1_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scn2681_device> m_duart;
	required_device<acia6850_device> m_acia;
	required_device<msm58321_device> m_rtc;
	required_shared_ptr<uint8_t> m_ram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map) ATTR_COLD;

	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	MC6845_UPDATE_ROW(crtc_update_row);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void informer_207_100_state::mem_map(address_map &map)
{
	map(0x0000, 0x27ff).ram().share("ram");
	map(0xc000, 0xffff).rom().region("maincpu", 0);
	map(0xff00, 0xff0f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xff20, 0xff20).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xff21, 0xff21).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xff40, 0xff41).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( informer_207_100 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

MC6845_ON_UPDATE_ADDR_CHANGED( informer_207_100_state::crtc_addr )
{
//  m_video_update_address = address;
}

MC6845_UPDATE_ROW( informer_207_100_state::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (int x = 0; x < x_count; x++)
	{
//      uint8_t attr = m_ram[ma + x * 2 + 0];
		uint8_t code = m_ram[ma + x * 2 + 1];
		uint8_t data = m_chargen[(code << 4) + ra];

		if (x == cursor_x)
			data = 0xff;

		// draw 8 pixels of the character
		bitmap.pix(y, x * 8 + 7) = pen[BIT(data, 0)];
		bitmap.pix(y, x * 8 + 6) = pen[BIT(data, 1)];
		bitmap.pix(y, x * 8 + 5) = pen[BIT(data, 2)];
		bitmap.pix(y, x * 8 + 4) = pen[BIT(data, 3)];
		bitmap.pix(y, x * 8 + 3) = pen[BIT(data, 4)];
		bitmap.pix(y, x * 8 + 2) = pen[BIT(data, 5)];
		bitmap.pix(y, x * 8 + 1) = pen[BIT(data, 6)];
		bitmap.pix(y, x * 8 + 0) = pen[BIT(data, 7)];
	}
}

static const gfx_layout char_layout =
{
	8,12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void informer_207_100_state::machine_start()
{
}

void informer_207_100_state::machine_reset()
{
	// start executing somewhere sane
	m_maincpu->set_pc(0xc000);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void informer_207_100_state::informer_207_100(machine_config &config)
{
	MC6809(config, m_maincpu, 19.7184_MHz_XTAL / 4); // unknown clock divisor
	m_maincpu->set_addrmap(AS_PROGRAM, &informer_207_100_state::mem_map);

	ACIA6850(config, m_acia, 0); // unknown clock

	SCN2681(config, m_duart, 0); // unknown clock

	MSM58321(config, m_rtc, 32.768_kHz_XTAL);

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(19.7184_MHz_XTAL, 832, 0, 640, 316, 0, 300);
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	R6545_1(config, m_crtc, 19.7184_MHz_XTAL / 8); // unknown clock divisor
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_on_update_addr_change_callback(FUNC(informer_207_100_state::crtc_addr));
	m_crtc->set_update_row_callback(FUNC(informer_207_100_state::crtc_update_row));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( in207100 )
	ROM_REGION(0x4000, "maincpu", 0)
	// 79505-001  V2.00  <unreadable>
	ROM_LOAD("79505-001.bin", 0x0000, 0x2000, CRC(272ebfac) SHA1(b6b9dc523028ace9e5a210e908de2260f36dde4a))
	// <Label lost>
	ROM_LOAD("207_100_2.bin", 0x2000, 0x2000, BAD_DUMP CRC(848d1b45) SHA1(77dd68951ac85e5dc51b51db002d90863b0fce43))

	ROM_REGION(0x1000, "chargen", 0)
	// 79496  REV 1.01  12-29-83
	ROM_LOAD("79496.bin", 0x0000, 0x1000, CRC(930ac23a) SHA1(74e6bf81b60e3504cb2b9f14a33e7c3e367dc825))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT   COMPAT  MACHINE           INPUT             CLASS                   INIT        COMPANY     FULLNAME            FLAGS
COMP( 1983, in207100, 0,       0,      informer_207_100, informer_207_100, informer_207_100_state, empty_init, "Informer", "Informer 207/100", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
