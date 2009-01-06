/***************************************************************************

  Golden Star
  Cherry Master
  Lucky 8 Lines

  Golden Star and Cherry Master seem to be almost the same thing, running on
  different hardware.  There are also various bootlegs / hacks, it isn't clear
  exactly what hardware each runs on, some appear to have no OKI for example.

  Driver by Mirko Buffoni.
  Additional Work: David Haywood & Roberto Fresca.


****************************************************************************

  Game notes:
  -----------

  * New Lucky 8 Lines:

  Sometimes the game boots with a "Coin Jam" message. Just reset the game to normalize.
  There are 2 sets of controls. Press the BIG key to switch between them.

  Press 9 to enter settings, press START to exit.
  Press 0 to enter stats, press START to exit.
  Keeping pressed 9 + 0 + RESET (F3), will enter the test mode. Press RESET to exit.

  New Lucky 8 Lines has two sets of controls than can be switched through each 'BIG' button.
  Even you can switch controls in middle of the game. When a set of controls are in use,
  the other set is blocked till 'BIG' button is pressed.


  * Cherry Bonus III:

  If a hopper status error appear when the player try to take score,
  pressing Key Out (W) will discharge the credits won.

  Cherry Bonus III has two sets of controls than can be switched through each 'BIG' button.
  Even you can switch controls in middle of the game. When a set of controls are in use,
  the other set is blocked till 'BIG' button is pressed.

  Controls Set2 is using reels stop buttons from Controls Set1.


***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/sn76496.h"
#include "machine/8255ppi.h"

static int dataoffset=0;

extern UINT8 *goldstar_video1, *goldstar_video2, *goldstar_video3;
extern size_t goldstar_video_size;
extern UINT8 *goldstar_reel1_scroll, *goldstar_reel2_scroll, *goldstar_reel3_scroll;


extern UINT8 *goldstar_reel1_ram;
extern WRITE8_HANDLER( goldstar_reel1_ram_w );
extern UINT8 *goldstar_reel2_ram;
extern WRITE8_HANDLER( goldstar_reel2_ram_w );
extern UINT8 *goldstar_reel3_ram;
extern WRITE8_HANDLER( goldstar_reel3_ram_w );

extern WRITE8_HANDLER( goldstar_fg_vidram_w );
extern WRITE8_HANDLER( goldstar_fg_atrram_w );

WRITE8_HANDLER( goldstar_fa00_w );
VIDEO_START( goldstar );
VIDEO_START( cherrym );
VIDEO_UPDATE( goldstar );

static UINT8 *nvram;
static size_t nvram_size;

static NVRAM_HANDLER( goldstar )
{
	if (read_or_write)
                mame_fwrite(file,nvram,nvram_size);
	else
	{
		if (file)
                        mame_fread(file,nvram,nvram_size);
		else
			memset(nvram,0xff,nvram_size);
	}
}


static WRITE8_HANDLER( protection_w )
{
	if (data == 0x2a)
		dataoffset = 0;
}

static READ8_HANDLER( protection_r )
{
	static const int data[4] = { 0x47, 0x4f, 0x4c, 0x44 };

	dataoffset %= 4;
	return data[dataoffset++];
}

static ADDRESS_MAP_START( goldstar_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xb7ff) AM_ROM
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_WRITE(goldstar_fg_vidram_w) AM_BASE(&videoram)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_WRITE(goldstar_fg_atrram_w) AM_BASE(&colorram)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM AM_WRITE( goldstar_reel1_ram_w ) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xe000, 0xe1ff) AM_RAM AM_WRITE( goldstar_reel2_ram_w ) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xe800, 0xe9ff) AM_RAM AM_WRITE( goldstar_reel3_ram_w ) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xf040, 0xf07f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xf080, 0xf0bf) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xf0c0, 0xf0ff) AM_RAM AM_BASE(&goldstar_reel3_scroll)

	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("IN0")
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("IN1")	/* Test Mode */
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("DSW1")
//  AM_RANGE(0xf803, 0xf803)
//  AM_RANGE(0xf804, 0xf804)
	AM_RANGE(0xf805, 0xf805) AM_READ_PORT("DSW4")	/* DSW 4 (also appears in 8910 port) */
	AM_RANGE(0xf806, 0xf806) AM_READ_PORT("DSW7")	/* (don't know to which one of the */
	AM_RANGE(0xf810, 0xf810) AM_READ_PORT("UNK1")
	AM_RANGE(0xf811, 0xf811) AM_READ_PORT("UNK2")
	AM_RANGE(0xf820, 0xf820) AM_READ_PORT("DSW2")
	AM_RANGE(0xf830, 0xf830) AM_READWRITE(ay8910_read_port_0_r,ay8910_write_port_0_w)
	AM_RANGE(0xf840, 0xf840) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(goldstar_fa00_w)
	AM_RANGE(0xfb00, 0xfb00) AM_READWRITE(okim6295_status_0_r,okim6295_data_0_w)
	AM_RANGE(0xfd00, 0xfdff) AM_READWRITE(SMH_RAM,paletteram_BBGGGRRR_w) AM_BASE(&paletteram)
	AM_RANGE(0xfe00, 0xfe00) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( goldstar_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW6")
ADDRESS_MAP_END



static ADDRESS_MAP_START( ncb3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xb7ff) AM_ROM
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_WRITE(goldstar_fg_vidram_w) AM_BASE(&videoram)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_WRITE(goldstar_fg_atrram_w) AM_BASE(&colorram)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM AM_WRITE(goldstar_reel1_ram_w) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xe000, 0xe1ff) AM_RAM AM_WRITE(goldstar_reel2_ram_w) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xe800, 0xe9ff) AM_RAM AM_WRITE(goldstar_reel3_ram_w) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xf040, 0xf07f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xf080, 0xf0bf) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xf100, 0xf17f) AM_RAM AM_BASE(&goldstar_reel3_scroll) // moved compared to goldstar

	AM_RANGE(0xf800, 0xf803) AM_DEVREADWRITE(PPI8255, "ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xf810, 0xf813) AM_DEVREADWRITE(PPI8255, "ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xf820, 0xf823) AM_DEVREADWRITE(PPI8255, "ppi8255_2", ppi8255_r, ppi8255_w)	/* Input/Output Ports */
	AM_RANGE(0xf830, 0xf830) AM_READWRITE(ay8910_read_port_0_r,ay8910_write_port_0_w)
	AM_RANGE(0xf840, 0xf840) AM_WRITE(ay8910_control_port_0_w)
//  AM_RANGE(0xf850, 0xf850) AM_WRITE(ncb3_p1_flip_w)   // need flip?
//  AM_RANGE(0xf860, 0xf860) AM_WRITE(ncb3_p2_flip_w)   // need flip?
	AM_RANGE(0xf870, 0xf870) AM_WRITE(sn76496_0_w)	/* guess... device is initialized, but doesn't seems to be used.*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( ncb3_readwriteport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_READ(ncb3_unkread_r)    // read from 0x00 when controls set1 is used...
//  AM_RANGE(0x02, 0x02) AM_READ(ncb3_unkread_r)    // read from 0x02 when controls set2 is used...
//  AM_RANGE(0x06, 0x06) AM_READ(ncb3_unkread_r)    // unknown...
//  AM_RANGE(0x08, 0x08) AM_READ(ncb3_unkread_r)    // unknown...
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW5")	/* confirmed for ncb3 */
//  AM_RANGE(0x81, 0x81) ---> large writes.

ADDRESS_MAP_END

