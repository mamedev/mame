// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
Consolidation and enhancement of documentation by Manfred Schneider based on previous work from
 PeT mess@utanet.at and Paul Robson (autismuk@aol.com) minor updates by incog

 Schematics, manuals and anything you can desire for at http://amigan.classicgaming.gamespy.com/

 TODO: implement the RESET key on the front panel
       find a dump of the character ROM
       convert the drawing code to tilemap


 1. General
   SYSTEM
- Signetics 2650 CPU at 3.58/4 MHz(for NTSC) or at 4,433/5 (for PAL)
- 1k x 8 of RAM (physically present but only 512 byte available)
- 2 x 2 axis Analogue Joysticks
- 2 x 12 button Controllers
- 3 buttons on system unit and CPU Reset
   VIDEO
- 128 x 208 pixel resolution (alternate 128 x 104 mode available)
- 16 x 26 (can be 16x13) background display (2 colour 8 x 8 pixel characters)
- 4 x 8 x 8 Sprites (2 colour 8 x 8 pixels)
- total of 8 user defined characters available
   SOUND
- Single channel beeper


2. Memory map
The memory map of the 2001 is below.
0000 - 0FFF 4k ROM Block 1 (first 4k of a cartridge)
1000 - 13FF     mirror of $1800-$1BFF
1400 - 17FF     mirror of $1800-$1BFF
1800 - 1BFF     UVI2637 Area (detail description in video/arcadia.c)
    1800 - 18CF     Screen display , upper 13 lines, characters/palette high bits
    18D0 - 18EF Free for user programs.
    18F0 - 18F7     Sprite coordinates y0x0y1x1y2x2y3x3
    18F8 - 1908     registers of UVI
    1909 - 197F     Unmapped
    1980 - 19BF     User defined characters (8 possible, 8 bytes per character)
    19C0 - 19F7     Unmapped
    19F8 - 19FF     registers of UVI
    1A00 - 1ACF Screen display , lower 13 lines, characters/palette high bits
    1AD0 - 1AFF User memory
    1B00 - 1BFF     mirror of 1900-19FF
1C00 - 1FFF     mirror of 1800-1BFF
2000 - 2FFF 4k ROM Block 2 (for 8k carts such as Jungler)
3000 - 3FFF mirror of 1000-1FFF
4000 - 4FFF mirror of 0000-0FFF
5000 - 5FFF mirror of 1000-1FFF
6000 - 6FFF mirror of 0000-0FFF
7000 - 7FFF mirror of 1000-1FFF

The Palladium VCG memory map is as follows.
0000 - 0FFF 4k ROM Block 1 (first 4k of a cartridge)
1000 - 17FF     could be ROM or RAM but no Cartridge uses this
1800 - 1BFF     UVI2637 Area (detail description in video/arcadia.c)
    1800 - 18CF     Screen display , upper 13 lines, characters/palette high bits
    18D0 - 18EF Free for user programs.
    18F0 - 18F7     Sprite coordinates y0x0y1x1y2x2y3x3
    18F8 - 1908     registers of UVI
    1909 - 197F     Unmapped
    1980 - 19BF     User defined characters (8 possible, 8 bytes per character)
    19C0 - 19F7     Unmapped
    19F8 - 19FF     registers of UVI
    1A00 - 1ACF Screen display , lower 13 lines, characters/palette high bits
    1AD0 - 1AFF User memory
    1B00 - 1BFF     mirror of 1900-19FF
1C00 - 1FFF     could be ROM or RAM but no Cartridge uses this
2000 - 2FFF 4k ROM Block  (for 8k carts such as Jungler)
3000 - 3FFF could be ROM or RAM but no Cartridge uses this
4000 - 4FFF 4k ROM Block  (first 2K used by Golf)
5000 - 5FFF 4k ROM Block
6000 - 6FFF 4k ROM Block
7000 - 7FFF 4k ROM Block


3. ROM Images
ROM Images are loaded into 0000-0FFF. If the ROM is an 8k ROM the
second half of the Rom is located at 2000-2FFF. Except for the Golf cart
which is located from 0x0000-0x0FFF and a 2kbyte block from 0x4000 only on
Palladium VCG.


