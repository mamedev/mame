// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Nihon Eniac BH-1000/Hammond GM-1000 sound module.

*******************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/m37710/m37710.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class gm1000_state : public driver_device
{
public:
	gm1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram", 0x800, ENDIANNESS_LITTLE)
	{
	}

	void gm1000(machine_config &config);

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<m37702s1_device> m_maincpu;
	memory_share_creator<u8> m_nvram;
};

HD44780_PIXEL_UPDATE(gm1000_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 24)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


void gm1000_state::mem_map(address_map &map)
{
	map(0x000400, 0x000403).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write)).umask16(0x00ff);
	map(0x004000, 0x007fff).ram();
	map(0x008000, 0x03ffff).rom().region("program", 0x8000);
	map(0x040000, 0x04ffff).rom().region("program", 0);
	map(0x050000, 0x050fff).lrw8(
		[this](offs_t offset) { return m_nvram[offset]; }, "nvram_r",
		[this](offs_t offset, u8 data) { m_nvram[offset] = data; }, "nvram_w").umask16(0x00ff);
	map(0x064000, 0x06efff).ram();
	map(0x070002, 0x070005).nopr();
}


static INPUT_PORTS_START(gm1000)
	// Keycode assignments are rather arbitrary and do not at all reflect the panel layout
	PORT_START("P4")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME(u8"Parameter \u2190") // ←
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME(u8"Parameter \u2192") // →
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME(u8"Data \u2190") // ←
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME(u8"Data \u2192") // →

	PORT_START("P6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("All Mute")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("P. Mute")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("P. Monitor")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Display")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME(u8"Part \u2190") // ←
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME(u8"Part \u2192") // →
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME(u8"Instrument \u2190") // ←
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME(u8"Instrument \u2192") // →
INPUT_PORTS_END

void gm1000_state::gm1000(machine_config &config)
{
	M37702S1(config, m_maincpu, 4'000'000); // unknown clock; type guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &gm1000_state::mem_map);
	m_maincpu->p4_in_cb().set_ioport("P4");
	m_maincpu->p6_in_cb().set_ioport("P6");
	m_maincpu->an0_cb().set_constant(0xc0); // battery voltage
	m_maincpu->an2_cb().set_constant(0x80); // ?

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*24, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");
	screen.set_color(rgb_t::green());

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 24);
	lcdc.set_pixel_update_cb(FUNC(gm1000_state::lcd_pixel_update));
	lcdc.set_function_set_at_any_time(true);
}

ROM_START(bh1000)
	ROM_REGION16_LE(0x40000, "program", 0)
	ROM_LOAD16_BYTE("bh1_mgs71a__u6_sysl__v2.1_bf66.u6", 0x00000, 0x20000, CRC(d003880c) SHA1(5cf727c42dd1b903c0048f147b461676e8c35faf)) // MX27C1000-90
	ROM_LOAD16_BYTE("bh1_mgs71a__u7_sysh__v2.1_2fac.u7", 0x00001, 0x20000, CRC(989417a1) SHA1(3de4f10a2e7cde5eb93f04bd75db36e194b1d991)) // MX27C1000-90

	ROM_REGION(0x800000, "waves", 0) // DIP42 mask ROMs "© SUZUKI 1993".
	ROM_LOAD("319-35006_wd06__m531602c-53.u13", 0x000000, 0x200000, CRC(afcea840) SHA1(f003b19b83560191bef03d0d2c1559d77bdaa227))
	ROM_LOAD("319-35007_wd07__m531602c-52.u14", 0x200000, 0x200000, CRC(1f322ddb) SHA1(5f3b1be61782b74e4696d23cd551aa07eb709bb7))
	ROM_LOAD("319-35008_wd08__m531602c-54.u15", 0x200000, 0x200000, CRC(be9c158b) SHA1(878b08ace7b54fa27180c9c45d4b90c04b4bb656))
	ROM_LOAD("319-35009_wd09__m531602c-55.u16", 0x200000, 0x200000, CRC(dee0b84a) SHA1(c528131182d24c42c9d64d3b7f811fd8fe88c3e5))
ROM_END

ROM_START(gm1000)
	ROM_REGION16_LE(0x40000, "program", 0)
	ROM_LOAD16_BYTE("bh1_mgs71a__u6_sysl__v2.1_bf66.u6", 0x00000, 0x20000, CRC(d003880c) SHA1(5cf727c42dd1b903c0048f147b461676e8c35faf)) // MX27C1000-90
	ROM_LOAD16_BYTE("bh1_mgs71a__u7_sysh__v2.1_2fac.u7", 0x00001, 0x20000, CRC(989417a1) SHA1(3de4f10a2e7cde5eb93f04bd75db36e194b1d991)) // MX27C1000-90

	ROM_REGION(0x800000, "waves", 0) // DIP42 mask ROMs "© SUZUKI 1993"
	ROM_LOAD("319-35006_wd06__m531602c-53.u13", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD("319-35007_wd07__m531602c-52.u14", 0x200000, 0x200000, NO_DUMP)
	ROM_LOAD("319-35008_wd08__m531602c-54.u15", 0x200000, 0x200000, NO_DUMP)
	ROM_LOAD("319-35009_wd09__m531602c-55.u16", 0x200000, 0x200000, NO_DUMP)
ROM_END

} // anonymous namespace

SYST(1994, bh1000, 0, 0, gm1000, gm1000, gm1000_state, empty_init, "Nihon Eniac Co., Ltd.", "Sound Saurus BH-1000", MACHINE_IS_SKELETON)
SYST(1994, gm1000, 0, 0, gm1000, gm1000, gm1000_state, empty_init, "Suzuki (Hammond license)", "GM-1000 GM Sound Module", MACHINE_IS_SKELETON)
