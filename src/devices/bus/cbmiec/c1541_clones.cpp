// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1541-compatible clone Disk Drive emulation

**********************************************************************/

#include "emu.h"
#include "c1541_clones.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(FSD1,                       fsd1_device,                       "fsd1",     "FSD-1 Disk Drive")
DEFINE_DEVICE_TYPE(FSD2,                       fsd2_device,                       "fsd2",     "FSD-2 Disk Drive")
DEFINE_DEVICE_TYPE(CSD1,                       csd1_device,                       "csd1",     "CSD-1 Disk Drive")
DEFINE_DEVICE_TYPE(INDUS_GT,                   indus_gt_device,                   "indusgt",  "Indus GT Disk Drive")
DEFINE_DEVICE_TYPE(TECHNICA,                   technica_device,                   "technica", "Westfalia Technica Disk Drive")
DEFINE_DEVICE_TYPE(BLUE_CHIP,                  blue_chip_device,                  "bluechip", "Amtech Blue Chip Disk Drive")
DEFINE_DEVICE_TYPE(COMMANDER_C2,               commander_c2_device,               "cmdrc2",   "Commander C-II Disk Drive")
DEFINE_DEVICE_TYPE(ENHANCER_2000,              enhancer_2000_device,              "enh2000",  "Enhancer 2000 Disk Drive")
DEFINE_DEVICE_TYPE(FD148,                      fd148_device,                      "fd148",    "Rapid Access FD-148 Disk Drive")
DEFINE_DEVICE_TYPE(MSD_SD1,                    msd_sd1_device,                    "msdsd1",   "MSD SD-1 Disk Drive")
DEFINE_DEVICE_TYPE(MSD_SD2,                    msd_sd2_device,                    "msdsd2",   "MSD SD-2 Disk Drive")


//-------------------------------------------------
//  ROM( fsd1 )
//-------------------------------------------------

