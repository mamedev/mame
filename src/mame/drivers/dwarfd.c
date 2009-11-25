/*
  Dwarfs Den by Electro-Sport
  http://www.arcadeflyers.net/?page=flyerdb&subpage=thumbs&id=3993

  driver by
   David Haywood
   Tomasz Slanina

 TODO:

 - finish 8275 CRT emulation
 - fix gfx glitches
 - correct colors
 - DIPs
 - layout(lamps)
 - NVRAM


DD hardware is weird and complicated. There's no dedicated DMA controller
for CRT.  When serial output pin of cpu is in high state, all reads from
work RAM are redirected to CRT:

005B: 3E C0      mvi  a,$c0
005D: 30         sim
005E: E1         pop  h ; 40 times "pop h" = transfer 80 bytes (line) from ram to crt fifo
...
0087: 0F         rrc
0088: 30         sim

$4c00-$4fff part of work RAM is adressed in different way than other blocks. No idea why
(this part of the pcb is a bit damaged).
Gfx decode is strange - may be wrong.
Gfx rom bank (6d/6b or 6a/6c pair) is switched by timer circuit - few 74161 counters connected to xtal.
Current implementations is just a guess, and doesn't work on test screen.


                                               ______________
  _____________________________________________||||||||||||||_______________________________________
  |         1         2         3         4         5         6         7         8         9      |
  |                                                                                      ________  |
  ||| L                                                   SN7445N74  74LS224N  7404-PC   |9L    |  |
D |||                                                                                    |______|  |
  |||                                                                   _____________    ________  |
  ||| K                         BATTERY         16-1-471  7400-PC       | M5L8085AP |    |9K    |  |
  |                                                                     |___________|    |______|  |
  |                                                                                      ________  |
  ||| J           74LS273NA                     --------  --------             74LS373N  |9J    |  |
C |||                                                                                    |______|  |
  |||                                                                                    ________  |
  ||| H           74LS273NA  SN7442AN  74107N   74161N    SN7432N    M3-7602-5 MDP1603   |9H    |  |
  |                                                                                      |______|  |
  |                                                                                                |
  ||| F           74LS273NA  7408N     7486N    OSC       7404                 MM2114N   MM2114N   |
P |||                                                                                              |
  |||                                  _____________                                               |
  ||| E                      SN7442AN  |iP8275     |      7400       74LS244N  MM2114N   MM2114N   |
  |                                    |___________|                                               |
  |                                    _____________      ________                                 |
  ||| D 16-1-471  MDP1603    7414      |AY-3-8910  |      |6D    |   74LS245   MM2114N   MM2114N   |
B |||                                  |___________|      |______|                                 |
  |||                                                     ________   _______                       |
  ||| C 16-1-471  MDP1603    7414      7414     7400      |6C    |   |7C   |   M3-6514-9 M3-6514-9 |
  |                                                       |______|                                 |
  |                                                       ________   __________________            |
  ||| B SN74175N             74174N    74LS374N 74LS374N  |6B    |   |                |  6-1-471   |
A |||                                                     |______|   |                |            |
  |||                        _______                      ________   |    ______      |            |
  ||| A DN74505N             |3A   |   74153N   74153N    |6A    |   |    LM324N      |  DIP-SW    |
  |                                                       |______|   |                |            |
  |                                                                  |________________|            |
  |________________________________________________________________________________________________|

  OSC = 10.595 MHz
  D,C,A = 20-pin connectors
  B = 26-pin connector
  P = 12-pin connector (seems power)
  Edge connector (top) is not JAMMA
  3A (63S080N) dumped as 82S123
  7C = non populated

*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"

typedef struct _dwarfd_state dwarfd_state;
struct _dwarfd_state
{
	/* memory pointers */
	UINT8 *  dw_ram;
	UINT8 *  videobuf;

	/* video-related */
	int bank;
	int line;
	int idx;
	int crt_access;

	/* i8275 */
	int i8275Command;
	int i8275HorizontalCharactersRow;
	int i8275CommandSeqCnt;
	int i8275SpacedRows;
	int i8275VerticalRows;
	int i8275VerticalRetraceRows;
	int i8275Underline;
	int i8275Lines;
	int i8275LineCounterMode;
	int i8275FieldAttributeMode;
	int i8275CursorFormat;
	int i8275HorizontalRetrace;
};


