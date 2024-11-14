// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for ITT Courier 1700 compact IBM 3278-compatible display terminal.

Next to the CPU is a 40-pin DIP marked:

    AMI 8421MAF
    2651-P2
    C02805
    PHILIPPINES

This device may be related to the Intel 8251, but it is definitely not a SCN2651 equivalent.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "itt1700_kbd.h"
#include "video/mc6845.h"
#include "screen.h"
#include "speaker.h"


namespace {

class itt1700_state : public driver_device
{
public:
	itt1700_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram")
		, m_chargen(*this, "chargen")
	{
	}

	void itt1700(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_videoram;
	required_region_ptr<u8> m_chargen;
};


MC6845_UPDATE_ROW(itt1700_state::update_row)
{
	u32 *px = &bitmap.pix(y);
	u16 page = 0x800;

	for (int i = 0; i < x_count; i++)
	{
		u8 chr = m_videoram[((ma + i) & 0x7ff) | page];
		u16 dots = m_chargen[u16(chr) << 4 | ra] << 1;
		rgb_t fg = rgb_t::white();
		rgb_t bg = rgb_t::black();

		for (int n = 9; n > 0; n--, dots <<= 1)
			*px++ = BIT(dots, 8) ? fg : bg;
	}
}

void itt1700_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0);
	map(0x2000, 0x3fff).ram().share("videoram");
	map(0x4000, 0x7fff).ram();
	map(0x8000, 0x8000).nopr();
}

void itt1700_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x11, 0x11).w("crtc", FUNC(hd6845s_device::register_w));
	map(0x20, 0x21).rw("upi", FUNC(i8741a_device::upi41_master_r), FUNC(i8741a_device::upi41_master_w));
}

static INPUT_PORTS_START(itt1700)
INPUT_PORTS_END

void itt1700_state::itt1700(machine_config &config)
{
	Z80(config, m_maincpu, 17.35_MHz_XTAL / 5); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &itt1700_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &itt1700_state::io_map);

	upi41_cpu_device &upi(I8741A(config, "upi", 16.6698_MHz_XTAL / 3)); // clock guessed
	upi.p1_out_cb().set("keyboard", FUNC(itt1700_keyboard_device::clock_w)).bit(0);
	upi.p1_out_cb().append("keyboard", FUNC(itt1700_keyboard_device::line1_w)).bit(1);
	upi.p1_out_cb().append("keyboard", FUNC(itt1700_keyboard_device::line2_w)).bit(2);
	// P20-P22 = PWM LEDs? (too high-frequency to be speaker output)
	upi.t0_in_cb().set("keyboard", FUNC(itt1700_keyboard_device::sense_r));

	ITT1700_KEYBOARD(config, "keyboard");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16.6698_MHz_XTAL, 882, 0, 720, 315, 0, 300);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", 16.6698_MHz_XTAL / 9)); // on video board
	crtc.set_char_width(9);
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_update_row_callback(FUNC(itt1700_state::update_row));
}

ROM_START(itt1700)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("173562-007_2732_u32.bin", 0x0000, 0x1000, CRC(b1f4b349) SHA1(89ca344b89a0a746cdedcefdf5ff594287a5aa92))

	ROM_REGION(0x0400, "upi", 0)
	ROM_LOAD("173561-002_8741_u17.bin", 0x0000, 0x0400, CRC(7af333fa) SHA1(1edd77e9e32e3c7280f0906625e06718a0ba109f))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("173563-001_2764_u56.bin", 0x0000, 0x2000, CRC(8ca58ab9) SHA1(b92e3985dd13afcf63dbb279f5fb9668d5eb645b))
ROM_END

} // anonymous namespace


COMP(1983, itt1700, 0, 0, itt1700, itt1700, itt1700_state, empty_init, "ITT Courier", "ITT 1700", MACHINE_IS_SKELETON)
