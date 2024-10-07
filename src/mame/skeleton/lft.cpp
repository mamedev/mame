// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************

2013-09-09 Skeleton of LFT computer system.

These are the monitor programs for 80186-based "S100+" systems by L/F Technologies. Many
of these boards were distributed under the IMS International name. Documentation available
online is woefully inadequate.

LFT1230 does extensive testing of devices at boot time. As they are mostly missing,
it gets caught in a loop. There's unknown devices in the i/o 008x, 00Ax, 00Cx range.
The devices at 008x and 00Cx would appear to be the same type.
BUG:If the rtc fails to clear the interrupt pin (first test), it cannot print the error
as the SCC hasn't yet been set up.
***Status: hangs at start


LFT1510 doesn't bother with the tests. The i/o ports seem to be the same.
***Status: mostly working


Note: Backspace/delete performs oddly.

******************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/mm58167.h"
#include "machine/z80scc.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"


namespace {

class lft_state : public driver_device
{
public:
	lft_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
	{ }

	void lft(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<mm58167_device> m_rtc;
	required_device<scc8530_device> m_scc;
};

void lft_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x5ffff).ram();
	map(0xfc000, 0xfffff).rom().region("roms", 0);
}

void lft_state::io_map(address_map &map)
{
	map.unmap_value_high();
	//map.global_mask(0x3ff);
	//map(0x0080, 0x0087) // unknown device
	//map(0x00a0, 0x00a?) // unknown device
	//map(0x00c0, 0x00c7) // unknown device
	map(0x0100, 0x0107).rw(m_scc, FUNC(z80scc_device::dc_ab_r), FUNC(z80scc_device::dc_ab_w)).umask16(0x00ff);
	map(0x0180, 0x01bf).rw(m_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0x00ff);
	//map(0x0200, 0x0207) // unknown device
}


/* Input ports */
static INPUT_PORTS_START( lft )
INPUT_PORTS_END

void lft_state::machine_reset()
{
}

void lft_state::lft(machine_config &config)
{
	/* basic machine hardware */
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &lft_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lft_state::io_map);

	// Devices
	MM58167(config, m_rtc, 32.768_kHz_XTAL);

	SCC8530N(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(scc8530_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(scc8530_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(scc8530_device::ctsa_w));

	m_scc->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, "terminal"));
	rs232b.rxd_handler().set(m_scc, FUNC(scc8530_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(scc8530_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(scc8530_device::ctsb_w));
}


/* ROM definition */
ROM_START( lft1230 )
	ROM_REGION16_LE(0x4000, "roms", 0)
	ROM_LOAD16_BYTE( "1230lf29", 0x0000, 0x2000, CRC(11c87367) SHA1(0879650aa98e19a4e6ca7b6ee7874f81c9c8ccfa) )
	ROM_LOAD16_BYTE( "1230lf42", 0x0001, 0x2000, CRC(ab82b620) SHA1(8c7d93950703f348e5ce0f9e376d157dd6098c6a) )
ROM_END

ROM_START( lft1510 )
	ROM_REGION16_LE(0x4000, "roms", 0)
	ROM_LOAD16_BYTE( "1510lfev", 0x2000, 0x1000, CRC(47dbb290) SHA1(b557e9a54a30d9a16edfdef4a6b12a5393d30bf3) )
	ROM_IGNORE(0x1000)
	ROM_LOAD16_BYTE( "1510lfod", 0x2001, 0x1000, CRC(ba8c23fc) SHA1(d4b82f69fccd653b31e7bd05ee884b323ff0007b) )
	ROM_IGNORE(0x1000)
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY             FULLNAME                      FLAGS
COMP( 1986, lft1510, 0,       0,      lft,     lft,   lft_state, empty_init, "L/F Technologies", "A1510 186 User Processor",   MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
COMP( 1985, lft1230, lft1510, 0,      lft,     lft,   lft_state, empty_init, "L/F Technologies", "A1230 186 Master Processor", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
