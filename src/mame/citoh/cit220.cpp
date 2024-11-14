// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for VT220-compatible terminals by C. Itoh/CIE Terminals and similar terminals by ADDS.

The CIT-220+ Video Terminal was introduced as a direct competitor to DEC's VT220. It copied the design of the VT220 closely
enough to provoke a lawsuit, which led to its eventual withdrawal in favor of its successor, the CIT224.

"COPYRIGHT MICROWEST 1984" can be found in the program ROMs of both the CIT-220+ and Viewpoint 122. The code is clearly similar,
and the SCN2674 video timing parameters appear to be identical.

************************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "cit220_kbd.h"
//#include "machine/eeprompar.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class cit220_state : public driver_device
{
public:
	cit220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_avdc(*this, "avdc")
		, m_chargen(*this, "chargen")
	{ }

	void cit220p(machine_config &config);
	void vp122(machine_config &config);
	void tabe22(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	void sod_w(int state);
	void cols_w(int state);
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void cit220p_mem_map(address_map &map) ATTR_COLD;
	void cit220p_io_map(address_map &map) ATTR_COLD;
	void vp122_mem_map(address_map &map) ATTR_COLD;
	void vp122_io_map(address_map &map) ATTR_COLD;

	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<scn2674_device> m_avdc;
	required_region_ptr<u8> m_chargen;

	bool m_132_cols = false;
};


void cit220_state::machine_start()
{
	subdevice<i8251_device>("usart")->write_cts(0);

	m_132_cols = false;
	save_item(NAME(m_132_cols));
}


void cit220_state::sod_w(int state)
{
	// probably asserts PBREQ on SCN2674 to access memory at Exxx
}

void cit220_state::cols_w(int state)
{
	if (state == m_132_cols)
	{
		m_132_cols = !state;
		m_avdc->set_character_width(m_132_cols ? 9 : 10);
		m_avdc->set_unscaled_clock(m_132_cols ? 22'096'000 / 9 : 14'916'000 / 10);
	}
}

void cit220_state::cit220p_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa1ff).rom().region("eeprom", 0x800);
	map(0xe000, 0xe7ff).ram(); // ???
}

void cit220_state::cit220p_io_map(address_map &map)
{
	map(0x00, 0x0f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x10, 0x11).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x27).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0xa0, 0xa0).rw(m_avdc, FUNC(scn2674_device::attr_buffer_r), FUNC(scn2674_device::attr_buffer_w));
	map(0xc0, 0xc0).rw(m_avdc, FUNC(scn2674_device::buffer_r), FUNC(scn2674_device::buffer_w));
}

void cit220_state::vp122_mem_map(address_map &map)
{
	map(0x0000, 0x9fff).rom().region("maincpu", 0);
	map(0xa000, 0xa7ff).ram().share("nvram");
	map(0xe000, 0xe7ff).noprw();
}

void cit220_state::vp122_io_map(address_map &map)
{
	map(0x00, 0x07).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x10, 0x1f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x20, 0x21).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x50, 0x50).rw(m_avdc, FUNC(scn2674_device::buffer_r), FUNC(scn2674_device::buffer_w));
	map(0x60, 0x60).rw(m_avdc, FUNC(scn2674_device::attr_buffer_r), FUNC(scn2674_device::attr_buffer_w));
	map(0x70, 0x73).w("pit", FUNC(pit8253_device::write));
}


SCN2674_DRAW_CHARACTER_MEMBER(cit220_state::draw_character)
{
	u16 dots = m_chargen[charcode << 4 | linecount] << 2;
	const int width = m_132_cols ? 9 : 10;

	if (BIT(dots, 2))
		dots |= 3;
	if (BIT(attrcode, 2))
		dots = ~dots;
	if (cursor)
		dots = ~dots;

	for (int i = 0; i < width; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 9) ? rgb_t::white() : rgb_t::black();
		dots <<= 1;
	}
}

void cit220_state::char_map(address_map &map)
{
	map(0x0000, 0x2fff).ram();
}

void cit220_state::attr_map(address_map &map)
{
	map(0x0000, 0x2fff).ram();
}


static INPUT_PORTS_START( cit220p )
INPUT_PORTS_END


