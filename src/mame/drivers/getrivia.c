/*

Trivia By Greyhound Electronics

  driver by Pierpaolo Prazzoli and graphic fixes by Tomasz Slanina
  based on Find Out driver

ROM BOARD (2764 ROMs)

FILE            CS
TRIVB1.BIN      D900
TRIVB2.BIN      4600

ROM BOARD (27128 ROMs)

FILE            CS
SPRT1_9.BIN     8600
GENERAL5.BIN    A900
ENTR2_9.BIN     B300
COMC2_9.BIN     2800
HCKY5_9.BIN     3700

ROM board has a part # UVM10B  1984
Main board has a part # UV-1B Rev 5 1982-83-84

Processor: Z80
Support Chips:(2) 8255s
RAM: 6117on ROM board and (24) MCM4517s on main board

Trivia games "No Coins" mode: if DSW "No Coins" is on, coin inputs are
replaced by a 6th button to start games. This is a feature of the PCB for private use.

Selection/Poker payout button: if pressed, all coins/credits are gone and added to the
payout bookkeeping, shown in the service mode under the coin in total. Last Winner shows
the last payout. Payout hardware is unknown.

Video Trivia sets (as stated from Greyhound Electronics, Inc. manual):

Series 1: (128K)           Series 2: (128K)           Series 3: (128K)
----------------           ----------------           ----------------
Science                    T.V. Mash                  Beatlemania
General I                  General II                 General III
Sports I                   Sports II                  Sports III
Music                      Comics                     Country-Western
Movies-T.V.                Entertainment              T.V. Soaps


Series 4: (128K)           Series 5: (128K)           Series 6: (128K)
----------------           ----------------           ----------------
History-Geography          The States                 Science II
Star Trek                  James Bond                 General IV
Baseball                   Hockey                     Commercials-Ads
Hollywood                  Elvismania                 Honeymooners
Television I               The Wild West              Television II


Series 7: (128K)           Series 8: (256K)           * Starting with Series 8
----------------           ----------------           "Announcement":
T.V. Dallas                Science                    3 Times as many
General V                  General                    questions in this
Kids Korner                Sports                     series!
Good Guys                  T.V./Entertainment
Biblical                   Adult Sex
                            or alt: Potpourri


Series 9: (256K)           Series 10: (256K)          Series 11: (256K)
----------------           -----------------          -----------------
Facts                      New Science                Rich and Famous
Rock-N-Roll                New General                Fast Women and Cars
Television                 New T.V. Mash              Aerospace
Artists-Athletes           New Entertainment          TV/Music Alternative
U.S.A. Trivia              New Sports                 General Facts
 or alt: Adult Sex 2        or alt: Adult Sex 3        or alt: Gay Times


NOTE: Series 8 and above are version 1.03a (currently in findout.c)

*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "machine/ticket.h"
#include "sound/dac.h"

static UINT8 *drawctrl;

static WRITE8_HANDLER( getrivia_bitmap_w )
{
	int sx,sy;
	int fg,bg,mask,bits;
	static int prevoffset, yadd;

	videoram[offset] = data;

	yadd = (offset==prevoffset) ? (yadd+1):0;
	prevoffset = offset;

	fg = drawctrl[0] & 7;
	bg = 2;
	mask = 0xff;//drawctrl[2];
	bits = drawctrl[1];

	sx = 8 * (offset % 64);
	sy = offset / 64;
	sy = (sy + yadd) & 0xff;


//if (mask != bits)
//  popmessage("color %02x bits %02x mask %02x\n",fg,bits,mask);

	if (mask & 0x80) *BITMAP_ADDR16(tmpbitmap, sy, sx+0) = (bits & 0x80) ? fg : bg;
	if (mask & 0x40) *BITMAP_ADDR16(tmpbitmap, sy, sx+1) = (bits & 0x40) ? fg : bg;
	if (mask & 0x20) *BITMAP_ADDR16(tmpbitmap, sy, sx+2) = (bits & 0x20) ? fg : bg;
	if (mask & 0x10) *BITMAP_ADDR16(tmpbitmap, sy, sx+3) = (bits & 0x10) ? fg : bg;
	if (mask & 0x08) *BITMAP_ADDR16(tmpbitmap, sy, sx+4) = (bits & 0x08) ? fg : bg;
	if (mask & 0x04) *BITMAP_ADDR16(tmpbitmap, sy, sx+5) = (bits & 0x04) ? fg : bg;
	if (mask & 0x02) *BITMAP_ADDR16(tmpbitmap, sy, sx+6) = (bits & 0x02) ? fg : bg;
	if (mask & 0x01) *BITMAP_ADDR16(tmpbitmap, sy, sx+7) = (bits & 0x01) ? fg : bg;
}

static WRITE8_DEVICE_HANDLER( lamps_w )
{
	/* 5 button lamps */
	set_led_status(0,data & 0x01);
	set_led_status(1,data & 0x02);
	set_led_status(2,data & 0x04);
	set_led_status(3,data & 0x08);
	set_led_status(4,data & 0x10);

	/* 3 button lamps for deal, cancel, stand in poker games;
    lamp order verified in poker and selection self tests */
	set_led_status(7,data & 0x20);
	set_led_status(5,data & 0x40);
	set_led_status(6,data & 0x80);
}

