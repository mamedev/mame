// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Game King board types:


Common name 038 or 3802

P/N 757-038-0x

EPROM sockets only
  Chip locations
   BASE - U8
   GME1 - U21
   GME2 - U5
   PLX1 - U20
   PLX2 - U4
   CG1  - U48
   CG2  - U47
   SND1-SND3 EPROMs on optional sound board



Common name 039 or 3902

P/N 757-039-0x

EPROM sockets and SIMM slots
  Chip locations:
   BASE  - U39
   GAME1 - U13
   GAME2 - U36
   PLX1  - U14
   PLX2  - U37 (games may use PXLF SIMMs instead of EPROMs)
   CG1   - U30
   CG2   - U53 (games may use CGF SIMM instead of EPROMs)
   PXLF Pixel Memory SIMM - SIMM slots J6, J7 & J8
   CGF CG Memory SIMM - SIMM slot J3

  J4 & J5 Two 120-pin sockets to connect MultiMedia Lite sound board
   SND1-SND4 EPROMs on optional MULTIMEDIA LITE 1 board
   SNDF SIMM on optional MULTIMEDIA LITE 2 board

MULTIMEDIA LITE boards:
 Multimedia Lite 1 - uses up to 4MB on EPROMs to store sound
 Multimedia Lite 2 - uses up to 16MB of SIMM to store sound

 Boards contain:
 Custom programmed Cypress CY37032-125JC CPLD
    32 Macrocells
    32 I/O Pins
     5 Dedicated Inputs
  labeled MML1 REV A (socketed) for EPROM type (4 32pin eprom sockets)
  labeled MML2 REV A (surface mounted) for SIMM type (1 72pin SIMM socket)
 16.9344MHz OSC
 Yamaha YMZ280-B sound chip
 1 3.5mm Audio out jack
 P4 & P5 Two 120pin connectors



Common name 044

P/N 757044

No EPROM or SIMM sockets

ONLY J6 & J7 Two 120-pin sockets to connect classic legacy or enhanced
             memory (flash) adapter boards.



GAME KING DELUXE - MEMORY 1
ASSY NO. 7682710

PCB board that connects to 044 boards via J6 & J7
    Adds the abillity to use legacy 038 EPROM based software
    or 039 EPROM + SIMM software

More chips (from eBay auction):
    2x Phillips / NXT 28C94 quad UART (8 serial channels total)
    ADV476 256 color RAMDAC
*/

#include "emu.h"
#include "cpu/i960/i960.h"
#include "machine/nvram.h"
#include "sound/ymz280b.h"
#include "screen.h"
#include "speaker.h"


class igt_gameking_state : public driver_device
{
public:
	igt_gameking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_palette(*this, "palette")
	{ }

	required_device<palette_device> m_palette;

	virtual void video_start() override;
	uint32_t screen_update_igt_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(igt_gk_28010008_r)
	{
		return machine().rand();  // don't quite understand this one
	};

	DECLARE_READ32_MEMBER(igt_gk_28030000_r)
	{
		return machine().rand();
	};

	DECLARE_READ32_MEMBER(uart_status_r);
	DECLARE_WRITE32_MEMBER(uart_w);
	DECLARE_WRITE32_MEMBER(clut_w);
	DECLARE_WRITE32_MEMBER(clut_mask_w);

private:
	int m_offset, m_r, m_g, m_b, m_state;
	bool m_bToggle;
	u8 m_clut_mask;
};

static INPUT_PORTS_START( igt_gameking )
INPUT_PORTS_END

WRITE32_MEMBER(igt_gameking_state::clut_w)
{
	if (mem_mask == 0x000000ff)
	{
		m_offset = data & 0xff;
		m_state = 0;
	}
	else if (mem_mask == 0x00ff0000)
	{
		switch (m_state)
		{
			case 0: m_r = (data>>16) & 0xff; m_state++; break;
			case 1: m_g = (data>>16) & 0xff; m_state++; break;
			case 2:
				m_b = (data>>16) & 0xff;
				//printf("CLUT: color %d = R %d G %d B %d\n", m_offset, m_r, m_g, m_b);
				m_palette->set_pen_color(m_offset, m_r<<18 | m_g<<10 | m_b<<2);
				m_state = 0;
				break;
		}
	}
}

WRITE32_MEMBER(igt_gameking_state::clut_mask_w)
{
	m_clut_mask = data & 0xff;
}

READ32_MEMBER(igt_gameking_state::uart_status_r)
{
	return 0x00040000;
}