4. Controls
All key controls are indicated by a bit going to '1'. Unused bits at
the memory location are set to zero.

Keypads

1900-1902 (Player 1) 1904-1906 (Player 2)
The keypads are arranged as follows :-

        1       2       3
        4       5       6
        7       8       9
      Enter     0     Clear

Row 1/4/7 is 1900/4, Row 2/5/8/0 is 1901/5 and Row 3/6/9 is 1902/6
The topmost key is bit 3, the lowermost key is bit 0.

Location $1908 contains bit 0 Start,bit 1 Option,bit 2 Difficulty.
These keys are "latched" i.e. a press causes a logic 1 to appear
on the current frame.

The fire buttons are equivalent to Keypad #2 e.g. they are 1901 and
1905 bit 3.

Palladium has 4 additional key per keypad which are mapped at
1903 palladium player 1
1907 palladium player 2


5. Other information
Interrupts are not supported
The Read/Write 2650 CPU Port-Commands do not appear to be connected to
anything in hardware. No cartridge has been found which uses them.

******************************************************************************/

#include "includes/arcadia.h"
#include "softlist.h"

static ADDRESS_MAP_START( arcadia_mem, AS_PROGRAM, 8, arcadia_state )
	AM_RANGE( 0x0000, 0x0fff) AM_DEVREAD("cartslot", arcadia_cart_slot_device, read_rom)
	AM_RANGE( 0x1800, 0x1aff) AM_READWRITE(video_r, video_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( arcadia_io, AS_IO, 8, arcadia_state )
	AM_RANGE( S2650_SENSE_PORT,S2650_SENSE_PORT) AM_READ(vsync_r)
ADDRESS_MAP_END

/* The Emerson Arcadia 2001 controllers have 2 fire buttons on the side,
   but actually they are wired to keypad button #2. The following definitions
   are meant to document this fact. The keypad has the following layout:

     1  2  3
     4  5  6
     7  8  9
    Cl  0 En                                                                  */

static INPUT_PORTS_START( arcadia )
	PORT_START("panel")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )    PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )    PORT_NAME("Option")          PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT )   PORT_NAME("Select")
//  PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )    PORT_NAME("Reset")           PORT_CODE(KEYCODE_R)         Not implemented

	PORT_START("controller1_col1")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad Clear") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("controller1_col2")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 2/Button") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 0") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("controller1_col3")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad Enter") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("controller1_extra")
	PORT_BIT( 0xff, 0xf0, IPT_UNUSED) // used in palladium

	PORT_START("controller2_col1")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 4") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 7") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad Clear") PORT_CODE(KEYCODE_V)

	PORT_START("controller2_col2")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 2/Button") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 5") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 8") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 0") PORT_CODE(KEYCODE_F)

	PORT_START("controller2_col3")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 6") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 9") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad Enter") PORT_CODE(KEYCODE_R)

	PORT_START("controller2_extra")
	PORT_BIT( 0xff, 0xf0, IPT_UNUSED) // used in palladium

/* FIXME: the joystick are analog - the actual definition is merely an hack */

#if 0
	// auto centering too slow, so only using 5 bits, and scaling at videoside
	PORT_START("controller1_joy_x")
	PORT_BIT( 0x1fe,0x10,IPT_AD_STICK_X)
	PORT_SENSITIVITY(1)
	PORT_KEYDELTA(2000)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_LEFT)
	PORT_CODE_INC(KEYCODE_RIGHT)
	PORT_CODE_DEC(JOYCODE_1_LEFT)
	PORT_CODE_INC(JOYCODE_1_RIGHT)
	PORT_PLAYER(1)
	PORT_RESET

	PORT_START("controller1_joy_y")
	PORT_BIT( 0x1fe,0x10,IPT_AD_STICK_Y)
	PORT_SENSITIVITY(1)
	PORT_KEYDELTA(2000)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_UP)
	PORT_CODE_INC(KEYCODE_DOWN)
	PORT_CODE_DEC(JOYCODE_1_UP)
	PORT_CODE_INC(JOYCODE_1_DOWN)
	PORT_PLAYER(1)
	PORT_RESET

	PORT_START("controller2_joy_x")
	PORT_BIT( 0x1ff,0x10,IPT_AD_STICK_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_DEL)
	PORT_CODE_INC(KEYCODE_PGDN)
	PORT_CODE_DEC(JOYCODE_2_LEFT)
	PORT_CODE_INC(JOYCODE_2_RIGHT)
	PORT_PLAYER(2)
	PORT_RESET

	PORT_START("controller2_joy_y")
	PORT_BIT( 0x1ff,0x10,IPT_AD_STICK_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_HOME)
	PORT_CODE_INC(KEYCODE_END)
	PORT_CODE_DEC(JOYCODE_2_UP)
	PORT_CODE_INC(JOYCODE_2_DOWN)
	PORT_PLAYER(2)
	PORT_RESET