//should be taken from crt params
static const int maxy = 25;
static const int maxx = 80;


#define TOPLINE 7
#define BOTTOMLINE 18

#define CHARACTERS_UNDEFINED -1

enum
{
	I8275_COMMAND_RESET=0,
	I8275_COMMAND_START,
	I8275_COMMAND_STOP,
	I8275_COMMAND_READ_LIGHT_PEN,
	I8275_COMMAND_LOAD_CURSOR,
	I8275_COMMAND_EI,
	I8275_COMMAND_DI,
	I8275_COMMAND_PRESET,

	NUM_I8275_COMMANDS
};

enum
{
	I8275_COMMAND_RESET_LENGTH = 4,
	I8275_COMMAND_START_LENGTH = 0,
	I8275_COMMAND_STOP_LENGTH = 0,
	I8275_COMMAND_EI_LENGTH = 0,
	I8275_COMMAND_DI_LENGTH = 0,
	I8275_COMMAND_READ_LIGHT_PEN_LENGTH = 2,
	I8275_COMMAND_LOAD_CURSOR_LENGTH = 2,
	I8275_COMMAND_PRESET_LENGTH = 0
};


static WRITE8_HANDLER (i8275_preg_w) //param reg
{
	dwarfd_state *state = (dwarfd_state *)space->machine->driver_data;

	switch (state->i8275Command)
	{
		case I8275_COMMAND_RESET:
		{
			switch (state->i8275CommandSeqCnt)
			{
				case 4:
				{
					//screen byte comp byte 1
					state->i8275SpacedRows = data >> 7;
					state->i8275HorizontalCharactersRow = (data & 0x7f) + 1;
					if (state->i8275HorizontalCharactersRow > 80)
					{
						logerror("i8275 Undefined num of characters/Row! = %d\n", state->i8275HorizontalCharactersRow);
						state->i8275HorizontalCharactersRow = CHARACTERS_UNDEFINED;
					}
					else
					{
						logerror("i8275 %d characters/row\n", state->i8275HorizontalCharactersRow);
					}
					if (state->i8275SpacedRows & 1)
					{
						logerror("i8275 spaced rows\n");
					}
					else
					{
						logerror("i8275 normal rows\n");
					}
					state->i8275CommandSeqCnt--;
				}
				break;

				case 3:
				{
					//screen byte comp byte 2
					state->i8275VerticalRows = (data & 0x3f) + 1;
					state->i8275VerticalRetraceRows = (data >> 6) + 1;

					logerror("i8275 %d rows\n", state->i8275VerticalRows);
					logerror("i8275 %d vertical retrace rows\n", state->i8275VerticalRetraceRows);

					state->i8275CommandSeqCnt--;
				}
				break;

				case 2:
				{
					//screen byte comp byte 3
					state->i8275Underline = (data >> 4) + 1;
					state->i8275Lines = (data & 0xf) + 1;
					logerror("i8275 underline placement: %d\n", state->i8275Underline);
					logerror("i8275 %d lines/row\n", state->i8275Lines);

					state->i8275CommandSeqCnt--;
				}
				break;

				case 1:
				{
					//screen byte comp byte 4
					state->i8275LineCounterMode = data >> 7;
					state->i8275FieldAttributeMode = (data >> 6) & 1;
					state->i8275CursorFormat = (data >> 4) & 3;
					state->i8275HorizontalRetrace = ((data & 0xf) + 1) << 1;
					logerror("i8275 line counter mode: %d\n", state->i8275LineCounterMode);
					if (state->i8275FieldAttributeMode)
					{
						logerror("i8275 field attribute mode non-transparent\n");
					}
					else
					{
						logerror("i8275 field attribute mode transparent\n");
					}

					switch (state->i8275CursorFormat)
					{
						case 0:	{logerror("i8275 cursor format - blinking reverse video block\n");}	break;
						case 1:	{logerror("i8275 cursor format - blinking underline\n");}break;
						case 2:	{logerror("i8275 cursor format - nonblinking reverse video block\n");}break;
						case 3:	{logerror("i8275 cursor format - nonblinking underline\n");}break;
					}

					logerror("i8275 %d chars for horizontal retrace\n",state->i8275HorizontalRetrace );
					state->i8275CommandSeqCnt--;
				}
				break;

				default:
				{
					logerror("i8275 illegal\n");
				}

			}
		}
		break;

		case I8275_COMMAND_START:
		{

		}
		break;

		case I8275_COMMAND_STOP:
		{

		}
		break;

	}

}

