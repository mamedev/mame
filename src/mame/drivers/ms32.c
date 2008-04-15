/* Jaleco MegaSystem 32 (Preliminary Driver)

--- this driver is about to undergo a major update
     based on actual hardware tests ---


Used by Jaleco in the Mid-90's this system, based on the V70 processor consisted
of a two board set up, the first a standard mainboard and the second a 'cartridge'

-- Hardware Information (from Guru) --

MS32 Motherboard
----------------

PCB ID  : MB-93140A EB91022-20079-1
CPU     : NEC D70632GD-20 (V70)
SOUND   : Z80, YMF271, YAC513
OSC     : 48.000MHz, 16.9344MHz, 40.000MHz
RAM     : Cypress CY7C199-25 (x10)
          Sharp LH528256-70 (x5)
          Sharp LH5168D-10 (x1)
          OKI M511664-80 (x8)
DIPs    : 8 position (x3)
OTHER   : 5.5v battery
          Some PALs
          2 pin connector for right speaker (sound out is STEREO)

          Custom chips:
                       JALECO SS91022-01 (208 PIN PQFP)
                       JALECO SS91022-02 (100 PIN PQFP)
                       JALECO SS91022-03 (176 PIN PQFP) *
                       JALECO SS91022-05 (120 PIN PQFP) *
                       JALECO SS91022-07 (208 PIN PQFP)
                       JALECO GS91022-01 (120 PIN PQFP)
                       JALECO GS91022-02 (160 PIN PQFP)
                       JALECO GS91022-03 (100 PIN PQFP)
                       JALECO GS91022-04 (100 PIN PQFP) *

ROMs:     None

Chips marked * also appear on a non-megasystem 32 tetris 2 plus board

MS32 Cartridge
--------------

Game Roms + Custom Chip

The Custom chip is probably related to the encryption?

Desert War     - Custom chip: JALECO SS91022-10 (144 pin PQFP) (located on small plug-in board with) (ID: SE93139 EB91022-30056)
Game Paradise  - Custom chip: JALECO SS91022-10 9515EV 420201 06441
Tetris Plus 2  - Custom chip: JALECO SS91022-10 9513EV 370121 06441
Tetris Plus    - Custom chip: JALECO SS92046-01 9412EV 450891 06441
kirarast       - Custom chip: JALECO SS92047-01 9425EV 367821 06441
Gratia (set 1) - Custom chip: JALECO SS92047-01 9423EV 450891 06441
Gratia (set 2) - Custom chip: JALECO SS91022-10
P47-Aces       - Custom chip: JALECO SS92048-01 9410EV 436091 06441
Angel Kiss     - Custom chip: Jaleco SS92047-01
others are unknown

Notes
-----

Some of the roms for each game are encrypted.

The only difference between the two Gratia sets is in the encrypted ROMs (they use
different custom chips). The program is the same.

16x16x8 'Scroll' Tiles (Non-Roz BG Layer)
8x8x8 'Ascii' Tiles (FG Layer)


ToDo / Notes
------------

Z80 + Sound Bits

Re-Add Priorities

Dip switches/inputs in t2m32 and f1superb
some games (hayaosi2) don't seeem to have service mode even if it's listed among the dips
service mode is still accessible through F1 though

Fix Anything Else (Palette etc.)

Replace CPU divider with Idle skip since we don't know how fast the v70 really is (cpu timing is ignored)...

Mirror Ram addresses?

Not sure about the main "global brightness" control register, I don't think it can make the palette
completely black because of kirarast attract mode, so I'm making it cut by 50% at most.

gametngk seems to need some kind of shadow sprites but the only difference in the sprite attributes is one of the
    priority bits, forcing sprites of that priority to be shadows doesn't work
tetrisp needs shadows as well, see the game selection screen.

The above might be related to the second "global brightness" control register, which is 000000 in all games
except gametngk, tetrisp, tp2m32 and gratia.

horizontal position of tx and bg tilemaps is off by 1 pixel in some games

There should be NVRAM somewhere, maybe fc000000-fc007fff

bbbxing: some sprite/roz/bg alignment issues

gratia: at the beginning of a level it shows the level name in the bottom right corner, scrolling it up
    and making the score display scroll out of the screen. Is this correct ar should there be a raster
    effect keeping the score on screen? And why didn't they just use sprites to do that?

gratia: the 3d sky shown at the beginning of the game has a black gap near the end. It would not be visible
    if I made the "global brightness" register cut to 100% instead of 50%. Mmmm...

gratia: the 3d sky seems to be the only place needed the "wrap" parameter to draw_roz to be set. All other
    games work fine with it not set, and there are many places where it definitely must not be set.

gratia: at the beginning of the game, before the sky appears, the city background appears for
    an instant. Missing layer enable register?

background color: pen 0 is correct for gametngk, but wrong for f1superb. Maybe it dpeends on the layer
    priority order?

roz layer wrapping: currently it's always ON, breaking places where it gets very small so it gets
    repeated on the screen (p47aces, kirarast, bbbxing, gametngk need it OFF).
    gratia and desertwr need it ON.

there are sprite lag issues, but they might be caused by cpu timing so it's better to leave
    them alone until the CPU clock is correct.


Not Working Games
-----------------

tp2m32 - writes invalid SBR, enables interrupts, crashes (fixed patching the bogus SBR).
f1superb - the road is straight despite the road signs saying otherwise? :-p
         - there are 4 unknown ROMS which might be related to the above.
         - the handler for IRQ 11 also seems to be valid, the game might need it.


Jaleco Megasystem 32 Game List - thanks to Yasuhiro
---------------------------------------------------

P-47 Aces (p47aces)
Game Tengoku / Game Paradise (gametngk)
Tetris Plus (tetrisp)
Tetris Plus 2 (tp2m32)
Best Bout Boxing (bbbxing)
Wangan Sensou / Desert War (desertwr)
Second Earth Gratia (92047-01 version) (gratia)
*Second Earth Gratia  (91022-10 version) (gratiaa)
*Super Strong Warriors
F-1 Super Battle (f1superb)

Idol Janshi Su-Chi-Pi 2 (47pie2)
Ryuusei Janshi Kirara Star (kirarast)
Mahjong Angel Kiss
*Vs. Janshi Brand New Stars

Hayaoshi Quiz Ouza Ketteisen (hayaosi2)
*Hayaoshi Quiz Nettou Namahousou
*Hayaoshi Quiz Grand Champion Taikai

Maybe some more...

Games marked * need dumping / redumping

*/

/********** BITS & PIECES **********/

#include "driver.h"
#include "deprecat.h"
#include "sound/ymf271.h"

extern UINT32 *ms32_fce00000;
extern UINT32 *ms32_roz_ctrl;
extern UINT32 *ms32_tx_scroll;
extern UINT32 *ms32_bg_scroll;
extern UINT32 *ms32_priram;
extern UINT32 *ms32_palram;
extern UINT32 *ms32_bgram;
extern UINT32 *ms32_rozram;
extern UINT32 *ms32_lineram;
extern UINT32 *ms32_spram;
extern UINT32 *ms32_txram;
extern UINT32 *ms32_mainram;

WRITE32_HANDLER( ms32_brightness_w );
WRITE32_HANDLER( ms32_palram_w );
READ32_HANDLER( ms32_txram_r );
WRITE32_HANDLER( ms32_txram_w );
READ32_HANDLER( ms32_rozram_r );
WRITE32_HANDLER( ms32_rozram_w );
READ32_HANDLER( ms32_lineram_r );
WRITE32_HANDLER( ms32_lineram_w );
READ32_HANDLER( ms32_bgram_r );
WRITE32_HANDLER( ms32_bgram_w );
READ32_HANDLER( ms32_spram_r );
WRITE32_HANDLER( ms32_spram_w );
READ32_HANDLER( ms32_priram_r );
WRITE32_HANDLER( ms32_priram_w );
WRITE32_HANDLER( ms32_gfxctrl_w );
VIDEO_START( ms32 );
VIDEO_UPDATE( ms32 );

static UINT32 *ms32_fc000000;

static UINT32 *ms32_mahjong_input_select;

static UINT32 to_main;

/********** READ INPUTS **********/

static READ32_HANDLER ( ms32_read_inputs1 )
{
	int a,b,c,d;
	a = input_port_read_indexed(machine, 0);	// unknown
	b = input_port_read_indexed(machine, 1);	// System inputs
	c = input_port_read_indexed(machine, 2);	// Player 1 inputs
	d = input_port_read_indexed(machine, 3);	// Player 2 inputs
	return a << 24 | b << 16 | c << 0 | d << 8;
}


static READ32_HANDLER ( ms32_mahjong_read_inputs1 )
{
	int a,b,c,d;
	a = input_port_read_indexed(machine, 0);	// unknown
	b = input_port_read_indexed(machine, 1);	// System inputs

	switch (ms32_mahjong_input_select[0])
	{
		case 0x01:
			c = input_port_read_indexed(machine, 8);	// Player 1 inputs
			break;
		case 0x02:
			c = input_port_read_indexed(machine, 9);	// Player 1 inputs
			break;
		case 0x04:
			c = input_port_read_indexed(machine, 10);	// Player 1 inputs
			break;
		case 0x08:
			c = input_port_read_indexed(machine, 11);	// Player 1 inputs
			break;
		case 0x10:
			c = input_port_read_indexed(machine, 12);	// Player 1 inputs
			break;
		default:
			c = 0;

	}
	d = input_port_read_indexed(machine, 3);	// Player 2 inputs
	return a << 24 | b << 16 | c << 0 | d << 8;
}


static READ32_HANDLER ( ms32_read_inputs2 )
{
	int a,b,c,d;
	a = input_port_read_indexed(machine, 4);	// Dip 1
	b = input_port_read_indexed(machine, 5);	// Dip 2
	c = input_port_read_indexed(machine, 6);	// Dip 3
	d = input_port_read_indexed(machine, 7);	// unused ?
	return a << 8 | b << 0 | c << 16 | d << 24;
}

static READ32_HANDLER ( ms32_read_inputs3 )
{
	int a,b,c,d;
	a = input_port_read_indexed(machine, 10); // unused?
	b = input_port_read_indexed(machine, 10); // unused?
	c = input_port_read_indexed(machine, 9);
	d = (input_port_read_indexed(machine, 8) - 0xb0) & 0xff;
	return a << 24 | b << 16 | c << 8 | d << 0;
}

static WRITE32_HANDLER( ms32_sound_w )
{
	soundlatch_w(machine,0, data & 0xff);
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, ASSERT_LINE);

	// give the Z80 time to respond
	cpu_spinuntil_time(ATTOTIME_IN_USEC(40));
}

static READ32_HANDLER( ms32_sound_r )
{
	return to_main^0xff;
}

static WRITE32_HANDLER( reset_sub_w )
{
	if(data) cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, PULSE_LINE); // 0 too ?
}




/********** MEMORY MAP **********/

/* some games games test more ram than others .. ram areas with closed comments before
the lines get tested by various games but I'm not sure are really used, maybe they're
mirror addresses? */

/*
p47 aces:
there are bugs in the test routine, so it checks twice the amount of RAM
actually present, relying on mirror addresses.
See how ASCII and SCROLL overlap.
SCRATCH RAM   fee00000-fee1ffff
ASCII RAM     fec00000-fec0ffff (actually fec00000-fec07fff ?)
SCROLL RAM    fec08000-fec17fff (actually fec08000-fec0ffff ?)
ROTATE RAM    fe000000-fe03ffff (actually fe000000-fe01ffff ?)
OBJECT RAM    fe800000-fe87ffff (actually fe800000-fe83ffff ?)
COLOR RAM     fd400000-fd40ffff (this one is actually larger than tested)
PRIORITY RAM  fd180000-fd1bffff (actually fd180000-fd19ffff ?)
SOUND RAM

This applies to most of the other games.
Also, gametngk uses mirror addresses for the background during gameplay, without
support for them bad tiles appear in the bg.
*/


