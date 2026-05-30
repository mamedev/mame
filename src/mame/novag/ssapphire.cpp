// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag (Perfect Technology) Star Sapphire (model 1003)

The chess engine is the same as the one in Star Diamond.

Hardware notes:
- PCB label: 800T015R0-FR4-2 2003.8.13
- Hitachi H8S/2312 12312VTE25, 25MHz XTAL
- unknown MCU behind epoxy blob, 4.194304MHz XTAL
- 512KB Flash ROM (SST 39VF400A), only 192KB used
- 256KB RAM (2*Hyundai HY62V8100B)
- custom LCD with embedded 8*9 touchscreen, piezo
- RJ-12 port for Novag Super System (always 57600 baud)

TODO:
- emulate MCU, H8 communicates with it via SCI0

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/h8/h8s2319.h"
#include "machine/nvram.h"
#include "sound/dac.h"

#include "screen.h"
#include "speaker.h"


namespace {

class ssapphire_state : public driver_device
{
public:
	ssapphire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232")
	{ }

	void ssapphire(machine_config &config);

private:
	// devices/pointers
	required_device<h8s2312_device> m_maincpu;
	required_device<dac_1bit_device> m_dac;
	required_device<rs232_port_device> m_rs232;

	void main_map(address_map &map) ATTR_COLD;
};



/*******************************************************************************
    Address Maps
*******************************************************************************/

void ssapphire_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x43ffff).ram().share("nvram");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ssapphire )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ssapphire_state::ssapphire(machine_config &config)
{
	// basic machine hardware
	H8S2312(config, m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssapphire_state::main_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8s2312_device::nvram_set_battery));
	m_maincpu->write_sci_tx<1>().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_maincpu->write_port1().set(m_dac, FUNC(dac_1bit_device::write)).bit(4);
	m_maincpu->read_port3().set_constant(0xff);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// rs232
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_maincpu, FUNC(h8s2312_device::sci_rx_w<1>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ssapphire ) // ID = H8S/SS V1.01
	ROM_REGION16_BE( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP("39vf400a.ic6", 0x00000, 0x80000, CRC(3854c8cb) SHA1(7cbf0d186dda6d1ea408423b8f185136cca0b291) )

	ROM_REGION16_BE( 0x2000, "mcu", 0 )
	ROM_LOAD16_WORD_SWAP("mcu.u2", 0x0000, 0x2000, NO_DUMP )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 2003, ssapphire, 0,      0,      ssapphire, ssapphire, ssapphire_state, empty_init, "Perfect Technology / Intelligent Heuristic Programming", "Star Sapphire", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