#else
	PORT_START("joysticks")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
#endif
INPUT_PORTS_END

/* The Palladium Video-Computer-Game controllers have no fire buttons on their side
   but have a 16 keys unlabeled keypad. The keys are fully independent and layed out
   according to this weird scheme, which keeps backward compatibility to Arcadia 2001's
   electrical wiring.

        2
    13 14 15
     1 16  3
     4  5  6
     7  8  9
    10 11 12                                                                           */

static INPUT_PORTS_START( plldium )
	PORT_START("panel")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )   PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT )  PORT_NAME("Selector A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )   PORT_NAME("Selector B")    PORT_CODE(KEYCODE_O)
//  PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )    PORT_NAME("Reset")           PORT_CODE(KEYCODE_R)         Not implemented

	PORT_START("controller1_col1")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED) // some bits must be high
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 10") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("controller1_col2")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 2/Button") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 11") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("controller1_col3")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 12") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("controller1_extra")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 13") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 14") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 15") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 16") PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("controller2_col1")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 4") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 7") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 10") PORT_CODE(KEYCODE_V)

	PORT_START("controller2_col2")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 2/Button") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 5") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 8") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 11") PORT_CODE(KEYCODE_F)

	PORT_START("controller2_col3")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 6") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 9") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 12") PORT_CODE(KEYCODE_R)

	PORT_START("controller2_extra")
	PORT_BIT( 0xf0, 0xf0, IPT_UNUSED)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 13") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 14") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 15") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 16") PORT_CODE(KEYCODE_H)

/* FIXME: the joystick are analog - the actual definition is merely an hack */

#if 0
	// auto centering too slow, so only using 5 bits, and scaling at videoside
	PORT_START("controller1_joy_x")
	PORT_BIT( 0x1fe,0x10,IPT_AD_STICK_X)
	PORT_SENSITIVITY(1)
	PORT_KEYDELTA(2000)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_LEFT)
	PORT_CODE_INC(KEYCODE_RIGHT)
	PORT_CODE_DEC(JOYCODE_1_LEFT)
	PORT_CODE_INC(JOYCODE_1_RIGHT)
	PORT_PLAYER(1)
	PORT_RESET

	PORT_START("controller1_joy_y")
	PORT_BIT( 0x1fe,0x10,IPT_AD_STICK_Y)
	PORT_SENSITIVITY(1)
	PORT_KEYDELTA(2000)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_UP)
	PORT_CODE_INC(KEYCODE_DOWN)
	PORT_CODE_DEC(JOYCODE_1_UP)
	PORT_CODE_INC(JOYCODE_1_DOWN)
	PORT_PLAYER(1)
	PORT_RESET

	PORT_START("controller2_joy_x")
	PORT_BIT( 0x1ff,0x10,IPT_AD_STICK_X)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_DEL)
	PORT_CODE_INC(KEYCODE_PGDN)
	PORT_CODE_DEC(JOYCODE_2_LEFT)
	PORT_CODE_INC(JOYCODE_2_RIGHT)
	PORT_PLAYER(2)
	PORT_RESET

	PORT_START("controller2_joy_y")
	PORT_BIT( 0x1ff,0x10,IPT_AD_STICK_Y)
	PORT_SENSITIVITY(100)
	PORT_KEYDELTA(10)
	PORT_MINMAX(0,0x1f)
	PORT_CODE_DEC(KEYCODE_HOME)
	PORT_CODE_INC(KEYCODE_END)
	PORT_CODE_DEC(JOYCODE_2_UP)
	PORT_CODE_INC(JOYCODE_2_DOWN)
	PORT_PLAYER(2)
	PORT_RESET
