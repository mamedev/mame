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

#define I8259A_TAG      "pic8259"
#define I8253_TAG       "pit8253"
#define I8255A_TAG      "ppi8255"
#define I8251A_TAG      "usart"
#define I8251A_BAUD_TAG "usart_baud"
#define RS232_TAG       "rs232"

class isbc8030_state : public driver_device
{
public:
	isbc8030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, I8251A_TAG)
		, m_ppi(*this, I8255A_TAG)
		, m_pic(*this, I8259A_TAG)
		, m_pit(*this, I8253_TAG)
		, m_rs232(*this, RS232_TAG)
	{ }

	void isbc8030(machine_config &config);
	void isbc8030_io(address_map &map);
	void isbc8030_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_usart;
	required_device<i8255_device> m_ppi;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<rs232_port_device> m_rs232;
};

ADDRESS_MAP_START(isbc8030_state::isbc8030_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(isbc8030_state::isbc8030_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xd8, 0xd9) AM_DEVREADWRITE(I8259A_TAG, pic8259_device, read, write)
	AM_RANGE(0xdc, 0xdf) AM_DEVREADWRITE(I8253_TAG, pit8253_device, read, write)
	AM_RANGE(0xe8, 0xeb) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
	AM_RANGE(0xec, 0xec) AM_MIRROR(0x02) AM_DEVREADWRITE(I8251A_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0xed, 0xed) AM_MIRROR(0x02) AM_DEVREADWRITE(I8251A_TAG, i8251_device, status_r, control_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( isbc8030 )
INPUT_PORTS_END

MACHINE_CONFIG_START(isbc8030_state::isbc8030)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, XTAL(22'118'400) / 4)
	MCFG_CPU_PROGRAM_MAP(isbc8030_mem)
	MCFG_CPU_IO_MAP(isbc8030_io)

	MCFG_DEVICE_ADD(I8259A_TAG, PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", 0))

	MCFG_DEVICE_ADD(I8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(22'118'400) / 18)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8259A_TAG, pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL(22'118'400) / 18)
	MCFG_PIT8253_CLK2(XTAL(22'118'400) / 18)
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_rxc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE(I8251A_TAG, i8251_device, write_txc))

	MCFG_DEVICE_ADD(I8251A_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

	MCFG_DEVICE_ADD(I8255A_TAG, I8255A, 0)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( isbc8030 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon830.bin", 0x0000, 0x0800, CRC(cda15115) SHA1(242dad14a919568178b363c3e27f22ec0a5849b3))
ROM_END

/*    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT      CLASS            INIT   COMPANY   FULLNAME       FLAGS */
COMP( 1978, isbc8030,  0,        0,      isbc8030,  isbc8030,  isbc8030_state,  0,     "Intel",  "iSBC 80/30",  MACHINE_NO_SOUND_HW )
