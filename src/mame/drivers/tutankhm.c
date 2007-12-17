/***************************************************************************

Tutankham :  memory map (preliminary)

driver by Mirko Buffoni

I include here the document based on Rob Jarrett's research because it's
really exaustive.



Tutankham Emu Info
------------------

By Rob Jarrett
robj@astound.com (until June 20, 1997)
or robncait@inforamp.net

Special thanks go to Pete Custerson for the schematics!!


I've recently been working on an emulator for Tutankham. Unfortunately,
time and resources are not on my side so I'd like to provide anyone with
the technical information I've gathered so far, that way someone can
finish the project.

First of all, I'd like to say that I've had no prior experience in
writing an emulator, and my hardware knowledge is weak. I've managed to
find out a fair amount by looking at the schematics of the game and the
disassembled ROMs. Using the USim C++ 6809 core I have the game sort of
up and running, albeit in a pathetic state. It's not playable, and
crashes after a short amount of time. I don't feel the source code is
worth releasing because of the bad design; I was using it as a testing
bed and anticipated rewriting everything in the future.

Here's all the info I know about Tutankham:

Processor: 6809
Sound: Z80 slave w/2 AY3910 sound chips
Graphics: Bitmapped display, no sprites (!)
Memory Map:

Address     R/W Bits        Function
------------------------------------------------------------------------------------------------------
$0000-$7fff             Video RAM
                    - Screen is stored sideways, 256x256 pixels
                    - 1 byte=2 pixels
        R/W aaaaxxxx    - leftmost pixel palette index
        R/W xxxxbbbb    - rightmost pixel palette index
                    - **** not correct **** Looks like some of this memory is for I/O state, (I think < $0100)
                      so you might want to blit from $0100-$7fff

$8000-$800f R/W     aaaaaaaa    Palette colors
                    - Don't know how to decode them into RGB values

$8100       W           Not sure
                    - Video chip function of some sort
                    ( split screen y pan position -- TT )

$8120       R           Not sure
                    - Read from quite frequently
                    - Some sort of video or interrupt thing?
                    - Or a random number seed?
                    ( watchdog reset -- NS )

$8160                   Dip Switch 2
                    - Inverted bits (ie. 1=off)
        R   xxxxxxxa    DSWI1
        R
        R           .
        R           .
        R           .
        R
        R
        R   axxxxxxx    DSWI8

$8180                   I/O: Coin slots, service, 1P/2P buttons
        R

$81a0                   Player 1 I/O
        R

$81c0                   Player 2 I/O
        R

$81e0                   Dip Switch 1
                    - Inverted bits
        R   xxxxxxxa    DSWI1
        R
        R           .
        R           .
        R           .
        R
        R
        R   axxxxxxx    DSWI8

$8200                   IST on schematics
                    - Enable/disable IRQ
        R/W xxxxxxxa    - a=1 IRQ can be fired, a=0 IRQ can't be fired

$8202                   OUT2 (Coin counter)
        W   xxxxxxxa    - Increment coin counter

$8203                   OUT1 (Coin counter)
        W   xxxxxxxa    - Increment coin counter

$8204                   Not sure - 401 on schematics
        W

$8205                   MUT on schematics
        R/W xxxxxxxa    - Sound amplification on/off?

$8206                   HFF on schematics
        W           - Don't know what it does
                    ( horizontal screen flip -- NS )

$8207                   Not sure - can't resolve on schematics
        W
                    ( vertical screen flip -- NS )

$8300                   Graphics bank select
        W   xxxxxaaa    - Selects graphics ROM 0-11 that appears at $9000-9fff
                    - But wait! There's only 9 ROMs not 12! I think the PCB allows 12
                      ROMs for patches/mods to the game. Just make 9-11 return 0's

$8600       W           SON on schematics
                    ( trigger interrupt on audio CPU -- NS )
$8608       R/W         SON on schematics
                    - Sound on/off? i.e. Run/halt Z80 sound CPU?

$8700       W   aaaaaaaa    SDA on schematics
                    - Sound data? Maybe Z80 polls here and plays the appropriate sound?
                    - If so, easy to trigger samples here

$8800-$8fff             RAM
        R/W         - Memory for the program ROMs

$9000-$9fff             Graphics ROMs ra1_1i.cpu - ra1_9i.cpu
        R   aaaaaaaa    - See address $8300 for usage

$a000-$afff             ROM ra1_1h.cpu
        R   aaaaaaaa    - 6809 Code

$b000-$bfff             ROM ra1_2h.cpu
        R   aaaaaaaa    - 6809 Code

$c000-$cfff             ROM ra1_3h.cpu
        R   aaaaaaaa    - 6809 Code

$d000-$dfff             ROM ra1_4h.cpu
        R   aaaaaaaa    - 6809 Code

$e000-$efff             ROM ra1_5h.cpu
        R   aaaaaaaa    - 6809 Code

$f000-$ffff             ROM ra1_6h.cpu
        R   aaaaaaaa    - 6809 Code

Programming notes:

I found that generating an IRQ every 4096 instructions seemed to kinda work. Again, I know
little about emu writing and I think some fooling with this number might be needed.

Sorry I didn't supply the DSW and I/O bits, this info is available elsewhere on the net, I
think at tant or something. I just couldn't remember what they were at this writing!!

If there are any questions at all, please feel free to email me at robj@astound.com (until
June 20, 1997) or robncait@inforamp.net.


BTW, this information is completely free - do as you wish with it. I'm not even sure if it's
correct! (Most of it seems to be). Giving me some credit if credit is due would be nice,
and please let me know about your emulator if you release it.


Sound board: uses the same board as Pooyan.

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "audio/timeplt.h"



extern UINT8 *tutankhm_videoram;
extern size_t tutankhm_videoram_size;
extern UINT8 *tutankhm_paletteram;
extern UINT8 *tutankhm_scroll;

WRITE8_HANDLER( tutankhm_flip_screen_x_w );
WRITE8_HANDLER( tutankhm_flip_screen_y_w );
VIDEO_UPDATE( tutankhm );


static WRITE8_HANDLER( tutankhm_bankselect_w )
{
	offs_t bankaddress;
	UINT8 *RAM = memory_region(REGION_CPU1);

	bankaddress = 0x10000 + (data & 0x0f) * 0x1000;
	memory_set_bankptr(1,&RAM[bankaddress]);
}


static WRITE8_HANDLER( tutankhm_coin_counter_w )
{
	coin_counter_w(offset ^ 1, data);
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_BASE(&tutankhm_videoram) AM_SIZE(&tutankhm_videoram_size)
	AM_RANGE(0x8000, 0x800f) AM_RAM AM_BASE(&tutankhm_paletteram)
	AM_RANGE(0x8100, 0x8100) AM_RAM AM_BASE(&tutankhm_scroll)
	AM_RANGE(0x8120, 0x8120) AM_READ(watchdog_reset_r)
	AM_RANGE(0x8160, 0x8160) AM_READ(input_port_0_r)	/* DSW2 (inverted bits) */
	AM_RANGE(0x8180, 0x8180) AM_READ(input_port_1_r)	/* IN0 I/O: Coin slots, service, 1P/2P buttons */
	AM_RANGE(0x81a0, 0x81a0) AM_READ(input_port_2_r)	/* IN1: Player 1 I/O */
	AM_RANGE(0x81c0, 0x81c0) AM_READ(input_port_3_r)	/* IN2: Player 2 I/O */
	AM_RANGE(0x81e0, 0x81e0) AM_READ(input_port_4_r)	/* DSW1 (inverted bits) */
	AM_RANGE(0x8200, 0x8200) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x8202, 0x8203) AM_WRITE(tutankhm_coin_counter_w)
	AM_RANGE(0x8205, 0x8205) AM_WRITE(MWA8_NOP)	/* ??? */
	AM_RANGE(0x8206, 0x8206) AM_WRITE(tutankhm_flip_screen_x_w)
	AM_RANGE(0x8207, 0x8207) AM_WRITE(tutankhm_flip_screen_y_w)
	AM_RANGE(0x8300, 0x8300) AM_WRITE(tutankhm_bankselect_w)
	AM_RANGE(0x8600, 0x8600) AM_WRITE(timeplt_sh_irqtrigger_w)
	AM_RANGE(0x8700, 0x8700) AM_WRITE(soundlatch_w)
	AM_RANGE(0x8800, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9fff) AM_READ(MRA8_BANK1)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( tutankhm )
	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "256 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Flash Bomb" )
	PORT_DIPSETTING(    0x40, "1 per Life" )
	PORT_DIPSETTING(    0x00, "1 per Game" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Disabled" )
/* 0x00 not commented out since the game makes the usual sound if you insert the coin */
INPUT_PORTS_END



static MACHINE_DRIVER_START( tutankhm )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6809, 1500000)			/* 1.5 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80,14318180/8)
	/* audio CPU */	/* 1.789772727 MHz */						\
	MDRV_CPU_PROGRAM_MAP(timeplt_sound_readmem,timeplt_sound_writemem)

	MDRV_SCREEN_REFRESH_RATE(30)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_30HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)	/* not sure about the visible area */

	MDRV_VIDEO_UPDATE(tutankhm)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 14318180/8)
	MDRV_SOUND_CONFIG(timeplt_ay8910_interface)
	MDRV_SOUND_ROUTE(0, "filter.0.0", 0.60)
	MDRV_SOUND_ROUTE(1, "filter.0.1", 0.60)
	MDRV_SOUND_ROUTE(2, "filter.0.2", 0.60)

	MDRV_SOUND_ADD(AY8910, 14318180/8)
	MDRV_SOUND_ROUTE(0, "filter.1.0", 0.60)
	MDRV_SOUND_ROUTE(1, "filter.1.1", 0.60)
	MDRV_SOUND_ROUTE(2, "filter.1.2", 0.60)

	MDRV_SOUND_ADD_TAG("filter.0.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.0.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.0.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD_TAG("filter.1.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.1.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD_TAG("filter.1.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( tutankhm )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )      /* 64k for M6809 CPU code + 64k for ROM banks */
	ROM_LOAD( "h1.bin",       0x0a000, 0x1000, CRC(da18679f) SHA1(8d2a3665db937d0e1d19300ae22277d9db61fcbc) ) /* program ROMs */
	ROM_LOAD( "h2.bin",       0x0b000, 0x1000, CRC(a0f02c85) SHA1(29a78b3ffd6b597772953543b02dd59acf5af38c) )
	ROM_LOAD( "h3.bin",       0x0c000, 0x1000, CRC(ea03a1ab) SHA1(27a3cca0595bac642caaf9ee2f276814442c8721) )
	ROM_LOAD( "h4.bin",       0x0d000, 0x1000, CRC(bd06fad0) SHA1(bd10bbb413d8dd362072522e902575d819fa8336) )
	ROM_LOAD( "h5.bin",       0x0e000, 0x1000, CRC(bf9fd9b0) SHA1(458ea2ff5eedaaa02e32444dd6004d2eaadbdeab) )
	ROM_LOAD( "h6.bin",       0x0f000, 0x1000, CRC(fe079c5b) SHA1(0757490aaa1cea4f4bbe1230d811a0d917f59e52) )
	ROM_LOAD( "j1.bin",       0x10000, 0x1000, CRC(7eb59b21) SHA1(664d3e08df0f3d6690838810b6fe273eec3b7821) ) /* graphic ROMs (banked) -- only 9 of 12 are filled */
	ROM_LOAD( "j2.bin",       0x11000, 0x1000, CRC(6615eff3) SHA1(e8455eab03f66642880595cfa0e9be285bf9fad0) )
	ROM_LOAD( "j3.bin",       0x12000, 0x1000, CRC(a10d4444) SHA1(683899e1014ee075b16d9d2610c3c5b5c4efedb6) )
	ROM_LOAD( "j4.bin",       0x13000, 0x1000, CRC(58cd143c) SHA1(e4ab27c09858cede478f4ed3ac6d7392e383a470) )
	ROM_LOAD( "j5.bin",       0x14000, 0x1000, CRC(d7e7ae95) SHA1(7068797770a6c42dc733b253bf6b7376eb6e071e) )
	ROM_LOAD( "j6.bin",       0x15000, 0x1000, CRC(91f62b82) SHA1(2a78039ee63226978544142727d00d1ccc6d2ab4) )
	ROM_LOAD( "j7.bin",       0x16000, 0x1000, CRC(afd0a81f) SHA1(cf10308a0fa4ffabd0deeb186b5602468028ff92) )
	ROM_LOAD( "j8.bin",       0x17000, 0x1000, CRC(dabb609b) SHA1(773b99b670db41a9de58d14b51f81ce0c446ca84) )
	ROM_LOAD( "j9.bin",       0x18000, 0x1000, CRC(8ea9c6a6) SHA1(fe1b299f8760fc5418179d3569932ee2c4dff461) )
	/* the other banks (1900-1fff) are empty */

	ROM_REGION(  0x10000 , REGION_CPU2, 0 ) /* 64k for Z80 sound CPU code */
	ROM_LOAD( "11-7a.bin",    0x0000, 0x1000, CRC(b52d01fa) SHA1(9b6cf9ea51d3a87c174f34d42a4b1b5f38b48723) )
	ROM_LOAD( "10-8a.bin",    0x1000, 0x1000, CRC(9db5c0ce) SHA1(b5bc1d89a7f7d7a0baae64390c37ee11f69a0e76) )
ROM_END


ROM_START( tutankst )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )      /* 64k for M6809 CPU code + 64k for ROM banks */
	ROM_LOAD( "h1.bin",       0x0a000, 0x1000, CRC(da18679f) SHA1(8d2a3665db937d0e1d19300ae22277d9db61fcbc) ) /* program ROMs */
	ROM_LOAD( "h2.bin",       0x0b000, 0x1000, CRC(a0f02c85) SHA1(29a78b3ffd6b597772953543b02dd59acf5af38c) )
	ROM_LOAD( "ra1_3h.cpu",   0x0c000, 0x1000, CRC(2d62d7b1) SHA1(910718f36735f2614cda0c3a1abdfa995d82dbd2) )
	ROM_LOAD( "h4.bin",       0x0d000, 0x1000, CRC(bd06fad0) SHA1(bd10bbb413d8dd362072522e902575d819fa8336) )
	ROM_LOAD( "h5.bin",       0x0e000, 0x1000, CRC(bf9fd9b0) SHA1(458ea2ff5eedaaa02e32444dd6004d2eaadbdeab) )
	ROM_LOAD( "ra1_6h.cpu",   0x0f000, 0x1000, CRC(c43b3865) SHA1(3112cf831c5b6318337e591ccb0003aeab722652) )
	ROM_LOAD( "j1.bin",       0x10000, 0x1000, CRC(7eb59b21) SHA1(664d3e08df0f3d6690838810b6fe273eec3b7821) ) /* graphic ROMs (banked) -- only 9 of 12 are filled */
	ROM_LOAD( "j2.bin",       0x11000, 0x1000, CRC(6615eff3) SHA1(e8455eab03f66642880595cfa0e9be285bf9fad0) )
	ROM_LOAD( "j3.bin",       0x12000, 0x1000, CRC(a10d4444) SHA1(683899e1014ee075b16d9d2610c3c5b5c4efedb6) )
	ROM_LOAD( "j4.bin",       0x13000, 0x1000, CRC(58cd143c) SHA1(e4ab27c09858cede478f4ed3ac6d7392e383a470) )
	ROM_LOAD( "j5.bin",       0x14000, 0x1000, CRC(d7e7ae95) SHA1(7068797770a6c42dc733b253bf6b7376eb6e071e) )
	ROM_LOAD( "j6.bin",       0x15000, 0x1000, CRC(91f62b82) SHA1(2a78039ee63226978544142727d00d1ccc6d2ab4) )
	ROM_LOAD( "j7.bin",       0x16000, 0x1000, CRC(afd0a81f) SHA1(cf10308a0fa4ffabd0deeb186b5602468028ff92) )
	ROM_LOAD( "j8.bin",       0x17000, 0x1000, CRC(dabb609b) SHA1(773b99b670db41a9de58d14b51f81ce0c446ca84) )
	ROM_LOAD( "j9.bin",       0x18000, 0x1000, CRC(8ea9c6a6) SHA1(fe1b299f8760fc5418179d3569932ee2c4dff461) )
	/* the other banks (1900-1fff) are empty */

	ROM_REGION(  0x10000 , REGION_CPU2, 0 ) /* 64k for Z80 sound CPU code */
	ROM_LOAD( "11-7a.bin",    0x0000, 0x1000, CRC(b52d01fa) SHA1(9b6cf9ea51d3a87c174f34d42a4b1b5f38b48723) )
	ROM_LOAD( "10-8a.bin",    0x1000, 0x1000, CRC(9db5c0ce) SHA1(b5bc1d89a7f7d7a0baae64390c37ee11f69a0e76) )
ROM_END



GAME( 1982, tutankhm, 0,        tutankhm, tutankhm, 0, ROT90, "Konami", "Tutankham", 0 )
GAME( 1982, tutankst, tutankhm, tutankhm, tutankhm, 0, ROT90, "[Konami] (Stern license)", "Tutankham (Stern)", 0 )