static READ8_HANDLER (i8275_preg_r) //param reg
{
	return 0;
}

static WRITE8_HANDLER (i8275_creg_w) //comand reg
{
	dwarfd_state *state = (dwarfd_state *)space->machine->driver_data;

	switch (data>>5)
	{
		case 0:
		{
			/* reset */
			state->i8275Command = I8275_COMMAND_RESET;
			state->i8275CommandSeqCnt = I8275_COMMAND_RESET_LENGTH;
		}
		break;

		case 5:
		{
			/* enable interrupt */
			state->i8275Command = I8275_COMMAND_EI;
			state->i8275CommandSeqCnt = I8275_COMMAND_EI_LENGTH;
		}
		break;

		case 6:
		{
			/* disable interrupt */
			state->i8275Command = I8275_COMMAND_DI;
			state->i8275CommandSeqCnt = I8275_COMMAND_DI_LENGTH;
		}
		break;

		case 7:
		{
			/* preset counters */
			state->i8275CommandSeqCnt = I8275_COMMAND_PRESET_LENGTH;

		}
		break;
	}
}

static READ8_HANDLER (i8275_sreg_r) //status
{
	return 0;
}

static READ8_HANDLER(dwarfd_ram_r)
{
	dwarfd_state *state = (dwarfd_state *)space->machine->driver_data;

	if (state->crt_access == 0)
	{
		return state->dw_ram[offset];
	}
	else
	{
		state->videobuf[state->line * 256 + state->idx] = state->dw_ram[offset];
		state->idx++;
		return state->dw_ram[offset];
	}
}

static WRITE8_HANDLER(dwarfd_ram_w)
{
	dwarfd_state *state = (dwarfd_state *)space->machine->driver_data;
	state->dw_ram[offset] = data;
}

static WRITE8_HANDLER(output1_w)
{
/*
 bits:
     0 - pp lamp
     1 - start lamp
     2 - ?
     3 - ?
     4 - unzap lamp
     5 - z1 lamp
     6 - z2 lamp
     7 - z3 lamp
*/
}

static WRITE8_HANDLER(output2_w)
{
/*
 bits:
     0 - z4
     1 - z5
     2 - coin counter?
     3 - ?
     4 - ?
     5 - ?
     6 - ?
     7 - ?
*/
}




static ADDRESS_MAP_START( mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_READWRITE(dwarfd_ram_r, dwarfd_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_data_address_w)

	AM_RANGE(0x20, 0x20) AM_READWRITE(i8275_preg_r, i8275_preg_w)
	AM_RANGE(0x21, 0x21) AM_READWRITE(i8275_sreg_r, i8275_creg_w)
	AM_RANGE(0x40, 0x40) AM_WRITENOP // unknown
	AM_RANGE(0x60, 0x60) AM_WRITE(output1_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(output2_w)
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("DSW1")
	AM_RANGE(0xc1, 0xc1) AM_READ_PORT("DSW2")
ADDRESS_MAP_END


static INPUT_PORTS_START( dwarfd )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Zap 1") //z1 zap 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Zap 2") //z2 zap 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Zap 3") //z3 zap 3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_NAME("Zap 4") //z4 zap 4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) PORT_NAME("Zap 5") //z5 zap 5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Bet x1") //x1 bet x1
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP ) PORT_NAME("Bet x2") //x2 bet x2

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("PP ?") PORT_CODE(KEYCODE_A) //pp
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Replace") PORT_CODE(KEYCODE_F) //rp replace
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) //tk take
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Unzap") //uz unzap
INPUT_PORTS_END


