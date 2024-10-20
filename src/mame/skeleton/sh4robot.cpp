// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

SH4 Robot

http://web.archive.org/web/20131127151413/perso.telecom-paristech.fr/~polti/robot/

Original site died. None of the downloads in the above wayback page work, so fairly useless.


2013-11-27 Skeleton driver.


      0x0000 0000 - 0x7FFF FFFF     : P0 area, cachable
      0x8000 0000 - 0x9FFF FFFF     : P1 area, cachable
      0xA000 0000 - 0xBFFF FFFF     : P2 area, non-cachable
      0xC000 0000 - 0xDFFF FFFF     : P3 area, cachable
      0xE000 0000 - 0xFFFF FFFF     : P4 area, non-cachable


      0x0000 0000 - 0x03FF FFFF     : Area 0 (boot, ROM)
      0x0400 0000 - 0x07FF FFFF     : Area 1 (FPGA)
      0x0800 0000 - 0x08FF FFFF     : Area 2 (SDRAM 1, 16M)
      0x0C00 0000 - 0x0CFF FFFF     : Area 3 (SDRAM 2, 16M)
      0x1000 0000 - 0x13FF FFFF     : Area 4 (FPGA)
      0x1400 0000 - 0x17FF FFFF     : Area 5 (FPGA)
      0x1800 0000 - 0x1BFF FFFF     : Area 6 (FPGA)
      0x1C00 0000 - 0x1FFF FFFF     : Area 7 (reserved)

****************************************************************************/

#include "emu.h"
#include "cpu/sh/sh4.h"


namespace {

class sh4robot_state : public driver_device
{
public:
	sh4robot_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	{ }

	void sh4robot(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<sh4_device> m_maincpu;
};


void sh4robot_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x00000fff).rom();
	map(0x08000000, 0x08ffffff).ram(); // SDRAM 1
	map(0x0c000000, 0x0cffffff).ram(); // SDRAM 2
	map(0xa0000000, 0xa0000fff).rom().region("maincpu", 0);
}

void sh4robot_state::io_map(address_map &map)
{
	map.unmap_value_high();
}

static INPUT_PORTS_START( sh4robot )
INPUT_PORTS_END

void sh4robot_state::sh4robot(machine_config &config)
{
	/* basic machine hardware */
	SH4LE(config, m_maincpu, 200000000); // SH7750
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(200000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sh4robot_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sh4robot_state::io_map);
	m_maincpu->set_force_no_drc(true);
}

/* ROM definition */
ROM_START( sh4robot )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bootloader.bin", 0x0000, 0x0882, CRC(d2ea0b7d) SHA1(7dd566c5e325d1ce1156a0bcbd7e10d011e9d35f))

	// FLASH TC58128AFT
	// flash blocks 1 till 199 (1*32*512 till 199*32*512)
	//ROM_LOAD( "vmlinux-nand_img_with_oob-2.6.10-v1.0", 0x0000, 0x149be0, CRC(eec69ef5) SHA1(524e26d2c2c28061911f4726646b18596d134736))
	// Root FS at flash blocks from 201 till end (201*32*512)
	//ROM_LOAD( "shix-linux-v1.0.yaffs", 0x0000, 0x7e9e40, CRC(7a7fdb04) SHA1(0b761e2d179335398399cb046de4e591157cb72f))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY      FULLNAME  FLAGS
COMP( 20??, sh4robot, 0,      0,      sh4robot, sh4robot, sh4robot_state, empty_init, "<unknown>", "Robot",  MACHINE_IS_SKELETON_MECHANICAL )
