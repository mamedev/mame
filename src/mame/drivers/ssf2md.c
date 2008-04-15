#include "driver.h"
#include "megadriv.h"

ROM_START( ssf2ghw )
	ROM_REGION( 0x1400000, REGION_CPU1, 0 ) /* 68000 Code */
	/* Special Case, custom PCB, linear ROM mapping of 5meg */
	ROM_LOAD16_BYTE( "rom_a", 0x000000, 0x200000,  CRC(59726521) SHA1(3120bac17f56c01ffb9d3f9e31efa0263e3774af) )
	ROM_LOAD16_BYTE( "rom_b", 0x000001, 0x200000,  CRC(7dad5540) SHA1(9279068b2218d239fdd557dd959ac70e74853178) )
	ROM_LOAD16_BYTE( "rom_c", 0x400000, 0x080000,  CRC(deb48624) SHA1(39ffa7de7b808e0b95cb039bb381705d77420933) )
	ROM_LOAD16_BYTE( "rom_d", 0x400001, 0x080000,  CRC(b99f6a5b) SHA1(adbe28a7522024bc66328ac86fecf9ded3310e8e) )
ROM_END

static READ16_HANDLER( ssf2ghw_dsw_r )
{
	static const char *const dswname[3] = { "DSWA", "DSWB", "DSWC" };
	return input_port_read(machine, dswname[offset]);
}

static DRIVER_INIT( ssf2ghw )
{
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xA130F0, 0xA130FF, 0, 0, SMH_NOP); // custom banking is disabled (!)
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x5fffff, 0, 0, SMH_BANK5, SMH_UNMAP);

	memory_set_bankptr( 5, memory_region( REGION_CPU1 )+0x400000 );

	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x770070, 0x770075, 0, 0, ssf2ghw_dsw_r );

	DRIVER_INIT_CALL(megadrij);

}

GAME( 1994, ssf2ghw,  0,   megadriv,    ssf2ghw,     ssf2ghw,  ROT0,   "bootleg / Capcom", "Super Street Fighter II - The New Challengers (Arcade bootleg of Japanese MegaDrive version)", 0 )
