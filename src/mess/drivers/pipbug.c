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

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/terminal.h"
#include "imagedev/snapquik.h"


class pipbug_state : public driver_device
{
public:
	pipbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_terminal(*this, TERMINAL_TAG) { }

	DECLARE_WRITE8_MEMBER(pipbug_ctrl_w);
	DECLARE_READ8_MEMBER(pipbug_serial_r);
	DECLARE_WRITE8_MEMBER(pipbug_serial_w);
	required_device<serial_terminal_device> m_terminal;
};

WRITE8_MEMBER( pipbug_state::pipbug_ctrl_w )
{
// 0x80 is written here - not connected in the baby 2650
}

READ8_MEMBER( pipbug_state::pipbug_serial_r )
{
	return m_terminal->tx_r();
}

WRITE8_MEMBER( pipbug_state::pipbug_serial_w )
{
	m_terminal->rx_w(data);
}

static ADDRESS_MAP_START(pipbug_mem, AS_PROGRAM, 8, pipbug_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x03ff) AM_ROM
	AM_RANGE( 0x0400, 0x7fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pipbug_io, AS_IO, 8, pipbug_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_WRITE(pipbug_ctrl_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_FO_PORT) AM_READWRITE(pipbug_serial_r,pipbug_serial_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pipbug )
INPUT_PORTS_END

static const serial_terminal_interface terminal_intf =
{
	DEVCB_NULL
};

QUICKLOAD_LOAD( pipbug )
{
	address_space &space = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);
	int i;
	int quick_addr = 0x0440;
	int exec_addr;
	int quick_length;
	UINT8 *quick_data;
	int read_;

	quick_length = image.length();
	quick_data = (UINT8*)malloc(quick_length);
	if (!quick_data)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot open file");
		image.message(" Cannot open file");
		return IMAGE_INIT_FAIL;
	}

	read_ = image.fread( quick_data, quick_length);
	if (read_ != quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
		image.message(" Cannot read the file");
		return IMAGE_INIT_FAIL;
	}

	if (quick_data[0] != 0xc4)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
		image.message(" Invalid header");
		return IMAGE_INIT_FAIL;
	}

	exec_addr = quick_data[1] * 256 + quick_data[2];

	if (exec_addr >= quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
		image.message(" Exec address beyond end of file");
		return IMAGE_INIT_FAIL;
	}

	if (quick_length < 0x444)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
		return IMAGE_INIT_FAIL;
	}

	if (quick_length > 0x8000)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
		return IMAGE_INIT_FAIL;
	}

	for (i = quick_addr; i < quick_length; i++)
	{
		space.write_byte(i, quick_data[i]);
	}

	/* display a message about the loaded quickload */
	image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

	// Start the quickload
	image.device().machine().device("maincpu")->state().set_pc(exec_addr);
	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( pipbug, pipbug_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(pipbug_mem)
	MCFG_CPU_IO_MAP(pipbug_io)

	/* video hardware */
	MCFG_SERIAL_TERMINAL_ADD(TERMINAL_TAG, terminal_intf, 110)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", pipbug, "pgm", 1)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( pipbug )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pipbug.rom", 0x0000, 0x0400, CRC(f242b93e) SHA1(f82857cc882e6b5fc9f00b20b375988024f413ff))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1979, pipbug,  0,       0,     pipbug,    pipbug, driver_device,    0,  "Signetics", "PIPBUG", GAME_NO_SOUND_HW )
