/***************************************************************************

 *****************************
 *** NINJA KID II hardware ***      (by Roberto Ventura)
 *****************************

Game authors:
Game design:    Tsutomu Fuzisawa
Program design: Satoru Kinjo
Char. design:   Tsutomu Fizisawa
Char. design:   Akemi Tsunoda
Sound compose:  Tsutomu Fuzisawa
Bgm create:     Mecano Associate
Data make:      Takashi Hayashi

General aspect.

The game is driven by a fast Z80 CPU.
Screen resolution is 256x192.
768 colors on screen.
96 sprites.

Rom Contents:

NK2_01.ROM              Main CPU program ROM
NK2_02.ROM              CPU banked data ROM 0 (banks 0 and 1)
NK2_03.ROM              CPU banked data ROM 1 (banks 2 and 3)
NK2_04.ROM              CPU banked data ROM 2 (banks 4 and 5)
NK2_05.ROM              CPU banked data ROM 3 (banks 6 and 7)
NK2_06.ROM              Sound CPU program ROM (encrypted)
NK2_07.ROM              Sprites data ROM 1
NK2_08.ROM              Sprites data ROM 0
NK2_09.ROM              Raw PCM samples (complete?)
NK2_10.ROM              Background data ROM 1
NK2_11.ROM              Background data ROM 0
NK2_12.ROM              Foreground data ROM

*** MEMORY MAP ***

0000-7fff       Main CPU ROM
8000-bfff       Banked CPU ROM
c000-c7ff       I/O
c800-cdff       Color RAM
d000-d7ff       "FRONT" tile map
d800-dfff       "BACK" tile map
e000-efff       Main RAM
f400-f7ff   ??? screen frame ???
f800-f9ff   CPU Stack
fa00-ffff       Sprite registers (misc RAM)


1) CPU

1 OSC 12 MHz
1 OSC 5 MHz
2 YM 2203C.......CaBBe!.

The Z80 runs in IM0,the game expects execution of RST10 each
frame.

Game level maps,additional code and data are available to main
program via CPU banking at lacations 8000-bf00

In two of the sets, the encrypted sound program ROM is replaced with a
double-sized decrypted version. I don0t know if they are bootlegs or originals.



2) I/O

c000    "KEYCOIN" button

        76543210
        || |  ||
        || |  |^-COIN 1
        || |  ^--COIN 2
        || ^-----TEST MODE (on the fly,to be pressed during boot)
        |^-------START 1
        ^--------START 2


c001    "PAD1"
c002    "PAD2"

        76543210
          ||||||
          |||||^-RIGHT
          ||||^--LEFT
          |||^---DOWN
          ||^----UP
          |^-----FIRE 0
          ^------FIRE 1


c003    "DIPSW1"

        76543210
        ||||||||
        |||||||^-UNUSED?
        ||||||^-->EXTRA
        |||||^--->LIVES
        ||||^----CONTINUE MODE
        |||^-----DEMO SOUND
        ||^------NORMAL/HARD
        |^-------LIVES 3/4
        ^--------ENGLISH/JAP


c004    "DIPSW2"

        76543210
        ||||||||
        |||||||^-TEST MODE
        ||||||^--TABLE/UPRIGHT
        |||||^---"CREDIT SERVICE"
        ||||^---->
        |||^----->>
        ||^------>>> coin/credit configurations
        |^------->>
        ^-------->

c200    Sound command
        This byte is written when game plays sound effects...
        it is set when music or sound effects (both pcm and fm) are triggered;
        I guess it is read by another CPU,then.

c201    Unknown,but used.

c202    Bank selector (0 to 7)

c203    Sprite 'overdraw'
        this is the most interesting feature of the game,when set
        the sprites drawn in the previous frame remain on the
        screen,so the game can perform special effects such as the
        'close up' when you die or the "infinite balls" appearing
        when an extra weapon is collected.
        Appears to work like a xor mask,a sprite removes older
        sprite 'bitmap' when present;other memory locations connected to
        this function may be f400-f7ff...should be investigated more.
        -mmmh... I believe this is sci-fiction for a non-bitmap game...

C208    Scroll X  0-7

C209    Scroll X  MSB

C20A    Scroll Y  0-7

C20B    Scroll Y  MSB

C20C    Background ENABLE
        when set to zero the background is totally black,but
        sprites are drawn correctly.


3) COLOR RAM

The palette system is dynamic,the game can show up to 768 different
colors on screen.

Palette depth is 12 bits as usual,two consecutive bytes are used for each
color code.

format: RRRRGGGGBBBB0000

Colors are organized in palettes,since graphics is 4 bits (16 colors)
each palette takes 32 bytes,the three different layers,BACK,SPRITES and
FRONT don't share any color,each has its own 256 color space,hence the
768 colors on screen.

c800-c9ff       Background palettes
ca00-cbff       Sprites palettes
cc00-cdff       Foreground palettes


4) TILE-BASED LAYERS

The tile format for background and foreground is the same,the
only difference is that background tiles are 16x16 pixels while foreground
tiles are only 8x8.

Background virtual screen is 512x512 pixels scrollable.

Two consecutive tiles bytes identify one tile.

        O7 O6 O5 O4 O3 O2 O1 O0         gfx Offset
        O9 O8 FY FX C3 C2 C1 C0         Attibute

        O= GFX offset (1024 tiles)
        F= Flip X and Y
        C= Color palette selector


5) SPRITES

Five bytes identify each sprite,but the registers actually used
are placed at intervals of 16.
Some of the remaining bytes are used (e.g. fa00),their meaning is totally
unknown to me,they seem to be related to the surprising additional sprite
feature of the game,but maybe they're just random writes in RAM.

The first sprite data is located at fa0b,then fa1b and so on.


0b      Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0         Y coord
0c      X7 X6 X5 X4 X3 X2 X1 X0         X coord
0d      O9 O8 FY FX -- -- EN X8         hi gfx - FLIP - Enable - hi X
0e      O7 O6 O5 O4 O3 O2 O1 O0         gfx - offset
0f      -- -- -- -- C3 C2 C1 C0         color

        Y= Y coordinate
        X= X coordinate (X8 is used to clip sprite on the left)
        O= Gfx offset (1024 sprites)
        F= Flip
       EN= Enable (this is maybe the Y8 coordinate bit,but it isn't set
           accordingly in the test mode
        C= Color palette selector

***************************************************************************/


