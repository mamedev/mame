// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        SH4 Robot

        http://perso.telecom-paristech.fr/~polti/robot/

        27/11/2013 Skeleton driver.


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
#include "cpu/sh4/sh4.h"

class sh4robot_state : public driver_device
{
public:
	sh4robot_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu") { }


	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(sh4robot_mem, AS_PROGRAM, 64, sh4robot_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x00000fff) AM_ROM
	AM_RANGE(0x08000000, 0x08ffffff) AM_RAM // SDRAM 1
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM // SDRAM 2
	AM_RANGE(0xa0000000, 0xa0000fff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh4robot_io, AS_IO, 64, sh4robot_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static INPUT_PORTS_START( sh4robot )
INPUT_PORTS_END

static MACHINE_CONFIG_START( sh4robot, sh4robot_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4LE, 200000000) // SH7750
	MCFG_SH4_MD0(1)
	MCFG_SH4_MD1(0)
	MCFG_SH4_MD2(1)
	MCFG_SH4_MD3(0)
	MCFG_SH4_MD4(0)
	MCFG_SH4_MD5(1)
	MCFG_SH4_MD6(0)
	MCFG_SH4_MD7(1)
	MCFG_SH4_MD8(0)
	MCFG_SH4_CLOCK(200000000)
	MCFG_CPU_PROGRAM_MAP(sh4robot_mem)
	MCFG_CPU_IO_MAP(sh4robot_io)

MACHINE_CONFIG_END

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

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    CONFIG COMPANY   FULLNAME       FLAGS */
COMP( 20??, sh4robot,  0,       0,  sh4robot,   sh4robot, driver_device,   0,   "<unknown>", "Robot", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
