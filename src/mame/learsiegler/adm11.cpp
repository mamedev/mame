// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for ADM 11 and related terminals by Lear Siegler, Inc.

    * ADM 11 (June 1983): LSI's first terminal to use the "High Touch"
      enclosure shared by the rest of its family and other LSI terminals.
      Its detachable typewriter-style keyboard has only 4 function keys
      (8 with shifting). Display is one 24x80 page plus a status line.
      Font supports "business graphics" as well as ASCII characters.
    * ADM 12 (March 1984): Expanded version of ADM 11 implementing block
      mode and many more editing functions. 2 pages of text are available
      in 80x24 mode, and there are also 80x48 and 158x24 modes.
    * ADM 11G & ADM 12G (March 1984): ADM 11 and ADM 12 enhanced with
      Tektronix Plot 10-like graphics.
    * ADM 1178 (March 1984): Emulates the keyboard and communications of the
      IBM 3278, though an external protocol converter is also required for
      network compatibility.
    * ADM 1278 (March 1984): Block mode version of the ADM 1178.
    * ADM 12plus (February 1985): Later version of the ADM 12, with 132-
      column and horizontal split display options. Up to 4 pages of display
      memory are supported.
    * ADM 11plus (June 1985): Later version of ADM 11 with more function
      keys and character and line insertion/deletion.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/eeprompar.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class adm11_state : public driver_device
{
public:
	adm11_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_avdc(*this, "avdc")
		, m_charram(*this, "charram")
		, m_attrram(*this, "attrram")
		, m_chargen(*this, "chargen")
		, m_mbc(false)
		, m_latched_attr(0)
	{
	}

	void adm12(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);
	void mbc_w(int state);

	u8 p1_r();
	void p1_w(u8 data);
	void char_latched_attr_w(offs_t offset, u8 data);
	void attr_latch_w(u8 data);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<scn2674_device> m_avdc;
	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_attrram;
	required_region_ptr<u8> m_chargen;

	bool m_mbc;
	u8 m_latched_attr;
};


void adm11_state::machine_start()
{
	save_item(NAME(m_mbc));
}

SCN2674_DRAW_CHARACTER_MEMBER(adm11_state::draw_character)
{
	u16 dots = m_chargen[charcode << 4 | linecount];
	dots |= (dots & 0x80) << 1;

	if (BIT(attrcode, 2))
		dots ^= 0x1ff;

	for (int i = 0; i < 9; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 8) ? rgb_t::white() : rgb_t::black();
		dots <<= 1;
	}
}

void adm11_state::mbc_w(int state)
{
	m_mbc = state;
}

u8 adm11_state::p1_r()
{
	// Bit 0 low = display busy?
	return 0xfe | (m_mbc ? 0 : 1);
}

void adm11_state::p1_w(u8 data)
{
}

void adm11_state::char_latched_attr_w(offs_t offset, u8 data)
{
	m_charram[offset] = data;
	m_attrram[offset] = m_latched_attr;
}

void adm11_state::attr_latch_w(u8 data)
{
	m_latched_attr = data;
}

void adm11_state::prog_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("program", 0);
}

void adm11_state::ext_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x21ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x4000, 0x4fff).mirror(0x1000).ram().share("charram");
	map(0x6000, 0x6fff).mirror(0x1000).w(FUNC(adm11_state::char_latched_attr_w));
	map(0x8000, 0x8fff).mirror(0x1000).ram().share("attrram");
	map(0xa000, 0xa000).mirror(0x1fff).w(FUNC(adm11_state::attr_latch_w));
	map(0xe000, 0xe007).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
}

void adm11_state::char_map(address_map &map)
{
	map.global_mask(0x0fff);
	map(0x0000, 0x0fff).ram().share("charram");
}

void adm11_state::attr_map(address_map &map)
{
	map.global_mask(0x0fff);
	map(0x0000, 0x0fff).ram().share("attrram");
}


static INPUT_PORTS_START(adm12)
INPUT_PORTS_END

void adm11_state::adm12(machine_config &config)
{
	I8031(config, m_maincpu, 11.0592_MHz_XTAL); // P8031AH
	m_maincpu->set_addrmap(AS_PROGRAM, &adm11_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &adm11_state::ext_map);
	m_maincpu->port_in_cb<1>().set(FUNC(adm11_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(adm11_state::p1_w));
	// TODO: RXD/TXD are serial communications; INT0 is serial keyboard data

	EEPROM_2804(config, "eeprom"); // X2804AP

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(15.93_MHz_XTAL, 900, 0, 720, 295, 0, 275); // 17.7 kHz horizontal
	screen.set_screen_update("avdc", FUNC(scn2674_device::screen_update));

	SCN2674(config, m_avdc, 15.93_MHz_XTAL / 9); // SCN2674B
	m_avdc->set_screen("screen");
	m_avdc->set_character_width(9); // 6 in 158-column mode?
	m_avdc->set_addrmap(0, &adm11_state::char_map);
	m_avdc->set_addrmap(1, &adm11_state::attr_map);
	m_avdc->set_display_callback(FUNC(adm11_state::draw_character));
	m_avdc->intr_callback().set_inputline(m_maincpu, MCS51_INT1_LINE);
	m_avdc->mbc_callback().set(FUNC(adm11_state::mbc_w));
}

// "MEM2764" silkscreened next to all memories, only one of which is an actual 2764 (most are SY2128-4 SRAMs)
ROM_START(adm12)
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("u13.bin", 0x0000, 0x2000, CRC(3c928176) SHA1(dd741c620da2ced9979456296c2af0387461cdf1)) // MBM2764-30

	// Keyboard MCU might be a COP420, as with the ADM 11

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("u35.bin", 0x0000, 0x1000, CRC(66d7bc44) SHA1(cd839839f29657207098d85900cb570285be91a6)) // HN462732-P
ROM_END

} // anonymous namespace


COMP(1984, adm12, 0, 0, adm12, adm12, adm11_state, empty_init, "Lear Siegler", "ADM 12 Video Display Terminal", MACHINE_IS_SKELETON)
