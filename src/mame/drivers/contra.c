/***************************************************************************

Contra/Gryzor (c) 1987 Konami

Notes:
    Press 1P and 2P together to advance through tests.

Credits:
    Carlos A. Lozano: CPU emulation
    Phil Stroffolino: video driver
    Jose Tejada Gomez (of Grytra fame) for precious information on sprites
    Eric Hustvedt: palette optimizations and cocktail support

2008-07
Dip locations and factory settings verified with manual

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "konamipt.h"

extern UINT8 *contra_fg_vram,*contra_fg_cram;
extern UINT8 *contra_bg_vram,*contra_bg_cram;
extern UINT8 *contra_text_vram,*contra_text_cram;

PALETTE_INIT( contra );

WRITE8_HANDLER( contra_fg_vram_w );
WRITE8_HANDLER( contra_fg_cram_w );
WRITE8_HANDLER( contra_bg_vram_w );
WRITE8_HANDLER( contra_bg_cram_w );
WRITE8_HANDLER( contra_text_vram_w );
WRITE8_HANDLER( contra_text_cram_w );

WRITE8_HANDLER( contra_K007121_ctrl_0_w );
WRITE8_HANDLER( contra_K007121_ctrl_1_w );
VIDEO_UPDATE( contra );
VIDEO_START( contra );


static WRITE8_HANDLER( contra_bankswitch_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(space->machine, "maincpu");


	bankaddress = 0x10000 + (data & 0x0f) * 0x2000;
	if (bankaddress < 0x28000)	/* for safety */
		memory_set_bankptr(space->machine, 1,&RAM[bankaddress]);
}

static WRITE8_HANDLER( contra_sh_irqtrigger_w )
{
	cputag_set_input_line(space->machine, "audiocpu", M6809_IRQ_LINE, HOLD_LINE);
}

static WRITE8_HANDLER( contra_coin_counter_w )
{
	if (data & 0x01) coin_counter_w(0,data & 0x01);
	if (data & 0x02) coin_counter_w(1,(data & 0x02) >> 1);
}

static WRITE8_HANDLER( cpu_sound_command_w )
{
	soundlatch_w(space,offset,data);
}



static ADDRESS_MAP_START( contra_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0007) AM_WRITE(contra_K007121_ctrl_0_w)
	AM_RANGE(0x0010, 0x0010) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0011, 0x0011) AM_READ_PORT("P1")
	AM_RANGE(0x0012, 0x0012) AM_READ_PORT("P2")

	AM_RANGE(0x0014, 0x0014) AM_READ_PORT("DSW1")
	AM_RANGE(0x0015, 0x0015) AM_READ_PORT("DSW2")
	AM_RANGE(0x0016, 0x0016) AM_READ_PORT("DSW3")

	AM_RANGE(0x0018, 0x0018) AM_WRITE(contra_coin_counter_w)
	AM_RANGE(0x001a, 0x001a) AM_WRITE(contra_sh_irqtrigger_w)
	AM_RANGE(0x001c, 0x001c) AM_WRITE(cpu_sound_command_w)
	AM_RANGE(0x001e, 0x001e) AM_WRITENOP	/* ? */
	AM_RANGE(0x0060, 0x0067) AM_WRITE(contra_K007121_ctrl_1_w)

	AM_RANGE(0x0c00, 0x0cff) AM_RAM AM_BASE(&paletteram)

	AM_RANGE(0x1000, 0x1fff) AM_RAM

	AM_RANGE(0x2000, 0x5fff) AM_READ(SMH_RAM)
	AM_RANGE(0x2000, 0x23ff) AM_WRITE(contra_fg_cram_w) AM_BASE(&contra_fg_cram)
	AM_RANGE(0x2400, 0x27ff) AM_WRITE(contra_fg_vram_w) AM_BASE(&contra_fg_vram)
	AM_RANGE(0x2800, 0x2bff) AM_WRITE(contra_text_cram_w) AM_BASE(&contra_text_cram)
	AM_RANGE(0x2c00, 0x2fff) AM_WRITE(contra_text_vram_w) AM_BASE(&contra_text_vram)
	AM_RANGE(0x3000, 0x37ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram)/* 2nd bank is at 0x5000 */
	AM_RANGE(0x3800, 0x3fff) AM_WRITE(SMH_RAM) // second sprite buffer
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(contra_bg_cram_w) AM_BASE(&contra_bg_cram)
	AM_RANGE(0x4400, 0x47ff) AM_WRITE(contra_bg_vram_w) AM_BASE(&contra_bg_vram)
	AM_RANGE(0x4800, 0x5fff) AM_WRITE(SMH_RAM)

	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK(1)
 	AM_RANGE(0x7000, 0x7000) AM_WRITE(contra_bankswitch_w)

	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_READ(soundlatch_r)
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITENOP /* read triggers irq reset and latch read (in the hardware only). */
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( contra )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:3")	/* Not Used according to manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 70000" )
	PORT_DIPSETTING(    0x10, "40000 80000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" ) PORT_DIPLOCATION("SW3:2")	/* Not Used according to manual */
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Sound" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Stereo ) )
INPUT_PORTS_END



