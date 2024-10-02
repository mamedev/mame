// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    BERT apparently stands for "Basic Einplatinen Rechner für TV-Serie." It is
    a small educational single-board computer manufactured by the West German
    firm Thomsen-Elektronik, based on the Z8671 microcontroller with a built-in
    BASIC/DEBUG ROM.

    Besides a 5-pin DIN connector for the terminal or PC, the board makes four
    generic input and/or output ports available to the user through pin headers.
    The first two ports, including the fully bidirectional port A, are directly
    wired to the processor. Two other ports are read or written through memory-
    mapped latches, and a fifth port represents the onboard DIP switches.

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z8/z8.h"
#include "machine/ram.h"


namespace {

class bert_state : public driver_device
{
public:
	bert_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mpu(*this, "mpu")
		, m_ram(*this, RAM_TAG)
		, m_serial(*this, "serial")
	{
	}

	void bert(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void portb_w(u8 data);
	void portd_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<z8_device> m_mpu;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_serial;
};

void bert_state::machine_start()
{
	m_mpu->space(AS_PROGRAM).install_ram(0x2000, 0x2000 + m_ram->size() - 1, 0x8000, m_ram->pointer());
}

void bert_state::portb_w(u8 data)
{
	m_serial->write_txd(BIT(data, 7));
}

void bert_state::portd_w(u8 data)
{
}

void bert_state::mem_map(address_map &map)
{
	// A15 (P07) is only decoded internally
	map(0x0800, 0x0fff).mirror(0x8000).noprw();
	map(0x1000, 0x1fff).mirror(0x8000).rom().region("eprom", 0).nopw(); // 2732
	map(0x5000, 0x5000).mirror(0x8fff).portr("PORTC"); // 74LS373
	map(0x6000, 0x6000).mirror(0x8fff).w(FUNC(bert_state::portd_w)); // 74LS373
	map(0x7000, 0x7000).mirror(0x8fff).portr("PORTE"); // 74LS368
}

static INPUT_PORTS_START(bert)
	PORT_START("PORTA")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("PORTC")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("PORTE")
	PORT_DIPNAME(0x07, 0x02, "Baud Rate") PORT_DIPLOCATION("E:!1,!2,!3")
	PORT_DIPSETTING(0x06, "110")
	PORT_DIPSETTING(0x00, "150")
	PORT_DIPSETTING(0x07, "300")
	PORT_DIPSETTING(0x05, "1200")
	PORT_DIPSETTING(0x04, "2400")
	PORT_DIPSETTING(0x03, "4800")
	PORT_DIPSETTING(0x02, "9600")
	PORT_DIPSETTING(0x01, "19200")
	PORT_DIPNAME(0x08, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("E:!4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("E:!5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, DEF_STR(Unused)) PORT_DIPLOCATION("E:!6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void bert_state::bert(machine_config &config)
{
	Z8671(config, m_mpu, 7.3728_MHz_XTAL);
	m_mpu->set_addrmap(AS_PROGRAM, &bert_state::mem_map);
	m_mpu->p2_in_cb().set_ioport("PORTA");
	m_mpu->p3_out_cb().set(FUNC(bert_state::portb_w));

	RAM(config, m_ram).set_default_size("2K").set_extra_options("4K"); // two 6116 sockets

	RS232_PORT(config, m_serial, default_rs232_devices, "terminal"); // TTL-level, not EIA/CCITT ±12V
	m_serial->rxd_handler().set_inputline(m_mpu, INPUT_LINE_IRQ3).invert(); // no actual inverter
}

ROM_START(bert)
	ROM_REGION(0x1000, "eprom", 0)
	ROM_LOAD("bert.bin", 0x0000, 0x1000, CRC(52ece0e7) SHA1(d39cc9b6248547cfa8fd49d43532abf9399348a5))
ROM_END

} // anonymous namespace


COMP(1987, bert, 0, 0, bert, bert, bert_state, empty_init, "VGS Verlagsgesellschaft", "BERT", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
