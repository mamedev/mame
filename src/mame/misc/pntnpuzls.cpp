// license:BSD-3-Clause
// copyright-holders:

/*
Paint 'N Puzzle Super (or Super Paint 'N Puzzle)
VIDEO PUZZLE V1.0 PCB
(C) 1994 Green Concepts International

Main components:
N80286-12 CPU
3.579545 XTAL (near CPU and empty socket at u11)
27.50000 MHz XTAL
4x GM76C256ALL-70 RAM
GM68B45S CRTC
HM86171-80 RAMDAC
KS82C54 PIT
W82C59 PIC
W86C450 UART
1.8432 MHz XTAL (near UART)
Card connector
Ticket connector
*/

#include "emu.h"

#include "cpu/i86/i286.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "video/mc6845.h"
#include "video/ramdac.h"

#include "screen.h"


namespace {

class pntnpuzls_state : public driver_device
{
public:
	pntnpuzls_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_font(*this, "font"),
		m_font_bank(*this, "font_bank"),
		m_vram(*this, "vram"),
		m_crtc(*this, "crtc"),
		m_ramdac(*this, "ramdac"),
		m_palette(*this, "palette")
	{ }

	void pntnpuzls(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_memory_region m_font;
	required_memory_bank m_font_bank;
	required_shared_ptr<uint16_t> m_vram;
	required_device<mc6845_device> m_crtc;
	required_device<ramdac_device> m_ramdac;
	required_device<palette_device> m_palette;

	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


uint32_t pntnpuzls_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void pntnpuzls_state::program_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram();
	map(0x0d0000, 0x0dffff).ram().share("vram"); // TODO: banked?
	map(0x0e0000, 0x0effff).bankr("font_bank");
	map(0x0f0000, 0x0fffff).rom().region("bios", 0);
	map(0xff0000, 0xffffff).rom().region("bios", 0);
}

void pntnpuzls_state::io_map(address_map &map)
{
	map(0x0308, 0x0308).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// TODO: may view select on bits 7-3
			logerror("$308 %02x\n", data);
			m_font_bank->set_entry(data & 7);
		})
	);

	map(0x0310, 0x0310).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0312, 0x0312).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0314, 0x0317).rw("pic", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);

	map(0x0320, 0x0320).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0322, 0x0322).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0324, 0x0324).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0328, 0x032f).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);

	map(0x0340, 0x034f).rw("uart", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w)).umask16(0x00ff);
}

// no DIP switches on PCB
static INPUT_PORTS_START( pntnpuzls )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: touchscreen
INPUT_PORTS_END

void pntnpuzls_state::machine_start()
{
	m_font_bank->configure_entries(0, 8, m_font->base(), 0x10000);
}

void pntnpuzls_state::ramdac_map(address_map &map)
{
	// TODO: writes to upper bits 7-6 but decodes better as 666
	map(0x000, 0x2ff).rw(m_ramdac, FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


void pntnpuzls_state::pntnpuzls(machine_config &config)
{
	I80286(config, m_maincpu, 27'500'000 / 2); // clock / divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &pntnpuzls_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &pntnpuzls_state::io_map);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	pic8259_device &pic(PIC8259(config, "pic"));
	pic.out_int_callback().set_inputline(m_maincpu, 0);

	pit8254_device &pit(PIT8254(config, "pit"));
	pit.set_clk<0>(XTAL(3'579'545)); // clocks?
	pit.out_handler<0>().set("pic", FUNC(pic8259_device::ir2_w));
	pit.set_clk<1>(XTAL(3'579'545));
	pit.set_clk<2>(XTAL(3'579'545));


	ins8250_device &uart(INS8250(config, "uart", 1.8432_MHz_XTAL));
	uart.out_int_callback().set("pic", FUNC(pic8259_device::ir4_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(pntnpuzls_state::screen_update));

	PALETTE(config, "palette").set_entries(0x100); // TODO

	RAMDAC(config, m_ramdac, 0, "palette");
	m_ramdac->set_addrmap(0, &pntnpuzls_state::ramdac_map);

	mc6845_device &crtc(MC6845(config, "crtc", 27'500'000 / 30)); // clock / divider not verified
	crtc.set_char_width(8);
	crtc.set_show_border_area(false);
	crtc.set_screen("screen");

	// TODO: sound? missing chip at u11?
}

ROM_START( pntnpuzls )
	ROM_REGION16_LE( 0x10000, "bios", 0 )
	ROM_LOAD16_BYTE( "u8.u8", 0x0000, 0x8000, CRC(269c2aed) SHA1(466da89a1a4c69668d2e0d6e49f3e20ec5b03d29) )
	ROM_LOAD16_BYTE( "u7.u7", 0x0001, 0x8000, CRC(39ca5f74) SHA1(b9256e296ff248ddc1d5c76fc5a1748a6a86ac80) )

	ROM_REGION16_LE( 0x80000, "font", 0 )
	ROM_LOAD16_BYTE( "pnp_1.1d_font_even.u6", 0x00000, 0x40000, CRC(c6af5a61) SHA1(737bf2b2e4e42124bcde60e7a00be42f1b7f32d2) )
	ROM_LOAD16_BYTE( "pnp_1.1d_font_odd.u5",  0x00001, 0x40000, CRC(6c0aa161) SHA1(190ff150d149101ab26b41640acdac023c3f11b1) )
ROM_END

} // anonymous namespace


GAME( 1994, pntnpuzls, 0, pntnpuzls, pntnpuzls, pntnpuzls_state, empty_init, ROT90, "Century Vending / Green Concepts International", "Paint 'N Puzzle Super", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