static const gfx_layout gfxlayout =
{
	8,8,
	0x4000,
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( contra )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout,       0, 8*16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout, 8*16*16, 8*16 )
GFXDECODE_END



static MACHINE_DRIVER_START( contra )

	/* basic machine hardware */
 	MDRV_CPU_ADD("maincpu", M6809, 1500000)
	MDRV_CPU_PROGRAM_MAP(contra_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

 	MDRV_CPU_ADD("audiocpu", M6809, 2000000)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_QUANTUM_TIME(HZ(600))	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(37*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 35*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(contra)
	MDRV_PALETTE_LENGTH(2*8*16*16)

	MDRV_PALETTE_INIT(contra)
	MDRV_VIDEO_START(contra)
	MDRV_VIDEO_UPDATE(contra)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3582071)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.60)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.60)
MACHINE_DRIVER_END


ROM_START( contra )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "633m03.18a",   0x20000, 0x08000, CRC(d045e1da) SHA1(ec781e98a6efb14861223250c6239b06ec98ed0b) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "633i02.17a",   0x10000, 0x10000, CRC(b2f7bd9a) SHA1(6c29568419bc49f0be3995b0c34edd9038f6f8d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for SOUND code */
	ROM_LOAD( "633e01.12a",   0x08000, 0x08000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )	/* 007121 #0 sprite lookup table */
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #0 char lookup table */
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )	/* 007121 #1 sprite lookup table */
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #1 char lookup table */
ROM_END

ROM_START( contra1 )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "633e03.18a",   0x20000, 0x08000, CRC(7fc0d8cf) SHA1(cf1cf15646a4e5dc72671e957bc51ca44d30995c) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "633i02.17a",   0x10000, 0x10000, CRC(b2f7bd9a) SHA1(6c29568419bc49f0be3995b0c34edd9038f6f8d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for SOUND code */
	ROM_LOAD( "633e01.12a",   0x08000, 0x08000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )	/* 007121 #0 sprite lookup table */
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #0 char lookup table */
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )	/* 007121 #1 sprite lookup table */
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #1 char lookup table */
ROM_END

