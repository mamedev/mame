/***************************************************************************

  pcfx.c

  Driver file to handle emulation of the NEC PC-FX.

***************************************************************************/


#include "emu.h"
#include "cpu/v810/v810.h"
#include "video/huc6261.h"
#include "video/huc6270.h"


class pcfx_state : public driver_device
{
public:
	pcfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


static ADDRESS_MAP_START( pcfx_mem, AS_PROGRAM, 32, pcfx_state )
	AM_RANGE( 0x00000000, 0x001FFFFF ) AM_RAM	/* RAM */
	AM_RANGE( 0x80700000, 0x807FFFFF ) AM_NOP	/* EXTIO */
	AM_RANGE( 0xE0000000, 0xE7FFFFFF ) AM_NOP
	AM_RANGE( 0xE8000000, 0xE9FFFFFF ) AM_NOP
	AM_RANGE( 0xF8000000, 0xF8000007 ) AM_NOP	/* PIO */
	AM_RANGE( 0xFFF00000, 0xFFFFFFFF ) AM_ROMBANK("bank1")	/* ROM */
ADDRESS_MAP_END


static ADDRESS_MAP_START( pcfx_io, AS_IO, 32, pcfx_state )
	AM_RANGE( 0x00000000, 0x000000FF ) AM_NOP	/* PAD */
	AM_RANGE( 0x00000100, 0x000001FF ) AM_NOP	/* HuC6230 */
	AM_RANGE( 0x00000200, 0x000002FF ) AM_NOP	/* HuC6271 */
	AM_RANGE( 0x00000300, 0x000003FF ) AM_DEVREADWRITE16( "huc6261", huc6261_device, read, write, 0xffff )	/* HuC6261 */
	AM_RANGE( 0x00000400, 0x000004FF ) AM_DEVREADWRITE8( "huc6270_a", huc6270_device, read, write, 0xff )	/* HuC6270-A */
	AM_RANGE( 0x00000500, 0x000005FF ) AM_DEVREADWRITE8( "huc6270_b", huc6270_device, read, write, 0xff )	/* HuC6270-B */
	AM_RANGE( 0x00000600, 0x000006FF ) AM_NOP	/* HuC6272 */
	AM_RANGE( 0x00000C80, 0x00000C83 ) AM_NOP
	AM_RANGE( 0x00000E00, 0x00000EFF ) AM_NOP
	AM_RANGE( 0x00000F00, 0x00000FFF ) AM_NOP
	AM_RANGE( 0x80500000, 0x805000FF ) AM_NOP	/* HuC6273 */
ADDRESS_MAP_END


static INPUT_PORTS_START( pcfx )
INPUT_PORTS_END


static WRITE_LINE_DEVICE_HANDLER( pcfx_irq_changed )
{
}


static const huc6261_interface pcfx_huc6261_config =
{
	"screen",
	"huc6270_a",
	"huc6270_b"
};


static const huc6270_interface pcfx_huc6270_a_config =
{
	0x20000,
	DEVCB_LINE(pcfx_irq_changed),
};


static const huc6270_interface pcfx_huc6270_b_config =
{
	0x20000,
	DEVCB_LINE(pcfx_irq_changed),
};


void pcfx_state::machine_reset()
{
	membank( "bank1" )->set_base( memregion("user1")->base() );
}

UINT32 pcfx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( pcfx, pcfx_state )
	MCFG_CPU_ADD( "maincpu", V810, XTAL_21_4772MHz )
	MCFG_CPU_PROGRAM_MAP( pcfx_mem)
	MCFG_CPU_IO_MAP( pcfx_io)

	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_UPDATE_DRIVER(pcfx_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_21_4772MHz, HUC6261_WPF, 64, 64 + 1024 + 64, HUC6261_LPF, 18, 18 + 242)

	MCFG_HUC6270_ADD( "huc6270_a", pcfx_huc6270_a_config )
	MCFG_HUC6270_ADD( "huc6270_b", pcfx_huc6270_b_config )
	MCFG_HUC6261_ADD( "huc6261", XTAL_21_4772MHz, pcfx_huc6261_config )

MACHINE_CONFIG_END


ROM_START( pcfx )
	ROM_REGION( 0x100000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "v100", "BIOS v1.00 - 2 Sep 1994" )
	ROMX_LOAD( "pcfxbios.bin", 0x000000, 0x100000, CRC(76ffb97a) SHA1(1a77fd83e337f906aecab27a1604db064cf10074), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v101", "BIOS v1.01 - 5 Dec 1994" )
	ROMX_LOAD( "pcfxv101.bin", 0x000000, 0x100000, CRC(236102c9) SHA1(8b662f7548078be52a871565e19511ccca28c5c8), ROM_BIOS(2) )

	ROM_REGION( 0x80000, "scsi_rom", 0 )
	ROM_LOAD( "fx-scsi.rom", 0x00000, 0x80000, CRC(f3e60e5e) SHA1(65482a23ac5c10a6095aee1db5824cca54ead6e5) )
ROM_END


ROM_START( pcfxga )
	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD( "pcfxga.rom", 0x000000, 0x100000, CRC(41c3776b) SHA1(a9372202a5db302064c994fcda9b24d29bb1b41c) )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT      INIT    COMPANY                       FULLNAME                  FLAGS */
CONS( 1994, pcfx,       0,      0,      pcfx,       pcfx, driver_device,      0,      "Nippon Electronic Company",  "PC-FX",                  GAME_NOT_WORKING | GAME_NO_SOUND )
CONS( 199?, pcfxga,     pcfx,   0,      pcfx,       pcfx, driver_device,      0,      "Nippon Electronic Company",  "PC-FX/GA (PC ISA Card)", GAME_NOT_WORKING | GAME_NO_SOUND )

