// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Contel Codata Corporation Codata

        2010-01-11 Skeleton driver.
        2013-08-26 Connected to a terminal.

        Chips: uPD7201C, AM9513, SCN68000. Crystal: 16 MHz

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class codata_state : public driver_device
{
public:
	codata_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_base(*this, "rambase"),
		m_terminal(*this, TERMINAL_TAG),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ16_MEMBER(keyin_r);
	DECLARE_READ16_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_shared_ptr<UINT16> m_p_base;
	required_device<generic_terminal_device> m_terminal;
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(codata_mem, AS_PROGRAM, 16, codata_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x0fffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x200000, 0x203fff) AM_ROM AM_REGION("user1", 0);
	AM_RANGE(0x400000, 0x403fff) AM_ROM AM_REGION("user1", 0x4000);
	AM_RANGE(0x600000, 0x600001) AM_READ(keyin_r) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0xff00)
	AM_RANGE(0x600002, 0x600003) AM_READ(status_r)
	//AM_RANGE(0x600000, 0x600003) uPD7201 SIO
	//AM_RANGE(0x800000, 0x800003) AM9513 5 channel timer
	//AM_RANGE(0xa00000, 0xbfffff) page map (rw)
	//AM_RANGE(0xc00000, 0xdfffff) segment map (rw), context register (r)
	//AM_RANGE(0xe00000, 0xffffff) context register (w), 16-bit parallel input port (r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( codata )
INPUT_PORTS_END


READ16_MEMBER( codata_state::keyin_r )
{
	UINT16 ret = m_term_data;
	m_term_data = 0;
	return ret << 8;
}

READ16_MEMBER( codata_state::status_r )
{
	return (m_term_data) ? 0x500 : 0x400;
}

WRITE8_MEMBER( codata_state::kbd_put )
{
	m_term_data = data;
}

void codata_state::machine_reset()
{
	UINT8* RAM = memregion("user1")->base();
	memcpy(m_p_base, RAM, 16);
	m_term_data = 0;
	m_maincpu->reset();
}

static MACHINE_CONFIG_START( codata, codata_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(codata_mem)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(codata_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( codata )
	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "27-0042-01a boot 00 u101 rev 3.6.2 9-28-83.u101", 0x0000, 0x2000, CRC(70014b16) SHA1(19a82000894d79817358d40ae520200e976be310))
	ROM_LOAD16_BYTE( "27-0043-01a boot 01 u102 rev 3.6.2 9-28-83.u102", 0x4000, 0x2000, CRC(fca9c314) SHA1(2f8970fad479000f28536003867066d6df9e33d9))
	ROM_LOAD16_BYTE( "27-0044-01a boot e0 u103 rev 3.6.2 9-28-83.u103", 0x0001, 0x2000, CRC(dc5d5cea) SHA1(b3e9248abf89d674c463d21d2f7be34508cf16c2))
	ROM_LOAD16_BYTE( "27-0045-01a boot e1 u104 rev 3.6.2 9-28-83.u104", 0x4001, 0x2000, CRC(a937e7b3) SHA1(d809bbd437fe7d925325958072b9e0dc33dd36a6))

	ROM_REGION( 0x240, "proms", 0 )
	ROM_LOAD( "p0.u502", 0x0000, 0x0020, CRC(20eb1183) SHA1(9b268792b28d858d6b6a1b6c4148af88a8d6b735) )
	ROM_LOAD( "p1.u602", 0x0020, 0x0020, CRC(ee1e5a14) SHA1(0d3346cb3b647fa2475bd7b4fa36ea6ecfdaf805) )
	ROM_LOAD( "p2.u503", 0x0040, 0x0200, CRC(12d9a6be) SHA1(fca99f9c5afc630ac67cbd4e5ba4e5242b826848) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1982, codata,  0,     0,       codata,    codata, driver_device,   0,   "Contel Codata Corporation", "Codata", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
