/**************************************************************************

  Winners Circle.
  1980-81-82, Corona Co., LTD.

  Driver written by Roberto Fresca.
  Based on roul driver from Roberto Zandona' & Angelo Salese.


  Games running in this hardware:

  * Winners Circle (81),       1981, Corona Co., LTD.
  * Winners Circle (82),       1982, Corona Co., LTD.
  * Ruleta RE-800 (earlier),   1991, Entretenimientos GEMINIS & GENATRON.
  * Ruleta RE-800 (v1.0),      1991, Entretenimientos GEMINIS & GENATRON.
  * Ruleta RE-800 (v3.0),      1992, Entretenimientos GEMINIS & GENATRON.


  Special thanks to Rob Ragon for his invaluable cooperation.

**************************************************************************

  Game Notes:
  ----------

  Winners Circle.
  4-players horse race game.

  Very rare game, due to massive conversions to any kind of roulette soft.
  
  ------

  This game is a milestone. If you ask any old operator about the gambling
  games history, he will say that every started with the Winners Circle.

  ------

**************************************************************************

  Hardware Notes:
  --------------


  * Winners Circle:

  1x Z80 @ 3 MHz. (main CPU)
  1x Z80 @ 2.4 MHz. (sound CPU)

  1x AY-3-8912A @ 1 MHz.

  16x 8116 (16K x 1bit). DRAM Video (should be set as 32 KB).
  1x 6116 (NVRAM)
  2x 2114 (Sound DRAM)

  4x 2732 (Main program ROMs).
  1x 2716 (Sound program ROM).

  1x 82s123 (Color bipolar PROM)

  1x Xtal @ 24 MHz. (main)
  1x Xtal @ 20 MHz. (video)


  * Ruleta RE-800:

  1x Z80 @ 2 MHz. (main CPU) <--- is this correct?
  1x Z80 @ 2.4 MHz. (sound CPU) <--- is this correct?

  1x AY-3-8912A @ 2 MHz.

  16x 8116 (16K x 1bit). DRAM Video (should be set as 32 KB).
  2x 6116 or 1x 6264 (NVRAM). Need to confirm if the whole 8K are mapped/accessed.
  2x 2114 (Sound DRAM)

  1x 27128 (Main program ROMs).
  1x 2716 (Sound program ROM).

  1x 82s123 (Color bipolar PROM)

  1x Xtal @ 16 MHz. (main)
  1x Xtal @ 20 MHz. (video)

  ...

  This hardware could be either way:

  - Single PCB, with 28 pins connector.
  - Dual PCB, with 18/22 pins connectors.
  - Dual PCB, with 28/28 pins connectors.


**************************************************************************

  Pinouts:
  -------

  H(XX) and M(XX) are the molex and pin equivalent.


  * Single Corona PCB (28)

  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M01/M02           GND |01| -12V                  M11 |
  | H08               GND |02| +5V                   M05 |
  |                    -  |03|  -                        |
  |                    -  |04|  -                        |
  |                    -  |05|  -                        |
  |                    -  |06|  -                        |
  |                    -  |07|  -                        |
  |                    -  |08|  -                        |
  |                    -  |09|  -                        |
  |           Total Reset |10| Jackpot Reset             |
  |                    -  |11|  -                        |
  |                    -  |12|  -                        |
  | H11               BET |13| Cursor UP             H12 |
  | H09        Credits IN |14| Credits OUT           H10 |
  | H14      Cursor RIGHT |15| Cursor DOWN           H13 |
  |                    -  |16| Cursor LEFT           H15 |
  |                    -  |17|  -                        |
  | H07      Credits LOCK |18|  -                        |
  | H01          Player 1 |19| Player 3              H03 |
  | H05          Player 5 |20| Player 4              H04 |
  | H02          Player 2 |21| Credits IN counter    M17 |
  | H06          Player 6 |22| Credits OUT counter   M18 |
  | M16              Sync |23| Red                   M13 |
  | M14             Green |24| Blue                  M15 |
  | M06/M07           +5V |25| +5V               M08/M19 |
  | M24       Speaker (+) |26| -5V                   M10 |
  | M09              +12V |27| +12V                  M09 |
  | M03/M23           GND |28| GND               M04/M12 |
  '-----------------------+--+---------------------------'


  * Dual Corona PCB (18*22)

    Top PCB (CS)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  |                    -  |01|  -                        |
  |                    -  |02|  -                        |
  |                    -  |03|  -                        |
  |                    -  |04| Credits LOCK          H07 |
  |                    -  |05|  -                        |
  |           Total Reset |06|  -                        |
  |         Jackpot Reset |07|  -                        |
  |                    -  |08| Credits IN            H09 |
  |                    -  |09| Credits OUT           H10 |
  | H11               BET |10| Cursor LEFT           H15 |
  | H12         Cursor UP |11|  -                        |
  |                    -  |12| Cursor RIGHT          H14 |
  |                    -  |13| Cursor DOWN           H13 |
  |                    -  |14| GND Credits LOCK      H08 |
  |                    -  |15| Player 3              H03 |
  |                    -  |16| Player 6              H06 |
  |                    -  |17| Player 4              H04 |
  |                    -  |18| Player 5              H05 |
  |                    -  |19| Player 1              H01 |
  |                 Reset |20| Player 2              H02 |
  |                    -  |21| Credits IN counter    M17 |
  | M09              +12V |22| Credits OUT counter   M18 |
  '-----------------------+--+---------------------------'

    Bottom PCB (CI)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M01               GND |01| GND                       |
  | M02               GND |02| GND                       |
  | M05               +5V |03| +5V                   M07 |
  | M06               +5V |04| +5V               M08/M19 |
  |                    -  |05|  -                        |
  | M09              +12V |06| +12V                  M09 |
  | M24       Speaker (+) |07| Speaker (-)           M23 |
  |                    -  |08|  -                        |
  |                    -  |09|  -                        |
  |                    -  |10|  -                        |
  |                    -  |11|  -                        |
  |                    -  |12|  -                        |
  |                    -  |13|  -                        |
  |                    -  |14|  -                        |
  | M15              Blue |15| Red                   M13 |
  | M16              Sync |16| Green                 M14 |
  | M03               GND |17| GND                   M12 |
  | M04               GND |18| GND                       |
  '-----------------------+--+---------------------------'


  * Dual Corona PCB (28*28)

    Top PCB (CS)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M08               +5V |01|  -                        |
  |                    -  |02|  -                        |
  |                    -  |03|  -                        |
  |                    -  |04| Credits LOCK          H07 |
  |                    -  |05|  -                        |
  |           Total Reset |06|  -                        |
  |         Jackpot Reset |07|  -                        |
  |                    -  |08| Credits IN            H09 |
  |                    -  |09| Credits OUT           H10 |
  | H11               BET |10| Cursor LEFT           H15 |
  | H12         Cursor UP |11|  -                        |
  |                    -  |12| Cursor RIGHT          H14 |
  |                    -  |13| Cursor DOWN           H13 |
  |                    -  |14| GND Credits LOCK      H08 |
  |                    -  |15| Player 3              H03 |
  |                    -  |16| Player 6              H06 |
  |                    -  |17| Player 4              H04 |
  |                    -  |18| Player 5              H05 |
  |                    -  |19| Player 1              H01 |
  |                 Reset |20| Player 2              H02 |
  | M09        +12V Audio |21| Credits IN counter    M17 |
  |                    -  |22|  -                        |
  | M24       Speaker (+) |23| Credits OUT counter   M18 |
  |                    -  |24| Speaker (-)           M23 |
  | M05/M19           +5V |25| GND                   M04 |
  |                    -  |26|  -                        |
  |                    -  |27|  -                        |
  |                    -  |28|  -                        |
  '-----------------------+--+---------------------------'

    Bottom PCB (CI)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M06               +5V |01| GND                   M01 |
  | M10               -5V |02| GND                   M02 |
  | M09              +12V |03|  -                        |
  | M11              -12V |04|  -                        |
  |                    -  |05|  -                        |
  |                    -  |06|  -                        |
  |                    -  |07|  -                        |
  | M16              Sync |08|  -                        |
  |                    -  |09|  -                        |
  | M15              Blue |10|  -                        |
  |                    -  |11|  -                        |
  |                    -  |12|  -                        |
  | M12         GND video |13|  -                        |
  |                    -  |14|  -                        |
  |                    -  |15|  -                        |
  | M14             Green |16|  -                        |
  | M13               Red |17|  -                        |
  |                    -  |18|  -                        |
  |                    -  |19|  -                        |
  |                    -  |20|  -                        |
  |                    -  |21|  -                        |
  |                    -  |22|  -                        |
  |                    -  |23|  -                        |
  |                    -  |24|  -                        |
  | M07               +5V |25| GND                   M03 |
  |                    -  |26|  -                        |
  |                    -  |27|  -                        |
  |                    -  |28|  -                        |
  '-----------------------+--+---------------------------'

**************************************************************************

  TODO:

  - Inputs/Outputs for Winners Circle.
  - Check the sound CPU connection in Winners Circle sets.


**************************************************************************/

