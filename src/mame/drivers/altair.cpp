// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        MITS Altair 8800b Turnkey

        04/12/2009 Initial driver by Miodrag Milanovic

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
#include "machine/clock.h"
#include "imagedev/snapquik.h"


class altair_state : public driver_device
{
public:
	altair_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
	{ }

	void altair(machine_config &config);

protected:
	DECLARE_QUICKLOAD_LOAD_MEMBER(altair);

	virtual void machine_reset() override;
	void io_map(address_map &map);
	void mem_map(address_map &map);

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
};



void altair_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xfcff).ram().share("ram");
	map(0xfd00, 0xfdff).rom();
	map(0xff00, 0xffff).rom();
}

void altair_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// TODO: Remove AM_MIRROR() and use SIO address S0-S7
	map(0x00, 0x01).mirror(0x10).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}

/* Input ports */
static INPUT_PORTS_START( altair )
INPUT_PORTS_END


QUICKLOAD_LOAD_MEMBER( altair_state,altair)
{
	int quick_length;
	int read_;
	quick_length = image.length();
	if (quick_length >= 0xfd00)
		return image_init_result::FAIL;
	read_ = image.fread(m_ram, quick_length);
	if (read_ != quick_length)
		return image_init_result::FAIL;

	return image_init_result::PASS;
}

void altair_state::machine_reset()
{
	// Set startup address done by turn-key
	m_maincpu->set_state_int(i8080_cpu_device::I8085_PC, 0xFD00);
}

MACHINE_CONFIG_START(altair_state::altair)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8080, 2_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	/* video hardware */
	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE("acia", acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(WRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600) // TODO: this is set using jumpers S3/S2/S1/S0
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("acia", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("acia", acia6850_device, write_rxc))

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", altair_state, altair, "bin", 0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( al8800bt )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "turnmon.bin",  0xfd00, 0x0100, CRC(5c629294) SHA1(125c76216954b681721fff84a3aca05094b21a28))
	ROM_LOAD( "88dskrom.bin", 0xff00, 0x0100, CRC(7c5232f3) SHA1(24f940ad70ad2829e1bc800c6790b6e993e6ebf6))
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY  FULLNAME         FLAGS
COMP( 1977, al8800bt, 0,      0,      altair,  altair,  altair_state, empty_init, "MITS",  "Altair 8800bt", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
