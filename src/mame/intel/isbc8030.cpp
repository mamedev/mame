// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

        Intel SBC 80/30 Single Board Computer

System Initialization (Reset):
The USART clock is initially set at 9600 baud. Two "U" characters are used
to check for baud rate. When the first "U" character is entered it is
checked for 9600, 4800, 2400, and 1200 baud rate. If a match is found then
that baud rate is set into the clock. If not, then a second "U" character
must be entered. The second "U" character is checked for 600, 300, 150,
and 110 baud. When the baud rate has been successfully determined, the sign
-on message "80/30 MONITOR" will be displayed on the console. When the
monitor is ready for a command, it will prompt with a period ".".

Download the User Manual to get the operating procedures.

Monitor Commands:
D  Display memory command
G  Program execute command
N  Single step command
I  Insert instruction into memory
M  Move memory command
R  Read hexadecimal file
S  Substitute memory command
W  Write hexadecimal file
X  Examine and modify CPU registers

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


namespace {

class isbc8030_state : public driver_device
{
public:
	isbc8030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, "usart")
		, m_ppi(*this, "ppi8255")
		, m_pic(*this, "pic8259")
		, m_pit(*this, "pit8253")
		, m_rs232(*this, "rs232")
	{ }

	void isbc8030(machine_config &config);

private:
	void isbc8030_io(address_map &map) ATTR_COLD;
	void isbc8030_mem(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8251_device> m_usart;
	required_device<i8255_device> m_ppi;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<rs232_port_device> m_rs232;
};

void isbc8030_state::isbc8030_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0xffff).ram();
}

void isbc8030_state::isbc8030_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xd8, 0xd9).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xdc, 0xdf).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xe8, 0xeb).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xec, 0xed).mirror(0x02).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
}

static INPUT_PORTS_START( isbc8030 )
INPUT_PORTS_END

void isbc8030_state::isbc8030(machine_config &config)
{
	I8085A(config, m_maincpu, XTAL(22'118'400) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &isbc8030_state::isbc8030_mem);
	m_maincpu->set_addrmap(AS_IO, &isbc8030_state::isbc8030_io);
	m_maincpu->in_inta_func().set(m_pic, FUNC(pic8259_device::acknowledge));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(XTAL(22'118'400) / 18);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(XTAL(22'118'400) / 18);
	m_pit->set_clk<2>(XTAL(22'118'400) / 18);
	m_pit->out_handler<2>().set(m_usart, FUNC(i8251_device::write_rxc));
	m_pit->out_handler<2>().append(m_usart, FUNC(i8251_device::write_txc));

	I8251(config, m_usart, 0);
	m_usart->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));

	I8255A(config, m_ppi, 0);

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	m_rs232->cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
}

/* ROM definition */
ROM_START( isbc8030 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon830.bin", 0x0000, 0x0800, CRC(cda15115) SHA1(242dad14a919568178b363c3e27f22ec0a5849b3))
ROM_END

} // anonymous namespace


/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME      FLAGS */
COMP( 1978, isbc8030, 0,      0,      isbc8030, isbc8030, isbc8030_state, empty_init, "Intel", "iSBC 80/30", MACHINE_NO_SOUND_HW )
