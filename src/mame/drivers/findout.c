/***************************************************************************

Reel Fun    (c) 1986
Find Out    (c) 1987
Trivia      (c) 1984 / 1986
Quiz        (c) 1991

driver by Nicola Salmoria

Trivia games "No Coins" mode: if DSW "No Coins" is on, coin inputs are
replaced by a 6th button to start games. This is a feature of the PCB for private use.

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "machine/ticket.h"
#include "sound/dac.h"


static UINT8 drawctrl[3];

static WRITE8_HANDLER( findout_drawctrl_w )
{
	drawctrl[offset] = data;
}

static WRITE8_HANDLER( findout_bitmap_w )
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


static READ8_DEVICE_HANDLER( portC_r )
{
	return 4;
}

static READ8_DEVICE_HANDLER( port1_r )
{
	const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	return input_port_read(device->machine, "IN0") | (ticket_dispenser_0_r(space, 0) >> 5);
}

static WRITE8_DEVICE_HANDLER( lamps_w )
{
	/* 5 button lamps */
	set_led_status(0,data & 0x01);
	set_led_status(1,data & 0x02);
	set_led_status(2,data & 0x04);
	set_led_status(3,data & 0x08);
	set_led_status(4,data & 0x10);
	/* lamps 6, 7, 8 in gt507, may be hopper slide 1, 2, 3 ? */
	set_led_status(5,data & 0x20);
	set_led_status(6,data & 0x40);
	set_led_status(7,data & 0x80);
}

static WRITE8_DEVICE_HANDLER( sound_w )
{
	const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	/* bit 3 - coin lockout (lamp6 in test modes, set to lamp 10 as in getrivia.c) */
	coin_lockout_global_w(~data & 0x08);
	set_led_status(9,data & 0x08);

	/* bit 5 - ticket out in trivia games */
	ticket_dispenser_w(space, 0, (data & 0x20)<< 2);

	/* bit 6 enables NMI */
	interrupt_enable_w(space, 0,data & 0x40);

	/* bit 7 goes directly to the sound amplifier */
	dac_data_w(devtag_get_device(device->machine, "dac"), ((data & 0x80) >> 7) * 255);

//  logerror("%s: sound_w %02x\n",cpuexec_describe_context(device->machine),data);
//  popmessage("%02x",data);
}

static const ppi8255_interface ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("DSWA"),		/* Port A read */
		DEVCB_HANDLER(port1_r),			/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_HANDLER(sound_w),			/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("IN1"),		/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_HANDLER(portC_r),			/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_HANDLER(lamps_w),			/* Port B write */
		DEVCB_NULL						/* Port C write */
	}
};

static MACHINE_RESET( findout )
{
	ticket_dispenser_init(machine, 100, 1, 1);
}


static READ8_HANDLER( catchall )
{
	int pc = cpu_get_pc(space->cpu);

	if (pc != 0x3c74 && pc != 0x0364 && pc != 0x036d)	/* weed out spurious blit reads */
		logerror("%04x: unmapped memory read from %04x\n",pc,offset);

	return 0xff;
}

static WRITE8_HANDLER( banksel_main_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "maincpu") + 0x8000);
}
static WRITE8_HANDLER( banksel_1_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "maincpu") + 0x10000);
}
static WRITE8_HANDLER( banksel_2_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "maincpu") + 0x18000);
}
static WRITE8_HANDLER( banksel_3_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "maincpu") + 0x20000);
}
static WRITE8_HANDLER( banksel_4_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "maincpu") + 0x28000);
}
static WRITE8_HANDLER( banksel_5_w )
{
	memory_set_bankptr(space->machine, 1,memory_region(space->machine, "maincpu") + 0x30000);
}


/* This signature is used to validate the question ROMs. Simple protection check? */
static int signature_answer,signature_pos;

static READ8_HANDLER( signature_r )
{
	return signature_answer;
}