#include "driver.h"
#include "sound/2203intf.h"
#include "sound/samples.h"
#include "machine/mc8123.h"

extern WRITE8_HANDLER( ninjakd2_bgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_fgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_scrollx_w );
extern WRITE8_HANDLER( ninjakd2_scrolly_w );
extern WRITE8_HANDLER( ninjakd2_sprite_overdraw_w );
extern WRITE8_HANDLER( ninjakd2_background_enable_w );
extern VIDEO_START( ninjakd2 );
extern VIDEO_UPDATE( ninjakd2 );

extern UINT8 *ninjakd2_bg_videoram, *ninjakd2_fg_videoram;

static int ninjakd2_bank_latch = 255;
static INT16 *sampledata[8];
static int samplelen[8];

static void ninjakd2_init_samples(void)
{
	int i,n;
	UINT8 *source = memory_region(REGION_SOUND1);
	static const int sample_info [9][2] = { {0x0000,0x0A00},{0x0A00,0x1D00},{0x2700,0x1700},
	{0x3E00,0x1500},{0x5300,0x0B00},{0x5E00,0x0A00},{0x6800,0x0E00},{0x7600,0x1E00},{0xF000,0x0400} };

	for (i=0;i<8;i++)
	{
		sampledata[i] = auto_malloc(sample_info[i][1] * sizeof(sampledata[0]));
		samplelen[i] = sample_info[i][1];
		for (n=0; n<sample_info[i][1]; n++)
			sampledata[i][n] = (INT8)(source[sample_info[i][0]+n] ^ 0x80) * 256;
	}
}


static INTERRUPT_GEN( ninjakd2_interrupt )
{
	cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, 0xd7);	/* RST 10h */
}

#ifdef UNUSED_FUNCTION
READ8_HANDLER( ninjakd2_bankselect_r )
{
	return ninjakd2_bank_latch;
}
#endif

static WRITE8_HANDLER( ninjakd2_bankselect_w )
{
	UINT8 *ROM = memory_region(REGION_CPU1);
	int bankaddress;

	if (data != ninjakd2_bank_latch)
	{
		ninjakd2_bank_latch = data;

		bankaddress = 0x10000 + ((data & 0x7) * 0x4000);
		memory_set_bankptr(1,&ROM[bankaddress]);	 /* Select 8 banks of 16k */
	}
}

static WRITE8_HANDLER( ninjakd2_pcm_play_w )
{
	int i;
	static const int sample_no[9] = { 0x00,0x0A,0x27,0x3E,0x53,0x5E,0x68,0x76,0xF0 };

	for(i=0;i<9;i++)
	 if (sample_no[i]==data) break;

	if (i==8)
		sample_stop(0);
	else
		sample_start_raw(0,sampledata[i],samplelen[i],16000,0);
}

