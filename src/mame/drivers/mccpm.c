// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        mc-CP/M-Computer

        31/08/2010 Skeleton driver.
        18/11/2010 Connected to a terminal
        28/09/2011 Added more bioses

Some Monitor commands (varies between versions):

B - boot a floppy (^N to regain control)
E - prints a number
I - Select boot drive/set parameters
K,O - display version header
N - newline
Z - print 'EFFF'

URL for v3.4: http://www.hanshehl.de/mc-prog.htm (German language)

I/O ports (my guess)
30 - fdc (1st drive)
40 - fdc (2nd drive)
F0 - terminal in/out
F1 - terminal status

SIO? - F0 to F3
PIO A-Data 0F4h, A-Command 0F5h, B-Data 0F6h, B-Command 0F7h

'maincpu' (F59C): unmapped i/o memory write to 00F1 = 01 & FF
'maincpu' (F59C): unmapped i/o memory write to 00F1 = 00 & FF
'maincpu' (F59C): unmapped i/o memory write to 00F1 = 03 & FF
'maincpu' (F59C): unmapped i/o memory write to 00F1 = E1 & FF
'maincpu' (F59C): unmapped i/o memory write to 00F1 = 04 & FF
'maincpu' (F59C): unmapped i/o memory write to 00F1 = 4C & FF
'maincpu' (F59C): unmapped i/o memory write to 00F1 = 05 & FF
'maincpu' (F59C): unmapped i/o memory write to 00F1 = EA & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = 01 & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = 00 & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = 03 & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = E1 & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = 04 & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = 4C & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = 05 & FF
'maincpu' (F5A5): unmapped i/o memory write to 00F3 = EA & FF
'maincpu' (F5A9): unmapped i/o memory write to 00F5 = CF & FF
'maincpu' (F5AD): unmapped i/o memory write to 00F5 = 7F & FF
'maincpu' (F5B1): unmapped i/o memory write to 00F7 = CF & FF
'maincpu' (F5B4): unmapped i/o memory write to 00F7 = 00 & FF
'maincpu' (F149): unmapped i/o memory write to 0040 = D0 & FF
'maincpu' (F14B): unmapped i/o memory write to 0030 = D0 & FF

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class mccpm_state : public driver_device
{
public:
	mccpm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_p_ram(*this, "p_ram")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(mccpm_f0_r);
	DECLARE_READ8_MEMBER(mccpm_f1_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	required_shared_ptr<UINT8> m_p_ram;
	UINT8 m_term_data;
	virtual void machine_reset();
};



READ8_MEMBER( mccpm_state::mccpm_f0_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

// bit 0 - key pressed
// bit 2 - ready to send to terminal
READ8_MEMBER( mccpm_state::mccpm_f1_r )
{
	return (m_term_data) ? 5 : 4;
}

static ADDRESS_MAP_START(mccpm_mem, AS_PROGRAM, 8, mccpm_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("p_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mccpm_io, AS_IO, 8, mccpm_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_READ(mccpm_f0_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0xf1, 0xf1) AM_READ(mccpm_f1_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mccpm )
INPUT_PORTS_END


void mccpm_state::machine_reset()
{
	UINT8* bios = memregion("maincpu")->base();
	memcpy(m_p_ram, bios, 0x1000);
}

WRITE8_MEMBER( mccpm_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( mccpm, mccpm_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(mccpm_mem)
	MCFG_CPU_IO_MAP(mccpm_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(mccpm_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mccpm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36", "V3.6")
	ROMX_LOAD( "mon36.j15",   0x0000, 0x1000, CRC(9c441537) SHA1(f95bad52d9392b8fc9d9b8779b7b861672a0022b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v34", "V3.4")
	ROMX_LOAD( "monhemc.bin", 0x0000, 0x1000, CRC(cae7b56e) SHA1(1f40be9491a595e6705099a452743cc0d49bfce8), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v34a", "V3.4 (alt)")
	ROMX_LOAD( "mc01mon.bin", 0x0000, 0x0d00, CRC(d1c89043) SHA1(f52a0ed3793dde0de74596be7339233b6a1770af), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                          FULLNAME       FLAGS */
COMP( 1981, mccpm,  0,      0,       mccpm,     mccpm, driver_device,    0, "GRAF Elektronik Systeme GmbH", "mc-CP/M-Computer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
