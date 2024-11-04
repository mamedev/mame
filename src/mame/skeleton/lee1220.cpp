// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Lee Data 1220 terminal (IBM 3278-compatible).

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/am9517a.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
//#include "machine/wd1933.h"
#include "video/mc6845.h"
#include "screen.h"

#include <algorithm>


namespace {

class lee1220_state : public driver_device
{
public:
	lee1220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
	{
	}

	void lee1220(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(update_row);

	u8 sdlc_r(offs_t offset);
	void sdlc_w(offs_t offset, u8 data);
	u8 c0_r();

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	//required_device<wd1933_device> m_sdlc;
	required_device<hd6845s_device> m_crtc;

	u8 m_sdlc_reg[6];
};

void lee1220_state::machine_start()
{
	std::fill_n(&m_sdlc_reg[0], 6, 0);
	save_item(NAME(m_sdlc_reg));
}

MC6845_UPDATE_ROW(lee1220_state::update_row)
{
}


u8 lee1220_state::sdlc_r(offs_t offset)
{
	return m_sdlc_reg[offset]; //m_sdlc->read(~offset & 7);
}

void lee1220_state::sdlc_w(offs_t offset, u8 data)
{
	logerror("%s: Writing %02X to WD1933 register %d\n", machine().describe_context(), data, offset);
	m_sdlc_reg[offset] = data;
	//m_sdlc->write(~offset & 7, ~data);
}

u8 lee1220_state::c0_r()
{
	return 0;
}

void lee1220_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("program", 0);
	map(0x4000, 0x4fff).ram();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xbfff).ram();
}

void lee1220_state::io_map(address_map &map)
{
	map(0x80, 0x8f).rw("dmac", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x90, 0x95).rw(FUNC(lee1220_state::sdlc_r), FUNC(lee1220_state::sdlc_w));
	map(0xa0, 0xa0).w("crtc", FUNC(hd6845s_device::address_w));
	map(0xa1, 0xa1).w("crtc", FUNC(hd6845s_device::register_w));
	map(0xb0, 0xb1).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xc0, 0xc0).r(FUNC(lee1220_state::c0_r));
	map(0xd0, 0xd3).w("pit", FUNC(pit8253_device::write));
}

static INPUT_PORTS_START(lee1220)
INPUT_PORTS_END

void lee1220_state::lee1220(machine_config &config)
{
	I8085A(config, m_maincpu, 10'000'000); // Intel P8085AH-2
	m_maincpu->set_addrmap(AS_PROGRAM, &lee1220_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lee1220_state::io_map);

	AM9517A(config, "dmac", 5'000'000); // Intel P8237A-5

	I8748(config, "kbdmcu", 5.9904_MHz_XTAL);

	//WD1933(config, m_sdlc); // WDC WD1933PL-11

	i8251_device &usart(I8251(config, "usart", 0)); // NEC D8251AC
	usart.rxrdy_handler().set_inputline(m_maincpu, I8085_RST55_LINE);

	PIT8253(config, "pit"); // Intel D8253-5

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(28'944'000, 1152, 0, 960, 420, 0, 400);
	screen.set_screen_update(m_crtc, FUNC(hd6845s_device::screen_update));

	HD6845S(config, m_crtc, 2'412'000); // Hitachi HD46505SP/HD6845SP
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(12);
	m_crtc->set_update_row_callback(FUNC(lee1220_state::update_row));
}

// Main board XTALs: X1 = 28.944 MHz, X2 = 47.46816 MHz
// TTL daughterboard XTAL: X1 = 5.0688 MHz
// Keyboard XTAL: #20 = 5.9904 MHz
ROM_START(lee1220)
	ROM_REGION(0x3000, "program", 0)
	ROM_LOAD("13260303.u21", 0x0000, 0x1000, CRC(28692f78) SHA1(60957e773cb4350d82b02db8f741bd32a43365f1))
	ROM_LOAD("13260403.u22", 0x1000, 0x1000, CRC(3c54a251) SHA1(8d2849a2590be5cf765d26ff80a170b263a4b080))
	ROM_LOAD("13260503.u23", 0x2000, 0x1000, CRC(0872e674) SHA1(24d07a2cfccf03e90334daea810fe0e7c533e5ec))

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("03278_d8748.2", 0x000, 0x400, CRC(a63ce4d8) SHA1(a713f3aae5e9096a627fab13573eee2170b42b1a))
ROM_END

} // anonymous namespace


COMP(1983, lee1220, 0, 0, lee1220, lee1220, lee1220_state, empty_init, "Lee Data", "1220 Display Terminal", MACHINE_IS_SKELETON)
