// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Morrow Tricep

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class tricep_state : public driver_device
{
public:
	tricep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	,
		m_p_ram(*this, "p_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ16_MEMBER(tricep_terminal_r);
	DECLARE_WRITE16_MEMBER(tricep_terminal_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	required_shared_ptr<UINT16> m_p_ram;
	virtual void machine_reset();
};



READ16_MEMBER( tricep_state::tricep_terminal_r )
{
	return 0xffff;
}

WRITE16_MEMBER( tricep_state::tricep_terminal_w )
{
	m_terminal->write(space, 0, data >> 8);
}

static ADDRESS_MAP_START(tricep_mem, AS_PROGRAM, 16, tricep_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("p_ram")
	AM_RANGE(0x00fd0000, 0x00fd1fff) AM_ROM AM_REGION("user1",0)
	AM_RANGE(0x00ff0028, 0x00ff0029) AM_READWRITE(tricep_terminal_r,tricep_terminal_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( tricep )
INPUT_PORTS_END


void tricep_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();

	memcpy((UINT8*)m_p_ram.target(),user1,0x2000);

	m_maincpu->reset();
}

WRITE8_MEMBER( tricep_state::kbd_put )
{
}

static MACHINE_CONFIG_START( tricep, tricep_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(tricep_mem)


	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(tricep_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tricep )
	ROM_REGION( 0x2000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tri2.4_odd.u37",  0x0000, 0x1000, CRC(31eb2dcf) SHA1(2d9df9262ee1096d0398505e10d209201ac49a5d))
	ROM_LOAD16_BYTE( "tri2.4_even.u36", 0x0001, 0x1000, CRC(4414dcdc) SHA1(00a3d293617dc691748ae85b6ccdd6723daefc0a))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY       FULLNAME       FLAGS */
COMP( 1985, tricep, 0,      0,       tricep,    tricep, driver_device,  0,  "Morrow Designs", "Tricep", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
