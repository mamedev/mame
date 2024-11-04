// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Ampex 210(+) and 230(+) video display terminals.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "ampex210_kbd.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class ampex210_state : public driver_device
{
public:
	ampex210_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
	{ }

	void ampex210p(machine_config &config);
	void ampex230p(machine_config &config);

private:
	void common_video(machine_config &config);

	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	u8 keyboard_r();
	u8 keyboard_reset_r();
	u8 keyboard_extra_r();
	void flags1_w(u8 data);
	void flags2_w(u8 data);
	void flags3_w(u8 data);
	u8 modem_r();
	void modem_w(u8 data);

	void ampex210_mem(address_map &map) ATTR_COLD;
	void ampex230_mem(address_map &map) ATTR_COLD;
	void ampex210_io(address_map &map) ATTR_COLD;
	void ampex230_io(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;
	void vram2_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};


SCN2672_DRAW_CHARACTER_MEMBER(ampex210_state::draw_character)
{
}

u8 ampex210_state::keyboard_r()
{
	return 0xff;
}

u8 ampex210_state::keyboard_reset_r()
{
	// data ignored
	return 0xff;
}

u8 ampex210_state::keyboard_extra_r()
{
	return 0;
}

void ampex210_state::flags1_w(u8 data)
{
}

void ampex210_state::flags2_w(u8 data)
{
}

void ampex210_state::flags3_w(u8 data)
{
}

u8 ampex210_state::modem_r()
{
	return 0;
}

void ampex210_state::modem_w(u8 data)
{
}

void ampex210_state::ampex210_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x9fff).ram();
	map(0xc000, 0xc000).r(FUNC(ampex210_state::modem_r));
}

void ampex210_state::ampex230_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x8007).rw("pvtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram().share("nvram");
}

void ampex210_state::ampex210_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw("pvtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x44, 0x47).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x80, 0x80).w(FUNC(ampex210_state::modem_w));
	map(0xc0, 0xc0).w("pvtc", FUNC(scn2672_device::buffer_w));
	map(0xc1, 0xc1).r("pvtc", FUNC(scn2672_device::buffer_r));
	map(0xc2, 0xc2).r(FUNC(ampex210_state::keyboard_r));
	map(0xc3, 0xc3).r(FUNC(ampex210_state::keyboard_reset_r));
	map(0xc4, 0xc4).w(FUNC(ampex210_state::flags1_w));
	map(0xc5, 0xc5).w(FUNC(ampex210_state::flags2_w));
	map(0xc6, 0xc6).r(FUNC(ampex210_state::keyboard_extra_r));
	map(0xc7, 0xc7).w(FUNC(ampex210_state::flags3_w));
}

void ampex210_state::ampex230_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w("pvtc", FUNC(scn2672_device::buffer_w));
	map(0x11, 0x11).r("pvtc", FUNC(scn2672_device::buffer_r));
	map(0x12, 0x12).w("pvtc", FUNC(scn2672_device::attr_buffer_w));
	map(0x13, 0x13).r("pvtc", FUNC(scn2672_device::attr_buffer_r));
	map(0x15, 0x15).r(FUNC(ampex210_state::keyboard_extra_r));
	map(0x16, 0x16).r(FUNC(ampex210_state::keyboard_r));
	map(0x17, 0x17).r(FUNC(ampex210_state::keyboard_reset_r));
	map(0x20, 0x20).w(FUNC(ampex210_state::flags1_w));
	map(0x21, 0x21).w(FUNC(ampex210_state::flags3_w));
	map(0x22, 0x22).w(FUNC(ampex210_state::flags2_w));
	map(0x30, 0x33).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x40, 0x43).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
}

void ampex210_state::vram_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // MB8464A-10L (second half unused?)
}

void ampex210_state::vram2_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
}


static INPUT_PORTS_START(ampex210p)
INPUT_PORTS_END