ROM_START( contrab )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "633m03.18a",   0x20000, 0x08000, CRC(d045e1da) SHA1(ec781e98a6efb14861223250c6239b06ec98ed0b) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "633i02.17a",   0x10000, 0x10000, CRC(b2f7bd9a) SHA1(6c29568419bc49f0be3995b0c34edd9038f6f8d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for SOUND code */
	ROM_LOAD( "633e01.12a",   0x08000, 0x08000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* bootleg versions use smaller gfx ROMs, but the data is the same */
	ROM_LOAD( "g-7.rom",      0x00000, 0x10000, CRC(57f467d2) SHA1(e30be315980f421143d1357174af678362836285) )
	ROM_LOAD( "g-10.rom",     0x10000, 0x10000, CRC(e6db9685) SHA1(4d5ccfe95b082fe9830e7a316f88fd6f02464900) )
	ROM_LOAD( "g-9.rom",      0x20000, 0x10000, CRC(875c61de) SHA1(e8dc42fef810a9f5471d96cb5297eb29296ba472) )
	ROM_LOAD( "g-8.rom",      0x30000, 0x10000, CRC(642765d6) SHA1(d1563a392b8d8409f0f2159c2e82cd34b9ca2900) )
	ROM_LOAD( "g-15.rom",     0x40000, 0x10000, CRC(daa2324b) SHA1(8a5fb8b79957291dc952e19e6973c64bb7230816) )
	ROM_LOAD( "g-16.rom",     0x50000, 0x10000, CRC(e27cc835) SHA1(cb980b1fed110c7e4ef21fa11f44e5aea100881b) )
	ROM_LOAD( "g-17.rom",     0x60000, 0x10000, CRC(ce4330b9) SHA1(0a2bd31baa0bc5e3745ee5ddac995557a551d58c) )
	ROM_LOAD( "g-18.rom",     0x70000, 0x10000, CRC(1571ce42) SHA1(04082ed78b5e7f20b99d6edfb6c363574abd6158) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	/* bootleg versions use smaller gfx ROMs, but the data is the same */
	ROM_LOAD( "g-4.rom",      0x00000, 0x10000, CRC(2cc7e52c) SHA1(7598a63346bf06dd34fd643fdff53fc3de6768a6) )
	ROM_LOAD( "g-5.rom",      0x10000, 0x10000, CRC(e01a5b9c) SHA1(58c99cf99f209c584da757320a2f107244056d4c) )
	ROM_LOAD( "g-6.rom",      0x20000, 0x10000, CRC(aeea6744) SHA1(220b42f707db99967bdcbd9ac66fcc83675a72aa) )
	ROM_LOAD( "g-14.rom",     0x30000, 0x10000, CRC(765afdc7) SHA1(b7f6871cb154ee7e42e683bce08b73b00e61b0bc) )
	ROM_LOAD( "g-11.rom",     0x40000, 0x10000, CRC(bd9ba92c) SHA1(e7f65ed20cd7754cc476e8fab7e56105cedcdb98) )
	ROM_LOAD( "g-12.rom",     0x50000, 0x10000, CRC(d0be7ec2) SHA1(5aa829b8ffbe3f5f92ba672b1c24bfb7836ba1a3) )
	ROM_LOAD( "g-13.rom",     0x60000, 0x10000, CRC(2b513d12) SHA1(152ebd849751cc2e95513134ce773a6b2eeb320e) )
	/* This last section, 0x70000-0x7ffff is empty */

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )	/* 007121 #0 sprite lookup table */
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #0 char lookup table */
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )	/* 007121 #1 sprite lookup table */
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #1 char lookup table */
	ROM_LOAD( "conprom.53",   0x0400, 0x0100, CRC(05a1da7e) SHA1(ec0bdfc9da05c99e6a283014769db6d641f1a0aa) )	/* unknown (only present in this bootleg) */
ROM_END

ROM_START( contraj )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "633n03.18a",   0x20000, 0x08000, CRC(fedab568) SHA1(7fd4546335bdeef7f8326d4cbde7fa36d74e5cfc) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "633k02.17a",   0x10000, 0x10000, CRC(5d5f7438) SHA1(489fe56ca57ef4f6a7792fba07a9656009f3f285) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for SOUND code */
	ROM_LOAD( "633e01.12a",   0x08000, 0x08000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )	/* 007121 #0 sprite lookup table */
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #0 char lookup table */
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )	/* 007121 #1 sprite lookup table */
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #1 char lookup table */
ROM_END

