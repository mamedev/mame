// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Digital Microsystems DMS-86

2010-01-11 Skeleton driver.

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


namespace {

class dms86_state : public driver_device
{
public:
	dms86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_sio(*this, "sio%u", 1U)
		, m_ctc(*this, "ctc")
	{ }

	void dms86(machine_config &config);
	void nmi_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void m1_ack_w(u8 data);

	u16 port9a_r();
	u16 port9c_r();
	void kbd_put(u8 data);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_term_data = 0U;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device_array<z80sio_device, 2> m_sio;
	required_device<z80ctc_device> m_ctc;
};


void dms86_state::nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}


void dms86_state::m1_ack_w(u8 data)
{
	m_sio[0]->z80daisy_decode(data);
	m_sio[1]->z80daisy_decode(data);
	m_ctc->z80daisy_decode(data);
}


u16 dms86_state::port9a_r()
{
	return m_term_data ? 0x40 : 0;
}

u16 dms86_state::port9c_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}


void dms86_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).ram();
	map(0xfe000, 0xfffff).rom().region("roms", 0);
	map(0xfed03, 0xfed03).w(FUNC(dms86_state::m1_ack_w));
}

void dms86_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x87).rw(m_sio[0], FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	map(0x88, 0x8f).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0x90, 0x97).rw(m_sio[1], FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	//map(0x98, 0x99)  // r SASI data read ; w SASI SEL probe
	map(0x9A, 0x9B).r(FUNC(dms86_state::port9a_r)); // r parallel ports, status byte ; w SASI ACK strobe, SASI data write
	// 9C,9D - no setup bytes; meant for printer // r Port P data read ; Port P data write
	map(0x9c, 0x9d).r(FUNC(dms86_state::port9c_r));
	map(0x9c, 0x9c).w(m_terminal, FUNC(generic_terminal_device::write));
	//map(0x9e, 0x9f)  // r SASI software reset ; w Port P AUX strobe
}

/* Input ports */
static INPUT_PORTS_START( dms86 )
	PORT_START("FRONT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Interrupt") PORT_CODE(KEYCODE_F2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, dms86_state, nmi_w)
INPUT_PORTS_END


void dms86_state::machine_start()
{
	save_item(NAME(m_term_data));

	m_term_data = 0;
}

void dms86_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void dms86_state::dms86(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(14'745'600) / 3); // according to the manual... hmm
	m_maincpu->set_addrmap(AS_PROGRAM, &dms86_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &dms86_state::io_map);

	// According to the manual the clock is 14,765,600 / 4 but that's wrong
	Z80CTC(config, m_ctc, XTAL(14'745'600) / 3);
	//m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);             // frame rate interrupt to maincpu
	m_ctc->zc_callback<0>().set(m_sio[0], FUNC(z80sio_device::rxtxcb_w));  // SIO1 Ch B
	m_ctc->zc_callback<1>().set(m_sio[0], FUNC(z80sio_device::txca_w));    // SIO1 Ch A
	m_ctc->zc_callback<1>().append(m_sio[0], FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<2>().set(m_sio[1], FUNC(z80sio_device::rxtxcb_w));  // SIO2
	m_ctc->zc_callback<2>().append(m_sio[1], FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<2>().append(m_sio[1], FUNC(z80sio_device::rxca_w));

	Z80SIO(config, m_sio[0], XTAL(14'745'600) / 3);
	m_sio[0]->out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_sio[0]->out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	Z80SIO(config, m_sio[1], XTAL(14'745'600) / 3);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_sio[0], FUNC(z80sio_device::rxb_w));
	rs232.dcd_handler().set(m_sio[0], FUNC(z80sio_device::dcdb_w)); // HiNet / Monitor switch
	rs232.cts_handler().set(m_sio[0], FUNC(z80sio_device::ctsb_w)).invert();

	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(dms86_state::kbd_put));
}

/* ROM definition */
ROM_START( dms86 )
	ROM_REGION16_LE( 0x2000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "hns-86_54-8678.bin", 0x0000, 0x1000, CRC(95f58e1c) SHA1(6fc8f087f0c887d8b429612cd035c6c1faab570c))
	ROM_LOAD16_BYTE( "hns-86_54-8677.bin", 0x0001, 0x1000, CRC(78fad756) SHA1(ddcbff1569ec6975b8489935cdcfa80eba413502))
ROM_END

} // Anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                 FULLNAME  FLAGS */
COMP( 1982, dms86, 0,      0,      dms86,   dms86, dms86_state, empty_init, "Digital Microsystems", "DMS-86", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
