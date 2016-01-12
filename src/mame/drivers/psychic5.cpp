// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/**************************
 *** PSYCHIC 5 hardware ***     (by Roberto Ventura)
 **************************


Psychic 5 (c) JALECO (early 1987)

driver by Jarek Parchanski


0) GENERAL.

The game has two Z80s.
The second CPU controls the two YM2203 sound chips.
Screen resolution is 224x256 (vertical CRT).
768 colors on screen.
96 sprites (16x16 or 32x32).

Some hardware features description is arbitrary since I
guessed their hardware implementation trusting what
I can recall of the 'real' arcade game.


1) ROM CONTENTS.

P5A 256 Kbit    Sound program ROM
P5B 512 Kbit    Sprites data ROM 0
P5C 512 Kbit    Sprites data ROM 1
P5D 256 Kbit    Main CPU program ROM
P5E 512 Kbit    Banked CPU data ROM (banks 0,1,2 and 3)
P5F 256 Kbit    Foreground data ROM
P5G 512 Kbit    Background data ROM 0
P5H 512 Kbit    Background data ROM 1

ROM banks 2 and 3 contain the eight level maps only.

Graphics format is pretty simple,each nibble contains information
for one pixel (4 planes packed).

All graphics is made of 8x8 tiles (32 consecutive bytes),16x16 tiles
are composed of 4 consecutive 8x8 tiles,while 32x32 sprites can be
considered as four 16x16 tiles assembled in the same way 8x8 tiles produce
one 16x16 tile.
Tile composition follows this scheme:

These are 4 consecutive tiles as stored in ROM - increasing memory
(the symbol ">" means the tiles are rotated 270 degrees):

A>;B>;C>;D>

This is the composite tile.

A> C>
B> D>

This is the way tile looks on the effective CRT after 90 deg rotation.

C^ D^
A^ B^


2) CPU.

The board mounts two crystals, 12.000 MHz and 5.000 MHz.
The possible effective main CPU clock may be 6 MHz (12/2).

The main Z80 runs in Interrupt mode 0 (IM0),the game program expects
execution of two different restart (RST) instructions.
RST 10,the main IRQ,is to be triggered each time the screen is refreshed.
RST 08 must be triggered in order to make the game work properly
(e.g. demo game),I guess sound has something to do with this IRQ,but I
don't know whether it is synchronized with video beam neither I know whether
it can be disabled.

Sound CPU runs in IM1.

The main CPU lies idle waiting the external IRQ occurrence executing code
from 012d to 0140.

Game data is paged,level maps and other data fit in ROM space
8000-bfff.

Video RAM is also banked at locations from c000 to dfff.



3) MAIN CPU MEMORY MAP.

0000-7fff   ROM
8000-bfff   paged ROM
c000-dfff   paged RAM (RAM/VRAM/IO)
f000-f1ff   I/O
f200-f7ff   Sprites registers (misc RAM)
f800-ffff   Work RAM

-paged RAM memory map

Bank 0

c000-cfff   Background tile buffer
d000-dfff   RAM (dummy background image for software collisions)

Bank 1

c000-c3ff   I/O
c400-cfff   Palette RAM
d000-dfff   Foreground tile buffer


4) I/O

-VRAM bank 1

c000    COIN SLOTS and START BUTTONS

        76543210
        ||    ||
        ||    |^-- coin 1
        ||    ^--- coin 2
        ||
        |^-------- start 1
        ^--------- start 2


c001    PLAYER 1 CONTROLS
c002    PLAYER 2 CONTROLS

        76543210
          ||||||
          |||||^-- right
          ||||^--- left
          |||^---- down
          ||^----- up
          |^------ fire 0
          ^------- fire 1


c003    DIPSWITCH 0

        76543210
        ||
        ^^-------- number of player's espers


c004    DIPSWITCH 1

        76543210
        |||    |
        |||    ^-- player's immortality
        ^^^------- coin/credit configurations

c308    BACKGROUND SCROLL Y  least significant 8 bits

c309    BACKGROUND SCROLL Y  most significant 2 bits

        76543210
              ||
              |^-- Y scroll bit 8
              ^--- Y scroll bit 9

