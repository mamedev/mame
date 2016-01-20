// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        MITS Altair 8800b Turnkey

        04/12/2009 Initial driver by Miodrag Milanovic

        Commands:
        All commands must be in uppercase. Address and data is
        specified in Octal format (not hex).

        Press space to input your command line (not return).

        D - Memory Dump
        J - Jump to address
        M - Modify memory

****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "imagedev/snapquik.h"


class altair_state : public driver_device
{
public:
	altair_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mc6850(*this, "mc6850"),
		m_ram(*this, "ram")
	{
	}

	DECLARE_QUICKLOAD_LOAD_MEMBER(altair);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);

protected:
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_mc6850;
	required_shared_ptr<UINT8> m_ram;
};



static ADDRESS_MAP_START(altair_mem, AS_PROGRAM, 8, altair_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xfcff ) AM_RAM AM_SHARE("ram")
	AM_RANGE( 0xfd00, 0xfdff ) AM_ROM
	AM_RANGE( 0xff00, 0xffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(altair_io, AS_IO, 8, altair_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// TODO: Remove AM_MIRROR() and use SIO address S0-S7
	AM_RANGE( 0x00, 0x00 ) AM_MIRROR(0x10) AM_DEVREADWRITE("mc6850", acia6850_device, status_r, control_w)
	AM_RANGE( 0x01, 0x01 ) AM_MIRROR(0x10) AM_DEVREADWRITE("mc6850", acia6850_device, data_r, data_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( altair )
INPUT_PORTS_END


QUICKLOAD_LOAD_MEMBER( altair_state,altair)
{
	int quick_length;
	int read_;
	quick_length = image.length();
	if (quick_length >= 0xfd00)
		return IMAGE_INIT_FAIL;
	read_ = image.fread(m_ram, quick_length);
	if (read_ != quick_length)
		return IMAGE_INIT_FAIL;

	return IMAGE_INIT_PASS;
}

WRITE_LINE_MEMBER(altair_state::write_acia_clock)
{
	m_mc6850->write_txc(state);
	m_mc6850->write_rxc(state);
}

void altair_state::machine_reset()
{
	// Set startup addess done by turn-key
	m_maincpu->set_state_int(I8085_PC, 0xFD00);
}

static MACHINE_CONFIG_START( altair, altair_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(altair_mem)
	MCFG_CPU_IO_MAP(altair_io)

	/* video hardware */
	MCFG_DEVICE_ADD("mc6850", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("mc6850", acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("mc6850", acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("mc6850", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600) // TODO: this is set using jumpers S3/S2/S1/S0
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(altair_state, write_acia_clock))

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

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1975, al8800bt,  0,       0,  altair,     altair, driver_device,   0,   "MITS",   "Altair 8800bt", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
