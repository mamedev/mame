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

***************************************************************************/

#include "emu.h"
#include "informer_207_100_kbd.h"

#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/input_merger.h"
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
	map(0x8000, 0xffff).rom().region("maincpu", 0);
	map(0xff00, 0xff0f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xff20, 0xff20).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xff21, 0xff21).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xff40, 0xff41).w(m_acia, FUNC(acia6850_device::write));
	map(0xff42, 0xff43).r(m_acia, FUNC(acia6850_device::read));
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
		bitmap.pix(y, x * 10 + 9) = pen[0];
		bitmap.pix(y, x * 10 + 8) = pen[0];
		bitmap.pix(y, x * 10 + 7) = pen[BIT(data, 0)];
		bitmap.pix(y, x * 10 + 6) = pen[BIT(data, 1)];
		bitmap.pix(y, x * 10 + 5) = pen[BIT(data, 2)];
		bitmap.pix(y, x * 10 + 4) = pen[BIT(data, 3)];
		bitmap.pix(y, x * 10 + 3) = pen[BIT(data, 4)];
		bitmap.pix(y, x * 10 + 2) = pen[BIT(data, 5)];
		bitmap.pix(y, x * 10 + 1) = pen[BIT(data, 6)];
		bitmap.pix(y, x * 10 + 0) = pen[BIT(data, 7)];
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
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void informer_207_100_state::informer_207_100(machine_config &config)
{
	MC6809E(config, m_maincpu, 19.7184_MHz_XTAL / 10); // clock divisor guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &informer_207_100_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_duart->b_tx_cb().set("kbd", FUNC(informer_207_100_kbd_device::rxd_w));
	m_duart->outport_cb().set(m_acia, FUNC(acia6850_device::write_txc)).bit(3);
	m_duart->outport_cb().append(m_acia, FUNC(acia6850_device::write_rxc)).bit(3);

	ACIA6850(config, m_acia);
	m_acia->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	informer_207_100_kbd_device &kbd(INFORMER_207_100_KBD(config, "kbd"));
	kbd.txd_callback().set(m_duart, FUNC(scn2681_device::rx_b_w));

	MSM58321(config, m_rtc, 32.768_kHz_XTAL);

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(19.7184_MHz_XTAL, 1040, 0, 800, 316, 0, 300);
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	R6545_1(config, m_crtc, 19.7184_MHz_XTAL / 10); // clock divisor guessed
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(10);
	m_crtc->set_on_update_addr_change_callback(FUNC(informer_207_100_state::crtc_addr));
	m_crtc->set_update_row_callback(FUNC(informer_207_100_state::crtc_update_row));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( in207100 )
	ROM_REGION(0x8000, "maincpu", 0)
	// <Label lost; EPROM type is SEEQ DQ1533-300 2764-30> (1 empty socket is directly to left)
	ROM_LOAD("79532-002.bin", 0x0000, 0x2000, CRC(848d1b45) SHA1(77dd68951ac85e5dc51b51db002d90863b0fce43))
	// 79505-002  V2.00  ED2F -2-3
	ROM_LOAD("79505-002.bin", 0x4000, 0x4000, CRC(3dfae553) SHA1(ae6849cacb07792769f93aa736f5603e28fa8635))

	ROM_REGION(0x1000, "chargen", 0)
	// 79496  REV 1.01  12-29-83
	ROM_LOAD("79496-101.bin", 0x0000, 0x1000, CRC(930ac23a) SHA1(74e6bf81b60e3504cb2b9f14a33e7c3e367dc825))

	ROM_REGION(0x220, "proms", 0)
	ROM_LOAD("82s131_z35.bin", 0x000, 0x200, CRC(5a002c87) SHA1(59e51ac7106f0925959655b1df1d8452db76943e))
	ROM_LOAD("82s123_z5.bin", 0x200, 0x020, CRC(eec80ecf) SHA1(fb58086229aed8187ecf0d24573b7e71980f271c))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT   COMPAT  MACHINE           INPUT             CLASS                   INIT        COMPANY     FULLNAME            FLAGS
COMP( 1983, in207100, 0,       0,      informer_207_100, informer_207_100, informer_207_100_state, empty_init, "Informer", "Informer 207/100", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