c30A    BACKGROUND SCROLL X  least significant 8 bits

c30B    BACKGROUND SCROLL X  MSB

        76543210
        ||||||||
        |||||||^-- X scroll bit 8
        ||||||^--- Unknown (title screen)
        ^^^^^^---- Unknown (title screen: 0xff)

c30C    SCREEN MODE

        76543210
              ||
              |^-- background enable bit (0 means black BG)
              ^--- grey background enable bit

c5fe    BACKGROUND PALETTE INTENSITY (red and green)

        76543210
        ||||||||
        ||||^^^^-- green intensity
        ^^^^------ red intensity

c5ff    BACKGROUND PALETTE INTENSITY (blue)

        76543210
        ||||||||
        ||||^^^^-- unknown (?)
        ^^^^------ blue intensity

-RAM f000-f1ff

f000    SOUND COMMAND (?)

f001    UNKNOWN
        maybe some external HW like a flashing light
        when a coin falls in slot (?)

f002    ROM PAGE SELECTOR
        select four (0-3) ROM pages at 8000-bfff.

f003    VRAM PAGE SELECTOR
        selects two (0-1) VRAM pages at c000-dfff.

f004    UNKNOWN

f005    UNKNOWN


5) COLOR RAM

The palette system is dynamic,the game can show up to 768 different
colors on screen.

Each color component (RGB) depth is 4 bits,two consecutive bytes are used
for each color code (12 bits).

format: RRRRGGGGBBBB0000

Colors are organized in palettes,since graphics is 4 bits (16 colors)
each palette takes 32 bytes;the three different layers (background,sprites
and foreground) don't share any color,each has its own 256 color space,hence
the 768 colors on screen.

c400-c5ff       Sprites palettes
c800-c9ff       Background palettes
ca00-cbff       Foreground palettes

The last palette colors for sprites and foreground are transparent
colors,these colors aren't displayed on screen so the actual maximum
color output is 736 colors.

Some additional palette effects are provided for background.
Sprite palette 15's transparent color (c5fe-c5ff) is the global
background intensity.
Background intensity is encoded in the same way of colors,and
affects the intensity of each color component (BGR).
The least significant nibble of c5ff is unknown,it assumes value
0xf occasionally when the other nibbles are changed.The only
value which is not 0 neither 0xf is 2 and it is assumed by this nibble
during the ride on the witches' broom.

When bit 1 of c30c (VRAM bank 1) is set background screen turns
grey.Notice the output of this function is passed to the palette
intensity process.
When you hit the witch the screen gets purple,this is done via
setting grey screen and scaling color components.
(BTW,I haven't seen a Psychic5 machine for about 10 years but
I think the resulting purple tonality in my emulator is way off...)

Palette intensity acts badly during title screen,when the game
scenario rapidly shows up under the golden logo;the problem is value
0xffff causes the background to be displayed as black.


6) TILE-BASED LAYERS

The tile format for background and foreground is the same,the
only difference is that background tiles are 16x16 pixels while foreground
tiles are only 8x8.

Background virtual screen is 1024 pixels tall and 512 pixel wide.

Two consecutive bytes identify one tile.

        O7 O6 O5 O4 O3 O2 O1 O0         gfx Offset
        O9 O8 FX FY C3 C2 C1 C0         Attribute

        O= GFX offset (1024 tiles)
        F= Flip X and Y
        C= Color palette selector

Tile layers are to be displayed 'bottom-up' (eg. c000-c040
is row 63 of background screen)

Both background and foreground playfields can scroll; background tiles
are arranged in a rotational (wrap-around) buffer; window can be scrolled
vertically by an eight pixel offset (i.e. enough to scroll a single tile
smoothly),the game has to copy in new tiles every time a row is scrolled
(this feature is only used in the credits section at the end of the game).
I haven't provided the address for this feature, since I don't
know where it is mapped.


7) SPRITES

Five bytes identify each sprite,but the registers actually used
are placed at intervals of 16.
The remaining bytes are used as backup sprite coordinates and
attributes,not real sprites.
Sprites are provided in 2 different sizes,16x16 and 32x32.
Larger sprites are addressed as 16x16 sprites,but they are all
aligned by 4.