static WRITE8_DEVICE_HANDLER( sound_w )
{
	const address_space *space = cputag_get_address_space(device->machine, "cpu", ADDRESS_SPACE_PROGRAM);

	/* bit 3 - coin lockout, lamp10 in poker / lamp6 in trivia test modes */
	coin_lockout_global_w(~data & 0x08);
	set_led_status(9,data & 0x08);

	/* bit 5 - ticket out in trivia games */
	ticket_dispenser_w(space, 0, (data & 0x20)<< 2);

	/* bit 6 enables NMI */
	interrupt_enable_w(space, 0, data & 0x40);

	/* bit 7 goes directly to the sound amplifier */
	dac_data_w(devtag_get_device(device->machine, "dac"), ((data & 0x80) >> 7) * 255);
}

static WRITE8_DEVICE_HANDLER( sound2_w )
{
	/* bit 3,6 - coin lockout, lamp10+11 in selection test mode */
	coin_lockout_w(0, ~data & 0x08);
	coin_lockout_w(1, ~data & 0x40);
	set_led_status(9,data & 0x08);
	set_led_status(10,data & 0x40);

	/* bit 4,5 - lamps 12, 13 in selection test mode;
    12 lights up if dsw maximum bet = 30 an bet > 15 or if dsw maximum bet = 10 an bet = 10 */
	set_led_status(11,data & 0x10);
	set_led_status(12,data & 0x20);

	/* bit 7 goes directly to the sound amplifier */
	dac_data_w(devtag_get_device(device->machine, "dac"), ((data & 0x80) >> 7) * 255);
}

static WRITE8_DEVICE_HANDLER( lamps2_w )
{
	/* bit 4 - play/raise button lamp, lamp 9 in poker test mode  */
	set_led_status(8,data & 0x10);
}

static WRITE8_DEVICE_HANDLER( nmi_w )
{
	const address_space *space = cputag_get_address_space(device->machine, "cpu", ADDRESS_SPACE_PROGRAM);

	/* bit 4 - play/raise button lamp, lamp 9 in selection test mode  */
	set_led_status(8,data & 0x10);

	/* bit 6 enables NMI */
	interrupt_enable_w(space, 0, data & 0x40);
}

static WRITE8_HANDLER( banksel_1_1_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x10000);
}
static WRITE8_HANDLER( banksel_2_1_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x14000);
}
static WRITE8_HANDLER( banksel_3_1_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x18000);
}
static WRITE8_HANDLER( banksel_4_1_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x1c000);
}
static WRITE8_HANDLER( banksel_5_1_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x20000);
}
static WRITE8_HANDLER( banksel_1_2_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x12000);
}
static WRITE8_HANDLER( banksel_2_2_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x16000);
}
static WRITE8_HANDLER( banksel_3_2_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x1a000);
}
static WRITE8_HANDLER( banksel_4_2_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x1e000);
}
static WRITE8_HANDLER( banksel_5_2_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "cpu") + 0x22000);
}

static ADDRESS_MAP_START( getrivia_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK(1)
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x600f, 0x600f) AM_WRITE(banksel_5_1_w)
	AM_RANGE(0x6017, 0x6017) AM_WRITE(banksel_4_1_w)
	AM_RANGE(0x601b, 0x601b) AM_WRITE(banksel_3_1_w)
	AM_RANGE(0x601d, 0x601d) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x601e, 0x601e) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x608f, 0x608f) AM_WRITE(banksel_5_2_w)
	AM_RANGE(0x6097, 0x6097) AM_WRITE(banksel_4_2_w)
	AM_RANGE(0x609b, 0x609b) AM_WRITE(banksel_3_2_w)
	AM_RANGE(0x609d, 0x609d) AM_WRITE(banksel_2_2_w)
	AM_RANGE(0x609e, 0x609e) AM_WRITE(banksel_1_2_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(SMH_RAM) AM_BASE(&drawctrl)
	AM_RANGE(0x8000, 0x9fff) AM_ROM /* space for diagnostic ROM? */
	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(getrivia_bitmap_w) AM_BASE(&videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gselect_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK(1)
	AM_RANGE(0x4000, 0x40ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x4400, 0x4400) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x4401, 0x4401) AM_WRITE(banksel_1_2_w)
	AM_RANGE(0x4402, 0x4402) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x4403, 0x4403) AM_WRITE(banksel_2_2_w)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(SMH_RAM) AM_BASE(&drawctrl)
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(getrivia_bitmap_w) AM_BASE(&videoram)
ADDRESS_MAP_END

