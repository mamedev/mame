// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        PIPBUG

        08/04/2010 Skeleton driver.
        16/05/2012 Connected to serial terminal.. working

        All input must be in uppercase.

        Commands:
        A - See and alter memory
        B - Set breakpoint (2 permitted)
        C - Clear breakpoint
        D - Dump memory to paper tape
        G - Go to address, run
        L - Load memory from paper tape
        S - See and alter registers

        PIPBUG isn't a computer; it is a the name of the bios used
        in a number of small 2650-based computers from 1976 to 1978.
        Examples include Baby 2650, Eurocard 2650, etc., plus Signetics
        own PC1001, PC1500, and KT9500 systems. PIPBUG was written by Signetics.

        The sole means of communication is via a serial terminal.
        PIPBUG uses the SENSE and FLAG pins as serial lines, thus
        there is no need for a UART. The baud rate is 110.

        The Baby 2650 (featured in Electronics Australia magazine in
        March 1977) has 256 bytes of RAM.

        The terminal is expected to have a papertape device attached, and
        use it to save and load programs. PIPBUG still thinks it is talking
        to the terminal, when in fact the data is flowing to the papertape
        reader and punch.

****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/s2650/s2650.h"
#include "machine/terminal.h"
#include "imagedev/snapquik.h"


class pipbug_state : public driver_device
{
public:
	pipbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rs232(*this, "rs232"),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_WRITE8_MEMBER(pipbug_ctrl_w);
	required_device<rs232_port_device> m_rs232;
	required_device<cpu_device> m_maincpu;
	DECLARE_QUICKLOAD_LOAD_MEMBER( pipbug );
};

WRITE8_MEMBER( pipbug_state::pipbug_ctrl_w )
{
// 0x80 is written here - not connected in the baby 2650
}

static ADDRESS_MAP_START(pipbug_mem, AS_PROGRAM, 8, pipbug_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x03ff) AM_ROM
	AM_RANGE( 0x0400, 0x7fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pipbug_io, AS_IO, 8, pipbug_state)
//  ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_WRITE(pipbug_ctrl_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READNOP // this has to return zero or the parameter to write_sense is ignored
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pipbug )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_110 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_110 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

QUICKLOAD_LOAD_MEMBER( pipbug_state, pipbug )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x440;
	int exec_addr;
	int quick_length;
	dynamic_buffer quick_data;
	int read_;
	int result = IMAGE_INIT_FAIL;

	quick_length = image.length();
	if (quick_length < 0x0444)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
	}
	else if (quick_length > 0x8000)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
	}
	else
	{
		quick_data.resize(quick_length);
		read_ = image.fread( &quick_data[0], quick_length);
		if (read_ != quick_length)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
			image.message(" Cannot read the file");
		}
		else if (quick_data[0] != 0xc4)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
			image.message(" Invalid header");
		}
		else
		{
			exec_addr = quick_data[1] * 256 + quick_data[2];

			if (exec_addr >= quick_length)
			{
				image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
				image.message(" Exec address beyond end of file");
			}
			else
			{
				for (i = quick_addr; i < read_; i++)
					space.write_byte(i, quick_data[i]);

				/* display a message about the loaded quickload */
				image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

				// Start the quickload
				m_maincpu->set_state_int(S2650_PC, exec_addr);

				result = IMAGE_INIT_PASS;
			}
		}
	}

	return result;
}

static MACHINE_CONFIG_START( pipbug, pipbug_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(pipbug_mem)
	MCFG_CPU_IO_MAP(pipbug_io)
	MCFG_S2650_FLAG_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))

	/* video hardware */
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("maincpu", s2650_device, write_sense))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", pipbug_state, pipbug, "pgm", 1)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( pipbug )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pipbug.rom", 0x0000, 0x0400, CRC(f242b93e) SHA1(f82857cc882e6b5fc9f00b20b375988024f413ff))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1979, pipbug,  0,       0,     pipbug,    pipbug, driver_device,    0,  "Signetics", "PIPBUG", MACHINE_NO_SOUND_HW )
