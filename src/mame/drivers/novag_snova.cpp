// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Novag Super Nova (model 904)

Hardware notes:
- Hitachi HD63A03YP MCU @ 16MHz (4MHz internal)
- 32KB ROM(TC57256AD-12), 8KB RAM(CXK58648P-10L)
- LCD with 4 digits and custom segments, no LCD chip
- buzzer, 16 LEDs, 8*8 chessboard buttons

TODO:
- everything
- if it turns out that the hardware is similar enough to supremo, merge drivers?

******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"

#include "speaker.h"


namespace {

class snova_state : public driver_device
{
public:
	snova_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_dac(*this, "dac")
	{ }

	// machine configs
	void snova(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<dac_bit_interface> m_dac;

	void main_map(address_map &map);
};

void snova_state::machine_start()
{
}



/******************************************************************************
    I/O
******************************************************************************/

// ...



/******************************************************************************
    Address Maps
******************************************************************************/

void snova_state::main_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6303y_cpu_device::hd6301y_io));
	map(0x0040, 0x013f).ram(); // internal
	map(0x4000, 0x5fff).ram();
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( snova )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void snova_state::snova(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &snova_state::main_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( nsnova ) // ID = N1.05
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("n_530.u5", 0x8000, 0x8000, CRC(727a0ada) SHA1(129c1edc5c1d2e12ce97ebef81c6d5555464a11d) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
CONS( 1990, nsnova, 0,      0,      snova,  snova, snova_state, empty_init, "Novag", "Super Nova (Novag)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

