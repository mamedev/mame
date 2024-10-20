// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

MITS Altair 8800b Turnkey

2009-12-04 Initial driver by Miodrag Milanovic

Supposedly introduced October 1977.

Commands:
 All commands must be in uppercase. Address and data is
 specified in Octal format (not hex).

 Press space to input your command line (not return).

D - Memory Dump
J - Jump to address
M - Modify memory

Reference:
 http://www.computercloset.org/MITSAltair8800bt.htm

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "cpu/i8085/i8085.h"
#include "machine/6850acia.h"
#include "machine/f4702.h"
#include "imagedev/snapquik.h"

namespace {

class altair_state : public driver_device
{
public:
	altair_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "mainram")
	{ }

	void altair(machine_config &config);

private:
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	virtual void machine_reset() override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
};



void altair_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xfcff).ram().share("mainram");
	map(0xfd00, 0xfdff).rom().region("maincpu",0);
	map(0xff00, 0xffff).rom().region("maincpu",0x100);
}

void altair_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// TODO: Remove mirror() and use SIO address S0-S7
	map(0x00, 0x01).mirror(0x10).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}

/* Input ports */
static INPUT_PORTS_START( altair )
	PORT_START("BAUD")
	PORT_DIPNAME(0xf, 0x8, "Bit Rate") PORT_DIPLOCATION("S3-S0:4,3,2,1")
	PORT_DIPSETTING(0x0, "External Rate")
	PORT_DIPSETTING(0x2, "50")
	PORT_DIPSETTING(0x3, "75")
	PORT_DIPSETTING(0xf, "110")
	PORT_DIPSETTING(0x4, "134.5")
	PORT_DIPSETTING(0xe, "150")
	PORT_DIPSETTING(0x5, "200")
	PORT_DIPSETTING(0xd, "300")
	PORT_DIPSETTING(0x6, "600")
	PORT_DIPSETTING(0xb, "1200")
	PORT_DIPSETTING(0xa, "1800")
	PORT_DIPSETTING(0x7, "2400")
	PORT_DIPSETTING(0x9, "4800")
	PORT_DIPSETTING(0x8, "9600")
INPUT_PORTS_END


QUICKLOAD_LOAD_MEMBER(altair_state::quickload_cb)
{
	int const quick_length = image.length();
	if (quick_length >= 0xfd00)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());
	int const read_ = image.fread(m_ram, quick_length);
	if (read_ != quick_length)
		return std::make_pair(image_error::UNSPECIFIED, std::string());

	return std::make_pair(std::error_condition(), std::string());
}

void altair_state::machine_reset()
{
	// Set startup address done by turn-key
	m_maincpu->set_state_int(i8080_cpu_device::I8085_PC, 0xFD00);
}

void altair_state::altair(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &altair_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &altair_state::io_map);

	/* video hardware */
	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.dcd_handler().set("acia", FUNC(acia6850_device::write_dcd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));
	rs232.txc_handler().set("brg", FUNC(f4702_device::im_w)); // molex pin 7 to be connected to cable pin 15

	f4702_device &brg(F4702(config, "brg", 2.4576_MHz_XTAL));
	brg.s_callback().set_ioport("BAUD");
	brg.z_callback().set("acia", FUNC(acia6850_device::write_txc));
	brg.z_callback().append("acia", FUNC(acia6850_device::write_rxc));

	/* quickload */
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(altair_state::quickload_cb));
}

/* ROM definition */
ROM_START( al8800bt )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "turnmon.bin",  0x0000, 0x0100, CRC(5c629294) SHA1(125c76216954b681721fff84a3aca05094b21a28))
	ROM_LOAD( "88dskrom.bin", 0x0100, 0x0100, CRC(7c5232f3) SHA1(24f940ad70ad2829e1bc800c6790b6e993e6ebf6))
ROM_END

} // Anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY  FULLNAME         FLAGS
COMP( 1977, al8800bt, 0,      0,      altair,  altair,  altair_state, empty_init, "MITS",  "Altair 8800bt", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