static ADDRESS_MAP_START( ms32_readmem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_READ(SMH_ROM)
	AM_RANGE(0xfc000000, 0xfc007fff) AM_READ(SMH_RAM)
	AM_RANGE(0xfc800000, 0xfc800003) AM_READ(SMH_NOP)	/* sound? */
	AM_RANGE(0xfcc00004, 0xfcc00007) AM_READ(ms32_read_inputs1)
	AM_RANGE(0xfcc00010, 0xfcc00013) AM_READ(ms32_read_inputs2)
/**/AM_RANGE(0xfce00600, 0xfce0065f) AM_READ(SMH_RAM)	/* roz control registers */
/**/AM_RANGE(0xfce00a00, 0xfce00a17) AM_READ(SMH_RAM)	/* tx scroll registers */
/**/AM_RANGE(0xfce00a20, 0xfce00a37) AM_READ(SMH_RAM)	/* bg scroll registers */

	AM_RANGE(0xfd000000, 0xfd000003) AM_READ(ms32_sound_r)
	AM_RANGE(0xfd0e0000, 0xfd0e0003) AM_READ(ms32_read_inputs3) /* analog controls in f1superb? */

///**/AM_RANGE(0xfd104000, 0xfd105fff) AM_READ(SMH_RAM) /* f1superb */
///**/AM_RANGE(0xfd144000, 0xfd145fff) AM_READ(SMH_RAM) /* f1superb */

	AM_RANGE(0xfd180000, 0xfd19ffff) AM_READ(ms32_priram_r)	/* priority ram */
	AM_RANGE(0xfd1a0000, 0xfd1bffff) AM_READ(ms32_priram_r)	/* mirror only used by memory test in service mode */

	AM_RANGE(0xfd400000, 0xfd43ffff) AM_READ(SMH_RAM) /* Palette */
///**/AM_RANGE(0xfd440000, 0xfd47ffff) AM_READ(SMH_RAM) /* f1superb color */

///**/AM_RANGE(0xfdc00000, 0xfdc006ff) AM_READ(SMH_RAM) /* f1superb */
///**/AM_RANGE(0xfde00000, 0xfde01fff) AM_READ(SMH_RAM) /* f1superb lineram */
	AM_RANGE(0xfe000000, 0xfe01ffff) AM_READ(ms32_rozram_r)	/* roz layer */
	AM_RANGE(0xfe020000, 0xfe03ffff) AM_READ(ms32_rozram_r)	/* mirror only used by memory test in service mode */
	AM_RANGE(0xfe200000, 0xfe201fff) AM_READ(ms32_lineram_r) /* line ram for roz layer */
///**/AM_RANGE(0xfe202000, 0xfe2fffff) AM_READ(SMH_RAM) /* f1superb vram */

	AM_RANGE(0xfe800000, 0xfe83ffff) AM_READ(ms32_spram_r)	/* sprites */
	AM_RANGE(0xfe840000, 0xfe87ffff) AM_READ(ms32_spram_r)	/* mirror only used by memory test in service mode */
	AM_RANGE(0xfec00000, 0xfec07fff) AM_READ(ms32_txram_r)	/* tx layer */
	AM_RANGE(0xfec08000, 0xfec0ffff) AM_READ(ms32_bgram_r)	/* bg layer */
	AM_RANGE(0xfec10000, 0xfec17fff) AM_READ(ms32_txram_r)	/* mirror only used by memory test in service mode */
	AM_RANGE(0xfec18000, 0xfec1ffff) AM_READ(ms32_bgram_r)
	AM_RANGE(0xfee00000, 0xfee1ffff) AM_READ(SMH_RAM)
	AM_RANGE(0xffe00000, 0xffffffff) AM_READ(SMH_BANK1)
ADDRESS_MAP_END

static WRITE32_HANDLER( pip_w )
{
	if (data)
		popmessage("fce00a7c = %02x",data);
}

static ADDRESS_MAP_START( ms32_writemem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xfc000000, 0xfc007fff) AM_WRITE(SMH_RAM) AM_BASE(&ms32_fc000000)	// NVRAM?
	AM_RANGE(0xfc800000, 0xfc800003) AM_WRITE(ms32_sound_w) /* sound? */
	AM_RANGE(0xfce00000, 0xfce00003) AM_WRITE(ms32_gfxctrl_w)	/* flip screen + other unknown bits */
	AM_RANGE(0xfce00034, 0xfce00037) AM_WRITE(SMH_NOP) // irq ack?
	AM_RANGE(0xfce00038, 0xfce0003b) AM_WRITE(reset_sub_w)
	AM_RANGE(0xfce00050, 0xfce0005f) AM_WRITE(SMH_NOP)	// watchdog? I haven't investigated
//  AM_RANGE(0xfce00000, 0xfce0007f) AM_WRITE(SMH_RAM) AM_BASE(&ms32_fce00000) /* registers not ram? */
	AM_RANGE(0xfce00280, 0xfce0028f) AM_WRITE(ms32_brightness_w)	// global brightness control
	AM_RANGE(0xfce00600, 0xfce0065f) AM_WRITE(SMH_RAM) AM_BASE(&ms32_roz_ctrl)	/* roz control registers */
//  { 0xfce00800, 0xfce0085f, // f1superb, roz #2 control?
	AM_RANGE(0xfce00a00, 0xfce00a17) AM_WRITE(SMH_RAM) AM_BASE(&ms32_tx_scroll)	/* tx layer scroll */
	AM_RANGE(0xfce00a20, 0xfce00a37) AM_WRITE(SMH_RAM) AM_BASE(&ms32_bg_scroll)	/* bg layer scroll */
	AM_RANGE(0xfce00a7c, 0xfce00a7f) AM_WRITE(pip_w)	// ??? layer related? seems to be always 0
//  AM_RANGE(0xfce00e00, 0xfce00e03)    coin counters + something else

//  AM_RANGE(0xfd104000, 0xfd105fff) AM_WRITE(SMH_RAM) /* f1superb */
//  AM_RANGE(0xfd144000, 0xfd145fff) AM_WRITE(SMH_RAM) /* f1superb */

	AM_RANGE(0xfd180000, 0xfd19ffff) AM_WRITE(ms32_priram_w) AM_BASE(&ms32_priram)	/* priority ram */
	AM_RANGE(0xfd1a0000, 0xfd1bffff) AM_WRITE(ms32_priram_w)			/* mirror only used by memory test in service mode */

	AM_RANGE(0xfd1c0000, 0xfd1c0003) AM_WRITE(SMH_RAM) AM_BASE(&ms32_mahjong_input_select) // ?

	AM_RANGE(0xfd400000, 0xfd43ffff) AM_WRITE(ms32_palram_w) AM_BASE(&ms32_palram) /* Palette */
///**/AM_RANGE(0xfd440000, 0xfd47ffff) AM_WRITE(SMH_RAM) /* f1superb color */
//  AM_RANGE(0xfdc00000, 0xfdc006ff) AM_WRITE(SMH_RAM) /* f1superb */
//  AM_RANGE(0xfde00000, 0xfde01fff) AM_WRITE(SMH_RAM) /* f1superb, lineram #2? */

	AM_RANGE(0xfe000000, 0xfe01ffff) AM_WRITE(ms32_rozram_w) AM_BASE(&ms32_rozram)	/* roz layer */
	AM_RANGE(0xfe020000, 0xfe03ffff) AM_WRITE(ms32_rozram_w)		/* mirror only used by memory test in service mode */
	AM_RANGE(0xfe1ffc88, 0xfe1fffff) AM_WRITE(SMH_NOP)	/* gratia writes here before falling into lineram, could be a mirror */
	AM_RANGE(0xfe200000, 0xfe201fff) AM_WRITE(ms32_lineram_w) AM_BASE(&ms32_lineram) /* line ram for roz layer */
///**/AM_RANGE(0xfe202000, 0xfe2fffff) AM_WRITE(SMH_RAM) /* f1superb vram */
///**/AM_RANGE(0xfe100000, 0xfe1fffff) AM_WRITE(SMH_RAM) /* gratia writes here ?! */
	AM_RANGE(0xfe800000, 0xfe83ffff) AM_WRITE(ms32_spram_w) AM_BASE(&ms32_spram)	/* sprites */
	AM_RANGE(0xfe840000, 0xfe87ffff) AM_WRITE(ms32_spram_w)		/* mirror only used by memory test in service mode */
	AM_RANGE(0xfec00000, 0xfec07fff) AM_WRITE(ms32_txram_w) AM_BASE(&ms32_txram)	/* tx layer */
	AM_RANGE(0xfec08000, 0xfec0ffff) AM_WRITE(ms32_bgram_w) AM_BASE(&ms32_bgram)	/* bg layer */
	AM_RANGE(0xfec10000, 0xfec17fff) AM_WRITE(ms32_txram_w)		/* mirror only used by memory test in service mode */
	AM_RANGE(0xfec18000, 0xfec1ffff) AM_WRITE(ms32_bgram_w)		/* mirror used by gametngk at the beginning of the game */
	AM_RANGE(0xfee00000, 0xfee1ffff) AM_WRITE(SMH_RAM) AM_BASE(&ms32_mainram)
	AM_RANGE(0xffe00000, 0xffffffff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END

/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( ms32 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START_TAG("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,2,1")\
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW3")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( bbbxing )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Timer Speed" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, "60/60" )
	PORT_DIPSETTING(    0x20, "50/60" )
	PORT_DIPSETTING(    0x10, "40/60" )
	PORT_DIPSETTING(    0x30, "35/60" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x80, "Jyoji's Attacking Power" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x40, 0x40, "Kim's Attacking Power" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x20, 0x20, "Thamalatt's Attacking Power" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x10, 0x10, "Jose's Attacking Power" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x08, 0x08, "Carolde's Attacking Power" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x04, 0x04, "Biff's Attacking Power" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x02, 0x02, "Grute's Attacking Power" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )

INPUT_PORTS_END

static INPUT_PORTS_START( desertwr )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Armors" ) PORT_DIPLOCATION("SW2:4,3")
//  PORT_DIPSETTING(    0x00, "2" )     // duplicate setting ?
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x40, 0x40, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Title screen" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )

INPUT_PORTS_END

static INPUT_PORTS_START( gametngk )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x40, 0x40, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Voice" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( tetrisp )
	PORT_INCLUDE( ms32 )

	/* There are inputs for players 3 and 4 in the "test mode",
       but NO addresses are read to check them ! */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, "1/1" )
	PORT_DIPSETTING(    0x30, "2/3" )
	PORT_DIPSETTING(    0x10, "3/5" )
	PORT_DIPSETTING(    0x20, "4/7" )
	PORT_DIPNAME( 0x0c, 0x0c, "Join In" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, "All Modes" )
	PORT_DIPSETTING(    0x04, "Normal and Puzzle Modes" )
	PORT_DIPSETTING(    0x08, "VS Mode" )
//  PORT_DIPSETTING(    0x00, "Normal and Puzzle Modes" )       // "can't play normal mode" in "test mode"
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x40, 0x40, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Voice" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, "English Only" )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "After VS Mode" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x08, "Game Ends" )
	PORT_DIPSETTING(    0x00, "Winner Continues" )
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( tp2m32 )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, "Endless Difficulty" ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x30, 0x30, "Puzzle Difficulty" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, "VS Match" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x40, 0x40, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Voice" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Join In" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Bomb" ) PORT_DIPLOCATION("SW3:7,6")
	PORT_DIPSETTING(    0x04, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x01, 0x01, "Freeze" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( p47aces )
	PORT_INCLUDE( ms32 )

	/* The Dip Switches for this game are completely wrong in the "test mode" ! */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, "500k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x40, 0x40, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x03, 0x03, "FG/BG X offset" ) PORT_DIPLOCATION("SW3:8,7")
	PORT_DIPSETTING(    0x03, "0/0" )
	PORT_DIPSETTING(    0x02, "5/5" )
//  PORT_DIPSETTING(    0x01, "5/5" )
	PORT_DIPSETTING(    0x00, "2/4" )

INPUT_PORTS_END

static INPUT_PORTS_START( gratia )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x08, 0x08, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, "200k and every 1000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x40, 0x40, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

INPUT_PORTS_END

static INPUT_PORTS_START( hayaosi2 )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)	// "Buzzer" (input 0 in "test mode")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)	// "Buzzer" (input 0 in "test mode")

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)	// "Buzzer" (input 0 in "test mode")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, "Computer's AI (VS Mode)" ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Highest ) )
	/*  Lap    Time    Questions */
      /*   1     1:00        4     */
      /*   2     1:00        6     */
      /*   3     1:30        8     */
      /*   4     1:30       10     */
      /*   4     2:00       14     */
      /* final   2:00       18     */
	PORT_DIPNAME( 0x30, 0x30, "Time (Race Mode)" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x00, "Default - 0:15" )
	PORT_DIPSETTING(    0x20, "Default - 0:10" )
	PORT_DIPSETTING(    0x30, "Default" )
	PORT_DIPSETTING(    0x10, "Default + 0:15" )
	/* Round   Default    More */
      /*   1       10        15  */
	PORT_DIPNAME( 0x08, 0x08, "Questions (VS Mode)" ) PORT_DIPLOCATION("SW2:5")	// TO DO : check all rounds
	PORT_DIPSETTING(    0x08, "Default" )
	PORT_DIPSETTING(    0x00, "More" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( hayaosi3 )
	PORT_INCLUDE( hayaosi2 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( kirarast )	// player 1 inputs done? others?
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
//PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) /* already mapped in mahjong inputs */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
//PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) /* ms32.c mahjongs don't have P2 inputs */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,2,1")\
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR (Difficulty) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Tumo Pinfu" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Campaign Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("MJ0")	// Mahjong Inputs 0x01
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("MJ1")	// Mahjong Inputs 0x02
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("MJ2")	// Mahjong Inputs 0x04
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("MJ3")	// Mahjong Inputs 0x08
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("MJ4")	// Mahjong Inputs 0x10 unused ?
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( 47pie2 )	// player 1 inputs done? others?
	PORT_INCLUDE( kirarast )

	PORT_MODIFY("IN1")
//PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) /* coin 2 is unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Tumo Pinfu" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Campaign Mode" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( f1superb )	// Mostly wrong !
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("AN0")	// Acceleration (wrong?)
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)


	PORT_START_TAG("AN1")	// Steering
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START_TAG("AN2?")	// Shift + Brake
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

/********** GFX DECODE **********/

/* sprites are contained in 256x256 "tiles" */
static GFXLAYOUT_RAW( spritelayout, 8, 256, 256, 256*8, 256*256*8 )
static GFXLAYOUT_RAW( bglayout, 8, 16, 16, 16*8, 16*16*8 )
static GFXLAYOUT_RAW( txlayout, 8, 8, 8, 8*8, 8*8*8 )

static GFXDECODE_START( ms32 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout, 0x0000, 0x10 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, bglayout,     0x2000, 0x10 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, bglayout,     0x1000, 0x10 )
	GFXDECODE_ENTRY( REGION_GFX4, 0, txlayout,     0x6000, 0x10 )
GFXDECODE_END



/********** INTERRUPTS **********/

/* Irqs used in desertwr:
   1  - 6a0 - minimal
   9  - 6c8 - minimal
   10 - 6d4 - big, vbl?
*/

static UINT16 irqreq;

static IRQ_CALLBACK(irq_callback)
{
	int i;
	for(i=15; i>=0 && !(irqreq & (1<<i)); i--);
	irqreq &= ~(1<<i);
	if(!irqreq)
		cpunum_set_input_line(machine, 0, 0, CLEAR_LINE);
	return i;
}

static void irq_init(void)
{
	irqreq = 0;
	cpunum_set_input_line(Machine, 0, 0, CLEAR_LINE);
	cpunum_set_irq_callback(0, irq_callback);
}

static void irq_raise(int level)
{
	irqreq |= (1<<level);
	cpunum_set_input_line(Machine, 0, 0, ASSERT_LINE);
}

static INTERRUPT_GEN(ms32_interrupt)
{
	if( cpu_getiloops() == 0 ) irq_raise(10);
	if( cpu_getiloops() == 1 ) irq_raise(9);
	/* hayaosi2 needs at least 12 IRQ 0 per frame to work (see code at FFE02289)
       kirarast needs it too, at least 8 per frame, but waits for a variable amount
       47pi2 needs ?? per frame (otherwise it hangs when you lose)
       in different points. Could this be a raster interrupt?
       Other games using it but not needing it to work:
       desertwr
       p47aces
       */
	if( cpu_getiloops() >= 3 && cpu_getiloops() <= 32 ) irq_raise(0);
}

/********** SOUND **********/

/*
 Jaleco Mega System 32 sound Z80

 RAM 62256 - the highest RAM adr line is grounded, only 16k is used

 0000-3eff: program ROM (fixed)
 3f00-3f0f: YMF271-F
 3f10     : RW :command latch
 3f20     : RW :2nd command latch  ?? (not connected on PCB)
 3f40     : W : YMF271 pin 4 (bit 1) , YMF271 pin 39 (bit 4)
 3f70     : W : unknown ? connected to JALECO GS91022-04 pin 55 (from GAL)
 3f80     : Bank select - $bB
 4000-7fff: RAM
 8000-bfff: banked ROM area #1 - bank B+1
 c000-ffff: banked ROM area #2 - bank b+1

 IRQ is unused (YMF271 timers are polled to control tempo)
 NMI reads the command latch
 code at $38 reads the 2nd command latch ??
*/

static READ8_HANDLER( latch_r )
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, CLEAR_LINE);
	return soundlatch_r(machine,0)^0xff;
}

