// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    The Visual 500 and 550 are the graphical members of the third series of display terminals released by Visual Technology.
    While they appear to use the same character generators as the text-mode-only Visual 300 and 330 and have similar detachable
    keyboards, they uniquely feature high-definition green-screen (P39 phosphor) monitors and support Tektronix 4010/4014-
    compatible graphics at a resolution of 768 x 555 pixels.

    The VT100-compatible Visual 550 was released first, and conforms to the ANSI X3.64 standard (like the Visual 300).
    The Visual 500 instead emulates the ADM3A, D200, Hazeltine 1500 and VT52 terminals (like the Visual 330).

***********************************************************************************************************************************/

#include "emu.h"
//include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "v102_kbd.h"
#include "machine/z80sio.h"
#include "video/scn2674.h"
#include "video/upd7220.h"
#include "screen.h"


namespace {

class v550_state : public driver_device
{
public:
	v550_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_chargen(*this, "chargen")
		, m_usart(*this, "usart")
	{ }

	void v550(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void pvtc_char_map(address_map &map) ATTR_COLD;
	void pvtc_attr_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_chargen;
	required_device<i8251_device> m_usart;
};


void v550_state::mem_map(address_map &map)
{
	map(0x0000, 0x7bff).rom().region("maincpu", 0);
	map(0x7c00, 0x7fff).ram().share("nvram"); // actually 4 bits wide
	map(0x8000, 0x87ff).ram();
}

void v550_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("gdc", FUNC(upd7220_device::read), FUNC(upd7220_device::write));
	map(0x10, 0x10).w("brg1", FUNC(com8116_device::stt_str_w));
	map(0x20, 0x23).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x31).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x40, 0x40).rw("mpsc", FUNC(upd7201_device::da_r), FUNC(upd7201_device::da_w));
	map(0x41, 0x41).rw("mpsc", FUNC(upd7201_device::ca_r), FUNC(upd7201_device::ca_w));
	map(0x48, 0x48).rw("mpsc", FUNC(upd7201_device::db_r), FUNC(upd7201_device::db_w));
	map(0x49, 0x49).rw("mpsc", FUNC(upd7201_device::cb_r), FUNC(upd7201_device::cb_w));
	map(0x50, 0x50).w("brg2", FUNC(com8116_device::stt_str_w));
	map(0x60, 0x67).rw("pvtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x70, 0x70).rw("pvtc", FUNC(scn2672_device::buffer_r), FUNC(scn2672_device::buffer_w));
	map(0x71, 0x71).rw("pvtc", FUNC(scn2672_device::attr_buffer_r), FUNC(scn2672_device::attr_buffer_w));
}

void v550_state::pvtc_char_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
}

void v550_state::pvtc_attr_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
}


INPUT_PORTS_START(v550)
INPUT_PORTS_END


void v550_state::machine_start()
{
	m_usart->write_cts(0);
}

void v550_state::v550(machine_config &config)
{
	Z80(config, m_maincpu, 34.846_MHz_XTAL / 16); // NEC D780C (2.177875 MHz verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &v550_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &v550_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // NEC D444C-2 + battery

	upd7220_device &gdc(UPD7220(config, "gdc", 34.846_MHz_XTAL / 16)); // NEC D7220D (2.177875 MHz verified)
	gdc.set_screen("screen");

	I8255(config, "ppi"); // NEC D8255AC-5

	I8251(config, m_usart, 34.846_MHz_XTAL / 16); // NEC D8251AC
	m_usart->txd_handler().set("keyboard", FUNC(v550_keyboard_device::write_rxd));
	m_usart->rxrdy_handler().set("mainint", FUNC(input_merger_device::in_w<1>));

	upd7201_device& mpsc(UPD7201(config, "mpsc", 34.846_MHz_XTAL / 16)); // NEC D7201C
	mpsc.out_int_callback().set("mainint", FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ANY_HIGH(config, "mainint").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	com8116_device &brg1(COM8116_020(config, "brg1", 1.8432_MHz_XTAL)); // SMC COM8116T-020
	brg1.ft_handler().set("mpsc", FUNC(upd7201_device::txcb_w));
	brg1.ft_handler().append("mpsc", FUNC(upd7201_device::rxcb_w));
	brg1.fr_handler().set("usart", FUNC(i8251_device::write_txc));
	brg1.fr_handler().append("usart", FUNC(i8251_device::write_rxc));

	com8116_device &brg2(COM8116_020(config, "brg2", 1.8432_MHz_XTAL)); // SMC COM8116T-020
	brg2.ft_handler().set("mpsc", FUNC(upd7201_device::txca_w));
	brg2.fr_handler().set("mpsc", FUNC(upd7201_device::rxca_w));

	v550_keyboard_device &keyboard(V550_KEYBOARD(config, "keyboard"));
	keyboard.txd_callback().set(m_usart, FUNC(i8251_device::write_rxd));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(34.846_MHz_XTAL, 19 * 102, 0, 19 * 80, 295, 0, 272);
	m_screen->set_screen_update(FUNC(v550_state::screen_update));

	scn2672_device &pvtc(SCN2672(config, "pvtc", 34.846_MHz_XTAL / 19));
	pvtc.set_addrmap(0, &v550_state::pvtc_char_map);
	pvtc.set_addrmap(1, &v550_state::pvtc_attr_map);
	pvtc.set_character_width(19);
	pvtc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	pvtc.set_screen("screen");
	// SCB2673 clock verified at 17.423 MHz
}


ROM_START( v550 )
	// Silkscreened on bottom left of PCB: "PA016-A REV B"

	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("e244-001_r07_u42.bin", 0x0000, 0x2000, CRC(d18a8b62) SHA1(7faf8a9f1ae3148adacbff960a2663793710cef2))
	ROM_LOAD("e244-002_r07_u43.bin", 0x2000, 0x2000, CRC(1b62db47) SHA1(7ad69aea6088545d843c2e95737895f81c269ccc))
	ROM_LOAD("e244-003_r07_u44.bin", 0x4000, 0x2000, CRC(f6d6f734) SHA1(a9efa8ebe86addb77872dbe9863ea7f75b33a2b9))
	ROM_LOAD("e244-017_r07_u45.bin", 0x6000, 0x2000, CRC(b0dcd535) SHA1(9237723d01a720217f50f756bb55c7e5ed05a594))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("e242-085_r03_u97.bin", 0x0000, 0x1000, CRC(8a491cee) SHA1(d8a9546a7dd2ffc0a5e54524ee16068dde56975c))
ROM_END

} // anonymous namespace


COMP( 1982, v550, 0, 0, v550, v550, v550_state, empty_init, "Visual Technology", "Visual 550", MACHINE_IS_SKELETON )
