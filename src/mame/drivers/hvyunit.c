/***************************************************************************************

 driver based on djboy.c / airbustr.c

 PROTECTION SIMULATION IS VERY PRELIMINARY AND SHOULD BE TREATED AS 100% GUESSWORK



Heavy Unit
Kaneko/Taito 1988

This game runs on Kaneko hardware. The game is similar to R-Type.

PCB Layout
----------
M6100391A
M6100392A  880210204
KNA-001
|----------------------------------------------------|
|                                                    |
|           6116                          6116       |
|      15Mhz                              6116       |
|                                 PAL                |
|           B73_09.2P             B73_11.5P          |
|                                                    |
|                                                    |
|                                 Z80-1   DSW1 DSW2 J|
|                                                   A|
|     16MHz                                         M|
|                                                   M|
|       12MHz                       6264  MERMAID   A|
| B73_05.1H                                          |
| B73_04.1F B73_08.2F  6116              Z80-2       |
| B73_03.1D                       Z80-3  B73_12.7E   |
| B73_02.1C B73_07.2C  PANDORA    B73_10.5C  6116    |
| B73_01.1B B73_06.2B 4164 4164   6264 PAL  YM3014   |
|                     4164 4164   PAL       YM2203   |
|----------------------------------------------------|

Notes:
      Z80-1 clock  : 6.000MHz
      Z80-2 clock  : 6.000MHz
      Z80-3 clock  : 6.000MHz
      YM2203 clock : 3.000MHz
      VSync        : 58Hz
      HSync        : 15.59kHz
               \-\ : KANEKO 1988. DIP40 chip, probably 8751 MCU (clock pins match)
      MERMAID    | : pin 18,19 = 6.000MHz (main clock)
                 | : pin 30 = 1.000MHz (prog/ale)
               /-/ : pin 22 = 111.48Hz (port 2 bit 1)

      PANDORA      : KANEKO PX79480FP-3 PANDORA-CHIP (C) KANEKO 1988


***************************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "video/kan_pand.h"

static UINT8 *videoram;
static UINT8 *colorram;
static tilemap_t *bg_tilemap;
static UINT16 hu_scrollx, hu_scrolly;
static UINT16 port0_data;

static int addr;
static UINT8 mcu_data;
static UINT8 mcu_status;
static UINT8 test_mcu;
static UINT8 mcu_ram_mux[0x100];
static struct
{
	int attract_timer;
	UINT8 program_flow;
	UINT8 access_ram_r,access_ram_w;
	UINT8 internal_ram[0x80];
	UINT8 internal_ram_index;
	UINT8 coin_counter;
}mcu_ram;

static MACHINE_RESET( mermaid )
{
	addr = 0xff;

	/* ticks for the attract mode. */
	mcu_ram.attract_timer = 0;
	/*
    helper for the program flow.
    0 = title screen
    1 = demo mode
    2 = ranking
    3 = Push 1p button
    4 = Push 1p AND/OR 2p button
    5 = player 1 plays
    6 = player 2 plays
    7 = service mode
    8 = game over screen
    ...
    */
	mcu_status = 0x8;
	mcu_ram.program_flow = 0;
	if(input_port_read(machine, "DSW1") & 4) //service mode
		mcu_ram.program_flow = 7;
	mcu_ram.coin_counter = 0;
}

static WRITE8_HANDLER( mermaid_data_w )
{
//  printf("%02x\n",data);
	if(mcu_ram.access_ram_w)
	{
		mcu_ram.internal_ram[(mcu_ram.internal_ram_index++)&0x7f] = data;
		mcu_ram.access_ram_w = 0;
	}
	else
	{
		mcu_data = data;
		mcu_ram_mux[data] = 0;
		if(data == 0)
		{
			/*next data will be an internal protection RAM write*/
			mcu_ram.access_ram_w = 1;
		}
	}
}

