// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Saitek Kasparov Renaissance

Saitek's 2nd version modular chesscomputer. It accepts the same modules as
Leonardo/Galileo. "OSA" version for Renaissance is 1.5.

Hardware notes:
- 6301Y0(mode 1) or HD6303YP MCU @ 10MHz
- 8KB RAM, 32KB ROM
- "HELIOS" I/O (NEC gate array)
- Seiko Epson SED1502F, LCD screen
- magnet sensors chessboard with 81 leds

The 6301Y0 seen on one of them, was a SX8A 6301Y0G84P, this is in fact the
MCU(+internal maskrom, disabled here) used in Saitek Conquistador.

The LCD screen is fairly large, it's the same one as in Saitek Simultano,
so a chessboard display + 7seg info.

TODO:
- WIP

******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "speaker.h"


namespace {

class ren_state : public driver_device
{
public:
	ren_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_dac(*this, "dac")
	{ }

	// machine configs
	void ren(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	optional_device<dac_bit_interface> m_dac;

	void main_map(address_map &map);
};

void ren_state::machine_start()
{
}



/******************************************************************************
    I/O
******************************************************************************/

// ...



/******************************************************************************
    Address Maps
******************************************************************************/

void ren_state::main_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6303y_cpu_device::hd6301y_io));
	map(0x0040, 0x013f).ram(); // internal
	map(0x4000, 0x5fff).ram();
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( ren )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void ren_state::ren(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ren_state::main_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( renaissa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw7_518d_u3.u3", 0x8000, 0x8000, CRC(21d2405f) SHA1(6ddcf9bdd30aa446fcaeab919a8f950dc3428365) ) // HN27C256AG-10
ROM_END

ROM_START( renaissaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sx7_518b.u3", 0x8000, 0x8000, CRC(a0c3ffe8) SHA1(fa170a6d4d54d41de77e0bb72f969219e6f376af) ) // MBM27C256H-10
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP  MACHINE INPUT CLASS      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1989, renaissa,  0,        0,   ren,    ren,  ren_state, empty_init, "Saitek", "Kasparov Renaissance (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1989, renaissaa, renaissa, 0,   ren,    ren,  ren_state, empty_init, "Saitek", "Kasparov Renaissance (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