The first sprite data is located at f20b,then f21b and so on.

0b      X7 X6 X5 X4 X3 X2 X1 X0         X coord (on screen)
0c      Y7 Y6 Y5 Y5 Y3 Y2 Y1 Y0         X coord (on screen)
0d      O9 O8 FX FY SZ X8 -- Y8         hi gfx - FLIP - hi X - hi Y
0e      O7 O6 O5 O4 O3 O2 O1 O0         gfx - offset
0f      -- -- -- -- C3 C2 C1 C0         color

    Y= Y coordinate (two's complemented) (Y8 is used to clip sprite on top border)
        X= X coordinate (X8 is used to clip 32x32 sprites on left border)
        O= Gfx offset (1024 sprites)
        F= Flip
    SZ=Size 0=16x16 sprite,1=32x32 sprite
        C= Color palette selector
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "includes/psychic5.h"


MACHINE_START_MEMBER(psychic5_state, psychic5)
{
	membank("mainbank")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_bank_latch));
}

MACHINE_START_MEMBER(psychic5_state, bombsa)
{
	membank("mainbank")->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_bank_latch));
}

void psychic5_state::machine_reset()
{
	m_bank_latch = 0xff;
	flip_screen_set(0);
}

/***************************************************************************

  Interrupt(s)

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(psychic5_state::scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   /* RST 10h - vblank */

	if(scanline == 0) // sprite buffer irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf);   /* RST 08h */
}



/***************************************************************************

  Memory Handler(s)

***************************************************************************/

READ8_MEMBER(psychic5_state::bankselect_r)
{
	return m_bank_latch;
}

WRITE8_MEMBER(psychic5_state::psychic5_bankselect_w)
{
	if (m_bank_latch != data)
	{
		m_bank_latch = data;
		membank("mainbank")->set_entry(data & 3);   /* Select 4 banks of 16k */
	}
}

WRITE8_MEMBER(psychic5_state::bombsa_bankselect_w)
{
	if (m_bank_latch != data)
	{
		m_bank_latch = data;
		membank("mainbank")->set_entry(data & 7);   /* Select 8 banks of 16k */
	}
}

WRITE8_MEMBER(psychic5_state::psychic5_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 7 toggles flip screen
	if (data & 0x80)
	{
		flip_screen_set(!flip_screen());
	}
}

WRITE8_MEMBER(psychic5_state::bombsa_flipscreen_w)
{
	// bit 7 toggles flip screen
	if (data & 0x80)
	{
		flip_screen_set(!flip_screen());
	}
}


/***************************************************************************

  Memory Map(s)

***************************************************************************/

static ADDRESS_MAP_START( psychic5_main_map, AS_PROGRAM, 8, psychic5_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("mainbank")
	AM_RANGE(0xc000, 0xdfff) AM_DEVICE("vrambank", address_map_bank_device, amap8)
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xf001, 0xf001) AM_READNOP AM_WRITE(psychic5_coin_counter_w)
	AM_RANGE(0xf002, 0xf002) AM_READWRITE(bankselect_r, psychic5_bankselect_w)
	AM_RANGE(0xf003, 0xf003) AM_READWRITE(vram_page_select_r, vram_page_select_w)
	AM_RANGE(0xf004, 0xf004) AM_NOP // ???
	AM_RANGE(0xf005, 0xf005) AM_READNOP AM_WRITE(psychic5_title_screen_w)
	AM_RANGE(0xf006, 0xf1ff) AM_NOP
	AM_RANGE(0xf200, 0xf7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( psychic5_vrambank_map, AS_PROGRAM, 8, psychic5_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x1000, 0x1fff) AM_RAM

	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x2001, 0x2001) AM_READ_PORT("P1")
	AM_RANGE(0x2002, 0x2002) AM_READ_PORT("P2")
	AM_RANGE(0x2003, 0x2003) AM_READ_PORT("DSW1")
	AM_RANGE(0x2004, 0x2004) AM_READ_PORT("DSW2")

	AM_RANGE(0x2308, 0x230c) AM_RAM AM_SHARE("bg_control")

	AM_RANGE(0x2400, 0x25ff) AM_RAM_WRITE(sprite_col_w) AM_SHARE("palette_ram_sp")
	AM_RANGE(0x2800, 0x29ff) AM_RAM_WRITE(bg_col_w) AM_SHARE("palette_ram_bg")
	AM_RANGE(0x2a00, 0x2bff) AM_RAM_WRITE(tx_col_w) AM_SHARE("palette_ram_tx")

	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")

