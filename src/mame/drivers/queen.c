/* Queen */

/*

Produttore  STG
N.revisione
CPU main PCB is a standard EPIA
ROMs    epia BIOS + solid state HD

1x VIA EPIA5000EAG (main PCB) with:
VT8231 South Bridge
VIA Eden Processor
VIA EPIA Companion Chip VT1612A (Audio CODEC)
VIA EPIA Companion Chip VT6103 (Networking)
processor speed is 533MHz <- likely to be a Celeron or a Pentium III class CPU -AS

 it's a 2002 era PC at least based on the BIOS,
  almost certainly newer than the standard 'PENTIUM' CPU

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i386/i386.h"


class queen_state : public driver_device
{
public:
	queen_state(const machine_config &mconfig, device_type type, const char *tag)
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


void queen_state::video_start()
{

}

bool queen_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( queen_map, AS_PROGRAM, 32, queen_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000fffff) AM_ROM AM_REGION("bios", 0) AM_WRITENOP
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0x10000000, 0x100001ff) AM_RAM
	AM_RANGE(0x20000000, 0x200001ff) AM_RAM
	AM_RANGE(0x30000000, 0x300001ff) AM_RAM
	AM_RANGE(0x40000000, 0x400001ff) AM_RAM
	AM_RANGE(0x50000000, 0x500001ff) AM_RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( queen_io, AS_IO, 32, queen_state )
	AM_RANGE(0x0000, 0x001f) AM_RAM//AM_DEVREADWRITE8("dma8237_1", dma8237_r, dma8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_RAM//AM_DEVREADWRITE8("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_RAM//AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_RAM//AM_READWRITE(kbdc8042_32le_r,          kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_RAM//AM_DEVREADWRITE8_MODERN("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_RAM//AM_READWRITE(at_page32_r,              at_page32_w)
	AM_RANGE(0x00a0, 0x00bf) AM_RAM//AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_RAM//AM_DEVREADWRITE("dma8237_2", at32_dma8237_2_r, at32_dma8237_2_w)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP
	AM_RANGE(0x01f0, 0x01f7) AM_RAM//AM_DEVREADWRITE("ide", ide_r, ide_w)
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
	AM_RANGE(0x0278, 0x027b) AM_WRITENOP//AM_WRITE(pnp_config_w)
	AM_RANGE(0x03f0, 0x03ff) AM_RAM//AM_DEVREADWRITE("ide", fdc_r, fdc_w)
	AM_RANGE(0x0a78, 0x0a7b) AM_WRITENOP//AM_WRITE(pnp_data_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_RAM//AM_DEVREADWRITE("pcibus", pci_32le_r,  pci_32le_w)
	AM_RANGE(0x4004, 0x4007) AM_RAM // - todo: identify these two.
	AM_RANGE(0x5000, 0x5007) AM_RAM // /
ADDRESS_MAP_END


static INPUT_PORTS_START( queen )
INPUT_PORTS_END


static MACHINE_CONFIG_START( queen, queen_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 533000000) // Celeron or Pentium 3, 533 Mhz
	MCFG_CPU_PROGRAM_MAP(queen_map)
	MCFG_CPU_IO_MAP(queen_io)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(0x200)
MACHINE_CONFIG_END




ROM_START( queen )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "bios-original.bin", 0x00000, 0x40000, CRC(feb542d4) SHA1(3cc5d8aeb0e3b7d9ed33248a4f3dc507d29debd9) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "pqiidediskonmodule", 0,SHA1(a56efcc711b1c5a2e63160b3088001a8c4fb56c2) )
ROM_END


GAME( 2002?, queen,  0,    queen, queen,  0, ROT0, "STG", "Queen?", GAME_IS_SKELETON )
