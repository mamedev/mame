// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/******************************************************************************************************************************************************

	Seibu CATS E-Touch Mahjong Series (c) 2001 Seibu Kaihatsu
	
	TODO:
	- Needs OBJ ROMs dump to progress;
	- DVD hookup;
	- EEPROM (prolly tied to area 0xa20000 here);
	- inputs & touchscreen;
	- is sound working?
	- unknown area read at 0x600;
	- verify interrupt table;
	- Any other port lingering in the 0x400-0x7ff area?
	
=======================================================================================================================================================
	Here is an excessively detailed list of the significant devices on the main board, including all those worth emulating and many that aren't:

    * Intel i386DX CPU (U0169; lower right corner). Contrary to my expectations, Seibu seems to have stuck by its x86 tradition instead of switching to the 
	  SH-2 with its built-in DMA, as they did in Fever Soccer.
    * SEI600 SB08-1513 custom DMA chip (U0154; above i386DX). This is currently emulated as part of the driver for the SPI system, where it handles the 
	  tilemap and palette DMA. E Jong Sakurasou, having only sprites, abandoned the tilemap DMA channel, but it may have been repurposed here.
    * RISE11 custom sprite chip (U0336; upper right corner). No surprises here; Raiden Fighters Jet, E Jong Sakurasou and Fever Soccer all use this with 
	  or without encryption.
    * Yamaha YMF721-S General MIDI OPL4-ML2 (U0274; to right of UARTs). MAME surprisingly lacks an emulation of this chip.
    * Yamaha YMZ280B-F PCMD8 (U0722; middle of board). Also used in E Jong Sakurasou.
    * NEC D71051GB UART x2 (U1133, U1134; lined up with DB9 ports).
    * MAXZ32 Serial Line Driver x2 (U1138, U1141; between UARTs and DB9 ports).
    * JRC 6355E Real Time Clock (U0513, above YMF721). I emulated this chip for Fever Soccer.
    * ST93C46AF Serial EEPROM (U0512; towards left center of board).
    * Yamaha YAC516-M DAC x2 (U0848, U085; below first row of batteries).
    * Xilinx XC9536 CPLDs "DVDMJ11" (U0235), "DVDMJ12" (U0236), XC9572 "DVDMJ13" (U1024).
    * Toshiba TA7252AP power amplifier (U0847; at left edge, next to AUDIO-IN).
    * JRC 4560 operational amplifier x2 (U0850, U087; amid battery jungle)
    * NEC UPC1830GT filter video chroma (U0935; towards lower left).
    * JRC 2246 video switch x3 (U1015, U1017, U1022; near bottom, above more batteries).
    * LM1881 Video Sync Separator (U1020).
    * RAM area A (U0230, U0231; between SEI600 and RISE11) = either 2x Toshiba TC551664BJ-15 or 2x Winbond W26010AJ-15.
    * RAM area B (U067, U062, U0326, U0327; to left of RISE11) = either 4x G-Link GLT725608-15J3 or 4x Cypress CY7C199-15VC (Ver1.3 board).
    * RAM area C (U0727, U0728) = either 2x BSI B562LV1024SC-70 or 1x Hitachi HM628512ALFP-7. The latter leaves U0727 unpopulated, revealing a label for 
	  a MX23C8000M mask ROM.

******************************************************************************************************************************************************/


#include "emu.h"
#include "includes/seibuspi.h"

#include "cpu/i386/i386.h"
#include "sound/ymz280b.h"
#include "screen.h"
#include "speaker.h"

// TBD, assume same as Seibu SPI
#define MAIN_CLOCK   (XTAL_50MHz/2)
#define PIXEL_CLOCK  (XTAL_28_63636MHz/4)

#define SPI_HTOTAL   (448)
#define SPI_HBEND    (0)
#define SPI_HBSTART  (320)

#define SPI_VTOTAL   (296)
#define SPI_VBEND    (0)
#define SPI_VBSTART  (240) /* actually 253, but visible area is 240 lines */


class seibucats_state : public seibuspi_state
{
public:
	seibucats_state(const machine_config &mconfig, device_type type, const char *tag)
		: seibuspi_state(mconfig, type, tag),
		m_ymz(*this, "ymz")
	{
	}

	required_device <ymz280b_device> m_ymz;
	// screen updates
//	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
//	IRQ_CALLBACK_MEMBER(spi_irq_callback);
//	INTERRUPT_GEN_MEMBER(spi_interrupt);
	
	DECLARE_READ8_MEMBER( sound_r );
	
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

//	virtual void video_start() override;


private:

};

READ8_MEMBER( seibucats_state::sound_r )
{
	if(offset == 0)
		return m_ymz->read(space,0);
	
//	printf("read unknown device\n");
	return machine().rand();
}

