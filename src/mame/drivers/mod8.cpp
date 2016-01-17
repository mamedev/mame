// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Microsystems International Limited MOD-8

M.I.L. was formed in 1968 from a joint venture between the Canadian
Government and Northern Telecom. It produced a variety of computer
chips, eproms, etc, plus parts for the telephone company. It folded
in 1975.
(Info from http://www.cse.yorku.ca/museum/v_tour/artifacts/artifacts.htm)


    2009-11-18 Skeleton driver.
    2009-12-02 Working driver [Miodrag Milanovic]
    2011-06-14 Modernised & above notes added.

Commands:
All commands consist of 3 uppercase letters. If further info is required
then a * prompt is printed on a new line, where you will enter the data.
All numbers are OCTAL (3/6 digits with leading zeros). Since a teletypewriter
is being used, there is no cursor. Do NOT press Enter except after these
commands, otherwise things get confusing.

LOC - set current location pointer (the CLP)
DLP - display CLP
DPS - dump symbolic
LDO - load octal
DPO - dump octal
LBF - load BNPF format
DBF - dump BNPF format
EDT - enter Edit Mode
XQT - initiate program execution
CPY - copy routine
TRN - translate routine
SBP - set breakpoint
CBP - clear breakpoint
PRG - program PROM

Pressing Ctrl-A will escape back to the monitor. You will see 8 dashes.

Commands in the Edit Mode:
When you enter the Edit Mode it displays the CLP followed by a slash.

nnn - enter a new value into this memory location and increment the CLP
` (tic) - decrement CLP
@ - same as XQT
R - return to monitor
*nnnnnn - change CLP to this value
space - display current contents of memory

While in 'space' mode, press a letter to increment CLP, or shift-delete
(underscore character) followed by a new byte for this location.

****************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"
#include "machine/teleprinter.h"

class mod8_state : public driver_device
{
public:
	mod8_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_teleprinter(*this, TELEPRINTER_TAG)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE8_MEMBER(out_w);
	DECLARE_WRITE8_MEMBER(tty_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(tty_r);
	IRQ_CALLBACK_MEMBER(mod8_irq_callback);
private:
	UINT16 m_tty_data;
	UINT8 m_tty_key_data;
	int m_tty_cnt;
	virtual void machine_reset() override;
	required_device<teleprinter_device> m_teleprinter;
	required_device<cpu_device> m_maincpu;
};

WRITE8_MEMBER( mod8_state::out_w )
{
	m_tty_data >>= 1;
	m_tty_data |= BIT(data, 0) ? 0x8000 : 0;
	m_tty_cnt++;

	if (m_tty_cnt == 10)
	{
		m_teleprinter->write(space, 0, (m_tty_data >> 7) & 0x7f);
		m_tty_cnt = 0;
	}
}

WRITE8_MEMBER( mod8_state::tty_w )
{
	m_tty_data = 0;
	m_tty_cnt = 0;
}

READ8_MEMBER( mod8_state::tty_r )
{
	UINT8 d = m_tty_key_data & 1;
	m_tty_key_data >>= 1;
	return d;
}

static ADDRESS_MAP_START(mod8_mem, AS_PROGRAM, 8, mod8_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000,0x6ff) AM_ROM
	AM_RANGE(0x700,0xfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mod8_io, AS_IO, 8, mod8_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ(tty_r)
	AM_RANGE(0x0a,0x0a) AM_WRITE(out_w)
	AM_RANGE(0x0b,0x0b) AM_WRITE(tty_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mod8 )
INPUT_PORTS_END

IRQ_CALLBACK_MEMBER(mod8_state::mod8_irq_callback)
{
	return 0xC0; // LAA - NOP equivalent
}

void mod8_state::machine_reset()
{
}

WRITE8_MEMBER( mod8_state::kbd_put )
{
	m_tty_key_data = data ^ 0xff;
	m_maincpu->set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( mod8, mod8_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8008, 800000)
	MCFG_CPU_PROGRAM_MAP(mod8_mem)
	MCFG_CPU_IO_MAP(mod8_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(mod8_state,mod8_irq_callback)

	/* video hardware */
	MCFG_DEVICE_ADD(TELEPRINTER_TAG, TELEPRINTER, 0)
	MCFG_GENERIC_TELEPRINTER_KEYBOARD_CB(WRITE8(mod8_state, kbd_put))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( mod8 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon8.001", 0x0000, 0x0100, CRC(b82ac6b8) SHA1(fbea5a6dd4c779ca1671d84089f857a3f548ffcb))
	ROM_LOAD( "mon8.002", 0x0100, 0x0100, CRC(8b82bc3c) SHA1(66222511527b27e56a5a1f9656d424d407eac7d3))
	ROM_LOAD( "mon8.003", 0x0200, 0x0100, CRC(679ae913) SHA1(22423efcb9051c9812fcbac9a27af70415d0dd81))
	ROM_LOAD( "mon8.004", 0x0300, 0x0100, CRC(2a4e580f) SHA1(8b0cb9660fde3cacd299faaa31724e4f3262d77f))
	ROM_LOAD( "mon8.005", 0x0400, 0x0100, CRC(e281bb1a) SHA1(cc7c2746e075512dbf5eed88ae3aea009558dbd0))
	ROM_LOAD( "mon8.006", 0x0500, 0x0100, CRC(b7e2f585) SHA1(5408adabc3df6e6ea8dcfb2327b2883b435ab85e))
	ROM_LOAD( "mon8.007", 0x0600, 0x0100, CRC(49a5c626) SHA1(66c1865db9151818d3b20ec3c68dd793cb98a221))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS           INIT   COMPANY                       FULLNAME       FLAGS */
COMP( 1974, mod8,   0,       0,      mod8,      mod8,   driver_device,   0, "Microsystems International Ltd", "MOD-8", MACHINE_NO_SOUND_HW)
