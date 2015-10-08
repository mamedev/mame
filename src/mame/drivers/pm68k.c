// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Callan PM68K Unix mainframe.

2013-09-04 Skeleton driver

Status: Boots into monitor, some commands work, some freeze.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class pm68k_state : public driver_device
{
public:
	pm68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_base(*this, "rambase"),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ16_MEMBER(keyin_r);
	DECLARE_READ16_MEMBER(status_r);
private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_shared_ptr<UINT16> m_p_base;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

READ16_MEMBER( pm68k_state::keyin_r )
{
	UINT16 ret = m_term_data;
	m_term_data = 0;
	return ret << 8;
}

READ16_MEMBER( pm68k_state::status_r )
{
	return (m_term_data) ? 0x500 : 0x400;
}


static ADDRESS_MAP_START(pm68k_mem, AS_PROGRAM, 16, pm68k_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x1fffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x200000, 0x205fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x600000, 0x600001) AM_READ(keyin_r) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0xff00)
	AM_RANGE(0x600002, 0x600003) AM_READ(status_r)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( pm68k )
INPUT_PORTS_END


void pm68k_state::machine_reset()
{
	UINT8* ROM = memregion("maincpu")->base();
	memcpy(m_p_base, ROM, 8);
	m_maincpu->reset();
}

WRITE8_MEMBER( pm68k_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( pm68k, pm68k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(pm68k_mem)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(pm68k_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pm68k )
	ROM_REGION16_BE(0x6000, "maincpu", 0)
	ROM_LOAD16_BYTE("u103", 0x00000, 0x1000, CRC(86d32d6c) SHA1(ce9c54b62c64c37ae9106fb06b8a2b2152d1ddf6) )
	ROM_LOAD16_BYTE("u101", 0x00001, 0x1000, CRC(66607e54) SHA1(06f380fdeba13dc3aee826dd166f4bd3031febb9) )
	ROM_LOAD16_BYTE("u104", 0x02000, 0x2000, CRC(ccd2ba4d) SHA1(5cdcf875e136aa9af5f150e0102cd209c496885e) )
	ROM_LOAD16_BYTE("u102", 0x02001, 0x2000, CRC(48182abd) SHA1(a6e4fb62c5f04cb397c6c3294723ec1f7bc3b680) )
ROM_END


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE INPUT   CLASS         INIT    COMPANY           FULLNAME       FLAGS */
COMP( 198?, pm68k, 0,      0,      pm68k,  pm68k,  driver_device, 0, "Callan Data Systems", "PM68K", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