static ADDRESS_MAP_START( seibucats_map, AS_PROGRAM, 32, seibucats_state )
	// TODO: map devices
	AM_RANGE(0x00000010, 0x00000013) AM_READ8(spi_status_r, 0x000000ff)
	AM_RANGE(0x00000484, 0x00000487) AM_WRITE(palette_dma_start_w)
	AM_RANGE(0x00000490, 0x00000493) AM_WRITE(video_dma_length_w)
	AM_RANGE(0x00000494, 0x00000497) AM_WRITE(video_dma_address_w)
	AM_RANGE(0x00000560, 0x00000563) AM_WRITE16(sprite_dma_start_w, 0xffff0000)
	
	AM_RANGE(0x00000408, 0x0000040f) AM_DEVWRITE8("ymz", ymz280b_device, write, 0x000000ff)
	AM_RANGE(0x00000600, 0x00000607) AM_READ8( sound_r, 0x0000ffff) //AM_DEVREAD8("ymz", ymz280b_device, read, 0x000000ff) 

	AM_RANGE(0x00000000, 0x0003ffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x00200000, 0x003fffff) AM_ROM AM_REGION("ipl", 0) AM_WRITENOP // emjjoshi attempts to write there?
	// following are likely to be Seibu CATS specific
	AM_RANGE(0x01200000, 0x01200007) AM_NOP // address/data 8-bit r/w
	AM_RANGE(0x01200100, 0x01200103) AM_WRITENOP
	AM_RANGE(0x01200104, 0x01200107) AM_READNOP
	AM_RANGE(0x01200200, 0x01200203) AM_READNOP
	AM_RANGE(0x01200204, 0x01200207) AM_NOP
	AM_RANGE(0x01200300, 0x01200303) AM_READNOP
	AM_RANGE(0x01200304, 0x01200307) AM_NOP
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( seibucats )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout sys386f_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ 0, 8, RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+8 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};


static GFXDECODE_START( seibucats )
	GFXDECODE_ENTRY( "gfx1", 0, sys386f_spritelayout,   5632, 16 ) // Not used, legacy charlayout
	GFXDECODE_ENTRY( "gfx2", 0, sys386f_spritelayout,   4096, 24 ) // Not used, legacy tilelayout
	GFXDECODE_ENTRY( "gfx3", 0, sys386f_spritelayout,   0, 96 )
GFXDECODE_END


void seibucats_state::machine_start()
{
}

void seibucats_state::machine_reset()
{
}

#if 0
// do not remove, might be needed for the DVD stuff (unchecked)
INTERRUPT_GEN_MEMBER(seibucats_state::spi_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE); // where is ack?
}

IRQ_CALLBACK_MEMBER(seibucats_state::spi_irq_callback)
{
	return 0x20;
}
#endif

static MACHINE_CONFIG_START( seibucats )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I386, MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(seibucats_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", seibuspi_state, spi_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(seibuspi_state, spi_irq_callback)
	
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//	MCFG_SCREEN_UPDATE_DRIVER(seibucats_state, screen_update)
	MCFG_SCREEN_UPDATE_DRIVER(seibuspi_state, screen_update_sys386f)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, SPI_HTOTAL, SPI_HBEND, SPI_HBSTART, SPI_VTOTAL, SPI_VBEND, SPI_VBSTART)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", seibucats)

	MCFG_PALETTE_ADD_INIT_BLACK("palette", 8192)
//	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	//MCFG_PALETTE_INIT_OWNER(seibucats_state, seibucats)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_16_384MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( emjjoshi )
	ROM_REGION32_LE( 0x200000, "ipl", 0 ) /* i386 program */
    ROM_LOAD32_BYTE( "prg0.u016",    0x000000, 0x080000, CRC(e69bed6d) SHA1(e9626e704c5d28419cfa6a7a2c1b13b4b46f941c) )
    ROM_LOAD32_BYTE( "prg1.u011",    0x000001, 0x080000, CRC(1082ede1) SHA1(0d1a682f37ede5c9070c14d1c3491a3082ad0759) )
    ROM_LOAD32_BYTE( "prg2.u017",    0x000002, 0x080000, CRC(df85a8f7) SHA1(83226767b0c33e8cc3baee6f6bb17e4f1a6c9c27) )
    ROM_LOAD32_BYTE( "prg3.u015",    0x000003, 0x080000, CRC(6fe7fd41) SHA1(e7ea9cb83bdeed4872f9e423b8294b9ca4b29b6b) )
	
	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF ) /* text layer roms - none! */

	ROM_REGION( 0x900000, "gfx2", ROMREGION_ERASEFF ) /* background layer roms - none! */

	ROM_REGION( 0x600000, "gfx3", 0)   
	ROM_LOAD("obj1.u0231", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD("obj2.u0233", 0x200000, 0x200000, NO_DUMP )
	ROM_LOAD("obj3.u0232", 0x400000, 0x200000, NO_DUMP )
//  obj4.u0234 empty slot

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "At the Girls Dorm SKTP-10002", 0, SHA1(be47c105089d6ef4ce05a6e1ba2ec7a3101015dc) )
ROM_END