ADDRESS_MAP_END


static ADDRESS_MAP_START( psychic5_sound_map, AS_PROGRAM, 8, psychic5_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( psychic5_soundport_map, AS_IO, 8, psychic5_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("ym2", ym2203_device, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bombsa_main_map, AS_PROGRAM, 8, psychic5_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("mainbank")
	AM_RANGE(0xc000, 0xcfff) AM_RAM

	/* ports look like the other games */
	AM_RANGE(0xd000, 0xd000) AM_WRITE(soundlatch_byte_w) // confirmed
	AM_RANGE(0xd001, 0xd001) AM_WRITE(bombsa_flipscreen_w)
	AM_RANGE(0xd002, 0xd002) AM_READWRITE(bankselect_r, bombsa_bankselect_w)
	AM_RANGE(0xd003, 0xd003) AM_READWRITE(vram_page_select_r, vram_page_select_w)
	AM_RANGE(0xd005, 0xd005) AM_WRITE(bombsa_unknown_w) // ?

	AM_RANGE(0xd000, 0xd1ff) AM_RAM
	AM_RANGE(0xd200, 0xd7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xffff) AM_DEVICE("vrambank", address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bombsa_sound_map, AS_PROGRAM, 8, psychic5_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf000, 0xf000) AM_WRITEONLY                               // Is this a confirm of some sort?
ADDRESS_MAP_END

static ADDRESS_MAP_START( bombsa_soundport_map, AS_IO, 8, psychic5_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bombsa_vrambank_map, AS_PROGRAM, 8, psychic5_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")

	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x2001, 0x2001) AM_READ_PORT("P1")
	AM_RANGE(0x2002, 0x2002) AM_READ_PORT("P2")
	AM_RANGE(0x2003, 0x2003) AM_READ_PORT("DSW1")
	AM_RANGE(0x2004, 0x2004) AM_READ_PORT("DSW2")

	AM_RANGE(0x2308, 0x230c) AM_RAM AM_SHARE("bg_control")

	AM_RANGE(0x3000, 0x31ff) AM_RAM_WRITE(sprite_col_w) AM_SHARE("palette_ram_sp")
	AM_RANGE(0x3200, 0x33ff) AM_RAM_WRITE(bg_col_w) AM_SHARE("palette_ram_bg")
	AM_RANGE(0x3400, 0x35ff) AM_RAM_WRITE(tx_col_w) AM_SHARE("palette_ram_tx")

	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")
ADDRESS_MAP_END


static INPUT_PORTS_START( psychic5 )
	PORT_START("SYSTEM")    /* system control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")        /* player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Invulnerability (Cheat)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bombsa )
	PORT_START("SYSTEM")    /* system control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")        /* player 1 control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* player 2 control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:8" )           // Coin_B
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:6" )           // Coin_B
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:5" )           // Coin_B
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:3" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, "2 Coins 1 Credit/4 Coins 3 Credits" )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:1" )           // flip screen ?
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8x8 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the four bitplanes for pixel are packed into one nibble */
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16x16 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the four bitplanes for pixel are packed into one nibble */
	{ 0, 4, 8, 12, 16, 20, 24, 28, 64*8, 64*8+4, 64*8+8, 64*8+12, 64*8+16, 64*8+20, 64*8+24, 64*8+28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8, 32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static GFXDECODE_START( psychic5 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,  0*16, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,   32*16, 16 )
GFXDECODE_END

static GFXDECODE_START( bombsa )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 32*16, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0*16,  16 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,   16*16, 16 )
GFXDECODE_END


static MACHINE_CONFIG_START( psychic5, psychic5_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)
	MCFG_CPU_PROGRAM_MAP(psychic5_main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", psychic5_state, scanline, "screen", 0, 1)

	MCFG_DEVICE_ADD("vrambank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(psychic5_vrambank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(14)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_5MHz)
	MCFG_CPU_PROGRAM_MAP(psychic5_sound_map)
	MCFG_CPU_IO_MAP(psychic5_soundport_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))      /* Allow time for 2nd cpu to interleave */

	MCFG_MACHINE_START_OVERRIDE(psychic5_state,psychic5)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(53.8)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	/* frames per second hand tuned to match game and music speed */
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(psychic5_state, screen_update_psychic5)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", psychic5)
	MCFG_PALETTE_ADD("palette", 768)

	MCFG_DEVICE_ADD("blend", JALECO_BLEND, 0)

	MCFG_VIDEO_START_OVERRIDE(psychic5_state,psychic5)
	MCFG_VIDEO_RESET_OVERRIDE(psychic5_state,psychic5)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_12MHz/8)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bombsa, psychic5_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2 ) /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(bombsa_main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", psychic5_state, scanline, "screen", 0, 1)

	MCFG_DEVICE_ADD("vrambank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bombsa_vrambank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(14)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_5MHz )
	MCFG_CPU_PROGRAM_MAP(bombsa_sound_map)
	MCFG_CPU_IO_MAP(bombsa_soundport_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_START_OVERRIDE(psychic5_state,bombsa)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(54)                /* Guru says : VSync - 54Hz . HSync - 15.25kHz */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(psychic5_state, screen_update_bombsa)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bombsa)
	MCFG_PALETTE_ADD("palette", 768)

	MCFG_VIDEO_START_OVERRIDE(psychic5_state,bombsa)
	MCFG_VIDEO_RESET_OVERRIDE(psychic5_state,psychic5)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_12MHz/8)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.30)
	MCFG_SOUND_ROUTE(1, "mono", 0.30)
	MCFG_SOUND_ROUTE(2, "mono", 0.30)
	MCFG_SOUND_ROUTE(3, "mono", 1.0)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(0, "mono", 0.30)
	MCFG_SOUND_ROUTE(1, "mono", 0.30)
	MCFG_SOUND_ROUTE(2, "mono", 0.30)
	MCFG_SOUND_ROUTE(3, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/



ROM_START( psychic5j )
	ROM_REGION( 0x20000, "maincpu", 0 )                     /* Main CPU */
	ROM_LOAD( "p5d",          0x00000, 0x08000, CRC(90259249) SHA1(ac2d8dd95f6c04b6ad726136931e37dcd537e977) )
	ROM_LOAD( "p5e",          0x10000, 0x10000, CRC(72298f34) SHA1(725be2fbf5f3622f646c0fb8e6677cbddf0b1fc2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                    /* Sound CPU */
	ROM_LOAD( "p5a",          0x00000, 0x08000, CRC(50060ecd) SHA1(e6051fb4a1fa9429cfb6084e8a5dfe994a08280b) )

	ROM_REGION( 0x20000, "gfx1", 0 )    /* sprite tiles */
	ROM_LOAD( "p5b",          0x00000, 0x10000, CRC(7e3f87d4) SHA1(b8e7fa3f96d2e3937e4cb530f105bb84d5743b43) )
	ROM_LOAD( "p5c",          0x10000, 0x10000, CRC(8710fedb) SHA1(c7e8dc6b733e4ecce37d56fc429c00ade8736ff3) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* background tiles */
	ROM_LOAD( "p5g",          0x00000, 0x10000, CRC(f9262f32) SHA1(bae2dc77be7024bd85f213e4da746c5903db6ea5) )
	ROM_LOAD( "p5h",          0x10000, 0x10000, CRC(c411171a) SHA1(d5893563715ba231e42b084b88f5176bb94a4da9) )

	ROM_REGION( 0x08000, "gfx3", 0 )    /* foreground tiles */
	ROM_LOAD( "p5f",          0x00000, 0x08000, CRC(04d7e21c) SHA1(6046c506bdedc233e3730f90c7897e847bec8758) )

	ROM_REGION( 0x08000, "proms", 0 )   /* Proms */
	ROM_LOAD( "my10.7l",    0x000, 0x200, CRC(6a7d13c0) SHA1(2a835a4ac1acb7663d0b915d0339af9800284da6) )
	ROM_LOAD( "my09.3t",    0x200, 0x400, CRC(59e44236) SHA1(f53d99694fa5acd7cc51dd78e09f0d2ef730e7a4) )
ROM_END

ROM_START( psychic5 )
	ROM_REGION( 0x20000, "maincpu", 0 )                     /* Main CPU */
	ROM_LOAD( "myp5d",          0x00000, 0x08000, CRC(1d40a8c7) SHA1(79b36e690ea334c066b55b1e39ceb5fe0688cd7b) )
	ROM_LOAD( "myp5e",          0x10000, 0x10000, CRC(2fa7e8c0) SHA1(d5096ebec58329346a3292ad2da1be3742fad093) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                    /* Sound CPU */
	ROM_LOAD( "myp5a",          0x00000, 0x10000, CRC(6efee094) SHA1(ae2b5bf6199121520bf8428b8b160b987f5b474f) )

	ROM_REGION( 0x20000, "gfx1", 0 )    /* sprite tiles */
	ROM_LOAD( "p5b",          0x00000, 0x10000, CRC(7e3f87d4) SHA1(b8e7fa3f96d2e3937e4cb530f105bb84d5743b43) )
	ROM_LOAD( "p5c",          0x10000, 0x10000, CRC(8710fedb) SHA1(c7e8dc6b733e4ecce37d56fc429c00ade8736ff3) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* background tiles */
	ROM_LOAD( "myp5g",          0x00000, 0x10000, CRC(617b074b) SHA1(7aaac9fddf5675b6698373333db3e096471d7ad6) )
	ROM_LOAD( "myp5h",          0x10000, 0x10000, CRC(a9dfbe67) SHA1(f31f75e88f9b37d7fe5b1a1a8e0299151b729ccf) )

	ROM_REGION( 0x08000, "gfx3", 0 )    /* foreground tiles */
	ROM_LOAD( "p5f",          0x00000, 0x08000, CRC(04d7e21c) SHA1(6046c506bdedc233e3730f90c7897e847bec8758) )

	ROM_REGION( 0x08000, "proms", 0 )   /* Proms */
	ROM_LOAD( "my10.7l",    0x000, 0x200, CRC(6a7d13c0) SHA1(2a835a4ac1acb7663d0b915d0339af9800284da6) )
	ROM_LOAD( "my09.3t",    0x200, 0x400, CRC(59e44236) SHA1(f53d99694fa5acd7cc51dd78e09f0d2ef730e7a4) )
ROM_END

/*
Bombs Away
Jaleco, 1988

PCB Layout
----------

BB-8744
|------------------------------------------|
|         1      Z80(1)          4         |
|         6116      5MHz                  |-|
|VOL      YM2203                 5        | |
|4558     YM2203      Z80(2)              | |
|4558 YM3014                     6        | |
|     YM3014                              | |
|J          6116                6264      | |
|A                                        |-|
|M                                         |
|M                                         |
|A                                        |-|
|                                         | |
|                                         | |
|                2   62256                | |
| DSW1                                    | |
|                3   62256                | |
| DSW2    82S137                          |-|
|                                          |
|------------------------------------------|
Notes:
      Z80(1) clock - 5.000MHz
      Z80(2) clock - 6.000MHz [12/2]
      YM2203 clock - 1.500MHz [12/8, both]
      VSync - 54Hz
      HSync - 15.25kHz


BB-8745
|--------------------------------|
|                                |
|                      |-----|  |-|
|                      |N8633|  | |
|                      |-S   |  | |
|                      |-----|  | |
|  82S123                       | |
|              |-----|          | |
|              |N8633|          |-|
|              |-V   |           |
|2018          |     |           |
|2018        7 |6116 |    12MHz |-|
|              |-----|          | |
|      |-----|                  | |
|      |N8633|                  | |
|      |-V64 |                  | |
|      |     |                  | |
| 9  8 |6264 |                  |-|
|      |-----|                   |
|--------------------------------|
Notes:
      3 soldered-in 'modules' are located on this PCB.
      N-8633-V 6-7 TOYOCOM
      N-8633-V64 6-7 TOYOCOM
      N-8633-S 6-7 TOYOCOM
      The 2 larger ones have 62 pins, 31 pins on each side of a small PCB. The PCB contains some
      surface mounted logic chips and some surface mounted RAM.
      The smaller module has 40 pins, 20 pins on each side of a small PCB. The PCB contains only
      logic on both sides.
*/

ROM_START( bombsa )
	ROM_REGION( 0x30000, "maincpu", 0 )                 /* Main CPU */
	ROM_LOAD( "4.7a",         0x00000, 0x08000, CRC(0191f6a7) SHA1(10a0434abbf4be068751e65c81b1a211729e3742) )
	/* these fail their self-test... should be checked on real hw (hold start1+start2 on boot) */
	ROM_LOAD( "5.7c",         0x10000, 0x08000, BAD_DUMP CRC(095c451a) SHA1(892ca84376f89640ad4d28f1e548c26bc8f72c0e) ) // contains palettes etc. but fails rom check??
	ROM_LOAD( "6.7d",         0x20000, 0x10000, BAD_DUMP CRC(89f9dc0f) SHA1(5cf6a7aade3d56bc229d3771bc4141ad0c0e8da2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                    /* Sound CPU */
	ROM_LOAD( "1.3a",         0x00000, 0x08000, CRC(92801404) SHA1(c4ff47989d355b18a909eaa88f138e2f68178ecc) )

	ROM_REGION( 0x20000, "gfx1", 0 )    /* sprite tiles */
	ROM_LOAD( "2.4p",         0x00000, 0x10000, CRC(bd972ff4) SHA1(63bfb455bc0ae1d31e6f1066864ec0c8d2d0cf99) )
	ROM_LOAD( "3.4s",         0x10000, 0x10000, CRC(9a8a8a97) SHA1(13328631202c196c9d8791cc6063048eb6be0472) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* background tiles */
	/* some corrupt 'blank' characters, should also be checked with a redump */
	ROM_LOAD( "8.2l",         0x00000, 0x10000, BAD_DUMP CRC(3391c769) SHA1(7ae7575ac81d6e0d915c279c1f57a9bc6d096bd6) )
	ROM_LOAD( "9.2m",         0x10000, 0x10000, BAD_DUMP CRC(5b315976) SHA1(d17cc1926f926bdd88b66ea6af88dac30880e7d4) )

	ROM_REGION( 0x08000, "gfx3", 0 )    /* foreground tiles */
	ROM_LOAD( "7.4f",         0x00000, 0x08000, CRC(400114b9) SHA1(db2f3ba05a2005ae0e0e7d19c8739353032cbeab) )

	ROM_REGION( 0x08000, "proms", 0 )   /* Proms */
	ROM_LOAD( "82s131.7l",    0x000, 0x200, CRC(6a7d13c0) SHA1(2a835a4ac1acb7663d0b915d0339af9800284da6) )
	ROM_LOAD( "82s137.3t",    0x200, 0x400, CRC(59e44236) SHA1(f53d99694fa5acd7cc51dd78e09f0d2ef730e7a4) )
ROM_END


GAME( 1987, psychic5,  0,        psychic5, psychic5, driver_device, 0, ROT270, "Jaleco / NMK", "Psychic 5 (World)", MACHINE_SUPPORTS_SAVE ) // "Oversea's version V2.00 CHANGED BY TAMIO NAKASATO" text present in ROM, various modifications (English names, more complete attract demo etc.)
GAME( 1987, psychic5j, psychic5, psychic5, psychic5, driver_device, 0, ROT270, "Jaleco / NMK", "Psychic 5 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bombsa,    0,        bombsa,   bombsa,   driver_device, 0, ROT270, "Jaleco", "Bombs Away", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