// TODO: where are mapped the lower 0x2000 bytes of the banks?
static ADDRESS_MAP_START( amuse_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK(1)
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x606f, 0x606f) AM_WRITE(banksel_5_1_w)
	AM_RANGE(0x6077, 0x6077) AM_WRITE(banksel_4_1_w)
	AM_RANGE(0x607b, 0x607b) AM_WRITE(banksel_3_1_w)
	AM_RANGE(0x607d, 0x607d) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x607e, 0x607e) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(SMH_RAM) AM_BASE(&drawctrl)
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(getrivia_bitmap_w) AM_BASE(&videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gepoker_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK(1)
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x60ef, 0x60ef) AM_WRITE(banksel_3_1_w)
	AM_RANGE(0x60f7, 0x60f7) AM_WRITE(banksel_2_2_w)
	AM_RANGE(0x60fb, 0x60fb) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x60fd, 0x60fd) AM_WRITE(banksel_1_2_w)
	AM_RANGE(0x60fe, 0x60fe) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(SMH_RAM) AM_BASE(&drawctrl)
	AM_RANGE(0x8000, 0xbfff) AM_ROM /* space for diagnostic ROM? */
	AM_RANGE(0xe000, 0xffff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(getrivia_bitmap_w) AM_BASE(&videoram)
ADDRESS_MAP_END


static INPUT_PORTS_START( gselect )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Poker: Discard Cards" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x06, "Poker: Pay on" )
	PORT_DIPSETTING(    0x06, "any Pair" )
	PORT_DIPSETTING(    0x04, "Pair of Eights or better" )
	PORT_DIPSETTING(    0x02, "Pair of Jacks or better" )
	PORT_DIPSETTING(    0x00, "Pair of Aces only" )
	PORT_DIPNAME( 0x08, 0x00, "Maximum Bet" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Poker: Credits needed for 2 Jokers" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xe0, 0x80, "Payout Percentage" )
	PORT_DIPSETTING(    0xe0, "35" )
	PORT_DIPSETTING(    0xc0, "40" )
	PORT_DIPSETTING(    0xa0, "45" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x60, "55" )
	PORT_DIPSETTING(    0x40, "60" )
	PORT_DIPSETTING(    0x20, "65" )
	PORT_DIPSETTING(    0x00, "70" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_IMPULSE(2) PORT_NAME("Button 12 ?")
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(2) PORT_NAME ("Payout")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_IMPULSE(2) PORT_NAME ("Play / Raise")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_IMPULSE(2) PORT_NAME ("Deal")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_IMPULSE(2) PORT_NAME ("Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_IMPULSE(2) PORT_NAME ("Stand")
/*  Button 8, 6, 7 order verified in test mode switch test */

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gepoker )
	PORT_INCLUDE( gselect )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no coin 2 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no button 12 */
INPUT_PORTS_END

static INPUT_PORTS_START( getrivia )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x01, "Questions" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
/*  PORT_DIPSETTING(    0x02, "5" )*/
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Show Answer" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Max Coins" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPNAME( 0x10, 0x00, "Timeout" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Tickets" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "No Coins" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x40)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x00) PORT_NAME ("Start in no coins mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x40)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ticket_dispenser_0_port_r, 0)		/* ticket status */
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( sextriv1 )
	PORT_INCLUDE( getrivia )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no coin 2 */
INPUT_PORTS_END


static const ppi8255_interface getrivia_ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("DSWA"),	/* Port A read */
		DEVCB_INPUT_PORT("IN0"),	/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_HANDLER(sound_w)		/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("IN1"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_HANDLER(lamps_w),		/* Port B write */
		DEVCB_HANDLER(lamps2_w)		/* Port C write */
	}
};

static const ppi8255_interface gselect_ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("DSWA"),	/* Port A read */
		DEVCB_INPUT_PORT("IN0"),	/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_HANDLER(sound2_w)				/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("IN1"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_INPUT_PORT("IN2"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_HANDLER(lamps_w),		/* Port B write */
		DEVCB_HANDLER(nmi_w)		/* Port C write */
	}
};

static MACHINE_RESET( getrivia )
{
	ticket_dispenser_init(machine, 100, 1, 1);
}

static MACHINE_RESET( gselect )
{
}

static MACHINE_DRIVER_START( getrivia )
	MDRV_CPU_ADD("cpu",Z80,4000000) /* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(getrivia_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(48, 511-48, 16, 255-16)

	MDRV_PALETTE_LENGTH(256)

	MDRV_MACHINE_RESET(getrivia)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_VIDEO_START(generic_bitmapped)
	MDRV_VIDEO_UPDATE(generic_bitmapped)

	MDRV_PPI8255_ADD( "ppi8255_0", getrivia_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", getrivia_ppi8255_intf[1] )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gselect )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("cpu")
	MDRV_CPU_PROGRAM_MAP(gselect_map)

	MDRV_MACHINE_RESET(gselect)

	MDRV_PPI8255_RECONFIG( "ppi8255_0", gselect_ppi8255_intf[0] )
	MDRV_PPI8255_RECONFIG( "ppi8255_1", gselect_ppi8255_intf[1] )
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( amuse )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("cpu")
	MDRV_CPU_PROGRAM_MAP(amuse_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gepoker )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("cpu")
	MDRV_CPU_PROGRAM_MAP(gepoker_map)
MACHINE_DRIVER_END

/***************************************************
Rom board is UVM-1A

Contains:
 3 2732  eproms (Program Code, 1 empty socket)
 2 X2212P (Ram chips, no battery backup)
 DM7408N

PCB labeled M075

****************************************************/

ROM_START( jokpoker )
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "m075.1", 0x00000, 0x1000, CRC(ad42465b) SHA1(3f06847a9aecb0592f99419dba9be5f18005d57b) ) /* rom board UMV-1A */
	ROM_LOAD( "m075.2", 0x01000, 0x1000, CRC(bd129fc2) SHA1(2e05ba34922c16d127be32941447013efea05bcd) )
	ROM_LOAD( "m075.3", 0x02000, 0x1000, CRC(45725bc9) SHA1(9e6dcbec955ef8190f2307ddb367b24b7f34338d) )
