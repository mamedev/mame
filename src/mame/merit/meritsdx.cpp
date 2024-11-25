// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    Skeleton driver for Merit Scorpion DX darts machines

    Hardware overview:
    Main CPU: AMD Elan SC300-33KC (Am386SXLV CPU core + 6845 CGA)
    Others: Dallas 1232, M48T08-150PC1, F82C735A, PCMCIA slot (for a modem),
            LCD CGA screen and magnetic stripe card reader
    OSC: 24.000 MHz

    Notes:
    Uses a ROM-DOS as its operating system.
    The card reader emulates a PS2 keyboard. The keyboard BIOS is undumped
    (it's a Phoenix BIOS on a protected Intel 8242 MCU).
    The Flash ROMs contain user data (from a championship on Spain). Once emulated,
    they must be reflashed with a factory reset (can be done on the emulated
    system) and updated on the driver.

*******************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "speaker.h"


namespace {

class meritsdx_state : public driver_device
{
public:
	meritsdx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void scrpndx(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void meritsdx_state::mem_map(address_map &map)
{
}

void meritsdx_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START(scrpndx)
INPUT_PORTS_END

void meritsdx_state::scrpndx(machine_config &config)
{
	I386SX(config, m_maincpu, 24_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &meritsdx_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &meritsdx_state::io_map);
}

ROM_START(scrpndx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "4999-02-00_u54-r0_c1997_mii.u54", 0x00000, 0x10000, CRC(dc834efc) SHA1(08bb1e0e818114ab3d4fa9cfecad337cf23a73d7) ) // mostly empty

	ROM_REGION(0x80000, "romdos", 0)
	ROM_LOAD( "4999-01-00_u20-r1_c1997_mii.u20", 0x00000, 0x80000, CRC(3c32355e) SHA1(56f846e655184b66db19cd01481e0016c463cf7d) )

	// The Flash ROMs contain user data
	ROM_REGION(0x400000, "flashroms", 0)
	ROM_LOAD( "am29f800bb.u22", 0x000000, 0x100000, CRC(9b7510a5) SHA1(76ed78da976f8898a7a084fa5a37ea66aba886ff) )
	ROM_LOAD( "am29f800bb.u23", 0x100000, 0x100000, CRC(8dc09838) SHA1(01e150daecac5f148a8cc6fb23715cf8b2cbd48a) )
	ROM_LOAD( "am29f800bb.u24", 0x200000, 0x100000, CRC(c5d5a5ee) SHA1(66bfb9a1eccd43c3d5ede0c4f17c231593729926) )
	ROM_LOAD( "am29f800bb.u25", 0x300000, 0x100000, CRC(1b26141d) SHA1(bf60c1f8ff9119aebc172fbacec3de89e2513c1c) )

	ROM_REGION(0x5ba, "plds", 0)
	ROM_LOAD( "sc_3986_intr_palce22v10q-25.u81",  0x000, 0x2dd, CRC(2a33bc54) SHA1(ed40828154f6c75f2c0109f6df6bbaef0773f841) )
	ROM_LOAD( "sc_3987_pc104_palce22v10q-25.u88", 0x2dd, 0x2dd, CRC(460aea86) SHA1(8ffbd42bfc90e356db7d618b376fab7fe761e216) )
ROM_END

} // anonymous namespace


GAME(1996, scrpndx, 0, scrpndx, scrpndx, meritsdx_state, empty_init, ROT0, "Merit", "Scorpion DX", MACHINE_IS_SKELETON_MECHANICAL) // OCT 15 1996 -- ASM 15:30