WRITE32_MEMBER(igt_gameking_state::uart_w)
{
	printf("%c", (data>>16) & 0x7f);
}

void igt_gameking_state::machine_start()
{
}

void igt_gameking_state::machine_reset()
{
	m_bToggle = false;
	m_offset = m_state = m_r = m_g = m_b = 0;
	m_clut_mask = 0xff;
}

void igt_gameking_state::video_start()
{
}

uint32_t igt_gameking_state::screen_update_igt_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



static ADDRESS_MAP_START( igt_gameking_mem, AS_PROGRAM, 32, igt_gameking_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM
	AM_RANGE(0x08000000, 0x081fffff) AM_ROM AM_REGION("game", 0)

	// it's unclear how much of this is saved and how much total RAM there is.
	AM_RANGE(0x10000000, 0x1001ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x10020000, 0x100fffff) AM_RAM

	AM_RANGE(0x18000000, 0x181fffff) AM_RAM // igtsc writes from 18000000 to 1817ffff, ms3 all the way to 181fffff.

	// 28010000-2801007f: first 28C94 QUART
	AM_RANGE(0x28010008, 0x2801000b) AM_READ(igt_gk_28010008_r)
	AM_RANGE(0x28010030, 0x28010033) AM_READ(uart_status_r) // channel D
	AM_RANGE(0x28010034, 0x28010037) AM_WRITE(uart_w)       // channel D
	// 28020000-2802007f: second 28C94 QUART
	AM_RANGE(0x28030000, 0x28030003) AM_READ(igt_gk_28030000_r)
	AM_RANGE(0x28040000, 0x2804ffff) AM_RAM
	AM_RANGE(0x28050000, 0x28050003) AM_DEVREADWRITE8("ymz", ymz280b_device, read, write, 0x00ff00ff)
	AM_RANGE(0x28060000, 0x28060003) AM_WRITE(clut_w)
	AM_RANGE(0x28060004, 0x28060007) AM_WRITE(clut_mask_w)

	AM_RANGE(0xa1000000, 0xa1011fff) AM_RAM // used by gkkey for restart IAC

ADDRESS_MAP_END

static const gfx_layout igt_gameking_layout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


static GFXDECODE_START( igt_gameking )
	GFXDECODE_ENTRY( "cg", 0, igt_gameking_layout,   0x0, 1  )
GFXDECODE_END



static MACHINE_CONFIG_START( igt_gameking, igt_gameking_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I960, XTAL_24MHz)
	MCFG_CPU_PROGRAM_MAP(igt_gameking_mem)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", igt_gameking)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(igt_gameking_state, screen_update_igt_gameking)
	MCFG_SCREEN_PALETTE("palette")
	// Xilinx used as video chip XTAL_26_66666MHz on board

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz) // enhanced sound on optional Media-Lite sub board
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_1FILL("nvram")
MACHINE_CONFIG_END

ROM_START( ms3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "3b5060ax.u8",  0x000000, 0x080000, CRC(aff8d874) SHA1(1cb972759ee12c944a1cfdbe68848c9b2e64a4d3) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "ea5006ax.u21", 0x000000, 0x080000, CRC(9109b2e2) SHA1(daa1f01315caf6e08c3cf8b0e4925c86d2cd8dc3) )
	ROM_LOAD16_BYTE( "ea5006ax.u5",  0x000001, 0x080000, CRC(66c33cf6) SHA1(600f75ab112348f43b38cafd6f871559372f2807) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "1g5032ax.u48", 0x000000, 0x040000, CRC(aba6002f) SHA1(2ed51aa8bbc1e703cd63f633d745dfa4fa7f3dd0) )
	ROM_LOAD16_BYTE( "1g5032ax.u47", 0x000001, 0x040000, CRC(605a71ec) SHA1(13fe64c611c0903a7b79d8680de3ac81f3226a67) )

	ROM_REGION( 0x200000, "plx", 0 )
	ROM_LOAD16_BYTE( "1g5032ax.u20", 0x000000, 0x100000, CRC(517e7478) SHA1(316a8e48ad6502f9508b06f900555d53ef40b464) )
	ROM_LOAD16_BYTE( "1g5032ax.u4",  0x000001, 0x100000, CRC(e67c878f) SHA1(b03f8d28924351e96bb9f24d32f0e4a40a51910c) )

	ROM_REGION( 0x200000, "snd", 0 )
	ROM_LOAD( "1h5053xx.u6",  0x000000, 0x080000, CRC(6735c65a) SHA1(198cacec5441aa615c0de63a0b4e47265636bcee) )

	ROM_REGION( 0x20000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x020000, CRC(acbbc6d9) SHA1(6e86d24ad3793b41f1f23f80f9bdb22767abc3bf) )
