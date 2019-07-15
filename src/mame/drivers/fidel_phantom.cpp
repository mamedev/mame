// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Fidelity Phantom (model 6100)

Fidelity licensed the design of the Milton/Phantom motorized chessboard and released
their own version. It has a small LCD panel added, the rest looks nearly the same from
the outside. After Fidelity was taken over by H&G, it was rereleased in 1990 as the
Mephisto Phantom. This is assumed to be identical.

Hardware notes:
- R65C02P4, XTAL marked 4.91?200
- 2*32KB ROM 27C256-15, 8KB RAM MS6264L-10
- LCD driver, display panel for digits
- magnetized x/y motor under chessboard, chesspieces have magnet underneath
- piezo speaker, LEDs, 8*8 chessboard buttons
- PCB label 510.1128A01

TODO:
- everything, this is a skeleton driver

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/r65c02.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
//#include "fidel_phantom.lh" // clickable


namespace {

class phantom_state : public driver_device
{
public:
	phantom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_dac(*this, "dac")
	{ }

	void fphantom(machine_config &config);
	void init_fphantom();

protected:
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<dac_bit_interface> m_dac;

	void main_map(address_map &map);
};

void phantom_state::machine_reset()
{
	m_rombank->set_entry(0);
}

void phantom_state::init_fphantom()
{
	m_rombank->configure_entries(0, 2, memregion("rombank")->base(), 0x4000);
}



/******************************************************************************
    I/O
******************************************************************************/

//..



/******************************************************************************
    Address Maps
******************************************************************************/

void phantom_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( fphantom )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void phantom_state::fphantom(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 4.9152_MHz_XTAL); // R65C02P4
	m_maincpu->set_periodic_int(FUNC(phantom_state::irq0_line_hold), attotime::from_hz(600)); // guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &phantom_state::main_map);

	//config.set_default_layout(layout_fidel_phantom);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fphantom ) // model 6100, PCB label 510.1128A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u_3c_yellow.u3", 0x8000, 0x8000, CRC(fb7c38ae) SHA1(a1aa7637705052cb4eec92644dc79aee7ba4d77c) ) // 27C256

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("u_4_white.u4",  0x0000, 0x8000, CRC(e4181ba2) SHA1(1f77d1867c6f566be98645fc252a01108f412c96) ) // 27C256
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     STATE          INIT           COMPANY, FULLNAME, FLAGS
CONS( 1988, fphantom, 0,      0, fphantom, fphantom, phantom_state, init_fphantom, "Fidelity Electronics", "Phantom Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