static VIDEO_START(dwarfd)
{
}

static void drawCrt( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	dwarfd_state *state = (dwarfd_state *)machine->driver_data;
	int x, y;
	for (y = 0; y < maxy; y++)
	{
		int count = y * 256;
		int bank2 = 4;

		if (y < TOPLINE || y > BOTTOMLINE)
		{
			bank2 = 0;
		}
		for (x = 0; x < maxx; x++)
		{
			int tile = 0;

			int b = 0; //end marker
			while (b == 0)
			{
				if (count < 0x8000)
					tile = state->videobuf[count++];
				else
						return;

				if (tile & 0x80)
				{
					if ((tile & 0xfc) == 0xf0)
					{
						switch (tile & 3)
						{
							case 0:
							case 1: break;

							case 2:
							case 3: return;
						}
					}
					if ((tile & 0xc0) == 0x80)
					{
						state->bank = (tile >> 2) & 3;
					}
					if ((tile & 0xc0) == 0xc0)
					{
						b = 1;
						tile = mame_rand(machine) & 0x7f;//(tile >> 2) & 0xf;
					}
				}
				else
					b = 1;
			}
			drawgfx_transpen(bitmap, cliprect, machine->gfx[0],
				tile + (state->bank + bank2) * 128,
				0,
				0, 0,
				x*8,y*8,0);
		}
	}
}


static VIDEO_UPDATE( dwarfd )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	drawCrt(screen->machine, bitmap, cliprect);
	return 0;
}

static WRITE_LINE_DEVICE_HANDLER( dwarfd_sod_callback )
{
	dwarfd_state *driver_state = (dwarfd_state *)device->machine->driver_data;
	driver_state->crt_access = state;
}


static I8085_CONFIG( dwarfd_i8085_config )
{
	DEVCB_NULL,						/* STATUS changed callback */
	DEVCB_NULL,						/* INTE changed callback */
	DEVCB_NULL,						/* SID changed callback (8085A only) */
	DEVCB_LINE(dwarfd_sod_callback)	/* SOD changed callback (8085A only) */
};


#define NUM_LINES 25
static INTERRUPT_GEN( dwarfd_interrupt )
{
	dwarfd_state *state = (dwarfd_state *)device->machine->driver_data;

	if (cpu_getiloops(device) < NUM_LINES)
	{
		cpu_set_input_line(device, I8085_RST65_LINE, HOLD_LINE); // 34 - every 8th line
		state->line = cpu_getiloops(device);
		state->idx = 0;
	}
	else
	{
		if (cpu_getiloops(device) == NUM_LINES)
		{
			cpu_set_input_line(device, I8085_RST55_LINE, HOLD_LINE);//2c - generated by  crt - end of frame
		}
	}
}

#if 0
static const gfx_layout tiles8x8_layout =
{
	4,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3},
	{8,0,24,16 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	8*32
};
#endif

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3},
	{8,12,0,4,24,28,16,20 },
//  {12,8,4,0,28,24,20,16 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	8*32
};


static const gfx_layout tiles8x8_layout0 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{8,0,24,16 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	8*32
};

static const gfx_layout tiles8x8_layout1 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 1 },
	{8,0,24,16 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	8*32
};

static const gfx_layout tiles8x8_layout2 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 2 },
	{8,0,24,16 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	8*32
};

static const gfx_layout tiles8x8_layout3 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 3 },
	{8,0,24,16 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	8*32
};
/*
static const gfx_layout tiles8x8_layout =
{
    8,8,
    RGN_FRAC(1,1),
    2,
    { 1,1},
    {6,6,2,2,14,14,10,10 },
    //{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
    { 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
    8*16
};
*/

