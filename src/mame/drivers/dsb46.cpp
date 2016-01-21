// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/********************************************************************************************************************

2013-07-31 Skeleton Driver [Curt Coder]
2013-07-31 Connected to terminal [Robbbert]


The photos show 3 boards:
- A scsi board (all 74-series TTL)
- CPU board (64k dynamic RAM, Z80A CPU, 2x Z80CTC, 2x Z80SIO/0, MB8877A, Z80DMA, 4x MC1488,
  4x MC1489, XTALS 1.8432MHz and 24MHz)
- ADES board (Adaptec Inc AIC-100, AIC-250, AIC-300, Intel D8086AH, unknown crystal)

Both roms contain Z80 code.


********************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class dsb46_state : public driver_device
{
public:
	dsb46_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE8_MEMBER(port1a_w);
	DECLARE_DRIVER_INIT(dsb46);
	DECLARE_MACHINE_RESET(dsb46);
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START( dsb46_mem, AS_PROGRAM, 8, dsb46_state )
	AM_RANGE(0x0000, 0x07ff) AM_READ_BANK("read") AM_WRITE_BANK("write")
	AM_RANGE(0x0800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsb46_io, AS_IO, 8, dsb46_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(port00_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x01, 0x01) AM_READ(port01_r)
	AM_RANGE(0x1a, 0x1a) AM_WRITE(port1a_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( dsb46 )
INPUT_PORTS_END

DRIVER_INIT_MEMBER(dsb46_state, dsb46)
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("read")->configure_entry(0, &RAM[0x10000]);
	membank("read")->configure_entry(1, &RAM[0x00000]);
	membank("write")->configure_entry(0, &RAM[0x00000]);
}

MACHINE_RESET_MEMBER( dsb46_state,dsb46 )
{
	membank("read")->set_entry(0);
	membank("write")->set_entry(0);
	m_term_data = 0;
	m_maincpu->reset();
}

WRITE8_MEMBER( dsb46_state::port1a_w )
{
	membank("read")->set_entry(data & 1);
}

READ8_MEMBER( dsb46_state::port01_r )
{
	return (m_term_data) ? 5 : 4;
}

READ8_MEMBER( dsb46_state::port00_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( dsb46_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( dsb46, dsb46_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(dsb46_mem)
	MCFG_CPU_IO_MAP(dsb46_io)
	MCFG_MACHINE_RESET_OVERRIDE(dsb46_state, dsb46)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(dsb46_state, kbd_put))
MACHINE_CONFIG_END

ROM_START( dsb46 )
	ROM_REGION( 0x10800, "maincpu", 0 )
	ROM_LOAD( "1538a.bin", 0x10000, 0x800, CRC(65b3e26e) SHA1(afe1f03f266b7d13fdb1f1bc6762df5e0aa5c764) )

	ROM_REGION( 0x4000, "ades", 0 )
	ROM_LOAD( "ades.bin", 0x0000, 0x4000, CRC(d374abf0) SHA1(331f51a2bb81375aeffbe63c1ebc1d7cd779b9c3) )
ROM_END

COMP( 198?, dsb46, 0, 0, dsb46, dsb46, dsb46_state, dsb46, "Davidge", "DSB-4/6",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
