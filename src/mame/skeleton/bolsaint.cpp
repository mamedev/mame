// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************

80188-based slot machines from Sleic/Petaco.

For "Bolsa Internacional", a complete manual with schematics can be downloaded from
  https://www.recreativas.org/manuales

Hardware info for "Bolsa Internacional":
  CPU PCB
    2 x OKI M82C55A-2
    Dallas DS1644-120 timekeeper RAM
    1 x OKI M82C51A-2
    Xtal 20.000 MHz
    AMD N80C188-20
  Sound PCB
    OKI M6376
    Xtal 5.0000 MHz

****************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "sound/okim6376.h"
#include "speaker.h"

#include "bus/rs232/rs232.h"

namespace {

class bolsaint_state : public driver_device
{
public:
	bolsaint_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi%u", 1U)
		, m_uart(*this, "uart")
		, m_oki(*this, "oki")
	{
	}

	void bolsaint(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i80188_cpu_device> m_maincpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<i8251_device> m_uart;
	required_device<okim6376_device> m_oki;

	void mem_map(address_map &map) ATTR_COLD;
	void peripheral_ctrl(offs_t offset, u16 data);
};

void bolsaint_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x3ffff).rom().region("bios", 0);
	map(0x40000, 0x5ffff).ram();
	// TODO: mapped by peripheral_ctrl
	map(0xa0000, 0xa0003).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0080, 0xa0083).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0100, 0xa0100).rw(m_uart, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xa0101, 0xa0101).rw(m_uart, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xc0000, 0xfffff).rom().region("bios", 0x40000);
}

void bolsaint_state::peripheral_ctrl(offs_t offset, u16 data)
{
	logerror("peripheral_ctrl %04x %04x\n", offset, data);
}

static INPUT_PORTS_START(bolsaint)
INPUT_PORTS_END

void bolsaint_state::machine_start()
{
}

void bolsaint_state::bolsaint(machine_config &config)
{
	I80188(config, m_maincpu, 20.0000_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &bolsaint_state::mem_map);
	m_maincpu->chip_select_callback().set(FUNC(bolsaint_state::peripheral_ctrl));
	m_maincpu->tmrout1_handler().set(m_uart, FUNC(i8251_device::write_txc));
	m_maincpu->tmrout1_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	I8255A(config, m_ppi[0]); // OKI M82C55A-2, on CPU PCB
	I8255A(config, m_ppi[1]); // OKI M82C55A-2, on CPU PCB

	// TODO: route RSCLK
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart->rxrdy_handler().set_inputline(m_maincpu, 0);
	m_uart->txrdy_handler().set_inputline(m_maincpu, 1);

	// Thru a MAX236
	// TODO: expects a specific device that also controls test mode
	// cfr pg. 31 "placa totalizadores y rs232"
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set(m_uart, FUNC(i8251_device::write_cts));

	SPEAKER(config, "mono").front_center();
	// There is a jumper on the sound PCB for selecting between 16KHz and 32KHz,
	// but is fixed (soldered) for 32KHz
	OKIM6376(config, m_oki, 5.0000_MHz_XTAL/8/2).add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

// Bolsa Internacional (Euro).
// Only two units were ever made for the euro-adapted version (there's a previous version which only supports pesetas).
ROM_START(bolsaint)
	ROM_REGION(0x080000, "bios", 0)
	// The machine serial number is hardcoded on the program ROM
	ROM_LOAD("sleic_bolsa_internacional_bi001-v3.22.ic6", 0x00000, 0x80000, CRC(3cfb3da0) SHA1(d003ec8edf1d85f03502bad33a286ec4fccb6ce2))

	ROM_REGION(0x100000, "oki", 0)
	// Four sockets, only two populated
	ROM_LOAD("bolsa_internacional_bisound01-1.ic4",       0x00000, 0x80000, CRC(4210a665) SHA1(2a8af1be8d8adfc1c630b8dd2babaa9e0f06f696))
	ROM_LOAD("bolsa_internacional_bisound02-2.ic5",       0x80000, 0x80000, CRC(910fa2d3) SHA1(0d48e22ef01865947d4716926406f40c126d639a))

	ROM_REGION(0x117, "plds", 0)
	// On the "roulette" PCB ("SLEIC-PETACO 011-114")
	ROM_LOAD("palce16v8h.bin", 0x000, 0x117, NO_DUMP)
ROM_END

} // anonymous namespace

//   YEAR  NAME       PARENT MACHINE   INPUT     CLASS            INIT        ROT   COMPANY         FULLNAME                      FLAGS
GAME(2000, bolsaint,  0,     bolsaint, bolsaint, bolsaint_state,  empty_init, ROT0, "Sleic/Petaco", "Bolsa Internacional (euro)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK) // VER.1.0 found on ROM string, but EPROM label reads V3.22