/* ncb3 findings...

  f800-f803 = 8255_1 (ctrl=9b) ; portA, B & C (input)
  f810-f813 = 8255_2 (ctrl=9b) ; portA, B & C (input)
  f820-f823 = 8255_3 (ctrl=90) ; portA (input); ports B & C (output)
  f830      = AY8910 RW
  f840      = AY8910 ctrl
  f850      = Unknown
  f870      = PSG (init writes)


  I/O

  00 = RW  (chrygld, ncb3 in ctrl set1)
  02 = RW  (ncb3 in ctrl set2)
  06 = RW
  08 = RW
  81 =  W

  00-0f = initial seq. writes

  Controls Set1 = write to f850 (0x1a), read from 0002.
  Controls Set2 = write to f860 (0x1a), read from 0000.

  Controls Set2 is using reels stop buttons from Controls Set1.

*/


static WRITE8_HANDLER( cm_outport0_w )
{
	/* lamps? */
}

static WRITE8_HANDLER( cm_outport1_w )
{
	/* quick extended writes */
}

static ADDRESS_MAP_START( cm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM AM_WRITENOP

	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_WRITE(goldstar_fg_vidram_w) AM_BASE(&videoram)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_WRITE(goldstar_fg_atrram_w) AM_BASE(&colorram)

	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)

	AM_RANGE(0xf000, 0xf1ff) AM_RAM AM_WRITE( goldstar_reel1_ram_w ) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xf200, 0xf3ff) AM_RAM AM_WRITE( goldstar_reel2_ram_w ) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xf400, 0xf5ff) AM_RAM AM_WRITE( goldstar_reel3_ram_w ) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xf600, 0xf7ff) AM_RAM

	AM_RANGE(0xf800, 0xf87f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xf880, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xfa7f) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xfa80, 0xfbff) AM_RAM
	AM_RANGE(0xfc00, 0xfc7f) AM_RAM AM_BASE(&goldstar_reel3_scroll)
	AM_RANGE(0xfc80, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cm_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(ay8910_read_port_0_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE(PPI8255, "cm_ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE(PPI8255, "cm_ppi8255_1", ppi8255_r, ppi8255_w)	/* DIP switches */
	AM_RANGE(0x10, 0x10) AM_WRITE (cm_outport0_w)	/* output port */
	AM_RANGE(0x11, 0x11) AM_WRITENOP
	AM_RANGE(0x12, 0x12) AM_WRITE (cm_outport1_w)	/* output port */
	AM_RANGE(0x13, 0x13) AM_WRITENOP	/* seems control for an extra PPI device */
ADDRESS_MAP_END


static READ8_HANDLER( cm91_r )
{
	return 0xff;
}

static ADDRESS_MAP_START( cmast91_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READ(cm91_r)
	AM_RANGE(0x21, 0x21) AM_READ(ay8910_read_port_0_r)
	AM_RANGE(0x22, 0x22) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0x23, 0x23) AM_WRITE(ay8910_control_port_0_w)
ADDRESS_MAP_END

static WRITE8_HANDLER( lucky8_outport_w )
{
	/* lamps */
}

static ADDRESS_MAP_START( lucky8_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_WRITE(goldstar_fg_vidram_w) AM_BASE(&videoram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_WRITE(goldstar_fg_atrram_w) AM_BASE(&colorram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM AM_WRITE(goldstar_reel1_ram_w) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM AM_WRITE(goldstar_reel2_ram_w) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xa800, 0xa9ff) AM_RAM AM_WRITE(goldstar_reel3_ram_w) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xb040, 0xb07f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xb100, 0xb17f) AM_RAM AM_BASE(&goldstar_reel3_scroll)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE(PPI8255, "ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE(PPI8255, "ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb820, 0xb823) AM_DEVREADWRITE(PPI8255, "ppi8255_2", ppi8255_r, ppi8255_w)	/* Input/Output Ports */
	AM_RANGE(0xb830, 0xb830) AM_READWRITE(ay8910_read_port_0_r,ay8910_write_port_0_w)
	AM_RANGE(0xb840, 0xb840) AM_WRITE(ay8910_control_port_0_w)	/* no sound... only use both ports for DSWs */
	AM_RANGE(0xb850, 0xb850) AM_WRITE(lucky8_outport_w)
	AM_RANGE(0xb870, 0xb870) AM_WRITE(sn76496_0_w)	/* sound */
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END


//static WRITE8_HANDLER( ladylinr_outport_w )
//{
/* LAMPS (b840)...

   .... ...x
   .... ..x.
   .... .x..
   .... x...  BET
   ...x ....  SMALL/INFO
   ..x. ....  START
   .x.. ....
   x... ....
*/
//	popmessage("Output: %02X", data);
//}

static ADDRESS_MAP_START( ladylinr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_WRITE(goldstar_fg_vidram_w) AM_BASE(&videoram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_WRITE(goldstar_fg_atrram_w) AM_BASE(&colorram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM AM_WRITE(goldstar_reel1_ram_w) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM AM_WRITE(goldstar_reel2_ram_w) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xa800, 0xa9ff) AM_RAM AM_WRITE(goldstar_reel3_ram_w) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xb040, 0xb07f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xb100, 0xb17f) AM_RAM AM_BASE(&goldstar_reel3_scroll)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE(PPI8255, "ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE(PPI8255, "ppi8255_1", ppi8255_r, ppi8255_w)	/* DSW bank */
	AM_RANGE(0xb830, 0xb830) AM_READWRITE(ay8910_read_port_0_r,ay8910_write_port_0_w)
	AM_RANGE(0xb840, 0xb840) AM_WRITE(ay8910_control_port_0_w)	/* no sound... only use ports */
	AM_RANGE(0xb850, 0xb850) AM_WRITENOP	/* just turn off the lamps, if exist */
	AM_RANGE(0xb870, 0xb870) AM_WRITE(sn76496_0_w)	/* sound */
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( cmv801 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop 2 / Big")          PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stop 1 / D-UP")         PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Stop All / Take")       PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Bet")                   PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop 3 / Small / Info") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Start")                 PORT_CODE(KEYCODE_N)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "35%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "45%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* not checked */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x08, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4") /* not working */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")	/* not working */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x20, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop 3 / Big")          PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stop 1 / D-UP")         PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Stop All / Take")       PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Bet")                   PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop 2 / Small / Info") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Start")                 PORT_CODE(KEYCODE_N)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x00, "Payout Switch" )
	PORT_DIPSETTING(    0x04, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "40%" )
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "30%" )
	PORT_DIPSETTING(    0x06, "38%" )
	PORT_DIPSETTING(    0x05, "46%" )
	PORT_DIPSETTING(    0x04, "54%" )
	PORT_DIPSETTING(    0x03, "62%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "78%" )
	PORT_DIPSETTING(    0x00, "86%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4") /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Flash 'Dyna - C.M.V.4' string in attract" ) PORT_DIPLOCATION("DSW5:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x20, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmaster )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop 3 / Big")          PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stop 1 / D-UP")         PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Take")                  PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Bet / Stop All")        PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop 2 / Small / Info") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Start")                 PORT_CODE(KEYCODE_N)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x00, "Payout Switch" )
	PORT_DIPSETTING(    0x04, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "45%" )
	PORT_DIPSETTING(    0x06, "50%" )
	PORT_DIPSETTING(    0x05, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4") /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")	/* not checked */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x20, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( goldstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet Red/2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3/Small/1/Info")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet Blue/Double/3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1/Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2/Big/Ticket")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start/Stop All/4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* this is not a coin, not sure what it is */
												/* maybe it's used to buy tickets. Will check soon. */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Game Style" )
	PORT_DIPSETTING(    0x01, "Gettoni" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out" )
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Automatic?" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "W-Up '7'" )
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-Up Pay Rate" )
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x20, 0x20, "W-Up Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0xc0, "8 Bet" )
	PORT_DIPSETTING(    0x80, "16 Bet" )
	PORT_DIPSETTING(    0x40, "32 Bet" )
	PORT_DIPSETTING(    0x00, "50 Bet" )

	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )
	PORT_DIPSETTING(    0x00, "75 %" )
	PORT_DIPSETTING(    0x01, "70 %" )
	PORT_DIPSETTING(    0x02, "65 %" )
	PORT_DIPSETTING(    0x03, "60 %" )
	PORT_DIPSETTING(    0x04, "55 %" )
	PORT_DIPSETTING(    0x05, "50 %" )
	PORT_DIPSETTING(    0x06, "45 %" )
	PORT_DIPSETTING(    0x07, "40 %" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0xc0, 0x40, "Coin C" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limited" )
	PORT_DIPSETTING(    0x07, "5000" )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x05, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x01, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Credit Limit" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type of Coin D" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min Bet" )
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Ticket Payment" )
	PORT_DIPSETTING(    0x80, "1 Ticket/100" )
	PORT_DIPSETTING(    0x00, "Pay All" )

	PORT_START("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW7")	/* ??? */
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x20, 0x00, "Show Woman" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chrygld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop 2 / Big / Bonus Game")      PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Blue Bet / D-UP / Card 3")       PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stop 1 / Take")                  PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Red Bet / Card 2")               PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop 3 / Small / Info / Card 1") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Start / Stop All / Card 4")      PORT_CODE(KEYCODE_N)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" )		PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")	/* some of these are wrong */
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )	PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x30, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // D-Type
	PORT_DIPSETTING(    0x20, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )				PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Credit Limit" )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )			PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "D-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "C-Type (Ticket)" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min.Bet" )		PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )				PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )				PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )

	/* DSW5 is not connected yet. Where the hell is connected? */
	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )		PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Coin In Limit" )			PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )			PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Girl" )				PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Coin In Rate" )			PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END

