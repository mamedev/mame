/*

 IGS PC based hardware

 4 boards
    1x NV440 gfx card
    1x sound card
    1x CF2IDE card
    1x proteection card with a IGS027A (ARM7 with internal ROM)

 The CF dump contains no MBR, it is thought that it is stored in either the main bios
 or provided by the IGS027A.  If you disconnect the protection card the bios will hang
 when detecting the HDDs which further backs up the theory that it is provided or
 decrypted by the 027A

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i386/i386.h"

class speeddrv_state : public driver_device
{
public:
	speeddrv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( speeddrv_map, AS_PROGRAM, 32, speeddrv_state )
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( speeddrv_io, AS_IO, 32, speeddrv_state )
ADDRESS_MAP_END


static INPUT_PORTS_START( speeddrv )
INPUT_PORTS_END



static MACHINE_CONFIG_START( speeddrv, speeddrv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486, 40000000 ) // ?? at least a pentium
	MCFG_CPU_PROGRAM_MAP(speeddrv_map)
	MCFG_CPU_IO_MAP(speeddrv_io)
MACHINE_CONFIG_END


ROM_START( speeddrv )
	ROM_REGION32_LE(0x40000, "bios", 0)	/* motherboard bios */
	ROM_LOAD( "mainbios", 0x0000, 0x040000, NO_DUMP )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "speed_driver_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "speeddrv", 0, SHA1(88712a37b75d84cf9b5a4bee9386285d1b3760b3) )
ROM_END

static DRIVER_INIT(speeddrv)
{

}

GAME( 2004,  speeddrv,  0,  speeddrv,  speeddrv,  speeddrv,  ROT0,  "IGS",    "Speed Driver",    GAME_IS_SKELETON )