#else
	PORT_START("joysticks")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )     PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )     PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )       PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )     PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )     PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )       PORT_PLAYER(2) PORT_8WAY
#endif
INPUT_PORTS_END

static const gfx_layout arcadia_charlayout =
{
	8,                      /*width*/
	1,                      /*height*/
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* x offsets */
	{ 0 },                  /* y offsets */
	1*8
};

static GFXDECODE_START( arcadia )
	GFXDECODE_ENTRY( "gfx1", 0x0000, arcadia_charlayout, 0, 68 )
GFXDECODE_END

static const rgb_t arcadia_colors[] =
{
	rgb_t::white,                  /* white */
	rgb_t(0xff, 0xff, 0x00), /* yellow */
	rgb_t(0x00, 0xff, 0xff), /* cyan */
	rgb_t(0x00, 0xff, 0x00), /* green */
	rgb_t(0xff, 0x00, 0xff), /* magenta */
	rgb_t(0xff, 0x00, 0x00), /* red */
	rgb_t(0x00, 0x00, 0xff), /* blue */
	rgb_t::black                   /* black */
};

static const unsigned short arcadia_palette[128+8] =  /* bgnd, fgnd */
{
	0,1,2,3,4,5,6,7,

	0,0, 0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 0,7,
	1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 1,6, 1,7,
	2,0, 2,1, 2,2, 2,3, 2,4, 2,5, 2,6, 2,7,
	3,0, 3,1, 3,2, 3,3, 3,4, 3,5, 3,6, 3,7,
	4,0, 4,1, 4,2, 4,3, 4,4, 4,5, 4,6, 4,7,
	5,0, 5,1, 5,2, 5,3, 5,4, 5,5, 5,6, 5,7,
	6,0, 6,1, 6,2, 6,3, 6,4, 6,5, 6,6, 6,7,
	7,0, 7,1, 7,2, 7,3, 7,4, 7,5, 7,6, 7,7
};

PALETTE_INIT_MEMBER(arcadia_state, arcadia)
{
	for (int i = 0; i < 8; i++)
		palette.set_indirect_color(i, arcadia_colors[i]);

	for (int i = 0; i < 128+8; i++)
		palette.set_pen_indirect(i, arcadia_palette[i]);
}

void arcadia_state::machine_start()
{
	if (m_cart->exists())
	{
		switch (m_cart->get_type())
		{
			case ARCADIA_STD:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x2000, 0xffff, read8_delegate(FUNC(arcadia_cart_slot_device::extra_rom),(arcadia_cart_slot_device*)m_cart));
				break;
			case ARCADIA_GOLF:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000, 0x4fff, read8_delegate(FUNC(arcadia_cart_slot_device::extra_rom),(arcadia_cart_slot_device*)m_cart));
				break;
		}
	}
}

static SLOT_INTERFACE_START(arcadia_cart)
	SLOT_INTERFACE_INTERNAL("std",      ARCADIA_ROM_STD)
	SLOT_INTERFACE_INTERNAL("golf",     ARCADIA_ROM_GOLF)
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( arcadia, arcadia_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 3580000/4)        /* 0.895 MHz */
	MCFG_CPU_PROGRAM_MAP(arcadia_mem)
	MCFG_CPU_IO_MAP(arcadia_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(arcadia_state, video_line,  262*60)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(128+2*XPOS, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 2*XPOS+128-1, 0, 262-1)
	MCFG_SCREEN_UPDATE_DRIVER(arcadia_state, screen_update_arcadia)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", arcadia )
	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(arcadia_palette))
	MCFG_PALETTE_INDIRECT_ENTRIES(8)
	MCFG_PALETTE_INIT_OWNER(arcadia_state, arcadia)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_ARCADIA_SOUND_ADD("custom")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* cartridge */
	MCFG_ARCADIA_CARTRIDGE_ADD("cartslot", arcadia_cart, nullptr)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","arcadia")