static WRITE8_HANDLER( signature_w )
{
	if (data == 0) signature_pos = 0;
	else
	{
		static const UINT8 signature[8] = { 0xff, 0x01, 0xfd, 0x05, 0xf5, 0x15, 0xd5, 0x55 };

		signature_answer = signature[signature_pos++];

		signature_pos &= 7;	/* safety; shouldn't happen */
	}
}



static ADDRESS_MAP_START( findout_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r,ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r,ppi8255_w)
	/* banked ROMs are enabled by low 6 bits of the address */
	AM_RANGE(0x601f, 0x601f) AM_WRITE(banksel_main_w)
	AM_RANGE(0x602f, 0x602f) AM_WRITE(banksel_5_w)
	AM_RANGE(0x6037, 0x6037) AM_WRITE(banksel_4_w)
	AM_RANGE(0x603b, 0x603b) AM_WRITE(banksel_3_w)
	AM_RANGE(0x603d, 0x603d) AM_WRITE(banksel_2_w)
	AM_RANGE(0x603e, 0x603e) AM_WRITE(banksel_1_w)
	AM_RANGE(0x6200, 0x6200) AM_WRITE(signature_w)
	AM_RANGE(0x6400, 0x6400) AM_READ(signature_r)
	AM_RANGE(0x7800, 0x7fff) AM_ROM /*space for diagnostic ROM?*/
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK(1)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(findout_drawctrl_w)
	AM_RANGE(0xc000, 0xffff) AM_WRITE(findout_bitmap_w)  AM_BASE(&videoram)
	AM_RANGE(0x0000, 0xffff) AM_READ(catchall)
ADDRESS_MAP_END

#define REELFUN_STANDARD_INPUT \
	PORT_START("IN0") \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
\
	PORT_START("IN1")	/* IN1 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1 Left A-Z") \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2 Right A-Z") \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("3 Select Letter") \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Start")\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Solve Puzzle") \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define TRIVIA_STANDARD_INPUT \
	PORT_START("IN0") \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* ticket status */ \
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
\
	PORT_START("IN1")	/* IN1 */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

