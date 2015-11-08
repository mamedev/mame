// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        MCS BASIC 52 and MCS BASIC 31 board

        03/12/2009 Skeleton driver.

        2012-08-08 Made to work [Robbbert]

BASIC-52 is an official Intel release.

BASIC-31 (and variants) as found on the below url, are homebrews.

http://dsaprojects.110mb.com/electronics/8031-ah/8031-bas.html


The driver is working, however there are issues with the cpu serial code.
When started, you are supposed to press Space and the system works out
the baud rate and boots up.

However, the way the cpu is written, it actually passes bytes around, so
the auto-speed detection doesn't work as intended. Also the cpu interface
is horribly outdated and needs to be brought up to date.

So, as it stands, start the driver, then press d and g in turn until
something starts happening. Basic-52 usually starts at a very slow rate,
about 1 character per second, while Basic-31 is much faster.

Once the system starts, all input must be in uppercase. Read the manual
to discover the special features of this Basic.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i8255.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class basic52_state : public driver_device
{
public:
	basic52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(unk_r);
	UINT8 m_term_data;
	required_device<mcs51_cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	virtual void machine_reset();
	DECLARE_WRITE8_MEMBER(to_term);
	DECLARE_READ8_MEMBER(from_term);
};


static ADDRESS_MAP_START(basic52_mem, AS_PROGRAM, 8, basic52_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x7fff) AM_RAM
	//AM_RANGE(0x8000, 0x9fff) AM_ROM // EPROM
	//AM_RANGE(0xc000, 0xdfff) // Expansion block
	//AM_RANGE(0xe000, 0xffff) // Expansion block
ADDRESS_MAP_END

static ADDRESS_MAP_START(basic52_io, AS_IO, 8, basic52_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x9fff) AM_ROM // EPROM
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)  // PPI-8255
	//AM_RANGE(0xc000, 0xdfff) // Expansion block
	//AM_RANGE(0xe000, 0xffff) // Expansion block
	AM_RANGE(0x20003, 0x20003) AM_READ(unk_r);
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( basic52 )
INPUT_PORTS_END

// won't compile unless these are static
WRITE8_MEMBER( basic52_state::to_term)
{
	m_terminal->write(space, 0, data);
}

READ8_MEMBER(basic52_state::from_term)
{
	return m_term_data;
}

READ8_MEMBER( basic52_state::unk_r)
{
	return m_term_data; // won't boot without this
}

void basic52_state::machine_reset()
{
	m_maincpu->i8051_set_serial_tx_callback(write8_delegate(FUNC(basic52_state::to_term),this));
	m_maincpu->i8051_set_serial_rx_callback(read8_delegate(FUNC(basic52_state::from_term),this));
}

WRITE8_MEMBER( basic52_state::kbd_put )
{
	m_maincpu->set_input_line(MCS51_RX_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(MCS51_RX_LINE, CLEAR_LINE);
	m_term_data = data;
}

static MACHINE_CONFIG_START( basic31, basic52_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8031, XTAL_11_0592MHz)
	MCFG_CPU_PROGRAM_MAP(basic52_mem)
	MCFG_CPU_IO_MAP(basic52_io)


	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(basic52_state, kbd_put))

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( basic52, basic31 )
	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", I8052, XTAL_11_0592MHz)
	MCFG_CPU_PROGRAM_MAP(basic52_mem)
	MCFG_CPU_IO_MAP(basic52_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( basic52 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v11", "v 1.1")
	ROMX_LOAD( "mcs-51-11.bin",  0x0000, 0x2000, CRC(4157b22b) SHA1(bd9e6869b400cc1c9b163243be7bdcf16ce72789), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v11b", "v 1.1b")
	ROMX_LOAD( "mcs-51-11b.bin", 0x0000, 0x2000, CRC(a60383cc) SHA1(9515cc435e2ca3d3adb19631c03a62dfbeab0826), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v131", "v 1.3.1")
	ROMX_LOAD( "mcs-51-131.bin", 0x0000, 0x2000, CRC(6a493162) SHA1(ed1079a6b4d4dbf448e15238c5a9e4dd004e401c), ROM_BIOS(3))
ROM_END

ROM_START( basic31 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v12", "v 1.2")
	ROMX_LOAD( "mcs-51-12.bin",  0x0000, 0x2000, CRC(ee667c7c) SHA1(e69b32e69ecda2012c7113649634a3a64e984bed), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v12a", "v 1.2a")
	ROMX_LOAD( "mcs-51-12a.bin", 0x0000, 0x2000, CRC(225bb2f0) SHA1(46e97643a7a5cb4c278f9e3c73d18cd93209f8bf), ROM_BIOS(2))
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1985, basic52,  0,       0,    basic52,   basic52, driver_device,  0,    "Intel", "MCS BASIC 52", MACHINE_NO_SOUND_HW)
COMP( 1985, basic31,  basic52, 0,    basic31,   basic52, driver_device,  0,    "Intel", "MCS BASIC 31", MACHINE_NO_SOUND_HW)