ROM_END

ROM_START( jokpokera )
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "jpbiwr930-1.bin", 0x00000, 0x2000, CRC(d0f4fec5) SHA1(5fcc72522df66464759d5ba3d5209bc7a3a80002) )  /* rom board UMV-7C */
	ROM_LOAD( "jpbiwr930-2.bin", 0x02000, 0x2000, CRC(824d1aee) SHA1(6eebde351c3b5bbed3796846d8d651b41ed6f84a) )
ROM_END



/***************************************************
Rom board is UVM-1B

Contains:
 4 2732  eproms (Program Code)
 Battery (3.5V litium battery) backed up NEC 8444XF301
 DM7408N

****************************************************/

ROM_START( superbwl )
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "super_bowl.1", 0x00000, 0x1000, CRC(82edf064) SHA1(8a26377590282f51fb39d013452ba11252e7dd05) ) /* rom board UMV-1B */
	ROM_LOAD( "super_bowl.2", 0x01000, 0x1000, CRC(2438dd1f) SHA1(26bbd1cb3d0d5b93f61b92ff95948ac9de060715) )
	ROM_LOAD( "super_bowl.3", 0x02000, 0x1000, CRC(9b111430) SHA1(9aaa755f3e4b369477c1a0525c994a19fe0f6107) )
	ROM_LOAD( "super_bowl.4", 0x03000, 0x1000, CRC(037cad42) SHA1(d4037a28bb49b31358b5d560e5e028d958ae2bc9) )
ROM_END

/***************************************************
Rom board is UVM 10 B

Contains:
 2 2764  eproms (Program Code)
 5 27128 eproms (Question roms)
 Battery (3V litium battery) backed up HM6117P-4
 SN74LS374
 MMI PAL10L8


Sets will be listed by "series" - the program code version
 is not as important as maintaining the correct questions
 sets as per known series
Missing sets will be filled as dumped, as question roms
 are interchangable, operators did thier own swaps

Note: Question roms that contain "#1" (or 2 ect)
      are corrected roms (spelling and / or answers)

****************************************************/

ROM_START( gtsers1 ) /* Series 1 (Complete) */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog101c_right", 0x00000, 0x2000, CRC(767f0e46) SHA1(5de7b54876fcbfb2328174ffe6b656ffea886fcb) ) /* rom board UMV 10 B */
	ROM_LOAD( "prog101c_left",  0x0a000, 0x2000, CRC(24c0a097) SHA1(b8de58baecb92775e0882cd6eca3b9e07cf7c5a5) )
	/* Question roms */
	ROM_LOAD( "science_#1",    0x10000, 0x4000, CRC(68259e09) SHA1(29e848b4744b767c51ff81a756fba7bf96daefec) )
	ROM_LOAD( "general_#1",    0x14000, 0x4000, CRC(25a0ef9d) SHA1(793abd779cc237e14933933747bbf27bbcbfcd32) )
	ROM_LOAD( "sports_#1",     0x18000, 0x4000, CRC(cb1744f5) SHA1(ea3f7bfcecf5c58c26aa0f34908ba5d54f7279ec) )
	ROM_LOAD( "music_#1",      0x1c000, 0x4000, CRC(1b546857) SHA1(31e04bb5016e8ef6dc48f9b3ddaeab5fe04f91c2) )
	ROM_LOAD( "movies-tv_#1",  0x20000, 0x4000, CRC(e9a55dad) SHA1(c87682e72bad3507b24eb6a52b4e430e0bfcdab6) )
ROM_END

ROM_START( gtsers2 ) /* Series 2 (Complete - question roms dated 2/9) */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog101c_right", 0x00000, 0x2000, CRC(767f0e46) SHA1(5de7b54876fcbfb2328174ffe6b656ffea886fcb) ) /* rom board UMV 10 B */
	ROM_LOAD( "prog101c_left",  0x0a000, 0x2000, CRC(24c0a097) SHA1(b8de58baecb92775e0882cd6eca3b9e07cf7c5a5) )
	/* Question roms */
	ROM_LOAD( "tv_mash",          0x10000, 0x4000, CRC(a86990fc) SHA1(6a11b038d48bb97feb4857546349ed93ea1f9273) )
	ROM_LOAD( "general_2",        0x14000, 0x4000, CRC(5798f2b3) SHA1(0636017969d9b1eac5d33cfb18cb36f7cf4cba88) )
	ROM_LOAD( "sports_2_#2",      0x18000, 0x4000, CRC(fb632622) SHA1(c14d8178f5cfc5994e2ab4f829e353fa75b57304) )
	ROM_LOAD( "comics_#1",        0x1c000, 0x4000, CRC(8c5cd561) SHA1(1ca566acf72ce636b1b34ee6b7cafb9584340bcc) )
	ROM_LOAD( "entertainment_#1", 0x20000, 0x4000, CRC(cd3ce4c7) SHA1(4bd121fa5899a96b015605f84179ed82be0a25f3) ) /* Correct spelling of "Acapella" */