ROM_START( contrajb )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "g-2.18a",      0x20000, 0x08000, CRC(bdb9196d) SHA1(fad170e8fda94c9c9d7b82433daa30b80af12efc) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "633k02.17a",   0x10000, 0x10000, CRC(5d5f7438) SHA1(489fe56ca57ef4f6a7792fba07a9656009f3f285) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for SOUND code */
	ROM_LOAD( "633e01.12a",   0x08000, 0x08000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* bootleg versions use smaller gfx ROMs, but the data is the same */
	ROM_LOAD( "g-7.rom",      0x00000, 0x10000, CRC(57f467d2) SHA1(e30be315980f421143d1357174af678362836285) )
	ROM_LOAD( "g-10.rom",     0x10000, 0x10000, CRC(e6db9685) SHA1(4d5ccfe95b082fe9830e7a316f88fd6f02464900) )
	ROM_LOAD( "g-9.rom",      0x20000, 0x10000, CRC(875c61de) SHA1(e8dc42fef810a9f5471d96cb5297eb29296ba472) )
	ROM_LOAD( "g-8.rom",      0x30000, 0x10000, CRC(642765d6) SHA1(d1563a392b8d8409f0f2159c2e82cd34b9ca2900) )
	ROM_LOAD( "g-15.rom",     0x40000, 0x10000, CRC(daa2324b) SHA1(8a5fb8b79957291dc952e19e6973c64bb7230816) )
	ROM_LOAD( "g-16.rom",     0x50000, 0x10000, CRC(e27cc835) SHA1(cb980b1fed110c7e4ef21fa11f44e5aea100881b) )
	ROM_LOAD( "g-17.rom",     0x60000, 0x10000, CRC(ce4330b9) SHA1(0a2bd31baa0bc5e3745ee5ddac995557a551d58c) )
	ROM_LOAD( "g-18.rom",     0x70000, 0x10000, CRC(1571ce42) SHA1(04082ed78b5e7f20b99d6edfb6c363574abd6158) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	/* bootleg versions use smaller gfx ROMs, but the data is the same */
	ROM_LOAD( "g-4.rom",      0x00000, 0x10000, CRC(2cc7e52c) SHA1(7598a63346bf06dd34fd643fdff53fc3de6768a6) )
	ROM_LOAD( "g-5.rom",      0x10000, 0x10000, CRC(e01a5b9c) SHA1(58c99cf99f209c584da757320a2f107244056d4c) )
	ROM_LOAD( "g-6.rom",      0x20000, 0x10000, CRC(aeea6744) SHA1(220b42f707db99967bdcbd9ac66fcc83675a72aa) )
	ROM_LOAD( "g-14.rom",     0x30000, 0x10000, CRC(765afdc7) SHA1(b7f6871cb154ee7e42e683bce08b73b00e61b0bc) )
	ROM_LOAD( "g-11.rom",     0x40000, 0x10000, CRC(bd9ba92c) SHA1(e7f65ed20cd7754cc476e8fab7e56105cedcdb98) )
	ROM_LOAD( "g-12.rom",     0x50000, 0x10000, CRC(d0be7ec2) SHA1(5aa829b8ffbe3f5f92ba672b1c24bfb7836ba1a3) )
	ROM_LOAD( "g-13.rom",     0x60000, 0x10000, CRC(2b513d12) SHA1(152ebd849751cc2e95513134ce773a6b2eeb320e) )
	/* This last section, 0x70000-0x7ffff is empty */


	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )	/* 007121 #0 sprite lookup table */
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #0 char lookup table */
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )	/* 007121 #1 sprite lookup table */
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #1 char lookup table */
ROM_END

ROM_START( gryzor )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "g2.18a",       0x20000, 0x08000, CRC(92ca77bd) SHA1(3a56f51a617edff9f2a60df0141dff040881b82a) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "g3.17a",       0x10000, 0x10000, CRC(bbd9e95e) SHA1(fd5de1bcc485de7b8fc2e321351c2e3ddd25d053) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for SOUND code */
	ROM_LOAD( "633e01.12a",   0x08000, 0x08000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )	/* 007121 #0 sprite lookup table */
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #0 char lookup table */
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )	/* 007121 #1 sprite lookup table */
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #1 char lookup table */
ROM_END

ROM_START( gryzora )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "633j03.18a",   0x20000, 0x08000, CRC(20919162) SHA1(2f375166428ee03f6e8ac0372a373bb8ab35e64c) )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_LOAD( "633j02.17a",   0x10000, 0x10000, CRC(b5922f9a) SHA1(441a23dc99a908ec2c09c855e73070dbab8c5ae2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for SOUND code */
	ROM_LOAD( "633e01.12a",   0x08000, 0x08000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )	/* 007121 #0 sprite lookup table */
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #0 char lookup table */
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )	/* 007121 #1 sprite lookup table */
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )	/* 007121 #1 char lookup table */
ROM_END



GAME( 1987, contra,   0,      contra, contra, 0, ROT90, "Konami", "Contra (US, Set 1)", 0 )
GAME( 1987, contra1,  contra, contra, contra, 0, ROT90, "Konami", "Contra (US, Set 2)", 0 )
GAME( 1987, contrab,  contra, contra, contra, 0, ROT90, "bootleg", "Contra (bootleg)", 0 )
GAME( 1987, contraj,  contra, contra, contra, 0, ROT90, "Konami", "Contra (Japan)", 0 )
GAME( 1987, contrajb, contra, contra, contra, 0, ROT90, "bootleg", "Contra (Japan bootleg)", 0 )
GAME( 1987, gryzor,   contra, contra, contra, 0, ROT90, "Konami", "Gryzor (Set 1)", 0 )
GAME( 1987, gryzora,  contra, contra, contra, 0, ROT90, "Konami", "Gryzor (Set 2)", 0 )