void ampex210_state::ampex210p(machine_config &config)
{
	Z80(config, m_maincpu, 3.6864_MHz_XTAL); // Z80ACPU; clock uncertain
	m_maincpu->set_addrmap(AS_PROGRAM, &ampex210_state::ampex210_mem);
	m_maincpu->set_addrmap(AS_IO, &ampex210_state::ampex210_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // UM6116-2

	mos6551_device &acia(MOS6551(config, "acia", 3.6864_MHz_XTAL / 2)); // AMI S6551AP
	acia.irq_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	scn2672_device &pvtc(SCN2672(config, "pvtc", 19.602_MHz_XTAL / 9)); // MC2672B4P
	pvtc.intr_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	pvtc.set_character_width(9);
	pvtc.set_display_callback(FUNC(ampex210_state::draw_character));
	pvtc.set_addrmap(0, &ampex210_state::vram_map);
	pvtc.set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_raw(19.602_MHz_XTAL, 900, 0, 720, 363, 0, 300);
	//screen.set_raw(32.147_MHz_XTAL, 1476, 0, 1188, 363, 0, 300);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));
}

void ampex210_state::ampex230p(machine_config &config)
{
	Z80(config, m_maincpu, 3.6864_MHz_XTAL); // Z80ACPU; clock uncertain
	m_maincpu->set_addrmap(AS_PROGRAM, &ampex210_state::ampex230_mem);
	m_maincpu->set_addrmap(AS_IO, &ampex210_state::ampex230_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 3.6864_MHz_XTAL)); // Z8430AB1
	ctc.set_clk<0>(3.6864_MHz_XTAL / 2);
	ctc.set_clk<1>(3.6864_MHz_XTAL / 2);
	ctc.set_clk<2>(3.6864_MHz_XTAL / 2);
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<2>().set("dart", FUNC(z80dart_device::rxtxcb_w));

	z80dart_device &dart(Z80DART(config, "dart", 3.6864_MHz_XTAL)); // Z80847004PSC
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0); // IM 1 autovectored

	AMPEX230_KEYBOARD(config, "keyboard");

	scn2672_device &pvtc(SCN2672(config, "pvtc", 19.602_MHz_XTAL / 9)); // MC2672B4P
	pvtc.intr_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	pvtc.set_character_width(9);
	pvtc.set_display_callback(FUNC(ampex210_state::draw_character));
	pvtc.set_addrmap(0, &ampex210_state::vram_map);
	pvtc.set_addrmap(1, &ampex210_state::vram2_map);
	pvtc.set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_raw(19.602_MHz_XTAL, 900, 0, 720, 363, 0, 312);
	//screen.set_raw(32.147_MHz_XTAL, 1476, 0, 1188, 363, 0, 312);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));
}


ROM_START(ampex210p) // Z80 (+6551,MC2672,3515260-01, 3 xtals, speaker) // 8k ram // amber
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("35-5960-03.u30", 0x0000, 0x8000, CRC(d3f86117) SHA1(f8a9b66899117b362b195bfc94c75bc902fb1376))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("35-526-01.u3", 0x0000, 0x1000, CRC(4659bcd2) SHA1(554574f55ed875baba0a6133648c44df763cc5c4))
ROM_END

ROM_START(ampex230p) // EPROMs stickered "© 1989 AMPEX CORP."
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("230_u11.bin", 0x0000, 0x8000, CRC(c8f93719) SHA1(81019b42245ca60c7de3ee5d3194c4d22fd38a8d))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("230_u2.bin", 0x0000, 0x2000, CRC(7143b773) SHA1(616d3c0c1a1f7a00bf16857324043955ab842994))
ROM_END

} // anonymous namespace


COMP(1988, ampex210p, 0, 0, ampex210p, ampex210p, ampex210_state, empty_init, "Ampex", "Ampex 210 plus Terminal (v3.0)", MACHINE_IS_SKELETON)
COMP(1988, ampex230p, 0, 0, ampex230p, ampex210p, ampex210_state, empty_init, "Ampex", "Ampex 230 plus Terminal (v4.0)", MACHINE_IS_SKELETON)