static READ8_HANDLER( mermaid_data_r )
{
	static UINT8 res;

	if(input_code_pressed_once(space->machine, KEYCODE_Z))
		test_mcu++;

	if(input_code_pressed_once(space->machine, KEYCODE_X))
		test_mcu--;

	if(mcu_ram.access_ram_r)
	{
		mcu_ram.access_ram_r = 0;
		return mcu_ram.internal_ram[(mcu_ram.internal_ram_index++)&0x7f];
	}

//  popmessage("%02x",test_mcu);

	switch(mcu_data)
	{
		/*
        pc=55f1
        store internal mcu ram values, not yet handled.
        */
		case 0:
		{
			/*next data will be an internal protection RAM read*/
			mcu_ram.access_ram_r = 1;
			return 0;
		}
		/*
        (PC=4f20) 01 $e003 = val
        (PC=4f2b) 01 $e004 = val
        (PC=4f5c) 01 val > 0xa and e06f = val coin counter (5)
            0 = title screen
            4 = "push 1p button"
            5 = "push 2p button"
            6 = copyright Kaneko msg (what is for?)
            7 = ranking
            8 = (trigger) attract mode
            b = (trigger?) player 1 plays
            c = (trigger?) player 2 plays
            0x10 = game over screen
            0x80 = coin error! msg
        (PC=4f82) 01 val<<=1 and check if < 0 program flow (6)
        (PC=4ef3) 01 $e06c = val, (complement it and AND $3)
        (PC=4f13) 01 $e06d = $e06e = val
        (PC=4f13) 01
        (PC=14b3) 01
        (PC=14ca) 81
        */
		case 1:
		{
			switch(mcu_ram_mux[1])
			{
				case 0:
					res = input_port_read(space->machine, "IN0");
					/*TODO: state of the button. */
					if(~res & 1 && (mcu_ram.program_flow == 3 || mcu_ram.program_flow == 4))
					{
						mcu_ram.coin_counter--;
						mcu_ram.program_flow = 5;
					}
					if(~res & 4 || ~res & 8)
					{
						mcu_ram.coin_counter++;
						mcu_ram.program_flow = (mcu_ram.coin_counter > 1) ? 4 : 3;
					}
					break;
				case 1: res = input_port_read(space->machine, "IN1"); break;
				case 2: res = input_port_read(space->machine, "IN2"); break;
				case 3: res = input_port_read(space->machine, "DSW1"); break;
				case 4: res = input_port_read(space->machine, "DSW2"); break;
				case 5:
					res = mcu_ram.coin_counter;
					break;
				case 6:
				/* WRONG! just as a test. */
				res = 0;
				if(mcu_ram.program_flow == 0)
				{
					mcu_ram.attract_timer++;
					//popmessage("%d",mcu_ram.attract_timer);
					if(mcu_ram.attract_timer > 600 && mcu_ram.program_flow == 0) { res = 8; mcu_ram.attract_timer = 0; mcu_ram.program_flow = 1; }
				}
				if(mcu_ram.program_flow == 1) //demo mode
				{
					mcu_ram.attract_timer++;
					popmessage("%d",mcu_ram.attract_timer);
					if(mcu_ram.attract_timer > 200 && mcu_ram.program_flow == 1) { res = 0;/*input_port_read(space->machine, "TEST");*/ mcu_ram.attract_timer = 0; mcu_ram.program_flow = 0; }
				}
				if(mcu_ram.program_flow == 3)
					res = 4;
				if(mcu_ram.program_flow == 4)
					res = 5;
				if(res == 0xb && mcu_ram.program_flow == 5)
					res = 0;
				else if(mcu_ram.program_flow == 5)
					res = 0xb;
				break;
			}
			//printf("(PC=%04x) %02x %02x\n",cpu_get_pc(space->cpu),mcu_data,res);

			mcu_ram_mux[1]++;
			if(mcu_ram_mux[1] > 6) { mcu_ram_mux[1] = 0; }
			return res;
		}
		/*
        (PC=4fbe) 03 ? (Is it read?)
        */
		case 3:  return mame_rand(space->machine);
		/*
        (PC=4e4d) 06 complement and put it to e06a
        (PC=4e59) 06 $e019 = val
        (PC=4f5c) 06 $e06f = val if NOT > 09
        (PC=4f82) 06 val<<=1 and check if < 0
        */
//      case 6: return 0;
		/*
        pc=5621 put the value to e003 (and 8)
        pc=562f put the value to e004 (and 4)
        */
		/* read back dsw. */
		case 0xff: return 0;
	}
	if(cpu_get_pc(space->cpu) != 0x4ee1 && cpu_get_pc(space->cpu) != 0x4e3b &&
	   cpu_get_pc(space->cpu) != 0x14ca && cpu_get_pc(space->cpu) != 0x14b3 &&
	   cpu_get_pc(space->cpu) != 0x550b && cpu_get_pc(space->cpu) != 0x551e &&
	   cpu_get_pc(space->cpu) != 0x5590)
		printf("(PC=%04x) %02x\n",cpu_get_pc(space->cpu),mcu_data);

	return 0;
}