void cit220_state::cit220p(machine_config &config)
{
	I8085A(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cit220_state::cit220p_mem_map);
	m_maincpu->set_addrmap(AS_IO, &cit220_state::cit220p_io_map);
	m_maincpu->out_sod_func().set(FUNC(cit220_state::sod_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	//m_screen->set_raw(14'916'000, 960, 0, 800, 259, 0, 240);
	m_screen->set_raw(22'096'000, 1422, 0, 1188, 259, 0, 240);
	m_screen->set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	SCN2674(config, m_avdc, 22'096'000 / 9);
	m_avdc->intr_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	m_avdc->set_character_width(9); // 10 in 80-column modes
	m_avdc->set_display_callback(FUNC(cit220_state::draw_character));
	m_avdc->set_addrmap(0, &cit220_state::char_map);
	m_avdc->set_addrmap(1, &cit220_state::attr_map);
	m_avdc->set_screen(m_screen);

	scn2681_device &duart(SCN2681(config, "duart", 3'686'400));
	duart.irq_cb().set_inputline(m_maincpu, I8085_RST55_LINE);
	duart.outport_cb().set("usart", FUNC(i8251_device::write_txc)).bit(3);
	duart.outport_cb().append("usart", FUNC(i8251_device::write_rxc)).bit(3);
	duart.outport_cb().append(FUNC(cit220_state::cols_w)).bit(7);

	i8251_device &usart(I8251(config, "usart", 4'000'000));
	usart.txd_handler().set("keyboard", FUNC(cit220p_keyboard_device::write_rxd));

	cit220p_keyboard_device &keyboard(CIT220P_KEYBOARD(config, "keyboard"));
	keyboard.txd_callback().set("usart", FUNC(i8251_device::write_rxd));
}

void cit220_state::vp122(machine_config &config)
{
	I8085A(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cit220_state::vp122_mem_map);
	m_maincpu->set_addrmap(AS_IO, &cit220_state::vp122_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z02

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.916_MHz_XTAL, 960, 0, 800, 259, 0, 240);
	//m_screen->set_raw(22.096_MHz_XTAL, 1422, 0, 1188, 259, 0, 240);
	m_screen->set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	SCN2674(config, m_avdc, 14.916_MHz_XTAL / 10);
	m_avdc->intr_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	m_avdc->set_character_width(10); // 9 in 132-column modes
	m_avdc->set_display_callback(FUNC(cit220_state::draw_character));
	m_avdc->set_addrmap(0, &cit220_state::char_map);
	m_avdc->set_addrmap(1, &cit220_state::attr_map);
	m_avdc->set_screen("screen");

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set_inputline(m_maincpu, I8085_RST55_LINE);
	duart.outport_cb().set("usart", FUNC(i8251_device::write_txc)).bit(3);
	duart.outport_cb().append("usart", FUNC(i8251_device::write_rxc)).bit(3);
	duart.outport_cb().append(FUNC(cit220_state::cols_w)).bit(7);

	I8251(config, "usart", 8_MHz_XTAL / 2);

	PIT8253(config, "pit");
	// Input clocks are video-related and should differ for 80-column and 132-column modes
}


ROM_START(cit220p)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("v17_001a.ic23", 0x0000, 0x4000, CRC(2cc43026) SHA1(366f57292c6e44571368c29e3258203779847356))
	ROM_LOAD("v17_001b.ic24", 0x4000, 0x4000, CRC(a56b16f1) SHA1(c68f26b35453153f7defcf1cf2b7ad7fe36cc9e7))

	ROM_REGION(0x1000, "eeprom", 0)
	ROM_LOAD("eeprom.bin",    0x0000, 0x1000, CRC(7b24878a) SHA1(20086fb792a24339b65abe627aefbcf48e2abcf4))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("v20_cg.ic17",   0x0000, 0x1000, CRC(76ef7ca9) SHA1(6e7799ca0a41350fbc369bbbd4ab581150f37b10))
ROM_END

/**************************************************************************************************************

ADDS Viewpoint 122 (VPT-122).
Chips: D8085AC-2, SCN2674B, SCB2675T, D8251AFC, SCN2681A, D8253C-2, 5x MB8128-15, MK48Z02B-20
Crystals: 22.096, 14.916, 3.6864, 8.000

***************************************************************************************************************/

ROM_START(vp122)
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD("223-48600.uj1", 0x0000, 0x4000, CRC(4d140c69) SHA1(04aa5a4f0c0e0d07b9dc983a6d626ee88ef8b8ba))
	ROM_LOAD("223-48500.ug1", 0x4000, 0x4000, CRC(4e98554d) SHA1(0cbb9cb7efd02a3209caed410ccc8495a5ec1772))
	ROM_LOAD("223-49400.uj2", 0x8000, 0x4000, CRC(447d90d3) SHA1(f8c0db824198b5a571eef80cc3eaf1e829aa2c2a))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("223-48700.uk4", 0x0000, 0x2000, CRC(4dbab4bd) SHA1(18e9a23ba22e2096fa529541fa329f5a56740e62))
ROM_END

} // anonymous namespace


COMP(1984, cit220p, 0, 0, cit220p, cit220p, cit220_state, empty_init, "C. Itoh Electronics", "CIT-220+ Video Terminal", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
COMP(1985, vp122, 0, 0, vp122, cit220p, cit220_state, empty_init, "ADDS", "Viewpoint 122", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