ROM_END

ROM_START( ms72c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "3B5019FA MULTISTAR 7 2c CONFIG.u8", 0x00000, 0x80000, CRC(6c326a31) SHA1(cd8ecc814ef4f379946ab3654dddd508c24ae56c) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "DA5001FA Gamebase GME1.u21", 0x000000, 0x100000, CRC(4cd63b5f) SHA1(440302a6ac844b453573e358b29c64f2e8ece80e) )
	ROM_LOAD16_BYTE( "DA5001FA Gamebase GME2.u5",  0x000001, 0x100000, CRC(663df2fe) SHA1(d2ac3129a346450168a9f76431b0fa8b78db3b37) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "1G5019FA Multistar 7 PUB.u48", 0x000000, 0x80000, CRC(ac50a155) SHA1(50d07ba5ca176c97adde169fda6e6385c8ec8299) )
	ROM_LOAD16_BYTE( "1G5019FA Multistar 7 PUB.u47", 0x000001, 0x80000, CRC(5fee078b) SHA1(a41591d14fbc12c68d773fbd1ac340d9427d68e9) )

	ROM_REGION( 0x200000, "plx", 0 )
	ROM_LOAD16_BYTE( "1G5019FA Multistar 7 PUB.u20", 0x000000, 0x100000, CRC(806ec7d4) SHA1(b9263f942b3d7101797bf87ad18cfddac9582791) )
	ROM_LOAD16_BYTE( "1G5019FA Multistar 7 PUB.u4",  0x000001, 0x100000, CRC(2e1e9c8a) SHA1(b6992f013f43debf43f4704396fc71e88449e365) )

	ROM_REGION( 0x200000, "snd", 0 )
	ROM_LOAD( "1H5008FA Multistar 7.u6", 0x000000, 0x100000, CRC(69656637) SHA1(28c2cf48856ee4f820146fdbd0f3c7e307892dc6) )

	ROM_REGION( 0x20000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x020000, CRC(b5e42dbc) SHA1(f6afadb6877bca2cef40725b001c7918f9c99359) )
ROM_END


