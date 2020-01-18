// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Novag Supremo

Hardware notes:
- Hitachi HD63A03YP MCU @ 8MHz (2MHz internal)
- 32KB ROM(TC57256AD-12), 2KB RAM(TC5516APL)
- LCD with 4 digits and custom segments, no LCD chip
- buzzer, 16 LEDs, 8*8 chessboard buttons

Novag Primo is assumed to be on similar hardware
Supremo also had a "limited edition" rerelease in 1990, plastic is fake-wood
instead of black, otherwise it's the same game.

TODO:
- does not work, most likely due to incomplete cpu emulation
  (extra I/O ports, unemulated timer registers)

******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "speaker.h"


namespace {

class supremo_state : public driver_device
{
public:
	supremo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_dac(*this, "dac")
	{ }

	// machine configs
	void supremo(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	optional_device<dac_bit_interface> m_dac;

	void main_map(address_map &map);
};

void supremo_state::machine_start()
{
}



/******************************************************************************
    I/O
******************************************************************************/

// ...



/******************************************************************************
    Address Maps
******************************************************************************/

void supremo_state::main_map(address_map &map)
{
	map(0x0000, 0x000e).rw(m_maincpu, FUNC(hd6303y_cpu_device::m6801_io_r), FUNC(hd6303y_cpu_device::m6801_io_w));
	map(0x0040, 0x013f).ram(); // internal
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( supremo )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void supremo_state::supremo(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &supremo_state::main_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
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

ROM_START( supremo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sp_a10.u5", 0x8000, 0x8000, CRC(745d010f) SHA1(365a8e2afcf63678ba0161b9082f6439a9d78c9f) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT     CLASS          INIT        COMPANY, FULLNAME, FLAGS
CONS( 1988, supremo, 0,      0,      supremo,  supremo, supremo_state, empty_init, "Novag", "Supremo", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