ROM_END

ROM_START( gtsers3 ) /* Series 3 (Complete - question roms dated 3/9) */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog102b_right",   0x00000, 0x2000, CRC(e8391f71) SHA1(a955eff87d622d4fcfd25f6d888c48ff82556879) )
	ROM_LOAD( "prog102b_left",    0x0a000, 0x2000, CRC(cc7b45a7) SHA1(c708f56feb36c1241358a42bb7dce25b799f1f0b) )
	/* Question roms */
	ROM_LOAD( "beatlemania_#1", 0x10000, 0x4000, CRC(c35ab539) SHA1(aa7c9b532aeb289b71c179e6ff1cc5b63dbe240c) )
	ROM_LOAD( "general_3",      0x14000, 0x4000, CRC(a60f17a4) SHA1(0d79be9e2e49b9817e94d410e25bb6dcda10aa9e) )
	ROM_LOAD( "sports_3_#3",    0x18000, 0x4000, CRC(b22cec38) SHA1(a416c3de9749fda3ab5ae5841304da0cef900cbf) )
	ROM_LOAD( "country-west",   0x1c000, 0x4000, CRC(3227c475) SHA1(d07ad4876122223fe7ab3f21781e0d847332ea5c) )
	ROM_LOAD( "tv_soaps",       0x20000, 0x4000, CRC(26914f3a) SHA1(aec380cea14d6acb71986f3d65c7620b16c174ae) )
ROM_END

ROM_START( gtsers4 ) /* Series 4 (Incomplete - question roms dated 4/9) */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "history-geog",   0x10000, 0x4000, CRC(76d6b026) SHA1(613809b247cb27773631a1bb34af485c2b1bd486) )
	ROM_LOAD( "star_trek",      0x14000, 0x4000, CRC(19764e00) SHA1(d7ed577dba02776ac58e8f34b833ed07679c0af1) )
	ROM_LOAD( "television_#1",  0x18000, 0x4000, CRC(0f646389) SHA1(23fefe2e6cc26767d52604e7ab15bb4db99a6e94) )
	/* Missing "baseball" */
	/* Missing "hollywood" */
ROM_END

ROM_START( gtsers5 ) /* Series 5 (Incomplete - question roms dated 5/9) */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "james_bond",    0x10000, 0x4000, CRC(fe9fadfd) SHA1(44b3fee1f14148f47b0b40600aabd5bff9b65e85) )
	ROM_LOAD( "hockey",        0x14000, 0x4000, CRC(4874a431) SHA1(f3c11dfbf71d101aa1a6cd3622b282a4ebe4664b) )
	/* Missing "the_states" */
	/* Missing "wild_west" */
	/* Missing "elvismania" */
ROM_END

ROM_START( gtsers7 ) /* Series 7 (Incomplete - question roms dated 7/9?) */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "general_5",     0x10000, 0x4000, CRC(81bf07c7) SHA1(a53f050b4ef8ffc0499b50224d4bbed4af0ca09c) )
	/* Missing "tv_dallas" */
	/* Missing "kids_korner" */
	/* Missing "good_guys" */
	/* Missing "biblical" */
ROM_END

ROM_START( gtsersa ) /* alt or older version questions */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "sports",               0x10000, 0x4000, CRC(9b4a17b6) SHA1(1b5358b5bc83c2817ecfa4e277fa351a679d5023) ) /* Series 1 question */
	ROM_LOAD( "entertainment_#1_old", 0x14000, 0x4000, CRC(2bffb3b4) SHA1(5947ebd708df35cefa86608392909c16b25d0710) ) /* Dated 2/9 - Spells "Acapella" as "Cappella" */
	ROM_LOAD( "sports_2",             0x18000, 0x4000, CRC(e8f8e168) SHA1(d2bc57dc0799dd8817b15857f17c4d7ee4d9f932) ) /* Dated 2/9 */
	ROM_LOAD( "comics",               0x1c000, 0x4000, CRC(7efdfe8f) SHA1(ec255777c61677ca32c49b9da5e85e07c0647e5f) ) /* Dated 2/9 */
	ROM_LOAD( "entertainment",        0x20000, 0x4000, CRC(b670b9e8) SHA1(0d2246fcc6c753694bc9bd1fc05ac439f24059ef) ) /* Dated 2/9 */
ROM_END