ROM_START( gkigt4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "M0000527 BASE (1-4002).bin", 0x00000, 0x80000, CRC(73981260) SHA1(24b42ae2796034815d35294efe0ac3d5c33100bd) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "G0001777 GME1 1 of 2 (2-80).bin", 0x000000, 0x100000, CRC(99d5829d) SHA1(b2ec16f35503ba6a0a41221fb3f52c5d2223ad79) )
	ROM_LOAD16_BYTE( "G0001777 GME2 2 of 2 (2-80).bin", 0x000001, 0x100000, CRC(3b7dfcc0) SHA1(2aeb35125c4320ba3198c44418c90fa6fd6270a9) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "C0000330 CG1 1 of 4 (2-40).bin", 0x000000, 0x80000, CRC(b92b8aa4) SHA1(05a1feac4012a73777eb28ab6e66e1dcadb9430f) )
	ROM_LOAD16_BYTE( "C0000330 CG2 2 of 4 (2-40).bin", 0x000001, 0x80000, CRC(4e0560b5) SHA1(109f0bd47cfb0ed593fc34c5904bc639b0097d12))

	ROM_REGION( 0x200000, "plx", 0 )
	ROM_LOAD16_BYTE( "C0000330 PLX1 3 of 4 (2-80).bin", 0x000000, 0x100000, CRC(806ec7d4) SHA1(b9263f942b3d7101797bf87ad18cfddac9582791) )
	ROM_LOAD16_BYTE( "C0000330 PLX2 4 of 4 (2-80).bin", 0x000001, 0x100000, CRC(c4ce5dc5) SHA1(cc5d090e88551550787b87d80aafe18ee1661dd7) )

	ROM_REGION( 0x200000, "snd", 0 )
	ROM_LOAD( "SWC00046 SND1 1 of 2 (2-80).rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "SWC00046 SND2 2 of 2 (2-80).rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END



ROM_START( gkigt4ms )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "M000526 BASE  (1-4002) MS.u39", 0x00000, 0x80000, CRC(4d095df5) SHA1(bd0cdc4c1b07ef2723ba22b14abaf581b017f190) )

	ROM_REGION32_LE( 0x200000, "game", 0 ) // same as gkigt4
	ROM_LOAD16_BYTE( "G0001777 GME1 1 of 2 (2-80).bin", 0x000000, 0x100000, CRC(99d5829d) SHA1(b2ec16f35503ba6a0a41221fb3f52c5d2223ad79) )
	ROM_LOAD16_BYTE( "G0001777 GME2 2 of 2 (2-80).bin", 0x000001, 0x100000, CRC(3b7dfcc0) SHA1(2aeb35125c4320ba3198c44418c90fa6fd6270a9) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "C000351 CG1 1 of 4 (2-40) MS.u30", 0x000000, 0x80000, CRC(2e841b28) SHA1(492b54e092b0d4028fd8edcb981bd1fd25dca47d) )
	ROM_LOAD16_BYTE( "C000351 CG2 2 of 4 (2-40) MS.u53", 0x000001, 0x80000, CRC(673fc86c) SHA1(4d844330c5602d725253b4f78781fa9e213b8556) )

	ROM_REGION( 0x200000, "plx", 0 )
	ROM_LOAD16_BYTE( "C000351 PXL1 3 of 4 (2-80) MS.u14", 0x000000, 0x100000, CRC(438fb625) SHA1(369c860dffa323c2e9be155da1989252f6b0e694) )
	ROM_LOAD16_BYTE( "C000351 PXL2 4 of 4 (2-80) MS.u37", 0x000001, 0x100000, CRC(22ec9c65) SHA1(bd944ae79faa8ceb73ed8f6f244fce6ff543ccd1) )

	ROM_REGION( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "SWC00046 SND1 1 of 2 (2-80).rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "SWC00046 SND2 2 of 2 (2-80).rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt43 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "M0000837 BASE (1-4002).bin", 0x00000, 0x80000, CRC(98841e5c) SHA1(3b04bc9bc170cfcc6145dc601a63bd1394a62897) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "G0002142 GME1 1 of 2 (2-80).bin", 0x000000, 0x100000, CRC(704ef406) SHA1(3f8f719342874243d479011372786a9b6b14f5b1) )
	ROM_LOAD16_BYTE( "G0002142 GME2 2 of 2 (2-80).bin", 0x000001, 0x100000, CRC(3a576a75) SHA1(d2de1b61808412fb2fe68400387dcdcb7910a770) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "C0000793 CG1 1 of 4 (2-40).bin", 0x000000, 0x80000, CRC(582137cc) SHA1(66686a2332a3844f816cf7e988a346f5f593d8f6) )
	ROM_LOAD16_BYTE( "C0000793 CG2 2 of 4 (2-40).bin", 0x000001, 0x80000, CRC(5e0b6310) SHA1(4bf718dc9859e8c10c9dca967185c57738249319) )

	ROM_REGION( 0x200000, "plx", 0 )
	ROM_LOAD16_BYTE( "C0000793 PLX1 3 of 4 (2-80).bin", 0x000000, 0x100000, CRC(6327a76e) SHA1(01ad5747788389d3d9d71a1c37472d33db3ba5fb) )
	ROM_LOAD16_BYTE( "C0000793 PLX2 4 of 4 (2-80).bin", 0x000001, 0x100000, CRC(5a400e90) SHA1(c01be47d03e9ec418d0e4e1293fcf2c890301430) )

	ROM_REGION( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "SWC00046 SND1 1 of 2 (2-80).rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "SWC00046 SND2 2 of 2 (2-80).rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt43n )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "M0000811 BASE (1-4002) NJ.bin", 0x00000, 0x80000,  CRC(4c659923) SHA1(4624179320cb284516980e2d3caea6fd45c3f967) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "G0001624 GME1 1 of 2 (2-80) NJ.bin", 0x000000, 0x100000, CRC(4aa4139b) SHA1(c3e13c84cc13d44de90a03d0b5d45f46d4f794ce) )
	ROM_LOAD16_BYTE( "G0001624 GME2 2 of 2 (2-80) NJ.bin", 0x000001, 0x100000, CRC(5b3bb8bf) SHA1(271131f06944074bedab7fe7c80fce1e2136c385) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "C0000770 CG1 1 of 4 (2-40) NJ.bin", 0x000000, 0x80000, CRC(35847c45) SHA1(9f6192a9cb43df1a32d13d09248f10d62cd5ad3c) )
	ROM_LOAD16_BYTE( "C0000770 CG2 2 of 4 (2-40) NJ.bin", 0x000001, 0x80000, CRC(2207af01) SHA1(6f59d624fbbae56af081f2a2f4eb3f7a6e6c0ec1) )

	ROM_REGION( 0x200000, "plx", 0 )
	ROM_LOAD16_BYTE( "C0000770 PLX1 3 of 4 (2-80) NJ.bin", 0x000000, 0x100000, CRC(d1e673cd) SHA1(22d0234e3efb5238d60c9aab4ffc171f28f5abac) )
	ROM_LOAD16_BYTE( "C0000770 PLX2 4 of 4 (2-80) NJ.bin", 0x000001, 0x100000, CRC(d99074f3) SHA1(a5829761f558f8e543a1442128c0ae3520d42318) )

	ROM_REGION( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "SWC00046 SND1 1 of 2 (2-80).rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "SWC00046 SND2 2 of 2 (2-80).rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigtez )
	ROM_REGION( 0x80000, "maincpu", 0 ) // same as gkigt4ms
	ROM_LOAD( "M000526 BASE  (1-4002) MS.u39", 0x00000, 0x80000, CRC(4d095df5) SHA1(bd0cdc4c1b07ef2723ba22b14abaf581b017f190) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "G0002955 GME1 1 of 2 (2-80) MS.u13", 0x000000, 0x100000, CRC(472c04a1) SHA1(00b7784d254390475c9aa1beac1700c42514cbed) )
	ROM_LOAD16_BYTE( "G0002955 GME2 2 of 2 (2-80) MS.u36", 0x000001, 0x100000, CRC(16903e65) SHA1(eb01c0f88212e8e35c35f897f17e12e859255270) )

	ROM_REGION( 0x100000, "cg", 0 ) // same as gkigt4ms
	ROM_LOAD16_BYTE( "C000351 CG1 1 of 4 (2-40) MS.u30", 0x000000, 0x80000, CRC(2e841b28) SHA1(492b54e092b0d4028fd8edcb981bd1fd25dca47d) )
	ROM_LOAD16_BYTE( "C000351 CG2 2 of 4 (2-40) MS.u53", 0x000001, 0x80000, CRC(673fc86c) SHA1(4d844330c5602d725253b4f78781fa9e213b8556) )

	ROM_REGION( 0x200000, "plx", 0 ) // same as gkigt4ms
	ROM_LOAD16_BYTE( "C000351 PXL1 3 of 4 (2-80) MS.u14", 0x000000, 0x100000, CRC(438fb625) SHA1(369c860dffa323c2e9be155da1989252f6b0e694) )
	ROM_LOAD16_BYTE( "C000351 PXL2 4 of 4 (2-80) MS.u37", 0x000001, 0x100000, CRC(22ec9c65) SHA1(bd944ae79faa8ceb73ed8f6f244fce6ff543ccd1) )

	ROM_REGION( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "SWC00046 SND1 1 of 2 (2-80).rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "SWC00046 SND2 2 of 2 (2-80).rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt5p )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "M0000761 BASE (1-4002).bin", 0x00000, 0x80000, CRC(efac4e4f) SHA1(0cf5b3eead66a791701a504330d9154e8f4d657d) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "G0001783 GME1 1 of 2 (2-80).bin", 0x000000, 0x100000, CRC(f6672841) SHA1(1f8fe98b931e7fd67e5cd56e193c44acabcb7c0a) )
	ROM_LOAD16_BYTE( "G0001783 GME1 2 of 2 (2-80).bin", 0x000001, 0x100000, CRC(639de8c0) SHA1(ad4fb79f12bf19b4b39691cda9f5e61f32fa2dd5) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "C0000517 CG1 1 of 4 (2-40).bin", 0x000000, 0x80000, CRC(26db44c9) SHA1(8afe145d1fb7535c651d78b23872b71c2c946509) )
	ROM_LOAD16_BYTE( "C0000517 CG2 2 of 4 (2-40).bin", 0x000001, 0x80000, CRC(3554ba38) SHA1(6e0b8506943559dbee4cfa7c9e4b60590c6529fb) )

	ROM_REGION( 0x200000, "plx", 0 )
	ROM_LOAD16_BYTE( "C0000517 PLX1 3 of 4 (2-80).bin", 0x000000, 0x100000, CRC(956ba40c) SHA1(7d8ae934ef663ea6b3f342455d1e8c70a1ca4581) )
	ROM_LOAD16_BYTE( "C0000517 PLX2 4 of 4 (2-80).bin", 0x000001, 0x100000, CRC(dff43975) SHA1(e1ca212e4e51175bcbab2af447863605f74ba77f) )

	ROM_REGION( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "SWC00046 SND1 1 of 2 (2-80).rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "SWC00046 SND2 2 of 2 (2-80).rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END


ROM_START( igtsc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "I0000838 BASE (1-4002).bin", 0x00000, 0x80000, CRC(7b66f0d5) SHA1(a13e7fa4062668ff7acb15e58025eeb401754898) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "G0001175 GME1 1 of 2 (2-80).bin", 0x000000, 0x100000, CRC(674e0172) SHA1(e7bfe13781988b9193f22ad93502e303ba9427eb) )
	ROM_LOAD16_BYTE( "G0001175 GME2 2 of 2 (2-80).bin", 0x000001, 0x100000, CRC(db76db22) SHA1(e389b11a05f0ef0dcee303ba91578f4cd56beba0) )

	// all these SIMM files are bad dumps, they never contains the byte value 0x0d (uploaded in ASCII mode with carriage return stripped out?)
	ROM_REGION( 0x0800000, "cg", 0 )
	// uses a SIMM
	ROM_LOAD( "C0000464 CGF.bin", 0x000000, 0x07ff9a3, BAD_DUMP CRC(52fcc9fd) SHA1(98089dcf550bc3670d29b7ee78e014154e672120) ) // should be 0x800000

	ROM_REGION( 0x1000000, "plx", 0 )
	// uses a SIMM
	ROM_LOAD( "C000464 PXL3.bin", 0x000000, 0xff73bb, BAD_DUMP CRC(c6acb3cf) SHA1(0ea2d2a506be43a2a8b9d05d80f765c8351494a2) ) // should be 0x1000000

	ROM_REGION( 0x1000000, "snd", 0 )
	// uses a SIMM
	ROM_LOAD( "DSS00076.simm", 0x000000, 0xfd7f81, BAD_DUMP CRC(5dd889b4) SHA1(9a6cb7599d268d110645ac8fe5d41a733cbaadc5) ) // should be 0x1000000
ROM_END


ROM_START( gkkey )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "KEY00017 (1-4002).bin", 0x00000, 0x80000, CRC(1579739f) SHA1(7b6257d17f74599a4ada3014d02a2e7c6686ab3f) )
	ROM_LOAD( "KEY00028 (1-4002).bin", 0x00000, 0x80000, CRC(bf06b98b) SHA1(5c46afb560bb5c0f7540b714c0dea851c6b18fe6) )

	ROM_REGION( 0x80000, "miscbad", 0 )
	// these are also bad dumps, again they never contains the byte value 0x0d (uploaded in ASCII mode with carriage return stripped out?)
	ROM_LOAD( "KEY00022 (1-4002).bin", 0x00000, 0x07feb9, BAD_DUMP CRC(c8149320) SHA1(bd0c62edb154e22949eba776d66c4c1a6c032d31) ) // should be 0x80000
	ROM_LOAD( "KEY00016 (1-4002).bin", 0x00000, 0x07ff9a, BAD_DUMP CRC(80c0c2c4) SHA1(e8df4e516c058aeacf1492151c38b5e73f161c8c) ) // ^
	ROM_LOAD( "KEY00040 (1-4002).bin", 0x00000, 0x07feb9, BAD_DUMP CRC(bdcb3694) SHA1(d7acf0e7620a388c10ceaec4a63b8411419a4f3f) ) // ^

	ROM_REGION32_LE( 0x200000, "game", ROMREGION_ERASEFF )
	ROM_REGION( 0x100000, "cg", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "plx", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "snd", ROMREGION_ERASEFF )
ROM_END

GAME( 1994, ms3,      0,            igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Multistar 3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1994, ms72c,    0,            igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Multistar 7 2c", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt4,   0,            igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Game King (v4.x)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt4ms, gkigt4,       igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Game King (v4.x, MS)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt43,  gkigt4,       igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Game King (v4.3)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt43n, gkigt4,       igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Game King (v4.3, NJ)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigtez,  gkigt4,       igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Game King (EZ Pay, v4.0, MS)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt5p,  gkigt4,       igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Game King (Triple-Five Play)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, igtsc,    0,            igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Super Cherry", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // SIMM dumps are bad.
GAME( 2003, gkkey,    0,            igt_gameking, igt_gameking, driver_device,  0, ROT0, "IGT", "Game King (Set Chips)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // only 2 are good dumps
