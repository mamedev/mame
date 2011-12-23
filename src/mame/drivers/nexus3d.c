/************************************************************************

    NEXUS 3D Version 1.0 Board from Interpark

    Games on this platform:

    Arcana Heart FULL, Examu Inc, 2006

    MagicEyes VRENDER 3D Soc (200 MHz ARM920T CPU / GFX / Sound)
    Also Has 2x QDSP QS1000 for sound

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
//#include "machine/i2cmem.h"


#define FLASH_PAGE_SIZE	(2048+64)

class nexus3d_state : public driver_device
{
public:
	nexus3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32* m_mainram;

	//
	UINT8 m_last_flash_cmd;

	int m_flash_addr_seq;
	UINT32 m_flash_addr;

	int m_flash_page_addr;
	UINT8 flash_page_data[FLASH_PAGE_SIZE];

	UINT8* m_flash_region;

};






// Flash handling is very similar (but not 100% identical) to the stuff used in cavesh3 (command structure seems very slightly different, probably due to larger size)
// this is just a skeleton implementation

void nexus3d_flash_reset(running_machine& machine)
{
	nexus3d_state *state = machine.driver_data<nexus3d_state>();
	state->m_last_flash_cmd = 0x00;
	state->m_flash_addr_seq = 0;
	state->m_flash_addr = 0;

	state->m_flash_page_addr = 0;
}

READ8_HANDLER( n3d_flash_r )
{
	nexus3d_state *state = space->machine().driver_data<nexus3d_state>();

	if (state->m_last_flash_cmd==0x70) return 0xe0;

	if (state->m_last_flash_cmd==0x00)
	{
		UINT8 retdat = state->flash_page_data[state->m_flash_page_addr];

		//logerror("n3d_flash_r %02x %04x\n", offset, state->m_flash_page_addr);

		state->m_flash_page_addr++;
		return retdat;
	}


	logerror("n3d_flash_r %02x\n", offset);
	return 0x00;

}


WRITE8_HANDLER( n3d_flash_cmd_w )
{
	nexus3d_state *state = space->machine().driver_data<nexus3d_state>();
	logerror("n3d_flash_cmd_w %02x %02x\n", offset, data);
	state->m_last_flash_cmd = data;

	if (data==0x00)
	{
		memcpy(state->flash_page_data, state->m_flash_region + state->m_flash_addr * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);

	}

}

WRITE8_HANDLER( n3d_flash_addr_w )
{
	nexus3d_state *state = space->machine().driver_data<nexus3d_state>();
//  logerror("n3d_flash_addr_w %02x %02x\n", offset, data);

	state->m_flash_addr_seq++;

	if (state->m_flash_addr_seq==3)
		state->m_flash_addr = (state->m_flash_addr & 0xffff00) | data;

	if (state->m_flash_addr_seq==4)
		state->m_flash_addr = (state->m_flash_addr & 0xff00ff) | data << 8;

	if (state->m_flash_addr_seq==5)
		state->m_flash_addr = (state->m_flash_addr & 0x00ffff) | data << 16;

	if (state->m_flash_addr_seq==5)
	{
		state->m_flash_addr_seq = 0;
		state->m_flash_page_addr = 0;
		logerror("set flash block to %08x\n", state->m_flash_addr);
	}

}

static READ32_HANDLER( nexus3d_unk_r )
{
	return space->machine().rand() ^ (space->machine().rand() << 16);
}

static READ32_HANDLER( nexus3d_unk2_r )
{
	return 0x00000000;//space->machine().rand() ^ (space->machine().rand() << 16);
}

static READ32_HANDLER( nexus3d_unk3_r )
{
	return 0x00000000;//space->machine().rand() ^ (space->machine().rand() << 16);
}

static WRITE32_HANDLER( nexus3d_unk2_w )
{

}

static WRITE32_HANDLER( nexus3d_unk3_w )
{

}

static ADDRESS_MAP_START( nexus3d_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE_MEMBER(nexus3d_state, m_mainram)

	AM_RANGE(0x00400000, 0x01ffffff) AM_RAM // ?? uploads various data, + pointers to data in the 0x01ffxxxx range, might be video system related

	// flash
	AM_RANGE(0x9C000000, 0x9C000003) AM_WRITE8( n3d_flash_r, 0xffffffff)
	AM_RANGE(0x9C000010, 0x9C000013) AM_WRITE8( n3d_flash_cmd_w, 0xffffffff)
	AM_RANGE(0x9C000018, 0x9C00001b) AM_WRITE8( n3d_flash_addr_w, 0xffffffff)

	// lots of accesses in this range
	AM_RANGE(0xC0000F44, 0xC0000F47) AM_READWRITE( nexus3d_unk2_r, nexus3d_unk2_w ) // often
	AM_RANGE(0xC0000F4C, 0xC0000F4f) AM_READWRITE( nexus3d_unk3_r, nexus3d_unk3_w ) // often

	AM_RANGE(0xE0000014, 0xE0000017) AM_READ( nexus3d_unk_r ) // sits waiting for this


ADDRESS_MAP_END

static INPUT_PORTS_START( nexus3d )

INPUT_PORTS_END


VIDEO_START( nexus3d )
{

}

SCREEN_UPDATE(nexus3d)
{
	return 0;
}

static MACHINE_RESET (nexus3d)
{
	nexus3d_flash_reset(machine);
}

static MACHINE_CONFIG_START( nexus3d, nexus3d_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM920T, 200000000)
	MCFG_CPU_PROGRAM_MAP(nexus3d_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1)
	MCFG_SCREEN_UPDATE(nexus3d)

	MCFG_PALETTE_LENGTH(256)

	MCFG_MACHINE_RESET( nexus3d )

	MCFG_VIDEO_START(nexus3d)
MACHINE_CONFIG_END


// The u1 flash on this is clearly recycled from a Happy Fish or Blue Elf multigame.
// Around 75% of the rom is NeoGeo, CPS2, Semicom etc. MAME romsets used by said multigame bootlegs
// which explains why the 1Gb flash rom hardly compresses, it's already compressed data.
//
// I highly suspect this upgrade (to Full) was done at the PCB shop to boost the value of the PCB, and
// that the original game used a smaller flash.  It seems highly unlikely that Examu would ship ROMs
// containing the entire backcatalog of SNK and Capcom material ;-)
//
// It's possible this set should be marked as a bootleg due to this although I imagine the actual valid
// part of the data will match a clean dump.
ROM_START( acheartf )
	ROM_REGION( 0x42000898, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcana_heart_full.u1",     0x000000, 0x42000898, CRC(1a171ca3) SHA1(774f3b8d5fb366901d819b5dc15ca49b0cd177b9) )

	ROM_REGION( 0x200000, "user2", 0 ) // QDSP stuff
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x000000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x000000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x000000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )

	ROM_REGION( 0x080000, "wavetable", ROMREGION_ERASEFF ) /* QDSP wavetable rom */
//  ROM_LOAD( "qs1001a",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) // missing from this set, but should be the same
ROM_END

static DRIVER_INIT( nexus3d )
{
	nexus3d_state *state = machine.driver_data<nexus3d_state>();
	// the first part of the flash ROM automatically gets copied to RAM
	memcpy( state->m_mainram, machine.region("user1")->base(), 4 * 1024);
	state->m_flash_region = machine.region("user1")->base();
}

GAME( 2006, acheartf, 0, nexus3d, nexus3d, nexus3d, ROT0, "Examu", "Arcana Heart Full",GAME_IS_SKELETON )