static ADDRESS_MAP_START( ninjakd2_main_cpu, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xc000, 0xc000) AM_READ(input_port_2_r)
	AM_RANGE(0xc001, 0xc001) AM_READ(input_port_0_r)
	AM_RANGE(0xc002, 0xc002) AM_READ(input_port_1_r)
	AM_RANGE(0xc003, 0xc003) AM_READ(input_port_3_r)
	AM_RANGE(0xc004, 0xc004) AM_READ(input_port_4_r)
	AM_RANGE(0xc200, 0xc200) AM_WRITE(soundlatch_w)
	AM_RANGE(0xc201, 0xc201) AM_WRITENOP		// unknown but used
	AM_RANGE(0xc202, 0xc202) AM_WRITE(ninjakd2_bankselect_w)
	AM_RANGE(0xc203, 0xc203) AM_WRITE(ninjakd2_sprite_overdraw_w)
	AM_RANGE(0xc208, 0xc209) AM_WRITE(ninjakd2_scrollx_w)
	AM_RANGE(0xc20a, 0xc20b) AM_WRITE(ninjakd2_scrolly_w)
	AM_RANGE(0xc20c, 0xc20c) AM_WRITE(ninjakd2_background_enable_w)
	AM_RANGE(0xc800, 0xcdff) AM_RAM AM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE(&paletteram)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_WRITE(ninjakd2_fgvideoram_w) AM_BASE(&ninjakd2_fg_videoram)
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_WRITE(ninjakd2_bgvideoram_w) AM_BASE(&ninjakd2_bg_videoram)
	AM_RANGE(0xe000, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xffff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjakd2_sound_cpu, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)
	AM_RANGE(0xefee, 0xefee) AM_NOP							/* CHIP COMMAND ?? */
	AM_RANGE(0xeff5, 0xeff6) AM_WRITENOP					/* SAMPLE FREQUENCY ??? */
	AM_RANGE(0xf000, 0xf000) AM_WRITE(ninjakd2_pcm_play_w)	/* PCM SAMPLE OFFSET*256 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjakd2_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(YM2203_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2203_write_port_0_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(YM2203_control_port_1_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(YM2203_write_port_1_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( ninjakd2 )
    PORT_START
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

    PORT_START    /* player 2 controls */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

    PORT_START
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	/* keep pressed during boot to enter service mode */
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

    PORT_START  /* dsw1 */
    PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
    PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:2,3")
    PORT_DIPSETTING(    0x04, "20000 50000" )
    PORT_DIPSETTING(    0x06, "30000 50000" )
    PORT_DIPSETTING(    0x02, "50000 100000" )
    PORT_DIPSETTING(    0x00, DEF_STR( None ) )
    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:4")
    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Yes )  )
    PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:5")
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On )  )
    PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:6")
    PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:7")
    PORT_DIPSETTING(    0x40, "3" )
    PORT_DIPSETTING(    0x00, "4" )
    PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:8")
    PORT_DIPSETTING(    0x00, DEF_STR( English ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

    PORT_START  /* dsw2 */
    PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
    PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
    PORT_DIPNAME( 0x04, 0x00, "Credit Service" ) PORT_DIPLOCATION("SW2:3")
    PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x04, DEF_STR( On ) )
    PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5")
    PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
    PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:6,7,8")
    PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
    PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,     /* 8*8 characters */
	1024,    /* 1024 characters */
	4,       /* 4 bits per pixel */
	{0,1,2,3 }, /* the bitplanes are packed in one nibble */
	{0, 4, 16384*8+0, 16384*8+4, 8, 12, 16384*8+8, 16384*8+12 },
	{16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	8*16
};

static const gfx_layout spritelayout =
{
	16,16,   /* 16*16 characters */
	1024,    /* 1024 sprites */
	4,       /* 4 bits per pixel */
	{0,1,2,3}, /* the bitplanes are packed in one nibble */
	{0,  4,  65536*8+0,  65536*8+4,  8, 12,  65536*8+8, 65536*8+12,
		16*8+0, 16*8+4, 16*8+65536*8+0, 16*8+65536*8+4, 16*8+8, 16*8+12, 16*8+65536*8+8, 16*8+65536*8+12},
	{16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7,
		32*8+16*0, 32*8+16*1, 32*8+16*2, 32*8+16*3, 32*8+16*4, 32*8+16*5, 32*8+16*6, 32*8+16*7},
	8*64
};

static GFXDECODE_START( ninjakd2 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout,  0*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 16*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, charlayout,   32*16, 16 )
GFXDECODE_END

static const struct Samplesinterface samples_interface =
{
	1,	/* 1 channel */
	NULL,
	ninjakd2_init_samples
};

/* handler called by the 2203 emulator when the internal timers cause an IRQ */
static void irqhandler(int irq)
{
	cpunum_set_input_line(1,0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YM2203interface ym2203_interface =
{
	0,0,0,0,irqhandler
};


static MACHINE_DRIVER_START( ninjakd2 )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 12000000/2)		/* 12000000/2 (verified on pcb) */
	MDRV_CPU_PROGRAM_MAP(ninjakd2_main_cpu,0)	/* very sensitive to these settings */
	MDRV_CPU_VBLANK_INT(ninjakd2_interrupt,1)

	MDRV_CPU_ADD(Z80, 5000000)
	/* audio CPU */		/* 5mhz crystal (verified on pcb) */
	MDRV_CPU_PROGRAM_MAP(ninjakd2_sound_cpu,0)
	MDRV_CPU_IO_MAP(ninjakd2_sound_io,0)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(10000))

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MDRV_GFXDECODE(ninjakd2)
	MDRV_PALETTE_LENGTH(768)

	MDRV_VIDEO_START(ninjakd2)
	MDRV_VIDEO_UPDATE(ninjakd2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 1500000) /* verified on pcb */
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD(YM2203, 1500000) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END



ROM_START( ninjakd2 )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "nk2_01.rom",   0x00000, 0x8000, CRC(3cdbb906) SHA1(f48f82528b5fc581ee3b1ccd0ef9cdecc7249bb3) )
	ROM_LOAD( "nk2_02.rom",   0x10000, 0x8000, CRC(b5ce9a1a) SHA1(295a7e1d41e1a8ee45f1250086a0c9314837eded) )
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "nk2_05.rom",   0x28000, 0x8000, CRC(5dac9426) SHA1(0916cddbbe1e93c32b96fe28e145d34b2a892e80) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "nk2_06.rom",   0x0000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) )  // sound z80 code encrypted

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* MC8123 key */
	ROM_LOAD( "ninjakd2.key",  0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x4000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )	/* background tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_10.rom",   0x08000, 0x4000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x4000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )	/* sprites tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_07.rom",   0x08000, 0x4000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x08000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_12.rom",   0x00000, 0x02000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )	/* foreground tiles */
	ROM_CONTINUE(             0x04000, 0x02000)
	ROM_CONTINUE(             0x02000, 0x02000)
	ROM_CONTINUE(             0x06000, 0x02000)

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )	/* raw pcm samples */
ROM_END