// MJ1-1537
ROM_START( emjscanb )
	ROM_REGION32_LE( 0x200000, "ipl", 0 ) /* i386 program */
    ROM_LOAD32_BYTE( "prg0.u016",    0x000000, 0x080000, CRC(6e5c7c16) SHA1(19c00833357b97d0ed91a962e95d3ae2582da66c) )
    ROM_LOAD32_BYTE( "prg1.u011",    0x000001, 0x080000, CRC(a5a17fdd) SHA1(3295ecb1055cf1ab612eb915aabe8d2895aeca6a) )
    ROM_LOAD32_BYTE( "prg2.u017",    0x000002, 0x080000, CRC(b89d7693) SHA1(174b2ecfd8a3c593a81905c1c9d62728f710f5d1) )
    ROM_LOAD32_BYTE( "prg3.u015",    0x000003, 0x080000, CRC(6b38a07b) SHA1(2131ae726fc38c8054801c1de4d17eec5b55dd2d) )
	
	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF ) /* text layer roms - none! */

	ROM_REGION( 0x900000, "gfx2", ROMREGION_ERASEFF ) /* background layer roms - none! */

	ROM_REGION( 0x600000, "gfx3", 0)   
	ROM_LOAD("obj1.u0231", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD("obj2.u0233", 0x200000, 0x200000, NO_DUMP )
	ROM_LOAD("obj3.u0232", 0x400000, 0x200000, NO_DUMP )
//  obj4.u0234 empty slot

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "Scandal Blue SKTP-10008", 0, SHA1(17fe67698a9bc5dbd452c4b1afa739294ec2011c) )
ROM_END

ROM_START( emjtrapz )
	ROM_REGION32_LE( 0x200000, "ipl", 0 ) /* i386 program */
    ROM_LOAD32_BYTE( "prg0.u016",    0x000000, 0x080000, CRC(88e4ef2a) SHA1(110451c09983ce4720f75b89282ca49f47169a85) )
    ROM_LOAD32_BYTE( "prg1.u011",    0x000001, 0x080000, CRC(e4716996) SHA1(6abd84c1e4facf6570988db0a63968a1647144b1) )
    ROM_LOAD32_BYTE( "prg2.u017",    0x000002, 0x080000, CRC(69995273) SHA1(a7e10d21a524a286acd0a8c19c41a101eee30626) )
    ROM_LOAD32_BYTE( "prg3.u015",    0x000003, 0x080000, CRC(99f86a19) SHA1(41deb5eb78c0a675da7e1b1bbd5c440e157c7a25) )
	
	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF ) /* text layer roms - none! */

	ROM_REGION( 0x900000, "gfx2", ROMREGION_ERASEFF ) /* background layer roms - none! */

	ROM_REGION( 0x600000, "gfx3", 0)   
	ROM_LOAD("obj1.u0231", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD("obj2.u0233", 0x200000, 0x200000, NO_DUMP )
	ROM_LOAD("obj3.u0232", 0x400000, 0x200000, NO_DUMP )
//  obj4.u0234 empty slot

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "Trap Zone SKTP-00009", 0, SHA1(b4a51f42eeaeefc329031651859caa108418a96e) )
ROM_END

// Gravure Collection
// Pakkun Ball TV
/* 01 */ // Mahjong Shichau zo!
/* 02 */ GAME( 1999, emjjoshi,  0,   seibucats,  seibucats, seibucats_state,  0,       ROT0, "Seibu Kaihatsu / CATS",      "E-Touch Mahjong Series #2: Joshiryou de NE! (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
/* 03 */ // Lingerie DE Ikou
/* 04 */ // Marumie Network
/* 05 */ // BINKAN Lips
/* 06 */ GAME( 2001, emjscanb,  0,   seibucats,  seibucats, seibucats_state,  0,       ROT0, "Seibu Kaihatsu / CATS",      "E-Touch Mahjong Series #6: Scandal Blue - Midara na Daishou (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
/* 07 */ GAME( 2001, emjtrapz,  0,   seibucats,  seibucats, seibucats_state,  0,       ROT0, "Seibu Kaihatsu / CATS",      "E-Touch Mahjong Series #7: Trap Zone - Yokubou no Kaisoku Densha (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
/* 08 */ // Poison
/* 09 */ // Nurse Call
/* 10 */ // Secret Love
/* 11 */ // Venus Shot
/* 12 */ // Platina Selection
/* 13 */ // Gal Jong
/* 14 */ // Yakin Jantou
/* 15 */ // Collector
/* 16 */ // Digicute
/* 17 */ // Gal Jong 2
/* 18 */ // Midnight Lovers
/* 19 */ // Sexual
/* 20 */ // Gekisha!
/* 21 */ // Fetish Navi
/* 22 */ // Venus On Line / Beauty On Line
/* 23 */ // Nurse Mania
/* 24 */ // Sexy Beach
/* 25 */ // Oshioki
/* 26 */ // Private Eyes
/* 27 */ // Gal Jong Kakutou Club
/* 28 */ // BINKAN Lips Plus 
