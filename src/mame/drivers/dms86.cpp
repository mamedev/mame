// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Digital Microsystems DMS-86

        11/01/2010 Skeleton driver.

Monitor commands:
A Display incoming terminal data in hex
B boot to HiNet
C copy memory
D dump memory to screen
F fill memory
G go (eg G address [breakpoint])
I in port
M memory test
O out port (e.g. O 84 77)
P set segment
S write a byte to memory
T type ascii string into the address set by D or S
X display registers
? help
: load intel hex format file


Note that bit 3 of port 82 is tested at boot. If low, the computer bypasses
the monitor and goes straight to "Joining HiNet".

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
#include "machine/terminal.h"


class dms86_state : public driver_device
{
public:
	dms86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ16_MEMBER( port82_r );
	DECLARE_READ16_MEMBER( port9a_r );
	DECLARE_READ16_MEMBER( port9c_r );
	void kbd_put(u8 data);

	void dms86(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	u8 m_term_data;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};


READ16_MEMBER( dms86_state::port82_r )
{
// HiNet / Monitor switch

	return 8;
}

READ16_MEMBER( dms86_state::port9a_r )
{
	return m_term_data ? 0x40 : 0;
}

READ16_MEMBER( dms86_state::port9c_r )
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}


ADDRESS_MAP_START(dms86_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("roms",0)
ADDRESS_MAP_END

ADDRESS_MAP_START(dms86_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x82, 0x83) AM_READ(port82_r)
	//AM_RANGE(0x80, 0x87) AM_DEVREADWRITE8("sio1", z80sio_device, ba_cd_r, ba_cd_w, 0x00ff)
	AM_RANGE(0x88, 0x8f) AM_DEVREADWRITE8("ctc", z80ctc_device, read, write, 0x00ff)
	AM_RANGE(0x90, 0x97) AM_DEVREADWRITE8("sio2", z80sio_device, ba_cd_r, ba_cd_w, 0x00ff)
	AM_RANGE(0x9A, 0x9B) AM_READ(port9a_r) // parallel SASI port
	AM_RANGE(0x9c, 0x9d) AM_READ(port9c_r)
	AM_RANGE(0x9c, 0x9d) AM_DEVWRITE8("terminal", generic_terminal_device, write, 0x00ff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dms86 )
INPUT_PORTS_END


void dms86_state::machine_reset()
{
}

void dms86_state::kbd_put(u8 data)
{
	m_term_data = data;
}

MACHINE_CONFIG_START(dms86_state::dms86)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL(14'745'600) / 3) // according to the manual... hmm
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	// According to the manual the clock is 14,765,600 / 4 but that couldn't possibly work.
	// By maths, clock should be 9600*32*4*16 = 19,660,800 but not working either
	// So, commented out because it makes the whole thing crawl, only get 18% on my machine
	//MCFG_DEVICE_ADD("ctc_clock", CLOCK, 19660800) //XTAL(14'745'600) / 4)
	//MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc" ,z80ctc_device, trg0))
	//MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc" ,z80ctc_device, trg1))
	//MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc" ,z80ctc_device, trg2))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(14'745'600) / 3)
	//MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))    // frame rate interrupt to maincpu
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("sio1", z80sio_device, rxtxcb_w))    // SIO1 Ch B
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("sio1", z80sio_device, txca_w))       // SIO1 Ch A
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio1" ,z80sio_device, rxca_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("sio2", z80sio_device, rxtxcb_w))       // SIO2
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio2" ,z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio2" ,z80sio_device, rxca_w))

	MCFG_DEVICE_ADD("sio1", Z80SIO, XTAL(14'745'600) / 3)
	MCFG_Z80SIO_OUT_TXDB_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio1", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio1", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio1", z80sio_device, ctsb_w)) MCFG_DEVCB_INVERT

	MCFG_DEVICE_ADD("sio2", Z80SIO, XTAL(14'745'600) / 3)
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(PUT(dms86_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dms86 )
	ROM_REGION( 0x2000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "hns-86_54-8678.bin", 0x0000, 0x1000, CRC(95f58e1c) SHA1(6fc8f087f0c887d8b429612cd035c6c1faab570c))
	ROM_LOAD16_BYTE( "hns-86_54-8677.bin", 0x0001, 0x1000, CRC(78fad756) SHA1(ddcbff1569ec6975b8489935cdcfa80eba413502))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE         INIT  COMPANY                 FULLNAME  FLAGS */
COMP( 1982, dms86,  0,       0,      dms86,     dms86,  dms86_state,  0,    "Digital Microsystems", "DMS-86", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
