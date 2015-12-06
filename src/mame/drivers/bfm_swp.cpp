// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Bellfruit SWP (Skill With Prizes) Video hardware
    aka Cobra 3

*/

/* Radio Times readme #1
---------------------------------------------

Music TR9C1710 (Video RAMDAC)

Part number :   STV8438CV
 Manufacture :  ST Microelectronics
Specification(s)
 Description :   8-Bit Triple DAC with Analog Switch
 Package :   PQFP44

MC68340 : Integrated Processor with DMA
 The MC68340 is a high-performance 32-bit integrated processor with direct memory access (DMA), combining an enhanced M68000-compatible processor, 32-bit DMA, and other peripheral subsystems on a single integrated circuit. The MC68340 CPU32 delivers 32-bit CISC processor performance from a lower cost 16-bit memory system. The combination of peripherals offered in the MC68340 can be found in a diverse range of microprocessor-based systems. Systems requiring very high-speed block transfers of data can benefit from the MC68340.


 Block Diagram


MC68340 Features
CPU32 Core
Two-Channel DMA Controller
DUART Compatible with 68681
Two 16-Bit Timer/Counters
Four Chip Selects
System Integration
32 Address Lines, 16 Data Lines
Low Power Management
4.8 MIPS @ 25 MHz
Available in 16 and 25 MHz
Available in 3.3 and 5V


l5380 -scsi controller

ymz280b - in MAME - sound chip

audio board

75585116 - PALCE 16V8H
75585118 - PALCE 16V8H

TI introduce the TMS320AV110 MPEG audio decoder based on TI's 16 bits
DSPs (about $14).

roms

game 95 400 009 type swp left ver. radt 1.1 - ST 27C4001
game 95 400 010 type swp right ver. radt 1.1 - ST 27C4001
95 004 056 sound left - ST 27C4001
95 004 057 sound right - AM 27C040

cd

version 95-100-302
*/

/* Radio Times readme #2
---------------------------------------------

Board
Difficult to make things out on this one, but the SCSI socket is clear to see - this is where the CDROM drive hooks in

CPU
Again, a 68k processor

DSP - Cobra 3

Just a board to decode the audio from the CD video. no ROM's feed it.

GLUE - No idea

Just some logic to sync audio with video with realtime graphics? - Clearly something is needed, as three separate boards are used to process this.

Music - video RAMDAC - have nothing extra to add, this clearly plays the video portion.

SCSI controller - just like in any PC

Sound

All related to the Yamaha YMZ280B-F, a very good music/PCM chip, probably not used to full potential here.

CD-ROM - I'm prepared to put money on Telly Addicts and TOTP being like this, with some minor alterations.

Ordinary ISO standard CDROM with 2048 byte sectors. Can be read on a PC to give MPEG-1 video files (.mpg) at 352x288 pixel resolution, which would probably expand to full screen - some have their own soundtracks, others subtitles. The frame rate of the video is 25fps, and the audio (where available) is 44KHz Stereo, 128Kbps MPEG Layer II (Which is not supported by Sellenoff's Capcom sound driver). Some graphics are stored encoded as .gfx files, whereas clip list data is either stored as an .stl file (listing 'number of clips' and clip name) or in .clp files (same, but larger - different round?)

Final.lis is a list of the questions asked in the end game, where players are asked to match the programme to the channel it is broadcasted on (Very easy to decode, listed in numerical order of channel as channel (space) programme name (not alphabetical, but accessed at random - end marker is blank line, Channel '5' is radio, since there was no Five TV in those days.) (Why am I telling you this - it's practically an emptier.)

Questions are listed by clip, in .QQQ files, encrypted.

.SEQ files are 'sequences' instructions to the video board, in the form of renamed .MPG files, again 352x288, MPEG-1.

TEST.TXT - suggests the content of a prototype version, which was expanded to make this. If the volume name is to be believed I may be related to the person who compiled this (though it's a big clan, highly unlikely ;))

*/


#include "emu.h"
#include "machine/68340.h"
#include "sound/ymz280b.h"

class bfm_swp_state : public driver_device
{
public:
	bfm_swp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	UINT32* m_cpuregion;
	UINT32* m_mainram;

	DECLARE_READ32_MEMBER(bfm_swp_mem_r);
	DECLARE_WRITE32_MEMBER(bfm_swp_mem_w);


	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}

protected:

	// devices
	required_device<m68340cpu_device> m_maincpu;

	virtual void machine_start() override;
};

READ32_MEMBER(bfm_swp_state::bfm_swp_mem_r)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_maincpu, offset * 4);

	switch ( cs )
	{
		case 1:
			if (offset<0x100000/4) return m_cpuregion[offset];

		case 2:
			offset&=0x3fff;
			return m_mainram[offset];

		default:
			logerror("%08x maincpu read access offset %08x mem_mask %08x cs %d\n", pc, offset*4, mem_mask, cs);

	}

	return 0x0000;
}