ROM_START( fsd1 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "fsd1.bin", 0x0000, 0x4000, CRC(57224cde) SHA1(ab16f56989b27d89babe5f89c5a8cb3da71a82f0) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *fsd1_device::device_rom_region() const
{
	return ROM_NAME( fsd1 );
}


//-------------------------------------------------
//  ROM( fsd2 )
//-------------------------------------------------

ROM_START( fsd2 )
	ROM_REGION( 0x4000, M6502_TAG, 0 ) // data lines D3 and D4 are swapped
	ROM_DEFAULT_BIOS("rb")
	ROM_SYSTEM_BIOS( 0, "ra", "Revision A" )
	ROMX_LOAD( "fsd2a.u3", 0x0000, 0x4000, CRC(edf18265) SHA1(47a7c4bdcc20ecc5e59d694b151f493229becaea), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "rb", "Revision B" )
	ROMX_LOAD( "fsd2b.u3", 0x0000, 0x4000, CRC(b39e4600) SHA1(991132fcc6e70e9a428062ae76055a150f2f7ac6), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v5.0" )
	ROMX_LOAD( "jiffydos v5.0.u3", 0x0000, 0x4000, CRC(46c3302c) SHA1(e3623658cb7af30c9d3bce2ba3b6ad5ee89ac1b8), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "rexdos", "REX-DOS" )
	ROMX_LOAD( "rdos.bin", 0x0000, 0x4000, CRC(8ad6dba1) SHA1(f279d327d5e16ea1b62fb18514fb679d0b442941), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *fsd2_device::device_rom_region() const
{
	return ROM_NAME( fsd2 );
}


//-------------------------------------------------
//  ROM( csd1 )
//-------------------------------------------------

ROM_START( csd1 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "ic14", 0x0000, 0x2000, CRC(adb6980e) SHA1(13051587dfe43b04ce1bf354b89438ddf6d8d76b) )
	ROM_LOAD( "ic15", 0x2000, 0x2000, CRC(b0cecfa1) SHA1(c67e79a7ffefc9e9eafc238cb6ff6bb718f19afb) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *csd1_device::device_rom_region() const
{
	return ROM_NAME( csd1 );
}


//-------------------------------------------------
//  ROM( indusgt )
//-------------------------------------------------

ROM_START( indusgt )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "u18 v1.1.u18", 0x0000, 0x2000, CRC(e401ce56) SHA1(9878053bdff7a036f57285c2c4974459df2602d8) )
	ROM_LOAD( "u17 v1.1.u17", 0x2000, 0x2000, CRC(575ad906) SHA1(f48837b024add84f888acd83a9cf9eb7d2379172) )

	ROM_REGION( 0x2000, "romdisk", 0 )
	ROM_LOAD( "u19 v1.1.u19", 0x0000, 0x2000, CRC(8f83e7a5) SHA1(5bceaad520dac9d0527723b3b454e8ec99748e5b) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *indus_gt_device::device_rom_region() const
{
	return ROM_NAME( indusgt );
}


//-------------------------------------------------
//  ROM( technica )
//-------------------------------------------------

ROM_START( technica )
	ROM_REGION( 0x4000, M6502_TAG, 0 ) // data lines should be scrambled
	ROM_LOAD( "technica dos plus.bin", 0x0000, 0x4000, BAD_DUMP CRC(6a1ef3ff) SHA1(1aaa52ed4a3f120ec8664bcefec890c7f9aaecf2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *technica_device::device_rom_region() const
{
	return ROM_NAME( technica );
}


//-------------------------------------------------
//  ROM( bluechip )
//-------------------------------------------------

ROM_START( bluechip )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "1", "1" )
	ROMX_LOAD( "bluechip_fd_stockrom.bin", 0x0000, 0x4000, CRC(d4293619) SHA1(18b3dc4c2f919ac8f288d0199e29993a0b53a9bd), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "2", "2" )
	ROMX_LOAD( "amtech_bluechip_rom.bin", 0x0000, 0x4000, CRC(3733ccea) SHA1(c11317cb9370e722950579a610a3effda313aeee), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *blue_chip_device::device_rom_region() const
{
	return ROM_NAME( bluechip );
}


//-------------------------------------------------
//  ROM( cmdrc2 )
//-------------------------------------------------

ROM_START( cmdrc2 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "commander_c-ii_8k_rom1.bin", 0x0000, 0x2000, CRC(cb19daf3) SHA1(9fab414451af54d0bed9d4c9fd5fab1b8720c269) )
	ROM_LOAD( "commander_c-ii_8k_rom2.bin", 0x2000, 0x2000, CRC(ed85a390) SHA1(eecf92fb8cc20a6c86e30f897d09d427509dd3d3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *commander_c2_device::device_rom_region() const
{
	return ROM_NAME( cmdrc2 );
}


//-------------------------------------------------
//  ROM( enh2000 )
//-------------------------------------------------

ROM_START( enh2000 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "enhancer 2000 comtel 2.6.bin", 0x0000, 0x4000, CRC(20353d3b) SHA1(473dd2e06037799e6f562c443165d9b2b9f4a368) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *enhancer_2000_device::device_rom_region() const
{
	return ROM_NAME( enh2000 );
}


//-------------------------------------------------
//  ROM( fd148 )
//-------------------------------------------------

ROM_START( fd148 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "rapid access fd148.bin", 0x0000, 0x4000, CRC(3733ccea) SHA1(c11317cb9370e722950579a610a3effda313aeee) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *fd148_device::device_rom_region() const
{
	return ROM_NAME( fd148 );
}


//-------------------------------------------------
//  ROM( msdsd1 )
//-------------------------------------------------

ROM_START( msdsd1 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "sd-1-1.3-c000.bin", 0x0000, 0x2000, CRC(f399778d) SHA1(c0d939c354d84018038c60a231fc43fb9279d8a4) )
	ROM_LOAD( "sd-1-1.3-e000.bin", 0x2000, 0x2000, CRC(7ac80da4) SHA1(99dd15c6d97938eba73880b18986a037e90742ab) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *msd_sd1_device::device_rom_region() const
{
	return ROM_NAME( msdsd1 );
}


//-------------------------------------------------
//  ROM( msdsd2 )
//-------------------------------------------------

ROM_START( msdsd2 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "sd-2-2.3-c000.bin", 0x0000, 0x2000, CRC(2207560e) SHA1(471e9b4a4ac09ceee9acc1774534510396f98b9a) )
	ROM_LOAD( "sd-2-2.3-e000.bin", 0x2000, 0x2000, CRC(4efd87a2) SHA1(4beec0b7ce2349add3b0a5bceee60826637df8d9) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *msd_sd2_device::device_rom_region() const
{
	return ROM_NAME( msdsd2 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  fsd1_device - constructor
//-------------------------------------------------

fsd1_device::fsd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, FSD1, tag, owner, clock) { }


//-------------------------------------------------
//  fsd2_device - constructor
//-------------------------------------------------

fsd2_device::fsd2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, FSD2, tag, owner, clock) { }


//-------------------------------------------------
//  csd1_device - constructor
//-------------------------------------------------

csd1_device::csd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, CSD1, tag, owner, clock) { }


//-------------------------------------------------
//  indus_gt_device - constructor
//-------------------------------------------------

indus_gt_device::indus_gt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, INDUS_GT, tag, owner, clock) { }


//-------------------------------------------------
//  technica_device - constructor
//-------------------------------------------------

technica_device::technica_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, TECHNICA, tag, owner, clock) { }


//-------------------------------------------------
//  blue_chip_device - constructor
//-------------------------------------------------

blue_chip_device::blue_chip_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, BLUE_CHIP, tag, owner, clock) { }


//-------------------------------------------------
//  commander_c2_device - constructor
//-------------------------------------------------

commander_c2_device::commander_c2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, COMMANDER_C2, tag, owner, clock) { }


//-------------------------------------------------
//  enhancer_2000_device - constructor
//-------------------------------------------------

enhancer_2000_device::enhancer_2000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, ENHANCER_2000, tag, owner, clock) { }


//-------------------------------------------------
//  fd148_device - constructor
//-------------------------------------------------

fd148_device::fd148_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, FD148, tag, owner, clock) { }


//-------------------------------------------------
//  msd_sd1_device - constructor
//-------------------------------------------------

msd_sd1_device::msd_sd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, MSD_SD1, tag, owner, clock) { }


//-------------------------------------------------
//  msd_sd2_device - constructor
//-------------------------------------------------

msd_sd2_device::msd_sd2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, MSD_SD2, tag, owner, clock) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void fsd2_device::device_start()
{
	c1541_device_base::device_start();

	// decrypt ROM
	u8 *rom = memregion(M6502_TAG)->base();

	for (offs_t offset = 0; offset < 0x4000; offset++)
	{
		const u8 data = bitswap<8>(rom[offset], 7, 6, 5, 3, 4, 2, 1, 0);

		rom[offset] = data;
	}
}