static INPUT_PORTS_START( reelfun )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x01, "Coinage Multiplier" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING( 0x08, "Credits per Coin" )
	PORT_DIPSETTING( 0x00, "Coins per Credit" )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )
	PORT_DIPSETTING( 0x10, "Horizontal" )
	PORT_DIPSETTING( 0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	REELFUN_STANDARD_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( findout )
	PORT_START("DSWA")		/* DSW A */
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Game Repetition" )
	PORT_DIPSETTING( 0x08, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Orientation" )
	PORT_DIPSETTING( 0x10, "Horizontal" )
	PORT_DIPSETTING( 0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, "Buy Letter" )
	PORT_DIPSETTING( 0x20, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Starting Letter" )
	PORT_DIPSETTING( 0x40, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Letter" )
	PORT_DIPSETTING( 0x80, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )

	REELFUN_STANDARD_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( gt103 )
	PORT_START("DSWA")		/* DSW A */
	PORT_DIPNAME( 0x07, 0x01, "Coinage Multiplier" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING( 0x08, "Credits per Coin" )
	PORT_DIPSETTING( 0x00, "Coins per Credit" )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )
	PORT_DIPSETTING( 0x10, "Horizontal" )
	PORT_DIPSETTING( 0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	TRIVIA_STANDARD_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( gt103a )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x01, "Questions" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
//  PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Show Answer" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Max Coins" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "10" )
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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* ticket status */
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( quiz )
	PORT_INCLUDE( gt103a )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	/* no tickets */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no coin 2 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no tickets */
INPUT_PORTS_END

static INPUT_PORTS_START( gt507uk )
	PORT_START("DSWA")		/* DSW A */
	PORT_DIPNAME( 0x01, 0x00, "If Ram Error" )
	PORT_DIPSETTING( 0x01, "Freeze" )
	PORT_DIPSETTING( 0x00, "Play" )
	PORT_DIPNAME( 0x0a, 0x08, "Payout" )
	PORT_DIPSETTING( 0x0a, "Bank" )
	PORT_DIPSETTING( 0x08, "N/A" )
	PORT_DIPSETTING( 0x02, "Credit" )
	PORT_DIPSETTING( 0x00, "Direct" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Hardware" )
	PORT_DIPSETTING( 0x04, "Hopper" )
	PORT_DIPSETTING( 0x00, "Solenoid" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	TRIVIA_STANDARD_INPUT
	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* coin 3, 2, 4 order verified in test mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
INPUT_PORTS_END



static MACHINE_DRIVER_START( findout )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,4000000)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(findout_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_MACHINE_RESET(findout)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(48, 511-48, 16, 255-16)

	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(generic_bitmapped)
	MDRV_VIDEO_UPDATE(generic_bitmapped)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( reelfun ) /* v7.01 */
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "reelfun.cnt",          0x00000, 0x4000, CRC(d9d1e92b) SHA1(337f66a37b3734d565b3ff3d912e0f690fd7c445) )
	ROM_LOAD( "reelfun.prg",          0x08000, 0x2000, CRC(615d846a) SHA1(ffa1c47393f4f364aa34d14cf3ac2f56d9eaecb0) )	/* banked */
	ROM_LOAD( "reelfun-1-title",      0x10000, 0x8000, CRC(0e165fbc) SHA1(a3a5b7db72ab86efe973f649f5dfe5133830e3fc) )	/* banked ROMs for solution data */
	ROM_LOAD( "reelfun-2-place",      0x18000, 0x8000, CRC(a0066bfd) SHA1(b6f031ab50eb396be79e79e06f2101400683ec3e) )
	ROM_LOAD( "reelfun-3-phrase",     0x20000, 0x8000, CRC(199e36b0) SHA1(d9dfe39c9a4fca1169150f8941f8ebc499dfbaf5) )
	ROM_LOAD( "reelfun-4-person",     0x28000, 0x8000, CRC(49b0710b) SHA1(a38b3251bcb8683d43bdb903036970140a9735e6) )
	ROM_LOAD( "reelfun-5-song_title", 0x30000, 0x8000, CRC(cce01c45) SHA1(c484f5828928edf39335cedf21acab0b9e2a6881) )
ROM_END

ROM_START( findout )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "12.bin",       0x00000, 0x4000, CRC(21132d4c) SHA1(e3562ee2f46b3f022a852a0e0b1c8fb8164f64a3) )
	ROM_LOAD( "11.bin",       0x08000, 0x2000, CRC(0014282c) SHA1(c6792f2ff712ba3759ff009950d78750df844d01) )	/* banked */
	ROM_LOAD( "13.bin",       0x10000, 0x8000, CRC(cea91a13) SHA1(ad3b395ab0362f3decf178824b1feb10b6335bb3) )	/* banked ROMs for solution data */
	ROM_LOAD( "14.bin",       0x18000, 0x8000, CRC(2a433a40) SHA1(4132d81256db940789a40aa1162bf1b3997cb23f) )
	ROM_LOAD( "15.bin",       0x20000, 0x8000, CRC(d817b31e) SHA1(11e6e1042ee548ce2080127611ce3516a0528ae0) )
	ROM_LOAD( "16.bin",       0x28000, 0x8000, CRC(143f9ac8) SHA1(4411e8ba853d7d5c032115ce23453362ab82e9bb) )
	ROM_LOAD( "17.bin",       0x30000, 0x8000, CRC(dd743bc7) SHA1(63f7e01ac5cda76a1d3390b6b83f4429b7d3b781) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(f3b663bb) SHA1(5a683951c8d3a2baac4b49e379d6e10e35465c8a) )	/* unknown */
ROM_END

ROM_START( gt507uk )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "triv_3_2.bin",    0x00000, 0x4000, CRC(2d72a081) SHA1(8aa32acf335d027466799b097e0de66bcf13247f) )
	ROM_LOAD( "rom_ad.bin",      0x08000, 0x2000, CRC(c81cc847) SHA1(057b7b75a2fe1abf88b23e7b2de230d9f96139f5) )
	ROM_LOAD( "aerospace",       0x10000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) )
	ROM_LOAD( "english_sport_4", 0x18000, 0x8000, CRC(6ae8a63d) SHA1(c6018141d8bbe0ed7619980bf7da89dd91d7fcc2) )
	ROM_LOAD( "general_facts",   0x20000, 0x8000, CRC(f921f108) SHA1(fd72282df5cee0e6ab55268b40785b3dc8e3d65b) )
	ROM_LOAD( "horrors",         0x28000, 0x8000, CRC(5f7b262a) SHA1(047480d6bf5c6d0603d538b84c996bd226f07f77) )
	ROM_LOAD( "pop_music",       0x30000, 0x8000, CRC(884fec7c) SHA1(b389216c17f516df4e15eee46246719dd4acb587) )
ROM_END

ROM_START( gt5 ) /* v5.06, From a TRIV3D romboard */
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program",         0x00000, 0x4000, CRC(e9d6226c) SHA1(42e62c5cafa3f051bf48c18c8c549ffcd4c766c5) )
	ROM_LOAD( "entertainment_2", 0x10000, 0x8000, CRC(c75c2331) SHA1(9c5947616a4cba2623c599def6cf3b2b1981b681) ) /* rom / question set #15 */
	ROM_LOAD( "facts_2",         0x18000, 0x8000, CRC(7836ef31) SHA1(6a84cfa39de392eed46a4b37752e00b6d094bbd6) )
	ROM_LOAD( "new_science_3",   0x20000, 0x8000, CRC(fcbc3bc3) SHA1(2dbdd39dce9dbf53c0954dec44a4f5109243dc60) )
	ROM_LOAD( "nfl_football",    0x28000, 0x8000, CRC(42eb2849) SHA1(c24e681a508ef8350f7e5d50aea2c31cf70ce5c9) )
	ROM_LOAD( "adult_sex_6",     0x30000, 0x8000, CRC(d66f35f7) SHA1(81b56756230b27b0903d0c5df30439726526afe2) )
ROM_END

ROM_START( gtsers8 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "science",         0x10000, 0x8000, CRC(2f940ebd) SHA1(bead4988ac0a97d70f2a3c0b40a05968436de2ed) )
	ROM_LOAD( "general",         0x18000, 0x8000, CRC(1efa01c3) SHA1(801ef5ab55184e488b08ef99ebd641ea4f7edb24) )
	ROM_LOAD( "sports",          0x20000, 0x8000, CRC(6bd1ba9a) SHA1(7caac1bd438a9b1d11fb33e11814b5d76951211a) )
	ROM_LOAD( "soccer",          0x28000, 0x8000, CRC(f821f860) SHA1(b0437ef5d31c507c6499c1fb732d2ba3b9beb151) )
	ROM_LOAD( "potpourri",       0x30000, 0x8000, CRC(f2968a28) SHA1(87c08c59dfee71e7bf071f09c3017c750a1c5694) )
	/* Missing Alternate question set: "Adult Sex" */
ROM_END

ROM_START( gtsers9 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "facts",           0x10000, 0x8000, CRC(21bd6181) SHA1(609ae1097a4011e90d03d4c4f03140fbe84c084a) )
	ROM_LOAD( "rock-n-roll",     0x18000, 0x8000, CRC(1be036b1) SHA1(0b262906044950319dd911b956ac2e0b433f6c7f) )
	ROM_LOAD( "television",      0x20000, 0x8000, CRC(731d4cc0) SHA1(184b6e48edda24f50e377a473a1a4709a218181b) )
	ROM_LOAD( "usa_trivia",      0x28000, 0x8000, CRC(829543b4) SHA1(deb0a4132852643ad884cf194b0a2e6671aa2b4e) )
	ROM_LOAD( "adult_sex_2",     0x30000, 0x8000, CRC(0d683f21) SHA1(f47ce3c31c4c5ed02247fa280303e6ae760315df) ) /* Listed as an alternate question set */
	/* Missing "Artists-Athletes" */
ROM_END

ROM_START( gtsers10 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "new_general",    0x10000, 0x8000, CRC(ba1f5b92) SHA1(7e94be0ef6904331d3a6b266e5887e9a15c5e7f9) )
	ROM_LOAD( "new_tv_mash",    0x18000, 0x8000, CRC(f73240c6) SHA1(78020644074da719414133a86a91c1328e5d8929) )
	ROM_LOAD( "new_entrtnmnt",  0x20000, 0x8000, CRC(0f54340c) SHA1(1ca4c23b542339791a2d8f4a9a857f755feca8a1) )
	ROM_LOAD( "new_sports",     0x28000, 0x8000, CRC(19eff1a3) SHA1(8e024ae6cc572176c90d819a438ace7b2512dbf2) )
	ROM_LOAD( "adult_sex_3",    0x30000, 0x8000, CRC(2c46e355) SHA1(387ab389abaaea8e870b00039dd884237f7dd9c6) ) /* Listed as an alternate question set */
	/* Missing "New Science" */
ROM_END

ROM_START( gtsers11 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "rich-famous",    0x10000, 0x8000, CRC(39e07e4a) SHA1(6e5a0bcefaa1169f313e8818cf50919108b3e121) )
	ROM_LOAD( "cars-women",     0x18000, 0x8000, CRC(4c5dd1df) SHA1(f3e2146eeab07ec71617c7614c6e8f6bc844e6e3) )
	ROM_LOAD( "aerospace",      0x20000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) )
	ROM_LOAD( "tv_music",       0x28000, 0x8000, CRC(5138e0fb) SHA1(102146d63752258c2fda95df49289c42b392c838) )
	ROM_LOAD( "gay_times",      0x30000, 0x8000, CRC(c4f9a8cf) SHA1(9247ecc5708aba263e0365fc43a1a7d0c2b7c391) ) /* Listed as an alternate question set */
	/* Missing "General Facts" */
ROM_END

ROM_START( gt103a1 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versiona",  0x00000, 0x4000, CRC(537d6566) SHA1(282a33e4a9fc54d34094393c00026bf31ccd6ab5) )
	ROM_LOAD( "new_science_2",   0x10000, 0x8000, CRC(3bd80fb8) SHA1(9a196595bc5dc6ed5ee5853786839ed4847fa436) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "nfl_football",    0x18000, 0x8000, CRC(d676b7cd) SHA1(d652d2441adb500f7af526d110d0335ea453d75b) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "rock_music",      0x20000, 0x8000, CRC(7f11733a) SHA1(d4d0dee75518edf986cb1241ade45ccb4840f088) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "war_and_peace",   0x28000, 0x8000, CRC(bc709383) SHA1(2fba4c80773abea7bbd826c39378b821cddaa255) )
	ROM_LOAD( "entertainment",   0x30000, 0x8000, CRC(07068c9f) SHA1(1aedc78d071281ec8b08488cd82655d41a77cf6b) )
ROM_END

ROM_START( gt103a2 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "vices",           0x10000, 0x8000, CRC(e6069955) SHA1(68f7453f21a4ce1be912141bbe947fbd81d918a3) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "cops_&_robbers",  0x18000, 0x8000, CRC(8b367c33) SHA1(013468157bf469c9cf138809fdc45b3ba60a423b) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "famous_couples",  0x20000, 0x8000, CRC(e0618218) SHA1(ff64fcd6dec83a2271b63c3ae64dc932a3954ec5) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "famous_quotes",   0x28000, 0x8000, CRC(0a27d8ae) SHA1(427e6ae25e47da7f7f7c3e92a37e330d711da90c) )
ROM_END

ROM_START( gt103a3 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",    0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "rock-n-roll_alt", 0x10000, 0x8000, CRC(8eb83052) SHA1(93e3c1ae6c2048fb44ecafe1013b6a96da38fa84) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "history-geog.",   0x18000, 0x8000, CRC(c9a70fc3) SHA1(4021e5d702844416e8c798ed0a57c9ecd20b1d4b) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "the_sixties",     0x20000, 0x8000, CRC(8cfa854e) SHA1(81428c12f99841db1c61b471ac8d00f0c411883b) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "tv_comedies",     0x28000, 0x8000, CRC(992ae38e) SHA1(312780d651a85a1c433f587ff2ede579456d3fd9) )
ROM_END

ROM_START( gt103aa )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",      0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "entertainment_alt", 0x10000, 0x8000, CRC(9a6628b9) SHA1(c0cb7e974329d4d5b91f107296d21a674e35a51b) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "general_alt",       0x18000, 0x8000, CRC(df34f7f9) SHA1(329d123eea711d5135dc02dd7b89b220ce8ddd28) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "science_alt",       0x20000, 0x8000, CRC(9eaebd18) SHA1(3a4d787cb006dbb23ce346577cb1bb5e543ba52c) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "science_alt2",      0x28000, 0x8000, CRC(ac93d348) SHA1(55550ba6b5daffdf9653854075ad4f8398a5e621) )
	ROM_LOAD( "sports_alt2",       0x30000, 0x8000, CRC(40207845) SHA1(2dddb9685dcefabfde07057a639aa9d08da2329e) )
ROM_END

ROM_START( gt103asx )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",    0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "adult_sex_2",     0x10000, 0x8000, CRC(0d683f21) SHA1(f47ce3c31c4c5ed02247fa280303e6ae760315df) )
	ROM_LOAD( "adult_sex_2_alt", 0x18000, 0x8000, CRC(8c0eacc8) SHA1(ddaa25548d161394b41c65a2db57a9fcf793062b) )
	ROM_LOAD( "adult_sex_3_alt", 0x20000, 0x8000, CRC(63cbd1d6) SHA1(8dcd5546dc8688d6b8404d5cf63d8a59acc9bf4c) )
	ROM_LOAD( "adult_sex_4",     0x28000, 0x8000, CRC(36a75071) SHA1(f08d31f241e1dc9b94b940cd2872a692f6f8475b) )
	ROM_LOAD( "adult_sex_5",     0x30000, 0x8000, CRC(fdbc3729) SHA1(7cb7cec4439ddc39de2f7f62c25623cfb869f493) )
ROM_END

ROM_START( quiz )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x00000, 0x4000, CRC(4e3204da) SHA1(291f1c9b8c4c07881621c3ecbba7af80f86b9520) )
	ROM_LOAD( "2.bin",        0x10000, 0x8000, CRC(b79f3ae1) SHA1(4b4aa50ec95138bc8ee4bc2a61bcbfa2515ac854) )
	ROM_LOAD( "3.bin",        0x18000, 0x8000, CRC(9c7e9608) SHA1(35ee9aa36d16bca64875640224c7fe9d327a95c3) )
	ROM_LOAD( "4.bin",        0x20000, 0x8000, CRC(30f6b4d0) SHA1(ab2624eb1a3fd9cd8d44433962d09496cd67d569) )
	ROM_LOAD( "5.bin",        0x28000, 0x8000, CRC(e9cdae21) SHA1(4de4a4edf9eccd8f9f7b935f47bee42c10ad606f) )
	ROM_LOAD( "6.bin",        0x30000, 0x8000, CRC(89e2b7e8) SHA1(e85c66f0cf37418f522c2d6384997d52f2f15117) )

	ROM_REGION( 0x0200, "proms", 0 ) /* unknown */
	ROM_LOAD( "prom_am27s29pc.bin", 0x0000, 0x0200, CRC(19e3f161) SHA1(52da3c1e50c2329454de14cb9c46149e573e562b) )
ROM_END

ROM_START( quiz211 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "1a.bin",         0x000000, 0x4000, CRC(116de0ea) SHA1(9af97b100aa2c79a58de055abe726d6e2e00aab4) )
	ROM_CONTINUE(				0x000000, 0x4000 ) // halves identical
	ROM_LOAD( "hobby.bin",      0x10000, 0x8000, CRC(c86d0c2b) SHA1(987ef17c7b9cc119511a16cbd98ec44d24665af5) )
	ROM_LOAD( "musica.bin",     0x18000, 0x8000, CRC(6b08990f) SHA1(bbc633dc4e0c395269d3d3fbf1f7617ea7adabf1) )
	ROM_LOAD( "natura.bin",     0x20000, 0x8000, CRC(f17b0d59) SHA1(ebe3d5a0247f3065f0c5d4ee0b846a737700f379) )
	ROM_LOAD( "spettacolo.bin", 0x28000, 0x8000, CRC(38b8e37a) SHA1(e6df575f61ac61e825d98eaef99c128647806a75) )
	ROM_LOAD( "mondiali90.bin", 0x30000, 0x4000, CRC(35622870) SHA1(f2dab64106ca4ef07175a0ad9491470964d8a0d2) )

	ROM_REGION( 0x0e00, "proms", 0 ) /* unknown */
	ROM_LOAD( "prom_27s13-1.bin", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-2.bin", 0x0200, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-3.bin", 0x0400, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-4.bin", 0x0600, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-5.bin", 0x0800, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-6.bin", 0x0a00, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_6349-1n.bin", 0x0c00, 0x0200, CRC(19e3f161) SHA1(52da3c1e50c2329454de14cb9c46149e573e562b) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal10l8cn.bin",   0x0000, 0x002c, CRC(86095226) SHA1(e7496efbd5ca240f0df2dfa5627402342c7f5384) )
ROM_END

/*
When games are first run a RAM error will occur because the nvram needs initialising.
When ERROR appears, press F2, then F3 to reset, then F2 again and the game will start
*/

GAME( 1986, gt507uk,  0,       findout, gt507uk, 0, ROT0, "Grayhound Electronics", "Trivia (UK Version 5.07)",               GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1986, gt5,      0,       findout, gt103,   0, ROT0, "Grayhound Electronics", "Trivia (Version 5.06)",                  GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )

GAME( 1984, gtsers8,  0,       findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Questions Series 8)",            GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers9,  gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Questions Series 9)",            GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers10, gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Questions Series 10)",           GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers11, gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Questions Series 11)",           GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103a1,  gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Version 1.03a) (alt 1)",         GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103a2,  gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Version 1.03a) (alt 2)",         GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103a3,  gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Version 1.03a) (alt 3)",         GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103aa,  gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Version 1.03a Alt questions)",   GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103asx, gtsers8, findout, gt103a,  0, ROT0, "Greyhound Electronics", "Trivia (Version 1.03a Sex questions)",   GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )

GAME( 1986, quiz,     0,       findout, quiz,    0, ROT0, "Italian bootleg",       "Quiz (Revision 2)",                      GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )

GAME( 1986, reelfun,  0,       findout, reelfun, 0, ROT0, "Grayhound Electronics", "Reel Fun (Version 7.01)",                GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1987, findout,  0,       findout, findout, 0, ROT0, "Elettronolo",           "Find Out (Version 4.04)",                GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )

GAME( 1991, quiz211,  0,       findout, quiz,    0, ROT0, "Elettronolo",           "Quiz (Revision 2.11)",                   GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
