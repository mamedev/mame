// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Vector 4

        08/12/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class vector4_state : public driver_device
{
public:
	vector4_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(vector4_02_r);
	DECLARE_READ8_MEMBER(vector4_03_r);
	DECLARE_WRITE8_MEMBER(vector4_02_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	virtual void machine_reset() override;
};



WRITE8_MEMBER( vector4_state::vector4_02_w )
{
	if (data < 0xff)
		m_terminal->write(space, 0, data);
}

READ8_MEMBER( vector4_state::vector4_03_r )
{
	return (m_term_data) ? 3 : 1;
}

READ8_MEMBER( vector4_state::vector4_02_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

static ADDRESS_MAP_START(vector4_mem, AS_PROGRAM, 8, vector4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf8ff) AM_ROM
	AM_RANGE(0xfc00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(vector4_io, AS_IO, 8, vector4_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READWRITE(vector4_02_r,vector4_02_w)
	AM_RANGE(0x03, 0x03) AM_READ(vector4_03_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vector4 )
INPUT_PORTS_END


void vector4_state::machine_reset()
{
	m_term_data = 0;
	m_maincpu->set_state_int(Z80_PC, 0xe000);
}

WRITE8_MEMBER( vector4_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( vector4, vector4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(vector4_mem)
	MCFG_CPU_IO_MAP(vector4_io)


	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(vector4_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vector4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS( 0, "v4c", "ver 4.0c" )
	ROMX_LOAD( "vg40cl_ihl.bin", 0xe000, 0x0400, CRC(dcaf79e6) SHA1(63619ddb12ff51e5862902fb1b33a6630f555ad7), ROM_BIOS(1))
	ROMX_LOAD( "vg40ch_ihl.bin", 0xe400, 0x0400, CRC(3ff97d70) SHA1(b401e49aa97ac106c2fd5ee72d89e683ebe34e34), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "v43", "ver 4.3" )
	ROMX_LOAD( "vg-em-43.bin",   0xe000, 0x1000, CRC(29a0fcee) SHA1(ca44de527f525b72f78b1c084c39aa6ce21731b5), ROM_BIOS(2))

	ROM_SYSTEM_BIOS( 2, "v5", "ver 5.0" )
	ROMX_LOAD( "vg-zcb50.bin",   0xe000, 0x1000, CRC(22d692ce) SHA1(cbb21b0acc98983bf5febd59ff67615d71596e36), ROM_BIOS(3))
	ROM_LOAD( "mfdc.bin", 0xf800, 0x0100, CRC(d82a40d6) SHA1(cd1ef5fb0312cd1640e0853d2442d7d858bc3e3b))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    INIT        COMPANY      FULLNAME       FLAGS */
COMP( 19??, vector4, 0,      0,       vector4,   vector4, driver_device, 0,   "Vector Graphics", "Vector 4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