ROM_START( gtsersb ) /* alt or older version questions */
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "beatlemania",    0x10000, 0x4000, CRC(cb241960) SHA1(e560b776b2cb5fd29d1663fffdf68f4427d674a9) ) /* Dated 3/9 */
	ROM_LOAD( "sports_3",       0x14000, 0x4000, CRC(5986996c) SHA1(56432c15a3b0204ed527c18e24716f17bb52dc4e) ) /* Dated 3/9 */
	ROM_LOAD( "television",     0x18000, 0x4000, CRC(413f34c8) SHA1(318f6b464449bf3f0c43c4210a667190c774eb67) ) /* Dated 4/9 */
	ROM_LOAD( "sex_triv",       0x1c000, 0x4000, CRC(cd0ce4e2) SHA1(2046ee3da94f00bf4a8b3fc62b1190d58e83cc89) ) /* Dated 7/9 - likely an alt series 7 question set */
	ROM_LOAD( "facts_of_life",  0x20000, 0x4000, CRC(1668c7bf) SHA1(6bf43de26f8a626560579ab75fd0890fe00f99dd) ) /* Uknown series question set */
ROM_END

ROM_START( sextriv1 )
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog1_right",   0x00000, 0x2000, CRC(73abcd12) SHA1(3b985f25a11507878cef6d11420e215065fb0906) )
	ROM_LOAD( "prog1_left",    0x0a000, 0x2000, CRC(04ee6ecd) SHA1(28342fcdcf36b34fa93f1a985163ca5aab03defe) )
	/* Question roms */
	ROM_LOAD( "adult_sex",    0x10000, 0x4000, CRC(509a8183) SHA1(635c784860e423b22aaea94abc53c1d9477cb1df) )
	ROM_LOAD( "arousing_sex", 0x14000, 0x4000, CRC(1dbf4578) SHA1(51a548d5fe59739e62b5f0e9e6ebc7deb8656210) )
	ROM_LOAD( "intimate_sex", 0x18000, 0x4000, CRC(1f46b626) SHA1(04aa5306c69d130e0f84fa390a773e73c06e5e9b) )
	ROM_LOAD( "sizzling_sex", 0x1c000, 0x4000, CRC(c718833d) SHA1(02ea341e56554dd9302fe95f45dcf446a2978917) )
ROM_END

ROM_START( sextriv2 )
	ROM_REGION( 0x24000, "cpu", 0 )
	ROM_LOAD( "prog1_right",     0x00000, 0x2000, CRC(73abcd12) SHA1(3b985f25a11507878cef6d11420e215065fb0906) )
	ROM_LOAD( "prog1_left",      0x0a000, 0x2000, CRC(04ee6ecd) SHA1(28342fcdcf36b34fa93f1a985163ca5aab03defe) )
	/* Question roms */
	ROM_LOAD( "general_sex",     0x10000, 0x4000, CRC(36fed946) SHA1(25d445ab6cb4b6f41a1dd7104ee1141e597b2e9e) )
	ROM_LOAD( "educational_sex", 0x14000, 0x4000, CRC(281cbe03) SHA1(9c3900cd2535f942a5cbae7edd46ac0170e04c52) )
	ROM_LOAD( "novelty_sex",     0x18000, 0x4000, CRC(26603979) SHA1(78061741e5224b3162be51e637a2fbb9a48962a3) )
ROM_END

/***************************************************
Rom board is UVM-4B

Contains 5 2764 eproms, MMI PAL16R4CN

Battery (3V litium battery) backed up HM6117P-4

Roms labeled as:

4/1  at spot 1
BLJK at spot 2
POKR at spot 3
SPRD at spot 4
SLOT at spot 3

Alt set included BONE in place of SPRD & a newer SLOT

These board sets may also be known as the V116 (or V16)
sets as the alt set also included that name on the labels

****************************************************/

ROM_START( gs4002 )
	ROM_REGION( 0x18000, "cpu", 0 )
	ROM_LOAD( "4-1.1",          0x00000, 0x2000, CRC(a456e456) SHA1(f36b96ac31ce0f128ecb94f94d1dbdd88ee03c77) ) /* V16M 5-4-84 */
	/* Banked roms */
	ROM_LOAD( "bljk_3-16-84.2", 0x10000, 0x2000, CRC(c3785523) SHA1(090f324fc7adb0a36b189cf04086f0e050895ee4) )
	ROM_LOAD( "pokr_5-16-84.3", 0x12000, 0x2000, CRC(f0e99cc5) SHA1(02fdc95974e503b6627930918fcc3c029a7a4612) )
	ROM_LOAD( "sprd_1-24-84.4", 0x14000, 0x2000, CRC(5fe90ed4) SHA1(38db69567d9c38f78127e581fdf924aca4926378) )
	ROM_LOAD( "slot_1-24-84.5", 0x16000, 0x2000, CRC(cd7cfa4c) SHA1(aa3de086e5a1018b9e5a18403a6144a6b0ed1036) )
ROM_END