static WRITE8_HANDLER( ms32_snd_bank_w )
{
	memory_set_bank(4, (data >> 0) & 0x0F);
	memory_set_bank(5, (data >> 4) & 0x0F);
}

static WRITE8_HANDLER( to_main_w )
{
		to_main=data;
		irq_raise(1);
}

static ADDRESS_MAP_START( ms32_snd_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3eff) AM_READ(SMH_ROM)
	AM_RANGE(0x3f00, 0x3f0f) AM_READ(YMF271_0_r)
	AM_RANGE(0x3f10, 0x3f10) AM_READ(latch_r)
	AM_RANGE(0x3f20, 0x3f20) AM_READ(SMH_NOP) /* 2nd latch ? */
	AM_RANGE(0x4000, 0x7fff) AM_READ(SMH_RAM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_BANK4)
	AM_RANGE(0xc000, 0xffff) AM_READ(SMH_BANK5)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ms32_snd_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3eff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x3f00, 0x3f0f) AM_WRITE(YMF271_0_w)
	AM_RANGE(0x3f10, 0x3f10) AM_WRITE(to_main_w)
	AM_RANGE(0x3f20, 0x3f20) AM_WRITE(SMH_NOP) /* to_main2_w  ? */
	AM_RANGE(0x3f40, 0x3f40) AM_WRITE(SMH_NOP)   /* YMF271 pin 4 (bit 1) , YMF271 pin 39 (bit 4) */
	AM_RANGE(0x3f70, 0x3f70) AM_WRITE(SMH_NOP)   // watchdog? banking? very noisy
	AM_RANGE(0x3f80, 0x3f80) AM_WRITE(ms32_snd_bank_w)
	AM_RANGE(0x4000, 0x7fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END

static const struct YMF271interface ymf271_interface =
{
	REGION_SOUND1
};

/********** MACHINE INIT **********/

static MACHINE_RESET( ms32 )
{
	memory_set_bankptr(1, memory_region(REGION_CPU1));
	memory_set_bank(4, 0);
	memory_set_bank(5, 1);
	irq_init();
}

/********** MACHINE DRIVER **********/

static MACHINE_DRIVER_START( ms32 )

	/* basic machine hardware */
	MDRV_CPU_ADD(V70, 20000000) // 20MHz
	MDRV_CPU_PROGRAM_MAP(ms32_readmem,ms32_writemem)
	MDRV_CPU_VBLANK_INT_HACK(ms32_interrupt,32)

	MDRV_CPU_ADD(Z80, 4000000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(ms32_snd_readmem, ms32_snd_writemem)

	MDRV_INTERLEAVE(1000)

	MDRV_MACHINE_RESET(ms32)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(ms32)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(ms32)
	MDRV_VIDEO_UPDATE(ms32)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMF271, 16934400)
	MDRV_SOUND_CONFIG(ymf271_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


/********** ROM LOADING **********/

ROM_START( bbbxing )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "bbbx25.bin", 0x000003, 0x80000, CRC(b526b41e) SHA1(44945931b159646468a954d5acdd2c6c61daf098) )
	ROM_LOAD32_BYTE( "bbbx27.bin", 0x000002, 0x80000, CRC(45b27ad8) SHA1(0af415b17400aabecdcb6d1d069f28b64780017f) )
	ROM_LOAD32_BYTE( "bbbx29.bin", 0x000001, 0x80000, CRC(85bbbe79) SHA1(bc5ebb96491762e6a0d202ddf7faeb57c66211b4) )
	ROM_LOAD32_BYTE( "bbbx31.bin", 0x000000, 0x80000, CRC(e0c865ed) SHA1(f21e8dc174c50d7afdd3f82c1c66dfcc002bdd07) )

	ROM_REGION( 0x1100000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "bbbx1.bin",   0x0000002, 0x200000, CRC(c1c10c3b) SHA1(e1f739f38e148c4d7aff6b81b3e42131c5c6c3dd) )
	ROM_LOAD32_WORD( "bbbx13.bin",  0x0000000, 0x200000, CRC(4b8c1574) SHA1(c389c70b532d54528a175f460ca3f329b34cf67c) )
	ROM_LOAD32_WORD( "bbbx2.bin",   0x0400002, 0x200000, CRC(03b77c1e) SHA1(f156ae6a4f2a8ae99815eb5a7b28425d273c1c3e) )
	ROM_LOAD32_WORD( "bbbx14.bin",  0x0400000, 0x200000, CRC(e9cfd83b) SHA1(8580c571a4144ea27c7fca7e86e3e4f5e4f5facb) )
	ROM_LOAD32_WORD( "bbbx3.bin",   0x0800002, 0x200000, CRC(bba0d1a4) SHA1(15f8de7478182c36927a84ee8de8eb8ac3e65d7b) )
	ROM_LOAD32_WORD( "bbbx15.bin",  0x0800000, 0x200000, CRC(6ab64a10) SHA1(4ee2cec6b9f8d729ff53851f7949c5cd3a8029d8) )
	ROM_LOAD32_WORD( "bbbx4.bin",   0x0c00002, 0x200000, CRC(97f97e3a) SHA1(260603a10fe742986aa4f7fb90e4f141bdadae17) )
	ROM_LOAD32_WORD( "bbbx16.bin",  0x0c00000, 0x200000, CRC(e001d6cb) SHA1(c887dbf192e6b46c86fd86bb5f58b44ab8fe8d73) )
	ROM_LOAD32_WORD( "bbbx5.bin",   0x1000002, 0x080000, CRC(64989edf) SHA1(033eab0e8a53607b2bb420f6356804b2cfa1544c) )
	ROM_LOAD32_WORD( "bbbx17.bin",  0x1000000, 0x080000, CRC(1d7ebaf0) SHA1(5aac7cb01013ce3be206318678aced5812bff9a9) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "bbbx9.bin",   0x000000, 0x200000, CRC(a41cb650) SHA1(1c55a4afe55c1250806f2d93c25842dc3fb7f987) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "bbbx11.bin",  0x000000, 0x200000, CRC(85238ca9) SHA1(1ce32d585fe66764d621c11882ef9d2abaea6256) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "bbbx32-2.bin",0x000000, 0x080000, CRC(3ffdae75) SHA1(2b837d28f7ecdd49e8525bd5c249e83021d5fe9f) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "bbbx21.bin",  0x000000, 0x040000, CRC(5f3ea01f) SHA1(761f6a5852312d2b12de009f3cf0476f5b2e906c) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "bbbx22.bin",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "bbbx23.bin",  0x200000, 0x200000, CRC(b7875a23) SHA1(62bb4c1318f98ea68894658d92ce08e84d386d0c) )
ROM_END

ROM_START( 47pie2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "93166-26.v11", 0x000003, 0x80000, CRC(e4e62134) SHA1(224b3e8dba56009bf2af6eceb7495e60302a6360) )
	ROM_LOAD32_BYTE( "93166-27.v11", 0x000002, 0x80000, CRC(7bd00919) SHA1(60565b5e1da5fee00ac4a7fb1202d7150dab49ee) )
	ROM_LOAD32_BYTE( "93166-28.v11", 0x000001, 0x80000, CRC(aa49eec2) SHA1(173afc596caa1c464fc3247cb64d36c1d97a1520) )
	ROM_LOAD32_BYTE( "93166-29.v11", 0x000000, 0x80000, CRC(92763e41) SHA1(eb593bbb586661c4c4e8728d845b146974d0bdf8) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "94019-02.1",  0x000002, 0x200000, CRC(f9d692f2) SHA1(666df55d26e00be39073173fa3616ac9dafbe615) )
	ROM_LOAD32_WORD( "94019-01.13", 0x000000, 0x200000, CRC(1ddfe825) SHA1(27fbf492fdb4f0b4b8db18330e840c130213e15e) )
	ROM_LOAD32_WORD( "94019-04.2",  0x400002, 0x200000, CRC(24ca77ec) SHA1(a5c575224ab276cbed5785f40fc0d35dd2748e74) )
	ROM_LOAD32_WORD( "94019-03.14", 0x400000, 0x200000, CRC(b26426c4) SHA1(2d95137edea7d0c380dd8fd99852254ad3e4c837) )
	ROM_LOAD32_WORD( "94019-06.3",  0x800002, 0x200000, CRC(c8aa4b57) SHA1(55da6a43ba6f0cb32f5d024f67cae21d04019d2a) )
	ROM_LOAD32_WORD( "94019-05.15", 0x800000, 0x200000, CRC(1da63eb4) SHA1(8193ad27ddfe6ba242b73082d1b4a400e88401ba) )
	ROM_LOAD32_WORD( "94019-08.4",  0xc00002, 0x200000, CRC(4b07edc9) SHA1(22aaa923a94a7bec997d2adabc8ec2c7696c33a5) )
	ROM_LOAD32_WORD( "94019-07.16", 0xc00000, 0x200000, CRC(34f471a8) SHA1(4c9c358a9bfdb436a211caa14d085e631609681d) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019-09.11", 0x000000, 0x200000, CRC(cde7bb6f) SHA1(47454dac4ce67ce8d7e0c5ef3a732477ac8170a7) )

	ROM_REGION( 0x100000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019-12.10", 0x000000, 0x100000, CRC(15ae05d9) SHA1(ac00d3766c42ccba4585b9acfacc81bcb940ac26) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "93166-30.bin", 0x000000, 0x080000, CRC(0c738883) SHA1(e552c1842d759e5e617eb9c6cc178620a461b4dd) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "93166-21.bin", 0x000000, 0x040000, CRC(e7fd1bf4) SHA1(74567530364bfd93bffddb588758d8498e197668) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "94019-10.22", 0x000000, 0x200000, CRC(745d41ec) SHA1(9118d0f27b65c9d37970326ccf86fdccb81d32f5) )
	ROM_LOAD( "94019-11.23", 0x200000, 0x200000, CRC(021dc350) SHA1(c71936091f86440201fdbdc94b0d1d22c4018188) )
ROM_END

ROM_START( 47pie2o )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "93166-26.v10", 0x000003, 0x80000, CRC(21dc94dd) SHA1(faf2eea891cb061d5df47ef31d9538feb0c1233c) )
	ROM_LOAD32_BYTE( "93166-27.v10", 0x000002, 0x80000, CRC(5bf18a7d) SHA1(70869dc37e6ad79ce4e85db71a03c5cccf9d732b) )
	ROM_LOAD32_BYTE( "93166-28.v10", 0x000001, 0x80000, CRC(b1261d51) SHA1(3f393aeb7a076c4d2d2cc7f22ead05f405186d80) )
	ROM_LOAD32_BYTE( "93166-29.v10", 0x000000, 0x80000, CRC(9211c82a) SHA1(0aa3f93293b81e0f66b985046eb5e91708693959) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "94019-02.1",  0x000002, 0x200000, CRC(f9d692f2) SHA1(666df55d26e00be39073173fa3616ac9dafbe615) )
	ROM_LOAD32_WORD( "94019-01.13", 0x000000, 0x200000, CRC(1ddfe825) SHA1(27fbf492fdb4f0b4b8db18330e840c130213e15e) )
	ROM_LOAD32_WORD( "94019-04.2",  0x400002, 0x200000, CRC(24ca77ec) SHA1(a5c575224ab276cbed5785f40fc0d35dd2748e74) )
	ROM_LOAD32_WORD( "94019-03.14", 0x400000, 0x200000, CRC(b26426c4) SHA1(2d95137edea7d0c380dd8fd99852254ad3e4c837) )
	ROM_LOAD32_WORD( "94019-06.3",  0x800002, 0x200000, CRC(c8aa4b57) SHA1(55da6a43ba6f0cb32f5d024f67cae21d04019d2a) )
	ROM_LOAD32_WORD( "94019-05.15", 0x800000, 0x200000, CRC(1da63eb4) SHA1(8193ad27ddfe6ba242b73082d1b4a400e88401ba) )
	ROM_LOAD32_WORD( "94019-08.4",  0xc00002, 0x200000, CRC(4b07edc9) SHA1(22aaa923a94a7bec997d2adabc8ec2c7696c33a5) )
	ROM_LOAD32_WORD( "94019-07.16", 0xc00000, 0x200000, CRC(34f471a8) SHA1(4c9c358a9bfdb436a211caa14d085e631609681d) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019-09.11", 0x000000, 0x200000, CRC(cde7bb6f) SHA1(47454dac4ce67ce8d7e0c5ef3a732477ac8170a7) )

	ROM_REGION( 0x100000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019-12.10", 0x000000, 0x100000, CRC(15ae05d9) SHA1(ac00d3766c42ccba4585b9acfacc81bcb940ac26) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "93166-30.bin", 0x000000, 0x080000, CRC(0c738883) SHA1(e552c1842d759e5e617eb9c6cc178620a461b4dd) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "93166-21.bin", 0x000000, 0x040000, CRC(e7fd1bf4) SHA1(74567530364bfd93bffddb588758d8498e197668) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "94019-10.22", 0x000000, 0x200000, CRC(745d41ec) SHA1(9118d0f27b65c9d37970326ccf86fdccb81d32f5) )
	ROM_LOAD( "94019-11.23", 0x200000, 0x200000, CRC(021dc350) SHA1(c71936091f86440201fdbdc94b0d1d22c4018188) )
ROM_END

ROM_START( desertwr )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "93166-26.37", 0x000003, 0x80000, CRC(582b9584) SHA1(027a987cde7e9e1b24aef6a3086eba61679ad0b6) )
	ROM_LOAD32_BYTE( "93166-27.38", 0x000002, 0x80000, CRC(cb60dda3) SHA1(0499b8ab19abdf8db8c18d778b3f9f6e0d277ff0) )
	ROM_LOAD32_BYTE( "93166-28.39", 0x000001, 0x80000, CRC(0de40efb) SHA1(c49c3b27939e428dec1f642b7fdb9a1ff760289a) )
	ROM_LOAD32_BYTE( "93166-29.40", 0x000000, 0x80000, CRC(fc25eae2) SHA1(a4d47fcb4d4c3285cf67d77d8a21478f344b98ca) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "94038-01.20", 0x000000, 0x200000, CRC(f11f83e2) SHA1(e3c99e6583003210483163c79182cb14aa334702) )
	ROM_LOAD32_WORD( "94038-02.3",  0x000002, 0x200000, CRC(3d1fa710) SHA1(5fae3e8c714cca88e22e432dd7275c6898c631a8) )
	ROM_LOAD32_WORD( "94038-03.21", 0x400000, 0x200000, CRC(84fd5790) SHA1(6187ff32a63f3b4105ea875f52237f0d4314f8b6) )
	ROM_LOAD32_WORD( "94038-04.4",  0x400002, 0x200000, CRC(b9ef5b78) SHA1(e2f160a93532f67948a557db717d44c926ae0e49) )
	ROM_LOAD32_WORD( "94038-05.22", 0x800000, 0x200000, CRC(feee1b8d) SHA1(39e1d424dd56257139ab5ab8e897caa1c9cd4e71) )
	ROM_LOAD32_WORD( "94038-06.5",  0x800002, 0x200000, CRC(d417f289) SHA1(39cca11989bb5dc95ef03013d23a7c100bcb36ab) )
	ROM_LOAD32_WORD( "94038-07.23", 0xc00000, 0x200000, CRC(426f4193) SHA1(98a16a70c225d7cd061fcd6e88992d393e6ef9fd) )
	ROM_LOAD32_WORD( "94038-08.6",  0xc00002, 0x200000, CRC(f4088399) SHA1(9d53880996f85776815840bca1f8c3958de4c275) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94038-11.13", 0x000000, 0x200000, CRC(bf2ec3a3) SHA1(904cd5ab2e855bdb94bc70efa6db42af672337d7) )
	ROM_LOAD( "94038-12.14", 0x200000, 0x200000, CRC(d0e113da) SHA1(57f27cbd58421a0afe724fec5628c4a29aad8868) )

	ROM_REGION( 0x400000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94038-09.12", 0x000000, 0x200000, CRC(72ec1ce7) SHA1(88bd9ca3aa7a6410e8fcf6fd70304f12724653bb) )
	ROM_LOAD( "94038-10.11", 0x200000, 0x200000, CRC(1e17f2a9) SHA1(19e6be1daa157593fbab84149f1f739dd39c9226) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "93166-30.41", 0x000000, 0x080000, CRC(980ab89c) SHA1(8468fc13a5988e25750e8d99ff464f46e86ab412) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "93166-21.30", 0x000000, 0x040000, CRC(9300be4c) SHA1(a8e9c1704abf26545aeb9a5d28fd0cafd38f2d84) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "92042-01.33", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "94038-13.34", 0x200000, 0x200000, CRC(b0cac8f2) SHA1(f7d2e32d9c2f301341f7c02678c2c1e09ce655ba) )
ROM_END

ROM_START( f1superb )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "f1sb26.bin", 0x000003, 0x80000, CRC(042fccd5) SHA1(4a69de3aef51adad502d54987468170b9e7bb8ac) )
	ROM_LOAD32_BYTE( "f1sb27.bin", 0x000002, 0x80000, CRC(5f96cf32) SHA1(c9c64576a8bb81a8e8bbe30b054ed33afd760b93) )
	ROM_LOAD32_BYTE( "f1sb28.bin", 0x000001, 0x80000, CRC(cfda8003) SHA1(460146556f606bf213d7e2ab29d2eb8827131bd0) )
	ROM_LOAD32_BYTE( "f1sb29.bin", 0x000000, 0x80000, CRC(f21f1481) SHA1(97a97ff3b9a71b1a024d8f2cfe57a1d02cec5ea4) )

	ROM_REGION( 0x2000000, REGION_GFX1, 0 ) /* 8x8 not all? */
	ROM_LOAD32_WORD( "f1sb1.bin",  0x0000002, 0x200000, CRC(53a3a97b) SHA1(c8cd501ae10d9eb48a02e4e59a1ce389a27afc44) )
	ROM_LOAD32_WORD( "f1sb13.bin", 0x0000000, 0x200000, CRC(36565a99) SHA1(db08ff3107dcc07ca31140715d7d4b7bd27fa4c5) )
	ROM_LOAD32_WORD( "f1sb2.bin",  0x0400002, 0x200000, CRC(a452f50a) SHA1(3782a37203b652ea5df5b04dc74a0fdace7b14cc) )
	ROM_LOAD32_WORD( "f1sb14.bin", 0x0400000, 0x200000, CRC(c0c20490) SHA1(9e93beae38c5cfca9f381b4d134c1d95cfa2223a) )
	ROM_LOAD32_WORD( "f1sb3.bin",  0x0800002, 0x200000, CRC(265d068c) SHA1(a2a7850fbc2a04e448f772544d2f6925f9aaf99c) )
	ROM_LOAD32_WORD( "f1sb15.bin", 0x0800000, 0x200000, CRC(575a146e) SHA1(bf67a89ac3145d38564b9dbc3c650c9d5f2bcd92) )
	ROM_LOAD32_WORD( "f1sb4.bin",  0x0c00002, 0x200000, CRC(0ccc66fd) SHA1(07ffef821300386224a7743e8f83088e3437c6db) )
	ROM_LOAD32_WORD( "f1sb16.bin", 0x0c00000, 0x200000, CRC(a2d017a1) SHA1(6c1dd67a1c9c18d69f7dbf7d4637671809be5c89) )
	ROM_LOAD32_WORD( "f1sb5.bin",  0x1000002, 0x200000, CRC(bff4271b) SHA1(dc5672f51348fe0a79352eeafaeeefd78428fe5a) )
	ROM_LOAD32_WORD( "f1sb17.bin", 0x1000000, 0x200000, CRC(2b9739d5) SHA1(db9e93fe79dfa041584730df9cc678caad073251) )
	ROM_LOAD32_WORD( "f1sb6.bin",  0x1400002, 0x200000, CRC(6caf48ec) SHA1(18ea445970285950c61c8eff74a3ab6387816990) )
	ROM_LOAD32_WORD( "f1sb18.bin", 0x1400000, 0x200000, CRC(c49055ff) SHA1(6038dc497583229060ad686090d6940c0a3d1558) )
	ROM_LOAD32_WORD( "f1sb7.bin",  0x1800002, 0x200000, CRC(a5458947) SHA1(7743d67a167f6eecfac6614d1336c6df425e5e95) )
	ROM_LOAD32_WORD( "f1sb19.bin", 0x1800000, 0x200000, CRC(b7cacf0d) SHA1(cea52b1062cf154ccba11640798902b0f9ddeb77) )
	ROM_LOAD32_WORD( "f1sb8.bin",  0x1c00002, 0x200000, CRC(ba3f1533) SHA1(3ff1c4cca8358fc8daf0d2c381672569085ac9ae) )
	ROM_LOAD32_WORD( "f1sb20.bin", 0x1c00000, 0x200000, CRC(fa349897) SHA1(31e08aa2821e409057e3094333b9ecbe04a6a38a) )

	ROM_REGION( 0x800000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "f1sb9.bin",  0x000000, 0x200000, CRC(66b12e1f) SHA1(4dc3f162a5116403cc0c491af335208672c8e9af) )
	ROM_LOAD( "f1sb10.bin", 0x200000, 0x200000, CRC(893d7f4b) SHA1(b2734f20f703a0dcf8b1fdaebf2b6198b2fb0f51) )
	ROM_LOAD( "f1sb11.bin", 0x400000, 0x200000, CRC(0b848bb5) SHA1(e4c0e434add151112352d6068e5de1a7098e6346) )
	ROM_LOAD( "f1sb12.bin", 0x600000, 0x200000, CRC(edecd5f4) SHA1(9b86802d08e5c8ec1a6dcea75dc8f050d3e96970) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "f1sb31.bin", 0x000000, 0x200000, CRC(1d0d2efd) SHA1(e6446ef9c71be9316c286157f71e0043347c6a5c) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "f1sb32.bin", 0x000000, 0x080000, CRC(1b31fcce) SHA1(354cc6f43cd3bf3ba921ac8c5631ab993bedf563) )

	ROM_REGION( 0x800000, REGION_USER1, 0 ) /* extra ROMs, unknown */
	ROM_LOAD( "f1sb2b.bin", 0x000000, 0x200000, CRC(18d73b16) SHA1(f06f4d31b15658cc1e1b559ae3b8a90b797546ca) )
	ROM_LOAD( "f1sb3b.bin", 0x200000, 0x200000, CRC(ce728fe0) SHA1(e0fd7b8f4d3dc9e2b15ddf027c61e67e5c1f22b5) )
	ROM_LOAD( "f1sb4b.bin", 0x400000, 0x200000, CRC(077180c5) SHA1(ab16739da709ecdbbb1264beba349ef6ecf3f8b1) )
	ROM_LOAD( "f1sb5b.bin", 0x600000, 0x200000, CRC(efabc47d) SHA1(195afde8a1f45da4fc04c3080a3cf5fdfff7be5e) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "f1sb21.bin", 0x000000, 0x040000, CRC(e131e1c7) SHA1(33f95a074930c49548069518d8c6dcde7fa25627) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "f1sb24.bin", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "f1sb23.bin", 0x200000, 0x200000, CRC(bfefa3ab) SHA1(7770cc9b091e258ede7f2780df61a592cc008dd7) )
ROM_END

ROM_START( gratia )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "94019.026", 0x000003, 0x80000, CRC(f398cba5) SHA1(11e06abebfdfc8a99b5c56e9f6ed389f645b6c72) )
	ROM_LOAD32_BYTE( "94019.027", 0x000002, 0x80000, CRC(ba3318c5) SHA1(9b100988b998c39b586b51fe9fee874dbf711610) )
	ROM_LOAD32_BYTE( "94019.028", 0x000001, 0x80000, CRC(e0762e89) SHA1(a567c347e7f73f1ef1c753d14ac4f58311380fac) )
	ROM_LOAD32_BYTE( "94019.029", 0x000000, 0x80000, CRC(8059800b) SHA1(7548d01b6ea15e962353b3585db6515e5819e5ce) )

	ROM_REGION( 0x0c00000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "94019.01", 0x000000, 0x200000, CRC(92d8ae9b) SHA1(02b36e6e14b28a9830e07fd328772dbb20b76889) )
	ROM_LOAD32_WORD( "94019.02", 0x000002, 0x200000, CRC(f7bd9cc4) SHA1(5658bfb4081439ab06c6ade2531581aa60d1c6be) )
	ROM_LOAD32_WORD( "94019.03", 0x400000, 0x200000, CRC(62a69590) SHA1(d95cc1e1ec85161ee6cd1ae77b405cf8ef81217a) )
	ROM_LOAD32_WORD( "94019.04", 0x400002, 0x200000, CRC(5a76a39b) SHA1(fc7c1ff9a0a3c2639fc52720aefe8b2a9e5d2857) )
	ROM_LOAD32_WORD( "94019.05", 0x800000, 0x200000, CRC(a16994df) SHA1(9170b1fd9252d7a9601c3b2e6b1ba86686730b86) )
	ROM_LOAD32_WORD( "94019.06", 0x800002, 0x200000, CRC(01d52ef1) SHA1(1585c7eb3729bab78467f627b7b671d619451a74) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019.08", 0x000000, 0x200000, CRC(abd124e0) SHA1(2da1d818c909e4abbb79ed03f3dbf15d744439ce) )
	ROM_LOAD( "94019.09", 0x200000, 0x200000, CRC(711ab08b) SHA1(185b80b965ac3aba4857b4f83637008c2c1cc6ff) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019_2.07", 0x000000, 0x200000, CRC(043f969b) SHA1(ad10339e561c1a65451a2e9a8e79ceda3787674a) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019_2.030",0x000000, 0x080000, CRC(f9543fcf) SHA1(8466c7893bc6c43e2a80b8f91a776fd0a345ea6c) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "94019.021",0x000000, 0x040000, CRC(6e8dd039) SHA1(f1e69c9b40b14ba0f8377a6d9b6c3933919bc803) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "92042.01", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common rom?
	ROM_LOAD( "94019.10", 0x200000, 0x200000, CRC(a751e316) SHA1(3d658370c71b83582fd132b3da441089df9bfd05) )
ROM_END

ROM_START( gratiaa )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "94019.026", 0x000003, 0x80000, CRC(f398cba5) SHA1(11e06abebfdfc8a99b5c56e9f6ed389f645b6c72) )
	ROM_LOAD32_BYTE( "94019.027", 0x000002, 0x80000, CRC(ba3318c5) SHA1(9b100988b998c39b586b51fe9fee874dbf711610) )
	ROM_LOAD32_BYTE( "94019.028", 0x000001, 0x80000, CRC(e0762e89) SHA1(a567c347e7f73f1ef1c753d14ac4f58311380fac) )
	ROM_LOAD32_BYTE( "94019.029", 0x000000, 0x80000, CRC(8059800b) SHA1(7548d01b6ea15e962353b3585db6515e5819e5ce) )

	ROM_REGION( 0x0c00000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "94019.01", 0x000000, 0x200000, CRC(92d8ae9b) SHA1(02b36e6e14b28a9830e07fd328772dbb20b76889) )
	ROM_LOAD32_WORD( "94019.02", 0x000002, 0x200000, CRC(f7bd9cc4) SHA1(5658bfb4081439ab06c6ade2531581aa60d1c6be) )
	ROM_LOAD32_WORD( "94019.03", 0x400000, 0x200000, CRC(62a69590) SHA1(d95cc1e1ec85161ee6cd1ae77b405cf8ef81217a) )
	ROM_LOAD32_WORD( "94019.04", 0x400002, 0x200000, CRC(5a76a39b) SHA1(fc7c1ff9a0a3c2639fc52720aefe8b2a9e5d2857) )
	ROM_LOAD32_WORD( "94019.05", 0x800000, 0x200000, CRC(a16994df) SHA1(9170b1fd9252d7a9601c3b2e6b1ba86686730b86) )
	ROM_LOAD32_WORD( "94019.06", 0x800002, 0x200000, CRC(01d52ef1) SHA1(1585c7eb3729bab78467f627b7b671d619451a74) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019.08", 0x000000, 0x200000, CRC(abd124e0) SHA1(2da1d818c909e4abbb79ed03f3dbf15d744439ce) )
	ROM_LOAD( "94019.09", 0x200000, 0x200000, CRC(711ab08b) SHA1(185b80b965ac3aba4857b4f83637008c2c1cc6ff) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019.07", 0x000000, 0x200000, BAD_DUMP CRC(acb75824) SHA1(3b43e00a2d240761565042c8feead25a83ef0eb1)  )	// FIXED BITS (xxxxxxxx11111111)

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "94019.030",0x000000, 0x080000, CRC(026b5379) SHA1(b9237477f1bf8ae83174e8231492fe667e6d6a13) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "94019.021",0x000000, 0x040000, CRC(6e8dd039) SHA1(f1e69c9b40b14ba0f8377a6d9b6c3933919bc803) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "92042.01", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common rom?
	ROM_LOAD( "94019.10", 0x200000, 0x200000, CRC(a751e316) SHA1(3d658370c71b83582fd132b3da441089df9bfd05) )
ROM_END

ROM_START( gametngk )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "mr94041.26", 0x000003, 0x80000, CRC(e622e774) SHA1(203c2a3563a337af4cec92a66e0fa410d901b01f) )
	ROM_LOAD32_BYTE( "mr94041.27", 0x000002, 0x80000, CRC(da862b9c) SHA1(17dc6da08d7f5551c8f4bc4d9c416dbfc82d8397) )
	ROM_LOAD32_BYTE( "mr94041.28", 0x000001, 0x80000, CRC(b3738934) SHA1(cd07572e55e83807e76179cfc6b97e0410067911) )
	ROM_LOAD32_BYTE( "mr94041.29", 0x000000, 0x80000, CRC(45154a45) SHA1(4c7c2c6738fdfe54ebe41a0ac6222cbfce5d7757) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr94041.01", 0x0000000, 0x200000, CRC(3f99adf7) SHA1(cbb8d2fc253b0c58e7eb9286c66e6b36daf9d4af) )
	ROM_LOAD32_WORD( "mr94041.02", 0x0000002, 0x200000, CRC(c3c5ae69) SHA1(5ed7f57a7139f87c680c68e44ea4022b917a9381) )
	ROM_LOAD32_WORD( "mr94041.03", 0x0400000, 0x200000, CRC(d858b6de) SHA1(a06cf529c9508c8c8508894e2e004373edd9debf) )
	ROM_LOAD32_WORD( "mr94041.04", 0x0400002, 0x200000, CRC(8c96ca20) SHA1(097cab64ef8e515c59178c36171f87bed4b3d1e5) )
	ROM_LOAD32_WORD( "mr94041.05", 0x0800000, 0x200000, CRC(ac664a0b) SHA1(bd002822a38369599a1b5a7456957de1d9cd976e) )
	ROM_LOAD32_WORD( "mr94041.06", 0x0800002, 0x200000, CRC(70dd0dd4) SHA1(da648c16ad0cb12ac66656522da14392be7772c9) )
	ROM_LOAD32_WORD( "mr94041.07", 0x0c00000, 0x200000, CRC(a6966af5) SHA1(3a65824f3f325af39d8e9932357ce9f8878f0321) )
	ROM_LOAD32_WORD( "mr94041.08", 0x0c00002, 0x200000, CRC(d7d2f73a) SHA1(0eb28f4cdea73aa8fed0b62cbac6cd7d7694c2ee) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr94041.11", 0x000000, 0x200000, CRC(00dcdbc3) SHA1(f7e34bc9f714ea81fc9855d90db792dd1e99bae8) )
	ROM_LOAD( "mr94041.12", 0x200000, 0x200000, CRC(0ce48329) SHA1(9c198cef998eb3b9c33123bd8cc02210498f82d9) )

	ROM_REGION( 0x400000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr94041.09", 0x000000, 0x200000, CRC(a33e6051) SHA1(d6e34b022eb36dcfa8cfe6d6d1254f994b3b3dca) )
	ROM_LOAD( "mr94041.10", 0x200000, 0x200000, CRC(b3497147) SHA1(df7d8ea7ec3e3df5e0c6658f14995df5479181bf) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr94041.30", 0x000000, 0x080000, CRC(c0f27b7f) SHA1(874fe80aa4b46520f844ef6efa61f28eabccbc4f) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "mr94041.21", 0x000000, 0x040000, CRC(38dcb837) SHA1(29fdde54e52dec4ee39a6f2db8e0d67774320d15) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "mr94041.13", 0x000000, 0x200000, CRC(fba84caf) SHA1(318270dbf825a8e0f315992c49a2dc34dd1df7c1) )
	ROM_LOAD( "mr94041.14", 0x200000, 0x200000, CRC(2d6308bd) SHA1(600b6ccdbb976301075e0b287124a9fd5fe7fc1b) )
ROM_END

ROM_START( hayaosi2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "mb93138a.25", 0x000003, 0x80000, CRC(563c6f2f) SHA1(bc2a61fd2e0adf58256feeef8491b67af6d6eacf) )
	ROM_LOAD32_BYTE( "mb93138a.27", 0x000002, 0x80000, CRC(fe8e283a) SHA1(fc6c06ae296110b1f5794187d5208b17541614cb) )
	ROM_LOAD32_BYTE( "mb93138a.29", 0x000001, 0x80000, CRC(e6fe3d0d) SHA1(9a0caab82b160991b4f2ac993e7e4b4c5d3bb15e) )
	ROM_LOAD32_BYTE( "mb93138a.31", 0x000000, 0x80000, CRC(d944bf8c) SHA1(ce93b5d2ebe886b38dc42b1e554b17dc951a51b4) )

	ROM_REGION( 0x900000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr93038.04",  0x000000, 0x200000, CRC(ab5edb11) SHA1(b7742aefbce9efc512c3526714b6f20a6c03af60) )
	ROM_LOAD32_WORD( "mr93038.05",  0x000002, 0x200000, CRC(274522f1) SHA1(717435d6bf1b2a2220a2f0a53b070bb81cc2ed2b) )
	ROM_LOAD32_WORD( "mr93038.06",  0x400000, 0x200000, CRC(f9961ebf) SHA1(e91b160cb5a76e3f6044cc71681dadf2fbff7e8b) )
	ROM_LOAD32_WORD( "mr93038.07",  0x400002, 0x200000, CRC(1abef1c5) SHA1(4b40adaebf9d9963493bfb285badbb19a5b181be) )
	ROM_LOAD32_WORD( "mb93138a.15", 0x800000, 0x080000, CRC(a5f64d87) SHA1(11bf017f700faba57a5a2edced7a5d81a581bc50) )
	ROM_LOAD32_WORD( "mb93138a.3",  0x800002, 0x080000, CRC(a2ae2b21) SHA1(65cee4e5e0a9b8dcac578e34210e1af7d7b2e6f7) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr93038.03",  0x000000, 0x200000, CRC(6999dec9) SHA1(eb4c6ba200cd08b41509314c659feb3af12117ee) )

	ROM_REGION( 0x100000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr93038.08",  0x000000, 0x100000, CRC(21282cb0) SHA1(52ea94a6457f7684674783c362052bcc40086dd0) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mb93138a.32", 0x000000, 0x080000, CRC(f563a144) SHA1(14d86e4992329811857e1faf282cd9ec530a364c) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "mb93138a.21", 0x000000, 0x040000, CRC(8e8048b0) SHA1(93285a0570ed829b36f4e8c57d133a7dd14f123d) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr92042.01",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr93038.01",  0x200000, 0x200000, CRC(b8a38bfc) SHA1(1aa7b69beebceb6f09a1ee006de054cb84002e94) )
ROM_END

/*

Hayaoshi Quiz Nettou Namahousou
(c)1993 Jaleco

MB-93140A EB91022-20079-1 (Motherboard. Mega System 32?)
MB-93138A EB91022-20078-1 (ROM board)
SE-93139 EB91022-30056 (extended board on ROM board)

CPU  : NEC JAPAN D70632GD-20 (V70)
Sound: Z80 YMF271-F
OSC  : 48.0000MHz (OSC1) 40.0000MHz (OSC2) 16.9344MHz (X1)

ROMs:
MR94027.02 (16M mask, location IC 3( 1)) [59976568]
MR94027.04 (16M mask, location IC 4( 2)) [6a16d13a]
MR94027.06 (16M mask, location IC 5( 3)) [1618785a]
MR94027.08 (16M mask, location IC 6( 4)) [753b05e0]

MR94027.09 (16M mask, location IC11( 9)) [32ead437]
MR94027.11 (16M mask, location IC13(11)) [b65d5096]

MR94027.01 (16M mask, location IC20(13)) [c72e5c6e]
MR94027.03 (16M mask, location IC21(14)) [3ff68f4f]
MR94027.05 (16M mask, location IC22(15)) [59545977]
MR94027.07 (16M mask, location IC23(16)) [c66099c4]

MB93138.21 (M27C2001, location IC30(21)) [008bc217] Ver1.0
actual label is "MB-93138 21 Ver1.0"

MR92042.01 (16M mask, location IC33(22)) [0fa26f65]
MR94027.10 (16M mask, locaiton IC34(23)) [e7cabe41]

MB93138.25 (M27C4001, location IC36(25)) [ba8cec03] Ver1.5
MB93138.27 (M27C4001, location IC38(27)) [571725df] Ver1.5
MB93138.29 (M27C4001, location IC40(29)) [da891976] Ver1.5
MB93138.31 (M27C4001, location IC42(31)) [2d17bb06] Ver1.5
actual label is "???????N?C?Y?M?????????IMB-93138-?? Ver.1.5"

MB93138.32 (M27C4001, location IC43(32)) [df5d00b4] Ver1.0
actual label is "MB-93138 32 Ver1.0"


PALs (not dumped):
91022-01.2 (18CV8,  IC83(2))
91022-02.1 (22CV10, IC62(1))

Custom chips:
SS91022-01 9348 ACBA (IC36, 208pin PQFP)
SS91022-02 9350 IAHA (IC 9, 100pin PQFP)
SS91022-03 9343EX006 (IC11, 176pin PQFP)
SS91022-05 9347EX002 (IC31, 120pin PQFP)
SS91022-07 9345EV 450881 06440 (IC70, 208pin PQFP)

GS91022-01 9340EK002 (IC46, 120pin PQFP)
GS91022-02 9334EK709 (IC6,  160pin PQFP)
GS91022-03 9335PP711 (IC7,  100pin PQFP)
GS91022-04 9334PP712 (IC24, 100pin PQFP)

SS92046-01 9338EV 436091 06441 (IC1 of EB91022-30056, 144pin PQFP)

Others:
Lithium battery + LH5168D-10L(SRAM)

*/


ROM_START( hayaosi3 )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "mb93138.25", 0x000003, 0x80000, CRC(ba8cec03) SHA1(edaa52e0b07307bb21168205ee0d5d6ff8168de9) )
	ROM_LOAD32_BYTE( "mb93138.27", 0x000002, 0x80000, CRC(571725df) SHA1(66575ec1a29d6fc1b50ae5a5ce8025bb1043deaf))
	ROM_LOAD32_BYTE( "mb93138.29", 0x000001, 0x80000, CRC(da891976) SHA1(27e8c395e92ca01b47bffdf766bc95a6c2150815) )
	ROM_LOAD32_BYTE( "mb93138.31", 0x000000, 0x80000, CRC(2d17bb06) SHA1(623b603c4002734427c882424a1e0dc889cf7e02) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr94027.01",  0x000000, 0x200000, CRC(c72e5c6e) SHA1(b98cd656c48c775953d00b5d8bafd4ffde76d8df) )
	ROM_LOAD32_WORD( "mr94027.02",  0x000002, 0x200000, CRC(59976568) SHA1(a280c352d612913834c76b8e23d86c937fd21281) )
	ROM_LOAD32_WORD( "mr94027.03",  0x400000, 0x200000, CRC(3ff68f4f) SHA1(1e367b92560c32c87e27fc0e99be3bdb5eb0510b) )
	ROM_LOAD32_WORD( "mr94027.04",  0x400002, 0x200000, CRC(6a16d13a) SHA1(65a7751c248c966fd01149418ce6bedba7a0d48a) )
	ROM_LOAD32_WORD( "mr94027.05",  0x800000, 0x200000, CRC(59545977) SHA1(2e0a83efd7ae210c0b4360e9572dd7eec38cd974) )
	ROM_LOAD32_WORD( "mr94027.06",  0x800002, 0x200000, CRC(1618785a) SHA1(3f2698d07a52947429313a78ebcedfdae478efd7) )
	ROM_LOAD32_WORD( "mr94027.07",  0xc00000, 0x200000, CRC(c66099c4) SHA1(5a6edffa39a98f38cc3cffbad9191fb2e794a812) )
	ROM_LOAD32_WORD( "mr94027.08",  0xc00002, 0x200000, CRC(753b05e0) SHA1(0424e92b32a73c27ecb549e6e9449446ea938e40) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr94027.09",  0x000000, 0x200000, CRC(32ead437) SHA1(b94175cf186b4ebcc180a4c092d2ffcdd9ff3b1d) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr94027.11",  0x000000, 0x200000, CRC(b65d5096) SHA1(2c4e1e3e9f96be8369cb2de142a82f94506f85c0) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mb93138.32", 0x000000, 0x080000, CRC(df5d00b4) SHA1(2bbbcd546d5b5170d81bf33b37b46b70b417c9c7) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "mb93138.21", 0x000000, 0x040000, CRC(008bc217) SHA1(eec66a86f285ccbc47eba17a4bb83cc1f8a5f425) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr92042.01",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr94027.10",  0x200000, 0x200000, CRC(e7cabe41) SHA1(5d903baed690a98856f7581319cf4dbfe1db47bb) )
ROM_END

ROM_START( kirarast )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "mr95025.26", 0x000003, 0x80000, CRC(eb7faf5f) SHA1(5b79ff3043db5ef2622ae1665145462d949c9bb8) )
	ROM_LOAD32_BYTE( "mr95025.27", 0x000002, 0x80000, CRC(80644d05) SHA1(6da8bf8aeb1477112f9022c0c5f472cbcd27df8e) )
	ROM_LOAD32_BYTE( "mr95025.28", 0x000001, 0x80000, CRC(6df8c384) SHA1(3ad01d3d51cfc1f48029c16ee1cc74fc59d7603c) )
	ROM_LOAD32_BYTE( "mr95025.29", 0x000000, 0x80000, CRC(3b6e681b) SHA1(148fa10631db53a4ad1dcdfb60b4f0654e077396) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr95025.01",  0x000000, 0x200000, CRC(02279069) SHA1(fb3ce00701271d0163f72e4f2e56faa9f16d8fd0) )
	ROM_LOAD32_WORD( "mr95025.02",  0x000002, 0x200000, CRC(885161d4) SHA1(1bc82de0b2759758d437db3ef9f0f7805f759b59) )
	ROM_LOAD32_WORD( "mr95025.03",  0x400000, 0x200000, CRC(1ae06df9) SHA1(e1493a386fd8c54c88afab43d13d73869ae467ee) )
	ROM_LOAD32_WORD( "mr95025.04",  0x400002, 0x200000, CRC(91ab7006) SHA1(0b99c352a696e21b2f31207cbf9b4a64edf543ce) )
	ROM_LOAD32_WORD( "mr95025.05",  0x800000, 0x200000, CRC(e61af029) SHA1(685315e833a168383c4c5cdaf72de172f14995b6) )
	ROM_LOAD32_WORD( "mr95025.06",  0x800002, 0x200000, CRC(63f64ffc) SHA1(a2a109be24b5f1ec2e41e423d4194394ea8c3c8b) )
	ROM_LOAD32_WORD( "mr95025.07",  0xc00000, 0x200000, CRC(0263a010) SHA1(b9c85647b406c89f0e839eac93eaf5d2e6963f7d) )
	ROM_LOAD32_WORD( "mr95025.08",  0xc00002, 0x200000, CRC(8efc00d6) SHA1(f750e0e21310ceceeae3ad80eb2fe2920f5a0076) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95025.10",  0x000000, 0x200000, CRC(ba7ad413) SHA1(b1f1c218dea3217f21d5e2f44db3786055ed879a) )
	ROM_LOAD( "mr95025.11",  0x200000, 0x200000, CRC(11557299) SHA1(6efa56f897ab026f965376a0d4032f7a0d20cafe) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95025.09",  0x000000, 0x200000, CRC(ca6cbd17) SHA1(9d16ef187b062590315066218e89bdf33cfd9865) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95025.30",  0x000000, 0x080000, CRC(aee6e0c2) SHA1(dee985f7a9773ba7a4d31a3833a7775d778bbe5a) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "mr95025.21",  0x000000, 0x040000, CRC(a6c70c7f) SHA1(fe2108f3e8d46ed53d8c5c98e8d0fdb19b77075d) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr95025.12",  0x000000, 0x200000, CRC(1dd4f766) SHA1(455befd3a216f2197cd2e7e4899d4f1af7d20bf7) )
	ROM_LOAD( "mr95025.13",  0x200000, 0x200000, CRC(0adfe5b8) SHA1(02309e5789b58896e5f68603502c76d4a917bd91) )
ROM_END

ROM_START( akiss )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "93166.26", 0x000003, 0x80000, CRC(5bdd01ee) SHA1(21b8e07bb7ef6b437a43719b02deeba970330900) )
	ROM_LOAD32_BYTE( "93166.27", 0x000002, 0x80000, CRC(bb11b2c9) SHA1(86ba06d28bc8f560ac3d05515d061e05c90d1628) )
	ROM_LOAD32_BYTE( "93166.28", 0x000001, 0x80000, CRC(20565478) SHA1(d532ab55be287f45d8d81317bb844c675eb1292c) )
	ROM_LOAD32_BYTE( "93166.29", 0x000000, 0x80000, CRC(ff454f0d) SHA1(db81aaaf4160eb62badbe08fc01543463470ac97) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "95008-01.13", 0x000000, 0x200000, CRC(1be66420) SHA1(9fc85e6108f230418e012ad05586010235139039))
	ROM_LOAD32_WORD( "95008-02.1",  0x000002, 0x200000, CRC(1cc4808e) SHA1(70a19d66b4f187320c67760bc453b6afb7d66f9a) )
	ROM_LOAD32_WORD( "95008-03.14", 0x400000, 0x200000, CRC(4045f0dc) SHA1(5ba9786618ecad9410dbdf3664f9dda848a754f7) )
	ROM_LOAD32_WORD( "95008-04.2",  0x400002, 0x200000, CRC(ef3c139d) SHA1(3de374e77443dd4e967dbb5da820fe1c8c78aa1b) )
	ROM_LOAD32_WORD( "95008-05.15", 0x800000, 0x200000, CRC(43ea4a84) SHA1(d9d9898edcf432998ed6b9a1622812def45cf369) )
	ROM_LOAD32_WORD( "95008-06.3",  0x800002, 0x200000, CRC(24f23d4e) SHA1(8a7b6f28f25227391df73edb096695c5fe8df7dc))
	ROM_LOAD32_WORD( "95008-07.16", 0xc00000, 0x200000, CRC(bf47747e) SHA1(b97121953f41039182e25ea023802df4524cf9bd) )
	ROM_LOAD32_WORD( "95008-08.4",  0xc00002, 0x200000, CRC(34829a09) SHA1(7229c56fee53a9d4d29cf0c9dec471b6cc4dc30b) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "95008-10.11",  0x000000, 0x200000, CRC(52da2e9e) SHA1(d7a29bdd1c6801aa8d36bc098e75091c63ba0766) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "95008-09.10",  0x000000, 0x200000,CRC(7236f6a0) SHA1(98dbb55f08d669ef3bf69394bb9739d0e6137fcb) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "93166.30",  0x000000, 0x080000, CRC(1807c1ea) SHA1(94696b8319c4982cb5d33423f56e2348f210cdb5) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "93166.21",  0x000000, 0x040000, CRC(01a03687) SHA1(2340c4ed19f434e8c23709edfc93259313aefaf9))
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "95008-11.22",  0x000000, 0x200000, CRC(23b9af76) SHA1(98b4087c142500dc759bda94d71c77634452a7ad))
	ROM_LOAD( "95008-12.23",  0x200000, 0x200000, CRC(780a2f45) SHA1(770cbf04e34ae7d72e6eb2304bcdfaff483cd8c1))
ROM_END

ROM_START( p47aces )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "p47-26.bin", 0x000003, 0x80000, CRC(e017b819) SHA1(942fb48e8bb3a263534a0351a1a9979d786bc475) )
	ROM_LOAD32_BYTE( "p47-27.bin", 0x000002, 0x80000, CRC(bd1b81e0) SHA1(b15f157fe3a30295f999a4c285da2d6f22d7fba6) )
 	ROM_LOAD32_BYTE( "p47-28.bin", 0x000001, 0x80000, CRC(4742a5f7) SHA1(cd297aa150082c545647c9a755cf2cdbdc98c988) )
	ROM_LOAD32_BYTE( "p47-29.bin", 0x000000, 0x80000, CRC(86e17d8b) SHA1(73004f243c6dfb86ce4cc61475dc7caaf452750e) )

	ROM_REGION( 0xe00000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "p47-01.bin",  0x000002, 0x200000, CRC(28732d3c) SHA1(15b2687bcad31793fc7d6a9dc3eccb7ad9b5f659) )
	ROM_LOAD32_WORD( "p47-13.bin",  0x000000, 0x200000, CRC(a6ccf999) SHA1(5d32fb6f6987ede6c125bec9581da4695ad64dff) )
	ROM_LOAD32_WORD( "p47-02.bin",  0x400002, 0x200000, CRC(128db576) SHA1(f6561f54f6b95842a5f14d29682449bf0d837a85) )
	ROM_LOAD32_WORD( "p47-14.bin",  0x400000, 0x200000, CRC(efc52b38) SHA1(589caaaba4e3ddaf41e05f0f12b8d4bc6d63fa5c) )
	ROM_LOAD32_WORD( "p47-03.bin",  0x800002, 0x200000, CRC(324cd504) SHA1(79b3ef3ae0aa14d903113ccf5b57d459c329cf12) )
	ROM_LOAD32_WORD( "p47-15.bin",  0x800000, 0x200000, CRC(ca164b17) SHA1(ea1cb0894632442f40d321b5843125f874768aae) )
	ROM_LOAD32_WORD( "p47-04.bin",  0xc00002, 0x100000, CRC(4b3372be) SHA1(cdc7d7615b6b5d45ca071b2967980dc6c6294ac0) )
	ROM_LOAD32_WORD( "p47-16.bin",  0xc00000, 0x100000, CRC(c23c5467) SHA1(5ff51ecb86ccbae2af160599890e13a7cc70072d) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "p47-11.bin",  0x000000, 0x200000, CRC(c1fe16b3) SHA1(8b9d2483ba06ab8072676e73d949c696535b3d26) )
	ROM_LOAD( "p47-12.bin",  0x200000, 0x200000, CRC(75871325) SHA1(9191263a52ec6ac325cf6130b35be7cdd1ec2f50) )

	ROM_REGION( 0x400000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "p47-10.bin",  0x000000, 0x200000, CRC(a44e9e06) SHA1(ff51796e160d996e931b92049e6214982f270caa) )
	ROM_LOAD( "p47-09.bin",  0x200000, 0x200000, CRC(226014a6) SHA1(090bdc1f6d2b9d33b431dbb49a457a4bb36cd3ad) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "p47-30.bin",  0x000000, 0x080000, CRC(7ba90fad) SHA1(c0a3d4458816f00b8f5eb4b6d4531d1abeaccbe5) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "p47-21.bin",  0x000000, 0x040000, CRC(f2d43927) SHA1(69ac20f339a515d58cafbcd6f7d7982ca5cda681) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "p47-22.bin",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) )
	ROM_LOAD( "p47-23.bin",  0x200000, 0x200000, CRC(547fa4d4) SHA1(8a5ecb3300646762f63d37a27e643e1f6ce5e775) )
ROM_END

ROM_START( tetrisp )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "mr95024.26", 0x000003, 0x80000, CRC(d318a9ba) SHA1(cae86d86518fdfeb736e7b2040277c76cc3b4017) )
	ROM_LOAD32_BYTE( "mr95024.27", 0x000002, 0x80000, CRC(2d69b6d3) SHA1(f0a513f449aa25808672fb27e3691ccabfba48a1) )
	ROM_LOAD32_BYTE( "mr95024.28", 0x000001, 0x80000, CRC(87522e16) SHA1(4f0d8abec046884d89c559e3a4a5ac9e0e47a0dc) )
	ROM_LOAD32_BYTE( "mr95024.29", 0x000000, 0x80000, CRC(43a61941) SHA1(a097c88c45d8486eb6ffdd13904b6eb2a3fa45b9) )

	ROM_REGION( 0x400000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr95024.01", 0x000002, 0x200000, CRC(cb0e92b9) SHA1(179cc9e2d819d7f6238e924184e8a383d172aa72) )
	ROM_LOAD32_WORD( "mr95024.13", 0x000000, 0x200000, CRC(4a825990) SHA1(f99ba9f88f5582259ba0e50480451d4e9d1d03b7) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95024.11", 0x000000, 0x200000, CRC(c0d5246f) SHA1(413285f6b40001281c4fcec1ce73400c3ae610ed) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95024.10", 0x000000, 0x200000, CRC(a03e4a8d) SHA1(d52c78d5e9d874dce514ffb035f2424409d8fb7a) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95024.30", 0x000000, 0x080000, CRC(cea7002d) SHA1(5462edaeb9339790b95ed15a4bfaab8fae655b12) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "mr95024.21", 0x000000, 0x040000, CRC(5c565e3b) SHA1(d349a8ca50d03c06d8978e6d3632b624f019dee4) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "mr95024.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr95024.23", 0x200000, 0x200000, CRC(57502a17) SHA1(ce880188854dc17d9ebbfa3c373469cf5e6858c2) )
ROM_END

ROM_START( tp2m32 )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "tp2m3226.26", 0x000003, 0x80000, CRC(152f0ccf) SHA1(1e318e125a54216ebf3f85740db1dd85aacac819) )
	ROM_LOAD32_BYTE( "tp2m3227.27", 0x000002, 0x80000, CRC(d89468d0) SHA1(023fbc13b0f6332217904c89225b330aa5742f20) )
	ROM_LOAD32_BYTE( "tp2m3228.28", 0x000001, 0x80000, CRC(041aac23) SHA1(3f7863ffa897978493e98445fe020dccbe521752) )
	ROM_LOAD32_BYTE( "tp2m3229.29", 0x000000, 0x80000, CRC(4e83b2ca) SHA1(2766793f050a6952f4f53a763686f95bd7544f3f) )

	ROM_REGION( 0x800000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "tp2m3204.11", 0x000000, 0x200000, CRC(b5a03129) SHA1(a50d8b70615c49216f647534d1658c1a6d58a783) )

	ROM_REGION( 0x400000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "tp2m3203.10", 0x000000, 0x400000, CRC(94af8057) SHA1(e3bc6e02fe4c503ae51284770a76abbeff989147) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "tp2m3230.30", 0x000000, 0x080000, CRC(6845e476) SHA1(61c33714db2e2b5ccdcef0e0d3efdc391fe6aba2) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "tp2m3221.21", 0x000000, 0x040000, CRC(2bcc4176) SHA1(74740fa13ab81b9819b4cfbe9d34a0749ba23b8f) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "tp2m3205.22", 0x000000, 0x200000, CRC(74aa5c31) SHA1(7e3f86198fb678244fab76bee9c72bbdfc818118) )
ROM_END



ROM_START( bnstars ) /* ver 1.1 */
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
 	ROM_LOAD32_BYTE( "vsjanshi26.37", 0x000003, 0x80000, CRC(75eeec8f) SHA1(26315381baa0abb470203dc565ad98c52fe17b20) )
	ROM_LOAD32_BYTE( "vsjanshi27.38", 0x000002, 0x80000, CRC(69f24ab9) SHA1(e019a444111e4ed7f9a378d6e2d13ddb9324bc49) )
	ROM_LOAD32_BYTE( "vsjanshi28.39", 0x000001, 0x80000, CRC(d075cfb6) SHA1(f70741e9f536d5c7604126d36c7aa8ed8f25c329) )
	ROM_LOAD32_BYTE( "vsjanshi29.40", 0x000000, 0x80000, CRC(bc395b50) SHA1(84d7cc492a11a5a9402e929f0bd138ad63e3d079) )

	ROM_REGION( 0x1000000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr96004-01.13", 0x000000, 0x200000, CRC(3366d104) SHA1(2de0cabe2ead777b5b02cade7f2003ef7f90b75b) )
	ROM_LOAD32_WORD( "mr96004-02.1",  0x000002, 0x200000, CRC(ad556664) SHA1(4b36f8d8d9efa37cf515af41d14433e7eafa27a2) )
	ROM_LOAD32_WORD( "mr96004-03.14", 0x400000, 0x200000, CRC(b399e2b1) SHA1(9b6a00a219db8d66dcf592160b7b5f7a86b8f0c9) )
	ROM_LOAD32_WORD( "mr96004-04.2",  0x400002, 0x200000, CRC(f4f4cf4a) SHA1(fe497989cf96c68602f68f14920aed44fd934573) )
	ROM_LOAD32_WORD( "mr96004-05.15", 0x800000, 0x200000, CRC(cd6c357e) SHA1(44cd2d0607c7ccd80f701cf1675fd283acb07252) )
	ROM_LOAD32_WORD( "mr96004-06.3",  0x800002, 0x200000, CRC(fc6daad7) SHA1(99f14ac6b06ad9a8a3d2e9f69b693c7ce420a47d) )
	ROM_LOAD32_WORD( "mr96004-07.16", 0xc00000, 0x200000, CRC(177e32fa) SHA1(3ca1f397dc28f1fa3a4136705b92c63e4e438f05) )
	ROM_LOAD32_WORD( "mr96004-08.4",  0xc00002, 0x200000, CRC(f6df27b2) SHA1(60590976020d86bdccd4eaf57b349ea31bec6830) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr96004-09.11",  0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr96004-11.10", 0x000000, 0x200000,  CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "vsjanshi30.41",  0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "vsjanshi21.30",  0x000000, 0x040000, CRC(d622bce1) SHA1(059fcc3c7216d3ea4f3a4226a06219375ce8c2bf) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.22",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )
ROM_END

/*

World PK Soccer V2
(c)1996 Jaleco

MegaSystem32 with I/O subboard OZ-93155

ROM board:
MB-93138A EB91022-20078-1

25 (actual label is "PK SOCCER V2 ROM 25 VER. 1.1") IC36
27 (actual label is "PK SOCCER V2 ROM 27 VER. 1.1") IC38
29 (actual label is "PK SOCCER V2 ROM 29 VER. 1.1") IC40
31 (actual label is "PK SOCCER V2 ROM 31 VER. 1.1") IC42
32 (actual label is "PK SOCCER V2 ROM 32 VER. 1.1") IC43

MR92042-01.22 (16M mask) IC33
MR92042-08.23 (16M mask) IC34

ws-21 25 (actual label is "MB93138 Ver1.0 WS-21") IC30

MR95033-01.13 (16M mask) IC20
MR95033-02.1  (16M mask) IC3
MR95033-03.14 (16M mask) IC21
MR95033-04.2  (16M mask) IC4
MR95033-05.15 (16M mask) IC22
MR95033-06.3  (16M mask) IC5

MR95033-07.9  (16M mask) IC11

MR95033-09.11 (16M mask) IC13

Daughter board:
SE-93139 EB91022-30056
Custom chip: SS92046-01 9338EV 436091 06441

*/

ROM_START( wpksocv2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "25", 0x000003, 0x80000, CRC(6c22a56c) SHA1(a03cbcfc024b39d2776f9e9897d1da07df6ae2d7) )
	ROM_LOAD32_BYTE( "27", 0x000002, 0x80000, CRC(50c594a8) SHA1(454a63d7b2a07399a64449205271b797bca1dec1) )
	ROM_LOAD32_BYTE( "29", 0x000001, 0x80000, CRC(22acd835) SHA1(0fa96a6dfde737d541842f85dc257776044e15b5) )
	ROM_LOAD32_BYTE( "31", 0x000000, 0x80000, CRC(f25e50f5) SHA1(b58722f11a8b94ef053caf531ac94a959350288a) )

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr95033-01.13", 0x000000, 0x200000, CRC(1f76ed57) SHA1(af9076b4b4c26b362825d892f46d2c04b4bb9d07) )
	ROM_LOAD32_WORD( "mr95033-02.1",  0x000002, 0x200000, CRC(5b119910) SHA1(aff44e355227dd159e388ab85a5b6d48644ff421) )
	ROM_LOAD32_WORD( "mr95033-03.14", 0x400000, 0x200000, CRC(8b6099ed) SHA1(c514cec1491aed00a5714c0b8d17c96e87ba50aa) )
	ROM_LOAD32_WORD( "mr95033-04.2",  0x400002, 0x200000, CRC(59144dc6) SHA1(0e192001d668791c91ca2af6b367067a5106a4b2) )
	ROM_LOAD32_WORD( "mr95033-05.15", 0x800000, 0x200000, CRC(cc5b8d0b) SHA1(70a5b9db600fc168d13ad54653cf1c8d2a45d991) )
	ROM_LOAD32_WORD( "mr95033-06.3",  0x800002, 0x200000, CRC(2f79942f) SHA1(73417d10f37bcd539b8081312226cf142a5a0d3d) )

	ROM_REGION( 0x200000, REGION_GFX2, 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95033-07.9", 0x000000, 0x200000, CRC(76cd2e0b) SHA1(41aa18dfb4e06547d1f6d7ce49e5225027d16bbb) )

	ROM_REGION( 0x200000, REGION_GFX3, 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr95033-09.11", 0x000000, 0x200000, CRC(8a6dae81) SHA1(e235f2865a9a003330bff1e4d0a017e5d10efd2a) )

	ROM_REGION( 0x080000, REGION_GFX4, 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "32", 0x000000, 0x080000, CRC(becc25c2) SHA1(4ae7665cd45ebd9586068e99327145194ba216fc) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* z80 program */
	ROM_LOAD( "ws-21", 0x000000, 0x040000, CRC(bdeff5d6) SHA1(920a6fc983d53f09510887e4e81ee89ccd5079e6) )
	ROM_RELOAD(              0x010000, 0x40000 )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* samples */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) )
	ROM_LOAD( "mr95033-08.23", 0x200000, 0x200000, CRC(89a291fa) SHA1(7746a0490134fc902ce2dc7b0d33b455d792c105) )
ROM_END


/********** DECRYPT **********/

/* 4 known types */

/* SS91022-10: desertwr, gratiaa, tp2m32, gametngk */

/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi2 */

/* SS92047-01: gratia, kirarast */

/* SS92048-01: p47aces, 47pie2, 47pie2o */

void ms32_rearrange_sprites(int region)
{
	/* sprites are not encrypted, but we need to move the data around to handle them as 256x256 tiles */
	int i;
	UINT8 *source_data;
	int source_size;

	UINT8 *result_data;

	source_data = memory_region       ( region );
	source_size = memory_region_length( region );

	result_data = malloc_or_die(source_size);

	for(i=0; i<source_size; i++)
	{
		int j = (i & ~0x07f8) | ((i & 0x00f8) << 3) | ((i & 0x0700) >> 5);

		result_data[i] = source_data[j];
	}

	memcpy (source_data, result_data, source_size);
	free (result_data);
}


void decrypt_ms32_tx(int addr_xor,int data_xor, int region)
{
	int i;
	UINT8 *source_data;
	int source_size;

	UINT8 *result_data;

	source_data = memory_region       ( region );
	source_size = memory_region_length( region );

	result_data = malloc_or_die(source_size);

	addr_xor ^= 0x1005d;

	for(i=0; i<source_size; i++)
	{
		int j;

		/* two groups of cascading XORs for the address */
		j = 0;
		i ^= addr_xor;

		if (BIT(i,18)) j ^= 0x40000;	// 18
		if (BIT(i,17)) j ^= 0x60000;	// 17
		if (BIT(i, 7)) j ^= 0x70000;	// 16
		if (BIT(i, 3)) j ^= 0x78000;	// 15
		if (BIT(i,14)) j ^= 0x7c000;	// 14
		if (BIT(i,13)) j ^= 0x7e000;	// 13
		if (BIT(i, 0)) j ^= 0x7f000;	// 12
		if (BIT(i,11)) j ^= 0x7f800;	// 11
		if (BIT(i,10)) j ^= 0x7fc00;	// 10

		if (BIT(i, 9)) j ^= 0x00200;	//  9
		if (BIT(i, 8)) j ^= 0x00300;	//  8
		if (BIT(i,16)) j ^= 0x00380;	//  7
		if (BIT(i, 6)) j ^= 0x003c0;	//  6
		if (BIT(i,12)) j ^= 0x003e0;	//  5
		if (BIT(i, 4)) j ^= 0x003f0;	//  4
		if (BIT(i,15)) j ^= 0x003f8;	//  3
		if (BIT(i, 2)) j ^= 0x003fc;	//  2
		if (BIT(i, 1)) j ^= 0x003fe;	//  1
		if (BIT(i, 5)) j ^= 0x003ff;	//  0

		i ^= addr_xor;

		/* simple XOR for the data */
		result_data[i] = source_data[j] ^ (i & 0xff) ^ data_xor;
	}

	memcpy (source_data, result_data, source_size);
	free (result_data);
}

void decrypt_ms32_bg(int addr_xor,int data_xor, int region)
{
	int i;
	UINT8 *source_data;
	int source_size;

	UINT8 *result_data;

	source_data = memory_region       ( region );
	source_size = memory_region_length( region );

	result_data = malloc_or_die(source_size);

	addr_xor ^= 0xc1c5b;

	for(i=0; i<source_size; i++)
	{
		int j;

		/* two groups of cascading XORs for the address */
		j = (i & ~0xfffff);	/* top bits are not affected */
		i ^= addr_xor;

		if (BIT(i,19)) j ^= 0x80000;	// 19
		if (BIT(i, 8)) j ^= 0xc0000;	// 18
		if (BIT(i,17)) j ^= 0xe0000;	// 17
		if (BIT(i, 2)) j ^= 0xf0000;	// 16
		if (BIT(i,15)) j ^= 0xf8000;	// 15
		if (BIT(i,14)) j ^= 0xfc000;	// 14
		if (BIT(i,13)) j ^= 0xfe000;	// 13
		if (BIT(i,12)) j ^= 0xff000;	// 12
		if (BIT(i, 1)) j ^= 0xff800;	// 11
		if (BIT(i,10)) j ^= 0xffc00;	// 10

		if (BIT(i, 9)) j ^= 0x00200;	//  9
		if (BIT(i, 3)) j ^= 0x00300;	//  8
		if (BIT(i, 7)) j ^= 0x00380;	//  7
		if (BIT(i, 6)) j ^= 0x003c0;	//  6
		if (BIT(i, 5)) j ^= 0x003e0;	//  5
		if (BIT(i, 4)) j ^= 0x003f0;	//  4
		if (BIT(i,18)) j ^= 0x003f8;	//  3
		if (BIT(i,16)) j ^= 0x003fc;	//  2
		if (BIT(i,11)) j ^= 0x003fe;	//  1
		if (BIT(i, 0)) j ^= 0x003ff;	//  0

		i ^= addr_xor;

		/* simple XOR for the data */
		result_data[i] = source_data[j] ^ (i & 0xff) ^ data_xor;
	}

	memcpy (source_data, result_data, source_size);
	free (result_data);
}



static void configure_banks(void)
{
	state_save_register_global(to_main);
	memory_configure_bank(4, 0, 16, memory_region(REGION_CPU2) + 0x14000, 0x4000);
	memory_configure_bank(5, 0, 16, memory_region(REGION_CPU2) + 0x14000, 0x4000);
}

/* SS91022-10: desertwr, gratiaa, tp2m32, gametngk */
static DRIVER_INIT (ss91022_10)
{
	configure_banks();
	ms32_rearrange_sprites(REGION_GFX1);
	decrypt_ms32_tx(0x00000,0x35, REGION_GFX4);
	decrypt_ms32_bg(0x00000,0xa3, REGION_GFX3);
}

/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi2 */
static DRIVER_INIT (ss92046_01)
{
	configure_banks();
	ms32_rearrange_sprites(REGION_GFX1);
	decrypt_ms32_tx(0x00020,0x7e, REGION_GFX4);
	decrypt_ms32_bg(0x00001,0x9b, REGION_GFX3);
}

/* SS92047-01: gratia, kirarast */
static DRIVER_INIT (ss92047_01)
{
	configure_banks();
	ms32_rearrange_sprites(REGION_GFX1);
	decrypt_ms32_tx(0x24000,0x18, REGION_GFX4);
	decrypt_ms32_bg(0x24000,0x55, REGION_GFX3);
}

/* SS92048-01: p47aces, 47pie2, 47pie2o */
static DRIVER_INIT (ss92048_01)
{
	configure_banks();
	ms32_rearrange_sprites(REGION_GFX1);
	decrypt_ms32_tx(0x20400,0xd6, REGION_GFX4);
	decrypt_ms32_bg(0x20400,0xd4, REGION_GFX3);
}

static DRIVER_INIT (kirarast)
{
//  { 0xfcc00004, 0xfcc00007, ms32_mahjong_read_inputs1 }
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfcc00004, 0xfcc00007, 0, 0, ms32_mahjong_read_inputs1 );

	DRIVER_INIT_CALL(ss92047_01);
}

static DRIVER_INIT (47pie2)
{
//  { 0xfcc00004, 0xfcc00007, ms32_mahjong_read_inputs1 }
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfcc00004, 0xfcc00007, 0, 0, ms32_mahjong_read_inputs1 );

	DRIVER_INIT_CALL(ss92048_01);
}

static DRIVER_INIT (f1superb)
{
#if 0 // we shouldn't need this hack, something else is wrong, and the x offsets are never copied either, v70 problems??
	UINT32 *pROM = (UINT32 *)memory_region(REGION_CPU1);
	pROM[0x19d04/4]=0x167a021a; // bne->br  : sprite Y offset table is always copied to RAM
#endif
	DRIVER_INIT_CALL(ss92046_01);
}

static DRIVER_INIT (bnstars)
{
//  { 0xfcc00004, 0xfcc00007, ms32_mahjong_read_inputs1 }
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfcc00004, 0xfcc00007, 0, 0, ms32_mahjong_read_inputs1 );

	DRIVER_INIT_CALL(ss92046_01);
}

/********** GAME DRIVERS **********/



GAME( 1994, hayaosi2, 0,        ms32, hayaosi2, ss92046_01, ROT0,   "Jaleco", "Hayaoshi Quiz Grand Champion Taikai", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1994, hayaosi3, 0,        ms32, hayaosi3, ss92046_01, ROT0,   "Jaleco", "Hayaoshi Quiz Nettou Namahousou", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1994, bbbxing,  0,        ms32, bbbxing,  ss92046_01, ROT0,   "Jaleco", "Best Bout Boxing", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1994, 47pie2,   0,        ms32, 47pie2,   47pie2,     ROT0,   "Jaleco", "Idol Janshi Su-Chi-Pie 2 (v1.1)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1994, 47pie2o,  47pie2,   ms32, 47pie2,   47pie2,     ROT0,   "Jaleco", "Idol Janshi Su-Chi-Pie 2 (v1.0)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1995, desertwr, 0,        ms32, desertwr, ss91022_10, ROT270, "Jaleco", "Desert War / Wangan Sensou", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1995, gametngk, 0,        ms32, gametngk, ss91022_10, ROT270, "Jaleco", "The Game Paradise - Master of Shooting! / Game Tengoku - The Game Paradise", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1995, tetrisp,  0,        ms32, tetrisp,  ss92046_01, ROT0,   "Jaleco / BPS", "Tetris Plus", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1995, p47aces,  0,        ms32, p47aces,  ss92048_01, ROT0,   "Jaleco", "P-47 Aces", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1995, akiss,    0,        ms32, 47pie2,   kirarast,   ROT0,   "Jaleco", "Mahjong Angel Kiss", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1996, gratia,   0,        ms32, gratia,   ss92047_01, ROT0,   "Jaleco", "Gratia - Second Earth (92047-01 version)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1996, gratiaa,  gratia,   ms32, gratia,   ss91022_10, ROT0,   "Jaleco", "Gratia - Second Earth (91022-10 version)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1996, kirarast, 0,        ms32, kirarast, kirarast,   ROT0,   "Jaleco", "Ryuusei Janshi Kirara Star", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1997, tp2m32,   tetrisp2, ms32, tp2m32,   ss91022_10, ROT0,   "Jaleco", "Tetris Plus 2 (MegaSystem 32 Version)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1997, bnstars,  bnstars1, ms32, 47pie2,   bnstars,    ROT0,   "Jaleco", "Vs. Janshi Brandnew Stars (MegaSystem32 Version)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1996, wpksocv2, 0,        ms32, ms32,     ss92046_01, ROT0,   "Jaleco", "World PK Soccer V2 (ver 1.1)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE | GAME_NOT_WORKING ) // custom IO board not emulated


/* these boot and show something */
GAME( 1994, f1superb, 0,        ms32, f1superb, f1superb, ROT0,   "Jaleco", "F1 Super Battle", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