static GFXDECODE_START( dwarfd )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout0, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout1, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout2, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout3, 0, 16 )
GFXDECODE_END

static PALETTE_INIT(dwarfd)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		int r = mame_rand(machine)|0x80;
		int g = mame_rand(machine)|0x80;
		int b = mame_rand(machine)|0x80;
		if (i == 0) r = g = b = 0;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
	palette_set_color(machine, 8, MAKE_RGB(255, 255, 0));
	palette_set_color(machine, 12, MAKE_RGB(127, 127, 255));
	palette_set_color(machine, 4, MAKE_RGB(0, 255, 0));
	palette_set_color(machine, 6, MAKE_RGB(255, 0, 0));
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN2"),
	DEVCB_INPUT_PORT("IN1"),
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_START( dwarfd )
{
	dwarfd_state *state = (dwarfd_state *)machine->driver_data;

	state_save_register_global(machine, state->bank);
	state_save_register_global(machine, state->line);
	state_save_register_global(machine, state->idx);
	state_save_register_global(machine, state->crt_access);

	/* i8275 */
	state_save_register_global(machine, state->i8275Command);
	state_save_register_global(machine, state->i8275HorizontalCharactersRow);
	state_save_register_global(machine, state->i8275CommandSeqCnt);
	state_save_register_global(machine, state->i8275SpacedRows);
	state_save_register_global(machine, state->i8275VerticalRows);
	state_save_register_global(machine, state->i8275VerticalRetraceRows);
	state_save_register_global(machine, state->i8275Underline);
	state_save_register_global(machine, state->i8275Lines);
	state_save_register_global(machine, state->i8275LineCounterMode);
	state_save_register_global(machine, state->i8275FieldAttributeMode);
	state_save_register_global(machine, state->i8275CursorFormat);
	state_save_register_global(machine, state->i8275HorizontalRetrace);
}

static MACHINE_RESET( dwarfd )
{
	dwarfd_state *state = (dwarfd_state *)machine->driver_data;

	state->bank = 0;
	state->line = 0;
	state->idx = 0;
	state->crt_access = 0;
	state->i8275Command = 0;
	state->i8275HorizontalCharactersRow = 0;
	state->i8275CommandSeqCnt = 0;
	state->i8275SpacedRows = 0;
	state->i8275VerticalRows = 0;
	state->i8275VerticalRetraceRows = 0;
	state->i8275Underline = 0;
	state->i8275Lines = 0;
	state->i8275LineCounterMode = 0;
	state->i8275FieldAttributeMode = 0;
	state->i8275CursorFormat = 0;
	state->i8275HorizontalRetrace = 0;
}

static MACHINE_DRIVER_START( dwarfd )

	/* driver data */
	MDRV_DRIVER_DATA(dwarfd_state)

	/* basic machine hardware */
	/* FIXME: The 8085A had a max clock of 6MHz, internally divided by 2! */
	MDRV_CPU_ADD("maincpu", 8085A, 10595000/3*2)        /* ? MHz */
	MDRV_CPU_CONFIG(dwarfd_i8085_config)
	MDRV_CPU_PROGRAM_MAP(mem_map)
	MDRV_CPU_IO_MAP(io_map)

	MDRV_CPU_VBLANK_INT_HACK(dwarfd_interrupt,NUM_LINES+4) //16 +vblank + 1 unused

	MDRV_MACHINE_START(dwarfd)
	MDRV_MACHINE_RESET(dwarfd)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(272*2, 200)
	MDRV_SCREEN_VISIBLE_AREA(0, 272*2-1, 0, 200-1)

	MDRV_GFXDECODE(dwarfd)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(dwarfd)

	MDRV_VIDEO_START(dwarfd)
	MDRV_VIDEO_UPDATE(dwarfd)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, 1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

