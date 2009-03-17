/************************************************************************************

=====================================================================================

California Chase
The Game Room, 1999

This is a rip off of Chase HQ running on PC hardware
and standard 15kHz arcade monitor

Main board is a cheap Taiwanese one made by SOYO.
Model SY-5EAS
Major main board chips are....
Cyrix 686MX-PR200 CPU
1M BIOS (Winbond 29C011)
64M RAM (2x 32M 72 pin SIMMs)
ETEQ EQ82C6617'97 MB817A0AG12 (QFP100, x2)
UT6164C32Q-6 (QFP100, x2)
ETEQ 82C6619'97 MB13J15001 (QFP208)
ETEQ 82C6618'97 MB14B10971 (QFP208)
UM61256
PLL52C61
SMC FD37C669QF (QFP100)
3V coin battery
3x PCI slots
4x 16-bit ISA slots
4x 72 pin RAM slots
connectors for COM1, COM2, LPT1, IDE0, IDE1, floppy etc
uses standard AT PSU

Video card is Trident TGUI9680 with 512k on-board VRAM
RAM is AS4C256K16EO-50JC (x2)
Trident BIOS V5.5 (DIP28). Actual size unknown, dumped as 64k, 128k, 256k and 512k (can only be one of these sizes)
6.5536MHz xtal

Custom JAMMA I/O board plugs into one ISA slot
Major components are...
XILINX XC3042
Dallas DS1220Y NVRAM (dumped)
MACH210 (protected)
16MHz OSC
VGA connector (tied to VGA card with a cable)
2x 4-pin connectors for controls
AD7547
ULN2803
LM324 (op amp)
TDA1552 (power amp)
ST TS272 (dual op amp)
2 volume pots
power input connector (from AT PSU)
JAMMA edge connector
ISA edge connector

HDD is WD Caviar 2170. C/H/S = 1010/6/55. Capacity = 170.6MB
The HDD is DOS-readable and in fact the OS is just Windows 98 DOS and can
be easily copied. Tested with another HDD.... formatted with DOS, copied
all files across to new HDD, boots up fine.

************************************************************************************/

#include "driver.h"
#include "cpu/i386/i386.h"

static VIDEO_START(calchase)
{

}

static VIDEO_UPDATE(calchase)
{
	return 0;
}

static ADDRESS_MAP_START( calchase_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0) AM_WRITENOP
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0x08000000, 0x080001ff) AM_RAM
	AM_RANGE(0x10000000, 0x100001ff) AM_RAM
	AM_RANGE(0x18000000, 0x180001ff) AM_RAM
	AM_RANGE(0x20000000, 0x200001ff) AM_RAM
	AM_RANGE(0x28000000, 0x280001ff) AM_RAM
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(calchase_io, ADDRESS_SPACE_IO, 32)
	AM_RANGE(0x0000, 0x001f) AM_RAM//AM_DEVREADWRITE8("dma8237_1", dma8237_r, dma8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_RAM//AM_DEVREADWRITE8("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_RAM//AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_RAM//AM_READWRITE(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_RAM//AM_READWRITE(mc146818_port32le_r,		mc146818_port32le_w)
	AM_RANGE(0x0080, 0x009f) AM_RAM//AM_READWRITE(at_page32_r,				at_page32_w)
	AM_RANGE(0x00a0, 0x00bf) AM_RAM//AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_RAM//AM_DEVREADWRITE("dma8237_2", at32_dma8237_2_r, at32_dma8237_2_w)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP
	AM_RANGE(0x01f0, 0x01f7) AM_RAM//AM_DEVREADWRITE("ide", ide_r, ide_w)
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
	AM_RANGE(0x0278, 0x027b) AM_WRITENOP//AM_WRITE(pnp_config_w)
	AM_RANGE(0x03f0, 0x03ff) AM_RAM//AM_DEVREADWRITE("ide", fdc_r, fdc_w)
	AM_RANGE(0x0a78, 0x0a7b) AM_WRITENOP//AM_WRITE(pnp_data_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_RAM//AM_DEVREADWRITE("pcibus", pci_32le_r,	pci_32le_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( calchase )
INPUT_PORTS_END


static MACHINE_DRIVER_START( calchase )
	MDRV_CPU_ADD("maincpu", PENTIUM, 200000000) // Cyrix 686MX-PR200 CPU
	MDRV_CPU_PROGRAM_MAP(calchase_map, 0)
	MDRV_CPU_IO_MAP(calchase_io, 0)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(calchase)
	MDRV_VIDEO_UPDATE(calchase)
MACHINE_DRIVER_END




ROM_START( calchase )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "mb_bios.bin", 0x00000, 0x20000, CRC(dea7a51b) SHA1(e2028c00bfa6d12959fc88866baca8b06a1eab68) )

	ROM_REGION( 0x8000, "video_bios", 0 )
	ROM_LOAD( "trident_tgui9680_bios.bin", 0x0000, 0x8000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )

	ROM_REGION( 0x800, "nvram", 0 )
	ROM_LOAD( "ds1220y_nv.bin", 0x000, 0x800, CRC(7912c070) SHA1(b4c55c7ca76bcd8dad1c4b50297233349ae02ed3) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "calchase", 0,SHA1(487e304ffeed23ca618fa936258136605ce9d1a1) )
ROM_END


GAME( 1999, calchase,  0,    calchase, calchase,  0, ROT0, "The Game Room", "California Chase", GAME_NOT_WORKING|GAME_NO_SOUND )