WRITE32_MEMBER(bfm_swp_state::bfm_swp_mem_w)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_maincpu, offset * 4);

	switch ( cs )
	{
		default:
			logerror("%08x maincpu write access offset %08x data %08x mem_mask %08x cs %d\n", pc, offset*4, data, mem_mask, cs);

		case 2:
			offset&=0x3fff;
			COMBINE_DATA(&m_mainram[offset]);
			break;


	}

}




static ADDRESS_MAP_START( bfm_swp_map, AS_PROGRAM, 32, bfm_swp_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(bfm_swp_mem_r, bfm_swp_mem_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( bfm_swp )
INPUT_PORTS_END


void bfm_swp_state::machine_start()
{
	m_cpuregion = (UINT32*)memregion( "maincpu" )->base();
	m_mainram = (UINT32*)auto_alloc_array_clear(machine(), UINT32, 0x10000);

}


static MACHINE_CONFIG_START( bfm_swp, bfm_swp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(bfm_swp_map)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(bfm_swp_state, screen_update)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymz", YMZ280B, 10000000 )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( c3_rtime )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400009.bin", 0x00001, 0x080000, CRC(a5e0a5ca) SHA1(e7063ddfb436152f15267fde2aa7695c8a262191) )
	ROM_LOAD16_BYTE( "95400010.bin", 0x00000, 0x080000, CRC(03fd5f72) SHA1(379cfc4ef5087f24989bc1f2246b6056e33fd472) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95400063.lhs", 0x00001, 0x080000, CRC(eecb5f3b) SHA1(a1c6ad61a65c5361c38aaae2a064983a978c45ea) )
	ROM_LOAD16_BYTE( "95400064.rhs", 0x00000, 0x080000, CRC(251689f5) SHA1(4589a409c6b0f2869f99a08df8d76223e54d5b3c) )
	ROM_LOAD16_BYTE( "95401063.lhs", 0x00001, 0x080000, CRC(ea98c159) SHA1(6f665d80b71af57b31194fdc981707822e62053e) )
	ROM_LOAD16_BYTE( "95401064.rhs", 0x00000, 0x080000, CRC(bc125897) SHA1(a83fdb54349d3ea5d183754bf4b9fee1f0b73be3) )
	ROM_LOAD16_BYTE( "radtimes.lhs", 0x00001, 0x080000, CRC(c6574297) SHA1(bd9744c4b08f9ae35fe1523ebcd68c52a36a32e0) )
	ROM_LOAD16_BYTE( "radtimes.rhs", 0x00000, 0x080000, CRC(ed2c24f0) SHA1(5f06b2de7e2b2dccee7763ea0938849d67256ff2) )
	ROM_LOAD16_BYTE( "rt017.lhs", 0x00001, 0x080000, CRC(d2272c39) SHA1(f583fe39c153dca2e86e875ca39056a8756e0d2c) )
	ROM_LOAD16_BYTE( "rt018.rhs", 0x00000, 0x080000, CRC(52999d03) SHA1(21d1e9034a26f6f73109e9e83272dcff104993e5) )
	ROM_LOAD16_BYTE( "rtimesp1", 0x00001, 0x080000, CRC(f856d377) SHA1(a9fac7e2188bbd087f70c1c00cbf790bc52d573b) )
	ROM_LOAD16_BYTE( "rtimesp2", 0x00000, 0x080000, CRC(130d0864) SHA1(034d6c4fdec3acd4329d16315aeac43b1f1a5e91) )

	ROM_REGION( 0x1000000, "ymz280b", 0 )
	ROM_LOAD( "95004056.bin", 0x000000, 0x080000, CRC(24e8f9fb) SHA1(0d484a8f368b0f2140f148a1dc84db85a100af38) )
	ROM_LOAD( "95004057.bin", 0x080000, 0x080000, CRC(f73c92d6) SHA1(08c7db2baccb703f99efb81f618719a7789ca564) )

	DISK_REGION( "scsi" ) // CDROM
	DISK_IMAGE_READONLY( "rtimes", 0, SHA1(20accfe236a0c85108cd2a205399ed8959f1a638) )
ROM_END

ROM_START( c3_telly )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95401023.lhs", 0x00001, 0x080000, CRC(85b95b56) SHA1(106e617fc92f95a6b3769db1fd4e5ab47c752c08) )
	ROM_LOAD16_BYTE( "95401024.rhs", 0x00000, 0x080000, CRC(835d25fd) SHA1(6d780332f6016d6e1404922e0ac439a499211be3) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95400021.p1",  0x00001, 0x080000, CRC(5c969746) SHA1(7458c613d7a3e7cf6a21e55f74dcdc052404f29c) )
	ROM_LOAD16_BYTE( "95400022.p2",  0x00000, 0x080000, CRC(fa1fdb7b) SHA1(eff87c197a62dba49d95810e8669026db2edb187) )
	ROM_LOAD16_BYTE( "95400023.lhs", 0x00001, 0x080000, CRC(b79279b8) SHA1(010edf0c299b0b01ab43f52dce540ff0847fb4c5) )
	ROM_LOAD16_BYTE( "95400024.rhs", 0x00000, 0x080000, CRC(835d25fd) SHA1(6d780332f6016d6e1404922e0ac439a499211be3) )
	ROM_LOAD16_BYTE( "95401021.p1",  0x00001, 0x080000, CRC(24a334d3) SHA1(672f16cbd2ddf627213de71024b6fbaa28f526a5) )
	ROM_LOAD16_BYTE( "95401022.p2",  0x00000, 0x080000, CRC(90af3767) SHA1(e529ad7eef5e6d2a6951d46e77aaad2087890445) )
	ROM_LOAD16_BYTE( "tadd13lh",     0x00001, 0x080000, CRC(2d6ed08c) SHA1(efa39b9ff5605c2e29971fb5e874c9a0c178b1f0) )
	ROM_LOAD16_BYTE( "tadd14rh",     0x00000, 0x080000, CRC(26dd6ed6) SHA1(553f29017494b6f7ecc98940d527f498316ea55e) )
	ROM_LOAD16_BYTE( "telad.tl",     0x00001, 0x080000, CRC(e6906027) SHA1(20ca64417ea3795dc26adfea717cb3d724019c34) )
	ROM_LOAD16_BYTE( "telad.tr",     0x00000, 0x080000, CRC(38dbee05) SHA1(ee33cdaa7f817beb49a3cff49a5493a50d8d4504) )
	ROM_LOAD16_BYTE( "tasndl",       0x00001, 0x080000, CRC(3f0b9d2b) SHA1(6db3451c26a3e673204c316403e0bb7127191a1f) )
	ROM_LOAD16_BYTE( "tasndr",       0x00000, 0x080000, CRC(2dd9ebcf) SHA1(4d118d37e18266f82fb2acb37f5fd106e0f25a1f) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "telsndl", 0x0000, 0x080000, CRC(74996fbd) SHA1(90e46130dccf47be1fcfaf549e548cdd4883e59d) )

	DISK_REGION( "scsi" ) // HDD
	DISK_IMAGE_READONLY( "telly", 0, SHA1(6675b8ed6ea47bfb89fbd10a2740e1e0a3faec54) )
ROM_END

ROM_START( c3_totp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400101.lo", 0x00001, 0x080000, CRC(c95164c7) SHA1(7b2fada6a3208666219a53cba08f7acad015763d) )
	ROM_LOAD16_BYTE( "95400102.hi", 0x00000, 0x080000, CRC(5ebba159) SHA1(34bcf48140261cd87d81a32581e965d722f42f71) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95401101.lo", 0x00001, 0x080000, CRC(97d2d90a) SHA1(d4a2afd3cc551986e76f107beb66e8c660a6ee1d) )
	ROM_LOAD16_BYTE( "95401102.hi", 0x00000, 0x080000, CRC(3599427f) SHA1(16d915553b2b490a047888c64ebcf952714b3168) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "totpsnd.lhs", 0x000000, 0x080000, CRC(56a73136) SHA1(10656ede18de9432a8a728cc59d000b5b1bf0150) )
	ROM_LOAD( "totpsnd.rhs", 0x080000, 0x080000, CRC(28d156ab) SHA1(ebf5c4e008015b9b56b3aa5228c05b8e298daa80) )

	DISK_REGION( "scsi" ) // uses a CD, only one we've seen has been damaged to a point where not all data could be read tho
	DISK_IMAGE_READONLY( "cd", 0, NO_DUMP )
ROM_END

ROM_START( c3_ppays )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400687.hi", 0x00000, 0x080000, CRC(56080e1c) SHA1(49391059b5a758690d4972abad04d7e7aef23423) )
	ROM_LOAD16_BYTE( "95400687.lo", 0x00001, 0x080000, CRC(8b2c9c3d) SHA1(921c900447870f6ae51a4f3baeb60ce94e732291) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "phrasesn.l", 0x0000, 0x080000, CRC(a436ccf8) SHA1(18c39aa2e68c32242e0de1347b25d4af44b84548) )

	DISK_REGION( "scsi" ) // CD or HDD
	DISK_IMAGE_READONLY( "cd or hdd", 0, NO_DUMP )
ROM_END




GAME( 199?, c3_rtime        , 0         , bfm_swp, bfm_swp, driver_device, 0, ROT0, "BFM", "Radio Times (Bellfruit) (Cobra 3)", MACHINE_IS_SKELETON )
GAME( 199?, c3_telly        , 0         , bfm_swp, bfm_swp, driver_device, 0, ROT0, "BFM", "Telly Addicts (Bellfruit) (Cobra 3)", MACHINE_IS_SKELETON )
GAME( 199?, c3_totp         , 0         , bfm_swp, bfm_swp, driver_device, 0, ROT0, "BFM", "Top of the Pops (Bellfruit) (Cobra 3?)", MACHINE_IS_SKELETON )
GAME( 199?, c3_ppays        , 0         , bfm_swp, bfm_swp, driver_device, 0, ROT0, "BFM", "The Phrase That Pays (Bellfruit) (Cobra 3?)", MACHINE_IS_SKELETON )