static INPUT_PORTS_START( ncb3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 - Stop 2 / Big / Bonus Game / Switch Controls") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 - Blue Bet / D-UP / Card 3")       PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 - Stop 1 / Take")                  PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 - Red Bet / Card 2")               PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 - Stop 3 / Small / Info / Card 1") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 - Start / Stop All / Card 4")      PORT_CODE(KEYCODE_N)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8 )  PORT_NAME("P2 - Stop 2 / Big / Bonus Game / Switch Controls") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("P2 - Blue Bet / D-UP / Card 3")       PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )  PORT_NAME("P2 - Stop 1 / Take")                  PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("P2 - Red Bet / Card 2")               PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_NAME("P2 - Stop 3 / Small / Info / Card 1") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("P2 - Start / Stop All / Card 4")      PORT_CODE(KEYCODE_H)

	/* to check DIP switches... */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" )		PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")	/* some of these are wrong */
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )	PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x30, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // D-Type
	PORT_DIPSETTING(    0x20, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )				PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Credit Limit" )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )			PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "D-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "C-Type (Ticket)" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min.Bet" )		PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )				PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )				PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )		PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Coin In Limit" )			PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )			PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Girl" )				PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Coin In Rate" )			PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END

static INPUT_PORTS_START( cb3a )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 - Stop 2 / Big / Bonus Game / Switch Controls") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 - Blue Bet / D-UP / Card 3")       PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 - Stop 1 / Take")                  PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 - Red Bet / Card 2")               PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 - Stop 3 / Small / Info / Card 1") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 - Start / Stop All / Card 4")      PORT_CODE(KEYCODE_N)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8 )  PORT_NAME("P2 - Stop 2 / Big / Bonus Game / Switch Controls") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("P2 - Blue Bet / D-UP / Card 3")       PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )  PORT_NAME("P2 - Stop 1 / Take")                  PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("P2 - Red Bet / Card 2")               PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_NAME("P2 - Stop 3 / Small / Info / Card 1") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("P2 - Start / Stop All / Card 4")      PORT_CODE(KEYCODE_H)

	/* to check DIP switches... */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" )		PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")	/* some of these are wrong */
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )	PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x30, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // D-Type
	PORT_DIPSETTING(    0x20, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )				PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Credit Limit" )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )			PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "D-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "C-Type (Ticket)" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min.Bet" )		PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )				PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )				PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )		PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Coin In Limit" )			PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )			PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Girl" )				PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Coin In Rate" )			PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8 )
	PORT_START("IN0")	/* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 - Big / Switch Controls") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 - Double-Up")             PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 - Take Score")            PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 - Bet")                   PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 - Small / Info")          PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 - Start")                 PORT_CODE(KEYCODE_N)

	PORT_START("IN1")	/* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("P2 - Big / Switch Controls") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("P2 - Double-Up")             PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("P2 - Take Score")            PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("P2 - Bet")                   PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("P2 - Small / Info")          PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("P2 - Start")                 PORT_CODE(KEYCODE_H)

	PORT_START("IN2")	/* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	// Coin1?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	// Coin2?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	// Coin3?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	// Coin4?

	PORT_START("IN4")	/* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPSETTING(    0x06, "83%" )
	PORT_DIPSETTING(    0x05, "86%" )
	PORT_DIPSETTING(    0x04, "89%" )
	PORT_DIPSETTING(    0x03, "92%" )
	PORT_DIPSETTING(    0x02, "95%" )
	PORT_DIPSETTING(    0x01, "98%" )
	PORT_DIPSETTING(    0x00, "101%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "W-UP Type" )	PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "Reels (automatic)" )
	PORT_DIPSETTING(    0x00, "Cards (Big/Small)" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Key In" )			PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8a )
	PORT_START("IN0")	/* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 - Big / Switch Controls") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 - Double-Up")             PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 - Take Score")            PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 - Bet")                   PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 - Small / Info")          PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 - Start")                 PORT_CODE(KEYCODE_N)

	PORT_START("IN1")	/* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("P2 - Big / Switch Controls") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("P2 - Double-Up")             PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("P2 - Take Score")            PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("P2 - Bet")                   PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("P2 - Small / Info")          PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("P2 - Start")                 PORT_CODE(KEYCODE_H)

	PORT_START("IN2")	/* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)	// Coin1?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)	// Coin2?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)	// Coin3?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)	// Coin4?

	PORT_START("IN4")	/* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key Out / Attendant") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stats")    PORT_CODE(KEYCODE_0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x02, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "30%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "50%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "W-UP Type" )	PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "Reels (automatic)" )
	PORT_DIPSETTING(    0x00, "Cards (Big/Small)" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Key In" )			PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ladylinr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper Muenze (Hopper Coin") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper Voll (Hopper Fill)")  PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Abschreib (Payout)")         PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Aufsteller (Supervisor)")    PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Buchhaltung (Bookkeeping)")  PORT_CODE(KEYCODE_9)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hoch (High) / Stop 3")       PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Gamble (D-UP)")              PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Nehmen (Take)")              PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Setzen (Bet)")               PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Tief (Low) / Stop 1 / Info") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Start / Stop 2")             PORT_CODE(KEYCODE_C)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Remote Credits" )	PORT_DIPLOCATION("DSW1:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "20 credits" )
	PORT_DIPSETTING(    0x10, "50 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Coin B & C" )		PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x00, "10 credits" )
	PORT_DIPSETTING(    0x20, "20 credits" )
	PORT_DIPNAME( 0x40, 0x40, "Reels Speed" )		PORT_DIPLOCATION("DSW1:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Input Test Mode" )	PORT_DIPLOCATION("DSW1:8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

/*  there are 2 extra DSW banks...
    both are tied to a daughterboard, maybe hooked to a device.
	they are not related to the original hardware, and are not
	listed in the Input Test Mode.
*/
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};


static const gfx_layout charlayout_chry10 =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static const gfx_layout tilelayoutbl =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static const gfx_layout tilelayout_chry10 =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};



static const gfx_layout tiles8x8x3_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles8x32x4_layout =
{
	8,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	 16*8,17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
     24*8,25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	32*8
};