ROM_START( dwarfd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9l_pd_50-3196_m5l2732k.bin", 0x0000, 0x1000, CRC(34e942ae) SHA1(d4f0ee7f29e1c1a93b4b30b950023dbf60596100) )
	ROM_LOAD( "9k_pd_50-3193_hn462732g.bin",0x1000, 0x1000, CRC(78f0c260) SHA1(d6c3b8b3ef4ce99a811e291f1396a47106683df9) )
	ROM_LOAD( "9j_pd_50-3192_mbm2732.bin",  0x2000, 0x1000, CRC(9c66ee6e) SHA1(49c20fa276508b3c7b0134909295ae04ee46890f) )
	ROM_LOAD( "9h_pd_50-3375_2732.bin",     0x3000, 0x1000, CRC(daf5551d) SHA1(933e3453c9e74ca6695137c9f6b1abc1569ad019) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "6a_pd_50_1991_2732.bin"      ,0x0000, 0x1000, CRC(6da494bc) SHA1(0323eaa5f81e3b8561225ccdd4654c9a11f2167c) )
	ROM_LOAD16_BYTE( "6b_pd_50-1992_tms2732ajl.bin",0x2000, 0x1000, CRC(69208e1a) SHA1(8706f8f0d2dfeba5cebc71985ea46a67de13bc7d) )
	ROM_LOAD16_BYTE( "6c_pd_50-1993_tms2732ajl.bin",0x0001, 0x1000, CRC(cd8e5e54) SHA1(0961739d72d80e0ac00e6cbf9643bcebfe74830d) )
	ROM_LOAD16_BYTE( "6d_pd_50-1994_tms2732ajl.bin",0x2001, 0x1000, CRC(ef52b88c) SHA1(3405152da3194a71f6dac6492f275c746e781ee7) )

	ROM_REGION( 0x4000*2, "gfx2", 0 )
	ROM_FILL(0,  0x4000*2, 0)

	ROM_REGION( 0x40, "proms", 0 )
	/* ??? colors */
	ROM_LOAD( "3a_50-1381_63s080n.bin",0x00, 0x20, CRC(451d0a72) SHA1(9ff6e2c5bd2b57bd607cb33e60e7ed25bea164b3) )
	/* memory map */
	ROM_LOAD( "7h_7602.bin",0x20, 0x20, CRC(d5457333) SHA1(5872c868638c08faef7365d9c6e41dc3f070bd97) )
ROM_END

static DRIVER_INIT(dwarfd)
{
	dwarfd_state *state = (dwarfd_state *)machine->driver_data;
	int i;
	UINT8 *src, *dst;

	/* expand gfx roms */
	src = memory_region(machine, "gfx1");
	dst = memory_region(machine, "gfx2");

	for (i = 0; i < 0x4000; i++)
	{
		UINT8 dat;
		dat = (src[i] & 0xf0) >> 0;
		dst[i * 2] = dat;

		dat = (src[i] & 0x0f)<<4;
		dst[i * 2 + 1] = dat;
	}

	/* use low bit as 'interpolation' bit */
	src = memory_region(machine, "gfx2");
	for (i = 0; i < 0x8000; i++)
	{
		if (src[i] & 0x10)
		{
			src[i] = src[i] & 0xe0;
	//      src[i] |= ((src[(i + 1) & 0x7fff] & 0xe0) >> 4);

		}
		else
		{
			src[i] = src[i] & 0xe0;
			src[i] |= (src[i] >> 4);

		}
	//      src[i] = src[i] & 0xe0;
	}

	state->videobuf = auto_alloc_array(machine, UINT8, 0x8000);
	state->dw_ram = auto_alloc_array(machine, UINT8, 0x1000);

	state_save_register_global_pointer(machine, state->videobuf, 0x8000);
	state_save_register_global_pointer(machine, state->dw_ram, 0x1000);

	memset(state->videobuf, 0, 0x8000);
	memset(state->dw_ram, 0, 0x1000);

}

GAME( 1981, dwarfd, 0, dwarfd, dwarfd, dwarfd, ORIENTATION_FLIP_Y, "Electro-Sport", "Dwarfs Den", GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