ROM_START( ninjak2a )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "nk2_01.bin",   0x00000, 0x8000, CRC(e6adca65) SHA1(33d483dde0853f37455cde32b461f4e919601b4b) )
	ROM_LOAD( "nk2_02.bin",   0x10000, 0x8000, CRC(d9284bd1) SHA1(e790fb1a718a1f7997931f2f390fe053655f231d) )
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "nk2_05.bin",   0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) )

	ROM_REGION( 2*0x10000, REGION_CPU2, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "nk2_06.bin",   0x10000, 0x8000, CRC(7bfe6c9e) SHA1(aef8cbeb0024939bf65f77113a5cf777f6613722) )	/* decrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )				/* decrypted data */

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x4000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )	/* background tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_10.rom",   0x08000, 0x4000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x4000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )	/* sprites tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_07.rom",   0x08000, 0x4000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x08000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_12.rom",   0x00000, 0x02000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )	/* foreground tiles */
	ROM_CONTINUE(             0x04000, 0x02000)
	ROM_CONTINUE(             0x02000, 0x02000)
	ROM_CONTINUE(             0x06000, 0x02000)

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )	/* raw pcm samples */
ROM_END

ROM_START( ninjak2b )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "1.3s",         0x00000, 0x8000, CRC(cb4f4624) SHA1(4fc66641adc0a2c0eca332f27c5777df62fa507b) )
	ROM_LOAD( "2.3q",         0x10000, 0x8000, CRC(0ad0c100) SHA1(c5bbc107ba07bd6950bb4d7377e827c084b8229b) )
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "nk2_05.rom",   0x28000, 0x8000, CRC(5dac9426) SHA1(0916cddbbe1e93c32b96fe28e145d34b2a892e80) )

	ROM_REGION( 2*0x10000, REGION_CPU2, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "nk2_06.bin",   0x10000, 0x8000, CRC(7bfe6c9e) SHA1(aef8cbeb0024939bf65f77113a5cf777f6613722) )	/* decrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )				/* decrypted data */

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x4000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )	/* background tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_10.rom",   0x08000, 0x4000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x4000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )	/* sprites tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_07.rom",   0x08000, 0x4000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x08000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_12.rom",   0x00000, 0x02000, CRC(db5657a9) SHA1(abbb033edb9a5a0c66ee5981d1e4df1ab334a82d) )	/* foreground tiles */
	ROM_CONTINUE(             0x04000, 0x02000)
	ROM_CONTINUE(             0x02000, 0x02000)
	ROM_CONTINUE(             0x06000, 0x02000)

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )	/* raw pcm samples */
ROM_END