ROM_START( gs4002a )
	ROM_REGION( 0x18000, "cpu", 0 )
	ROM_LOAD( "4-1.1",          0x00000, 0x2000, CRC(a456e456) SHA1(f36b96ac31ce0f128ecb94f94d1dbdd88ee03c77) ) /* V16M 5-4-84 */
	/* Banked roms */
	ROM_LOAD( "bljk_3-16-84.2", 0x10000, 0x2000, CRC(c3785523) SHA1(090f324fc7adb0a36b189cf04086f0e050895ee4) )
	ROM_LOAD( "pokr_5-16-84.3", 0x12000, 0x2000, CRC(f0e99cc5) SHA1(02fdc95974e503b6627930918fcc3c029a7a4612) )
	ROM_LOAD( "bone_5-16-84.4", 0x14000, 0x2000, CRC(eccd2fb0) SHA1(2683e432ffcca4280c31f57b2596e4389bc59b7b) )
	ROM_LOAD( "slot_9-24-84.5", 0x16000, 0x2000, CRC(25d8c504) SHA1(2d52b66e8a1f06f486015440668bd924a123dad0) )
ROM_END

/*
Greyhound Poker board...

Standard Greyhound Electronics Inc UV-1B mainboard.

Rom board UVM 10 B or UMV 10 C

Battery backed up NEC D449C ram
PAL16R4
74L374

Roms in this order on the UMV 10 C board:

Label        Normaly in the slot
--------------------------------
High
Control
rom1         Joker Poker
rom2         Black jack
rom3         Rolling Bones
rom4         Casino Slots
rom5         Horse Race

For UMV 10 B: 1C, 2C, 1, 2, 3, 4, & 5

There looks to be several types and combos for these, some are "ICB" or "IAM"
It also looks like operators mixed & matched to upgrade (some times incorrectly)
their boards.  Sets will be filled in as found and dumped.

There are some versions, like, the ICB sets that use 2764 roms for all roms. While the IAM set uses
27128 roms for all roms.  These roms are the correct size, but it's currently not known if the rom
board (UVM 10 B/C) "sees" them as 27128 or the standard size of 2764.

Dumped, but not known to be supported by any High/Control combo:
ROM_LOAD( "rollingbones_am_3-16-84",  0x16000, 0x4000, CRC(41879e9b) SHA1(5106d5772bf43b28817e27efd16c785359cd929e) ) // Might work with IAM control, once it gets figured out

The ICB set is known as the M105 set as some label sets included that name.

*/

ROM_START( gepoker ) /* v50.02 with most roms for ICB dated 8-16-84 */
	ROM_REGION( 0x1b000, "cpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_8-16",    0x00000, 0x2000, CRC(0103963d) SHA1(9bc646e721048b84111e0686eaca23bc24eee3e2) )
	ROM_LOAD( "high_icb_6-25-85-5",  0x0e000, 0x2000, CRC(dfb6592e) SHA1(d68de9f537d3c14279dc576424d195bb266e3897) )
	/* Banked roms */
	ROM_LOAD( "jokerpoker_icb_8-16-84",    0x10000, 0x2000, CRC(0834a1e6) SHA1(663e6f4e0586eb9b84d3098aef8c596585c27304) )
	ROM_LOAD( "blackjack_icb_8-16-84",     0x12000, 0x2000, CRC(cff27ffd) SHA1(fd85b54400b2f22ae92042b01a2c162e64d2d066) )
	ROM_LOAD( "rollingbones_icb_8-16-84",  0x14000, 0x2000, CRC(52d66cb6) SHA1(57db34906fcafd37f3a361df209dafe080aeac16) )
	ROM_LOAD( "casinoslots_icb_8-16-84",   0x16000, 0x2000, CRC(3db002a3) SHA1(7dff4efceee37b25328303cf0606bf4baa4df5f3) )
	ROM_LOAD( "horserace_icb_3-19-85",     0x18000, 0x2000, CRC(f1e6e61e) SHA1(944b1ab4af911e5ed136f1fca3c44219726eeebb) )
ROM_END

ROM_START( gepoker1 ) /* v50.02 with roms for ICB dated 9-30-86 */
	ROM_REGION( 0x1b000, "cpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_9-30",    0x00000, 0x2000, CRC(08b996f2) SHA1(5f5efb5015ec9571cc94734c18debfadaa28f585) )
	ROM_LOAD( "high_icb_6-25-85-5a", 0x0e000, 0x2000, CRC(6ddc1750) SHA1(ee19206b7f4a98e3e7647414127f4e09b3e9134f) )
	/* Banked roms */
	ROM_LOAD( "jokerpoker_icb_9-30-86",    0x10000, 0x2000, CRC(a1473367) SHA1(9b37ccafc02704e8f1d61150326494e86148d84e) )
	ROM_LOAD( "blackjack_icb_9-30-86",     0x12000, 0x2000, CRC(82804184) SHA1(2e2e6a80c99c8eb226dc54c1d32d0bf24de300a4) )
	ROM_LOAD( "casinoslots_icb_9-30-86",   0x14000, 0x2000, CRC(713c3963) SHA1(a9297c04fc44522ca6891516a2c744712132896a) )
	ROM_LOAD( "beatthespread_icb_9-30-86", 0x16000, 0x2000, CRC(93654d2a) SHA1(3aa5a54b91867c03182e93a7f1607545503a33f7) )
	ROM_LOAD( "instantbingo_t24_10-07-86", 0x18000, 0x2000, CRC(de87ed0a) SHA1(4a26d93368c1a39dd38aabe450c34203101f0ef7) ) /* Found with this set, is it compatible or an operater swap? */
