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

	DECLARE_WRITE_LINE_MEMBER(nmi_w);

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


WRITE_LINE_MEMBER(dms86_state::nmi_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
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


void dms86_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).ram();
	map(0xfe000, 0xfffff).rom().region("roms", 0);
}

void dms86_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x87).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	map(0x88, 0x8f).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0x90, 0x97).rw("sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	map(0x9A, 0x9B).r(this, FUNC(dms86_state::port9a_r)); // parallel SASI port
	map(0x9c, 0x9d).r(this, FUNC(dms86_state::port9c_r));
	map(0x9c, 0x9c).w("terminal", FUNC(generic_terminal_device::write));
}

/* Input ports */
static INPUT_PORTS_START( dms86 )
	PORT_START("FRONT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Interrupt") PORT_CODE(KEYCODE_F2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, dms86_state, nmi_w)
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
	MCFG_DEVICE_ADD("maincpu", I8086, XTAL(14'745'600) / 3) // according to the manual... hmm
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	// According to the manual the clock is 14,765,600 / 4 but that couldn't possibly work.
	// By maths, clock should be 9600*32*4*16 = 19,660,800 but not working either
	// So, commented out because it makes the whole thing crawl, only get 18% on my machine
	//MCFG_DEVICE_ADD("ctc_clock", CLOCK, 19660800) //XTAL(14'745'600) / 4)
	//MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("ctc" ,z80ctc_device, trg0))
	//MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc" ,z80ctc_device, trg1))
	//MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc" ,z80ctc_device, trg2))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(14'745'600) / 3)
	//MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))    // frame rate interrupt to maincpu
	MCFG_Z80CTC_ZC0_CB(WRITELINE("sio1", z80sio_device, rxtxcb_w))    // SIO1 Ch B
	MCFG_Z80CTC_ZC1_CB(WRITELINE("sio1", z80sio_device, txca_w))       // SIO1 Ch A
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio1" ,z80sio_device, rxca_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE("sio2", z80sio_device, rxtxcb_w))       // SIO2
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio2" ,z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio2" ,z80sio_device, rxca_w))

	MCFG_DEVICE_ADD("sio1", Z80SIO, XTAL(14'745'600) / 3)
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(WRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(WRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio1", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio1", z80sio_device, dcdb_w)) // HiNet / Monitor switch
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio1", z80sio_device, ctsb_w)) MCFG_DEVCB_INVERT

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

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                 FULLNAME  FLAGS */
COMP( 1982, dms86, 0,      0,      dms86,   dms86, dms86_state, empty_init, "Digital Microsystems", "DMS-86", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