#define WC_MAIN_XTAL		(XTAL_24MHz)	/* Main for Winners Circle boards */
#define RE_MAIN_XTAL		(XTAL_16MHz)	/* Main for roulette boards */
#define VIDEO_XTAL			(XTAL_20MHz)	/* Video circuitry Xtal (all) */
#define AY_CLK				(1000000)		/* AY-3-8912 clock, measured */


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "re800.lh"
#include "machine/nvram.h"

#define VIDEOBUF_SIZE 512*512

static UINT8 blitter_x_reg = 0;
static UINT8 blitter_y_reg = 0;
static UINT8 blitter_aux_reg = 0;
static UINT8 blitter_unk_reg = 0;
static UINT8 *videobuf;

static UINT8 lamp = 0;
static UINT8 lamp_old = 0;


/*********************************************
*               Video Hardware               *
*********************************************/

static PALETTE_INIT( winner )
{
	int bit6, bit7, bit0, bit1, r, g, b;
	int i;

	for (i = 0; i < 0x20; ++i)
	{
		bit7 = (color_prom[0] >> 7) & 0x01;
		bit6 = (color_prom[0] >> 6) & 0x01;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		b = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		g = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		r = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

static WRITE8_HANDLER( blitter_y_w )
{
	blitter_y_reg = data;
}

static WRITE8_HANDLER( blitter_unk_w )
{
	blitter_unk_reg = data;
}

static WRITE8_HANDLER( blitter_x_w )
{
	blitter_x_reg = data;
}

static WRITE8_HANDLER( blitter_aux_w )
{
	blitter_aux_reg = data;
}

static READ8_HANDLER( blitter_status_r )
{
/* code checks bit 6 and/or bit 7 */
	//return space->machine->rand() & 0xc0;
	/*
		x--- ---- blitter busy
		-x-- ---- vblank
	*/

	return 0x80 | ((space->machine->primary_screen->vblank() & 1) << 6);
}

static void blitter_execute(int x, int y, int color, int width, int flag)
{
	int i;
	int xdir = (flag & 0x10)    ? -1 : 1;
	int ydir = (!(flag & 0x20)) ? -1 : 1;

	if(width == 0) //ignored
		return;

	if((flag & 0xc0) == 0) /* square shape / layer clearance */
	{
		int xp, yp;

		if(x != 128 || y != 128 || width != 8)
			printf("%02x %02x %02x %02x %02x\n", x, y, color, width, flag);

		for(yp = 0; yp < 0x100; yp++)
			for(xp = 0; xp < 0x100; xp++)
				videobuf[(yp & 0x1ff) * 512 + (xp & 0x1ff)] = color;
	}
	else /* line shape */
	{
		//printf("%02x %02x %02x %02x %02x\n",x,y,color,width,flag);

		for(i = 0; i < width; i++)
		{
			videobuf[(y & 0x1ff) * 512 + (x & 0x1ff)] = color;

			if(flag & 0x40) { x+=xdir; }
			if(flag & 0x80) { y+=ydir; }
		}
	}
}

static WRITE8_HANDLER( blitter_trig_wdht_w)
{
	blitter_execute(blitter_x_reg, 0x100 - blitter_y_reg, blitter_aux_reg & 0xf, data, blitter_aux_reg & 0xf0);
}

static VIDEO_START(winner)
{
	videobuf = auto_alloc_array_clear(machine, UINT8, VIDEOBUF_SIZE);
}

static SCREEN_UPDATE(winner)
{
	int x, y;

	for (y = 0; y < 256; y++)
		for (x = 0; x < 256; x++)
			*BITMAP_ADDR16(bitmap, y, x) = videobuf[y * 512 + x];

	return 0;
}


/*******************************************
*           Read & Write Handlers          *
*******************************************/

static WRITE8_HANDLER( sound_latch_w )
{
	soundlatch_w(space, 0, data & 0xff);
	cputag_set_input_line(space->machine, "soundcpu", 0, ASSERT_LINE);
}

static READ8_HANDLER( sound_latch_r )
{
	cputag_set_input_line(space->machine, "soundcpu", 0, CLEAR_LINE);
	return soundlatch_r(space, 0);
}


static WRITE8_HANDLER( ball_w )
{
	lamp = data;

	output_set_lamp_value(data, 1);
	output_set_lamp_value(lamp_old, 0);
	lamp_old = lamp;
}

static READ8_HANDLER( test_r )
{
	return space->machine->rand() & 0xff;
}


/********  Multiplexed Inputs  ********/

static int input_selector = 0;

static READ8_HANDLER( mux_port_r )
{
	switch( input_selector )
	{
		case 0x01: return input_port_read(space->machine, "IN0-1");
		case 0x02: return input_port_read(space->machine, "IN0-2");
		case 0x04: return input_port_read(space->machine, "IN0-3");
		case 0x08: return input_port_read(space->machine, "IN0-4");
		case 0x10: return input_port_read(space->machine, "IN0-5");
		case 0x20: return input_port_read(space->machine, "IN0-6");
	}

	return 0xff;
}

static WRITE8_HANDLER( mux_port_w )
{
/*  - bits -
    7654 3210
    --xx xxxx   Input selector.
    -x-- ----   Credits In counter pulse.
    x--- ----   Credits Out counter pulse.

   Data is written inverted.

*/
	input_selector = (data ^ 0xff) & 0x3f;	/* Input Selector,  */

	coin_counter_w(space->machine, 0, (data ^ 0xff) & 0x40);	/* Credits In (mechanical meters) */
	coin_counter_w(space->machine, 1, (data ^ 0xff) & 0x80);	/* Credits Out (mechanical meters) */
}


/***************************************************
*               --- MEMORY MAPS ---                *
***************************************************/

/* Winner Circle 1981

  Blitter Ports...
  RE-800   -   Winner

    F0           70
    F1           74
    F2           72
    F3           76
    F4           71

  Writes: (begin) EF/D8 (00)
           71 (FF/00)
		   DF (01)
           71 (FF/00)
		   D8 (00....) --> snd_latch?

  Reads: E8/E9....EA/EB/EC/ED/EE

  RAM data is relocated to B800-B8FF

*/
static ADDRESS_MAP_START( winner81_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_RAM
	AM_RANGE(0x8000, 0x8fff) AM_RAM // (nvram)
	AM_RANGE(0xb800, 0xb8ff) AM_RAM // copied from 8000 (0x10 bytes, repeated)
ADDRESS_MAP_END

static ADDRESS_MAP_START( winner81_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x70, 0x70) AM_WRITE(blitter_x_w)
	AM_RANGE(0x71, 0x71) AM_WRITE(blitter_unk_w)
	AM_RANGE(0x72, 0x72) AM_WRITE(blitter_trig_wdht_w)
	AM_RANGE(0x74, 0x74) AM_WRITE(blitter_y_w)
	AM_RANGE(0x75, 0x75) AM_READ(blitter_status_r)
	AM_RANGE(0x76, 0x76) AM_WRITE(blitter_aux_w)

	AM_RANGE(0xd8, 0xd8) AM_WRITENOP
	AM_RANGE(0xdf, 0xdf) AM_WRITE(sound_latch_w)

	AM_RANGE(0xe8, 0xe8) AM_READ_PORT("DSW1")
	AM_RANGE(0xe9, 0xe9) AM_READ_PORT("DSW2")

	AM_RANGE(0xea, 0xea) AM_READ_PORT("IN0")
	AM_RANGE(0xeb, 0xeb) AM_READ_PORT("IN1")
	AM_RANGE(0xec, 0xec) AM_READ_PORT("IN2")
	AM_RANGE(0xed, 0xed) AM_READ_PORT("IN3")
	AM_RANGE(0xee, 0xee) AM_READ_PORT("IN4")
ADDRESS_MAP_END

static ADDRESS_MAP_START(  winner81_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(  winner81_sound_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(sound_latch_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_address_data_w)
ADDRESS_MAP_END

/*  Winners Circle 1982

  Blitter F0-F4
  Status: F5

  Reads:  F9 (maybe DIP, since was polled at beginning)
          F8-FA-F8-FB-FF-FA (F8/FB/FF/FA)

  Writes: FE (writes 01, 02 and 03 during attract) Seems snd_latch or traffic lights.
          after FE write, poll FD constantly...
*/

static ADDRESS_MAP_START( winner82_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_RAM //AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x8000, 0x8fff) AM_RAM //AM_BASE_SIZE_GENERIC(nvram) 801a comm
ADDRESS_MAP_END

static ADDRESS_MAP_START( winner82_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_WRITE(blitter_x_w)
	AM_RANGE(0xf1, 0xf1) AM_WRITE(blitter_y_w)
	AM_RANGE(0xf2, 0xf2) AM_WRITE(blitter_trig_wdht_w)
	AM_RANGE(0xf3, 0xf3) AM_WRITE(blitter_aux_w)
	AM_RANGE(0xf4, 0xf4) AM_WRITE(blitter_unk_w)
	AM_RANGE(0xf5, 0xf5) AM_READ(blitter_status_r)

	AM_RANGE(0xf9, 0xf9) AM_READ_PORT("DSW1")

	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("IN0")
	AM_RANGE(0xfa, 0xfa) AM_READ_PORT("IN1")
	AM_RANGE(0xfb, 0xfb) AM_READ_PORT("IN2")
	AM_RANGE(0xfd, 0xfd) AM_READ(test_r)	/* dunno WTF is, but after the snd latch is polled constantly */
	AM_RANGE(0xfe, 0xfe) AM_WRITE(sound_latch_w)
	AM_RANGE(0xff, 0xff) AM_READ_PORT("IN3")
//	AM_RANGE(0xee, 0xee) AM_READ_PORT("IN4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( winner82_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( winner82_sound_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(sound_latch_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_address_data_w)
ADDRESS_MAP_END

/* Ruleta RE-800

  Blitter: F0-F5
  Status:  F5
  Ball:    FF

  F4        W   (FF/00)
  F8       R    DIP switches? (polled just at game start)
  F9       R    DIP switches? (polled after reset, and when insert credits)

  FC        W   mux selector & meters
  FD       R    muxed port

  FE        W   snd_latch... writes 02 on events, 07 when ball.

  Port:
  0 - credits
  1 - clear credits
  2 - bet
  3 - left
  4 - right
  5 - up
  6 - down
  7 - unknown

*/

static ADDRESS_MAP_START( re800_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("nvram") //801a comm?
ADDRESS_MAP_END

static ADDRESS_MAP_START( re800_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_WRITE(blitter_x_w)
	AM_RANGE(0xf1, 0xf1) AM_WRITE(blitter_y_w)
	AM_RANGE(0xf2, 0xf2) AM_WRITE(blitter_trig_wdht_w)
	AM_RANGE(0xf3, 0xf3) AM_WRITE(blitter_aux_w)
	AM_RANGE(0xf4, 0xf4) AM_WRITE(blitter_unk_w)
	AM_RANGE(0xf5, 0xf5) AM_READ(blitter_status_r)
	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("IN1")
	AM_RANGE(0xf9, 0xf9) AM_READ_PORT("DSW1")
	AM_RANGE(0xfc, 0xfc) AM_WRITE(mux_port_w)
	AM_RANGE(0xfd, 0xfd) AM_READ(mux_port_r)
	AM_RANGE(0xfe, 0xfe) AM_WRITE(sound_latch_w)
	AM_RANGE(0xff, 0xff) AM_WRITE(ball_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( re800_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( re800_sound_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(sound_latch_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_address_data_w)
ADDRESS_MAP_END


/*********************************
*         Input Ports            *
*********************************/

static INPUT_PORTS_START( winner )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( winner82 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")	/* credits */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Player B - Coin 2")	/* 10 credits / pulse */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Player A - Coin 2")	/* 10 credits / pulse */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Player D - Coin 2")	/* 10 credits / pulse */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player C - Coin 2")	/* 10 credits / pulse */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Player B - Coin 1")	/* 1 credit / pulse */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("Player A - Coin 1")	/* 1 credit / pulse */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Player D - Coin 1")	/* 1 credit / pulse */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Player C - Coin 1")	/* 1 credit / pulse */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( re800 )
	/* Multiplexed from just one single port */
	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1)        PORT_NAME("Player 1 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)        PORT_NAME("Player 1 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player 1 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)     PORT_NAME("Player 1 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)    PORT_NAME("Player 1 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)       PORT_NAME("Player 1 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)     PORT_NAME("Player 1 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)     PORT_NAME("Player 1 - Unknown")

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2)     PORT_NAME("Player 2 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)     PORT_NAME("Player 2 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Player 2 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Player 2 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Player 2 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Player 2 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Player 2 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Player 2 - Unknown")

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Player 3 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Player 3 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Player 3 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Player 3 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Player 3 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Player 3 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Player 3 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Player 3 - Unknown")

	PORT_START("IN0-4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Player 4 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Player 4 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player 4 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("Player 4 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Player 4 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Player 4 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("Player 4 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("Player 4 - Unknown")

	PORT_START("IN0-5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Player 5 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Player 5 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Player 5 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("Player 5 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Player 5 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Player 5 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("Player 5 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O) PORT_NAME("Player 5 - Unknown")

	PORT_START("IN0-6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6)     PORT_NAME("Player 6 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)     PORT_NAME("Player 6 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Player 6 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DEL)   PORT_NAME("Player 6 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGDN)  PORT_NAME("Player 6 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_HOME)  PORT_NAME("Player 6 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_END)   PORT_NAME("Player 6 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Player 6 - Unknown")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Credits Lock" )	/* lock the credits in/out */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( re800v3 )
	/* Multiplexed from just one single port */
	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1)        PORT_NAME("Player 1 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)        PORT_NAME("Player 1 - Credits Out / Fin (end)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player 1 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)     PORT_NAME("Player 1 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)    PORT_NAME("Player 1 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)       PORT_NAME("Player 1 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)     PORT_NAME("Player 1 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)     PORT_NAME("Player 1 - Unknown")

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2)     PORT_NAME("Player 2 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)     PORT_NAME("Player 2 - Credits Out / Cambia (change)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Player 2 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Player 2 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Player 2 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Player 2 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Player 2 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Player 2 - Unknown")

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Player 3 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Player 3 - Credits Out / Sel (select)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Player 3 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Player 3 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Player 3 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Player 3 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Player 3 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Player 3 - Unknown")

	PORT_START("IN0-4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Player 4 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Player 4 - Credits Out")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player 4 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("Player 4 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Player 4 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Player 4 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("Player 4 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("Player 4 - Unknown")

	PORT_START("IN0-5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Player 5 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Player 5 - Credits Out")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Player 5 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("Player 5 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Player 5 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Player 5 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("Player 5 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O) PORT_NAME("Player 5 - Unknown")

	PORT_START("IN0-6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6)     PORT_NAME("Player 6 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)     PORT_NAME("Player 6 - Credits Out")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Player 6 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DEL)   PORT_NAME("Player 6 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGDN)  PORT_NAME("Player 6 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_HOME)  PORT_NAME("Player 6 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_END)   PORT_NAME("Player 6 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Player 6 - Unknown")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_F2) PORT_NAME("Settings (in bookkeeping mode)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Bookkeeping Mode" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Credits Lock" )	/* lock the credits in/out */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/*******************************************
*             Machine Drivers              *
*******************************************/

static MACHINE_CONFIG_START( winner81, driver_device )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, WC_MAIN_XTAL/8)	/* measured */
	MCFG_CPU_PROGRAM_MAP(winner81_map)
	MCFG_CPU_IO_MAP(winner81_cpu_io_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_CPU_ADD("soundcpu", Z80, WC_MAIN_XTAL/10)	/* measured */
	MCFG_CPU_PROGRAM_MAP(winner81_sound_map)
	MCFG_CPU_IO_MAP(winner81_sound_cpu_io_map)
	MCFG_CPU_PERIODIC_INT(nmi_line_pulse,120) //unknown timing

//	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_PALETTE_INIT(winner)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) //not accurate
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE(winner)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(winner)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8912, AY_CLK)	/* measured */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( winner82, driver_device )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, WC_MAIN_XTAL/8)	/* measured */
	MCFG_CPU_PROGRAM_MAP(winner82_map)
	MCFG_CPU_IO_MAP(winner82_cpu_io_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_CPU_ADD("soundcpu", Z80, WC_MAIN_XTAL/10)	/* measured */
	MCFG_CPU_PROGRAM_MAP(winner82_sound_map)
	MCFG_CPU_IO_MAP(winner82_sound_cpu_io_map)
	MCFG_CPU_PERIODIC_INT(nmi_line_pulse,120) //unknown timing

//	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_PALETTE_INIT(winner)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) //not accurate
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE(winner)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(winner)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8912, AY_CLK)	/* measured */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( re800, driver_device )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, RE_MAIN_XTAL/8)	/* measured 2MHz. but seems a bit low */
	MCFG_CPU_PROGRAM_MAP(re800_map)
	MCFG_CPU_IO_MAP(re800_cpu_io_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_CPU_ADD("soundcpu", Z80, RE_MAIN_XTAL/8)	/* measured 2MHz. but seems a bit low */
	MCFG_CPU_PROGRAM_MAP(re800_sound_map)
	MCFG_CPU_IO_MAP(re800_sound_cpu_io_map)
	MCFG_CPU_PERIODIC_INT(nmi_line_pulse,120) //unknown timing

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_PALETTE_INIT(winner)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) //not accurate
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 32*8-1)
	MCFG_SCREEN_UPDATE(winner)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(winner)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8912, AY_CLK)	/* measured */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/**************** Corona Co,LTD. Hardware ****************
*  > ROM Loading Routines...                             *
*********************************************************/
/***************************************************

  Winners Circle
  1980-81-82 Corona Co.LTD.

  PCB silkscreened CRN-MK-1000

  2x Z80.
  1x AY-3-8912.

  Xtal: 1x 24 MHz. (CPU)
        1x 20 MHz. (Video)

  2x 8 DIP switches banks.

  All ICs are scratched to avoid the recognizement.

  Note:
  4_2732_80e0.bin is identical to son_2716_4070.bin,
  for whatever reason on this set

***************************************************/

ROM_START(winner81)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("1_2732_c61e.bin",	0x0000, 0x1000, CRC(841cdbd1) SHA1(87caeec0a80460c72408ceae28480fe2f3ba3052) )
	ROM_LOAD("2_2732_eafa.bin",	0x1000, 0x1000, CRC(216f71d2) SHA1(913454bc78487c099c854e35d594454077266590) )
	ROM_LOAD("3_2732_121e.bin",	0x2000, 0x1000, CRC(b5849762) SHA1(f62ea11ae5834dd84081d8716798c2ac0b879a35) )
	ROM_LOAD("4_2732_80e0.bin",	0x3000, 0x1000, CRC(13d0d2a6) SHA1(13b102f23e559971c4728fbbe0938341aacbff11) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("son_2716_4070.bin",	0x0000, 0x0800, CRC(547068a8) SHA1(fe0e1272feb0196b14554d7c3cb043212508bfbc) )
	ROM_RELOAD(                     0x0800, 0x0800 ) //reads here during horse race

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "corona_82s123.bin",	0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END

ROM_START(winner82)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("p1.32.bin",	0x0000, 0x1000, CRC(5eb58841) SHA1(160ba8a19b0926aab6f47497e625449d35efea2a) )
	ROM_LOAD("p2.32.bin",	0x1000, 0x1000, CRC(52567aeb) SHA1(b4723aff00a18f3cb9ee5c3071ed671f920458cf) )
	ROM_LOAD("p3.32.bin",	0x2000, 0x1000, CRC(1ae24244) SHA1(a10fba097b18607b1bc10d8720a020c1e01e75c3) )
	ROM_LOAD("p4.32.bin",	0x3000, 0x1000, CRC(15631824) SHA1(24d5607b186dc880609dd076012d0bc29d7581ba) )

	ROM_REGION( 0x10000, "soundcpu", 0 )	/* need confirmation if it's the same */
	ROM_LOAD("son_2716_4070.bin",	0x0000, 0x0800, BAD_DUMP CRC(547068a8) SHA1(fe0e1272feb0196b14554d7c3cb043212508bfbc) )
	ROM_RELOAD(                     0x0800, 0x0800 ) //reads here during horse race

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "corona_82s123.bin",	0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END

/***************************************************

  Ruleta RE-800
  Entretenimientos GEMINIS & GENATRON (C) 1991-92

  2x Z80.
  1x AY-3-8912.

  Xtal: 1x 16 MHz. (CPU)
        1x 20 MHz. (Video)

  1 or 2x 8 DIP switches banks.

***************************************************/

ROM_START(re800ea)
	ROM_REGION( 0x10000, "maincpu", 0 )	/* seems Genatron RE800, early revision */
	ROM_LOAD("ruleta1.128",	0x0000, 0x4000, CRC(e5931763) SHA1(35d47276275e691ae5f4f85bc54992381672df1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("re800snd.16",	0x0000, 0x0800, CRC(54d312fa) SHA1(6ed31f091362f7baa59afef91410fe946894e2ee) )
	ROM_RELOAD(                     0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promcoro.123",	0x0000, 0x0020, BAD_DUMP CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END

ROM_START(re800v1)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("re800v1.128",	0x0000, 0x4000, CRC(503647fb) SHA1(ccecb18058a672d955c5f94b0c049e6dd64d12e3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("re800snd.16",	0x0000, 0x0800, CRC(54d312fa) SHA1(6ed31f091362f7baa59afef91410fe946894e2ee) )
	ROM_RELOAD(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promcoro.123",	0x0000, 0x0020, BAD_DUMP CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END

ROM_START(re800v3)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("re800v3.128",	0x0000, 0x4000, CRC(d08a2a1a) SHA1(d2382e7545da808fb7e5a639eda90b759275983b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("re800snd.16",	0x0000, 0x0800, CRC(54d312fa) SHA1(6ed31f091362f7baa59afef91410fe946894e2ee) )
	ROM_RELOAD(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promcoro.123",	0x0000, 0x0020, BAD_DUMP CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END


/******************************************
*              Game Drivers               *
******************************************/

/*     YEAR  NAME      PARENT    MACHINE   INPUT     INIT      ROT      COMPANY                     FULLNAME                              FLAGS                                             LAYOUT      */
GAME(  1981, winner81, 0,        winner81, winner,   0,        ROT0,   "Corona Co.,LTD.",          "Winners Circle (81)",                 GAME_NOT_WORKING )
GAME(  1982, winner82, 0,        winner82, winner82, 0,        ROT0,   "Corona Co.,LTD.",          "Winners Circle (82)",                 GAME_NOT_WORKING )
GAMEL( 1991, re800ea,  0,        re800,    re800,    0,        ROT90,  "Entretenimientos GEMINIS", "Ruleta RE-800 (earlier, no attract)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_COLORS,  layout_re800 )
GAMEL( 1991, re800v1,  0,        re800,    re800,    0,        ROT90,  "Entretenimientos GEMINIS", "Ruleta RE-800 (v1.0)",                GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_COLORS,  layout_re800 )
GAMEL( 1991, re800v3,  0,        re800,    re800v3,  0,        ROT90,  "Entretenimientos GEMINIS", "Ruleta RE-800 (v3.0)",                GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_COLORS,  layout_re800 )
