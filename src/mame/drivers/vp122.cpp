// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for ADDS Viewpoint 122 terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "video/scn2674.h"
#include "screen.h"

class vp122_state : public driver_device
{
public:
	vp122_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_avdc(*this, "avdc")
		, m_chargen(*this, "chargen")
	{ }

	void vp122(machine_config &config);

private:
	virtual void machine_start() override;

	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void io_map(address_map &map);
	void mem_map(address_map &map);
	void char_map(address_map &map);
	void attr_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<scn2674_device> m_avdc;
	required_region_ptr<u8> m_chargen;
};


void vp122_state::machine_start()
{
	subdevice<i8251_device>("usart")->write_cts(0);
}

void vp122_state::mem_map(address_map &map)
{
	map(0x0000, 0x9fff).rom().region("maincpu", 0);
	map(0xa000, 0xa7ff).ram().share("nvram");
	map(0xe000, 0xe7ff).noprw();
}

void vp122_state::io_map(address_map &map)
{
	map(0x00, 0x07).rw("avdc", FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x10, 0x1f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x20, 0x20).rw("usart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x21, 0x21).rw("usart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x50, 0x50).rw("avdc", FUNC(scn2674_device::buffer_r), FUNC(scn2674_device::buffer_w));
	map(0x60, 0x60).rw("avdc", FUNC(scn2674_device::attr_buffer_r), FUNC(scn2674_device::attr_buffer_w));
	map(0x70, 0x73).w("pit", FUNC(pit8253_device::write));
}


SCN2674_DRAW_CHARACTER_MEMBER(vp122_state::draw_character)
{
}

void vp122_state::char_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1800, 0x2fff).ram();
}

void vp122_state::attr_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1800, 0x2fff).ram();
}


static INPUT_PORTS_START( vp122 )
INPUT_PORTS_END

void vp122_state::vp122(machine_config &config)
{
	I8085A(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vp122_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vp122_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z02

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.916_MHz_XTAL, 960, 0, 800, 259, 0, 240);
	//m_screen->set_raw(22.096_MHz_XTAL, 1422, 0, 1188, 259, 0, 240);
	m_screen->set_screen_update("avdc", FUNC(scn2674_device::screen_update));

	SCN2674(config, m_avdc, 14.916_MHz_XTAL / 10);
	m_avdc->intr_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	m_avdc->set_character_width(10); // 9 in 132-column modes
	m_avdc->set_display_callback(FUNC(vp122_state::draw_character));
	m_avdc->set_addrmap(0, &vp122_state::char_map);
	m_avdc->set_addrmap(1, &vp122_state::attr_map);
	m_avdc->set_screen("screen");

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set_inputline("maincpu", I8085_RST55_LINE);
	duart.outport_cb().set("usart", FUNC(i8251_device::write_txc)).bit(3);
	duart.outport_cb().append("usart", FUNC(i8251_device::write_rxc)).bit(3);
	// OP7 = 0 for 80-column modes, 1 for 132-column modes

	I8251(config, "usart", 8_MHz_XTAL / 2);

	PIT8253(config, "pit", 0);
	// Input clocks are video-related and should differ for 80-column and 132-column modes
}

/**************************************************************************************************************

ADDS Viewpoint 122 (VPT-122).
Chips: D8085AC-2, SCN2674B, SCB2675T, D8251AFC, SCN2681A, D8253C-2, 5x MB8128-15, MK48Z02B-20
Crystals: 22.096, 14.916, 3.6864, 8.000

***************************************************************************************************************/

ROM_START( vp122 )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "223-48600.uj1", 0x0000, 0x4000, CRC(4d140c69) SHA1(04aa5a4f0c0e0d07b9dc983a6d626ee88ef8b8ba) )
	ROM_LOAD( "223-48500.ug1", 0x4000, 0x4000, CRC(4e98554d) SHA1(0cbb9cb7efd02a3209caed410ccc8495a5ec1772) )
	ROM_LOAD( "223-49400.uj2", 0x8000, 0x4000, CRC(447d90d3) SHA1(f8c0db824198b5a571eef80cc3eaf1e829aa2c2a) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "223-48700.uk4", 0x0000, 0x2000, CRC(4dbab4bd) SHA1(18e9a23ba22e2096fa529541fa329f5a56740e62) )
ROM_END

COMP( 1985, vp122, 0, 0, vp122, vp122, vp122_state, empty_init, "ADDS", "Viewpoint 122", MACHINE_IS_SKELETON )