ROM_END

ROM_START( gepoker2 ) /* v50.02 with control dated 9-30-84 */
	ROM_REGION( 0x1b000, "cpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_9-30",  0x00000, 0x2000, CRC(08b996f2) SHA1(5f5efb5015ec9571cc94734c18debfadaa28f585) )
	ROM_LOAD( "high_icb_6-25a",    0x0e000, 0x2000, CRC(6ddc1750) SHA1(ee19206b7f4a98e3e7647414127f4e09b3e9134f) )
	/* Banked roms */
	ROM_LOAD( "jokerpoker_cb_10-19-88",    0x10000, 0x2000, CRC(a590af75) SHA1(63bc64fbc9ac0c489b1f4894d77a4be13d7251e7) )
	ROM_LOAD( "horserace_icb_1-1-87",      0x12000, 0x2000, CRC(6d5092e3) SHA1(ef99d1b858aef3c438c61c2b17e371dc6aca6623) )
ROM_END

ROM_START( amuse ) /* v50.08 with most roms for IAM dated 8-16-84 */
	ROM_REGION( 0x24000, "cpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_iam_8-16",  0x00000, 0x4000, CRC(345434b9) SHA1(ec880f6f5e90aa971937e0270701e323f6a83671) ) /* all roms were 27128, twice the size of other sets */
	ROM_LOAD( "high_iam_8-16",     0x08000, 0x4000, CRC(57000fb7) SHA1(144723eb528c4816b9aa4b0ba77d2723c6442546) ) /* Is only the 1st half used by the board / program? */
	/* Banked roms */
	ROM_LOAD( "jokerpoker_iam_8-16-84",    0x10000, 0x4000, CRC(33794a87) SHA1(2b46809623713582746d9f817db33077f15a3684) ) /* This set is verified correct by 3 different sets checked */
	ROM_LOAD( "blackjack_iam_8-16-84",     0x14000, 0x4000, CRC(6e10b5b8) SHA1(5dc294b4a562193a99b0d307323fcc084a053426) )
	ROM_LOAD( "rollingbones_iam_8-16-84",  0x18000, 0x4000, CRC(26949774) SHA1(20571b955521ec3929430249aa651cee8a97043d) )
	ROM_LOAD( "casinoslots_iam_8-16-84",   0x1c000, 0x4000, CRC(c5a1eec6) SHA1(43d31bfe4cbbb6b86f52f675f513050866443176) )
	ROM_LOAD( "horserace_iam_3-19-84",     0x20000, 0x4000, CRC(7b9e75cb) SHA1(0db8da6f5f59f57886766bec96102d43796567ef) )
ROM_END

static DRIVER_INIT( setbank )
{
	memory_set_bankptr(machine, 1,memory_region(machine, "cpu") + 0x2000);
}

GAME( 1982, jokpoker, 0,        gselect,  gselect,  setbank, ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03B)",            GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1983, jokpokera,jokpoker, gselect,  gselect,  setbank, ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03BI)",            GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1982, superbwl, 0,        gselect,  gselect,  setbank, ROT0, "Greyhound Electronics", "Super Bowl (Version 16.03B)",             GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )

GAME( 1982, gs4002,   0,        gselect,  gselect,  0,       ROT0, "Greyhound Electronics", "Selection (Version 40.02TMB, set 1)",     GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1982, gs4002a,  gs4002,   gselect,  gselect,  0,       ROT0, "Greyhound Electronics", "Selection (Version 40.02TMB, set 2)",     GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )

GAME( 1982, amuse,    0,        amuse,    gepoker,  0,       ROT0, "Greyhound Electronics", "Amuse (Version 50.08 IBA)",               GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )

GAME( 1984, gepoker,  0,        gepoker,  gepoker,  0,       ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 1)",        GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gepoker1, gepoker,  gepoker,  gepoker,  0,       ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 2)",        GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gepoker2, gepoker,  gepoker,  gepoker,  0,       ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 3)",        GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )

GAME( 1984, gtsers1,  0,        getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 1)",             GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers2,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 2)",             GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers3,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 3)",             GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers4,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 4)",             GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers5,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 5)",             GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers7,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 7)",             GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsersa,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Alt revision questions set 1)",   GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsersb,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Alt revision questions set 2)",   GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )

GAME( 1985, sextriv1, 0,        getrivia, sextriv1, 0,       ROT0, "Kinky Kit and Game Co.", "Sexual Trivia (Version 1.02SB, set 1)",  GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1985, sextriv2, sextriv1, getrivia, sextriv1, 0,       ROT0, "Kinky Kit and Game Co.", "Sexual Trivia (Version 1.02SB, set 2)",  GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