/*
---- x--- MCU status
---- -x-- ?
*/
static READ8_HANDLER( mermaid_status_r )
{
//  printf("R St\n");

	mcu_status^=0xc;

	return mcu_status | 0x10;
//  return mame_rand(space->machine);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x0f) << 8);
	int color = (attr >> 4);

	SET_TILE_INFO(1, code, color, 0);
}

static VIDEO_START(hvyunit)
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
}

#define SX_POS	152
#define SY_POS	258

static VIDEO_UPDATE(hvyunit)
{
	running_device *pandora = devtag_get_device(screen->machine, "pandora");
	tilemap_set_scrollx( bg_tilemap,0, ((port0_data&0x40)<<2)+ hu_scrollx + SX_POS); //TODO
	tilemap_set_scrolly( bg_tilemap,0, ((port0_data&0x80)<<1)+ hu_scrolly + SY_POS); // TODO

//  popmessage("%02x %02x",hu_scrollx[0],hu_scrolly[0]);

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	pandora_update(pandora, bitmap, cliprect);
	return 0;
}

static VIDEO_EOF(hvyunit)
{
	running_device *pandora = devtag_get_device(machine, "pandora");
	pandora_eof(pandora);
}

static WRITE8_HANDLER( trigger_nmi_on_sound_cpu2 )
{
	soundlatch_w(space, 0, data);
	cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( trigger_nmi_on_slave_cpu)
{
	cputag_set_input_line(space->machine, "slave", INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( master_bankswitch_w )
{
	unsigned char *ROM = memory_region(space->machine, "master");
	int bank=data&7;

	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine, "bank1",ROM);
}


static WRITE8_HANDLER( hu_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( hu_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static ADDRESS_MAP_START( master_memory, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_DEVREADWRITE("pandora", pandora_spriteram_r, pandora_spriteram_w)
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(master_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(master_bankswitch_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(master_bankswitch_w) // correct?
	AM_RANGE(0x02, 0x02) AM_WRITE(trigger_nmi_on_slave_cpu)
ADDRESS_MAP_END

static WRITE8_HANDLER( slave_bankswitch_w )
{
	unsigned char *ROM = memory_region(space->machine, "slave");
	int bank=(data&0x03);
	port0_data=data;
	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine, "bank2",ROM);
}

static WRITE8_HANDLER( hu_scrollx_w)
{
	hu_scrollx=data;
}

static WRITE8_HANDLER( hu_scrolly_w)
{
	hu_scrolly=data;
}

static ADDRESS_MAP_START( slave_memory, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_WRITE(hu_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_WRITE(hu_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xd000, 0xd1ff) AM_RAM_WRITE(paletteram_xxxxRRRRGGGGBBBB_split2_w) AM_BASE_GENERIC(paletteram2)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM_WRITE(paletteram_xxxxRRRRGGGGBBBB_split1_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(slave_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(slave_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(trigger_nmi_on_sound_cpu2)
	AM_RANGE(0x04, 0x04) AM_READWRITE(mermaid_data_r,mermaid_data_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(hu_scrolly_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(hu_scrollx_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(mermaid_status_r)
	AM_RANGE(0x0e, 0x0e) AM_RAM //should be coin info, see djoy.c

//  AM_RANGE(0x22, 0x22) AM_READ(hu_scrolly_hi_reset) //22/a2 taken from ram $f065
//  AM_RANGE(0xa2, 0xa2) AM_READ(hu_scrolly_hi_set)

ADDRESS_MAP_END

static WRITE8_HANDLER( sound_bankswitch_w )
{
	unsigned char *ROM = memory_region(space->machine, "soundcpu");
	int bank=data&0x3;

	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine, "bank3",ROM);
}

static ADDRESS_MAP_START( sound_memory, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank3")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( hvyunit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT ) // copied from DJ Boy, might not
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/*copied from DJ Boy, WRONG

HEAVY UNIT
Manufacturer: Taito
Year: 1988 - Genre: Shooter
Orientation: Horizontal
Conversion Class: JAMMA
DIP Switch Settings:
SW#1
--------------------------------------------------------------------
DESCRIPTION                           1   2   3   4   5   6   7   8
--------------------------------------------------------------------
CABINET TYPE       TABLE             OFF
                   UPRIGHT           ON
--------------------------------------------------------------------
VIDEO SCREEN       NORMAL                OFF
                   FLIP                  ON
--------------------------------------------------------------------
TEST MODE          NORMAL                    OFF
                   TEST                      ON
--------------------------------------------------------------------
COIN/CREDIT
COIN #1            1C/1P                             OFF OFF
                   1C/2P                             ON  OFF
                   2C/1P                             OFF ON
                   2C/3P                             ON  ON

COIN #2            1C/1P                                     OFF OFF
                   1C/2P                                     ON  OFF
                   2C/1P                                     OFF ON
                   2C/3P                                     ON  ON
--------------------------------------------------------------------
NOT USED           SW#4 ALWAYS OFF
--------------------------------------------------------------------


SW#2
--------------------------------------------------------------------
DESCRIPTION                           1   2   3   4   5   6   7   8
--------------------------------------------------------------------
DIFFICULTY         NORMAL            OFF OFF
                   EASY              ON  OFF
                   HARD              OFF ON
                   HARDEST           ON  ON
--------------------------------------------------------------------
CONTINUE           YES                       OFF
                   NO                        ON
--------------------------------------------------------------------
BONUS              NO                            OFF
                   YES                           ON
--------------------------------------------------------------------
# LIVES            3                                 OFF OFF
                   4                                 ON  OFF
                   5                                 OFF ON
                   7                                 ON  ON
--------------------------------------------------------------------
DEMO SOUND         YES                                       OFF
                   NO                                        ON
--------------------------------------------------------------------
NOT USED           SW#8 ALWAYS OFF
--------------------------------------------------------------------

*/
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, "TEST" )
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

/****************** Graphics ************************/

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{
		0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,
		8*32+0*4,8*32+1*4,8*32+2*4,8*32+3*4,8*32+4*4,8*32+5*4,8*32+6*4,8*32+7*4
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,
		16*32+0*32,16*32+1*32,16*32+2*32,16*32+3*32,16*32+4*32,16*32+5*32,16*32+6*32,16*32+7*32
	},
	4*8*32
};

static GFXDECODE_START( hvyunit )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0x100, 16 ) /* sprite bank */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0x000, 16 ) /* background tiles */
GFXDECODE_END

/****************** Machine ************************/


static INTERRUPT_GEN( hvyunit_interrupt )
{
	addr ^= 0x02;
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, addr);
}

static const kaneko_pandora_interface hvyunit_pandora_config =
{
	"screen",	/* screen tag */
	0,	/* gfx_region */
	0, 0	/* x_offs, y_offs */
};


static MACHINE_DRIVER_START( hvyunit )

	MDRV_CPU_ADD("master", Z80,6000000)
	MDRV_CPU_PROGRAM_MAP(master_memory)
	MDRV_CPU_IO_MAP(master_io)
	MDRV_CPU_VBLANK_INT_HACK(hvyunit_interrupt,2)

	MDRV_CPU_ADD("slave", Z80,6000000)
	MDRV_CPU_PROGRAM_MAP(slave_memory)
	MDRV_CPU_IO_MAP(slave_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, 6000000)
	MDRV_CPU_PROGRAM_MAP(sound_memory)
	MDRV_CPU_IO_MAP(sound_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_QUANTUM_TIME(HZ(6000))
	MDRV_MACHINE_RESET(mermaid)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)

	MDRV_GFXDECODE(hvyunit)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_KANEKO_PANDORA_ADD("pandora", hvyunit_pandora_config)

	MDRV_VIDEO_START(hvyunit)
	MDRV_VIDEO_UPDATE(hvyunit)
	MDRV_VIDEO_EOF(hvyunit)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 3000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END



ROM_START( hvyunit )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_10.5c",  0x00000, 0x20000, CRC(ca52210f) SHA1(346951962aa5bbad641117dbd66f035dddc7c0bf) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_11.5p",  0x00000, 0x10000, CRC(cb451695) SHA1(116fd59f96a54c22fae65eea9ee5e58cb9ce5074) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "i8751_mermaid",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) // loading only verified for roms marked 'ok'
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) ) // attract ok
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x120000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) ) // attract ok
	ROM_LOAD( "b73_01.1b",  0x140000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) ) // attract ok
	ROM_LOAD( "b73_02.1c",  0x160000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) ) // attract ok
	ROM_LOAD( "b73_03.1d",  0x180000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	ROM_LOAD( "b73_04.1f",  0x1a0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	ROM_LOAD( "b73_05.1h",  0x1c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitj )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_30.5c",  0x00000, 0x20000, CRC(600af545) SHA1(c52b9be2bae28848ad0818c296f000a1bda4fa4f) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_14.5p",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "i8751_mermaid",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) // loading only verified for roms marked 'ok'
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) ) // attract ok
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x120000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) ) // attract ok
	ROM_LOAD( "b73_01.1b",  0x140000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) ) // attract ok
	ROM_LOAD( "b73_02.1c",  0x160000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) ) // attract ok
	ROM_LOAD( "b73_03.1d",  0x180000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	ROM_LOAD( "b73_04.1f",  0x1a0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	ROM_LOAD( "b73_05.1h",  0x1c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunito )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_13.5c",  0x00000, 0x20000, CRC(e2874601) SHA1(7f7f3287113b8622eb365d04135d2d9c35d70554) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_14.5p",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "i8751_mermaid",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) // loading only verified for roms marked 'ok'
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) ) // attract ok
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x120000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) ) // attract ok
	ROM_LOAD( "b73_01.1b",  0x140000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) ) // attract ok
	ROM_LOAD( "b73_02.1c",  0x160000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) ) // attract ok
	ROM_LOAD( "b73_03.1d",  0x180000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	ROM_LOAD( "b73_04.1f",  0x1a0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	ROM_LOAD( "b73_05.1h",  0x1c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

GAME( 1988, hvyunit, 0,        hvyunit, hvyunit, 0, ROT0, "Kaneko / Taito", "Heavy Unit (World)" ,GAME_NOT_WORKING )
GAME( 1988, hvyunitj, hvyunit, hvyunit, hvyunit, 0, ROT0, "Kaneko / Taito", "Heavy Unit (Japan, Newer)" ,GAME_NOT_WORKING )
GAME( 1988, hvyunito, hvyunit, hvyunit, hvyunit, 0, ROT0, "Kaneko / Taito", "Heavy Unit (Japan, Older)" ,GAME_NOT_WORKING )