MACHINE_CONFIG_END


ROM_START(advsnha)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(bndarc)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(arcadia)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(tccosmos)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(dynavisn)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(ekusera)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(hanihac)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(hmg2650)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(intmpt03)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(ixl2000)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(intervsn)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(itmcmtp3)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(lvision)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(leonardo)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(mratlus)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(ormatu)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(plldium)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(polyvcg)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(poppympt)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(prestmpt)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(rowtrn2k)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(tvg2000)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(sheenhvc)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(soundic)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(telefevr)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(tempestm)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(tbbympt3)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(trakcvg)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(tunixha)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(tryomvgc)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(orbituvi)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END

ROM_START(vdmaster)
	ROM_REGION(0x8000,"maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x100,"gfx1", ROMREGION_ERASEFF)
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

DRIVER_INIT_MEMBER(arcadia_state,arcadia)
{
	int i;
	UINT8 *gfx=memregion("gfx1")->base();
	for (i=0; i<256; i++) gfx[i]=i;
#if 0
	// this is here to allow developement of some simple testroutines
	// for a real console
	{
		UINT8 *rom=memregion("maincpu")->base();
		/* this is a simple routine to display all rom characters
		   on the display for a snapshot */
		static const UINT8 prog[]={ // address 0 of course
		0x20, // eorz, 0
		0x1b, 0x01, // bctr,a $0004
		0x17, // retc a
		0x76, 0x20, // ppsu ii

		// fill screen
		0x04, 0x00, // lodi,0 0
		0x04|1, 0x00, // lodi,1 0
		0xcc|1, 0x78, 0x10, //a: stra,0 $1800,r1
		0x75,9, //cpsl wc|c
		0x84,0x41, // addi,0 0x41
		0x75,9, //cpsl wc|c
		0x84|1, 0x01, // addi,1 1
		0xe4|1, 0x40, // comi,1 40
		0x98, 0x80-15, // bcfr,0 a

		0x04, 0xff, // lodi,0 7
		0xcc, 0x18, 0xfc, // stra,0 $19f8
		0x04, 0x00, // lodi,0 7
		0xcc, 0x18, 0xfd, // stra,0 $18fd
		0x04, 0x07, // lodi,0 7
		0xcc, 0x19, 0xf9, // stra,0 $19f9

		0x04, 0x00, // lodi,0 7
		0xcc, 0x19, 0xbe, // stra,0 $19bf
		0x04, 0x00, // lodi,0 7
		0xcc, 0x19, 0xbf, // stra,0 $19bf

		//loop: 0x0021
		// print keyboards
		0x04|1, 0x00, //y:lodi,1 0
		0x0c|1, 0x79, 0x00, //x: ldra,0 1900,r1
		0x44|0, 0x0f, //andi,0 0f
		0x64|0, 0x10, //ori,0  10
		0xcc|1, 0x78, 0x01, //stra,0 1840,r1
		0x75,9, //cpsl wc|c
		0x84|1, 0x01, //addi,1 1
		0xe4|1, 0x09, //comi,1 9
		0x98, 0x80-18, //bcfr,0 x

		// cycle colors
		0x0c|1, 0x19, 0x00, //ldra,1 1900
		0x44|1, 0xf, //andi,0 0f
		0xe4|1, 1, //comi,1 1
		0x98, +10, //bcfr,0 c
		0x0c, 0x19, 0xbf,//ldra,0 19f9,0
		0x84, 1, //addi,0 1
		0xcc, 0x19, 0xbf, //stra,0 19f9,0
		0x18|3, 12, // bctr,a
		0xe4|1, 2, //c:comi,1 2
		0x98, +10, //bcfr,0 d
		0x0c, 0x19, 0xbf, //ldra,0 19f9,0
		0x84, 8, //addi,0 8
		0xcc, 0x19, 0xbf, //stra,0 19f9,0
		0x18|3, 12, // bctr,a

		// cycle colors
		0xe4|1, 4, //comi,1 4
		0x98, +10, //bcfr,0 c
		0x0c, 0x19, 0xbe,//ldra,0 19f9,0
		0x84, 1, //addi,0 1
		0xcc, 0x19, 0xbe, //stra,0 19f9,0
		0x18|3, 12, // bctr,a
		0xe4|1, 8, //c:comi,1 2
		0x98, +8+9, //bcfr,0 d
		0x0c, 0x19, 0xbe, //ldra,0 19f9,0
		0x84, 8, //addi,0 8
		0xcc, 0x19, 0xbe, //stra,0 19f9,0

		0x0c, 0x19, 0x00, //b: ldra,0 1900
		0x44|0, 0xf, //andi,0 0f
		0xe4, 0, //comi,0 0
		0x98, 0x80-9, //bcfr,0 b

		0x0c, 0x19, 0xbe, //ldra,0 19bf
		0xcc, 0x19, 0xf8, //stra,0 19f8
		0x0c, 0x19, 0xbf, //ldra,0 19bf
		0xcc, 0x19, 0xf9, //stra,0 19f8

		0x0c, 0x19, 0xbe, //ldra,0 17ff
		0x44|0, 0xf, //andi,0 7
		0x64|0, 0x10, //ori,0  10
		0xcc, 0x18, 0x0d, //stra,0 180f
		0x0c, 0x19, 0xbe, //x: ldra,0 19bf
		0x50, 0x50, 0x50, 0x50, //shr,0 4
		0x44|0, 0xf, //andi,0 7
		0x64|0, 0x10, //ori,0  10
		0xcc, 0x18, 0x0c, //stra,0 180e

		0x0c, 0x19, 0xbf, //ldra,0 17ff
		0x44|0, 0xf, //andi,0 7
		0x64|0, 0x10, //ori,0  10
		0xcc, 0x18, 0x0f, //stra,0 180f
		0x0c, 0x19, 0xbf, //x: ldra,0 19bf
		0x50, 0x50, 0x50, 0x50, //shr,0 4
		0x44|0, 0xf, //andi,0 7
		0x64|0, 0x10, //ori,0  10
		0xcc, 0x18, 0x0e, //stra,0 180e

		0x0c, 0x18, 0x00, //ldra,0 1800
		0x84, 1, //addi,0 1
		0xcc, 0x18, 0x00, //stra,0 1800

//      0x1b, 0x80-20-29-26-9-8-2 // bctr,a y
		0x1c|3, 0, 0x32, // bcta,3 loop

		// calling too many subdirectories causes cpu to reset!
		// bxa causes trap
		};
#if 1
		FILE *f;
		f=fopen("chartest.bin","wb");
		fwrite(prog, ARRAY_LENGTH(prog), sizeof(prog[0]), f);
		fclose(f);
#endif
		for (i=0; i<ARRAY_LENGTH(prog); i++) rom[i]=prog[i];

	}
#endif
}


/*   YEAR  NAME       PARENT     COMPAT    MACHINE       INPUT     INIT          COMPANY               FULLNAME */
CONS(1983, advsnha,   arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Advision",           "Advision Home Arcade", MACHINE_IMPERFECT_SOUND )    /* France */
CONS(1982, bndarc,    arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Bandai",             "Arcadia", MACHINE_IMPERFECT_SOUND )                 /* Japan */
CONS(1982, arcadia,   0,         0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Emerson",            "Arcadia 2001", MACHINE_IMPERFECT_SOUND )            /* U.S.A. */
CONS(198?, tccosmos,  arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Mobilar?",           "Tele-Computer Cosmos", MACHINE_IMPERFECT_SOUND )    /* Spain? I have only found pictures of a German Cosmos ( http://www.pong-picture-page.de/catalog/product_info.php?products_id=2170 ) */
CONS(1982, dynavisn,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Yamagiwa",           "Dynavision", MACHINE_IMPERFECT_SOUND )              /* Japan */
CONS(1982, ekusera,   intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "P.I.C",              "Ekusera", MACHINE_IMPERFECT_SOUND )                 /* Japan */
CONS(1982, hanihac,   arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Hanimex",            "Hanimex Home Arcade Centre", MACHINE_IMPERFECT_SOUND )  /* UK */
CONS(1982, hmg2650,   arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Hanimex",            "HMG-2650", MACHINE_IMPERFECT_SOUND )                 /* Germany */
CONS(198?, intmpt03,  0,         arcadia,  arcadia,      arcadia, arcadia_state,  arcadia,      "Intelligent Game",   "Intelligent Game MPT-03", MACHINE_IMPERFECT_SOUND )  /* U.S.A */
CONS(198?, ixl2000,   arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Intercord",          "Intercord XL 2000 System", MACHINE_IMPERFECT_SOUND ) /* Germany */
CONS(198?, intervsn,  ormatu,    0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Intervision",        "Intervision 2001", MACHINE_IMPERFECT_SOUND )         /* Switzerland */
CONS(198?, itmcmtp3,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "ITMC",               "ITMC MPT-03", MACHINE_IMPERFECT_SOUND )              /* France */
CONS(1982, lvision,   arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Leisure-Dynamics",   "Leisure-Vision", MACHINE_IMPERFECT_SOUND )           /* Canada */
CONS(1982, leonardo,  arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "GiG Electronics",    "Leonardo", MACHINE_IMPERFECT_SOUND )                 /* Italy */
CONS(1983, mratlus,   plldium,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "H.G.S.",             "Mr. Altus Tele Brain", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )    /* Germany */
CONS(198?, ormatu,    0,         arcadia,  arcadia,      arcadia, arcadia_state,  arcadia,      "Ormatu Electronics", "Ormatu 2001", MACHINE_IMPERFECT_SOUND )              /* Netherlands */
CONS(198?, plldium,   0,         arcadia,  arcadia,      plldium, arcadia_state,  arcadia,      "Neckermann",         "Palladium Video-Computer-Game", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )      /* Germany, 16 keys instead of 12 */
CONS(1983, polyvcg,   plldium,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Polybrain",          "Polybrain Video Computer Game", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )      /* Germany */
CONS(198?, poppympt,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Transonic",          "Poppy MPT-03 Tele Computer Spiel", MACHINE_IMPERFECT_SOUND )           /* Germany */
CONS(198?, prestmpt,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Prestige",           "Prestige Video Computer Game MPT-03", MACHINE_IMPERFECT_SOUND )        /* France */
CONS(198?, rowtrn2k,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Rowtron",            "Rowtron 2000", MACHINE_IMPERFECT_SOUND )               /* UK */
CONS(1982, tvg2000,   arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Schmid",             "Schmid TVG 2000", MACHINE_IMPERFECT_SOUND )            /* Germany */
CONS(198?, sheenhvc,  ormatu,    0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Sheen",              "Sheen Home Video Centre 2001", MACHINE_IMPERFECT_SOUND )     /* Australia */
CONS(198?, soundic,   intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Soundic",            "Soundic MPT-03", MACHINE_IMPERFECT_SOUND )             /* Finland */
CONS(198?, telefevr,  arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Tchibo",             "Tele-Fever", MACHINE_IMPERFECT_SOUND )                 /* Germany */
CONS(198?, tempestm,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Tempest",            "Tempest MPT-03", MACHINE_IMPERFECT_SOUND )             /* Australia */
CONS(198?, tbbympt3,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Tobby",              "Tobby MPT-03", MACHINE_IMPERFECT_SOUND )               /* ? */
CONS(198?, trakcvg,   plldium,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Trakton",            "Trakton Computer Video Game", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )      /* Australia */
CONS(1982, tunixha,   arcadia,   0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Monaco Leisure",     "Tunix Home Arcade", MACHINE_IMPERFECT_SOUND )          /* New Zealand */
CONS(198?, tryomvgc,  intmpt03,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Tryom",              "Tryom Video Game Center", MACHINE_IMPERFECT_SOUND )    /* U.S.A */
CONS(198?, orbituvi,  0,         arcadia,  arcadia,      arcadia, arcadia_state,  arcadia,      "Orbit Electronics",  "UVI Compu-Game", MACHINE_IMPERFECT_SOUND )             /* New Zealand */
CONS(198?, vdmaster,  orbituvi,  0,        arcadia,      arcadia, arcadia_state,  arcadia,      "Grandstand",         "Video Master", MACHINE_IMPERFECT_SOUND )               /* New Zealand */