ROM_START( rdaction )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "1.3u",  	      0x00000, 0x8000, CRC(5c475611) SHA1(2da88a95b5d68b259c8ae48af1438a82a1d601c1) )
	ROM_LOAD( "2.3s",         0x10000, 0x8000, CRC(a1e23bd2) SHA1(c3b6574dc9fa66b4f41c37754a0d20a865f8bc28) )
	ROM_LOAD( "nk2_03.rom",   0x18000, 0x8000, CRC(ad275654) SHA1(7d29a17132adb19aeee9b98be5b76bd6e91f308e) )
	ROM_LOAD( "nk2_04.rom",   0x20000, 0x8000, CRC(e7692a77) SHA1(84beb8b02c564bffa9cc00313214e8f109bd40f9) )
	ROM_LOAD( "nk2_05.bin",   0x28000, 0x8000, CRC(960725fb) SHA1(160c8bfaf089cbeeef2023f12379793079bff93b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "nk2_06.rom",   0x0000, 0x10000, CRC(d3a18a79) SHA1(e4df713f89d8a8b43ef831b14864c50ec9b53f0b) )  // sound z80 code encrypted

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* MC8123 key */
	ROM_LOAD( "ninjakd2.key",  0x0000, 0x2000, CRC(ec25318f) SHA1(619da3f69f9919e1457f79ee1d38e7ec80c4ebb0) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_11.rom",   0x00000, 0x4000, CRC(41a714b3) SHA1(b05f48d71a9837914c12c13e0b479c8a6dc8c25e) )	/* background tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_10.rom",   0x08000, 0x4000, CRC(c913c4ab) SHA1(f822c5621b3e32c1a284f6367bdcace81c1c74b3) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "nk2_08.rom",   0x00000, 0x4000, CRC(1b79c50a) SHA1(8954bc51cb9fbbe16b09381f35c84ccc56a803f3) )	/* sprites tiles */
	ROM_CONTINUE(             0x10000, 0x4000)
	ROM_CONTINUE(             0x04000, 0x4000)
	ROM_CONTINUE(             0x14000, 0x4000)
	ROM_LOAD( "nk2_07.rom",   0x08000, 0x4000, CRC(0be5cd13) SHA1(8f94a8fef6668aaf13329715fee81302dbd6c685) )
	ROM_CONTINUE(             0x18000, 0x4000)
	ROM_CONTINUE(             0x0c000, 0x4000)
	ROM_CONTINUE(             0x1c000, 0x4000)

	ROM_REGION( 0x08000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "12.5n",        0x00000, 0x02000, CRC(0936b365) SHA1(3705f42b76ab474357e77c1a9b8e3755c7ab2c0c) )	/* foreground tiles */
	ROM_CONTINUE(             0x04000, 0x02000)
	ROM_CONTINUE(             0x02000, 0x02000)
	ROM_CONTINUE(             0x06000, 0x02000)

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )
	ROM_LOAD( "nk2_09.rom",   0x0000, 0x10000, CRC(c1d2d170) SHA1(0f325815086fde90fd85360d3660042b0b68ba96) )	/* raw pcm samples */
ROM_END



static DRIVER_INIT( ninjakd2 )
{
	mc8123_decrypt_rom(1, memory_region(REGION_USER1), 0, 0);
}

static DRIVER_INIT( bootleg )
{
	memory_set_decrypted_region(1, 0x0000, 0x7fff, memory_region(REGION_CPU2) + 0x10000);
}



GAME( 1987, ninjakd2, 0,        ninjakd2, ninjakd2, ninjakd2, ROT0, "UPL", "Ninja-Kid II (set 1)", 0 )
GAME( 1987, ninjak2a, ninjakd2, ninjakd2, ninjakd2, bootleg,  ROT0, "UPL", "Ninja-Kid II (set 2)", 0 )
GAME( 1987, ninjak2b, ninjakd2, ninjakd2, ninjakd2, bootleg,  ROT0, "UPL", "Ninja-Kid II (set 3)", 0 )
GAME( 1987, rdaction, ninjakd2, ninjakd2, ninjakd2, ninjakd2, ROT0, "UPL (World Games license)", "Rad Action", 0 )
