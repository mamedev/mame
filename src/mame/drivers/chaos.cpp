// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

    Chaos2

    08/04/2010 Skeleton driver.
    19/05/2012 Connected to a terminal, system is usable [Robbbert]

    This is a homebrew system: http://koo.corpus.cam.ac.uk/chaos/

    There are no schematics or manuals, so the results might not be
    totally accurate.

    With the DOS config switch turned off, the only accepted input
    is a line starting with '&'. The actual commands are unknown.

    With DOS enabled, a large number of commands become available.
    These are:
    access, ask, ascdis, bpclr, bpset, close, control, copy, devfive, dir,
    end, exec, execute, fill, find, goto, if, input, let, list, load, lowercase,
    memdis, memset, open, port, read, reboot, runhex, run, save, type, typesl,
    verify.
    An example is: memdis 0 f
    Don't try 'fill' - it fills all memory with zeroes, crashing the system.

    ToDo:
    - Connect up floppy disk (no info available on this)

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class chaos_state : public driver_device
{
public:
	chaos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_terminal(*this, TERMINAL_TAG),
		m_p_ram(*this, "p_ram") ,
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ8_MEMBER(port1e_r);
	DECLARE_WRITE8_MEMBER(port1f_w);
	DECLARE_READ8_MEMBER(port90_r);
	DECLARE_READ8_MEMBER(port91_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	virtual void machine_reset() override;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<UINT8> m_p_ram;
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( chaos_mem, AS_PROGRAM, 8, chaos_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_SHARE("p_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( chaos_io, AS_IO, 8, chaos_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1e, 0x1e) AM_READ(port1e_r)
	AM_RANGE(0x1f, 0x1f) AM_READWRITE(port90_r, port1f_w)
	AM_RANGE(0x90, 0x90) AM_READ(port90_r)
	AM_RANGE(0x91, 0x91) AM_READ(port91_r)
	AM_RANGE(0x92, 0x92) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x101, 0x103) AM_NOP // stops error log filling up while using debug
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( chaos )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Enable DOS")
	PORT_CONFSETTING(    0x01, DEF_STR(No))
	PORT_CONFSETTING(    0x00, DEF_STR(Yes))
INPUT_PORTS_END


// Port 1E - Bit 0 indicates key pressed, Bit 1 indicates ok to output

READ8_MEMBER( chaos_state::port1e_r )
{
	return (m_term_data) ? 1 : 0;
}

WRITE8_MEMBER( chaos_state::port1f_w )
{
	// make the output readable on our terminal
	if (data == 0x09)
		return;
	else
	if (!data)
		data = 0x24;

	m_terminal->write(space, 0, data);

	if (data == 0x0d)
		m_terminal->write(space, 0, 0x0a);
}

READ8_MEMBER( chaos_state::port90_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

// Status port
// Bit 0 = L use ports 1E & 1F; H use ports 90 & 92
// Bit 3 = key pressed
// Bit 7 = ok to output

READ8_MEMBER( chaos_state::port91_r )
{
	UINT8 ret = 0x80 | ioport("CONFIG")->read();
	ret |= (m_term_data) ? 8 : 0;
	return ret;
}

WRITE8_MEMBER( chaos_state::kbd_put )
{
	m_term_data = data;
}

void chaos_state::machine_reset()
{
	// copy the roms into ram
	UINT8* ROM = memregion("maincpu")->base();
	memcpy(m_p_ram, ROM, 0x3000);
	memcpy(m_p_ram+0x7000, ROM+0x7000, 0x1000);
}

static MACHINE_CONFIG_START( chaos, chaos_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(chaos_mem)
	MCFG_CPU_IO_MAP(chaos_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(chaos_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( chaos )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "chaos.001", 0x0000, 0x1000, CRC(3b433e72) SHA1(5b487337d71253d0e64e123f405da9eaf20e87ac))
	ROM_LOAD( "chaos.002", 0x1000, 0x1000, CRC(8b0b487f) SHA1(0d167cf3004a81c87446f2f1464e3debfa7284fe))
	ROM_LOAD( "chaos.003", 0x2000, 0x1000, CRC(5880db81) SHA1(29b8f1b03c83953f66464ad1fbbfe2e019637ce1))
	ROM_LOAD( "chaos.004", 0x7000, 0x1000, CRC(5d6839d6) SHA1(237f52f0780ac2e29d57bf06d0f7a982eb523084))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY     FULLNAME       FLAGS */
COMP( 1983, chaos,  0,      0,       chaos,     chaos, driver_device,   0,     "<unknown>",  "Chaos 2", MACHINE_NO_SOUND_HW )
