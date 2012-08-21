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

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/terminal.h"
#include "imagedev/snapquik.h"


class altair_state : public driver_device
{
public:
	altair_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	,
		m_ram(*this, "ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(sio_status_r);
	DECLARE_READ8_MEMBER(sio_data_r);
	DECLARE_READ8_MEMBER(sio_key_status_r);
	DECLARE_WRITE8_MEMBER(sio_command_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	required_shared_ptr<UINT8> m_ram;
};



READ8_MEMBER(altair_state::sio_status_r)
{
	return (m_term_data) ? 1 : 2;
}

WRITE8_MEMBER(altair_state::sio_command_w)
{

}

READ8_MEMBER(altair_state::sio_data_r)
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER(altair_state::sio_key_status_r)
{
	return (m_term_data) ? 0x40 : 0x01;
}

static ADDRESS_MAP_START(altair_mem, AS_PROGRAM, 8, altair_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xfcff ) AM_RAM AM_SHARE("ram")
	AM_RANGE( 0xfd00, 0xfdff ) AM_ROM
	AM_RANGE( 0xff00, 0xffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(altair_io, AS_IO, 8, altair_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_READ(sio_key_status_r)
	AM_RANGE( 0x01, 0x01 ) AM_MIRROR(0x10) AM_READ(sio_data_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE(sio_status_r,sio_command_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( altair )
INPUT_PORTS_END


QUICKLOAD_LOAD(altair)
{
	altair_state *state = image.device().machine().driver_data<altair_state>();
	int quick_length;
	int read_;
	quick_length = image.length();
	if (quick_length >= 0xfd00)
		return IMAGE_INIT_FAIL;
	read_ = image.fread(state->m_ram, quick_length);
	if (read_ != quick_length)
		return IMAGE_INIT_FAIL;

	return IMAGE_INIT_PASS;
}


static MACHINE_RESET(altair)
{
	altair_state *state = machine.driver_data<altair_state>();
	// Set startup addess done by turn-key
	cpu_set_reg(machine.device("maincpu"), I8085_PC, 0xFD00);

	state->m_term_data = 0;
}

WRITE8_MEMBER( altair_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(altair_state, kbd_put)
};

static MACHINE_CONFIG_START( altair, altair_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(altair_mem)
	MCFG_CPU_IO_MAP(altair_io)

	MCFG_MACHINE_RESET(altair)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", altair, "bin", 0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( al8800bt )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "turnmon.bin",  0xfd00, 0x0100, CRC(5c629294) SHA1(125c76216954b681721fff84a3aca05094b21a28))
	ROM_LOAD( "88dskrom.bin", 0xff00, 0x0100, CRC(7c5232f3) SHA1(24f940ad70ad2829e1bc800c6790b6e993e6ebf6))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1975, al8800bt,  0,       0,	altair, 	altair, driver_device,	 0,   "MITS",   "Altair 8800bt", GAME_NOT_WORKING | GAME_NO_SOUND_HW)