#if 0 // decodes an extra plane for cmv4 / cmasterb, not sure if we need to
static const gfx_layout tiles8x32x5_layout =
{
	8,32,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	 16*8,17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
     24*8,25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	32*8
};
#endif

static GFXDECODE_START( goldstar )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( bl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayoutbl, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( ml )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( chry10 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_chry10,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_chry10, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( ncb3 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128, 4 )
GFXDECODE_END

static GFXDECODE_START( cmv801 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 ) // or is there a register for the +64?
GFXDECODE_END

#if 0 // decodes an extra plane for cmv4 / cmasterb, not sure if we need to
static GFXDECODE_START( cmasterb )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x5_layout, 0, 4 )
GFXDECODE_END
#endif


/*************************************
*      PPI 8255 (x3) Interfaces      *
*************************************/

static const ppi8255_interface ncb3_ppi8255_intf[3] =
{
	{	/* A, B & C set as input */
		DEVICE8_PORT("IN0"),	/* Port A read */
		DEVICE8_PORT("IN3"),	/* Port B read */ //Player2 controls, confirmed.
		NULL,					/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVICE8_PORT("IN1"),	/* Port A read */
		DEVICE8_PORT("IN2"),	/* Port B read */
		DEVICE8_PORT("DSW1"),	/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVICE8_PORT("DSW2"),	/* Port A read */
		NULL,					/* Port B read */
		NULL,					/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	}
};

static const ppi8255_interface cm_ppi8255_intf[2] =
{
	{	/* A, B & C set as input */
		DEVICE8_PORT("IN0"),	/* Port A read */
		DEVICE8_PORT("IN1"),	/* Port B read */
		DEVICE8_PORT("IN2"),	/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVICE8_PORT("DSW1"),	/* Port A read */
		DEVICE8_PORT("DSW2"),	/* Port B read */
		DEVICE8_PORT("DSW3"),	/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	}
};

static const ppi8255_interface lucky8_ppi8255_intf[3] =
{
	{	/* A, B & C set as input */
		DEVICE8_PORT("IN0"),	/* Port A read */
		DEVICE8_PORT("IN1"),	/* Port B read */
		DEVICE8_PORT("IN2"),	/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVICE8_PORT("IN3"),	/* Port A read */
		DEVICE8_PORT("IN4"),	/* Port B read */
		DEVICE8_PORT("DSW1"),	/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVICE8_PORT("DSW2"),	/* Port A read */
		NULL,					/* Port B read */
		NULL,					/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	}
};

static const ppi8255_interface ladylinr_ppi8255_intf[2] =
{
	{	/* A, B & C set as input */
		DEVICE8_PORT("IN0"),	/* Port A read */
		DEVICE8_PORT("IN1"),	/* Port B read */
		DEVICE8_PORT("IN2"),	/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVICE8_PORT("DSW1"),	/* Port A read */
		NULL,					/* Port B read */
		NULL,					/* Port C read */
		NULL,					/* Port A write */
		NULL,					/* Port B write */
		NULL					/* Port C write */
	}
};

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_7_r,	/* DSW 4 */
	input_port_6_r,	/* DSW 3 */
	NULL,
	NULL
};

static const ay8910_interface cm_ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_6_r,	/* DSW 4 */
	input_port_7_r,	/* DSW 5 */
	NULL,
	NULL
};

static const ay8910_interface lucky8_ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_7_r,	/* DSW 3 */
	input_port_8_r,	/* DSW 4 */
	NULL,
	NULL
};

static const ay8910_interface ladylinr_ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	NULL,
	NULL,
	NULL,
	NULL
};

