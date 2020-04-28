// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

SciSys Kasparov Leonardo, Saitek Kasparov Galileo.

This is SciSys's answer to H+G Mephisto modular chesscomputers, but unlike the
Mephistos, these boards are actual chesscomputers and not an accessory.

They called the expansion capability "OSA", for "Open Systems Architecture".
One for a link port to a PC, and one for a module slot. The expansion modules
are basically entire chesscomputers, making the whole thing combined a
'dual brain' chesscomputer.

Hardware notes:

Leonardo (1986):
- 6301Y0 MCU @ 12MHz
- 32KB ROM(27C256)
- 8KB RAM(M5M5165P-15 or compatible)
- magnet sensors chessboard with 16 leds

The 6301Y0 was seen with internal maskrom serial A96 and B40. It appears to be
running in mode 1 (expanded mode): the internal ROM is disabled and the MCU can
be emulated as if it's a HD6303Y. It's not known what's on the internal ROM,
it could even be from another SciSys chesscomputer.

Galileo (1988):
- HD6303YP MCU @ 12MHz
- almost same as Leonardo

Galileo PCB is different, but essentially it's the same hardware as Leonardo.
The 1.4 ROM is identical to it too, even though it's a different MCU type.
And on the outside, the button panel was redesigned a bit.

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

class leo_state : public driver_device
{
public:
	leo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_dac(*this, "dac")
	{ }

	// machine configs
	void leo(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	optional_device<dac_bit_interface> m_dac;

	void main_map(address_map &map);
};

void leo_state::machine_start()
{
}



/******************************************************************************
    I/O
******************************************************************************/

// ...



/******************************************************************************
    Address Maps
******************************************************************************/

void leo_state::main_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6303y_cpu_device::hd6301y_io));
	map(0x0040, 0x013f).ram(); // internal
	map(0x4000, 0x5fff).ram();
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( leo )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void leo_state::leo(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &leo_state::main_map);

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

ROM_START( leonardo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw6.1_osa1.4_510.u9", 0x8000, 0x8000, CRC(e39676b2) SHA1(288c5f2608277cb4c3ca71cb2e642a6a62c01dca) ) // D27C256AD-12
ROM_END

ROM_START( leonardoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sx6_617l_osa1.2.u9", 0x8000, 0x8000, CRC(4620f827) SHA1(4ae566646d032dd5bcca48316dd90a11e06772f1) ) // D27C256AD-12
ROM_END

ROM_START( galileo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw6.1_osa1.4_510.u9", 0x8000, 0x8000, CRC(e39676b2) SHA1(288c5f2608277cb4c3ca71cb2e642a6a62c01dca) ) // MBM27C256H-10
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP MACHINE INPUT CLASS      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, leonardo,  0,        0,  leo,    leo,  leo_state, empty_init, "SciSys", "Kasparov Leonardo (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1986, leonardoa, leonardo, 0,  leo,    leo,  leo_state, empty_init, "SciSys", "Kasparov Leonardo (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

CONS( 1988, galileo,   0,        0,  leo,    leo,  leo_state, empty_init, "Saitek", "Kasparov Galileo", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
