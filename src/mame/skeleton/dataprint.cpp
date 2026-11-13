// license:BSD-3-Clause
// copyright-holders: stonedDiscord
/***********************************************************************************************************************************

NSM Dataprint 3000

A portable dot-matrix printer used to print out billing information
from electronic game machines for bookkeeping and tax purposes.
Dates seen 1996-1998

________________PWR__
D                   |
B                   |
9                   |
|      74HCT251     |
| E                 |
| P                 |
| R                 |
| O   Connector     BTN
| M                 |
|     M48T18        |
|    TMS70C02NL  Q  |
|___________________|

Q: Crystal

TODO:
	- PCMCIA SRAM
	- Verify crystal frequency
	- Verify LED order
	- Hook up buzzer and sensors
	- Printer emulation
	- Serial port once TMS7000 has it implemented

************************************************************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/tms7000/tms7000.h"
#include "imagedev/printer.h"
#include "machine/timekpr.h"
#include "sound/beep.h"

#include "speaker.h"

#include "dataprint.lh"


namespace {

class dataprint_state : public driver_device
{
public:
	dataprint_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_printer(*this, "printer")
		, m_rs232(*this, "rs232")
		, m_leds(*this, "led%u", 0U)
		, m_buzzer(*this, "buzzer")
	{ }

	void dp3000(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void leds_w(u8 data);

	required_device<tms7000_device> m_maincpu;
	required_device<timekeeper_device> m_nvram;
	required_device<printer_image_device> m_printer;
	required_device<rs232_port_device> m_rs232;
	output_finder<8> m_leds;
	required_device<beep_device> m_buzzer;
};

void dataprint_state::leds_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

void dataprint_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(m_nvram, FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0x2000, 0xffff).rom().nopw();
}

static INPUT_PORTS_START( dp3000 )
	PORT_START("PORTA")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void dataprint_state::dp3000(machine_config &config)
{
	TMS70C02(config, m_maincpu, 4'194'304); // unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &dataprint_state::mem_map);
	m_maincpu->in_porta().set_ioport("PORTA");
	m_maincpu->out_portb().set(FUNC(dataprint_state::leds_w));

	M48T58(config, "nvram"); // ST M48T18

	PRINTER(config, m_printer);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_buzzer, 0).add_route(ALL_OUTPUTS, "mono", 1.0);

	config.set_default_layout(layout_dataprint);
}

ROM_START( dp407 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("nsm_118_576__firmware_4.07.bin", 0x00000, 0x10000, CRC(fa87ccb3) SHA1(55e9b845c10e1c4e8a524057c10fb2c3abf0b515))
ROM_END

ROM_START( dp411 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("nsm_118_576__firmware_4.11.bin", 0x00000, 0x10000, CRC(ee11ea18) SHA1(7a402e7a5c1ea92af28d667fc8bc57a3359346f3))
ROM_END

ROM_START( dp412 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("nsm_118_576__firmware_4.12.bin", 0x00000, 0x10000, CRC(264aa7c8) SHA1(72e49328eb44d2696ff68167a33ee7712663361f))
ROM_END

} // anonymous namespace


SYST( 199?, dp407, dp412, 0, dp3000, dp3000, dataprint_state, empty_init, "NSM", "Dataprint 3000 (4.07)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 199?, dp411, dp412, 0, dp3000, dp3000, dataprint_state, empty_init, "NSM", "Dataprint 3000 (4.11)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 199?, dp412,     0, 0, dp3000, dp3000, dataprint_state, empty_init, "NSM", "Dataprint 3000 (4.12)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
