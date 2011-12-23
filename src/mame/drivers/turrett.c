/*

Turret Tower by Dell Electronics

PCB Info
========

Silkscreened    Copyright (c) 2001 Dell Electroinics Labs, Ltd

Samsung SV2001H Hard drive  stickered   (c)DELL V1.XX
                        TOCAB0181
                        TURRET TOWER

IDT         79R3041-25J
        XG0110P

Xilinx Spartan  XCS30XL         x2
        PQ208AKP0105
        D1164035A
        4c


Xilinx      XC9572
        PC84AEM0109
        A1172748A
        10C

IDT     71124           x8
        S12Y
        N0048M

COMPAQ      MT16LSDT1664AG-10CY5    SDRAM stick x2

.u7 AM29F040B   stickered   U7 (c)DELL

.u8 AM29F040B   stickered   U8 (c)DELL

.u12    AM29F040B   stickered   U12 (c)DELL

.u13    AM29F040B   stickered   U13 (c)DELL

.u29            stickered   TTML(1) (c) DELL    Unmarked chip looks like 28 PIN DIP PLD


CHDMAN info
Version 0.128
Input offset    511
Cycliders   2438
Heads       255
Sectors     63
Bytes/Sector    512
Sectors/Hunk    8
Logical size    20,053,232,640

Windows showed a 5.94 gig partion empty and a 12.74 unallocated partition


*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/mips/r3000.h"


class turrett_state : public driver_device
{
public:
	turrett_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
};


#define R3041_CLOCK		25000000


void turrett_state::video_start()
{
}

bool turrett_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 32, turrett_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x02000010, 0x02000013) AM_RAM
	AM_RANGE(0x02000040, 0x02000043) AM_RAM
	AM_RANGE(0x02000050, 0x02000053) AM_RAM
	AM_RANGE(0x02000060, 0x02000063) AM_RAM
	AM_RANGE(0x02000070, 0x02000073) AM_RAM
	AM_RANGE(0x04000100, 0x04000103) AM_RAM
	AM_RANGE(0x08000000, 0x08000003) AM_RAM
	AM_RANGE(0x08000004, 0x08000007) AM_RAM
	AM_RANGE(0x08000008, 0x0800000b) AM_RAM
	AM_RANGE(0x0800000c, 0x0800000f) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( turrett )
INPUT_PORTS_END


static const r3000_cpu_core r3000_config =
{
	0,		/* 1 if we have an FPU, 0 otherwise */
	2048,	/* code cache size */
	512		/* data cache size */
};


static MACHINE_CONFIG_START( turrett, turrett_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R3041BE, R3041_CLOCK)
	MCFG_CPU_PROGRAM_MAP(cpu_map)
	MCFG_CPU_CONFIG(r3000_config)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(0x2000)
MACHINE_CONFIG_END


ROM_START( turrett )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "turret.u13", 0x000000, 0x080000, CRC(85287007) SHA1(990b954905c66340d3e88918b2f8cc7f1b9c7cf4) )
	ROM_LOAD32_BYTE( "turret.u12", 0x000001, 0x080000, CRC(a2a498fc) SHA1(47f2c9c9f2496b49fd923acb400166e963095e1d) )
	ROM_LOAD32_BYTE( "turret.u8",  0x000002, 0x080000, CRC(ddff4898) SHA1(a8f859a0dcab8ec83fbfe255d58b3e644933b923) )
	ROM_LOAD32_BYTE( "turret.u7",  0x000003, 0x080000, CRC(fa8b5a5a) SHA1(658e9eeadc9c70185973470565d562c76f4fcdd7) )

	DISK_REGION( "disks" )
	DISK_IMAGE( "turrett", 0, SHA1(b0c98c5876870dd8b3e37a38fe35846c9e011df4) )
ROM_END


GAME( 2001, turrett, 0, turrett, turrett, 0, ROT0, "Dell Electronics (Namco license)", "Turret Tower", GAME_IS_SKELETON )
