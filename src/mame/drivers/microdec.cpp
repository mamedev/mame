// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Morrow Designs Micro Decision

        2009-12-10 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class microdec_state : public driver_device
{
public:
	microdec_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_terminal(*this, TERMINAL_TAG),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<generic_terminal_device> m_terminal;
	required_device<cpu_device> m_maincpu;
};


READ8_MEMBER( microdec_state::status_r )
{
	return (m_term_data) ? 3 : 1;
}

READ8_MEMBER( microdec_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

static ADDRESS_MAP_START(microdec_mem, AS_PROGRAM, 8, microdec_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x1000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(microdec_io, AS_IO, 8, microdec_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xfa, 0xfb) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0xfc, 0xfc) AM_READ(keyin_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0xfd, 0xfd) AM_READ(status_r)
	// AM_RANGE(0xf0, 0xf3) 8253 PIT (md3 only) used as a baud rate generator for serial ports
	// AM_RANGE(0xf4, 0xf4) Centronics data
	// AM_RANGE(0xf5, 0xf5) motor check (md1/2)
	// AM_RANGE(0xf5, 0xf5) Centronics status (md3) read bit 3 (ack=1); read bit 4 (busy=1); write bit 7 (stb=0)
	// AM_RANGE(0xf6, 0xf6) rom enable (w=enable; r=disable)
	// AM_RANGE(0xf7, 0xf7) VFO Count set
	// AM_RANGE(0xf8, 0xf8) Motor and SHift control
	// AM_RANGE(0xfa, 0xfb) uPD765C fdc FA=status; FB=data
	// AM_RANGE(0xfc, 0xfd) Serial Port 1 FC=data FD=status
	// AM_RANGE(0xfe, 0xff) Serial Port 2 FE=data FF=status
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( microdec )
INPUT_PORTS_END

void microdec_state::machine_start()
{
}

void microdec_state::machine_reset()
{
	m_term_data = 0;
}

WRITE8_MEMBER( microdec_state::kbd_put )
{
	m_term_data = data;
}

static SLOT_INTERFACE_START( microdec_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( microdec, microdec_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(microdec_mem)
	MCFG_CPU_IO_MAP(microdec_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(microdec_state, kbd_put))

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", microdec_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", microdec_floppies, "525hd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( md2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v13", "v1.3" )
	ROMX_LOAD( "md2-13.bin",  0x0000, 0x0800, CRC(43f4c9ab) SHA1(48a35cbee4f341310e9cba5178c3fd6e74ef9748), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v13a", "v1.3a" )
	ROMX_LOAD( "md2-13a.bin", 0x0000, 0x0800, CRC(d7fcddfd) SHA1(cae29232b737ebb36a27b8ad17bc69e9968f1309), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "v13b", "v1.3b" )
	ROMX_LOAD( "md2-13b.bin", 0x0000, 0x1000, CRC(a8b96835) SHA1(c6b111939aa7e725da507da1915604656540b24e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "v20", "v2.0" )
	ROMX_LOAD( "md2-20.bin",  0x0000, 0x1000, CRC(a604735c) SHA1(db6e6e82a803f5cbf4f628f5778a93ae3e211fe1), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "v23", "v2.3" )
	ROMX_LOAD( "md2-23.bin",  0x0000, 0x1000, CRC(49bae273) SHA1(00381a226fe250aa3636b0b740df0af63efb0d18), ROM_BIOS(5))
ROM_END

ROM_START( md3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v23a", "v2.3a" )
	ROMX_LOAD( "md3-23a.bin", 0x0000, 0x1000, CRC(95d59980) SHA1(ae65a8e8e2823cf4cf6b1d74c0996248e290e9f1), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v25", "v2.5" )
	ROMX_LOAD( "md3-25.bin",  0x0000, 0x1000, CRC(14f86bc5) SHA1(82fe022c85f678744bb0340ca3f88b18901fdfcb), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "v31", "v3.1" )
	ROMX_LOAD( "md3-31.bin",  0x0000, 0x1000, CRC(bd4014f6) SHA1(5b33220af34c64676756177db4915f97840b2996), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     CLASS         INIT     COMPANY                  FULLNAME       FLAGS */
COMP( 1982, md2,    0,      0,       microdec,  microdec, driver_device, 0,    "Morrow Designs", "Micro Decision MD-2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1982, md3,    md2,    0,       microdec,  microdec, driver_device, 0,    "Morrow Designs", "Micro Decision MD-3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
