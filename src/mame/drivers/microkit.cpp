// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    RCA COSMAC Microkit

    http://www.vintagecomputer.net/browse_thread.cfm?id=511

    Press CR or LF to get the * prompt.
    Commands:
    $Pxxxx - Jump to address xxxx
    ?Mxxxx yyyy - Dump memory starting at xxxx for yyyy bytes
    !Mxxxx yy zz... - Write data (yy etc) to memory xxxx. Data gets entered when you
                      press the space after the data.

    There's no sound or storage facilities, therefore no software.

    ToDo:
    - No technical manual or schematic available, so the entire driver is bodgy guesswork.
    - Address 0 needs to be read/writeable, otherwise the numbers you enter will get
            internally corrupted.
    - Address 8000 is IDL which hangs the system, so program counter is preset to 8001.
    - The keyboard is hooked up serially, which is ok, but the output to the terminal
            is rubbish, so parallel is used so you can at least see something.
    - When you enter commands, you can't see what you're doing.
    - When you enter numbers, they display as rubbish or act as control codes. They
            internally work though.
    - The computer looks like a rack-mount metal box with a rudimentary front panel.
            Buttons are: Reset; Load; Run program; Run Utility
            There is a RUN LED.
            None of these items are emulated.
            It also has a power switch and lamp, and a fuse.

*****************************************************************************************/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "bus/rs232/rs232.h"
#include "machine/terminal.h"


class microkit_state : public driver_device
{
public:
	microkit_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rs232(*this, "rs232")
		, m_terminal(*this, "terminal")
	{ }

	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(ram_r);

private:
	virtual void machine_reset() override;
	UINT8 m_resetcnt;
	UINT8 m_ram_data;
	required_device<cosmac_device> m_maincpu;
	required_device<rs232_port_device> m_rs232;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START( microkit_mem, AS_PROGRAM, 8, microkit_state )
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(ram_r,ram_w)
	AM_RANGE(0x8000, 0x81ff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8200, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( microkit_io, AS_IO, 8, microkit_state )
	AM_RANGE(0x07, 0x07) AM_WRITENOP // writes a lots of zeros here
ADDRESS_MAP_END

static INPUT_PORTS_START( microkit )
INPUT_PORTS_END

READ_LINE_MEMBER( microkit_state::clear_r )
{
	if (m_resetcnt < 0x10)
		m_maincpu->set_state_int(COSMAC_R0, 0x8001); // skip IDL
	if (m_resetcnt < 0x20)
		m_resetcnt++;
	// set reset pin to normal
	return 1;
}

READ8_MEMBER( microkit_state::ram_r )
{
	return m_ram_data;
}

WRITE8_MEMBER( microkit_state::ram_w )
{
	m_ram_data = data;
	if (data > 0 && data < 0x80)
		m_terminal->write(space, 0, data);
}

void microkit_state::machine_reset()
{
	m_resetcnt = 0;
	m_ram_data = 0;
}

static DEVICE_INPUT_DEFAULTS_START( serial_keyb )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END


static MACHINE_CONFIG_START( microkit, microkit_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", CDP1802, 1750000)
	MCFG_CPU_PROGRAM_MAP(microkit_mem)
	MCFG_CPU_IO_MAP(microkit_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(microkit_state, clear_r))

	/* video hardware */
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("maincpu", cosmac_device, ef4_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("keyboard", serial_keyb)
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
MACHINE_CONFIG_END

ROM_START( microkit )
	ROM_REGION( 0x200, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "3.2b", 0x000, 0x100, CRC(6799357e) SHA1(c46e3322b8b1b6534a7da04806be29fa265951b7) )
	ROM_LOAD( "4.2a", 0x100, 0x100, CRC(27267bad) SHA1(838df9be2dc175584a1a6ee1770039118e49482e) )
ROM_END

COMP( 1975, microkit,    0,      0,      microkit,        microkit, driver_device, 0,      "RCA",  "COSMAC Microkit",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
