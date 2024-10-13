// license:BSD-3-Clause
// copyright-holders:AJR
/*************************************************************************************************

    Skeleton driver for Roland PR-100 MIDI sequencer.

    There are two custom gate arrays: M60012-0105SP on the main board and M6003A-0117SP on the
    panel board. The main board gate array multiplexes addresses for the four MB81464 DRAMs and
    decodes all CPU I/O space accesses. It also generates a "metronome" signal which is this
    device's only audio output. The panel board's gate array is almost entirely controlled by
    the main gate array, with only the reset and power signals generated independently.

    The LCD unit is a DM0815.

*************************************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/z180/z180.h"
#include "machine/eepromser.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "mb87013.h"


namespace {

class roland_pr100_state : public driver_device
{
public:
	roland_pr100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_port9x(0)
	{
	}

	void pr100(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 port9x_r(offs_t offset);
	void port9x_w(offs_t offset, u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z180_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	u8 m_port9x;
};

void roland_pr100_state::machine_start()
{
	save_item(NAME(m_port9x));
}

u8 roland_pr100_state::port9x_r(offs_t offset)
{
	return m_port9x;
}

void roland_pr100_state::port9x_w(offs_t offset, u8 data)
{
	logerror("port%02X = %02X\n", offset + 0x90, data);
	m_port9x = data;
}


void roland_pr100_state::mem_map(address_map &map)
{
	map.global_mask(0x3ffff); // A18 not connected
	map(0x00000, 0x0ffff).mirror(0x10000).rom().region("program", 0); // CE = A17
	map(0x20000, 0x3ffff).ram();
}

void roland_pr100_state::io_map(address_map &map)
{
	map(0x0000, 0x003f).noprw(); // internal
	map(0x0090, 0x0093).mirror(0xff00).rw(FUNC(roland_pr100_state::port9x_r), FUNC(roland_pr100_state::port9x_w)); // EEPROM I/O?
	map(0x00a0, 0x00a3).rw("qdc", FUNC(mb87013_device::read), FUNC(mb87013_device::write));
}


static INPUT_PORTS_START(pr100)
INPUT_PORTS_END

void roland_pr100_state::pr100(machine_config &config)
{
	HD64180RP(config, m_maincpu, 10_MHz_XTAL); // HD64B180R0P
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_pr100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &roland_pr100_state::io_map);

	mb87013_device &qdc(MB87013(config, "qdc", 6.5_MHz_XTAL));
	qdc.sio_rd_callback().set("sio", FUNC(i8251_device::read));
	qdc.sio_wr_callback().set("sio", FUNC(i8251_device::write));
	qdc.txc_callback().set("sio", FUNC(i8251_device::write_txc));
	qdc.rxc_callback().set("sio", FUNC(i8251_device::write_rxc));
	qdc.rxd_callback().set("sio", FUNC(i8251_device::write_rxd));
	qdc.dsr_callback().set("sio", FUNC(i8251_device::write_dsr));
	qdc.op4_callback().set("qdc", FUNC(mb87013_device::rts_w));

	i8251_device &sio(I8251(config, "sio", 6.5_MHz_XTAL)); // MB89251
	sio.dtr_handler().set("qdc", FUNC(mb87013_device::dtr_w));
	sio.txd_handler().set("qdc", FUNC(mb87013_device::txd_w));
	sio.rxrdy_handler().set("siodrq", FUNC(input_merger_device::in_w<0>));
	sio.txrdy_handler().set("siodrq", FUNC(input_merger_device::in_w<0>));
	sio.write_cts(0);

	INPUT_MERGER_ANY_HIGH(config, "siodrq").output_handler().set_inputline(m_maincpu, Z180_INPUT_LINE_DREQ1); // 74ALS08

	EEPROM_93C46_16BIT(config, m_eeprom); // HY93C46
}

ROM_START(pr100)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("roland_mbm27c512-20.ic10", 0x00000, 0x10000, CRC(41160b69) SHA1(11e5fb001dd004a5625d9a75fb1acac4ade614c8))
ROM_END

ROM_START(pr100_201)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("pr100-201_mbm27c512-20.ic10", 0x00000, 0x10000, CRC(2f3bc01c) SHA1(114ff687ffac4d517edba77d427422c4d2a6d369))
ROM_END

} // anonymous namespace


SYST(1987, pr100,     0,     0, pr100, pr100, roland_pr100_state, empty_init, "Roland", "PR-100 Digital Sequencer (v2.02)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(1987, pr100_201, pr100, 0, pr100, pr100, roland_pr100_state, empty_init, "Roland", "PR-100 Digital Sequencer (v2.01)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