static MACHINE_DRIVER_START( goldstar )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(goldstar_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(goldstar)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( goldstbl )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(goldstar_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(bl)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( moonlght )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(goldstar_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ml)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)// clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END


static PALETTE_INIT(cm)
{
	int i;
	for (i=0;i<0x100;i++)
	{
		int r,g,b;
		UINT8 dat;

		UINT8*proms = memory_region(machine, "proms");

		dat = proms[0x000+i] | (proms[0x100+i]<<4);

		r = (dat & 0x07) << 5;
		g = (dat & 0x38) << 2;
		b = (dat & 0xc0) << 0;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

	}
}

static PALETTE_INIT(cmast91)
{
	int i;
	for (i=0;i<0x100;i++)
	{
		int r,g,b;

		UINT8*proms = memory_region(machine, "proms");

		b = proms[0x000+i]<<4;
		g = proms[0x100+i]<<4;
		r = proms[0x200+i]<<4;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

	}
}

static PALETTE_INIT(lucky8)
{
	int i;
	int r,g,b;
	UINT8 dat;
	UINT8 *proms;

	proms = memory_region(machine, "proms");
	for (i=0;i<0x100;i++)
	{

		dat = proms[0x000+i] | (proms[0x100+i]<<4);

		r = (dat & 0x07) << 5;
		g = (dat & 0x38) << 2;
		b = (dat & 0xc0) << 0;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

	}

	proms = memory_region(machine, "proms2");
	for (i=0;i<0x20;i++)
	{
		dat = proms[i];

		r = (dat & 0x07) << 5;
		g = (dat & 0x38) << 2;
		b = (dat & 0xc0) << 0;

		palette_set_color(machine, i+0x80, MAKE_RGB(r, g, b));
	}

}


static MACHINE_DRIVER_START( chrygld )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(ncb3_map,0)
	MDRV_CPU_IO_MAP(ncb3_readwriteport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ncb3_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ncb3_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", ncb3_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(chry10)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76489, 3000000)	/* 3 MHz. */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("ay", AY8910,1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



static MACHINE_DRIVER_START( ncb3 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(ncb3_map,0)
	MDRV_CPU_IO_MAP(ncb3_readwriteport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ncb3_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ncb3_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", ncb3_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)

	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76489, 3000000)	/* 3 MHz. */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("ay", AY8910,1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cm )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(cm_map,0)
	MDRV_CPU_IO_MAP(cm_portmap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "cm_ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "cm_ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cmv801)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910,1500000)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cmast91 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3000000)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(cm_map,0)
	MDRV_CPU_IO_MAP(cmast91_portmap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "cm_ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "cm_ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(cmv801)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cmast91)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum
MACHINE_DRIVER_END

#if 0 // not currently used, might be needed later
static MACHINE_DRIVER_START( cmb )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(cm_map,0)
	MDRV_CPU_IO_MAP(cm_portmap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cmasterb)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
//  MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END
#endif


static MACHINE_DRIVER_START( lucky8 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 12000000/3) // ?? Mhz
	MDRV_CPU_PROGRAM_MAP(lucky8_map,0)
	//MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", lucky8_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", lucky8_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", lucky8_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_INIT(lucky8)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'

	MDRV_SOUND_ADD("sn1", SN76489, 3000000)	/* 3 MHz. */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("ay", AY8910,1500000)
	MDRV_SOUND_CONFIG(lucky8_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ladylinr )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 12000000/4) // ?? Mhz
	MDRV_CPU_PROGRAM_MAP(ladylinr_map,0)
	//MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ladylinr_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ladylinr_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_INIT(lucky8)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'

	MDRV_SOUND_ADD("sn1", SN76489, 3000000)	/* 3 MHz. */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("ay", AY8910,1500000)
	MDRV_SOUND_CONFIG(ladylinr_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( goldstar )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "gs4-cpu.bin",  0x0000, 0x10000, CRC(73e47d4d) SHA1(df2d8233572dc12e8a4b56e5d4f6c566e4ababc9) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "gs3.bin",      0x00000, 0x08000, CRC(8454ce3c) SHA1(74686ebb91f191db8cbc3d0417a5e8112c5b67b1) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( goldstbl )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "gsb-cpu.bin",  0x0000, 0x10000, CRC(82b238c3) SHA1(1306e700e213f423bdd79b182aa11335796f7f38) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "gsb-spr.bin",  0x00000, 0x08000, CRC(52ecd4c7) SHA1(7ef013020521a0c19ecd67db1c00047e78a3c736) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

/*

Cherry I Gold

Anno    1998
Produttore
N.revisione W4BON (rev.1)


CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46) (missing)
1x D71055C (u40) (missing)
1x YM2149 (u39)
1x SN76489AN (u38)
1x oscillator 12.0C45

ROMs

1x I27256 (u3)
1x I27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
1x GAL20V8 (pl1)(read protected)
1x PALCE20V8H (pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x PEEL22CV10 (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)


Cherry Gold  (Cherry 10)

Anno    1997
Produttore
N.revisione W4BON (rev.1)

CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46)
1x D71055C (u40)
1x WF19054 (u39)(equivalent to AY-3-8910)
1x SN76489AN (u38)
1x PIC16F84 (on a small daughterboard)(read protected)
1x oscillator 12.000

ROMs

1x TMS27C256 (u3)
1x TMS27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
2x PALCE20V8H (pl1,pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x GAL22V10B (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)

*/

ROM_START( chry10 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ver.1h2.u20",  0x0000, 0x10000, CRC(85bbde06) SHA1(f44d335feb4697b195e9fc7e5aeaabf099e21ed8) )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic16f84.bad.dump",    0x00000, 0x014f4, BAD_DUMP CRC(876ff1ed) SHA1(fcd6892e2b8371030af15e4d8c9f4a351ce0551c) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "27c010.u1",      0x00000, 0x20000, CRC(05515cf8) SHA1(366dd44ae93bdc4cf456f97f38edac83441cbc89) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "proms", ROMREGION_DISPOSE )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02e5, "palgal", ROMREGION_DISPOSE )
	ROM_LOAD( "palce20v8h.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "gal22v10b.pl5.bad.dump",     0x00000, 0x02e5, BAD_DUMP CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
ROM_END



ROM_START( chrygld )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ol-v9.u20",  0x00000, 0x10000, CRC(b61c0695) SHA1(63c44b20fd7f76bdb33331273d2610e8cfd31add) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ol-la.u1",      0x00000, 0x20000, CRC(c3c912f1) SHA1(a2131f092ae1971f79a11d6a18b031cd98529320) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "proms", ROMREGION_DISPOSE )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02dd, "palgal", ROMREGION_DISPOSE )
	ROM_LOAD( "gal20v8.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(bf885908) SHA1(6cac1022172ee0c178fd3b9c187b1ffb4742898f) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump", 0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "peel22cv10a.pl5.bad.dump",0x00000, 0x02dd, BAD_DUMP CRC(8e6075d9) SHA1(f2c1b6497a4d9e873d36b89771c135a2cd91d05f) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
ROM_END


ROM_START( moonlght )
	ROM_REGION( 0x20000, "main", 0 )
	ROM_LOAD( "4.bin",  	  0x0000, 0x20000, CRC(ecb06cfb) SHA1(e32613cac5583a0fecf04fca98796b91698e530c) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END



ROM_START( ncb3 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "8.512", 0x00000, 0x10000, CRC(1f669cd0) SHA1(fd394119e33c017507fde87a710577e37dcdec07) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "2.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "3.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	//ROM_LOAD( "4.256", 0x10000, 0x08000, BAD_DUMP CRC(a390f1f2) SHA1(0a04a5af51f91f04773125f703c7cd3397d192f2) ) // FIXED BITS (xxxx1xxx) - use main_7.256 from set below instead?
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "7.764", 0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764", 0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "5.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "1.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", ROMREGION_DISPOSE )	/* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END

/*
mame -romident cb3.zip
cpu_u6.512          NO MATCH
main_3.764          = 5.764                 New Cherry Bonus 3
main_4.764          = 1.764                 New Cherry Bonus 3
main_5.256          = 2.256                 New Cherry Bonus 3
main_6.256          = 3.256                 New Cherry Bonus 3
main_7.256          NO MATCH

C:\mame061208>src\mame\mamedriv.c

*/

ROM_START( cb3 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "cpu_u6.512", 0x00000, 0x10000, CRC(d17c936b) SHA1(bf90edd214118116da675bcfca41247d5891ac90) ) // encrypted??

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "main_5.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "main_6.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	/* 2 roms missing - the first 2 roms below taken from above set */
	ROM_LOAD( "7.764",      0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764",      0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "main_3.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "main_4.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", ROMREGION_DISPOSE )	/* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END

/*
CB3A
Known differences with ncb3:

- Double-Up rate: 50% and 80% instead of 80% and 90%.

*/

ROM_START( cb3a )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "cb3a01.bin", 0x00000, 0x10000, CRC(53b099ab) SHA1(612d86d7f011a554903400e60e2c4a0d4f24e095) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "2.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "3.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "7.764", 0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764", 0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "5.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "1.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", ROMREGION_DISPOSE )	/* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END



ROM_START( cmv801 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "prg512",   0x0000, 0x10000, CRC(2f6e3fe9) SHA1(c5ffa51478a0dc2d8ff6a0f286cfb461011bb55d) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "m5.256",   0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "m6.256",   0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "m7.256",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "m3.64",     0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "m4.64",	   0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "m1.64",     0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "m2.64",     0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x200, "proms", 0 ) // pal
	ROM_LOAD( "prom2.287", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "prom3.287", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 ) // something else?
	ROM_LOAD( "prom1.287", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/*
2764.u10                m4.64                   IDENTICAL
27256.u11               m6.256                  IDENTICAL
2764.u14                m1.64                   IDENTICAL
2764.u15                m3.64                   IDENTICAL
27256.u16               m5.256                  IDENTICAL
27256.u4                m7.256                  IDENTICAL
82s129.u46              prom1.287               IDENTICAL
82s129.u70              prom3.287               IDENTICAL
82s129.u84              prom2.287               IDENTICAL
2764.u9                 m2.64                   IDENTICAL
27512.u53               prg512                  4.640198%
27256.u81                                       NO MATCH

PCB Layout
----------

|-----|  |------|  |---------------------------|
|     |--|      |--|  ROM.U4                   |
|                     ROM.U11  ROM.U10  ROM.U9 |
|_            DSW5(8) ROM.U16  ROM.U15  ROM.U14|
  |  WF19054  DSW4(8)                          |
 _|           DSW3(8) 6116                     |
|             DSW2(8) 6116         6116        |
|             DSW1(8)                          |
|    ?DIP40                        6116        |
|                                              |
|                     PROM.U46            12MHz|
|                                              |
|    8255                       PAL            |
|         ROM.U53                              |
|                                              |
|                                              |
|                                   PAL        |
|BATTERY  PROM.U70        PAL   PAL            |
|_        PROM.U84        6116  ROM.U81    Z80 |
  |--------------------------------------------|
Notes:
      Z80 @ 3.0MHz [12/4]
      WF19054 = AY-3-8910 @ 1.5MHz [12/8]
      ?DIP40 - Maybe another 8255 or HD6845 or something else?
*/

ROM_START( cmv4 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "27256.u81",  0x0000, 0x1000, CRC(e27e98a3) SHA1(1eb03f6c770f25ff5e3c25a1f9b9294c6b3c61d9) )
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "27256.u16",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "27256.u11",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "27256.u4",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "2764.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "2764.u10",	0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2764.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "2764.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	// contains a bitmap? and an extra plane for gfx2, should it be used?
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/* looks like a bootleg of cmv4 */
ROM_START( cmaster )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "3.bin",   0x00000, 0x1000, CRC(ccb64229) SHA1(532f4b59952702a3609ff20239acbbacaf71f38f) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_IGNORE(0x8000) // 2nd half is identical

	ROM_REGION( 0x30000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "6.bin",   0x00000, 0x10000, CRC(a98d610f) SHA1(d14b3bc8bd6dc9fe2d3fb05ec08224b1a9d52bee) )
	ROM_LOAD( "7.bin",   0x10000, 0x10000, CRC(a0ffd2d6) SHA1(e78d20d3ab578ccc880bc19678782cb1f8d3671e) )
	ROM_LOAD( "8.bin",   0x20000, 0x10000, CRC(4f67fca7) SHA1(808e84b9b1f67f137528bb76b0e8aac3dceba20c) )

	// 4-9 all of these contain the same bitmap at 0x0000-0xe000
	// rom 9 matches the rom containing the bitmap in the cmv4 set, but also contains an extra gfx2 plane
	//  should that plane be used??
	ROM_REGION( 0x50000, "graphics", ROMREGION_DISPOSE )
	ROM_LOAD( "4.bin",   0x00000, 0x10000, CRC(52240e0f) SHA1(7b8375e1f91fdff2b4ccc2e81fbcf843f7ede292) )
	ROM_LOAD( "5.bin",	 0x10000, 0x10000, CRC(763973c1) SHA1(b364f22041f1d678332554edb3c718cf0ad778b4) )
	ROM_LOAD( "1.bin",   0x20000, 0x10000, CRC(634fe2ad) SHA1(2284a09446c8928060270861d372a19c0c9d827a) )
	ROM_LOAD( "2.bin",   0x30000, 0x10000, CRC(a3d59f79) SHA1(588c45550cca673390a35af9617c68c853ff84ba) )
	ROM_LOAD( "9.bin",   0x40000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	// for now we don't copy the extra plane..
	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_COPY( "graphics", 0x0e000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x1e000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x2e000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x3e000, 0x06000, 0x2000 )

	// copy one of the versions of the bitmap
	ROM_REGION( 0x10000, "user1", ROMREGION_DISPOSE )
	ROM_COPY( "graphics", 0x40000, 0x00000, 0xe000 )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/* doesn't seem to have 'cherry master 2' anywhere on the screen.. */
/*

    Cherry Master II ver 8.41

    CPU : LH0080B Z80B CPU

    RAM : MB8128-15 x2

        MB8416-20 x2

        HM6116

    Sound : Winbond WF19054

    Crystal : 12.000 Mhz

    Dip SW :

    4x8 dip + 1 Switch (main test ???)
*/

ROM_START( cm2v841 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "u81.9",  0x0000,  0x1000, CRC(09e44314) SHA1(dbb7e9afc9a1dc0d4ce7b150324077f3f3579c02) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",	0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", ROMREGION_DISPOSE )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cm2841a )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "msii841.u81",  0x3000,  0x1000, CRC(977db602) SHA1(0fd3d6781b654ac6befdc9278f84ca708d5d448c) )
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "msii841.u16", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "msii841.u11", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "msii841.u4",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	/* these gfx are in a different format to usual */
	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "msii841.u1",  0x0000,  0x4000, CRC(cf322ed2) SHA1(84df96229b7bdba0ab498e3bf9c77d7a7661f7b3) )
	ROM_LOAD( "msii841.u2",  0x4000,  0x4000, CRC(58c05653) SHA1(59454c07f4fe5b684d078cf97f2b1ee05b02f4ed) )

	ROM_REGION( 0x10000, "user1", ROMREGION_DISPOSE )
	ROM_LOAD( "msii841.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmast91 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "8.bin",   0x00000, 0x01000, CRC(31a16d9f) SHA1(f007148449d66954b780f12a9f910968a4052482) )
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x40000, "user1", 0 ) // unknown, bitmaps, or sound?
	ROM_LOAD( "9.bin",   0x00000, 0x40000, CRC(92342276) SHA1(f9436752f2ec67cf873fd01c729c7c113dc18be0) ) // ?

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "7.bin",   0x00000, 0x8000, CRC(1edf1f1d) SHA1(558fa01f1efd7f6541047d3930bdce0974bae5b0))
	ROM_LOAD( "6.bin",   0x08000, 0x8000, CRC(13582e74) SHA1(27e318542606b8e8d38250749ba996402d314abd) )
	ROM_LOAD( "5.bin",   0x10000, 0x8000, CRC(28ff88cc) SHA1(46bc0407be857e8348159735b60cfb660f047a56) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.bin",     0x00000, 0x8000, CRC(71bdab69) SHA1(d2c594ed88d6368df15b623c48eecc1c219b839e) )
	ROM_LOAD( "2.bin",     0x08000, 0x8000, CRC(fccd48d7) SHA1(af564f5ef9ff5b6363897ce6bdf0b21123911fd4) )
	ROM_LOAD( "3.bin",     0x10000, 0x8000, CRC(dc77d04a) SHA1(d8656130cde54d4bb96307899f6d607867e49e6c) )
	ROM_LOAD( "4.bin",	   0x18000, 0x8000, CRC(0dbabaa2) SHA1(44235b19dac1c996e2166672b03f6e3888ecbefa) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "p1.bin", 0x0000, 0x0100, CRC(ac529f04) SHA1(5bc92e50c85bb23e609172cc15c430ddea7fdcb5) )
	ROM_LOAD( "p2.bin", 0x0100, 0x0100, CRC(3febce95) SHA1(c7c0fec0fb024ebf7d7365a09d28ba3d0037b0b4) )
	ROM_LOAD( "p3.bin", 0x0200, 0x0100, CRC(99dbdf19) SHA1(3680335406f63289f8d9a81b4cd163e4aa0c14d4) )

	ROM_REGION( 0x100, "proms2", 0 ) // screen layout?
	ROM_LOAD( "p4.bin", 0x0000, 0x0100, CRC(72212427) SHA1(e87a91f28284313c706ebb8175a3586780636e31) )
ROM_END

/*

        Lucky 8 Line
        Falcon 1989

        G14           6116  9
        G13   D13           8
              D12
        6116                 Z80
        6116                 8255
        7                    8255
        6            SW1     8255
 12MHz  5            SW2     8910
        4  6116      SW4
        3  6116      SW3
        2  6116
        1  6116

    ---

*/

ROM_START( lucky8 )
	ROM_REGION( 0x8000, "main", 0 )
	ROM_LOAD( "8",	 0x0000, 0x4000, CRC(a187573e) SHA1(864627502025dbc83a0049fc98505655cec7b181) )
	ROM_LOAD( "9",   0x4000, 0x4000, CRC(6f62672e) SHA1(05662ef1a70f93b09e48de497b049a282f070735) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "5",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "6",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(c415b9d0) SHA1(fd558fe8a116c33bbd712a639224d041447a45c1) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "d12", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	/* missing prom? - using one from other dump */
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "g14", 0x0000, 0x0100, CRC(bd48de71) SHA1(e4fa1e774af1499bc568be5b2deabb859d8c8172) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "g13", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

/*
unknown koren or chinese bootleg of something?

XTAL 12MHz
Z80 @ 3MHz
AY3-8910 @ 1.5MHz
8255 x3
RAM 6116 x5
76489 x1
8-position DSW x4

----

13 and 13 files
g14                                             FIXED BITS (0000xxxx)
g14                                             BADADDR                xxxxx---
g13                                             FIXED BITS (1x1xxxxx11xxxxxx)
d13                                             FIXED BITS (xxxxxx0xxxxxxxxx)
d13                                             1ST AND 2ND HALF IDENTICAL
d12                                             FIXED BITS (0000xxxx)
                        prom1                   FIXED BITS (xxxxxx0xxxxxxxxx)
                        prom1                   1ST AND 2ND HALF IDENTICAL
                        prom2                   FIXED BITS (1x11xxxx11x1xxxx)
                        prom3                   FIXED BITS (0000xxxx)
                        prom4                   FIXED BITS (0000xxxx)
                        prom5                   FIXED BITS (00001xxx)
                        prom5                   BADADDR                xxxxxxx-
d13                     prom1                   IDENTICAL
d12                     prom3                   IDENTICAL
6                       7                       IDENTICAL
5                       6                       IDENTICAL
4                       5                       IDENTICAL
3                       4                       IDENTICAL
2                       3                       IDENTICAL
1                       2                       IDENTICAL
7                       8                       99.990845%
g13                     prom2                   90.625000%
g14                     prom4                   61.718750%
9                                               NO MATCH
8                                               NO MATCH
                        1                       NO MATCH
                        prom5                   NO MATCH

There is a loop at 0x0010 that decrement (HL) when is pointing to ROM space.
This should be worked out or patched to allow boot the game.
Seems to be related to timing since once patched the game is very fast.

*/
ROM_START( lucky8a )
	ROM_REGION( 0x8000, "main", 0 )
	// we have to patch this, it might be bad
	ROM_LOAD( "1",	 0x0000, 0x8000, BAD_DUMP CRC(554cddff) SHA1(8a0678993c7010f70adc9e9443b51cf5929bf110) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "6",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "7",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "8",  0x10000, 0x8000, CRC(80b35f06) SHA1(561d257d7bc8976cfa08f36d84961f1263509b5b) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "2",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "3",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "4",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "5",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "prom3", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "prom1", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "prom5", 0x000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "prom2", 0x0000, 0x0020, CRC(7b1a769f) SHA1(788b3573df17d398c74662fec4fd7693fc27e2ef) )
ROM_END

/*
    LADY LINER - TAB Austria

    Hardware Notes:
    ---------------

    CPU:   1x Z80.
    Sound: 1x AY8930.
    I/O:   2x P8255A.

    Clock: 1x Xtal @ 12.0000MHz.

    ROMs:  1x NM27C256Q (ladybrd)
           7x M27C64A (1,2,3,4,71,72,73)
           2x PROM AM27S19PC (39,73)
           3x PROM AM27S21PC (37,38,96)

    RAM:   4x HY6116ALP-10 (near graphics ROMs)
           1x HY6116ALP-10 (near program ROM)

    1x 8 DIP Switches.
    1x trimmer (volume).

    Connectors:  1x 18x2 edge connector.
                 1x 22x2 edge connector.

    Both connectors are wired to a strange small PCB with:
	1x 6x2 edge connector + 1x 60x2 edge connector (with smaller spacing) + 2x 8 DIP Switches.

    Silkscreened on PCB:
    "TAB AUSTRIA"

    Sticker on PCB:
	"TAB Austria" & "LL 2690"

*/

ROM_START( ladylinr )
	ROM_REGION( 0x8000, "main", 0 )
	ROM_LOAD( "ladybrd.bin",	0x0000, 0x8000, CRC(44d2aed0) SHA1(1afe6178d1bf4ad0b623f33be879ed5180ad2db1) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ll73.bin",	0x00000, 0x8000, CRC(afa4a705) SHA1(779340713df7029553cfc1c57997dfdd96a0f0cc) )
	ROM_LOAD( "ll72.bin",	0x08000, 0x8000, CRC(bd1d8a39) SHA1(01e37704c753352024e79b0b83b040f8288b9aed) )
	ROM_LOAD( "ll71.bin",	0x10000, 0x8000, CRC(1c417efa) SHA1(491579a76d80c4f488ef94393d12a190571ae285) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.bin",	0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2.bin",	0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3.bin",	0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4.bin",	0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s21pc.38",	0x0000, 0x0100, CRC(06a0ee6f) SHA1(e793fbb9e14e4e6c6d6783a36edee74f28e7e214) )
	ROM_LOAD( "am27s21pc.37",	0x0100, 0x0100, CRC(8589d23c) SHA1(9629c0d8af3cce47ef376898a4be84c0752a265b) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "am27s19pc.39",	0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "am27s21pc.96",	0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "am27s19pc.73",	0x0000, 0x0020, CRC(b48d0b41) SHA1(01d2d0fd5e79c17043e97146001150b4b32ac86c) )
ROM_END



/* these 'Amcoe' games look like bootlegs of cherry master
  the z80 roms are encrypted */
ROM_START( schery98 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "sk98133.bin",	 0x0000, 0x10000, CRC(77a5dd54) SHA1(e693f477b42b83f1f5e45fb7c56486119bf91856) )

	ROM_REGION( 0x20000, "graphics", ROMREGION_DISPOSE )
	ROM_LOAD( "sk98h.bin",  0x00000, 0x10000, CRC(0574357b) SHA1(96a846f6d49dd67ad078ad9240e632f79ae1b437) )
	ROM_LOAD( "sk98l.bin",  0x10000, 0x10000, CRC(ebe802a4) SHA1(178542c204fd1027874e6d2c099edaa7878c993f) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_COPY( "graphics", 0x04000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x0c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x14000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x18000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x4000 )
	ROM_COPY( "graphics", 0x10000, 0x00000, 0x4000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sk98u19.bin", 0x0000, 0x0100, CRC(796c7960) SHA1(0f64a8119fd4809a5ac79585b415b34b2a83e9dc) )
	ROM_LOAD( "sk98u20.bin", 0x0100, 0x0100, CRC(a0862663) SHA1(e27c3bba5f87b51a19ea8068f2ce7b82a6f0eedb) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "sku1920.bin", 0x0000, 0x0100, CRC(a8c86d5e) SHA1(d19cd5e57ac8fdd685540c1bb2e1474d1326362b) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sk98t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END



ROM_START( schery97 )
	ROM_REGION( 0x20000, "main", 0 )
	ROM_LOAD( "sc352.bin",	 0x00000, 0x10000, CRC(d3857d85) SHA1(e97b2634f0993631023c08f6baf800461abfad12) )
	ROM_LOAD( "sc352c4.bin", 0x10000, 0x10000, CRC(44f55f6e) SHA1(8b6e8618281de480979de37c7b36a0e68a524f47) ) // ?? alt program?

	ROM_REGION( 0x20000, "graphics", ROMREGION_DISPOSE )
	ROM_LOAD( "sc97h.bin",  0x00000, 0x10000, CRC(def39ee2) SHA1(5e6817bd947ebf16d0313285a00876b796b71cab) )
	ROM_LOAD( "sc97l.bin",  0x10000, 0x10000, CRC(6f4d6aea) SHA1(6809c26e6975cac97b0f8c01a508d4e022859b1a) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_COPY( "graphics", 0x04000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x0c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x14000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x18000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x4000 )
	ROM_COPY( "graphics", 0x10000, 0x00000, 0x4000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sc97u19.bin", 0x0000, 0x0100, CRC(6a01caca) SHA1(9b3e9eebb9fcc8770f7e92f0f1c0434516ee613d) )
	ROM_LOAD( "sc97u20.bin", 0x0100, 0x0100, CRC(5899c1d5) SHA1(c335b99bb58da3a11005a8952a15d9f43bdff157) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "scu1920.bin", 0x0000, 0x0100, CRC(3aa291dd) SHA1(f35c916b5463ff9ec6e57048af29a746148a13af) )
	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sc97t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END


static DRIVER_INIT(goldstar)
{
	int A;
	UINT8 *ROM = memory_region(machine, "main");

	for (A = 0;A < 0x10000;A++)
	{
		if ((A & 0x30) == 0)
			ROM[A] ^= 0x82;
		else
			ROM[A] ^= 0xcc;
	}
}

// this block swapping is the same for chry10, chrygld and cb3
//  the underlying bitswaps / xors are different however
static void do_blockswaps(UINT8* ROM)
{
	int A;
	UINT8 *buffer;

	static UINT16 cherry_swaptables[32] = {
		/* to align with goldstar */
		0x0800, 0x4000, 0x2800, 0x5800,
		0x1800, 0x3000, 0x6800, 0x7000,
		0x0000, 0x4800, 0x2000, 0x5000,
		0x1000, 0x7800, 0x6000, 0x3800,
		/* bit below, I'm not sure, no exact match, but only the first ones matter,
           as the is just garbage */
		0xc000, 0xc800, 0xd000, 0xd800,
		0xe000, 0xe800, 0xf000, 0xf800,
		0x8000, 0x8800, 0x9000, 0x9800,
		0xa000, 0xa800, 0xb000, 0xb800,
	};

	buffer = malloc(0x10000);
	memcpy(buffer,ROM,0x10000);

	// swap some 0x800 blocks around..
	for (A =0;A<32; A++)
	{
		memcpy(ROM+A*0x800,buffer+cherry_swaptables[A],0x800);
	}

	free(buffer);
}

static void dump_to_file(running_machine* machine, UINT8* ROM)
{
	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, 0x10000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}

UINT8 decrypt(UINT8 cipherText, UINT16 address)
{
	int idx;
	UINT8 output;
	int rotation[8] = {1, 0, 0, 1, 0, 1, 1, 1};
	int sbox[8] = {0x08, 0x08, 0x28, 0x00, 0x20, 0x20, 0x88, 0x88};

	idx = BIT(cipherText, 1) | (BIT(address,0) << 1) | (BIT(address, 4) << 2);

	if (rotation[idx] == 0)
		output = BITSWAP8(cipherText, 5, 6, 3, 4, 7, 2, 1, 0);   // rotates bit #3, #5 and #7 in one direction...
	else
		output = BITSWAP8(cipherText, 3, 6, 7, 4, 5, 2, 1, 0);   // ... or in the other

	return output ^ sbox[idx];
}

UINT8 chry10_decrypt(UINT8 cipherText)
{
	return cipherText ^ (BIT(cipherText, 4) << 3) ^ (BIT(cipherText, 1) << 5) ^ (BIT(cipherText, 6) << 7);
}

static DRIVER_INIT( chry10 )
{
	UINT8 *ROM = memory_region(machine, "main");
	int size = memory_region_length(machine, "main");
	int start = 0;

	UINT8 *buffer;
	int i;

	for (i = start; i < size; i++)
	{
		ROM[i] = chry10_decrypt(ROM[i]);
	}

	buffer = malloc_or_die(size);
	memcpy(buffer, ROM, size);

	free(buffer);

	do_blockswaps(ROM);
	dump_to_file(machine, ROM);
}

static DRIVER_INIT( cb3 )
{
	UINT8 *ROM = memory_region(machine, "main");
	int size = memory_region_length(machine, "main");
	int start = 0;

	UINT8 *buffer;
	int i;

	for (i = start; i < size; i++)
	{
		ROM[i] = decrypt(ROM[i], i);
	}

	buffer = malloc_or_die(size);
	memcpy(buffer, ROM, size);

	free(buffer);

	do_blockswaps(ROM);
	dump_to_file(machine, ROM);
}


static DRIVER_INIT( chrygld )
{
	int A;
	UINT8 *ROM = memory_region(machine, "main");
	do_blockswaps(ROM);

	// a data bitswap
	for (A = 0;A < 0x10000;A++)
	{
		UINT8 dat = ROM[A];
		dat =  BITSWAP8(dat,5,6,3,4,7,2,1,0);
		ROM[A] = dat;
	}

	dump_to_file(machine, ROM);
}

static DRIVER_INIT(cm)
{
	UINT8 *ROM = memory_region(machine, "main");

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0021] = 0x9b;
	ROM[0x0025] = 0x9b;
}

static DRIVER_INIT(cmv4)
{
	UINT8 *ROM = memory_region(machine, "main");

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0209] = 0x9b;
	ROM[0x020d] = 0x9b;
}

static DRIVER_INIT(cmast91)
{
	UINT8 *ROM = memory_region(machine, "main");

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0070] = 0x9b;
	ROM[0x0a92] = 0x9b;
}

static DRIVER_INIT(lucky8a)
{
	UINT8 *ROM = memory_region(machine, "main");

	ROM[0x0010] = 0x21;
}


/*********************************************
*                Game Drivers                *
**********************************************

      YEAR  NAME      PARENT    MACHINE   INPUT     INIT      ROT    COMPANY              FULLNAME                                 FLAGS  */
GAME( 199?, goldstar, 0,        goldstar, goldstar, goldstar, ROT0, "IGS",               "Golden Star",                            0 )
GAME( 199?, goldstbl, goldstar, goldstbl, goldstar, 0,        ROT0, "IGS",               "Golden Star (Blue version)",             0 )
GAME( 199?, moonlght, goldstar, moonlght, goldstar, 0,        ROT0, "bootleg",           "Moon Light (bootleg of Golden Star)",    0 )
GAME( 199?, chrygld,  0,        chrygld,  chrygld,  chrygld,  ROT0, "bootleg",           "Cherry Gold I",                          0 )
GAME( 199?, chry10,   0,        chrygld,  ncb3,     chry10,   ROT0, "bootleg",           "Cherry 10 (bootleg of Golden Star)",     GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )

// are these really dyna, or bootlegs?
GAME( 199?, ncb3,     0,        ncb3,     ncb3,     0,        ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, set 1)",     0 )
GAME( 199?, cb3a,     ncb3,     ncb3,     cb3a,     0,        ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, set 2)",     0 )
GAME( 199?, cb3,      ncb3,     ncb3,     ncb3,     cb3,      ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, encrypted)", 0 )

// cherry master hardware has a rather different mem map, but is basically the same
GAME( 198?, cmv801,   0,        cm,       cmv801,   cm,       ROT0, "Corsica",           "Cherry Master (Corsica, ver.8.01)",      0 ) /* says ED-96 where the manufacturer is on some games.. */

GAME( 1992, cmv4,     0,        cm,       cmv4,     cmv4,     ROT0, "Dyna",              "Cherry Master (ver.4)",                  0 )
GAME( 1991, cmaster,  cmv4,     cm,       cmaster,  0,        ROT0, "Dyna",              "Cherry Master I (ver.1.01)",             0 )
GAME( 198?, cm2v841,  cmv4,     cm,       cmv801,   0,        ROT0, "Dyna",              "Cherry Master (II?) (ver.8.41, set 1)",  GAME_NOT_WORKING )
GAME( 198?, cm2841a,  cmv4,     cm,       cmv801,   0,        ROT0, "Dyna",              "Cherry Master (II?) (ver.8.41, set 2)",  GAME_NOT_WORKING )

GAME( 1991, cmast91,  0,        cmast91,  cmv801,   cmast91,  ROT0, "Dyna",              "Cherry Master '91 (ver.1.30)",           GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_NOT_WORKING )

GAME( 1989, lucky8,   0,        lucky8,   lucky8,   0,        ROT0, "Wing Co.Ltd / GEI", "New Lucky 8 Lines (set 1)",              GAME_IMPERFECT_COLORS )
GAME( 1989, lucky8a,  lucky8,   lucky8,   lucky8a,  lucky8a,  ROT0, "Wing Co.Ltd / GEI", "New Lucky 8 Lines (set 2)",              GAME_IMPERFECT_COLORS )

GAME( 198?, ladylinr, 0,        ladylinr, ladylinr, 0,        ROT0, "TAB Austria",       "Lady Liner",                             0 )

// bootlegs of cherry master?
GAME( 1998, schery98, 0,        cm,       cmv801,   0,        ROT0, "Amcoe",             "Skill Cherry '98",                       GAME_NOT_WORKING )
GAME( 1998, schery97, 0,        cm,       cmv801,   0,        ROT0, "Amcoe",             "Skill Cherry '97",                       GAME_NOT_WORKING )

