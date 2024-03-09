// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Visual Technology Visual 50/55

    VT52 compatible terminal

    Hardware:
    - D780C (Z80)
    - 2764 and 2732 EPROM
    - 2x TMM314APL-1 (1k RAM)
    - X2210D NOVRAM (64x4)
    - Intel P8253-5 PIT
    - D8255AC-5 PPI
    - 2x D8251AC USART
    - SCN2672A
    - 2673
    - TMM2016P-2 (2k VRAM)
    - C68100 IC240-001R00 (character generator?)
    - 17.320 MHz XTAL

    TODO:
    - Character generator ROM is undumped
    - Keyboard (also not dumped)
    - Screen rendering
    - Screen brightness control
    - PPI connections are unknown
    - AUX port

    Notes:
    - PCB marked "PA015 REV B 1183"

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/x2212.h"
#include "video/scn2674.h"

#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class visual50_state : public driver_device
{
public:
	visual50_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_novram(*this, "novram"),
		m_pit(*this, "pit"),
		m_screen(*this, "screen"),
		m_pvtc(*this, "pvtc"),
		m_usart(*this, "usart%u", 0U)
	{ }

	void visual50(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device<x2210_device> m_novram;
	required_device<pit8253_device> m_pit;
	required_device<screen_device> m_screen;
	required_device<scn2672_device> m_pvtc;
	required_device_array<i8251_device, 2> m_usart;

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void char_map(address_map &map);

	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	uint8_t recall_r();
	void recall_w(uint8_t data);
	uint8_t store_r();
	void store_w(uint8_t data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void visual50_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
	map(0x8000, 0x83ff).ram();
	map(0xa000, 0xa03f).rw(m_novram, FUNC(x2210_device::read), FUNC(x2210_device::write));
	map(0xc000, 0xc000).rw(FUNC(visual50_state::recall_r), FUNC(visual50_state::recall_w));
	map(0xe000, 0xe000).rw(FUNC(visual50_state::store_r), FUNC(visual50_state::store_w));
}

void visual50_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x10, 0x11).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x21).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x40, 0x40).r(m_pvtc, FUNC(scn2672_device::buffer_r));
	map(0x50, 0x50).w(m_pvtc, FUNC(scn2672_device::buffer_w));
	map(0x60, 0x63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x70, 0x73).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

void visual50_state::char_map(address_map &map)
{
	map(0x000, 0x7ff).ram();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( visual50 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

SCN2672_DRAW_CHARACTER_MEMBER( visual50_state::draw_character )
{
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t visual50_state::recall_r()
{
	if (!machine().side_effects_disabled())
	{
		m_novram->recall(1);
		m_novram->recall(0);
	}

	return 0xff;
}

void visual50_state::recall_w(uint8_t data)
{
	m_novram->recall(1);
	m_novram->recall(0);
}

uint8_t visual50_state::store_r()
{
	if (!machine().side_effects_disabled())
	{
		m_novram->store(1);
		m_novram->store(0);
	}

	return 0xff;
}

void visual50_state::store_w(uint8_t data)
{
	m_novram->store(1);
	m_novram->store(0);
}

void visual50_state::machine_start()
{
}

void visual50_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void visual50_state::visual50(machine_config &config)
{
	Z80(config, m_maincpu, 17.320_MHz_XTAL / 8); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &visual50_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &visual50_state::io_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	X2210(config, m_novram);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(17.320_MHz_XTAL / 8);
	m_pit->set_clk<1>(17.320_MHz_XTAL / 8);
	m_pit->set_clk<2>(17.320_MHz_XTAL / 8);
	m_pit->out_handler<0>().set(m_usart[0], FUNC(i8251_device::write_rxc)); // or txc?
	m_pit->out_handler<1>().set(m_usart[1], FUNC(i8251_device::write_rxc));
	m_pit->out_handler<1>().append(m_usart[1], FUNC(i8251_device::write_txc));
	m_pit->out_handler<2>().set(m_usart[0], FUNC(i8251_device::write_txc)); // or rxc?

	I8255(config, "ppi");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::amber()); // unknown color
	m_screen->set_raw(17.320_MHz_XTAL, 900, 0, 720, 321, 0, 300);
	m_screen->set_screen_update(m_pvtc, FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 17.320_MHz_XTAL / 9);
	m_pvtc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(9);
	m_pvtc->set_display_callback(FUNC(visual50_state::draw_character));
	m_pvtc->set_addrmap(0, &visual50_state::char_map);

	// mainport
	I8251(config, m_usart[0], 17.320_MHz_XTAL / 8); // divider not verified
	m_usart[0]->rxrdy_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_usart[0]->txd_handler().set("mainport", FUNC(rs232_port_device::write_txd));
	m_usart[0]->rts_handler().set("mainport", FUNC(rs232_port_device::write_rts));

	rs232_port_device &mainport(RS232_PORT(config, "mainport", default_rs232_devices, nullptr));
	mainport.rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	mainport.cts_handler().set(m_usart[0], FUNC(i8251_device::write_cts));

	// keyboard
	I8251(config, m_usart[1], 17.320_MHz_XTAL / 8); // divider not verified
	m_usart[1]->rxrdy_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( visual50 )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("e244-012.bin", 0x0000, 0x2000, CRC(755f0722) SHA1(80b90882c52c9ddcc1cf455c9195d4e1ccda8ce8))
	ROM_LOAD("e262-055.bin", 0x2000, 0x1000, CRC(90a142e8) SHA1(5f4c403b7ab09dcb3cfdc8f57f65e0a52992feed))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY              FULLNAME     FLAGS
COMP( 1983, visual50, 0,      0,      visual50, visual50, visual50_state, empty_init, "Visual Technology", "Visual 50", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
