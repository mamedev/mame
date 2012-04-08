/***************************************************************************

Some Dynax/Nakanihon games using the third version of their blitter

Driver by Nicola Salmoria, Luca Elia

Hardware:
CPU: Z80 or 68000
Sound: (AY-3-8910) + YM2413 + MSM6295
Other: Real Time Clock (Oki MSM6242B or 72421B)

------------------------------------------------------------------------------------------------------------------
Year + Game              Board              CPU   Sound              Custom                                 Other
------------------------------------------------------------------------------------------------------------------
92 Monkey Mole Panic                        2xZ80 AY8910 YM2413 M6295 NL-001 1108(x2)   1427(x2)            8251
93 Animalandia Jr.                          2xZ80 AY8910 YM2413 M6295 NL-001 NL-003(x2) NL-004(x2)          8251
93 Quiz Channel Question N7311208L1-2       Z80          YM2413 M6295 NL-002 1108F0405  1427F0071
93 First Funky Fighter   N7403208L-2        2xZ80 YM2149 YM2413 M6295 NL-001 NL-002     NL-005
94 Mysterious World      D7107058L1-1       Z80   YM2149 YM2413 M6295 NL-002 1108F0405  1427F0071 4L02?
94 Mysterious Universe   D7107058L1-1       Z80   YM2149 YM2413 M6295 NL-002 1108F0405  1427F0071
94 Mysterious Orient     D7107058L1-1       Z80   YM2149 YM2413 M6295 NL-002 1108F0405  1427F0071
94 Quiz 365                                 68000 AY8910 YM2413 M6295
94 Rong Rong (J)         N8010178L1         Z80          YM2413 M6295 NL-002 1108F0405  1427F0071 4L02F2637
94 Hana Ginga            D8102048L1         Z80   YM2149 YM2413 M6295 NL-002 1108F0405  1427F0071 4L02?
94 Super Hana Paradise   N8010178L1+N73RSUB Z80          YM2413 M6295 NL-002 1108F0406  1427F0071 4L02F2637
95 Dai Chuuka Ken        D11107218L1        Z80   AY8910 YM2413 M6295 70C160F009
95 Hana Gokou            N83061581L1        Z80   AY8910 YM2413 M6295 NL-002 1108?      1427?     4L02?
95 Hana Gokou Bangaihen  N10805078L1        Z80   AY8910 YM2413 M6295 NL-002 1108?      1427?     4L02?
95 Nettoh Quiz Champion                     68000 AY8910 YM2413 M6295
95 Don Den Lover (J)     D1120901L8         68000 YMZ284 YM2413 M6295 NL-005
96 Don Den Lover (HK)    D11309208L1        68000 YMZ284 YM2413 M6295 NL-005
96 Panel&Variety Akamaru                    68000 YMZ284 YM2413 M6295 NL-005
96 Fantasic Love         NS5000101+?        Z80   YMZ284 YM2413 M6295 NL-005
96 Hana Kanzashi                            Z80          YM2413 M6295 70C160F011?
96 Mj Seiryu Densetsu    NM5020403          Z80   YMZ284 YM2413 M6295 70C160F011?
97 Hana Kagerou                             Z80          YM2413 M6295 70C160F011
98 Chuukanejyo           D11107218L1        Z80   AY8910 YM2413 M6295 70C160F009
98 Reach Ippatsu                            Z80          YM2413 M6295 70C160F011
99 Jong-Tei              NM532-9902         Z80          YM2413 M6295 4L10FXXXX?
02 Mj Daimyojin          TSM015-0111        Z80          YM2413 M6295 70C160F011
04 Mj Momotarou          TSM015-0111?       Z80          YM2413 M6295 70C160F011?
------------------------------------------------------------------------------------------------------------------

TODO:

- NVRAM, RTC

- verify whether clip_width/height is actually clip_x_end/y_end
  (this also applies to rectangles drawing, command 1c):
  the girl in hanakanz divided in 3 chunks (during the intro when bet is on)
  is ok with the latter setting; scene 2 of gal 1 check in hkagerou (press 1 in scene 1)
  is maybe clipped too much this way and hints at the former setting being correct.
  There is an #if to switch between the two modes in do_plot.

- ddenlovr: understand the extra commands for the blitter compressed data,
  used only by this game.

- ddenlovr: sometimes the colors of the girl in the presentation before the
  beginning of a stage are wrong, and they correct themselves when the board
  is drawn.

- the registers right after the palette bank selectors (e00048-e0004f in ddenlovr)
  are not understood. They are related to the layer enable register and to the
  unknown blitter register 05.
  ddenlovr has a function at 001798 to initialize these four registers. It uses
  a table with 7 possible different layouts:
  0f 0f 3f cf
  4f 0f af 1f
  0f 0f 6f 9f
  0f 0f 0f ff
  0f 0f 7f 8f
  0f 0f cf 3f
  0f 0f 8f 7f
  the table is copied to e00048-e0004f and is also used to create a 16-entry
  array used to feed blitter register 05. Every element of the array is the OR
  of the values in the table above corresponding to bits set in the layer enable
  register. Note that in the table above the top 4 bits are split among the four
  entries.

- The meaning of blitter commands 43 and 8c is not understood.

- quizchq: it locks up, some samples are played at the wrong pitch

- quiz365 protection

- ddenlovj, akamaru: the elapsed time text in the "game information" screen
  is all wrong (RTC/interrupts related).

- sryudens: Transparency problems (Test->Option->Gal, Bonus Game during Demo mode).
  e.g. in the latter, transparency pen is set to ff instead of 0 (I/O address 2b)

Notes:

- daimyojn: In Test->Option, press "N Ron Ron N" to access more options

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "machine/msm6242.h"
#include "includes/dynax.h"


/***************************************************************************

                        Blitter Data Format

The gfx data is a bitstream. Command size is always 3 bits, argument size
can be from 1 to 8 bits (up to 16 bits seem to be allowed, but not used).

Data starts with an 8 bit header:
7------- not used
-654---- size-1 of arguments indicating pen number (B)
----3210 size-1 of arguments indicating number of pixels (A)

The commands are:
000 Increment Y
001 Followed by A bits (N) and by B bits (P): draw N+1 pixels using pen P
010 Followed by A bits (N) and by (N+1)*B bits: copy N+1 pixels
011 Followed by A bits (N): skip N pixels
100 not used
101 Followed by 4 bits: change argument size
110 Followed by 3 bits: change pen size
111 Stop.

The drawing operation is verified (quiz365) to modify ddenlovr_blit_y.

***************************************************************************/

enum { BLIT_NEXT = 0, BLIT_LINE, BLIT_COPY, BLIT_SKIP, BLIT_CHANGE_NUM, BLIT_CHANGE_PEN, BLIT_UNKNOWN, BLIT_STOP };
//                                          0          1                2                   3               4               5                   6                   7
static const int ddenlovr_commands[8]	= { BLIT_NEXT, BLIT_LINE,		BLIT_COPY,			BLIT_SKIP,		BLIT_UNKNOWN,	BLIT_CHANGE_NUM,	BLIT_CHANGE_PEN,	BLIT_STOP	};
static const int hanakanz_commands[8]	= { BLIT_NEXT, BLIT_CHANGE_PEN,	BLIT_CHANGE_NUM,	BLIT_UNKNOWN,	BLIT_SKIP,		BLIT_COPY,			BLIT_LINE,			BLIT_STOP	};
static const int mjflove_commands[8]	= { BLIT_STOP, BLIT_CHANGE_PEN,	BLIT_CHANGE_NUM,	BLIT_UNKNOWN,	BLIT_SKIP,		BLIT_COPY,			BLIT_LINE,			BLIT_NEXT	};

VIDEO_START( ddenlovr )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int i;

	for (i = 0; i < 8; i++)
	{
		state->m_ddenlovr_pixmap[i] = auto_alloc_array(machine, UINT8, 512 * 512);
		state->m_ddenlovr_scroll[i * 2 + 0] = state->m_ddenlovr_scroll[i * 2 + 1] = 0;
	}

	state->m_extra_layers = 0;

	state->m_ddenlovr_clip_ctrl = 0x0f;
	state->m_ddenlovr_layer_enable = state->m_ddenlovr_layer_enable2 = 0x0f;
	state->m_ddenlovr_blit_pen_mask = 0xff;

	// older games do not set these !?
	state->m_ddenlovr_clip_width = 0x400;
	state->m_ddenlovr_clip_height = 0x400;

	state->m_ddenlovr_blit_rom_bits = 8;
	state->m_ddenlovr_blit_commands = ddenlovr_commands;

	/* init to 0 the remaining elements */
	state->m_ddenlovr_dest_layer = 0;
	state->m_ddenlovr_blit_flip = 0;
	state->m_ddenlovr_blit_x = 0;
	state->m_ddenlovr_blit_y = 0;
	state->m_ddenlovr_blit_address = 0;
	state->m_ddenlovr_blit_pen = 0;
	state->m_ddenlovr_blit_pen_mode = 0;
	state->m_ddenlovr_blitter_irq_flag = 0;
	state->m_ddenlovr_blitter_irq_enable = 0;
	state->m_ddenlovr_rect_width = 0;
	state->m_ddenlovr_rect_height = 0;
	state->m_ddenlovr_line_length = 0;
	state->m_ddenlovr_clip_x = 0;
	state->m_ddenlovr_clip_y = 0;
	state->m_ddenlovr_priority = 0;
	state->m_ddenlovr_priority2 = 0;
	state->m_ddenlovr_bgcolor = 0;
	state->m_ddenlovr_bgcolor2 = 0;
	state->m_ddenlovr_blit_latch = 0;
	state->m_ddenlovr_blit_regs[0] = 0;
	state->m_ddenlovr_blit_regs[1] = 0;

	for (i = 0; i < 8; i++)
	{
		state->m_ddenlovr_palette_base[i] = 0;
		state->m_ddenlovr_palette_mask[i] = 0;
		state->m_ddenlovr_transparency_pen[i] = 0;
		state->m_ddenlovr_transparency_mask[i] = 0;
	}

	/* register save states */
	state->save_item(NAME(state->m_ddenlovr_dest_layer));
	state->save_item(NAME(state->m_ddenlovr_blit_flip));
	state->save_item(NAME(state->m_ddenlovr_blit_x));
	state->save_item(NAME(state->m_ddenlovr_blit_y));
	state->save_item(NAME(state->m_ddenlovr_blit_address));
	state->save_item(NAME(state->m_ddenlovr_blit_pen));
	state->save_item(NAME(state->m_ddenlovr_blit_pen_mode));
	state->save_item(NAME(state->m_ddenlovr_blitter_irq_flag));
	state->save_item(NAME(state->m_ddenlovr_blitter_irq_enable));
	state->save_item(NAME(state->m_ddenlovr_rect_width));
	state->save_item(NAME(state->m_ddenlovr_rect_height));
	state->save_item(NAME(state->m_ddenlovr_clip_width));
	state->save_item(NAME(state->m_ddenlovr_clip_height));
	state->save_item(NAME(state->m_ddenlovr_line_length));
	state->save_item(NAME(state->m_ddenlovr_clip_ctrl));
	state->save_item(NAME(state->m_ddenlovr_clip_x));
	state->save_item(NAME(state->m_ddenlovr_clip_y));
	state->save_item(NAME(state->m_ddenlovr_scroll));
	state->save_item(NAME(state->m_ddenlovr_priority));
	state->save_item(NAME(state->m_ddenlovr_priority2));
	state->save_item(NAME(state->m_ddenlovr_bgcolor));
	state->save_item(NAME(state->m_ddenlovr_bgcolor2));
	state->save_item(NAME(state->m_ddenlovr_layer_enable));
	state->save_item(NAME(state->m_ddenlovr_layer_enable2));
	state->save_item(NAME(state->m_ddenlovr_palette_base));
	state->save_item(NAME(state->m_ddenlovr_palette_mask));
	state->save_item(NAME(state->m_ddenlovr_transparency_pen));
	state->save_item(NAME(state->m_ddenlovr_transparency_mask));
	state->save_item(NAME(state->m_ddenlovr_blit_latch));
	state->save_item(NAME(state->m_ddenlovr_blit_pen_mask));
	state->save_item(NAME(state->m_ddenlovr_blit_regs));

	state->save_pointer(NAME(state->m_ddenlovr_pixmap[0]), 512 * 512);
	state->save_pointer(NAME(state->m_ddenlovr_pixmap[1]), 512 * 512);
	state->save_pointer(NAME(state->m_ddenlovr_pixmap[2]), 512 * 512);
	state->save_pointer(NAME(state->m_ddenlovr_pixmap[3]), 512 * 512);
	state->save_pointer(NAME(state->m_ddenlovr_pixmap[4]), 512 * 512);
	state->save_pointer(NAME(state->m_ddenlovr_pixmap[5]), 512 * 512);
	state->save_pointer(NAME(state->m_ddenlovr_pixmap[6]), 512 * 512);
	state->save_pointer(NAME(state->m_ddenlovr_pixmap[7]), 512 * 512);
}

static VIDEO_START( mmpanic )
{
	dynax_state *state = machine.driver_data<dynax_state>();

	VIDEO_START_CALL(ddenlovr);

	state->m_extra_layers = 1;
}

static VIDEO_START( hanakanz )
{
	dynax_state *state = machine.driver_data<dynax_state>();

	VIDEO_START_CALL(ddenlovr);

	state->m_ddenlovr_blit_rom_bits = 16;
	state->m_ddenlovr_blit_commands = hanakanz_commands;
}

static VIDEO_START( mjflove )
{
	dynax_state *state = machine.driver_data<dynax_state>();

	VIDEO_START_CALL(ddenlovr);

	state->m_ddenlovr_blit_commands = mjflove_commands;
}

static void ddenlovr_flipscreen_w( UINT8 data )
{
	logerror("flipscreen = %02x (%s)\n", data, (data & 1) ? "off" : "on");
}

static void ddenlovr_blit_flip_w( running_machine &machine, UINT8 data )
{
	dynax_state *state = machine.driver_data<dynax_state>();

	if ((data ^ state->m_ddenlovr_blit_flip) & 0xec)
	{
#ifdef MAME_DEBUG
		popmessage("warning ddenlovr_blit_flip = %02x", data);
#endif
		logerror("warning ddenlovr_blit_flip = %02x\n", data);
	}

	state->m_ddenlovr_blit_flip = data;
}

WRITE8_HANDLER( ddenlovr_bgcolor_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_bgcolor = data;
}

static WRITE8_HANDLER( ddenlovr_bgcolor2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_bgcolor2 = data;
}

static WRITE16_HANDLER( ddenlovr16_bgcolor_w )
{
	if (ACCESSING_BITS_0_7)
		ddenlovr_bgcolor_w(space, offset, data);
}


WRITE8_HANDLER( ddenlovr_priority_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_priority = data;
}

static WRITE8_HANDLER( ddenlovr_priority2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_priority2 = data;
}

static WRITE16_HANDLER( ddenlovr16_priority_w )
{
	if (ACCESSING_BITS_0_7)
		ddenlovr_priority_w(space, offset, data);
}


WRITE8_HANDLER( ddenlovr_layer_enable_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_layer_enable = data;
}

static WRITE8_HANDLER( ddenlovr_layer_enable2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_layer_enable2 = data;
}


static WRITE16_HANDLER( ddenlovr16_layer_enable_w )
{
	if (ACCESSING_BITS_0_7)
		ddenlovr_layer_enable_w(space, offset, data);
}



static void do_plot( running_machine &machine, int x, int y, int pen )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int addr, temp;
	int xclip, yclip;

	y &= 0x1ff;
	x &= 0x1ff;

	// swap x & y (see hanakanz gal check)
	if (state->m_ddenlovr_blit_flip & 0x10)	{  temp = x;   x = y;   y = temp;  }

	// clipping rectangle (see hanakanz / hkagerou gal check)
#if 0
	xclip	=	(x < state->m_ddenlovr_clip_x) || (x > state->m_ddenlovr_clip_x + state->m_ddenlovr_clip_width);
	yclip	=	(y < state->m_ddenlovr_clip_y) || (y > state->m_ddenlovr_clip_y + state->m_ddenlovr_clip_height);
#else
	xclip	=	(x < state->m_ddenlovr_clip_x) || (x > state->m_ddenlovr_clip_width);
	yclip	=	(y < state->m_ddenlovr_clip_y) || (y > state->m_ddenlovr_clip_height);
#endif

	if (!(state->m_ddenlovr_clip_ctrl & 1) &&  xclip) return;
	if (!(state->m_ddenlovr_clip_ctrl & 2) && !xclip) return;
	if (!(state->m_ddenlovr_clip_ctrl & 4) &&  yclip) return;
	if (!(state->m_ddenlovr_clip_ctrl & 8) && !yclip) return;

	addr = 512 * y + x;

	if (state->m_ddenlovr_dest_layer & 0x0001) state->m_ddenlovr_pixmap[0][addr] = pen;
	if (state->m_ddenlovr_dest_layer & 0x0002) state->m_ddenlovr_pixmap[1][addr] = pen;
	if (state->m_ddenlovr_dest_layer & 0x0004) state->m_ddenlovr_pixmap[2][addr] = pen;
	if (state->m_ddenlovr_dest_layer & 0x0008) state->m_ddenlovr_pixmap[3][addr] = pen;

	if (!state->m_extra_layers)	return;

	if (state->m_ddenlovr_dest_layer & 0x0100) state->m_ddenlovr_pixmap[4][addr] = pen;
	if (state->m_ddenlovr_dest_layer & 0x0200) state->m_ddenlovr_pixmap[5][addr] = pen;
	if (state->m_ddenlovr_dest_layer & 0x0400) state->m_ddenlovr_pixmap[6][addr] = pen;
	if (state->m_ddenlovr_dest_layer & 0x0800) state->m_ddenlovr_pixmap[7][addr] = pen;
}


INLINE int fetch_bit( UINT8 *src_data, int src_len, int *bit_addr )
{
	int baddr = *bit_addr;

	*bit_addr = ((*bit_addr) + 1) & 0x7ffffff;

	if (baddr / 8 >= src_len)
	{
#ifdef MAME_DEBUG
		popmessage("GFX ROM OVER %06x", baddr / 8);
#endif
		return 1;
	}

	return (src_data[baddr / 8] >> (7 - (baddr & 7))) & 1;
}

INLINE int fetch_word( UINT8 *src_data, int src_len, int *bit_addr, int word_len )
{
	int res = 0;

	while (word_len--)
	{
		res = (res << 1) | fetch_bit(src_data, src_len, bit_addr);
	}
	return res;
}



INLINE void log_draw_error( int src, int cmd )
{
#ifdef MAME_DEBUG
	popmessage("%06x: warning unknown pixel command %02x", src, cmd);
#endif
	logerror("%06x: warning unknown pixel command %02x\n", src, cmd);
}

/*  Copy from ROM
    initialized arguments are
    0D/0E/0F source data pointer
    14 X
    02 Y
    00 dest layer
    05 unknown, related to layer
    04 blit_pen
    06 blit_pen_mode (replace values stored in ROM)
*/

static int blit_draw( running_machine &machine, int src, int sx )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	UINT8 *src_data = machine.region("blitter")->base();
	int src_len = machine.region("blitter")->bytes();
	int bit_addr = (src & 0xffffff) * state->m_ddenlovr_blit_rom_bits;	/* convert to bit address */
	int pen_size, arg_size, cmd;
	int x;
	int xinc = (state->m_ddenlovr_blit_flip & 1) ? -1 : 1;
	int yinc = (state->m_ddenlovr_blit_flip & 2) ? -1 : 1;

	pen_size = fetch_word(src_data, src_len, &bit_addr, 4) + 1;
	arg_size = fetch_word(src_data, src_len, &bit_addr, 4) + 1;

#ifdef MAME_DEBUG
	if (pen_size > 4 || arg_size > 8)
		popmessage("warning: pen_size %d arg_size %d", pen_size, arg_size);
#endif

	// sryudens game bug
	if (pen_size == 16 && arg_size == 16)
		return src;

	x = sx;

	for (;;)
	{
		cmd = fetch_word(src_data, src_len, &bit_addr, 3);
		switch (state->m_ddenlovr_blit_commands[cmd])
		{
			case BLIT_NEXT:
				/* next line */
				state->m_ddenlovr_blit_y += yinc;
				x = sx;
				break;

			case BLIT_LINE:
				{
					int length = fetch_word(src_data, src_len, &bit_addr, arg_size);
					int pen    = fetch_word(src_data, src_len, &bit_addr, pen_size);

					if (state->m_ddenlovr_blit_pen_mode)
						pen = (state->m_ddenlovr_blit_pen & 0x0f);
					pen |= state->m_ddenlovr_blit_pen & 0xf0;

					while (length-- >= 0)
					{
						do_plot(machine, x, state->m_ddenlovr_blit_y, pen);
						x += xinc;
					}
				}
				break;

			case BLIT_COPY:
				{
					int length = fetch_word(src_data, src_len, &bit_addr, arg_size);

					while (length-- >= 0)
					{
						int pen = fetch_word(src_data, src_len, &bit_addr, pen_size);
						if (state->m_ddenlovr_blit_pen_mode)
							pen = (state->m_ddenlovr_blit_pen & 0x0f);
						pen |= state->m_ddenlovr_blit_pen & 0xf0;

						do_plot(machine, x, state->m_ddenlovr_blit_y, pen);
						x += xinc;
					}
				}
				break;

			case BLIT_SKIP:
				x += xinc * fetch_word(src_data, src_len, &bit_addr, arg_size);
				break;

			case BLIT_CHANGE_NUM:
				arg_size = fetch_word(src_data, src_len, &bit_addr, 4) + 1;
				break;

			case BLIT_CHANGE_PEN:
				pen_size = fetch_word(src_data, src_len, &bit_addr, 3) + 1;
				break;

			default:
				log_draw_error(src, cmd);
			// fall through
			case BLIT_STOP:
				return ((bit_addr + state->m_ddenlovr_blit_rom_bits - 1) / state->m_ddenlovr_blit_rom_bits) & 0xffffff;
		}
	}
}



/*  Draw a simple rectangle
*/
static void blit_rect_xywh( running_machine &machine )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int x, y;

#ifdef MAME_DEBUG
//  if (state->m_ddenlovr_clip_ctrl != 0x0f)
//      popmessage("RECT clipx=%03x clipy=%03x ctrl=%x", state->m_ddenlovr_clip_x, state->m_ddenlovr_clip_y, state->m_ddenlovr_clip_ctrl);
#endif

	for (y = 0; y <= state->m_ddenlovr_rect_height; y++)
		for (x = 0; x <= state->m_ddenlovr_rect_width; x++)
			do_plot(machine, x + state->m_ddenlovr_blit_x, y + state->m_ddenlovr_blit_y, state->m_ddenlovr_blit_pen);
}



/*  Unknown. Initialized arguments are
    00 dest layer
    05 unknown, related to layer
    14 X - always 0?
    02 Y
    0a width - always 0?
    0b height
    04 blit_pen
    0c line_length - always 0?
*/
static void blit_rect_yh( running_machine &machine )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int start = 512 * state->m_ddenlovr_blit_y;
	int length = 512 * (state->m_ddenlovr_rect_height + 1);

#ifdef MAME_DEBUG
//  if (state->m_ddenlovr_clip_ctrl != 0x0f)
//      popmessage("UNK8C clipx=%03x clipy=%03x ctrl=%x", state->m_ddenlovr_clip_x, state->m_ddenlovr_clip_y, state->m_ddenlovr_clip_ctrl);
#endif

	if (start < 512 * 512)
	{
		if (start + length > 512 * 512)
			length = 512 * 512 - start;

		if (state->m_ddenlovr_dest_layer & 0x0001) memset(state->m_ddenlovr_pixmap[0] + start, state->m_ddenlovr_blit_pen, length);
		if (state->m_ddenlovr_dest_layer & 0x0002) memset(state->m_ddenlovr_pixmap[1] + start, state->m_ddenlovr_blit_pen, length);
		if (state->m_ddenlovr_dest_layer & 0x0004) memset(state->m_ddenlovr_pixmap[2] + start, state->m_ddenlovr_blit_pen, length);
		if (state->m_ddenlovr_dest_layer & 0x0008) memset(state->m_ddenlovr_pixmap[3] + start, state->m_ddenlovr_blit_pen, length);

		if (!state->m_extra_layers)	return;

		if (state->m_ddenlovr_dest_layer & 0x0100) memset(state->m_ddenlovr_pixmap[4] + start, state->m_ddenlovr_blit_pen, length);
		if (state->m_ddenlovr_dest_layer & 0x0200) memset(state->m_ddenlovr_pixmap[5] + start, state->m_ddenlovr_blit_pen, length);
		if (state->m_ddenlovr_dest_layer & 0x0400) memset(state->m_ddenlovr_pixmap[6] + start, state->m_ddenlovr_blit_pen, length);
		if (state->m_ddenlovr_dest_layer & 0x0800) memset(state->m_ddenlovr_pixmap[7] + start, state->m_ddenlovr_blit_pen, length);
	}
}



/*  Fill from (X,Y) to end of ddenlovr_pixmap
    initialized arguments are
    00 dest layer
    05 unknown, related to layer
    14 X
    02 Y
    04 blit_pen
*/
static void blit_fill_xy( running_machine &machine, int x, int y )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int start = 512 * y + x;

#ifdef MAME_DEBUG
//  if (x || y)
//      popmessage("FILL command X %03x Y %03x", x, y);
#endif

	if (state->m_ddenlovr_dest_layer & 0x0001) memset(state->m_ddenlovr_pixmap[0] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);
	if (state->m_ddenlovr_dest_layer & 0x0002) memset(state->m_ddenlovr_pixmap[1] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);
	if (state->m_ddenlovr_dest_layer & 0x0004) memset(state->m_ddenlovr_pixmap[2] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);
	if (state->m_ddenlovr_dest_layer & 0x0008) memset(state->m_ddenlovr_pixmap[3] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);

	if (!state->m_extra_layers)	return;

	if (state->m_ddenlovr_dest_layer & 0x0100) memset(state->m_ddenlovr_pixmap[4] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);
	if (state->m_ddenlovr_dest_layer & 0x0200) memset(state->m_ddenlovr_pixmap[5] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);
	if (state->m_ddenlovr_dest_layer & 0x0400) memset(state->m_ddenlovr_pixmap[6] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);
	if (state->m_ddenlovr_dest_layer & 0x0800) memset(state->m_ddenlovr_pixmap[7] + start, state->m_ddenlovr_blit_pen, 512 * 512 - start);
}



/*  Draw horizontal line
    initialized arguments are
    00 dest layer
    05 unknown, related to layer
    14 X
    02 Y
    0c line length
    04 blit_pen
    ddenlovr_blit_x and ddenlovr_blit_y are left pointing to the last pixel at the end of the command
*/
static void blit_horiz_line( running_machine &machine )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int i;

#ifdef MAME_DEBUG
	popmessage("LINE X");

	if (state->m_ddenlovr_clip_ctrl != 0x0f)
		popmessage("LINE X clipx=%03x clipy=%03x ctrl=%x", state->m_ddenlovr_clip_x, state->m_ddenlovr_clip_y, state->m_ddenlovr_clip_ctrl);

	if (state->m_ddenlovr_blit_flip)
		popmessage("LINE X flip=%x", state->m_ddenlovr_blit_flip);
#endif

	for (i = 0; i <= state->m_ddenlovr_line_length; i++)
		do_plot(machine, state->m_ddenlovr_blit_x++, state->m_ddenlovr_blit_y, state->m_ddenlovr_blit_pen);
}



/*  Draw vertical line
    initialized arguments are
    00 dest layer
    05 unknown, related to layer
    14 X
    02 Y
    0c line length
    04 blit_pen
    ddenlovr_blit_x and ddenlovr_blit_y are left pointing to the last pixel at the end of the command
*/
static void blit_vert_line( running_machine &machine )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int i;

#ifdef MAME_DEBUG
	popmessage("LINE Y");

	if (state->m_ddenlovr_clip_ctrl != 0x0f)
		popmessage("LINE Y clipx=%03x clipy=%03x ctrl=%x", state->m_ddenlovr_clip_x, state->m_ddenlovr_clip_y, state->m_ddenlovr_clip_ctrl);
#endif

	for (i = 0; i <= state->m_ddenlovr_line_length; i++)
		do_plot(machine, state->m_ddenlovr_blit_x, state->m_ddenlovr_blit_y++, state->m_ddenlovr_blit_pen);
}




INLINE void log_blit( running_machine &machine, int data )
{
	dynax_state *state = machine.driver_data<dynax_state>();

#if 1
	logerror("%s: blit src %06x x %03x y %03x flags %02x layer %02x pen %02x penmode %02x w %03x h %03x linelen %03x flip %02x clip: ctrl %x xy %03x %03x wh %03x %03x\n",
			machine.describe_context(),
			state->m_ddenlovr_blit_address, state->m_ddenlovr_blit_x, state->m_ddenlovr_blit_y, data,
			state->m_ddenlovr_dest_layer, state->m_ddenlovr_blit_pen, state->m_ddenlovr_blit_pen_mode, state->m_ddenlovr_rect_width, state->m_ddenlovr_rect_height, state->m_ddenlovr_line_length, state->m_ddenlovr_blit_flip,
			state->m_ddenlovr_clip_ctrl, state->m_ddenlovr_clip_x, state->m_ddenlovr_clip_y, state->m_ddenlovr_clip_width, state->m_ddenlovr_clip_height);
#endif
}

static void blitter_w( address_space *space, int blitter, offs_t offset, UINT8 data, int irq_vector )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	int hi_bits;

g_profiler.start(PROFILER_VIDEO);

	switch (offset)
	{
	case 0:
		state->m_ddenlovr_blit_regs[blitter] = data;
		break;

	case 1:
		hi_bits = (state->m_ddenlovr_blit_regs[blitter] & 0xc0) << 2;

		switch (state->m_ddenlovr_blit_regs[blitter] & 0x3f)
		{
		case 0x00:
			if (blitter)	state->m_ddenlovr_dest_layer = (state->m_ddenlovr_dest_layer & 0x00ff) | (data << 8);
			else			state->m_ddenlovr_dest_layer = (state->m_ddenlovr_dest_layer & 0xff00) | (data << 0);
			break;

		case 0x01:
			ddenlovr_flipscreen_w(data);
			break;

		case 0x02:
			state->m_ddenlovr_blit_y = data | hi_bits;
			break;

		case 0x03:
			ddenlovr_blit_flip_w(space->machine(), data);
			break;

		case 0x04:
			state->m_ddenlovr_blit_pen = data;
			break;

		case 0x05:
			state->m_ddenlovr_blit_pen_mask = data;
			break;

		case 0x06:
			// related to pen, can be 0 or 1 for 0x10 blitter command
			// 0 = only bits 7-4 of ddenlovr_blit_pen contain data
			// 1 = bits 3-0 contain data as well
			state->m_ddenlovr_blit_pen_mode = data;
			break;

		case 0x0a:
			state->m_ddenlovr_rect_width = data | hi_bits;
			break;

		case 0x0b:
			state->m_ddenlovr_rect_height = data | hi_bits;
			break;

		case 0x0c:
			state->m_ddenlovr_line_length = data | hi_bits;
			break;

		case 0x0d:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0xffff00) | (data <<0);
			break;
		case 0x0e:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0xff00ff) | (data <<8);
			break;
		case 0x0f:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0x00ffff) | (data<<16);
			break;

		case 0x14:
			state->m_ddenlovr_blit_x = data | hi_bits;
			break;

		case 0x16:
			state->m_ddenlovr_clip_x = data | hi_bits;
			break;

		case 0x17:
			state->m_ddenlovr_clip_y = data | hi_bits;
			break;

		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			state->m_ddenlovr_scroll[blitter * 8 + (state->m_ddenlovr_blit_regs[blitter] & 7)] = data | hi_bits;
			break;

		case 0x20:
			state->m_ddenlovr_clip_ctrl = data;
			break;

		case 0x24:

			log_blit(space->machine(), data);

			switch (data)
			{
				case 0x04:	blit_fill_xy(space->machine(), 0, 0);
							break;
				case 0x14:	blit_fill_xy(space->machine(), state->m_ddenlovr_blit_x, state->m_ddenlovr_blit_y);
							break;

				case 0x10:	state->m_ddenlovr_blit_address = blit_draw(space->machine(), state->m_ddenlovr_blit_address, state->m_ddenlovr_blit_x);
							break;

				case 0x13:	blit_horiz_line(space->machine());
							break;
				case 0x1b:	blit_vert_line(space->machine());
							break;

				case 0x1c:	blit_rect_xywh(space->machine());
							break;

				// These two are issued one after the other (43 then 8c)
				// 8c is issued immediately after 43 has finished, without
				// changing any argument
				case 0x43:	break;
				case 0x8c:	blit_rect_yh(space->machine());
							break;

				default:
							;
				#ifdef MAME_DEBUG
					popmessage("unknown blitter command %02x", data);
					logerror("%06x: unknown blitter command %02x\n", cpu_get_pc(&space->device()), data);
				#endif
			}

			if (irq_vector)
				/* quizchq */
				device_set_input_line_and_vector(&space->device(), 0, HOLD_LINE, irq_vector);
			else
			{
				/* ddenlovr */
				if (state->m_ddenlovr_blitter_irq_enable)
				{
					state->m_ddenlovr_blitter_irq_flag = 1;
					device_set_input_line(&space->device(), 1, HOLD_LINE);
				}
			}
			break;

		default:
			logerror("%06x: Blitter %d reg %02x = %02x\n", cpu_get_pc(&space->device()), blitter, state->m_ddenlovr_blit_regs[blitter], data);
			break;
		}
	}

g_profiler.stop();
}




// differences wrt blitter_data_w: slightly different blitter commands
static void blitter_w_funkyfig( running_machine &machine, int blitter, offs_t offset, UINT8 data, int irq_vector )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int hi_bits;

g_profiler.start(PROFILER_VIDEO);

	switch(offset)
	{
	case 0:
		state->m_ddenlovr_blit_regs[blitter] = data;
		break;

	case 1:
		hi_bits = (state->m_ddenlovr_blit_regs[blitter] & 0xc0) << 2;

		switch (state->m_ddenlovr_blit_regs[blitter] & 0x3f)
		{
		case 0x00:
			if (blitter)	state->m_ddenlovr_dest_layer = (state->m_ddenlovr_dest_layer & 0x00ff) | (data << 8);
			else			state->m_ddenlovr_dest_layer = (state->m_ddenlovr_dest_layer & 0xff00) | (data << 0);
			break;

		case 0x01:
			ddenlovr_flipscreen_w(data);
			break;

		case 0x02:
			state->m_ddenlovr_blit_y = data | hi_bits;
			break;

		case 0x03:
			ddenlovr_blit_flip_w(machine, data);
			break;

		case 0x04:
			state->m_ddenlovr_blit_pen = data;
			break;

		case 0x05:
			state->m_ddenlovr_blit_pen_mask = data;
			break;

		case 0x06:
			// related to pen, can be 0 or 1 for 0x10 blitter command
			// 0 = only bits 7-4 of ddenlovr_blit_pen contain data
			// 1 = bits 3-0 contain data as well
			state->m_ddenlovr_blit_pen_mode = data;
			break;

		case 0x0a:
			state->m_ddenlovr_rect_width = data | hi_bits;
			break;

		case 0x0b:
			state->m_ddenlovr_rect_height = data | hi_bits;
			break;

		case 0x0c:
			state->m_ddenlovr_line_length = data | hi_bits;
			break;

		case 0x0d:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0xffff00) | (data <<  0);
			break;
		case 0x0e:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0xff00ff) | (data <<  8);
			break;
		case 0x0f:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0x00ffff) | (data << 16);
			break;

		case 0x14:
			state->m_ddenlovr_blit_x = data | hi_bits;
			break;

		case 0x16:
			state->m_ddenlovr_clip_x = data | hi_bits;
			break;

		case 0x17:
			state->m_ddenlovr_clip_y = data | hi_bits;
			break;

		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			state->m_ddenlovr_scroll[blitter * 8 + (state->m_ddenlovr_blit_regs[blitter] & 7)] = data | hi_bits;
			break;

		case 0x20:
			state->m_ddenlovr_clip_ctrl = data;
			break;

		case 0x24:

			log_blit(machine, data);

			switch (data)
			{

				case 0x84:	// same as 04?
				case 0x04:	blit_fill_xy(machine, 0, 0);
							break;

//              unused?
//              case 0x14:  blit_fill_xy(machine, state->m_ddenlovr_blit_x, state->m_ddenlovr_blit_y);
//                          break;

				case 0x00/*0x10*/:	state->m_ddenlovr_blit_address = blit_draw(machine, state->m_ddenlovr_blit_address, state->m_ddenlovr_blit_x);
							break;

				case 0x0b:	// same as 03? see the drawing of the R in "cRoss hatch" (key test)
				case 0x03/*0x13*/:	blit_horiz_line(machine);
							break;
//              unused?
//              case 0x1b:  blit_vert_line(machine);
//                          break;

				case 0x0c/*0x1c*/:	blit_rect_xywh(machine);
							break;

				// These two are issued one after the other (43 then 8c)
				// 8c is issued immediately after 43 has finished, without
				// changing any argument
				case 0x43:	break;
				case 0x8c:	blit_rect_yh(machine);
							break;

				default:
							;
				#ifdef MAME_DEBUG
					popmessage("unknown blitter command %02x", data);
					logerror("%s: unknown blitter command %02x\n", machine.describe_context(), data);
				#endif
			}

			device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, irq_vector);
			break;

		default:
			logerror("%s: Blitter %d reg %02x = %02x\n", machine.describe_context(), blitter, state->m_ddenlovr_blit_regs[blitter], data);
			break;
		}
	}

g_profiler.stop();
}




static WRITE8_HANDLER( hanakanz_blitter_reg_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_blit_latch = data;
}

// differences wrt blitter_data_w: registers are shuffled around, hi_bits in the low bits, clip_w/h, includes layers registers
static WRITE8_HANDLER( hanakanz_blitter_data_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	int hi_bits;

g_profiler.start(PROFILER_VIDEO);

	hi_bits = (state->m_ddenlovr_blit_latch & 0x03) << 8;

	switch (state->m_ddenlovr_blit_latch & 0xfe)
	{
		case 0x00:
			state->m_ddenlovr_dest_layer = data;
			break;

		case 0x04:
			ddenlovr_flipscreen_w(data);
			break;

		case 0x08:
			state->m_ddenlovr_blit_y = data | hi_bits;
			break;

		case 0x0c:
			ddenlovr_blit_flip_w(space->machine(), data);
			break;

		case 0x10:
			state->m_ddenlovr_blit_pen = data;
			break;

		case 0x14:
			state->m_ddenlovr_blit_pen_mask = data;
			break;

		case 0x18:
			// related to pen, can be 0 or 1 for 0x10 blitter command
			// 0 = only bits 7-4 of ddenlovr_blit_pen contain data
			// 1 = bits 3-0 contain data as well
			state->m_ddenlovr_blit_pen_mode = data;
			break;

		case 0x28:
			state->m_ddenlovr_rect_width = data | hi_bits;
			break;

		case 0x2c:
			state->m_ddenlovr_rect_height = data | hi_bits;
			break;

		case 0x30:
			state->m_ddenlovr_line_length = data | hi_bits;
			break;

		case 0x34:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0xffff00) | (data <<  0);
			break;
		case 0x38:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0xff00ff) | (data <<  8);
			break;
		case 0x3c:
			state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address & 0x00ffff) | (data << 16);
			break;

		case 0x50:
			state->m_ddenlovr_blit_x = data | hi_bits;
			break;

		case 0x58:
			state->m_ddenlovr_clip_x = data | hi_bits;
			break;

		case 0x5c:
			state->m_ddenlovr_clip_y = data | hi_bits;
			break;

		case 0x60:
		case 0x64:
		case 0x68:
		case 0x6c:
		case 0x70:
		case 0x74:
		case 0x78:
		case 0x7c:
			state->m_ddenlovr_scroll[(state->m_ddenlovr_blit_latch & 0x1c) >> 2] = data | hi_bits;
			break;

		case 0x80:
			state->m_ddenlovr_clip_ctrl = data;
			break;

		case 0x88:
		case 0x8a:	// can be 3ff
			state->m_ddenlovr_clip_height = data | hi_bits;
			break;

		case 0x8c:
		case 0x8e:	// can be 3ff
			state->m_ddenlovr_clip_width = data | hi_bits;
			break;

		case 0xc0:
		case 0xc2:
		case 0xc4:
		case 0xc6:
			state->m_ddenlovr_palette_base[(state->m_ddenlovr_blit_latch >> 1) & 3] = data | (hi_bits & 0x100);
			break;

		case 0xc8:
		case 0xca:
		case 0xcc:
		case 0xce:
			state->m_ddenlovr_palette_mask[(state->m_ddenlovr_blit_latch >> 1) & 3] = data;
			break;

		case 0xd0:
		case 0xd2:
		case 0xd4:
		case 0xd6:
			state->m_ddenlovr_transparency_pen[(state->m_ddenlovr_blit_latch >> 1) & 3] = data;
			break;

		case 0xd8:
		case 0xda:
		case 0xdc:
		case 0xde:
			state->m_ddenlovr_transparency_mask[(state->m_ddenlovr_blit_latch >> 1) & 3] = data;
			break;

		case 0xe4:
			ddenlovr_priority_w(space, 0, data);
			break;

		case 0xe6:
			ddenlovr_layer_enable_w(space, 0, data);
			break;

		case 0xe8:
			state->m_ddenlovr_bgcolor = data | hi_bits;
			break;

		case 0x90:

			log_blit(space->machine(), data);

			switch (data)
			{
				case 0x04:	blit_fill_xy(space->machine(), 0, 0);
							break;
				case 0x14:	blit_fill_xy(space->machine(), state->m_ddenlovr_blit_x, state->m_ddenlovr_blit_y);
							break;

				case 0x10:	state->m_ddenlovr_blit_address = blit_draw(space->machine(), state->m_ddenlovr_blit_address, state->m_ddenlovr_blit_x);
							break;

				case 0x13:	blit_horiz_line(space->machine());
							break;
				case 0x1b:	blit_vert_line(space->machine());
							break;

				case 0x1c:	blit_rect_xywh(space->machine());
							break;

				// These two are issued one after the other (43 then 8c)
				// 8c is issued immediately after 43 has finished, without
				// changing any argument
				case 0x43:	break;
				case 0x8c:	blit_rect_yh(space->machine());
							break;

				default:
							;
				#ifdef MAME_DEBUG
					popmessage("unknown blitter command %02x", data);
					logerror("%06x: unknown blitter command %02x\n", cpu_get_pc(&space->device()), data);
				#endif
			}

			// NO IRQ !?

			break;

		default:
			logerror("%06x: Blitter 0 reg %02x = %02x\n", cpu_get_pc(&space->device()), state->m_ddenlovr_blit_latch, data);
			break;
	}

g_profiler.stop();
}


static WRITE8_HANDLER( rongrong_blitter_w )
{
	blitter_w(space, 0, offset, data, 0xf8);
}

static WRITE16_HANDLER( ddenlovr_blitter_w )
{
	if (ACCESSING_BITS_0_7)
		blitter_w(space, 0, offset, data & 0xff, 0);
}


static WRITE16_HANDLER( ddenlovr_blitter_irq_ack_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
	{
		if (data & 1)
		{
			state->m_ddenlovr_blitter_irq_enable = 1;
		}
		else
		{
			state->m_ddenlovr_blitter_irq_enable = 0;
			state->m_ddenlovr_blitter_irq_flag = 0;
		}
	}
}


static READ8_HANDLER( rongrong_gfxrom_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 *rom  = space->machine().region("blitter")->base();
	size_t size = space->machine().region("blitter")->bytes();
	int address = state->m_ddenlovr_blit_address;

	if (address >= size)
	{
		logerror("CPU#0 PC %06X: Error, Blitter address %06X out of range\n", cpu_get_pc(&space->device()), address);
		address %= size;
	}

	state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address + 1) & 0xffffff;

	return rom[address];
}

static READ16_HANDLER( ddenlovr_gfxrom_r )
{
	return rongrong_gfxrom_r(space, offset);
}


static void copylayer(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer )
{
	dynax_state *state = machine.driver_data<dynax_state>();
	int x,y;
	int scrollx = state->m_ddenlovr_scroll[layer / 4 * 8 + (layer % 4) + 0];
	int scrolly = state->m_ddenlovr_scroll[layer / 4 * 8 + (layer % 4) + 4];

	int palbase = state->m_ddenlovr_palette_base[layer];
	int penmask = state->m_ddenlovr_palette_mask[layer];

	int transpen = state->m_ddenlovr_transparency_pen[layer];
	int transmask = state->m_ddenlovr_transparency_mask[layer];

	palbase  &= ~penmask;
	transpen &= transmask;

	if (((state->m_ddenlovr_layer_enable2 << 4) | state->m_ddenlovr_layer_enable) & (1 << layer))
	{
		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				int pen = state->m_ddenlovr_pixmap[layer][512 * ((y + scrolly) & 0x1ff) + ((x + scrollx) & 0x1ff)];
				if ((pen & transmask) != transpen)
				{
					pen &= penmask;
					pen |= palbase;
					bitmap.pix16(y, x) = pen;
				}
			}
		}
	}
}

SCREEN_UPDATE_IND16(ddenlovr)
{
	dynax_state *state = screen.machine().driver_data<dynax_state>();

	static const int order[24][4] =
	{
		{ 3,2,1,0 }, { 2,3,1,0 }, { 3,1,2,0 }, { 1,3,2,0 }, { 2,1,3,0 }, { 1,2,3,0 },
		{ 3,2,0,1 }, { 2,3,0,1 }, { 3,0,2,1 }, { 0,3,2,1 }, { 2,0,3,1 }, { 0,2,3,1 },
		{ 3,1,0,2 }, { 1,3,0,2 }, { 3,0,1,2 }, { 0,3,1,2 }, { 1,0,3,2 }, { 0,1,3,2 },
		{ 2,1,0,3 }, { 1,2,0,3 }, { 2,0,1,3 }, { 0,2,1,3 }, { 1,0,2,3 }, { 0,1,2,3 }
	};

	int pri;

	int enab = state->m_ddenlovr_layer_enable;
	int enab2 = state->m_ddenlovr_layer_enable2;

#if 0
	static int base = 0x0;
	const UINT8 *gfx = screen.machine().region("blitter")->base();
	int next;
	memset(state->m_ddenlovr_pixmap[0], 0, 512 * 512);
	memset(state->m_ddenlovr_pixmap[1], 0, 512 * 512);
	memset(state->m_ddenlovr_pixmap[2], 0, 512 * 512);
	memset(state->m_ddenlovr_pixmap[3], 0, 512 * 512);
	state->m_ddenlovr_dest_layer = 8;
	state->m_ddenlovr_blit_pen = 0;
	state->m_ddenlovr_blit_pen_mode = 0;
	state->m_ddenlovr_blit_y = 5;
	state->m_ddenlovr_clip_ctrl = 0x0f;
	next = blit_draw(screen.machine(), base, 0);
	popmessage("GFX %06x", base);
	if (screen.machine().input().code_pressed(KEYCODE_S)) base = next;
	if (screen.machine().input().code_pressed_once(KEYCODE_X)) base = next;
	if (screen.machine().input().code_pressed(KEYCODE_C)) { base--; while ((gfx[base] & 0xf0) != 0x30) base--; }
	if (screen.machine().input().code_pressed(KEYCODE_V)) { base++; while ((gfx[base] & 0xf0) != 0x30) base++; }
	if (screen.machine().input().code_pressed_once(KEYCODE_D)) { base--; while ((gfx[base] & 0xf0) != 0x30) base--; }
	if (screen.machine().input().code_pressed_once(KEYCODE_F)) { base++; while ((gfx[base] & 0xf0) != 0x30) base++; }
#endif

	bitmap.fill(state->m_ddenlovr_bgcolor, cliprect);

#ifdef MAME_DEBUG
	if (screen.machine().input().code_pressed(KEYCODE_Z))
	{
		int mask, mask2;

		mask = 0;

		if (screen.machine().input().code_pressed(KEYCODE_Q))	mask |= 1;
		if (screen.machine().input().code_pressed(KEYCODE_W))	mask |= 2;
		if (screen.machine().input().code_pressed(KEYCODE_E))	mask |= 4;
		if (screen.machine().input().code_pressed(KEYCODE_R))	mask |= 8;

		mask2 = 0;

		if (state->m_extra_layers)
		{
			if (screen.machine().input().code_pressed(KEYCODE_A))	mask2 |= 1;
			if (screen.machine().input().code_pressed(KEYCODE_S))	mask2 |= 2;
			if (screen.machine().input().code_pressed(KEYCODE_D))	mask2 |= 4;
			if (screen.machine().input().code_pressed(KEYCODE_F))	mask2 |= 8;
		}

		if (mask || mask2)
		{
			state->m_ddenlovr_layer_enable &= mask;
			state->m_ddenlovr_layer_enable2 &= mask2;
		}
	}
#endif

	pri = state->m_ddenlovr_priority;

	if (pri >= 24)
	{
		popmessage("priority = %02x", pri);
		pri = 0;
	}

	copylayer(screen.machine(), bitmap, cliprect, order[pri][0]);
	copylayer(screen.machine(), bitmap, cliprect, order[pri][1]);
	copylayer(screen.machine(), bitmap, cliprect, order[pri][2]);
	copylayer(screen.machine(), bitmap, cliprect, order[pri][3]);

	if (state->m_extra_layers)
	{
		pri = state->m_ddenlovr_priority2;

		if (pri >= 24)
		{
			popmessage("priority2 = %02x", pri);
			pri = 0;
		}

		copylayer(screen.machine(), bitmap, cliprect, order[pri][0] + 4);
		copylayer(screen.machine(), bitmap, cliprect, order[pri][1] + 4);
		copylayer(screen.machine(), bitmap, cliprect, order[pri][2] + 4);
		copylayer(screen.machine(), bitmap, cliprect, order[pri][3] + 4);
	}

	state->m_ddenlovr_layer_enable = enab;
	state->m_ddenlovr_layer_enable2 = enab2;

	return 0;
}

static CUSTOM_INPUT( ddenlovr_special_r )
{
	dynax_state *state = field.machine().driver_data<dynax_state>();
	return state->m_ddenlovr_blitter_irq_flag;
}

static WRITE16_HANDLER( ddenlovr_coincounter_0_w )
{
	if (ACCESSING_BITS_0_7)
		coin_counter_w(space->machine(), 0, data & 1);
}
static WRITE16_HANDLER( ddenlovr_coincounter_1_w )
{
	if (ACCESSING_BITS_0_7)
		coin_counter_w(space->machine(), 1, data & 1);
}


static WRITE8_HANDLER( rongrong_palette_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	int r, g, b, d1, d2, indx;

	state->m_palram[offset] = data;

	indx = ((offset & 0x1e0) >> 1) | (offset & 0x00f);
	d1 = state->m_palram[offset & ~0x10];
	d2 = state->m_palram[offset |  0x10];

	r = d1 & 0x1f;
	g = d2 & 0x1f;
	/* what were they smoking??? */
	b = ((d1 & 0xe0) >> 5) | (d2 & 0xc0) >> 3;

	palette_set_color_rgb(space->machine(), indx, pal5bit(r), pal5bit(g), pal5bit(b));
}

static WRITE16_HANDLER( ddenlovr_palette_w )
{
	if (ACCESSING_BITS_0_7)
		rongrong_palette_w(space, offset, data & 0xff);
}


WRITE8_HANDLER( ddenlovr_palette_base_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_palette_base[offset] = data;
}

static WRITE8_HANDLER( ddenlovr_palette_base2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_palette_base[offset + 4] = data;
}

WRITE8_HANDLER( ddenlovr_palette_mask_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_palette_mask[offset] = data;
}

static WRITE8_HANDLER( ddenlovr_palette_mask2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_palette_mask[offset + 4] = data;
}


WRITE8_HANDLER( ddenlovr_transparency_pen_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_transparency_pen[offset] = data;
}

static WRITE8_HANDLER( ddenlovr_transparency_pen2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_transparency_pen[offset + 4] = data;
}


WRITE8_HANDLER( ddenlovr_transparency_mask_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_transparency_mask[offset] = data;
}

static WRITE8_HANDLER( ddenlovr_transparency_mask2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_ddenlovr_transparency_mask[offset + 4] = data;
}


static WRITE16_HANDLER( ddenlovr16_palette_base_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
		state->m_ddenlovr_palette_base[offset] = data & 0xff;
}

static WRITE16_HANDLER( ddenlovr16_palette_mask_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
		state->m_ddenlovr_palette_mask[offset] = data & 0xff;
}

static WRITE16_HANDLER( ddenlovr16_transparency_pen_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
		state->m_ddenlovr_transparency_pen[offset] = data & 0xff;
}

static WRITE16_HANDLER( ddenlovr16_transparency_mask_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
		state->m_ddenlovr_transparency_mask[offset] = data & 0xff;
}


static WRITE8_DEVICE_HANDLER( quizchq_oki_bank_w )
{
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base((data & 1) * 0x40000);
}

static WRITE16_DEVICE_HANDLER( ddenlovr_oki_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		okim6295_device *oki = downcast<okim6295_device *>(device);
		oki->set_bank_base((data & 7) * 0x40000);
	}
}



static WRITE16_DEVICE_HANDLER( quiz365_oki_bank1_w )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
	{
		state->m_okibank = (state->m_okibank & 2) | (data & 1);
		okim6295_device *oki = downcast<okim6295_device *>(device);
		oki->set_bank_base(state->m_okibank * 0x40000);
	}
}

static WRITE16_DEVICE_HANDLER( quiz365_oki_bank2_w )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
	{
		state->m_okibank = (state->m_okibank & 1) | ((data & 1) << 1);
		okim6295_device *oki = downcast<okim6295_device *>(device);
		oki->set_bank_base(state->m_okibank * 0x40000);
	}
}



static READ8_HANDLER( unk_r )
{
	return 0x78;
}

static READ16_HANDLER( unk16_r )
{
	return unk_r(space, offset);
}


static WRITE8_DEVICE_HANDLER( ddenlovr_select_w )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();
	state->m_dsw_sel = data;
}

static WRITE16_HANDLER( ddenlovr_select_16_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
		state->m_dsw_sel = data;
}

static WRITE8_HANDLER( ddenlovr_select2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_input_sel = data;
}

static WRITE16_HANDLER( ddenlovr_select2_16_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
		state->m_input_sel = data;
}

static READ8_HANDLER( rongrong_input2_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

//  logerror("%04x: input2_r offset %d select %x\n", cpu_get_pc(&space->device()), offset, state->m_input_sel);
	/* 0 and 1 are read from offset 1, 2 from offset 0... */
	switch (state->m_input_sel)
	{
		case 0x00:	return input_port_read(space->machine(), "P1");
		case 0x01:	return input_port_read(space->machine(), "P2");
		case 0x02:	return input_port_read(space->machine(), "SYSTEM");
	}
	return 0xff;
}


static READ8_DEVICE_HANDLER( quiz365_input_r )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))	return input_port_read(device->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 1))	return input_port_read(device->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 2))	return input_port_read(device->machine(), "DSW3");
	if (!BIT(state->m_dsw_sel, 3))	return 0xff;//device->machine().rand();
	if (!BIT(state->m_dsw_sel, 4))	return 0xff;//device->machine().rand();
	return 0xff;
}

static READ16_HANDLER( quiz365_input2_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

//  logerror("%04x: input2_r offset %d select %x\n",cpu_get_pc(&space->device()), offset, state->m_input_sel);
	/* 0 and 1 are read from offset 1, 2 from offset 0... */
	switch (state->m_input_sel)
	{
		case 0x10:	return input_port_read(space->machine(), "P1");
		case 0x11:	return input_port_read(space->machine(), "P2");
		case 0x12:	return input_port_read(space->machine(), "SYSTEM");
	}
	return 0xff;
}


static WRITE8_HANDLER( rongrong_blitter_busy_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_rongrong_blitter_busy_select = data;

	if (data != 0x18)
		logerror("%04x: rongrong_blitter_busy_w data = %02x\n", cpu_get_pc(&space->device()), data);
}

static READ8_HANDLER( rongrong_blitter_busy_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_rongrong_blitter_busy_select)
	{
		case 0x18:	return 0;	// bit 5 = blitter busy

		default:
			logerror("%04x: rongrong_blitter_busy_r with select = %02x\n", cpu_get_pc(&space->device()), state->m_rongrong_blitter_busy_select);
	}
	return 0xff;
}


static WRITE16_HANDLER( quiz365_coincounter_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (ACCESSING_BITS_0_7)
	{
		if (state->m_input_sel == 0x1c)
		{
			coin_counter_w(space->machine(), 0, ~data & 1);
			coin_counter_w(space->machine(), 1, ~data & 4);
		}
	}
}

/*
37,28,12    11      ->      88
67,4c,3a    ??      ->      51
*/
static READ16_HANDLER( quiz365_protection_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_quiz365_protection[0])
	{
		case 0x3a:
			return 0x0051;
		default:
			return 0x0088;
	}
}

static WRITE16_HANDLER( quiz365_protection_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	COMBINE_DATA(state->m_quiz365_protection + offset);
}

static ADDRESS_MAP_START( quiz365_map, AS_PROGRAM, 16, dynax_state )
	AM_RANGE(0x000000, 0x17ffff) AM_ROM													// ROM

	AM_RANGE(0x200000, 0x2003ff) AM_WRITE_LEGACY(ddenlovr_palette_w)							// Palette

	AM_RANGE(0x200c02, 0x200c03) AM_READ_LEGACY(quiz365_protection_r)							// Protection
	AM_RANGE(0x200e0a, 0x200e0d) AM_WRITE_LEGACY(quiz365_protection_w)							// Protection
//  AM_RANGE(0x201000, 0x2017ff) AM_WRITEONLY                                      // ?

	AM_RANGE(0x300200, 0x300201) AM_WRITE_LEGACY(ddenlovr_select2_16_w)
	AM_RANGE(0x300202, 0x300203) AM_WRITE_LEGACY(quiz365_coincounter_w)						// Coin Counters + more stuff written on startup
	AM_RANGE(0x300204, 0x300207) AM_READ_LEGACY(quiz365_input2_r)								//

	AM_RANGE(0x300240, 0x300247) AM_WRITE_LEGACY(ddenlovr16_palette_base_w)
	AM_RANGE(0x300248, 0x30024f) AM_WRITE_LEGACY(ddenlovr16_palette_mask_w)
	AM_RANGE(0x300250, 0x300257) AM_WRITE_LEGACY(ddenlovr16_transparency_pen_w)
	AM_RANGE(0x300258, 0x30025f) AM_WRITE_LEGACY(ddenlovr16_transparency_mask_w)
	AM_RANGE(0x300268, 0x300269) AM_WRITE_LEGACY(ddenlovr16_bgcolor_w)
	AM_RANGE(0x30026a, 0x30026b) AM_WRITE_LEGACY(ddenlovr16_priority_w)
	AM_RANGE(0x30026c, 0x30026d) AM_WRITE_LEGACY(ddenlovr16_layer_enable_w)
	AM_RANGE(0x300270, 0x300271) AM_READ_LEGACY(unk16_r)										// ? must be 78 on startup (not necessary in ddlover)
	AM_RANGE(0x300280, 0x300283) AM_WRITE_LEGACY(ddenlovr_blitter_w)
	AM_RANGE(0x300286, 0x300287) AM_READ_LEGACY(ddenlovr_gfxrom_r)								// Video Chip

	AM_RANGE(0x3002c0, 0x3002c1) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)// Sound
	AM_RANGE(0x300300, 0x300303) AM_DEVWRITE8_LEGACY("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0x300340, 0x30035f) AM_DEVREADWRITE8("rtc", msm6242_device, read, write,0x00ff)
	AM_RANGE(0x300380, 0x300383) AM_DEVWRITE8_LEGACY("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x300384, 0x300385) AM_DEVREAD8_LEGACY("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0x3003c2, 0x3003c3) AM_DEVWRITE_LEGACY("oki", quiz365_oki_bank1_w)
	AM_RANGE(0x3003ca, 0x3003cb) AM_WRITE_LEGACY(ddenlovr_blitter_irq_ack_w)					// Blitter irq acknowledge
	AM_RANGE(0x3003cc, 0x3003cd) AM_DEVWRITE_LEGACY("oki", quiz365_oki_bank2_w)

	AM_RANGE(0xff0000, 0xffffff) AM_RAM													// RAM
ADDRESS_MAP_END


static READ16_HANDLER( ddenlovj_dsw_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT16 dsw = 0;
	if ((~*state->m_dsw_sel16) & 0x01)	dsw |= input_port_read(space->machine(), "DSW1");
	if ((~*state->m_dsw_sel16) & 0x02)	dsw |= input_port_read(space->machine(), "DSW2");
	if ((~*state->m_dsw_sel16) & 0x04)	dsw |= input_port_read(space->machine(), "DSW3");
	return dsw;
}

static WRITE16_HANDLER( ddenlovj_coincounter_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine(), 0, data & 0x01);
		coin_counter_w(space->machine(), 1, data & 0x04);
		//                data & 0x80 ?
	}
}

static CUSTOM_INPUT( ddenlovj_blitter_r )
{
	dynax_state *state = field.machine().driver_data<dynax_state>();
	return state->m_ddenlovr_blitter_irq_flag ? 0x03 : 0x00;		// bit 4 = 1 -> blitter busy
}

static ADDRESS_MAP_START( ddenlovj_map, AS_PROGRAM, 16, dynax_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM // ROM

	AM_RANGE(0x200000, 0x2003ff) AM_WRITE_LEGACY(ddenlovr_palette_w)							// Palette
//  AM_RANGE(0x201000, 0x2017ff) AM_WRITEONLY                                      // ? B0 on startup, then 00

	AM_RANGE(0x300040, 0x300047) AM_WRITE_LEGACY(ddenlovr16_palette_base_w)
	AM_RANGE(0x300048, 0x30004f) AM_WRITE_LEGACY(ddenlovr16_palette_mask_w)
	AM_RANGE(0x300050, 0x300057) AM_WRITE_LEGACY(ddenlovr16_transparency_pen_w)
	AM_RANGE(0x300058, 0x30005f) AM_WRITE_LEGACY(ddenlovr16_transparency_mask_w)
	AM_RANGE(0x300068, 0x300069) AM_WRITE_LEGACY(ddenlovr16_bgcolor_w)
	AM_RANGE(0x30006a, 0x30006b) AM_WRITE_LEGACY(ddenlovr16_priority_w)
	AM_RANGE(0x30006c, 0x30006d) AM_WRITE_LEGACY(ddenlovr16_layer_enable_w)
	AM_RANGE(0x300070, 0x300071) AM_READ_LEGACY(unk16_r)										// ? must be 78 on startup (not necessary in ddlover)
	AM_RANGE(0x300080, 0x300083) AM_WRITE_LEGACY(ddenlovr_blitter_w)
	AM_RANGE(0x300086, 0x300087) AM_READ_LEGACY(ddenlovr_gfxrom_r)								// Video Chip
	AM_RANGE(0x3000c0, 0x3000c3) AM_DEVWRITE8_LEGACY("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0x300100, 0x30011f) AM_DEVREADWRITE8("rtc", msm6242_device, read, write,0x00ff)
	AM_RANGE(0x300140, 0x300143) AM_DEVWRITE8_LEGACY("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x300180, 0x300181) AM_READ_PORT("P1")
	AM_RANGE(0x300182, 0x300183) AM_READ_PORT("P2")
	AM_RANGE(0x300184, 0x300185) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300186, 0x300187) AM_READ_LEGACY(ddenlovj_dsw_r)								// DSW
	AM_RANGE(0x300188, 0x300189) AM_WRITE_LEGACY(ddenlovj_coincounter_w)						// Coin Counters
	AM_RANGE(0x30018a, 0x30018b) AM_WRITEONLY AM_BASE(m_dsw_sel16)			// DSW select
	AM_RANGE(0x30018c, 0x30018d) AM_DEVWRITE_LEGACY("oki", ddenlovr_oki_bank_w)
	AM_RANGE(0x3001ca, 0x3001cb) AM_WRITE_LEGACY(ddenlovr_blitter_irq_ack_w)					// Blitter irq acknowledge
	AM_RANGE(0x300240, 0x300241) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)// Sound
	AM_RANGE(0xff0000, 0xffffff) AM_RAM													// RAM
ADDRESS_MAP_END


static READ16_HANDLER( ddenlovrk_protection1_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	switch (*state->m_protection1)
	{
		case 0x007e:	return 0x00aa;
	}
	return *state->m_protection1;
}

static READ16_HANDLER( ddenlovrk_protection2_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	switch (*state->m_protection1)
	{
		case 0x0000:	return *state->m_protection2;
	}
	return 0x80;
}
static WRITE16_HANDLER( ddenlovrk_protection2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	COMBINE_DATA(state->m_protection2);
	state->m_oki->set_bank_base(((*state->m_protection2) & 0x7) * 0x40000);
}

static ADDRESS_MAP_START( ddenlovrk_map, AS_PROGRAM, 16, dynax_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM														// ROM

	AM_RANGE(0x100000, 0x100001) AM_RAM_READ_LEGACY(ddenlovrk_protection1_r) AM_BASE(m_protection1)
	AM_RANGE(0x200000, 0x200001) AM_READWRITE_LEGACY(ddenlovrk_protection2_r, ddenlovrk_protection2_w) AM_BASE(m_protection2)

	AM_RANGE(0xd00000, 0xd003ff) AM_WRITE_LEGACY(ddenlovr_palette_w)								// Palette
//  AM_RANGE(0xd01000, 0xd017ff) AM_RAM                                                    // ? B0 on startup, then 00

	AM_RANGE(0xe00040, 0xe00047) AM_WRITE_LEGACY(ddenlovr16_palette_base_w)
	AM_RANGE(0xe00048, 0xe0004f) AM_WRITE_LEGACY(ddenlovr16_palette_mask_w)
	AM_RANGE(0xe00050, 0xe00057) AM_WRITE_LEGACY(ddenlovr16_transparency_pen_w)
	AM_RANGE(0xe00058, 0xe0005f) AM_WRITE_LEGACY(ddenlovr16_transparency_mask_w)
	AM_RANGE(0xe00068, 0xe00069) AM_WRITE_LEGACY(ddenlovr16_bgcolor_w)
	AM_RANGE(0xe0006a, 0xe0006b) AM_WRITE_LEGACY(ddenlovr16_priority_w)
	AM_RANGE(0xe0006c, 0xe0006d) AM_WRITE_LEGACY(ddenlovr16_layer_enable_w)
	AM_RANGE(0xe00070, 0xe00071) AM_READNOP
	AM_RANGE(0xe00080, 0xe00083) AM_WRITE_LEGACY(ddenlovr_blitter_w)
	AM_RANGE(0xe00086, 0xe00087) AM_READ_LEGACY(ddenlovr_gfxrom_r)									// Video Chip

	AM_RANGE(0xe00100, 0xe00101) AM_READ_PORT("P1")
	AM_RANGE(0xe00102, 0xe00103) AM_READ_PORT("P2")
	AM_RANGE(0xe00104, 0xe00105) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe00200, 0xe00201) AM_READ_PORT("DSW")
	AM_RANGE(0xe00302, 0xe00303) AM_WRITE_LEGACY(ddenlovr_blitter_irq_ack_w)						// Blitter irq acknowledge
	AM_RANGE(0xe00308, 0xe00309) AM_WRITE_LEGACY(ddenlovr_coincounter_0_w)							// Coin Counters
	AM_RANGE(0xe0030c, 0xe0030d) AM_WRITE_LEGACY(ddenlovr_coincounter_1_w)							//

	AM_RANGE(0xe00400, 0xe00403) AM_DEVWRITE8_LEGACY("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0xe00500, 0xe0051f) AM_DEVREADWRITE8("rtc", msm6242_device, read, write,0x00ff)
	AM_RANGE(0xe00600, 0xe00603) AM_DEVWRITE8_LEGACY("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0xe00604, 0xe00605) AM_DEVREAD8_LEGACY("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0xe00700, 0xe00701) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)	// Sound
	AM_RANGE(0xff0000, 0xffffff) AM_RAM														// RAM
ADDRESS_MAP_END




static ADDRESS_MAP_START( ddenlovr_map, AS_PROGRAM, 16, dynax_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM														// ROM

	AM_RANGE(0x300000, 0x300001) AM_DEVWRITE_LEGACY("oki", ddenlovr_oki_bank_w)

	AM_RANGE(0xd00000, 0xd003ff) AM_WRITE_LEGACY(ddenlovr_palette_w)								// Palette
//  AM_RANGE(0xd01000, 0xd017ff) AM_RAM                                                   // ? B0 on startup, then 00

	AM_RANGE(0xe00040, 0xe00047) AM_WRITE_LEGACY(ddenlovr16_palette_base_w)
	AM_RANGE(0xe00048, 0xe0004f) AM_WRITE_LEGACY(ddenlovr16_palette_mask_w)
	AM_RANGE(0xe00050, 0xe00057) AM_WRITE_LEGACY(ddenlovr16_transparency_pen_w)
	AM_RANGE(0xe00058, 0xe0005f) AM_WRITE_LEGACY(ddenlovr16_transparency_mask_w)
	AM_RANGE(0xe00068, 0xe00069) AM_WRITE_LEGACY(ddenlovr16_bgcolor_w)
	AM_RANGE(0xe0006a, 0xe0006b) AM_WRITE_LEGACY(ddenlovr16_priority_w)
	AM_RANGE(0xe0006c, 0xe0006d) AM_WRITE_LEGACY(ddenlovr16_layer_enable_w)
	AM_RANGE(0xe00070, 0xe00071) AM_READ_LEGACY(unk16_r)											// ? must be 78 on startup (not necessary in ddlover)
	AM_RANGE(0xe00080, 0xe00083) AM_WRITE_LEGACY(ddenlovr_blitter_w)
	AM_RANGE(0xe00086, 0xe00087) AM_READ_LEGACY(ddenlovr_gfxrom_r)									// Video Chip

	AM_RANGE(0xe00100, 0xe00101) AM_READ_PORT("P1")
	AM_RANGE(0xe00102, 0xe00103) AM_READ_PORT("P2")
	AM_RANGE(0xe00104, 0xe00105) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe00200, 0xe00201) AM_READ_PORT("DSW")
	AM_RANGE(0xe00302, 0xe00303) AM_WRITE_LEGACY(ddenlovr_blitter_irq_ack_w)						// Blitter irq acknowledge
	AM_RANGE(0xe00308, 0xe00309) AM_WRITE_LEGACY(ddenlovr_coincounter_0_w)							// Coin Counters
	AM_RANGE(0xe0030c, 0xe0030d) AM_WRITE_LEGACY(ddenlovr_coincounter_1_w)							//

	AM_RANGE(0xe00400, 0xe00403) AM_DEVWRITE8_LEGACY("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0xe00500, 0xe0051f) AM_DEVREADWRITE8("rtc", msm6242_device, read, write,0x00ff)
	AM_RANGE(0xe00600, 0xe00603) AM_DEVWRITE8_LEGACY("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0xe00604, 0xe00605) AM_DEVREAD8_LEGACY("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0xe00700, 0xe00701) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)	// Sound
	AM_RANGE(0xff0000, 0xffffff) AM_RAM														// RAM
ADDRESS_MAP_END


static CUSTOM_INPUT( nettoqc_special_r )
{
	dynax_state *state = field.machine().driver_data<dynax_state>();
	return state->m_ddenlovr_blitter_irq_flag ? 0x03 : 0x00;
}

static READ16_HANDLER( nettoqc_input_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))	return input_port_read(space->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 1))	return input_port_read(space->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 2))	return input_port_read(space->machine(), "DSW3");
	return 0xffff;
}

/*
    Protection:

    Writes 37 28 12 to 200e0b then 11 to 200e0d. Expects to read 88 from 200c03
    Writes 67 4c 3a to 200e0b then 19 to 200e0d. Expects to read 51 from 200c03
*/

static READ16_HANDLER( nettoqc_protection_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_protection1[0] & 0xff)
	{
		case 0x3a:	return 0x0051;
		default:	return 0x0088;
	}
}

static WRITE16_HANDLER( nettoqc_coincounter_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine(), 0, data & 0x01);
		coin_counter_w(space->machine(), 1, data & 0x04);
		//                data & 0x80 ?
	}
}

static WRITE16_DEVICE_HANDLER( nettoqc_oki_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		okim6295_device *oki = downcast<okim6295_device *>(device);
		oki->set_bank_base((data & 3) * 0x40000);
	}
}

static ADDRESS_MAP_START( nettoqc_map, AS_PROGRAM, 16, dynax_state )
	AM_RANGE(0x000000, 0x17ffff) AM_ROM														// ROM

	AM_RANGE(0x200000, 0x2003ff) AM_WRITE_LEGACY(ddenlovr_palette_w)								// Palette
	AM_RANGE(0x200c02, 0x200c03) AM_READ_LEGACY(nettoqc_protection_r)								//
	AM_RANGE(0x200e0a, 0x200e0d) AM_WRITEONLY AM_BASE(m_protection1)			//
	AM_RANGE(0x201000, 0x2017ff) AM_WRITEONLY											// ?

	AM_RANGE(0x300040, 0x300047) AM_WRITE_LEGACY(ddenlovr16_palette_base_w)
	AM_RANGE(0x300048, 0x30004f) AM_WRITE_LEGACY(ddenlovr16_palette_mask_w)
	AM_RANGE(0x300050, 0x300057) AM_WRITE_LEGACY(ddenlovr16_transparency_pen_w)
	AM_RANGE(0x300058, 0x30005f) AM_WRITE_LEGACY(ddenlovr16_transparency_mask_w)
	AM_RANGE(0x300068, 0x300069) AM_WRITE_LEGACY(ddenlovr16_bgcolor_w)
	AM_RANGE(0x30006a, 0x30006b) AM_WRITE_LEGACY(ddenlovr16_priority_w)
	AM_RANGE(0x30006c, 0x30006d) AM_WRITE_LEGACY(ddenlovr16_layer_enable_w)
	AM_RANGE(0x300070, 0x300071) AM_READ_LEGACY(unk16_r)											// ? must be 78 on startup (not necessary in ddlover)
	AM_RANGE(0x300080, 0x300083) AM_WRITE_LEGACY(ddenlovr_blitter_w)
	AM_RANGE(0x300086, 0x300087) AM_READ_LEGACY(ddenlovr_gfxrom_r)									// Video Chip
	AM_RANGE(0x3000c0, 0x3000c3) AM_DEVWRITE8_LEGACY("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0x300100, 0x30011f) AM_DEVREADWRITE8("rtc", msm6242_device, read, write,0x00ff)
	AM_RANGE(0x300140, 0x300143) AM_DEVWRITE8_LEGACY("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x300180, 0x300181) AM_READ_PORT("P1")
	AM_RANGE(0x300182, 0x300183) AM_READ_PORT("P2")
	AM_RANGE(0x300184, 0x300185) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300186, 0x300187) AM_READ_LEGACY(nettoqc_input_r)									// DSW's
	AM_RANGE(0x300188, 0x300189) AM_WRITE_LEGACY(nettoqc_coincounter_w)							// Coin Counters
	AM_RANGE(0x30018a, 0x30018b) AM_WRITE_LEGACY(ddenlovr_select_16_w)								//
	AM_RANGE(0x30018c, 0x30018d) AM_DEVWRITE_LEGACY("oki", nettoqc_oki_bank_w)
	AM_RANGE(0x3001ca, 0x3001cb) AM_WRITE_LEGACY(ddenlovr_blitter_irq_ack_w)						// Blitter irq acknowledge
	AM_RANGE(0x300240, 0x300241) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)	// Sound
	AM_RANGE(0xff0000, 0xffffff) AM_RAM														// RAM
ADDRESS_MAP_END


/***************************************************************************
                                Rong Rong
***************************************************************************/

static READ8_HANDLER( rongrong_input_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))	return input_port_read(space->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 1))	return input_port_read(space->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 2))	return 0xff;//space->machine().rand();
	if (!BIT(state->m_dsw_sel, 3))	return 0xff;//space->machine().rand();
	if (!BIT(state->m_dsw_sel, 4))	return input_port_read(space->machine(), "DSW3");
	return 0xff;
}

static WRITE8_HANDLER( rongrong_select_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

//logerror("%04x: rongrong_select_w %02x\n",cpu_get_pc(&space->device()),data);

	/* bits 0-4 = **both** ROM bank **AND** input select */
	memory_set_bank(space->machine(), "bank1", data & 0x1f);
	state->m_dsw_sel = data;

	/* bits 5-7 = RAM bank */
	memory_set_bank(space->machine(), "bank2", ((data & 0xe0) >> 5));
}


static ADDRESS_MAP_START( quizchq_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM											// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM											// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")									// RAM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1") AM_WRITE_LEGACY(rongrong_palette_w)		// ROM (Banked)
ADDRESS_MAP_END

static ADDRESS_MAP_START( quizchq_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(rongrong_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1b, 0x1b) AM_READWRITE_LEGACY(rongrong_blitter_busy_r, rongrong_blitter_busy_w)

	AM_RANGE(0x1c, 0x1c) AM_READ_LEGACY(rongrong_input_r)
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(rongrong_select_w)
	AM_RANGE(0x20, 0x20) AM_WRITE_LEGACY(ddenlovr_select2_w)
	AM_RANGE(0x22, 0x23) AM_READ_LEGACY(rongrong_input2_r)

	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x60, 0x61) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)

	AM_RANGE(0x80, 0x83) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x84, 0x87) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x88, 0x8b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x8c, 0x8f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x94, 0x94) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x95, 0x95) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x96, 0x96) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x98, 0x98) AM_READ_LEGACY(unk_r)							// ? must be 78 on startup

	AM_RANGE(0xa0, 0xaf) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0xc0, 0xc0) AM_DEVWRITE_LEGACY("oki", quizchq_oki_bank_w)
	AM_RANGE(0xc2, 0xc2) AM_WRITENOP						// enables palette RAM at 8000
ADDRESS_MAP_END



static ADDRESS_MAP_START( rongrong_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM											// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM											// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")									// RAM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1") AM_WRITE_LEGACY(rongrong_palette_w)		// ROM (Banked)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rongrong_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(rongrong_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1b, 0x1b) AM_READWRITE_LEGACY(rongrong_blitter_busy_r, rongrong_blitter_busy_w)

	AM_RANGE(0x1c, 0x1c) AM_READ_LEGACY(rongrong_input_r)
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(rongrong_select_w)

	AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x60, 0x61) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)

	AM_RANGE(0x80, 0x83) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x84, 0x87) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x88, 0x8b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x8c, 0x8f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x94, 0x94) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x95, 0x95) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x96, 0x96) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x98, 0x98) AM_READ_LEGACY(unk_r)									// ? must be 78 on startup

	AM_RANGE(0xa0, 0xa0) AM_WRITE_LEGACY(ddenlovr_select2_w)
	AM_RANGE(0xa2, 0xa3) AM_READ_LEGACY(rongrong_input2_r)
	AM_RANGE(0xc2, 0xc2) AM_WRITENOP									// enables palette RAM at f000, and protection device at f705/f706/f601
ADDRESS_MAP_END
/*
1e input select,1c input read
    3e=dsw1 3d=dsw2
a0 input select,a2 input read (protection?)
    0=? 1=? 2=coins(from a3)
*/


/***************************************************************************
                                Monkey Mole Panic
***************************************************************************/


static READ8_HANDLER( magic_r )
{
	return 0x01;
}

static WRITE8_HANDLER( mmpanic_rombank_w )
{
	memory_set_bank(space->machine(), "bank1", data & 0x7);
	/* Bit 4? */
}

static WRITE8_HANDLER( mmpanic_soundlatch_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	state->soundlatch_w(*space, 0, data);
	device_set_input_line(state->m_soundcpu, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( mmpanic_blitter_w )
{
	blitter_w(space, 0, offset, data, 0xdf);	// RST 18
}
static WRITE8_HANDLER( mmpanic_blitter2_w )
{
	blitter_w(space, 1, offset, data, 0xdf);	// RST 18
}

static void mmpanic_update_leds(running_machine &machine)
{
	dynax_state *state = machine.driver_data<dynax_state>();
	set_led_status(machine, 0, state->m_mmpanic_leds);
}

/* leds 1-8 */
static WRITE8_HANDLER( mmpanic_leds_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_mmpanic_leds = (state->m_mmpanic_leds & 0xff00) | data;
	mmpanic_update_leds(space->machine());
}
/* led 9 */
static WRITE8_HANDLER( mmpanic_leds2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_mmpanic_leds = (state->m_mmpanic_leds & 0xfeff) | (data ? 0x0100 : 0);
	mmpanic_update_leds(space->machine());
}


static WRITE8_HANDLER( mmpanic_lockout_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (state->m_dsw_sel == 0x0c)
	{
		coin_counter_w(space->machine(), 0, (~data) & 0x01);
		coin_lockout_w(space->machine(), 0, (~data) & 0x02);
		set_led_status(space->machine(), 1, (~data) & 0x04);
	}
}

static READ8_HANDLER( mmpanic_link_r )	{ return 0xff; }

/* Main CPU */

static ADDRESS_MAP_START( mmpanic_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0051, 0x0051) AM_READ_LEGACY(magic_r)								// ?
	AM_RANGE(0x0000, 0x5fff) AM_ROM											// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM											// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")									// RAM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1") AM_WRITE_LEGACY(rongrong_palette_w)		// ROM (Banked)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mmpanic_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)

	// Layers 0-3:
	AM_RANGE(0x20, 0x23) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x24, 0x27) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x28, 0x2b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x2c, 0x2f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x34, 0x34) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x35, 0x35) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x36, 0x36) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x38, 0x38) AM_READ_LEGACY(unk_r)				// ? must be 78 on startup

	// Layers 4-7:
	AM_RANGE(0x40, 0x43) AM_WRITE_LEGACY(ddenlovr_palette_base2_w)
	AM_RANGE(0x44, 0x47) AM_WRITE_LEGACY(ddenlovr_palette_mask2_w)
	AM_RANGE(0x48, 0x4b) AM_WRITE_LEGACY(ddenlovr_transparency_pen2_w)
	AM_RANGE(0x4c, 0x4f) AM_WRITE_LEGACY(ddenlovr_transparency_mask2_w)
	AM_RANGE(0x54, 0x54) AM_WRITE_LEGACY(ddenlovr_bgcolor2_w)
	AM_RANGE(0x55, 0x55) AM_WRITE_LEGACY(ddenlovr_priority2_w)
	AM_RANGE(0x56, 0x56) AM_WRITE_LEGACY(ddenlovr_layer_enable2_w)
	AM_RANGE(0x58, 0x58) AM_READ_LEGACY(unk_r)				// ? must be 78 on startup

	AM_RANGE(0x60, 0x61) AM_WRITE_LEGACY(mmpanic_blitter_w)
	AM_RANGE(0x63, 0x63) AM_READ_LEGACY(rongrong_gfxrom_r)	// Video Chip
	AM_RANGE(0x64, 0x65) AM_WRITE_LEGACY(mmpanic_blitter2_w)
	AM_RANGE(0x68, 0x68) AM_DEVWRITE_LEGACY("aysnd", ddenlovr_select_w)
	AM_RANGE(0x69, 0x69) AM_WRITE_LEGACY(mmpanic_lockout_w)
	AM_RANGE(0x6a, 0x6a) AM_READ_PORT("IN0")
	AM_RANGE(0x6b, 0x6b) AM_READ_PORT("IN1")
	AM_RANGE(0x6c, 0x6d) AM_READ_LEGACY(mmpanic_link_r)	// Other cabinets?
	AM_RANGE(0x74, 0x74) AM_WRITE_LEGACY(mmpanic_rombank_w)
	AM_RANGE(0x78, 0x78) AM_WRITENOP				// 0, during RST 08 (irq acknowledge?)
	AM_RANGE(0x7c, 0x7c) AM_DEVREADWRITE("oki", okim6295_device, read, write)	// Sound
	AM_RANGE(0x8c, 0x8c) AM_WRITE_LEGACY(mmpanic_soundlatch_w)	//
	AM_RANGE(0x88, 0x88) AM_WRITE_LEGACY(mmpanic_leds_w)		// Leds
	AM_RANGE(0x90, 0x90) AM_WRITENOP				// written just before port 8c
	AM_RANGE(0x94, 0x94) AM_READ_PORT("DSW1")
	AM_RANGE(0x98, 0x98) AM_READ_PORT("DSW2")
	AM_RANGE(0x9c, 0x9c) AM_READ_PORT("DSW3")		// DSW 1&2 high bits
	AM_RANGE(0xa6, 0xa6) AM_WRITE_LEGACY(mmpanic_leds2_w)		//
ADDRESS_MAP_END

/* Sound CPU */

static ADDRESS_MAP_START( mmpanic_sound_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM	// ROM
	AM_RANGE(0x6000, 0x66ff) AM_RAM	// RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM	// ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mmpanic_sound_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_r)
	AM_RANGE(0x02, 0x02) AM_READNOP		// read just before port 00
	AM_RANGE(0x04, 0x04) AM_NOP					// read only once at the start
	AM_RANGE(0x06, 0x06) AM_WRITENOP	// almost always 1, sometimes 0
	AM_RANGE(0x08, 0x09) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x0e, 0x0e) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
ADDRESS_MAP_END



/***************************************************************************
                            The First Funky Fighter
***************************************************************************/

/* Main CPU */

static ADDRESS_MAP_START( funkyfig_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")			// RAM (Banked)

	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x81ff) AM_WRITE_LEGACY(rongrong_palette_w)
	AM_RANGE(0x8400, 0x87ff) AM_WRITENOP
ADDRESS_MAP_END


static READ8_HANDLER( funkyfig_busy_r )
{
					// bit 0 ?
	return 0x00;	// bit 7 = blitter busy
}

static WRITE8_HANDLER( funkyfig_blitter_w )
{
	blitter_w_funkyfig(space->machine(), 0, offset, data, 0xe0);
}

static WRITE8_HANDLER( funkyfig_rombank_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	state->m_dsw_sel = data;

	memory_set_bank(space->machine(), "bank1", data & 0x0f);
	// bit 4 selects palette ram at 8000?
	memory_set_bank(space->machine(), "bank2", ((data & 0xe0) >> 5));
}

static READ8_HANDLER( funkyfig_dsw_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))  return input_port_read(space->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 1))  return input_port_read(space->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 2))  return input_port_read(space->machine(), "DSW3");
	logerror("%06x: warning, unknown bits read, ddenlovr_select = %02x\n", cpu_get_pc(&space->device()), state->m_dsw_sel);
	return 0xff;
}

static READ8_HANDLER( funkyfig_coin_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x22:	return input_port_read(space->machine(), "IN2");
		case 0x23:	return state->m_funkyfig_lockout;
	}
	logerror("%06x: warning, unknown bits read, ddenlovr_select2 = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel);
	return 0xff;
}

static READ8_HANDLER( funkyfig_key_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x20:	return input_port_read(space->machine(), "IN0");
		case 0x21:	return input_port_read(space->machine(), "IN1");
	}
	logerror("%06x: warning, unknown bits read, ddenlovr_select2 = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel);
	return 0xff;
}

static WRITE8_HANDLER( funkyfig_lockout_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x2c:
			state->m_funkyfig_lockout = data;
			coin_counter_w(space->machine(), 0,   data  & 0x01);
			coin_lockout_w(space->machine(), 0, (~data) & 0x02);
			if (data & ~0x03)
				logerror("%06x: warning, unknown bits written, lockout = %02x\n", cpu_get_pc(&space->device()), data);
			break;

//      case 0xef:  16 bytes on startup

		default:
			logerror("%06x: warning, unknown bits written, ddenlovr_select2 = %02x, data = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel, data);
	}
}

static ADDRESS_MAP_START( funkyfig_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("oki", okim6295_device, read, write)	// Sound
	AM_RANGE(0x01, 0x01) AM_WRITE_LEGACY(mmpanic_leds_w)		// Leds
	AM_RANGE(0x02, 0x02) AM_WRITE_LEGACY(mmpanic_soundlatch_w)	//
	AM_RANGE(0x04, 0x04) AM_READ_LEGACY(funkyfig_busy_r)
	AM_RANGE(0x1c, 0x1c) AM_READ_LEGACY(funkyfig_dsw_r)
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(funkyfig_rombank_w)
	AM_RANGE(0x20, 0x21) AM_WRITE_LEGACY(funkyfig_blitter_w)
	AM_RANGE(0x23, 0x23) AM_READ_LEGACY(rongrong_gfxrom_r)		// Video Chip
	AM_RANGE(0x40, 0x4f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)

	// Layers 0-3:
	AM_RANGE(0x60, 0x63) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x64, 0x67) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x68, 0x6b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x6c, 0x6f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x74, 0x74) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x75, 0x75) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x76, 0x76) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x78, 0x78) AM_READ_LEGACY(unk_r)					// ? must be 78 on startup

	AM_RANGE(0x80, 0x80) AM_WRITE_LEGACY(ddenlovr_select2_w)
	AM_RANGE(0x81, 0x81) AM_WRITE_LEGACY(funkyfig_lockout_w)
	AM_RANGE(0x82, 0x82) AM_READ_LEGACY(funkyfig_coin_r)
	AM_RANGE(0x83, 0x83) AM_READ_LEGACY(funkyfig_key_r)

	AM_RANGE(0xa2, 0xa2) AM_WRITE_LEGACY(mmpanic_leds2_w)
ADDRESS_MAP_END


/* Sound CPU */

static ADDRESS_MAP_START( funkyfig_sound_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READ(soundlatch_r)
	AM_RANGE(0x04, 0x04) AM_READNOP	// read only once at the start
ADDRESS_MAP_END



/***************************************************************************

    Hana Kanzashi

***************************************************************************/

static WRITE8_HANDLER( hanakanz_rombank_w )
{
	memory_set_bank(space->machine(), "bank1", data & 0x0f);
	memory_set_bank(space->machine(), "bank2", ((data & 0xf0) >> 4));
}

static ADDRESS_MAP_START( hanakanz_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM								// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM								// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")						// RAM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")						// ROM (Banked)
ADDRESS_MAP_END


static WRITE8_HANDLER( hanakanz_keyb_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_keyb = data;
}

static WRITE8_HANDLER( hanakanz_dsw_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_dsw_sel = data;
}

static READ8_HANDLER( hanakanz_keyb_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	UINT8 val = 0xff;

	if      (!BIT(state->m_keyb, 0))   val = input_port_read(space->machine(), offset ? "KEY5" : "KEY0");
	else if (!BIT(state->m_keyb, 1))   val = input_port_read(space->machine(), offset ? "KEY6" : "KEY1");
	else if (!BIT(state->m_keyb, 2))   val = input_port_read(space->machine(), offset ? "KEY7" : "KEY2");
	else if (!BIT(state->m_keyb, 3))   val = input_port_read(space->machine(), offset ? "KEY8" : "KEY3");
	else if (!BIT(state->m_keyb, 4))   val = input_port_read(space->machine(), offset ? "KEY9" : "KEY4");

	val |= input_port_read(space->machine(), offset ? "HOPPER" : "BET");
	return val;
}

static READ8_HANDLER( hanakanz_dsw_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))   return input_port_read(space->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 1))   return input_port_read(space->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 2))   return input_port_read(space->machine(), "DSW3");
	if (!BIT(state->m_dsw_sel, 3))   return input_port_read(space->machine(), "DSW4");
	if (!BIT(state->m_dsw_sel, 4))   return input_port_read(space->machine(), "DSW5");
	return 0xff;
}

static READ8_HANDLER( hanakanz_busy_r )
{
	return 0x80;	// bit 7 == 0 -> blitter busy
}

static READ8_HANDLER( hanakanz_gfxrom_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 *rom  = space->machine().region("blitter")->base();
	size_t size = space->machine().region("blitter")->bytes();
	int address = (state->m_ddenlovr_blit_address & 0xffffff) * 2;

	if (address >= size)
	{
		logerror("CPU#0 PC %06X: Error, Blitter address %06X out of range\n", cpu_get_pc(&space->device()), address);
		address %= size;
	}

	if (offset == 0)
	{
		state->m_romdata[0] = rom[address + 0];
		state->m_romdata[1] = rom[address + 1];

		state->m_ddenlovr_blit_address = (state->m_ddenlovr_blit_address + 1) & 0xffffff;

		return state->m_romdata[0];
	}
	else
	{
		return state->m_romdata[1];
	}
}


static WRITE8_HANDLER( hanakanz_coincounter_w )
{
	// bit 0 = coin counter
	// bit 1 = out counter
	// bit 2 = 1 if bet on
	// bit 3 = 1 if bet off

	coin_counter_w(space->machine(), 0, data & 1);

	if (data & 0xf0)
		logerror("%04x: warning, coin counter = %02x\n", cpu_get_pc(&space->device()), data);

#ifdef MAME_DEBUG
//      popmessage("93 = %02x", data);
#endif
}

static WRITE8_HANDLER( hanakanz_palette_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (state->m_ddenlovr_blit_latch & 0x80)
	{
		state->m_palette_index = data | ((state->m_ddenlovr_blit_latch & 1) << 8);
	}
	else
	{
		// 0bbggggg bbbrrrrr
		// 04343210 21043210

		int g = state->m_ddenlovr_blit_latch & 0x1f;
		int r = data & 0x1f;
		int b = ((data & 0xe0) >> 5) | ((state->m_ddenlovr_blit_latch & 0x60) >> 2);
		palette_set_color_rgb(space->machine(), (state->m_palette_index++) & 0x1ff, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

static WRITE8_DEVICE_HANDLER( hanakanz_oki_bank_w )
{
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base((data & 0x40) ? 0x40000 : 0);
}

static READ8_HANDLER( hanakanz_rand_r )
{
	return space->machine().rand();
}

static ADDRESS_MAP_START( hanakanz_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x2c, 0x2c) AM_READ_LEGACY(hanakanz_busy_r) AM_DEVWRITE_LEGACY("oki", hanakanz_oki_bank_w)
	AM_RANGE(0x2e, 0x2e) AM_WRITE_LEGACY(hanakanz_blitter_reg_w)
	AM_RANGE(0x30, 0x30) AM_WRITE_LEGACY(hanakanz_rombank_w)
	AM_RANGE(0x31, 0x31) AM_WRITE_LEGACY(hanakanz_dsw_w)
	AM_RANGE(0x32, 0x32) AM_READ_LEGACY(hanakanz_dsw_r)
	AM_RANGE(0x80, 0x80) AM_WRITE_LEGACY(hanakanz_blitter_data_w)
	AM_RANGE(0x81, 0x81) AM_WRITE_LEGACY(hanakanz_palette_w)
	AM_RANGE(0x83, 0x84) AM_READ_LEGACY(hanakanz_gfxrom_r)
	AM_RANGE(0x90, 0x90) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x91, 0x92) AM_READ_LEGACY(hanakanz_keyb_r)
	AM_RANGE(0x93, 0x93) AM_WRITE_LEGACY(hanakanz_coincounter_w)
	AM_RANGE(0x94, 0x94) AM_WRITE_LEGACY(hanakanz_keyb_w)
	AM_RANGE(0x96, 0x96) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xe0, 0xef) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( hkagerou_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x2c, 0x2c) AM_READ_LEGACY(hanakanz_busy_r) AM_DEVWRITE_LEGACY("oki", hanakanz_oki_bank_w)
	AM_RANGE(0x2e, 0x2e) AM_WRITE_LEGACY(hanakanz_blitter_reg_w)
	AM_RANGE(0x30, 0x30) AM_WRITE_LEGACY(hanakanz_rombank_w)
	AM_RANGE(0x31, 0x31) AM_WRITE_LEGACY(hanakanz_dsw_w)
	AM_RANGE(0x32, 0x32) AM_READ_LEGACY(hanakanz_dsw_r)
	AM_RANGE(0x80, 0x80) AM_WRITE_LEGACY(hanakanz_blitter_data_w)
	AM_RANGE(0x81, 0x81) AM_WRITE_LEGACY(hanakanz_palette_w)
	AM_RANGE(0x83, 0x84) AM_READ_LEGACY(hanakanz_gfxrom_r)
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0xb0, 0xb0) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb1, 0xb2) AM_READ_LEGACY(hanakanz_keyb_r)
	AM_RANGE(0xb3, 0xb3) AM_WRITE_LEGACY(hanakanz_coincounter_w)
	AM_RANGE(0xb4, 0xb4) AM_WRITE_LEGACY(hanakanz_keyb_w)
	AM_RANGE(0xb6, 0xb6) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xe0, 0xef) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
ADDRESS_MAP_END


static WRITE8_HANDLER( mjreach1_protection_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_prot_val = data;
}

static READ8_HANDLER( mjreach1_protection_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	return state->m_prot_val;
}

static ADDRESS_MAP_START( mjreach1_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x2c, 0x2c) AM_READ_LEGACY(hanakanz_busy_r) AM_DEVWRITE_LEGACY("oki", hanakanz_oki_bank_w)
	AM_RANGE(0x2e, 0x2e) AM_WRITE_LEGACY(hanakanz_blitter_reg_w)
	AM_RANGE(0x30, 0x30) AM_WRITE_LEGACY(hanakanz_rombank_w)
	AM_RANGE(0x31, 0x31) AM_WRITE_LEGACY(hanakanz_dsw_w)
	AM_RANGE(0x32, 0x32) AM_READ_LEGACY(hanakanz_dsw_r)
	AM_RANGE(0x80, 0x80) AM_WRITE_LEGACY(hanakanz_blitter_data_w)
	AM_RANGE(0x81, 0x81) AM_WRITE_LEGACY(hanakanz_palette_w)
	AM_RANGE(0x83, 0x84) AM_READ_LEGACY(hanakanz_gfxrom_r)
	AM_RANGE(0x90, 0x90) AM_WRITE_LEGACY(hanakanz_keyb_w)
	AM_RANGE(0x92, 0x92) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0x93, 0x93) AM_READWRITE_LEGACY(mjreach1_protection_r, mjreach1_protection_w)
	AM_RANGE(0x94, 0x94) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x95, 0x96) AM_READ_LEGACY(hanakanz_keyb_r)
	AM_RANGE(0x97, 0x97) AM_WRITE_LEGACY(hanakanz_coincounter_w)
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xe0, 0xef) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
ADDRESS_MAP_END


/***************************************************************************
     Mahjong Chuukanejyo
***************************************************************************/

static READ8_HANDLER( mjchuuka_keyb_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 val = 0xff;

	if      (!BIT(state->m_keyb, 0))   val = input_port_read(space->machine(), offset ? "KEY5" : "KEY0");
	else if (!BIT(state->m_keyb, 1))   val = input_port_read(space->machine(), offset ? "KEY6" : "KEY1");
	else if (!BIT(state->m_keyb, 2))   val = input_port_read(space->machine(), offset ? "KEY7" : "KEY2");
	else if (!BIT(state->m_keyb, 3))   val = input_port_read(space->machine(), offset ? "KEY8" : "KEY3");
	else if (!BIT(state->m_keyb, 4))   val = input_port_read(space->machine(), offset ? "KEY9" : "KEY4");

	val |= input_port_read(space->machine(), offset ? "HOPPER" : "BET");

	if (offset)
		val |= 0x80;	// blitter busy

	return val;
}

static WRITE8_HANDLER( mjchuuka_blitter_w )
{
	hanakanz_blitter_reg_w(space, 0, offset >> 8);
	hanakanz_blitter_data_w(space, 0, data);
}

static void mjchuuka_get_romdata(running_machine &machine)
{
	dynax_state *state = machine.driver_data<dynax_state>();
	UINT8 *rom = machine.region("blitter")->base();
	size_t size = machine.region("blitter")->bytes();
	int address = (state->m_ddenlovr_blit_address & 0xffffff) * 2;

	if (address >= size)
	{
		logerror("%s: Error, Blitter address %06X out of range\n", machine.describe_context(), address);
		address %= size;
	}

	state->m_romdata[0] = rom[address + 0];
	state->m_romdata[1] = rom[address + 1];
}

static READ8_HANDLER( mjchuuka_gfxrom_0_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	mjchuuka_get_romdata(space->machine());
	state->m_ddenlovr_blit_address++;
	return state->m_romdata[0];
}

static READ8_HANDLER( mjchuuka_gfxrom_1_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	return state->m_romdata[1];
}

static WRITE8_HANDLER( mjchuuka_palette_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT16 rgb = (offset & 0xff00) | data;

	if (rgb & 0x8000)
	{
		state->m_palette_index = rgb & 0x1ff;
	}
	else
	{
		// 0bbggggg bbbrrrrr
		// 04343210 21043210

		int r = (rgb >> 0) & 0x1f;
		int g = (rgb >> 8) & 0x1f;
		int b = ((rgb >> 5) & 0x07) | ((rgb & 0x6000) >> 10);
		palette_set_color_rgb(space->machine(), (state->m_palette_index++) & 0x1ff, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

static WRITE8_HANDLER( mjchuuka_coincounter_w )
{
	// bit 0 = in counter
	// bit 1 = out counter
	// bit 3 = lockout
	// bit 8?

	coin_counter_w(space->machine(), 0,  data   & 0x01);
	coin_lockout_w(space->machine(), 0, (~data) & 0x08);

	if (data & 0x74)
		logerror("%04x: warning, coin counter = %02x\n", cpu_get_pc(&space->device()), data);

#ifdef MAME_DEBUG
//    popmessage("40 = %02x",data);
#endif
}

static WRITE8_DEVICE_HANDLER( mjchuuka_oki_bank_w )
{
	// data & 0x08 ?
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base((data & 0x01) ? 0x40000 : 0);

#ifdef MAME_DEBUG
//    popmessage("1e = %02x",data);
#endif
}

static ADDRESS_MAP_START( mjchuuka_portmap, AS_IO, 8, dynax_state ) 	// 16 bit I/O
	AM_RANGE(0x13, 0x13) AM_MIRROR(0xff00) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0x1c, 0x1c) AM_MIRROR(0xff00) AM_WRITE_LEGACY(hanakanz_rombank_w)
	AM_RANGE(0x1e, 0x1e) AM_MIRROR(0xff00) AM_DEVWRITE_LEGACY("oki", mjchuuka_oki_bank_w	)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0xff00) AM_MASK(0xff00) AM_WRITE_LEGACY(mjchuuka_blitter_w)
	AM_RANGE(0x21, 0x21) AM_MIRROR(0xff00) AM_MASK(0xff00) AM_WRITE_LEGACY(mjchuuka_palette_w)
	AM_RANGE(0x23, 0x23) AM_MIRROR(0xff00) AM_READ_LEGACY(mjchuuka_gfxrom_0_r)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0xff00) AM_WRITE_LEGACY(mjchuuka_coincounter_w)
	AM_RANGE(0x41, 0x41) AM_MIRROR(0xff00) AM_WRITE_LEGACY(hanakanz_keyb_w)
	AM_RANGE(0x42, 0x42) AM_MIRROR(0xff00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x43, 0x44) AM_MIRROR(0xff00) AM_READ_LEGACY(mjchuuka_keyb_r)
	AM_RANGE(0x45, 0x45) AM_MIRROR(0xff00) AM_READ_LEGACY(mjchuuka_gfxrom_1_r)
	AM_RANGE(0x60, 0x60) AM_MIRROR(0xff00) AM_READ_PORT("DSW1")
	AM_RANGE(0x61, 0x61) AM_MIRROR(0xff00) AM_READ_PORT("DSW2")
	AM_RANGE(0x62, 0x62) AM_MIRROR(0xff00) AM_READ_PORT("DSW3")
	AM_RANGE(0x63, 0x63) AM_MIRROR(0xff00) AM_READ_PORT("DSW4")
	AM_RANGE(0x64, 0x64) AM_MIRROR(0xff00) AM_READ_PORT("DSW5")		// DSW 1-4 high bits
	AM_RANGE(0x80, 0x80) AM_MIRROR(0xff00) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa0, 0xa1) AM_MIRROR(0xff00) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0xc0, 0xcf) AM_MIRROR(0xff00) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0xe0, 0xe1) AM_MIRROR(0xff00) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
ADDRESS_MAP_END


/***************************************************************************
                        Mahjong The Mysterious World
***************************************************************************/

static ADDRESS_MAP_START( mjmyster_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM				// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM				// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")		// RAM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")		// ROM/RAM (Banked)
	AM_RANGE(0xf000, 0xf1ff) AM_WRITE_LEGACY(rongrong_palette_w)	// RAM enabled by bit 4 of rombank
	AM_RANGE(0xf200, 0xffff) AM_WRITENOP		// ""
ADDRESS_MAP_END

static WRITE8_HANDLER( mjmyster_rambank_w )
{
	memory_set_bank(space->machine(), "bank2", data & 0x07);
	//logerror("%04x: rambank = %02x\n", cpu_get_pc(&space->device()), data);
}

static WRITE8_HANDLER( mjmyster_select2_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_input_sel = data;

	if (data & 0x80)
		state->m_keyb = 1;
}

static READ8_HANDLER( mjmyster_coins_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x00:	return input_port_read(space->machine(), "SYSTEM");
		case 0x01:	return 0xff;
		case 0x02:	return 0xff;	// bit 7 = 0 -> blitter busy, + hopper switch
		case 0x03:	return 0xff;
	}

	logerror("%06x: warning, unknown bits read, ddenlovr_select2 = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel);

	return 0xff;
}

static READ8_HANDLER( mjmyster_keyb_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 ret = 0xff;

	if      (BIT(state->m_keyb, 0))   ret = input_port_read(space->machine(), "KEY0");
	else if (BIT(state->m_keyb, 1))   ret = input_port_read(space->machine(), "KEY1");
	else if (BIT(state->m_keyb, 2))   ret = input_port_read(space->machine(), "KEY2");
	else if (BIT(state->m_keyb, 3))   ret = input_port_read(space->machine(), "KEY3");
	else if (BIT(state->m_keyb, 4))   ret = input_port_read(space->machine(), "KEY4");
	else	logerror("%06x: warning, unknown bits read, keyb = %02x\n", cpu_get_pc(&space->device()), state->m_keyb);

	state->m_keyb <<= 1;

	return ret;
}

static READ8_HANDLER( mjmyster_dsw_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))   return input_port_read(space->machine(), "DSW4");
	if (!BIT(state->m_dsw_sel, 1))   return input_port_read(space->machine(), "DSW3");
	if (!BIT(state->m_dsw_sel, 2))   return input_port_read(space->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 3))   return input_port_read(space->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 4))   return input_port_read(space->machine(), "DSW5");
	logerror("%06x: warning, unknown bits read, ddenlovr_select = %02x\n", cpu_get_pc(&space->device()), state->m_dsw_sel);
	return 0xff;
}

static WRITE8_HANDLER( mjmyster_coincounter_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x0c:
			coin_counter_w(space->machine(), 0, (~data) & 0x01);	// coin in
			coin_counter_w(space->machine(), 0, (~data) & 0x02);	// coin out actually
			#ifdef MAME_DEBUG
//              popmessage("cc: %02x",data);
			#endif

			break;

		default:
			logerror("%06x: warning, unknown bits written, ddenlovr_select2 = %02x, data = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel, data);
	}
}

static WRITE8_HANDLER( mjmyster_blitter_w )
{
	blitter_w(space, 0, offset, data, 0xfc);
}

static ADDRESS_MAP_START( mjmyster_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(mjmyster_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1c, 0x1c) AM_WRITE_LEGACY(mjmyster_rambank_w)
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(mmpanic_rombank_w)
	AM_RANGE(0x20, 0x20) AM_WRITE_LEGACY(mjmyster_select2_w)
	AM_RANGE(0x21, 0x21) AM_WRITE_LEGACY(mjmyster_coincounter_w)
	AM_RANGE(0x22, 0x22) AM_READ_LEGACY(mjmyster_coins_r)
	AM_RANGE(0x23, 0x23) AM_READ_LEGACY(mjmyster_keyb_r)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x42, 0x43) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x44, 0x44) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x46, 0x46) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x48, 0x48) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0x60, 0x6f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0x80, 0x83) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x84, 0x87) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x88, 0x8b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x8c, 0x8f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x94, 0x94) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x95, 0x95) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x96, 0x96) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x98, 0x98) AM_READ_LEGACY(unk_r)							// ? must be 78 on startup
	AM_RANGE(0xc2, 0xc2) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0xc3, 0xc3) AM_READ_LEGACY(mjmyster_dsw_r)
ADDRESS_MAP_END

/***************************************************************************
                            Hanafuda Hana Ginga
***************************************************************************/

static WRITE8_HANDLER( hginga_rombank_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	memory_set_bank(space->machine(), "bank1", data & 0x7);
	state->m_hginga_rombank = data;
}

// similar to rongrong
static READ8_HANDLER( hginga_protection_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 *rom = space->machine().region("maincpu")->base();

	if (state->m_hginga_rombank & 0x10)
		return hanakanz_rand_r(space, 0);
	return rom[0x10000 + 0x8000 * (state->m_hginga_rombank & 0x7) + 0xf601 - 0x8000];
}

static ADDRESS_MAP_START( hginga_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM								// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM								// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")						// RAM (Banked)
	AM_RANGE(0xf601, 0xf601) AM_READ_LEGACY(hginga_protection_r)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")						// ROM/RAM (Banked)
	AM_RANGE(0xf000, 0xf1ff) AM_WRITE_LEGACY(rongrong_palette_w)	// RAM enabled by bit 4 of rombank
	AM_RANGE(0xf700, 0xf706) AM_WRITENOP
ADDRESS_MAP_END

static READ8_DEVICE_HANDLER( hginga_dsw_r )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))   return input_port_read(device->machine(), "DSW4");
	if (!BIT(state->m_dsw_sel, 1))   return input_port_read(device->machine(), "DSW3");
	if (!BIT(state->m_dsw_sel, 2))   return input_port_read(device->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 3))   return input_port_read(device->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 4))   return input_port_read(device->machine(), "DSW5");

	logerror("%s: warning, unknown bits read, ddenlovr_select = %02x\n", device->machine().describe_context(), state->m_dsw_sel);
	return 0xff;
}

static WRITE8_HANDLER( hginga_input_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_input_sel = data;
	state->m_keyb = 0;
}

static READ8_HANDLER( hginga_coins_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x20:	return input_port_read(space->machine(), "SYSTEM");
		case 0x21:	return input_port_read(space->machine(), "BET");
		case 0x22:	return 0x7f;	// bit 7 = blitter busy, bit 6 = hopper
		case 0x23:	return state->m_coins;
	}
	logerror("%04x: coins_r with select = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel);
	return 0xff;
}

static WRITE8_HANDLER( hginga_80_w )
{
//  popmessage("port 80 = %02x", data);
}

static WRITE8_HANDLER( hginga_coins_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x2d:
			break;
		case 0x2c:
			// bit 0 = coin counter
			// bit 1 = out counter
			// bit 2 = 1 if bet on
			// bit 3 = 1 if bet on
			// bit 7?
			coin_counter_w(space->machine(), 0, data & 1);
#ifdef MAME_DEBUG
//          popmessage("COINS %02x", data);
#endif
			state->m_coins = data;
			break;
		default:
			logerror("%04x: coins_w with select = %02x, data = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel, data);
	}
}

static READ8_HANDLER( hginga_input_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	static const char *const keynames0[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };
	static const char *const keynames1[] = { "KEY5", "KEY6", "KEY7", "KEY8", "KEY9" };

	switch (state->m_input_sel)
	{
		case 0x2d:
			return 0xff;

		// player 1
		case 0xa1:
			return input_port_read(space->machine(), keynames0[state->m_keyb++]);

		// player 2
		case 0xa2:
			return input_port_read(space->machine(), keynames1[state->m_keyb++]);
	}
	logerror("%04x: input_r with select = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel);
	return 0xff;
}

static WRITE8_HANDLER( hginga_blitter_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	if (offset == 0)
	{
		state->m_ddenlovr_blit_latch = data;
	}
	else
	{
		switch (state->m_ddenlovr_blit_latch & 0x3f)
		{
			case 0x00:
				switch (data & 0xf)
				{
					case 0x03:
					case 0x06:
					case 0x0a:
						data = data & ~2;	// do not mirror writes of other layers to layer 1? (see code at 38d)
						break;
				}
				break;

			case 0x24:
				if (data == 0x1b)
					data = 0x13;			// vertical lines -> horizontal lines (see numbers drawn on cards on "first chance")
				break;
		}
	}
	blitter_w(space, 0, offset, data, 0xfc);
}

static ADDRESS_MAP_START( hginga_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(hginga_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1c, 0x1c) AM_READNOP AM_WRITE_LEGACY(mjmyster_rambank_w)
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(hginga_rombank_w)
	AM_RANGE(0x22, 0x23) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x24, 0x24) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x26, 0x26) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x28, 0x28) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0x40, 0x40) AM_WRITE_LEGACY(hginga_input_w)
	AM_RANGE(0x41, 0x41) AM_WRITE_LEGACY(hginga_coins_w)
	AM_RANGE(0x42, 0x42) AM_READ_LEGACY(hginga_coins_r)
	AM_RANGE(0x43, 0x43) AM_READ_LEGACY(hginga_input_r)
	AM_RANGE(0x60, 0x6f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0x80, 0x80) AM_WRITE_LEGACY(hginga_80_w)
	AM_RANGE(0xa0, 0xa3) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0xa4, 0xa7) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0xa8, 0xab) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0xac, 0xaf) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0xb4, 0xb4) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0xb5, 0xb5) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0xb6, 0xb6) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0xb8, 0xb8) AM_READ_LEGACY(unk_r)	// ? must be 78 on startup
ADDRESS_MAP_END


/***************************************************************************
                             Hanafuda Hana Gokou
***************************************************************************/

static UINT8 hgokou_player_r( address_space *space, int player )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 hopper_bit = ((state->m_hopper && !(space->machine().primary_screen->frame_number() % 10)) ? 0 : (1 << 6));

	if (!BIT(state->m_input_sel, 0))   return input_port_read(space->machine(), player ? "KEY5" : "KEY0") | hopper_bit;
	if (!BIT(state->m_input_sel, 1))   return input_port_read(space->machine(), player ? "KEY6" : "KEY1") | hopper_bit;
	if (!BIT(state->m_input_sel, 2))   return input_port_read(space->machine(), player ? "KEY7" : "KEY2") | hopper_bit;
	if (!BIT(state->m_input_sel, 3))   return input_port_read(space->machine(), player ? "KEY8" : "KEY3") | hopper_bit;
	if (!BIT(state->m_input_sel, 4))   return input_port_read(space->machine(), player ? "KEY9" : "KEY4") | hopper_bit;

	return 0x7f;	// bit 7 = blitter busy, bit 6 = hopper
}

static WRITE8_HANDLER( hgokou_dsw_sel_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_dsw_sel = data;
}

static READ8_HANDLER( hgokou_input_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_dsw_sel)
	{
		case 0x20:	return input_port_read(space->machine(), "SYSTEM");
		case 0x21:	return hgokou_player_r(space, 1);
		case 0x22:	return hgokou_player_r(space, 0);
		case 0x23:	return state->m_coins;
	}
	logerror("%06x: warning, unknown bits read, dsw_sel = %02x\n", cpu_get_pc(&space->device()), state->m_dsw_sel);
	return 0xff;
}

static WRITE8_HANDLER( hgokou_input_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_dsw_sel)
	{
		case 0x2c:
			// bit 0 = coin counter
			// bit 1 = out counter
			// bit 2 = hopper
			// bit 7 = ?
			coin_counter_w(space->machine(), 0, data & 1);
			state->m_hopper = data & 0x04;
#ifdef MAME_DEBUG
//          popmessage("COINS %02x",data);
#endif
			state->m_coins = data;
			break;

		case 0x2d:	state->m_input_sel = data;	break;

		case 0x2f:	break;	// ? written with 2f (hgokou)

		default:
			logerror("%04x: input_w with select = %02x, data = %02x\n", cpu_get_pc(&space->device()), state->m_dsw_sel, data);
	}
}

// similar to rongrong
static READ8_HANDLER( hgokou_protection_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 *rom = space->machine().region("maincpu")->base();

	if (state->m_hginga_rombank == 0)
		return hanakanz_rand_r(space, 0);
	return rom[0x10000 + 0x8000 * (state->m_hginga_rombank & 0x7) + 0xe601 - 0x8000];
}

static ADDRESS_MAP_START( hgokou_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM								// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM								// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")						// RAM (Banked)
	AM_RANGE(0xe601, 0xe601) AM_READ_LEGACY(hgokou_protection_r)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")						// ROM (Banked)
	AM_RANGE(0xe000, 0xe1ff) AM_WRITE_LEGACY(rongrong_palette_w)
	AM_RANGE(0xe700, 0xe706) AM_WRITENOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( hgokou_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(hginga_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1c, 0x1c) AM_READNOP AM_WRITE_LEGACY(mjmyster_rambank_w)		// ? ack on RTC int
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(hginga_rombank_w)
	AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0x40, 0x43) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x44, 0x47) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x48, 0x4b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x4c, 0x4f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x54, 0x54) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x55, 0x55) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x56, 0x56) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x58, 0x58) AM_READ_LEGACY(unk_r)									// ? must be 78 on startup
	AM_RANGE(0x60, 0x60) AM_WRITE_LEGACY(hgokou_dsw_sel_w)
	AM_RANGE(0x61, 0x61) AM_WRITE_LEGACY(hgokou_input_w)
	AM_RANGE(0x62, 0x62) AM_READ_LEGACY(hgokou_input_r)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x82, 0x83) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x84, 0x84) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x86, 0x86) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x88, 0x88) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0xb0, 0xb0) AM_READ_LEGACY(hanakanz_rand_r)
ADDRESS_MAP_END


/***************************************************************************
                        Hanafuda Hana Gokou Bangaihen
***************************************************************************/

static READ8_HANDLER( hgokbang_input_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	UINT8 ret;
	switch (state->m_dsw_sel)
	{
		case 0x2d:
			if (state->m_input_sel == 0xff)	// reset auto-incrementing input_sel
				state->m_input_sel = 0xfe;
			return 0;	// discarded
		case 0xa1:
			ret = hgokou_player_r(space, 1);
			state->m_input_sel <<= 1;		// auto-increment input_sel
			state->m_input_sel |= 1;
			return ret;
		case 0xa2:
			ret = hgokou_player_r(space, 0);
			state->m_input_sel <<= 1;		// auto-increment input_sel
			state->m_input_sel |= 1;
			return ret;
	}
	logerror("%06x: warning, unknown bits read, dsw_sel = %02x\n", cpu_get_pc(&space->device()), state->m_dsw_sel);
	return 0xff;
}

static ADDRESS_MAP_START( hgokbang_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(hginga_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1c, 0x1c) AM_READNOP AM_WRITE_LEGACY(mjmyster_rambank_w)		// ? ack on RTC int
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(hginga_rombank_w)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x22, 0x23) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x24, 0x24) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x26, 0x26) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x28, 0x28) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0x40, 0x40) AM_WRITE_LEGACY(hgokou_dsw_sel_w)
	AM_RANGE(0x41, 0x41) AM_WRITE_LEGACY(hgokou_input_w)
	AM_RANGE(0x42, 0x42) AM_READ_LEGACY(hgokou_input_r)
	AM_RANGE(0x43, 0x43) AM_READ_LEGACY(hgokbang_input_r)
	AM_RANGE(0x60, 0x6f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0xa0, 0xa3) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0xa4, 0xa7) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0xa8, 0xab) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0xac, 0xaf) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0xb4, 0xb4) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0xb5, 0xb5) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0xb6, 0xb6) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0xb8, 0xb8) AM_READ_LEGACY(unk_r)									// ? must be 78 on startup
	AM_RANGE(0xe0, 0xe0) AM_READ_LEGACY(hanakanz_rand_r)
ADDRESS_MAP_END


/***************************************************************************
                            Super Hana Paradise
***************************************************************************/

static WRITE8_HANDLER( hparadis_select_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	state->m_dsw_sel = data;
	state->m_keyb = 0;

	memory_set_bank(space->machine(), "bank1", data & 0x07);
	memory_set_bank(space->machine(), "bank2", ((data & 0xe0) >> 5));
}


static READ8_HANDLER( hparadis_input_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	static const char *const keynames0[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };
	static const char *const keynames1[] = { "KEY5", "KEY6", "KEY7", "KEY8", "KEY9" };

	switch (state->m_input_sel)
	{
		case 0x00:	return input_port_read(space->machine(), "P1");
		case 0x01:	return input_port_read(space->machine(), "P2");
		case 0x02:	return input_port_read(space->machine(), "SYSTEM");
		case 0x0d:	return 0x00;
		case 0x80:	return input_port_read(space->machine(), keynames0[state->m_keyb++]);	// P1 (Keys)
		case 0x81:	return input_port_read(space->machine(), keynames1[state->m_keyb++]);	// P2 (Keys)
	}
	logerror("%06x: warning, unknown bits read, input_sel = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel);
	return 0xff;
}

static READ8_HANDLER( hparadis_dsw_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (!BIT(state->m_dsw_sel, 0))	return input_port_read(space->machine(), "DSW1");
	if (!BIT(state->m_dsw_sel, 1))	return input_port_read(space->machine(), "DSW2");
	if (!BIT(state->m_dsw_sel, 2))	return 0xff;
	if (!BIT(state->m_dsw_sel, 3))	return 0xff;
	if (!BIT(state->m_dsw_sel, 4))	return input_port_read(space->machine(), "DSW3");
	return 0xff;
}

static WRITE8_HANDLER( hparadis_coin_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x0c:	coin_counter_w(space->machine(), 0, data & 1);	break;
		case 0x0d:	break;
		default:
			logerror("%04x: coins_w with select = %02x, data = %02x\n",cpu_get_pc(&space->device()), state->m_input_sel, data);
	}
}

static ADDRESS_MAP_START( hparadis_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM								// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM								// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")						// RAM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")						// ROM (Banked)
	AM_RANGE(0xc000, 0xc1ff) AM_WRITE_LEGACY(rongrong_palette_w)
ADDRESS_MAP_END

// the RTC seems unused
static ADDRESS_MAP_START( hparadis_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(rongrong_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1b, 0x1b) AM_READWRITE_LEGACY(rongrong_blitter_busy_r, rongrong_blitter_busy_w)
	AM_RANGE(0x1c, 0x1c) AM_READ_LEGACY(hparadis_dsw_r)
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(hparadis_select_w)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x60, 0x61) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x80, 0x83) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x84, 0x87) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x88, 0x8b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x8c, 0x8f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x94, 0x94) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x95, 0x95) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x96, 0x96) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x98, 0x98) AM_READ_LEGACY(unk_r)	// ? must be 78 on startup
	AM_RANGE(0xa0, 0xa0) AM_WRITE_LEGACY(hginga_input_w)
	AM_RANGE(0xa1, 0xa1) AM_WRITE_LEGACY(hparadis_coin_w)
	AM_RANGE(0xa2, 0xa3) AM_READ_LEGACY(hparadis_input_r)
	AM_RANGE(0xc2, 0xc2) AM_WRITENOP	// enables palette RAM at c000
ADDRESS_MAP_END


/***************************************************************************
                          Mahjong Mysterious World
***************************************************************************/

static READ8_HANDLER( mjmywrld_coins_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_input_sel)
	{
		case 0x80:	return input_port_read(space->machine(), "SYSTEM");
		case 0x81:	return 0x00;
		case 0x82:	return 0xff;	// bit 7 = 0 -> blitter busy, + hopper switch
		case 0x83:	return 0x00;
	}

	logerror("%06x: warning, unknown bits read, input_sel = %02x\n", cpu_get_pc(&space->device()), state->m_input_sel);

	return 0xff;
}

static ADDRESS_MAP_START( mjmywrld_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE_LEGACY(mjmyster_blitter_w)
	AM_RANGE(0x03, 0x03) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x1c, 0x1c) AM_WRITE_LEGACY(mjmyster_rambank_w)
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(hginga_rombank_w)
	AM_RANGE(0x20, 0x20) AM_WRITE_LEGACY(mjmyster_select2_w)
	AM_RANGE(0x21, 0x21) AM_WRITE_LEGACY(mjmyster_coincounter_w)
	AM_RANGE(0x22, 0x22) AM_READ_LEGACY(mjmywrld_coins_r)
	AM_RANGE(0x23, 0x23) AM_READ_LEGACY(mjmyster_keyb_r)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x42, 0x43) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x44, 0x44) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0x46, 0x46) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x48, 0x48) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0x60, 0x6f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0x80, 0x83) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x84, 0x87) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x88, 0x8b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x8c, 0x8f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x94, 0x94) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x95, 0x95) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x96, 0x96) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x98, 0x98) AM_READ_LEGACY(unk_r)	// ? must be 78 on startup
	AM_RANGE(0xc0, 0xc0) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0xe0, 0xe0) AM_READ_LEGACY(mjmyster_dsw_r)
ADDRESS_MAP_END


/***************************************************************************
                  Panel & Variety Akamaru Q Jousyou Dont-R
***************************************************************************/

static READ16_HANDLER( akamaru_protection1_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	return (state->m_prot_16 & 0x0008) ? 0x0001 : 0x0000;
}

static WRITE16_HANDLER( akamaru_protection1_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	int bank;

	COMBINE_DATA(&state->m_prot_16);
	// BCD number?
	bank = (((state->m_prot_16 >> 4) & 0x0f) % 10) * 10 + ((state->m_prot_16 & 0x0f) % 10);
	state->m_oki->set_bank_base(bank * 0x40000);

//  popmessage("bank $%0x (%d)", state->m_prot_16, bank);
}

static READ16_HANDLER( akamaru_protection2_r )
{
	return 0x55;
}

static READ16_HANDLER( akamaru_dsw_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT16 dsw = 0;

	if (state->m_dsw_sel16[1] == 0xff)	dsw |= input_port_read(space->machine(), "DSW1");
	if (state->m_dsw_sel16[0] == 0xff)	dsw |= input_port_read(space->machine(), "DSW2");
	return dsw;
}

static READ16_HANDLER( akamaru_blitter_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	return state->m_ddenlovr_blitter_irq_flag << 6;	// bit 7 = 1 -> blitter busy
}

static READ16_HANDLER( akamaru_e0010d_r )
{
	return 0xffff;	// read but not used, it seems
}

static ADDRESS_MAP_START( akamaru_map, AS_PROGRAM, 16, dynax_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM														// ROM

	AM_RANGE(0x213570, 0x213571) AM_WRITE_LEGACY(akamaru_protection1_w)							// OKI bank
	AM_RANGE(0x624680, 0x624681) AM_READ_LEGACY(akamaru_protection1_r)

	AM_RANGE(0xd00000, 0xd003ff) AM_WRITE_LEGACY(ddenlovr_palette_w)								// Palette
//  AM_RANGE(0xd01000, 0xd017ff) AM_WRITEONLY                                          // 0

	AM_RANGE(0xe00040, 0xe00047) AM_WRITE_LEGACY(ddenlovr16_palette_base_w)
	AM_RANGE(0xe00048, 0xe0004f) AM_WRITE_LEGACY(ddenlovr16_palette_mask_w)
	AM_RANGE(0xe00050, 0xe00057) AM_WRITE_LEGACY(ddenlovr16_transparency_pen_w)
	AM_RANGE(0xe00058, 0xe0005f) AM_WRITE_LEGACY(ddenlovr16_transparency_mask_w)
	AM_RANGE(0xe00068, 0xe00069) AM_WRITE_LEGACY(ddenlovr16_bgcolor_w)
	AM_RANGE(0xe0006a, 0xe0006b) AM_WRITE_LEGACY(ddenlovr16_priority_w)
	AM_RANGE(0xe0006c, 0xe0006d) AM_WRITE_LEGACY(ddenlovr16_layer_enable_w)
	AM_RANGE(0xe00070, 0xe00071) AM_READ_LEGACY(unk16_r)											// ? must be 78 on startup (not necessary in ddlover)
	AM_RANGE(0xe00080, 0xe00083) AM_WRITE_LEGACY(ddenlovr_blitter_w)
	AM_RANGE(0xe00086, 0xe00087) AM_READ_LEGACY(ddenlovr_gfxrom_r)									// Video Chip

	AM_RANGE(0xe00100, 0xe00101) AM_READ_PORT("P1")
	AM_RANGE(0xe00102, 0xe00103) AM_READ_PORT("P2")
	AM_RANGE(0xe00104, 0xe00105) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xe00106, 0xe00107) AM_READ_LEGACY(akamaru_protection2_r)
	AM_RANGE(0xe00108, 0xe0010b) AM_WRITEONLY AM_BASE(m_protection2)

	AM_RANGE(0xe0010c, 0xe0010d) AM_READ_LEGACY(akamaru_e0010d_r)
	AM_RANGE(0xe00200, 0xe00201) AM_READ_LEGACY(akamaru_dsw_r)										// DSW

	AM_RANGE(0xe00204, 0xe00205) AM_READ_LEGACY(akamaru_blitter_r)									// Blitter Busy & IRQ
	AM_RANGE(0xe00302, 0xe00303) AM_WRITE_LEGACY(ddenlovr_blitter_irq_ack_w)						// Blitter irq acknowledge

	AM_RANGE(0xe00304, 0xe00307) AM_WRITEONLY AM_BASE(m_dsw_sel16)				// DSW select
	AM_RANGE(0xe00308, 0xe00309) AM_WRITE_LEGACY(ddenlovr_coincounter_0_w)							// Coin Counters
	AM_RANGE(0xe0030c, 0xe0030d) AM_WRITE_LEGACY(ddenlovr_coincounter_1_w)							//

	AM_RANGE(0xe00400, 0xe00403) AM_DEVWRITE8_LEGACY("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0xe00500, 0xe0051f) AM_DEVREADWRITE8("rtc", msm6242_device, read, write, 0x00ff)
	AM_RANGE(0xe00600, 0xe00603) AM_DEVWRITE8_LEGACY("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0xe00604, 0xe00605) AM_DEVREAD8_LEGACY("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0xe00700, 0xe00701) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)	// Sound
	AM_RANGE(0xff0000, 0xffffff) AM_RAM														// RAM
ADDRESS_MAP_END


/***************************************************************************
                          Mahjong Fantasic Love
***************************************************************************/

static WRITE8_HANDLER( mjflove_rombank_w )
{
	memory_set_bank(space->machine(), "bank1", data & 0xf);
}

static WRITE8_DEVICE_HANDLER( mjflove_okibank_w )
{
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base((data & 0x07) * 0x40000);
	//popmessage("SOUND = %02x", data);
}

static READ8_HANDLER( mjflove_protection_r )
{
	return 0x27;
}

static READ8_HANDLER( mjflove_keyb_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 val = 0xff;

	if      (!BIT(state->m_keyb, 0))   val = input_port_read(space->machine(), offset ? "KEY5" : "KEY0");
	else if (!BIT(state->m_keyb, 1))   val = input_port_read(space->machine(), offset ? "KEY6" : "KEY1");
	else if (!BIT(state->m_keyb, 2))   val = input_port_read(space->machine(), offset ? "KEY7" : "KEY2");
	else if (!BIT(state->m_keyb, 3))   val = input_port_read(space->machine(), offset ? "KEY8" : "KEY3");
	else if (!BIT(state->m_keyb, 4))   val = input_port_read(space->machine(), offset ? "KEY9" : "KEY4");

	return val;
}

static CUSTOM_INPUT( mjflove_blitter_r )
{
	dynax_state *state = field.machine().driver_data<dynax_state>();

	// bit 7 = 1 -> blitter busy
	// bit 6 = 0 -> VBLANK?
	// bit 5 = 0 -> RTC?
	return state->m_mjflove_irq_cause;
}

static WRITE8_HANDLER( mjflove_blitter_w )
{
	blitter_w(space, 0, offset, data, 0);
}

static WRITE8_HANDLER( mjflove_coincounter_w )
{
	// bit 0 = in counter
	coin_counter_w(space->machine(), 0, data & 0x01);

	if (data & 0xfe)
	{
		logerror("%04x: warning, coin counter = %02x\n", cpu_get_pc(&space->device()), data);
//      popmessage("COIN = %02x", data);
	}
}

static ADDRESS_MAP_START( mjflove_portmap, AS_IO, 8, dynax_state )	// 16 bit I/O
	AM_RANGE(0x0010, 0x0010) AM_READ_LEGACY(hanakanz_rand_r) AM_MIRROR(0xff00)
	AM_RANGE(0x001c, 0x001c) AM_READ_PORT("DSW2") AM_MIRROR(0xff00)
	AM_RANGE(0x001e, 0x001e) AM_WRITE_LEGACY(hanakanz_keyb_w) AM_MIRROR(0xff00)
	AM_RANGE(0x0020, 0x0023) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x0024, 0x0027) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x0028, 0x002b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x002c, 0x002f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x0034, 0x0034) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x0035, 0x0035) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x0036, 0x0036) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x0038, 0x0038) AM_READNOP			// ? ack or watchdog
	AM_RANGE(0x0040, 0x0041) AM_WRITE_LEGACY(mjflove_blitter_w) AM_MIRROR(0xff00)
	AM_RANGE(0x0043, 0x0043) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x0080, 0x0081) AM_READ_LEGACY(mjflove_keyb_r)
	AM_RANGE(0x0082, 0x0082) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x00da, 0x00da) AM_READ_LEGACY(mjflove_protection_r) AM_MIRROR(0xff00)
	AM_RANGE(0x00f2, 0x00f2) AM_WRITE_LEGACY(mjmyster_rambank_w) AM_MIRROR(0xff00)
	AM_RANGE(0x00f8, 0x00f8) AM_WRITE_LEGACY(mjflove_rombank_w) AM_MIRROR(0xff00)
	AM_RANGE(0x00fa, 0x00fa) AM_DEVWRITE_LEGACY("oki", mjflove_okibank_w)
	AM_RANGE(0x0100, 0x0100) AM_READ_PORT("DSW1")
	AM_RANGE(0x0181, 0x0181) AM_WRITENOP						// ? int. enable
	AM_RANGE(0x0184, 0x0184) AM_WRITE_LEGACY(mjflove_coincounter_w)
	AM_RANGE(0x0200, 0x0201) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x0280, 0x028f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0x0300, 0x0301) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0x0380, 0x0380) AM_DEVREADWRITE("oki", okim6295_device, read, write)
ADDRESS_MAP_END


/***************************************************************************
                          Mahjong Jong-Tei
***************************************************************************/

static WRITE8_DEVICE_HANDLER( jongtei_okibank_w )
{
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base(((data >> 4) & 0x07) * 0x40000);
}

static WRITE8_HANDLER( jongtei_dsw_keyb_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_dsw_sel = data;
	state->m_keyb = data;
}

static READ8_HANDLER( jongtei_busy_r )
{
	return 0x04;	// !bit 2 = blitter busy
}

static ADDRESS_MAP_START( jongtei_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x2c, 0x2c) AM_READ_LEGACY(jongtei_busy_r) AM_DEVWRITE_LEGACY("oki", jongtei_okibank_w)
	AM_RANGE(0x2e, 0x2e) AM_WRITE_LEGACY(hanakanz_blitter_reg_w)
	AM_RANGE(0x30, 0x30) AM_WRITE_LEGACY(hanakanz_rombank_w)
	AM_RANGE(0x31, 0x31) AM_WRITE_LEGACY(jongtei_dsw_keyb_w)
	AM_RANGE(0x32, 0x32) AM_READ_LEGACY(hanakanz_dsw_r)
	AM_RANGE(0x40, 0x40) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x41, 0x42) AM_READ_LEGACY(hanakanz_keyb_r)
	AM_RANGE(0x43, 0x43) AM_WRITE_LEGACY(hanakanz_coincounter_w)
	AM_RANGE(0x46, 0x46) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0x60, 0x60) AM_WRITE_LEGACY(hanakanz_blitter_data_w)
	AM_RANGE(0x61, 0x61) AM_WRITE_LEGACY(hanakanz_palette_w)
	AM_RANGE(0x63, 0x64) AM_READ_LEGACY(hanakanz_gfxrom_r)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xc0, 0xcf) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
ADDRESS_MAP_END


/***************************************************************************
                            Mahjong Seiryu Densetsu
***************************************************************************/

static ADDRESS_MAP_START( sryudens_map, AS_PROGRAM, 8, dynax_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM								// ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM								// RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank2")						// RAM (Banked)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")						// ROM (Banked)
	AM_RANGE(0xe000, 0xe1ff) AM_WRITE_LEGACY(rongrong_palette_w)
ADDRESS_MAP_END

static READ8_HANDLER( sryudens_keyb_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 val = 0x3f;

	if      (!BIT(state->m_keyb, 0))   val = input_port_read(space->machine(), offset ? "KEY5" : "KEY0");
	else if (!BIT(state->m_keyb, 1))   val = input_port_read(space->machine(), offset ? "KEY6" : "KEY1");
	else if (!BIT(state->m_keyb, 2))   val = input_port_read(space->machine(), offset ? "KEY7" : "KEY2");
	else if (!BIT(state->m_keyb, 3))   val = input_port_read(space->machine(), offset ? "KEY8" : "KEY3");
	else if (!BIT(state->m_keyb, 4))   val = input_port_read(space->machine(), offset ? "KEY9" : "KEY4");

	val |= input_port_read(space->machine(), offset ? "HOPPER" : "BET");
	if (offset)
		val &= 0x7f;	// bit 7 = blitter busy
	return val;
}

static WRITE8_HANDLER( sryudens_coincounter_w )
{
	// bit 0 = coin counter
	// bit 1 = out counter
	// bit 2
	// bit 4
	// bit 7 = ? mostly on

	coin_counter_w(space->machine(), 0, data & 1);

	if (data & 0x68)
		logerror("%04x: warning, coin counter = %02x\n", cpu_get_pc(&space->device()), data);

#ifdef MAME_DEBUG
//  popmessage("COIN = %02x", data);
#endif
}

static WRITE8_HANDLER( sryudens_rambank_w )
{
	memory_set_bank(space->machine(), "bank2", data & 0x0f);
	//logerror("%04x: rambank = %02x\n", cpu_get_pc(&space->device()), data);
}

static ADDRESS_MAP_START( sryudens_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0x04, 0x05) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0x1c, 0x1c) AM_READNOP AM_WRITE_LEGACY(sryudens_rambank_w)	// ? ack on RTC int
	AM_RANGE(0x1e, 0x1e) AM_WRITE_LEGACY(mjflove_rombank_w)
	AM_RANGE(0x20, 0x23) AM_WRITE_LEGACY(ddenlovr_palette_base_w)
	AM_RANGE(0x24, 0x27) AM_WRITE_LEGACY(ddenlovr_palette_mask_w)
	AM_RANGE(0x28, 0x2b) AM_WRITE_LEGACY(ddenlovr_transparency_pen_w)
	AM_RANGE(0x2c, 0x2f) AM_WRITE_LEGACY(ddenlovr_transparency_mask_w)
	AM_RANGE(0x34, 0x34) AM_WRITE_LEGACY(ddenlovr_bgcolor_w)
	AM_RANGE(0x35, 0x35) AM_WRITE_LEGACY(ddenlovr_priority_w)
	AM_RANGE(0x36, 0x36) AM_WRITE_LEGACY(ddenlovr_layer_enable_w)
	AM_RANGE(0x38, 0x38) AM_READNOP			// ? ack or watchdog
	AM_RANGE(0x40, 0x41) AM_WRITE_LEGACY(mjflove_blitter_w)
	AM_RANGE(0x43, 0x43) AM_READ_LEGACY(rongrong_gfxrom_r)
	AM_RANGE(0x50, 0x50) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0x70, 0x70) AM_DEVWRITE_LEGACY("oki", quizchq_oki_bank_w)
	AM_RANGE(0x80, 0x8f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0x90, 0x90) AM_READ_PORT("DSW1")
	AM_RANGE(0x91, 0x91) AM_READ_PORT("DSW2")
	AM_RANGE(0x92, 0x92) AM_READ_PORT("DSW4")
	AM_RANGE(0x93, 0x93) AM_READ_PORT("DSW3")
	AM_RANGE(0x94, 0x94) AM_READ_PORT("DSWTOP")
	AM_RANGE(0x98, 0x98) AM_WRITE_LEGACY(sryudens_coincounter_w)
	AM_RANGE(0x99, 0x99) AM_WRITE_LEGACY(hanakanz_keyb_w)
	AM_RANGE(0x9a, 0x9a) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x9b, 0x9c) AM_READ_LEGACY(sryudens_keyb_r)
ADDRESS_MAP_END


/***************************************************************************
                            Mahjong Daimyojin
***************************************************************************/

static READ8_HANDLER( daimyojn_keyb1_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 val = 0x3f;

	if      (!BIT(state->m_keyb, 0))  val = input_port_read(space->machine(), "KEY0");
	else if (!BIT(state->m_keyb, 1))  val = input_port_read(space->machine(), "KEY1");
	else if (!BIT(state->m_keyb, 2))  val = input_port_read(space->machine(), "KEY2");
	else if (!BIT(state->m_keyb, 3))  val = input_port_read(space->machine(), "KEY3");
	else if (!BIT(state->m_keyb, 4))  val = input_port_read(space->machine(), "KEY4");

	val |= input_port_read(space->machine(), "BET");
	return val;
}

static READ8_HANDLER( daimyojn_keyb2_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	UINT8 val = 0x3f;

	if      (!BIT(state->m_keyb, 0))  val = input_port_read(space->machine(), "KEY5");
	else if (!BIT(state->m_keyb, 1))  val = input_port_read(space->machine(), "KEY6");
	else if (!BIT(state->m_keyb, 2))  val = input_port_read(space->machine(), "KEY7");
	else if (!BIT(state->m_keyb, 3))  val = input_port_read(space->machine(), "KEY8");
	else if (!BIT(state->m_keyb, 4))  val = input_port_read(space->machine(), "KEY9");

	val |= input_port_read(space->machine(), "HOPPER");
	return val;
}

// 1B18: D4 ED 76 C9 CB
// 1B1D: 96 AF 34 8B 89

static WRITE8_HANDLER( daimyojn_protection_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_prot_val = data;
}

static READ8_HANDLER( daimyojn_protection_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	switch (state->m_prot_val)
	{
		case 0xd4:	return 0x96;
		case 0xed:	return 0xaf;
		case 0x76:	return 0x34;
		case 0xc9:	return 0x8b;
		case 0xcb:	return 0x89;
	}
	return 0xff;
}

// 1ADD: D4 ED 76 C9 CB
// 1AE2: D9 E0 7B C4 C6

static READ8_HANDLER( momotaro_protection_r )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	switch (state->m_prot_val)
	{
		case 0xd4: return 0xd9;
		case 0xed: return 0xe0;
		case 0x76: return 0x7b;
		case 0xc9: return 0xc4;
		case 0xcb: return 0xc6;
	}

	return 0xff;
}

static WRITE8_DEVICE_HANDLER( daimyojn_okibank_w )
{
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base(((data >> 4) & 0x01) * 0x40000);
}

static WRITE8_HANDLER( daimyojn_palette_sel_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();
	state->m_daimyojn_palette_sel = data;
}

static WRITE8_HANDLER( daimyojn_blitter_data_palette_w )
{
	dynax_state *state = space->machine().driver_data<dynax_state>();

	if (state->m_daimyojn_palette_sel & 0x01)
		hanakanz_palette_w(space, offset, data);
	else
		hanakanz_blitter_data_w(space, offset, data);
}

static READ8_HANDLER( daimyojn_year_hack_r )
{
	// See code at C8D7, 633f holds reg B of the RTC
	return offset ? 1 : 0;	// year = 0x10 (BCD)
}

static ADDRESS_MAP_START( daimyojn_portmap, AS_IO, 8, dynax_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x2c, 0x2c) AM_READ_LEGACY(jongtei_busy_r) AM_DEVWRITE_LEGACY("oki", daimyojn_okibank_w)
	AM_RANGE(0x2e, 0x2e) AM_WRITE_LEGACY(daimyojn_palette_sel_w)
	AM_RANGE(0x30, 0x30) AM_WRITE_LEGACY(hanakanz_blitter_reg_w)
	AM_RANGE(0x31, 0x31) AM_WRITE_LEGACY(jongtei_dsw_keyb_w)
	AM_RANGE(0x32, 0x32) AM_READ_LEGACY(hanakanz_dsw_r)
	AM_RANGE(0x40, 0x40) AM_WRITE_LEGACY(daimyojn_blitter_data_palette_w)
	AM_RANGE(0x42, 0x44) AM_READ_LEGACY(hanakanz_gfxrom_r)
	AM_RANGE(0x8a, 0x8b) AM_READ_LEGACY(daimyojn_year_hack_r)	// ?
	AM_RANGE(0x80, 0x8f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa8, 0xa8) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xaa, 0xaa) AM_READ_LEGACY(daimyojn_keyb1_r)
	AM_RANGE(0xac, 0xac) AM_READ_LEGACY(daimyojn_keyb2_r)
	AM_RANGE(0xae, 0xae) AM_WRITE_LEGACY(hanakanz_coincounter_w)
	AM_RANGE(0xb0, 0xb0) AM_WRITE_LEGACY(mjmyster_rambank_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE_LEGACY(mjflove_rombank_w)
	AM_RANGE(0xd0, 0xd0) AM_READ_LEGACY(hanakanz_rand_r)
	AM_RANGE(0xe0, 0xe0) AM_READWRITE_LEGACY(daimyojn_protection_r, daimyojn_protection_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( ddenlovj )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL )	// blitter busy flag
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ddenlovj_blitter_r, NULL)	// blitter irq flag? (bit 5) & RTC (bit 6)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Helps" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x06, "1 (Easy)" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x07, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x18, 0x18, "Timer Speed" )
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Force Test Mode?" )	// shows all dsw's as off
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ddenlovr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? quiz365
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ddenlovr_special_r, NULL)	// blitter irq flag
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// blitter busy flag

	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Comments / Help" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Show Girl" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, "Yes, Clothed" )
	PORT_DIPSETTING(    0xc0, "Yes, Bikini" )
	PORT_DIPSETTING(    0x00, "Yes, Topless" )
INPUT_PORTS_END


static INPUT_PORTS_START( nettoqc )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL )	// blitter busy flag
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(nettoqc_special_r, NULL)	// ? (bit 5) & blitter irq flag (bit 6)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6*" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7*" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1*" )
	PORT_DIPSETTING(    0x02, "0" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3*" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4*" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5*" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6*" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7*" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-8*" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8*" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9*" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x00, "Detailed Tests" )	// menu "8 OPTION" in service mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( quiz365 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? quiz365
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ddenlovr_special_r, NULL)	// blitter irq flag
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// blitter busy flag

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-5*" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 1-6&7" )
	PORT_DIPSETTING(    0x40, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
//  PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x02, "0" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-2&3" )
	PORT_DIPSETTING(    0x08, "0" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x00, "Detailed Tests" )	// menu "8 OPTION" in service mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( rongrong )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? quiz365
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? blitter irq flag ?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? blitter busy flag ?

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Helps" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x30, 0x30, "VS Rounds" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
//  PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Select Round" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( quizchq )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? quiz365
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? blitter irq flag ?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )	// ? blitter busy flag ?

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Set Date" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( mmpanic )
	PORT_START("IN0")	/* 6a (68 = 1:used? 2:normal 3:goes to 69) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )	// busy?

	PORT_START("IN1")	/* 6b (68 = 0 & 1) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x1c, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x1c, "0" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x14, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
//  PORT_DIPSETTING(    0x04, "5" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Linked Cabinets" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7*" )	// 2-0 is related to the same thing (flip?)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0*" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-2&3*" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4*" )	// used?
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5*" )	// used?
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6*" )	// 6 & 7?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7*" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Set Date" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2*" )	// used?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3*" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6*" )	// used?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7*" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( animaljr )
	PORT_START("IN0")	/* 6a (68 = 1:used? 2:normal 3:goes to 69) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )	// busy?

	PORT_START("IN1")	/* 6b (68 = 0 & 1) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	// tested ?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x80, "Difficulty?" )
	PORT_DIPSETTING(    0xe0, "0" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0xa0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x60, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x00, "7" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2*" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 2-3&4" )	// used ?
//  PORT_DIPSETTING(    0x10, "0" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Pirate Fight" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Taito Copyright" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0*" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Tickets" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2*" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hanakanz )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Unknown 1-0&1&2" )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Unknown 1-5&6" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x80, "10" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-2&3" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 2-4&5" )
	PORT_DIPSETTING(    0x30, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x10, "250" )
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7" )
	PORT_DIPSETTING(    0xc0, "50" )
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x00, "80" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Game Type" )
	PORT_DIPSETTING(    0x03, "8 Cards" )
	PORT_DIPSETTING(    0x02, "6 Cards (Bets)" )
	PORT_DIPSETTING(    0x01, "6 Cards (Bets)?" )
	PORT_DIPSETTING(    0x00, "6 Cards (Bets)??" )
	PORT_DIPNAME( 0x04, 0x04, "(C) Nihon (Censored)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 3-3&4" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x60, 0x60, "Unknown 3-5&6" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Girl" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 4-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Keyboard" )
	PORT_DIPSETTING(    0x40, "Hanafuda" )
	PORT_DIPSETTING(    0x00, "Mahjong" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-9" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-8&9" )
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-8" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-9" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Allow Bets" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("HOPPER")
	PORT_DIPNAME( 0x40, 0x40, "? Hopper M." )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hkagerou )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )		// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_NAME("P2 1 (Hanafuda) / P2 A (Mahjong)") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_NAME("P2 5 (Hanafuda) / P2 E (Mahjong)") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )					// P2 I (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Yes")	// P2 M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )					// P2 Kan (not used)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_NAME("P2 2 (Hanafuda) / P2 B (Mahjong)") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_NAME("P2 6 (Hanafuda) / P2 F (Mahjong)") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )					// P2 J (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 No")	// P2 N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )					// P2 Reach (not used)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// P2 BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_NAME("P2 3 (Hanafuda) / P2 C (Mahjong)") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 G (not used)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 K (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 Chi (not used)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 Ron (not used)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_NAME("P2 4 (Hanafuda) / P2 D (Mahjong)") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 H (not used)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 L (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 PON (not used)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)								// P2 ?? (not used)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)		// P2 t (Take)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// P2 w (W.Up)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// P2 f (Flip Flop)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)		// P2 b (Big)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)		// P2 s (Small)

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_NAME("P1 1 (Hanafuda) / P1 A (Mahjong)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_NAME("P1 5 (Hanafuda) / P1 E (Mahjong)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )										// P1 I (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Yes") PORT_CODE(KEYCODE_Y)	// P1 M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )										// P1 Kan (not used)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_NAME("P1 2 (Hanafuda) / P1 B (Mahjong)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_NAME("P1 6 (Hanafuda) / P1 F (Mahjong)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )										// P1 J (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 No") PORT_CODE(KEYCODE_N)	// P1 N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )										// P1 Reach (not used)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )									// P1 BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_NAME("P1 3 (Hanafuda) / P1 C (Mahjong)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P1 G (not used)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P1 K (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P1 Chi (not used)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P1 Ron (not used)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_NAME("P1 4 (Hanafuda) / P1 D (Mahjong)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P1 H (not used)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P1 L (not used)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P1 PON (not used)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)				// P1 ?? (not used)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )		// P1 t (Take)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// P1 w (W.Up)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// P1 f (Flip Flop)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )		// P1 b (Big)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )		// P1 s (Small)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Unknown 1-0&1&2" )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Unknown 1-5&6" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Credits Per Note" )
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPSETTING(    0x80, "50" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-2&3" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 2-4&5" )
	PORT_DIPSETTING(    0x30, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x10, "250" )
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7" )
	PORT_DIPSETTING(    0xc0, "50" )
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x00, "80" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Game Type?" )
	PORT_DIPSETTING(    0x01, "0" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "(C) Nihon (Censored)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 3-3&4" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x60, 0x60, "Unknown 3-5&6" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Girl?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 4-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Keyboard" )
	PORT_DIPSETTING(    0x40, "Hanafuda" )
	PORT_DIPSETTING(    0x00, "Mahjong" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-8" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-9" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-8&9" )
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-8" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-9" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Disable Bets" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("HOPPER")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static INPUT_PORTS_START( mjreach1 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )	// Start 2

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x18, 0x18, "YAKUMAN Times" )
//  PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x20, 0x20, "3 BAI In YAKUMAN Bonus Chance" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto Tsumo" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Timing" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls (Demo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Girls Show After 3 Renso" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Girls (Play)" )	// Shown as always OFF in dips sheet
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Boys In Game" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Boys" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Don Den Key" )
	PORT_DIPSETTING(    0x80, "Start" )
	PORT_DIPSETTING(    0x00, "Flip/Flop" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Computer Strength" )
	PORT_DIPSETTING(    0x0c, "Weak" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x04, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x10, 0x10, "Game Style" )
	PORT_DIPSETTING(    0x10, "Credit" )
	PORT_DIPSETTING(    0x00, "Credit Time" )
	PORT_DIPNAME( 0x20, 0x20, "Start Method (Credit Time)" )
	PORT_DIPSETTING(    0x20, "?" )
	PORT_DIPSETTING(    0x00, "Rate" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-9" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Bets?" )
	PORT_DIPSETTING(    0x40, "0" )
	PORT_DIPSETTING(    0x00, "1" )

	PORT_START("HOPPER")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static INPUT_PORTS_START( jongtei )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )	// Start 2

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
//  PORT_DIPSETTING(    0x08, "2" ) // ? these don't let you start a game
//  PORT_DIPSETTING(    0x04, "3" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x18, 0x18, "YAKUMAN Times" )
//  PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x20, 0x20, "3 BAI In YAKUMAN Bonus Chance" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto Tsumo" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Timing" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Girls Show After 3 Renso" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
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

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Computer Strength" )
	PORT_DIPSETTING(    0x0c, "Weak" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x04, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x10, 0x10, "Game Style" )
	PORT_DIPSETTING(    0x10, "Credit" )
	PORT_DIPSETTING(    0x00, "Credit Time" )
	PORT_DIPNAME( 0x20, 0x20, "Start Method (Credit Time)" )
	PORT_DIPSETTING(    0x20, "?" )
	PORT_DIPSETTING(    0x00, "Rate" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-9" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Bets?" )
	PORT_DIPSETTING(    0x40, "0" )
	PORT_DIPSETTING(    0x00, "1" )

	PORT_START("HOPPER")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static INPUT_PORTS_START( mjchuuka )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )	// Start 2

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x20, "1 2 3 4 6 8 10 15" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
//  PORT_DIPSETTING(    0x08, "2" ) // ? these don't let you start a game
//  PORT_DIPSETTING(    0x04, "3" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "255" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1000?" )
	PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x18, 0x18, "YAKUMAN Times" )
//  PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3?" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DonDen Key" )
	PORT_DIPSETTING(    0x40, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, "Girls" )
	PORT_DIPSETTING(    0x0c, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, "Dressed" )
	PORT_DIPSETTING(    0x04, "Underwear" )
	PORT_DIPSETTING(    0x00, "Nude" )
	PORT_DIPNAME( 0x10, 0x00, "Girls Speech" )
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

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Credits Per Note" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x0c, 0x0c, "Computer Strength?" )
	PORT_DIPSETTING(    0x0c, "Weak" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x04, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-9" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("HOPPER")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static INPUT_PORTS_START( funkyfig )
	PORT_START("IN0")	/* Keys (port 83 with port 80 = 20) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("IN1")	/* ? (port 83 with port 80 = 21) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )	// ?
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")	/* Coins (port 82 with port 80 = 22) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")	/* (low bits, port 1c with rombank = 1e) */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x1c, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x14, "1" )
	PORT_DIPSETTING(    0x1c, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
//  PORT_DIPSETTING(    0x04, "5" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Linked Cabinets" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Play Rock Smash" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")	/* (low bits, port 1c with rombank = 1d) */
	PORT_DIPNAME( 0x01, 0x01, "2 Player Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-2&3*" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4*" )	// used
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5*" )	// used
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7*" )	// used
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START("DSW3")	/* (high bits, port 1c with rombank = 1b) */
	PORT_DIPNAME( 0x01, 0x01, "Continue?" )	// related to continue
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Debug Text" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Country" )
	PORT_DIPSETTING(    0x08, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static INPUT_PORTS_START( mjmyster )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
//  PORT_DIPSETTING(    0x08, "2" ) // ? these don't let you start a game
//  PORT_DIPSETTING(    0x04, "3" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x70, "Cut" )
	PORT_DIPSETTING(    0x60, "1 T" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x30, "700" )
	PORT_DIPSETTING(    0x20, "1000" )
	PORT_DIPSETTING(    0x10, "1000?" )
	PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "YAKUMAN Times" )
//  PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3?" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Payout" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hong_Kong ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x03, 0x03, "Computer Strength?" )
	PORT_DIPSETTING(    0x03, "Weak" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x01, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DonDen Key" )
	PORT_DIPSETTING(    0x08, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hginga )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
//  PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2	)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1	)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Girls" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Hint" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Cards Labels" )
	PORT_DIPSETTING(    0x80, "Numbers" )
	PORT_DIPSETTING(    0x00, "Letters" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 3-0&1" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 3-3&4" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 3-6&7" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "2 5 6 20 50 100" )
	PORT_DIPSETTING(    0x20, "2 5 6 20 50 200" )
	PORT_DIPSETTING(    0x10, "2 5 6 20 50 250" )
	PORT_DIPSETTING(    0x00, "2 5 6 20 50 300" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7" )
	PORT_DIPSETTING(    0xc0, "50" )
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x00, "80" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Unknown 1-0&1&2" )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Unknown 1-5&6" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Keyboard" )
	PORT_DIPSETTING(    0x00, "Hanafuda" )	// Requires different inputs
	PORT_DIPSETTING(    0x01, "Mahjong" )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-8" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-9" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Allow Bets" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START( hgokou )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "s"

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Girls" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Hint" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Cards Labels" )
	PORT_DIPSETTING(    0x80, "Numbers" )
	PORT_DIPSETTING(    0x00, "Letters" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 3-0&1" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 3-3&4" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 3-6&7" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "2 5 6 20 50 100" )
	PORT_DIPSETTING(    0x20, "2 5 6 20 50 200" )
	PORT_DIPSETTING(    0x10, "2 5 6 20 50 250" )
	PORT_DIPSETTING(    0x00, "2 5 6 20 50 300" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7" )
	PORT_DIPSETTING(    0xc0, "50" )
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x00, "80" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Unknown 1-0&1&2" )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Unknown 1-5&6" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Control Panel" )
	PORT_DIPSETTING(    0x00, "Hanafuda" )
	PORT_DIPSETTING(    0x01, "Mahjong" )	// Requires different inputs
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-8" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-9" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjmyornt )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
//  PORT_DIPSETTING(    0x08, "2" ) // ? these don't let you start a game
//  PORT_DIPSETTING(    0x04, "3" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x70, "Cut" )
	PORT_DIPSETTING(    0x60, "1 T" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x30, "700" )
	PORT_DIPSETTING(    0x20, "1000" )
	PORT_DIPSETTING(    0x10, "1000?" )
	PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "YAKUMAN Times" )
//  PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3?" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Payout" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
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

	PORT_START("DSW5")
	PORT_DIPNAME( 0x03, 0x03, "Computer Strength?" )
	PORT_DIPSETTING(    0x03, "Weak" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x01, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DonDen Key" )
	PORT_DIPSETTING(    0x08, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

#ifdef UNUSED_DEFINITION
static INPUT_PORTS_START( mjmyuniv )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
//  PORT_DIPSETTING(    0x08, "2" ) // ? these don't let you start a game
//  PORT_DIPSETTING(    0x04, "3" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x70, "Cut" )
	PORT_DIPSETTING(    0x60, "1 T" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x30, "700" )
	PORT_DIPSETTING(    0x20, "1000" )
	PORT_DIPSETTING(    0x10, "1000?" )
	PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "YAKUMAN Times" )
//  PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3?" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Payout" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
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

	PORT_START("DSW5")
	PORT_DIPNAME( 0x03, 0x03, "Computer Strength?" )
	PORT_DIPSETTING(    0x03, "Weak" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x01, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DonDen Key" )
	PORT_DIPSETTING(    0x08, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END
#endif

static INPUT_PORTS_START( akamaru )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	/* Test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )	// blitter irq flag
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// blitter busy flag

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjflove )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mjflove_blitter_r, NULL)	// RTC (bit 5) & blitter irq flag (bit 6)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// blitter busy flag

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )	// Start 2

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("DSW1")	/* IN11 - DSW1 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")	/* IN12 - DSW2 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x14, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x1c, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( hparadis )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")	/* IN1 - Player 2 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// analyzer
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )	// data clear
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* keyb 1 */
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1	)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "s"

	/* keyb 2 */
	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2	)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// PON
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Control Panel" )
	PORT_DIPSETTING(    0x60, "Hanafuda" )
	PORT_DIPSETTING(    0x40, "Mahjong" )
	PORT_DIPSETTING(    0x20, DEF_STR( Joystick ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x80, 0x80, "First Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "0?" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
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

static INPUT_PORTS_START( sryudens )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )		// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )		// note2

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )	// Start 2

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
//  PORT_DIPSETTING(    0xxx, "1 2 3 4 5 6 8 10" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
//  PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 2C_2C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
//  PORT_DIPSETTING(    0x08, "2" ) // ? these don't let you start a game
//  PORT_DIPSETTING(    0x04, "3" )
//  PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
//  PORT_DIPSETTING(    0x01, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DonDen Key" )
	PORT_DIPSETTING(    0x20, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Undress Girl" )
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

	PORT_START("DSWTOP")
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "0" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
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

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Bets?" )
	PORT_DIPSETTING(    0x40, "0" )
	PORT_DIPSETTING(    0x00, "1" )

	PORT_START("HOPPER")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( daimyojn )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )		// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )		// note2

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )	// Start 2

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)	// BET

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG	 ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )	// E
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )	// I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )	// M
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// Kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	// Start 1

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )	// B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )	// F
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )	// J
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )	// N
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// Reach
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )	// BET

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )	// C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )	// G
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )	// K
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// Chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// Ron
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )	// D
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )	// H
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )	// L
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// Pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// nothing

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE       )	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP   )	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP   )	// Flip Flop
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG         )	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL       )	// "s"

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate (%)" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
//  PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
//  PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
//  PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
//  PORT_DIPSETTING(    0x30, "2 3 6 8 12 15 30 50" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, "Multiplier" )
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0xc0, "Payout" )
	PORT_DIPSETTING(    0xc0, "300" )
	PORT_DIPSETTING(    0x80, "500" )
	PORT_DIPSETTING(    0x40, "700" )
	PORT_DIPSETTING(    0x00, "1000" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
//  PORT_DIPSETTING(    0x01, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
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

	PORT_START("DSW4")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "DonDen Key" )
	PORT_DIPSETTING(    0x02, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
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

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
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

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Bets?" )
	PORT_DIPSETTING(    0x40, "0" )
	PORT_DIPSETTING(    0x00, "1" )

	PORT_START("HOPPER")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static MACHINE_START( ddenlovr )
{
	dynax_state *state = machine.driver_data<dynax_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_soundcpu = machine.device("soundcpu");
	state->m_oki = machine.device<okim6295_device>("oki");

	state->save_item(NAME(state->m_input_sel));
	state->save_item(NAME(state->m_dsw_sel));
	state->save_item(NAME(state->m_keyb));
	state->save_item(NAME(state->m_coins));
	state->save_item(NAME(state->m_hopper));

	state->save_item(NAME(state->m_okibank));
	state->save_item(NAME(state->m_rongrong_blitter_busy_select));

	state->save_item(NAME(state->m_prot_val));
	state->save_item(NAME(state->m_prot_16));
	state->save_item(NAME(state->m_quiz365_protection));

	state->save_item(NAME(state->m_mmpanic_leds));
	state->save_item(NAME(state->m_funkyfig_lockout));
	state->save_item(NAME(state->m_romdata));
	state->save_item(NAME(state->m_palette_index));
	state->save_item(NAME(state->m_hginga_rombank));
	state->save_item(NAME(state->m_mjflove_irq_cause));
	state->save_item(NAME(state->m_daimyojn_palette_sel));
	state->save_item(NAME(state->m_palram));

	state->save_item(NAME(state->m_irq_count));
}

static MACHINE_RESET( ddenlovr )
{
	dynax_state *state = machine.driver_data<dynax_state>();

	state->m_input_sel = 0;
	state->m_dsw_sel = 0;
	state->m_keyb = 0;
	state->m_coins = 0;
	state->m_hopper = 0;

	state->m_okibank = 0;
	state->m_rongrong_blitter_busy_select = 0;
	state->m_prot_val = 0;
	state->m_prot_16 = 0;
	state->m_mmpanic_leds = 0;
	state->m_funkyfig_lockout = 0;
	state->m_palette_index = 0;
	state->m_hginga_rombank = 0;
	state->m_mjflove_irq_cause = 0;
	state->m_daimyojn_palette_sel = 0;
	state->m_irq_count = 0;

	state->m_quiz365_protection[0] = 0;
	state->m_quiz365_protection[1] = 0;
	state->m_romdata[0] = 0;
	state->m_romdata[1] = 0;

	memset(state->m_palram, 0, ARRAY_LENGTH(state->m_palram));
}

static MACHINE_START( rongrong )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 0x20, &ROM[0x010000], 0x8000);
	memory_configure_bank(machine, "bank2", 0, 8,    &ROM[0x110000], 0x1000);

	MACHINE_START_CALL(ddenlovr);
}

static MACHINE_START( mmpanic )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 8,    &ROM[0x10000], 0x8000);

	MACHINE_START_CALL(ddenlovr);
}

static MACHINE_START( funkyfig )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 0x10, &ROM[0x10000], 0x8000);
	memory_configure_bank(machine, "bank2", 0, 8,    &ROM[0x90000], 0x1000);

	MACHINE_START_CALL(ddenlovr);
}

static MACHINE_START( hanakanz )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 0x10, &ROM[0x10000], 0x8000);
	memory_configure_bank(machine, "bank2", 0, 0x10, &ROM[0x90000], 0x1000);

	MACHINE_START_CALL(ddenlovr);
}

static MACHINE_START( mjmyster )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 8,    &ROM[0x10000], 0x8000);
	memory_configure_bank(machine, "bank2", 0, 8,    &ROM[0x90000], 0x1000);

	MACHINE_START_CALL(ddenlovr);
}

static MACHINE_START( hparadis )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 8,    &ROM[0x10000], 0x8000);
	memory_configure_bank(machine, "bank2", 0, 8,    &ROM[0x50000], 0x1000);

	MACHINE_START_CALL(ddenlovr);
}

static MACHINE_START( mjflove )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 0x10, &ROM[0x10000], 0x8000);
	memory_configure_bank(machine, "bank2", 0, 8,    &ROM[0x90000], 0x1000);

	MACHINE_START_CALL(ddenlovr);
}

static MACHINE_START( sryudens )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	memory_configure_bank(machine, "bank1", 0, 0x10, &ROM[0x10000], 0x8000);
	memory_configure_bank(machine, "bank2", 0, 0x10, &ROM[0x90000], 0x1000);

	MACHINE_START_CALL(ddenlovr);
}

/***************************************************************************
                            Don Den Lover Vol.1
***************************************************************************/

static MSM6242_INTERFACE( ddenlovr_rtc_intf )
{
	DEVCB_NULL
};


static MACHINE_CONFIG_START( ddenlovr, dynax_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_24MHz / 2)
	MCFG_CPU_PROGRAM_MAP(ddenlovr_map)
	MCFG_CPU_VBLANK_INT("screen", irq1_line_hold)

	MCFG_MACHINE_START(ddenlovr)
	MCFG_MACHINE_RESET(ddenlovr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(336, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 336-1, 5, 256-16+5-1)
	MCFG_SCREEN_UPDATE_STATIC(ddenlovr)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(ddenlovr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_28_63636MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_28_63636MHz / 16)	// or /8 ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_OKIM6295_ADD("oki", XTAL_28_63636MHz / 28, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* devices */
	MCFG_MSM6242_ADD("rtc", ddenlovr_rtc_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddenlovj, ddenlovr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ddenlovj_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddenlovrk, ddenlovr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ddenlovrk_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( akamaru, ddenlovr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(akamaru_map)
MACHINE_CONFIG_END


static const ay8910_interface quiz365_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(quiz365_input_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ddenlovr_select_w)
};

static MACHINE_CONFIG_DERIVED( quiz365, ddenlovr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(quiz365_map)

	MCFG_SOUND_MODIFY("aysnd")
	MCFG_SOUND_CONFIG(quiz365_ay8910_interface)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( nettoqc, ddenlovr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(nettoqc_map)
MACHINE_CONFIG_END

/***************************************************************************
                                Rong Rong
***************************************************************************/

/* the CPU is in Interrupt Mode 2
   vector can be 0xee, 0xf8 0xfa 0xfc
   rongrong: 0xf8 and 0xfa do nothing
   quizchq: 0xf8 and 0xfa are very similar, they should be triggered by the blitter
   0xee is vblank
   0xfc is from the 6242RTC
 */
static INTERRUPT_GEN( quizchq_irq )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();
//  int scanline = param;

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
//  if (downcast<cpu_device *>(state->m_maincpu)->input_state(0))
//      return;

	device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, 0xee);
}

static WRITE_LINE_DEVICE_HANDLER( quizchq_rtc_irq )
{
	dynax_state *drvstate = device->machine().driver_data<dynax_state>();

	device_set_input_line_and_vector(drvstate->m_maincpu, 0, HOLD_LINE, 0xfc);
}

static MSM6242_INTERFACE( quizchq_rtc_intf )
{
	DEVCB_LINE(quizchq_rtc_irq)
};

static MACHINE_CONFIG_START( quizchq, dynax_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/2)	/* Verified */
	MCFG_CPU_PROGRAM_MAP(quizchq_map)
	MCFG_CPU_IO_MAP(quizchq_portmap)
	MCFG_CPU_VBLANK_INT("screen",quizchq_irq)

	MCFG_MACHINE_START(rongrong)
	MCFG_MACHINE_RESET(ddenlovr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(336, 256+22)
	MCFG_SCREEN_VISIBLE_AREA(0, 336-1, 5, 256-16+5-1)
	MCFG_SCREEN_UPDATE_STATIC(ddenlovr)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(ddenlovr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_28_63636MHz/8) // 3.579545Mhz, verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.50)

	MCFG_OKIM6295_ADD("oki", XTAL_28_63636MHz/28, OKIM6295_PIN7_HIGH) // clock frequency verified 1.022MHz, pin 7 verified high
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_MSM6242_ADD("rtc", quizchq_rtc_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( rongrong, quizchq )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(rongrong_map)
	MCFG_CPU_IO_MAP(rongrong_portmap)
MACHINE_CONFIG_END

/***************************************************************************

    Monkey Mole Panic

***************************************************************************/

/*  the CPU is in Interrupt Mode 0:

    RST 08 is vblank
    RST 18 is from the 6242RTC
    RST 20 is from the link device?
 */

static INTERRUPT_GEN( mmpanic_irq )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();
	//int scanline = param;

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	//if (downcast<cpu_device *>(state->m_maincpu)->input_state(0))
	//  return;

	device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, 0xcf); // RST 08, vblank
}


static WRITE_LINE_DEVICE_HANDLER( mmpanic_rtc_irq )
{
	dynax_state *drvstate = device->machine().driver_data<dynax_state>();

	device_set_input_line_and_vector(drvstate->m_maincpu, 0, HOLD_LINE, 0xdf); // RST 18, clock
}

static MSM6242_INTERFACE( mmpanic_rtc_intf )
{
	DEVCB_LINE(mmpanic_rtc_irq)
};


static MACHINE_CONFIG_START( mmpanic, dynax_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 8000000)
	MCFG_CPU_PROGRAM_MAP(mmpanic_map)
	MCFG_CPU_IO_MAP(mmpanic_portmap)
	MCFG_CPU_VBLANK_INT("screen",mmpanic_irq)

	MCFG_CPU_ADD("soundcpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(mmpanic_sound_map)
	MCFG_CPU_IO_MAP(mmpanic_sound_portmap)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)	// NMI by main cpu

	MCFG_MACHINE_START(mmpanic)
	MCFG_MACHINE_RESET(ddenlovr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(336, 256+22)
	MCFG_SCREEN_VISIBLE_AREA(0, 336-1, 5, 256-16+5-1)
	MCFG_SCREEN_UPDATE_STATIC(ddenlovr)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(mmpanic)	// extra layers

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("aysnd", AY8910, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_OKIM6295_ADD("oki", 1022720, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* devices */
	MCFG_MSM6242_ADD("rtc", mmpanic_rtc_intf)
MACHINE_CONFIG_END


/***************************************************************************

    Hana Kanzashi

***************************************************************************/

/*  the CPU is in Interrupt Mode 2
    vector can be 0xe0, 0xe2
    0xe0 is vblank
    0xe2 is from the 6242RTC
 */

static INTERRUPT_GEN( hanakanz_irq )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	//if (downcast<cpu_device *>(state->m_maincpu)->input_state(0))
	//  return;

	device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, 0xe0);
}

static WRITE_LINE_DEVICE_HANDLER(hanakanz_rtc_irq)
{
	dynax_state *drvstate = device->machine().driver_data<dynax_state>();

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	//if (downcast<cpu_device *>(drvstate->m_maincpu)->input_state(0))
	//  return;

	device_set_input_line_and_vector(drvstate->m_maincpu, 0, HOLD_LINE, 0xe2);
}

static MSM6242_INTERFACE( hanakanz_rtc_intf )
{
	DEVCB_LINE(hanakanz_rtc_irq)
};

static MACHINE_CONFIG_START( hanakanz, dynax_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,8000000)	// TMPZ84C015BF-8
	MCFG_CPU_PROGRAM_MAP(hanakanz_map)
	MCFG_CPU_IO_MAP(hanakanz_portmap)
	MCFG_CPU_VBLANK_INT("screen", hanakanz_irq)

	MCFG_MACHINE_START(hanakanz)
	MCFG_MACHINE_RESET(ddenlovr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(336, 256+22)
	MCFG_SCREEN_VISIBLE_AREA(0, 336-1, 5, 256-11-1)
	MCFG_SCREEN_UPDATE_STATIC(ddenlovr)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(hanakanz)	// blitter commands in the roms are shuffled around

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_OKIM6295_ADD("oki", 1022720, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* devices */
	MCFG_MSM6242_ADD("rtc", hanakanz_rtc_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hkagerou, hanakanz )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(hkagerou_portmap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjreach1, hanakanz )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(mjreach1_portmap)
MACHINE_CONFIG_END


/***************************************************************************
     Mahjong Chuukanejyo
***************************************************************************/

/*  the CPU is in Interrupt Mode 2
    vector can be 0xf8, 0xfa
    0xf8 is vblank
    0xfa is from the 6242RTC
 */
static INTERRUPT_GEN( mjchuuka_irq )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	//if (downcast<cpu_device *>(state->m_maincpu)->input_state(0))
	//  return;

	device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, 0xf8);
}

static WRITE_LINE_DEVICE_HANDLER(mjchuuka_rtc_irq)
{
	dynax_state *drvstate = device->machine().driver_data<dynax_state>();

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	//if (downcast<cpu_device *>(drvstate->m_maincpu)->input_state(0))
	//  return;

	device_set_input_line_and_vector(drvstate->m_maincpu, 0, HOLD_LINE, 0xfa);
}

static MSM6242_INTERFACE( mjchuuka_rtc_intf )
{
	DEVCB_LINE(mjchuuka_rtc_irq)
};


static MACHINE_CONFIG_DERIVED( mjchuuka, hanakanz )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(mjchuuka_portmap)
	MCFG_CPU_VBLANK_INT("screen",mjchuuka_irq)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(mjchuuka_rtc_intf)

	MCFG_SOUND_ADD("aysnd", AY8910, 1789772)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( funkyfig, mmpanic )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(funkyfig_map)
	MCFG_CPU_IO_MAP(funkyfig_portmap)
	MCFG_CPU_VBLANK_INT("screen",mjchuuka_irq)

	MCFG_MACHINE_START(funkyfig)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(mjchuuka_rtc_intf)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_IO_MAP(funkyfig_sound_portmap)

	MCFG_VIDEO_START(ddenlovr)	// no extra layers?
MACHINE_CONFIG_END


/***************************************************************************
                        Mahjong The Mysterious World
***************************************************************************/

/*  the CPU is in Interrupt Mode 2
    vector can be 0xf8, 0xfa, 0xfc
    0xf8 is vblank
    0xfa and/or 0xfc are from the blitter (almost identical)
    NMI triggered by the RTC

    To do:

    The game randomly locks up (like quizchq?) because of some lost
    blitter interrupt I guess (nested blitter irqs?). Hence the hack
    to trigger the blitter irq every frame.
 */

static TIMER_DEVICE_CALLBACK( mjmyster_irq )
{
	dynax_state *state = timer.machine().driver_data<dynax_state>();
	int scanline = param;

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	if (downcast<cpu_device *>(state->m_maincpu)->input_state(0))
		return;

	if(scanline == 245)
		device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, 0xf8);

	if(scanline == 0)
		device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, 0xfa);
}

static const ay8910_interface mjmyster_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ddenlovr_select_w)
};

static WRITE_LINE_DEVICE_HANDLER(mjmyster_rtc_irq)
{
	dynax_state *drvstate = device->machine().driver_data<dynax_state>();

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	//if (downcast<cpu_device *>(drvstate->m_maincpu)->input_state(0))
	//  return;

	device_set_input_line(drvstate->m_maincpu, INPUT_LINE_NMI, PULSE_LINE);
}

static MSM6242_INTERFACE( mjmyster_rtc_intf )
{
	DEVCB_LINE(mjmyster_rtc_irq)
};


static MACHINE_CONFIG_DERIVED( mjmyster, quizchq )

	/* basic machine hardware */
	MCFG_DEVICE_REMOVE("maincpu")

	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/2)	/* Verified */
	MCFG_CPU_PROGRAM_MAP(mjmyster_map)
	MCFG_CPU_IO_MAP(mjmyster_portmap)
	MCFG_TIMER_ADD_SCANLINE("scantimer", mjmyster_irq, "screen", 0, 1)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(mjmyster_rtc_intf)

	MCFG_MACHINE_START(mjmyster)

	MCFG_SOUND_ADD("aysnd", AY8910, 3579545)
	MCFG_SOUND_CONFIG(mjmyster_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************
                            Hanafuda Hana Ginga
***************************************************************************/

/*  the CPU is in Interrupt Mode 2
    vector can be 0xf8, 0xfa, 0xfc, 0xee
    0xf8 is vblank
    0xfa and/or 0xfc are from the blitter (almost identical)
    0xee triggered by the RTC
 */
static INTERRUPT_GEN( hginga_irq )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();
//  int scanline = param;

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise hginga would lock up. */
//  if (downcast<cpu_device *>(state->m_maincpu)->input_state(0))
//      return;

	device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, 0xf8);
}

static const ay8910_interface hginga_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	// A            B
	DEVCB_HANDLER(hginga_dsw_r),	DEVCB_NULL,								// R
	DEVCB_NULL,						DEVCB_HANDLER(ddenlovr_select_w)		// W
};

static WRITE_LINE_DEVICE_HANDLER(hginga_rtc_irq)
{
	dynax_state *drvstate = device->machine().driver_data<dynax_state>();

	/* I haven't found a irq ack register, so I need this kludge to
       make sure I don't lose any interrupt generated by the blitter,
       otherwise quizchq would lock up. */
	//if (downcast<cpu_device *>(drvstate->m_maincpu)->input_state(0))
	//  return;

	device_set_input_line_and_vector(drvstate->m_maincpu, 0, HOLD_LINE, 0xee);
}

static MSM6242_INTERFACE( hginga_rtc_intf )
{
	DEVCB_LINE(hginga_rtc_irq)
};


static MACHINE_CONFIG_DERIVED( hginga, quizchq )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hginga_map)
	MCFG_CPU_IO_MAP(hginga_portmap)
	MCFG_CPU_VBLANK_INT("screen",hginga_irq)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(hginga_rtc_intf)

	MCFG_MACHINE_START(mjmyster)

	MCFG_SOUND_ADD("aysnd", AY8910, 3579545)
	MCFG_SOUND_CONFIG(hginga_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hgokou, quizchq )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hgokou_map)
	MCFG_CPU_IO_MAP(hgokou_portmap)
	MCFG_CPU_VBLANK_INT("screen",hginga_irq)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(hginga_rtc_intf)

	MCFG_MACHINE_START(mjmyster)

	MCFG_SOUND_ADD("aysnd", AY8910, 3579545)
	MCFG_SOUND_CONFIG(hginga_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hgokbang, hgokou )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(hgokbang_portmap)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjmywrld, mjmyster )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hginga_map)
	MCFG_CPU_IO_MAP(mjmywrld_portmap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjmyuniv, quizchq )

	/* basic machine hardware */
	MCFG_DEVICE_REMOVE("maincpu")

	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/2)	/* Verified */
	MCFG_CPU_PROGRAM_MAP(mjmyster_map)
	MCFG_CPU_IO_MAP(mjmyster_portmap)
	MCFG_TIMER_ADD_SCANLINE("scantimer", mjmyster_irq, "screen", 0, 1)

	MCFG_MACHINE_START(mjmyster)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(mjmyster_rtc_intf)

	MCFG_SOUND_ADD("aysnd", AY8910, 1789772)
	MCFG_SOUND_CONFIG(mjmyster_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjmyornt, quizchq )

	/* basic machine hardware */
	MCFG_DEVICE_REMOVE("maincpu")

	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/2)	/* Verified */
	MCFG_CPU_PROGRAM_MAP(quizchq_map)
	MCFG_CPU_IO_MAP(mjmyster_portmap)
	MCFG_TIMER_ADD_SCANLINE("scantimer", mjmyster_irq, "screen", 0, 1)

	MCFG_MACHINE_START(mjmyster)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(mjmyster_rtc_intf)

	MCFG_SOUND_ADD("aysnd", AY8910, 1789772)
	MCFG_SOUND_CONFIG(mjmyster_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


static INTERRUPT_GEN( mjflove_irq )
{
	dynax_state *state = device->machine().driver_data<dynax_state>();

	state->m_mjflove_irq_cause = 1;
	device_set_input_line(state->m_maincpu, 0, HOLD_LINE);
}

static WRITE_LINE_DEVICE_HANDLER(mjflove_rtc_irq)
{
	dynax_state *drvstate = device->machine().driver_data<dynax_state>();

	drvstate->m_mjflove_irq_cause = 2;
	device_set_input_line(drvstate->m_maincpu, 0, HOLD_LINE);
}

static MSM6242_INTERFACE( mjflove_rtc_intf )
{
	DEVCB_LINE(mjflove_rtc_irq)
};

static MACHINE_CONFIG_DERIVED( mjflove, quizchq )

	/* basic machine hardware */
	MCFG_DEVICE_REMOVE("maincpu")

	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/2)	/* Verified */
	MCFG_CPU_PROGRAM_MAP(rongrong_map)
	MCFG_CPU_IO_MAP(mjflove_portmap)
	MCFG_CPU_VBLANK_INT("screen",mjflove_irq)

	MCFG_MACHINE_START(mjflove)

	MCFG_DEVICE_MODIFY("rtc")
	MCFG_DEVICE_CONFIG(mjflove_rtc_intf)

	MCFG_VIDEO_START(mjflove)	// blitter commands in the roms are shuffled around

	MCFG_SOUND_ADD("aysnd", AY8910, 28636363/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/*  It runs in IM 2, thus needs a vector on the data bus:
    0xee is vblank  */
static INTERRUPT_GEN( hparadis_irq )
{
	device_set_input_line_and_vector(device, 0, HOLD_LINE, 0xee);
}

static MACHINE_CONFIG_DERIVED( hparadis, quizchq )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hparadis_map)
	MCFG_CPU_IO_MAP(hparadis_portmap)
	MCFG_CPU_VBLANK_INT("screen", hparadis_irq)

	MCFG_MACHINE_START(hparadis)
MACHINE_CONFIG_END



static MACHINE_CONFIG_START( jongtei, dynax_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_20MHz / 2)	// ?
	MCFG_CPU_PROGRAM_MAP(hanakanz_map)
	MCFG_CPU_IO_MAP(jongtei_portmap)
	MCFG_CPU_VBLANK_INT("screen", hanakanz_irq)

	MCFG_MACHINE_START(hanakanz)
	MCFG_MACHINE_RESET(ddenlovr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(336, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 336-1, 5, 256-11-1)
	MCFG_SCREEN_UPDATE_STATIC(ddenlovr)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(hanakanz)	// blitter commands in the roms are shuffled around

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_28_63636MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_OKIM6295_ADD("oki", XTAL_28_63636MHz / 28, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* devices */
	MCFG_MSM6242_ADD("rtc", hanakanz_rtc_intf)
MACHINE_CONFIG_END

/***************************************************************************
                            Mahjong Seiryu Densetsu
***************************************************************************/

static MACHINE_CONFIG_START( sryudens, dynax_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_16MHz / 2)	// ?
	MCFG_CPU_PROGRAM_MAP(sryudens_map)
	MCFG_CPU_IO_MAP(sryudens_portmap)
	MCFG_CPU_VBLANK_INT("screen",mjchuuka_irq)

	MCFG_MACHINE_START(sryudens)
	MCFG_MACHINE_RESET(ddenlovr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.8532)	// VSync 60.8532Hz, HSync 15.2790kHz
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(336, 256+22)
	MCFG_SCREEN_VISIBLE_AREA(0, 336-1, 0+5, 256-12-1)
	MCFG_SCREEN_UPDATE_STATIC(ddenlovr)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(mjflove)	// blitter commands in the roms are shuffled around

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_28_63636MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_28_63636MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_OKIM6295_ADD("oki", XTAL_28_63636MHz / 28, OKIM6295_PIN7_HIGH)	// ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* devices */
	MCFG_MSM6242_ADD("rtc", mjchuuka_rtc_intf)
MACHINE_CONFIG_END

/***************************************************************************
                            Mahjong Daimyojin
***************************************************************************/


static MACHINE_CONFIG_START( daimyojn, dynax_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_20MHz / 2)
	MCFG_CPU_PROGRAM_MAP(hanakanz_map)
	MCFG_CPU_IO_MAP(daimyojn_portmap)
	MCFG_CPU_VBLANK_INT("screen", hanakanz_irq)

	MCFG_MACHINE_START(mjflove)
	MCFG_MACHINE_RESET(ddenlovr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.7922)	// HSync 15.4248kHz
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(336, 256+22)
	MCFG_SCREEN_VISIBLE_AREA(0, 336-1-1, 1, 256-15-1)
	MCFG_SCREEN_UPDATE_STATIC(ddenlovr)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(hanakanz)	// blitter commands in the roms are shuffled around

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_28_63636MHz / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_OKIM6295_ADD("oki", XTAL_28_63636MHz / 28, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* devices */
	MCFG_MSM6242_ADD("rtc", hanakanz_rtc_intf)
MACHINE_CONFIG_END



/***************************************************************************

Monkey Mole Panic
Nakanihon/Taito 1992
                      7001A  5563    6242
                      6295   7002
                             Z80
     8910                   5563   16MHz
     DynaX NL-001           7003              14.318MHz
                            Z80               24 MHz
          2018
                  DynaX   524256  524256       DynaX
                  1108    524256  524256       1427
                  DynaX   524256  524256       DynaX
                  1108    524256  524256       1427

     8251                      7006    7005   7004


The game asks players to slap buttons on a control panel and see mole-like creatures
get crunched on the eye-level video screen.

An on-screen test mode means the ticket dispenser can be adjusted from 1-99 tickets
and 15 score possibilities.

It also checks PCB EPROMs, switches and lamps, and the built-in income analyzer.

There are six levels of difficulty for one or two players.

The games are linkable (up to four) for competitive play.

***************************************************************************/

ROM_START( mmpanic )
	ROM_REGION( 0x50000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "nwc7002a",     0x00000, 0x40000, CRC(725b337f) SHA1(4d1f1ebc4de524d959dde60498d3f7038c7f3ed2) )
	ROM_RELOAD(               0x10000, 0x40000 )

	ROM_REGION( 0x20000, "soundcpu", 0 )	/* Z80 Code */
	ROM_LOAD( "nwc7003",      0x00000, 0x20000, CRC(4f02ce44) SHA1(9a3abd9c555d5863a2110d84d1a3f582ba9d56b9) )	// 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x280000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "nwc7004",      0x000000, 0x100000, CRC(5b4ad8c5) SHA1(a92a0bef01c71e745597ec96e7b8aa0ec26dc59d) )
	ROM_LOAD( "nwc7005",      0x100000, 0x100000, CRC(9ec41956) SHA1(5a92d725cee7052e1c3cd671b58795125c6a4ea9) )
	ROM_LOAD( "nwc7006a",     0x200000, 0x080000, CRC(9099c571) SHA1(9762612f41384602d545d2ec6dabd5f077d5fe21) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "nwc7001a",     0x00000, 0x40000, CRC(1ae3660d) SHA1(c4711f00a30f7d2c80fe241d7e0a464f0bb2555f) )
ROM_END


/***************************************************************************
Animalandia Jr.
Taito/Nakanihon, 1993

A wack-a-mole type kids game using 9 buttons

PCB Layout
----------
N7006018l1-3
|----------------------------------------------------|
|HA1384  VOL         7501S      5563  M6242  BATTERY |
|        358     PAL     M6295  7502S  32.768kHz     |
|                                                    |
|     VOL                     Z80-1                  |
|       358                             16MHz        |
|    YM2413                                          |
|J   AY3-8910                             14.31818MHz|
|A                                                   |
|M   NL-001                   5563                   |
|M                            7503S            24MHz |
|A       2018                 Z80-2                  |
|                                                    |
|                                             NL-003 |
|                  NL-004     524256  524256         |
|                             524256  524256  NL-003 |
|          DSW1(10)           524256  524256         |
|                  NL-004     524256  524256  PAL    |
|75179     DSW2(10)                                  |
|TMP82C51                   7506S    7505     7504   |
|----------------------------------------------------|
Notes:
      Z80-1 clock - 8.000MHz [16/2]
      Z80-2 clock - 3.579545MHz [14.31818/4]
      M6295 clock- 1.02272MHz [14.31818/14]. Sample rate = 2000000 / 132
      AY3-8910 clock - 1.7897725MHz [14.31818/8]
      YM2413 clock   - 3.579545MHz [14.31818/4]
      VSync -  \
      HSync -  / No reading, dead PCB

Z80 x2
OSC 24MHz, 14.31818MHz, 16MHz
Oki M6295 + YM2413 + AY-3-8910
Oki 6242 + 3.6v battery + 32.768kHz (rtc)
Toshiba TMP82C51 (USART)
Custom ICs NL-004 (x2), NL-003 (x2), NL-001
RAM 8kx8 near 7502S
RAM 8kx8 near 7503S
RAM 2kx8 near NL-001
RAM 32kx8 (x8) near NL-003 & NL-003
DIPs 10-position (x2)
PAL near 7504
PAL near 7501S

probably 7501S is damaged, I can not get a consistent read. 10 reads supplied for comparison.

***************************************************************************/

ROM_START( animaljr ) /* English version */
	ROM_REGION( 0x50000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "7502a.2e", 0x00000, 0x40000, CRC(78aa0f24) SHA1(5ae8cd27ddbd4d0d40112010d7c1ce3d55e02173) )
	ROM_RELOAD(           0x10000, 0x40000 )

	ROM_REGION( 0x20000, "soundcpu", 0 )	/* Z80 Code */
	ROM_LOAD( "7503a.8e", 0x00000, 0x20000, CRC(a7032aae) SHA1(13f61b7e631b75f7af36f670c181614631801048) )	// 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "7504.17b",  0x000000, 0x100000, CRC(b62de6a3) SHA1(62abf09b52844d3b3325e8931cb572c15581964f) )
	ROM_LOAD( "7505.17d",  0x100000, 0x080000, CRC(729b073f) SHA1(8e41fafc47adbe76452e92ab1459536a5a46784d) )
	ROM_LOAD( "7506a.17f", 0x180000, 0x080000, CRC(21fb7d86) SHA1(1323225d64903a07f180673556463df5e60039eb) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7501a.1h", 0x00000, 0x40000, CRC(52174727) SHA1(974029774eb8951d54f1eb4efa4f336e460456aa) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "n75a.2j",  0x0000, 0x0117, CRC(0191d68d) SHA1(0b792708c8e9e84a6e07485c7723376cc58f64a6) ) /* lattice GAL16V8A-25LP */
	ROM_LOAD( "n75b.15b", 0x0000, 0x0117, CRC(c6365977) SHA1(c55a5a0771aa299eec55263657f12cb3d756fac5) ) /* lattice GAL16V8A-25LP */
ROM_END

ROM_START( animaljrs ) /* Spanish version */
	ROM_REGION( 0x50000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "7502s.2e", 0x00000, 0x40000, CRC(4b14a4be) SHA1(79f7207f7311c627ece1a0d8571b4bddcdefb336) )
	ROM_RELOAD(           0x10000, 0x40000 )

	ROM_REGION( 0x20000, "soundcpu", 0 )	/* Z80 Code */
	ROM_LOAD( "7503s.8e", 0x00000, 0x20000, CRC(d1fac899) SHA1(dde2824d73b13c18b83e4c4b63fe7835bce87ea4) )	// 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "7504.17b",  0x000000, 0x100000, CRC(b62de6a3) SHA1(62abf09b52844d3b3325e8931cb572c15581964f) )
	ROM_LOAD( "7505.17d",  0x100000, 0x080000, CRC(729b073f) SHA1(8e41fafc47adbe76452e92ab1459536a5a46784d) )
	ROM_LOAD( "7506s.17f", 0x180000, 0x080000, CRC(1be1ae17) SHA1(57bf9bcd9df49cdbb1311ec9e850cb1a141e5069) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7501s.1h", 0x00000, 0x40000, BAD_DUMP CRC(59debb66) SHA1(9021722d3f8956946f102eddc7c676e1ef41574e) )
ROM_END


ROM_START( animaljrj ) /* Japanese version */
	ROM_REGION( 0x50000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "nwc_7502.2e", 0x00000, 0x40000, CRC(c526cf56) SHA1(466378125c06de1475de37c2e0b80c7522b82308) )
	ROM_RELOAD(              0x10000, 0x40000 )

	ROM_REGION( 0x20000, "soundcpu", 0 )	/* Z80 Code */
	ROM_LOAD( "nwc_7503.8e", 0x00000, 0x20000, CRC(9c27e0b6) SHA1(e904725912391a776ef22cc79e25b9c8cf90ebf6) )	// 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "nwc_7504.17b", 0x000000, 0x100000, CRC(b62de6a3) SHA1(62abf09b52844d3b3325e8931cb572c15581964f) )
	ROM_LOAD( "nwc_7505.17d", 0x100000, 0x080000, CRC(729b073f) SHA1(8e41fafc47adbe76452e92ab1459536a5a46784d) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "nwc_7501.1h", 0x00000, 0x40000, CRC(c821e589) SHA1(45ece97a1cd5114871ff07d2593057635d928959) )
ROM_END


/***************************************************************************

Quiz Channel Question (JPN ver.)
(c)1993 Nakanihon

N7311208L1-2
N73SUB

CPU:    TMPZ84C015BF-8

Sound:  YM2413
        M6295 - 1.022Mhz pin 7 HI? (unverified from jpn ver)

OSC:    16MHz
    28.6363MHz
    32.768KHz ?

Custom: NL-002 - Nakanihon
    (1108F0405) - Dynax
    (1427F0071) - Dynax

Others: M6242B (RTC?)

***************************************************************************/

ROM_START( quizchq )
	ROM_REGION( 0x118000, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "nwc7302.3e",   0x00000, 0x80000, CRC(14217f2d) SHA1(3cdffcf73e62586893bfaa7c47520b0698d3afda) )
	ROM_RELOAD(               0x10000, 0x80000 )
	ROM_LOAD( "nwc7303.4e",   0x90000, 0x80000, CRC(ffc77601) SHA1(b25c4a027e1fa4397dd86299dfe9251022b0d174) )

	ROM_REGION( 0x320000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "nwc7307.s4b",  0x000000, 0x80000, CRC(a09d1dbe) SHA1(f17af24293eea803ebb5c758bffb4519dcad3a71) )
	ROM_LOAD( "nwc7306.s3b",  0x080000, 0x80000, CRC(52d27aac) SHA1(3c38278a5ce757ca0c4a22e4de6052132edd7cbc) )
	ROM_LOAD( "nwc7305.s2b",  0x100000, 0x80000, CRC(5f50914e) SHA1(1fe5df146e028995c53a5aca896546898d7b5914) )
	ROM_LOAD( "nwc7304.s1b",  0x180000, 0x80000, CRC(72866919) SHA1(12b0c95f98c8c76a47e561e1d5035b62f1ec0789) )
	ROM_LOAD( "nwc7310.s4a",  0x200000, 0x80000, CRC(5939aeab) SHA1(6fcf63d6801cb506822a6d06b7bce45ecbb0b4dd) )
	ROM_LOAD( "nwc7309.s3a",  0x280000, 0x80000, CRC(88c863b2) SHA1(60e5098c84ffb302abce788a064c323bece9cc6b) )
	ROM_LOAD( "nwc7308.s2a",  0x300000, 0x20000, CRC(6eb5c81d) SHA1(c8e31e246e1235c045f5a881c6db43a2aff848ff) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "nwc7301.1f",   0x00000, 0x80000, CRC(52c672e8) SHA1(bc05155f4d9c711cc2ed187a4dd2207b886452f0) )	// 2 banks
ROM_END

/***************************************************************************

Quiz Channel Question (Chinese ver.)
(c)1993 Laxan (licensed from Nakanihon)

N7311208L1-2
N73SUB

CPU:    TMPZ84C015BF-8 @8mhz (16MHz/2) (verified)

Sound:  YM2413 - 3.579545MHz (28.6363/8) (verified)
        M6295 - 1.022MHz (28.6363/28); pin 7 HI (verified)

OSC:    16MHz - cpu
    28.6363MHz - ym2413 and m6295
    32.768KHz - RTC

Custom: NL-002 - Nakanihon
    (1108F0405) - Dynax
    (1427F0071) - Dynax

Others: M6242B (RTC)

***************************************************************************/
ROM_START( quizchql )
	ROM_REGION( 0x118000, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "2.rom",        0x00000, 0x80000, CRC(1bf8fb25) SHA1(2f9a62654a018f19f6783be655d992c457551fc9) )
	ROM_RELOAD(               0x10000, 0x80000 )
	ROM_LOAD( "3.rom",        0x90000, 0x80000, CRC(6028198f) SHA1(f78c3cfc0663b44655cb75928941a5ec4a57c8ba) )

	ROM_REGION( 0x420000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "4.rom",        0x000000, 0x80000, CRC(e6bdea31) SHA1(cb39d1d5e367ad2623c2bd0b2966541aa41bbb9b) )
	ROM_LOAD( "5.rom",        0x080000, 0x80000, CRC(c243f10a) SHA1(22366a9441b8317780e85065accfa59fe1cd8258) )
	ROM_LOAD( "11.rom",       0x100000, 0x80000, CRC(c9ae5880) SHA1(1bbda7293178132797dd017d71b24aba5ce57022) )
	ROM_LOAD( "7.rom",        0x180000, 0x80000, CRC(a490aa4e) SHA1(05ff9982f0fb1062701063905aeeb50f37283e18) )
	ROM_LOAD( "6.rom",        0x200000, 0x80000, CRC(fbf713b6) SHA1(3ce73fa30dc020053b313dca1587ef6dd8ba1690) )
	ROM_LOAD( "8.rom",        0x280000, 0x80000, CRC(68d4b79f) SHA1(5937760495461dbe6a12670d631754c772171289) )
	ROM_LOAD( "10.rom",       0x300000, 0x80000, CRC(d56eaf0e) SHA1(56214de0b08c7db703a9af7dfd7e2deb74f36542) )
	ROM_LOAD( "9.rom",        0x380000, 0x80000, CRC(a11d535a) SHA1(5e95f07807cd2a5a0eae6cb5c70ccf4516d65124) )
	ROM_LOAD( "12.rom",       0x400000, 0x20000, CRC(43f8e5c7) SHA1(de4c8cc0948b0ce9e1ddf4bea434a7640db451e2) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1snd.rom",     0x00000, 0x80000, CRC(cebb9220) SHA1(7a2ee750f2e608a37858b849914316dc778bcae2) )	// 2 banks
ROM_END



ROM_START( quiz365 )
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "7805.4b",  0x000000, 0x080000, CRC(70f93543) SHA1(03fb3f19b451c49359719e72baf294b2e9873307) )
	ROM_LOAD16_BYTE( "7804.4d",  0x000001, 0x080000, CRC(2ae003f4) SHA1(4aafc75a68989d3a006a5959a64d589472f17474) )
	ROM_LOAD16_BYTE( "7803.3b",  0x100000, 0x040000, CRC(10d315b1) SHA1(9f1bb57ba32152cca3b88fc3f841451b2b506a74) )
	ROM_LOAD16_BYTE( "7802.3d",  0x100001, 0x040000, CRC(6616caa3) SHA1(3b3fda61fa62c10b4d9e07e898018ffc9fab0f91) )

	ROM_REGION( 0x380000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "7810.14b", 0x000000, 0x100000, CRC(4b1a4984) SHA1(581ee032b396d65cd604f39846153a4dcb296aad) )
	ROM_LOAD( "7809.13b", 0x100000, 0x100000, CRC(139d52ab) SHA1(08d705301379fcb952cbb1add0e16a148e611bbb) )
	ROM_LOAD( "7808.12b", 0x200000, 0x080000, CRC(a09fd4a4) SHA1(016ecbf1d27a4890dee01e1966ec5efff6eb3afe) )
	ROM_LOAD( "7807.11b", 0x280000, 0x080000, CRC(988b3e84) SHA1(6c42d33c15806d1abe83994370c07ab7e446a111) )
	ROM_LOAD( "7806.10b", 0x300000, 0x080000, CRC(7f9aa228) SHA1(e5b4ece2df4d85c61af1fb9fbb8530fd3b8ef35e) )

	ROM_REGION( 0x100000, "oki", 0 )	/* Samples */
	// piggy-backed sample roms dumped as 2 separate files
	ROM_LOAD( "7801.1fu",     0x000000, 0x080000, CRC(53519d67) SHA1(c83b8504d5154c6667e25ff6e222e190ae771bc0) )
	ROM_LOAD( "7801.1fd",     0x080000, 0x080000, CRC(448c58dd) SHA1(991a4e2f82d2ee9b0839a76962c00e0848623879) )
ROM_END

ROM_START( quiz365t )
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "7805.rom", 0x000000, 0x080000, CRC(6db33222) SHA1(5f0cc9a15815252d8d5e85975ce8770717eb3ac8) )
	ROM_LOAD16_BYTE( "7804.rom", 0x000001, 0x080000, CRC(46d04ace) SHA1(b6489309d7704d2382802aa0f2f7526e367667ad) )
	ROM_LOAD16_BYTE( "7803.rom", 0x100000, 0x040000, CRC(5b7a78d3) SHA1(6ade16df301b57e4a7309834a47ca72300f50ffa) )
	ROM_LOAD16_BYTE( "7802.rom", 0x100001, 0x040000, CRC(c3238a9d) SHA1(6b4b2ab1315fc9e2667b4f8f394e00a27923f926) )

	ROM_REGION( 0x400000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "7810.rom", 0x000000, 0x100000, CRC(4b1a4984) SHA1(581ee032b396d65cd604f39846153a4dcb296aad) )
	ROM_LOAD( "7809.rom", 0x100000, 0x100000, CRC(139d52ab) SHA1(08d705301379fcb952cbb1add0e16a148e611bbb) )
	ROM_LOAD( "7808.rom", 0x200000, 0x080000, CRC(a09fd4a4) SHA1(016ecbf1d27a4890dee01e1966ec5efff6eb3afe) )
	ROM_LOAD( "7806.rom", 0x280000, 0x100000, CRC(75767c6f) SHA1(aef925dec3acfc01093d29f44e4a70f0fe28f66d) )
	ROM_LOAD( "7807.rom", 0x380000, 0x080000, CRC(60fb1dfe) SHA1(35317220b6401ccb03bb4ab7d3c0b6ab7637d82a) )

	ROM_REGION( 0x100000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7801.rom", 0x080000, 0x080000, CRC(285cc62a) SHA1(7cb3bd0ead303787964bcf7a0ecf896b6a6bfa54) )	// bank 2,3
	ROM_CONTINUE(         0x000000, 0x080000 )				// bank 0,1
ROM_END



/***************************************************************************

                                Rong Rong

Here are the proms for Nakanihon's Rong Rong
It's a quite nice Puzzle game.
The CPU don't have any numbers on it except for this:
Nakanihon
NL-002
3J3  JAPAN
For the sound it uses A YM2413

***************************************************************************/

ROM_START( rongrong )
	ROM_REGION( 0x118000, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "8002e.3e",     0x00000, 0x80000, CRC(062fa1b6) SHA1(f15a78c4192dbc56bb6ac0f92cffee88040b0a17) )
	ROM_RELOAD(               0x10000, 0x80000 )
	/* 90000-10ffff empty */

	ROM_REGION( 0x280000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "8003.8c",      0x000000, 0x80000, CRC(f57192e5) SHA1(e33f5243028520492cd876be3e4b6a76a9b20d46) )
	ROM_LOAD( "8004.9c",      0x080000, 0x80000, CRC(c8c0b5cb) SHA1(d0c99908022b7d5d484e6d1990c00f15f7d8665a) )
	ROM_LOAD( "8005e.10c",    0x100000, 0x80000, CRC(11c7a23c) SHA1(96d6b82db2555f7d0df661367a7a09bd4eaecba9) )
	ROM_LOAD( "8006e.11c",    0x180000, 0x80000, CRC(137e9b83) SHA1(5458f8982ce84990f0bc56f9269e46c691301ba1) )
	ROM_LOAD( "8007e.12c",    0x200000, 0x80000, CRC(374a1d50) SHA1(bbbbaf048b06caaca292b9e3d4bf408ba5259ad6) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "8001w.2f",     0x00000, 0x40000, CRC(8edc87a2) SHA1(87e8ad50be025263e682cbfb5623f3a35b17118f) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.1e",   0x0000, 0x0117, CRC(cf1b6e1d) SHA1(f1db4cd0636f390d745be33026b2e9e0da599d22) )
ROM_END

/***************************************************************************

 Rong Rong (Europe)
 Nakanihon

 Revision: 6.2.14

 CPU:
 1x Nakanihon NL-002-3D1 (main)
 1x oscillator 28.6363MHz
 1x Toshiba TMPZ84C015BF-8-9328ECZ
 1x OKI M6295-3372202 (sound)
 1x YM2413-9344HAAG (sound)
 1x Tpshiba TD62003AP-9348K
 1x DYNAX 1108F405-9401EAI
 1x DYNAX 4L02F2637-9337EAI
 1x DYNAX 1427F0071-9232EAI

 ROMs:
 1x M27C2001-12FI (8001W)(sound)
 1x M27C4001-12FI (8002E)(sound)
 2x TC534000AP (8003-8004)
 2x M27C4001-12FI (8005E-8007E)
 1x TMS 27C040-15 (8006E)
 1x GAL16V8B-25LP

***************************************************************************/

ROM_START( rongrongg )
	ROM_REGION( 0x118000, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "rr_8002g.rom", 0x00000, 0x80000, CRC(9a5d2885) SHA1(9ca049085d14b1cfba6bd48adbb0b883494e7d29) )
	ROM_RELOAD(               0x10000, 0x80000 )
	/* 90000-10ffff empty */

	ROM_REGION( 0x280000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "rr_8003.rom",  0x000000, 0x80000, CRC(f57192e5) SHA1(e33f5243028520492cd876be3e4b6a76a9b20d46) )
	ROM_LOAD( "rr_8004.rom",  0x080000, 0x80000, CRC(c8c0b5cb) SHA1(d0c99908022b7d5d484e6d1990c00f15f7d8665a) )
	ROM_LOAD( "rr_8005g.rom", 0x100000, 0x80000, CRC(11c7a23c) SHA1(96d6b82db2555f7d0df661367a7a09bd4eaecba9) )
	ROM_LOAD( "rr_8006g.rom", 0x180000, 0x80000, CRC(f3de77e6) SHA1(13839837eab6acf6f8d6a9ca08fe56c872d50e6a) )
	ROM_LOAD( "rr_8007g.rom", 0x200000, 0x80000, CRC(38a8caa3) SHA1(41d6745bb340b7f8708a6b772f241989aa7fa09d) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "rr_8001w.rom", 0x00000, 0x40000, CRC(8edc87a2) SHA1(87e8ad50be025263e682cbfb5623f3a35b17118f) )
ROM_END

/***************************************************************************

Rong Rong (Japan)
(c)1994 Nakanihon (Dynax)

N8010178L1

CPU   : TMPZ84C015BF-8
Sound : YM2413, M6295
OSC   : 28.6363MHz, ?(near CPU)
DIPs  : 10 position (x2)
Custom: NL-002
        1427F0071 (10D)
        4L02F2637 (10F)
        1108F0405 (10H)

ROMs  : 8001.2F      [9fc8a367] - Samples

        8002.3E      [27142684] - Main program

        8003.8C      [f57192e5] \
        8004.9C      [c8c0b5cb] |
        8005.10C     [d1e5f74c] |- Blitter data
        8006.11C     [bcbd1b0b] |
        8007.12C     [c76cbb69] /

***************************************************************************/

ROM_START( rongrongj )
	ROM_REGION( 0x118000, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "8002.3e", 0x00000, 0x80000, CRC(27142684) SHA1(4626576d032a89b558c8542f82b286e5673f8662) )
	ROM_RELOAD(          0x10000, 0x80000 )
	/* 90000-10ffff empty */

	ROM_REGION( 0x240000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "8003.8c",  0x000000, 0x80000, CRC(f57192e5) SHA1(e33f5243028520492cd876be3e4b6a76a9b20d46) )
	ROM_LOAD( "8004.9c",  0x080000, 0x80000, CRC(c8c0b5cb) SHA1(d0c99908022b7d5d484e6d1990c00f15f7d8665a) )
	ROM_LOAD( "8005.10c", 0x100000, 0x80000, CRC(d1e5f74c) SHA1(808b37a4992f27768b85eea24fd868d5c9b1e1c0) )
	ROM_LOAD( "8006.11c", 0x180000, 0x80000, CRC(bcbd1b0b) SHA1(9b2f990de495b8fafbed71e9649d715f30768f0e) )
	ROM_LOAD( "8007.12c", 0x200000, 0x40000, CRC(c76cbb69) SHA1(691133fb4d6669106ea10880757168c45661154f) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "8001.2f",  0x00000, 0x40000, CRC(9fc8a367) SHA1(bd8c9ddb3c1c8867321ca235f2d53cdc4c837907) )
ROM_END

/***************************************************************************

Netto Quiz Champion (c) Nakanihon

CPU: 68HC000
Sound: OKI6295
Other: HN46505, unknown 68 pin, unknown 100 pin (x2), unknown 64 pin (part numbers scratched off).
PLDs: GAL16L8B (x2, protected)
RAM: TC524258BZ-10 (x5), TC55257BSPL-10 (x2), TC5588P-35
XTAL1: 16 MHz
XTAL2: 28.63636 MHz

***************************************************************************/

ROM_START( nettoqc )
	ROM_REGION( 0x180000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "10305.rom", 0x000000, 0x080000, CRC(ebb14a1f) SHA1(5e4511a878d0bcede79a287fb184e912c9eb7dc5) )
	ROM_LOAD16_BYTE( "10303.rom", 0x000001, 0x080000, CRC(30c114c3) SHA1(fa9c26d465d2d919e141bbc080a04ac0f87c7010) )
	ROM_LOAD16_BYTE( "10306.rom", 0x100000, 0x040000, CRC(f19fe827) SHA1(37907bf3206af5f4613dc80b6bd91c87dd6645ab) )
	ROM_LOAD16_BYTE( "10304.rom", 0x100001, 0x040000, CRC(da1f56e5) SHA1(76c865927ee8392dd77476a248816e04e60c784a) )
	ROM_CONTINUE(                 0x100001, 0x040000 )	// 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x400000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "10307.rom", 0x000000, 0x100000, CRC(c7a3b05f) SHA1(c931670c5d14f8446404ad00d785fa73d97dedfc) )
	ROM_LOAD( "10308.rom", 0x100000, 0x100000, CRC(416807a1) SHA1(bccf746ddc9750e3956299fec5b3737a53b24c36) )
	ROM_LOAD( "10309.rom", 0x200000, 0x100000, CRC(81841272) SHA1(659c009c41ae54d330da41922c8afd1fb293d854) )
	ROM_LOAD( "10310.rom", 0x300000, 0x080000, CRC(0f790cda) SHA1(97c79b02ba95551514f8dee701bd71b53e41abf4) )
	ROM_LOAD( "10311.rom", 0x380000, 0x080000, CRC(41109231) SHA1(5e2f4684fd65dcdfb61a94099e0600c23a4740b2) )

	ROM_REGION( 0x100000, "oki", 0 )	/* Samples */
	ROM_LOAD( "10301.rom", 0x000000, 0x080000, CRC(52afd952) SHA1(3ed6d92b78552d390ee305bb216648dbf6d63daf) )
	ROM_LOAD( "10302.rom", 0x080000, 0x080000, CRC(6e2d4660) SHA1(d7924af8807f7238a7885b204a8c352ff75298b7) )
ROM_END


/***************************************************************************

Don Den Lover Vol.1 -Shiro Kuro Tsukeyo!-
(c)1995 Dynax
D1120901L8

CPU: 68000(surface scratched)
Sound: YM2413(OPLL)
       YMZ284-D(AY-3-8910 without I/O ports)
       M6295
OSC: 2x ?MHz (surface-scratched)
Video: HD46505SP-2(HD68B45SP)
Others: Battery, RTC 62421B
All custom chips, PALs, GALs are surface-scratched.

ROMs:
1121.2N      [e2b8359e] \
1122.2M      [e8619d66] -- Samples

1123.2H      [d41cbed0] \
1124.2D      [6a9ec557] -- Programs

1125.2B      [0181f53c] \
1126.2A      [17ff2df4] |
1127.3D      [9c136914] |- Blitter data
1128.3C      [757c9941] |
1129.3B      [957bc57e] /

***************************************************************************/

ROM_START( ddenlovj )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "1124.2d", 0x000000, 0x040000, CRC(6a9ec557) SHA1(e1512601910a9d06e16a20e10ab7acc96a0819bd) )
	ROM_LOAD16_BYTE( "1123.2h", 0x000001, 0x040000, CRC(d41cbed0) SHA1(5c80f6a6cf15518120f664a0446355e80eeb2a0f) )

	ROM_REGION( 0xe80000, "blitter", 0 )	/* blitter data */
	/* 000000-bfffff empty */
	ROM_LOAD( "1125.2b", 0xc00000, 0x080000, CRC(0181f53c) SHA1(64a6a2f00f81f7181700b83912033a6ee8bbf73a) )
	ROM_LOAD( "1126.2a", 0xc80000, 0x080000, CRC(17ff2df4) SHA1(7b6723e0a0f471698735a31aa19dc4ebabe35e8c) )
	ROM_LOAD( "1127.3d", 0xd00000, 0x080000, CRC(9c136914) SHA1(ff1f9a90814523cafdaa2ed36926482b1078aa89) )
	ROM_LOAD( "1128.3c", 0xd80000, 0x080000, CRC(757c9941) SHA1(31206112d4b20369b6584cae75dbe3b3e0ca9825) )
	ROM_LOAD( "1129.3b", 0xe00000, 0x080000, CRC(957bc57e) SHA1(801f5cc4e9da8b46dcd9488741585a5c8c88b51a) )

	ROM_REGION( 0x200000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1122.2m", 0x080000, 0x080000, CRC(e8619d66) SHA1(b40db5db1bd0a12cd66eb5192e53ddc5b85bd1cf) )	// bank 4, 5
	ROM_LOAD( "1121.2n", 0x100000, 0x080000, CRC(e2b8359e) SHA1(d882635370405610a1707d9e39a0d8e025ad6e22) )	// bank 2, 3
ROM_END


/***************************************************************************

Don Den Lover (Korea)
Dynax, 1995

PCB Layout
----------

Top Board

N113SUB
|-----------------|
|     PLCC44      |
|1F 1E       1B 1A|
|                 |
|                 |
|                 |
|   2E 2D 2C 2B 2A|
|                 |
|-----------------|
Notes:
      PLCC44 - Actel A1010B CPLD

Bottom Board

|-------------------------------------|
|UPC1241  BATTERY                     |
|LM358 LM358   M28    6264     68000  |
|YM2413    72421      6264            |
|ULN2003  95105           PAL         |
|J                              24MHz |
|A                           M514262  |
|M                           M514262  |
|M          28.63636MHz      M514262  |
|A          62256            M514262  |
|                                     |
|                      NL-005         |
|DSW1(8)                              |
|-------------------------------------|
Notes:
      68000 - clock 12.000MHz [24/2]
      M28   - M6295 clock 1.022727143MHz [28.63636/28]. Pin7 HIGH
      95105 - unknown DIP40 chip
      72421 - RTC IC
      YM2413- clock 3.579545MHz [28.63636/8]
      VSync - 60.8516Hz
      HSync - 15.2782kHz

Hardware info by Guru

***************************************************************************/

ROM_START( ddenlovrk )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rom.1a", 0x000000, 0x40000, CRC(868c45f8) SHA1(023ceaa30cfa03470ef005c8b739a85ae9764e15) )
	ROM_LOAD16_BYTE( "rom.1b", 0x000001, 0x40000, CRC(4fab3c90) SHA1(61a756a3ccae39f3a649371116b9d940d3b1b852) )

	ROM_REGION( 0x280000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "rom.2a", 0x000000, 0x80000, CRC(ee143d8e) SHA1(61a36c64d450209071e996b418adf416dfa68fd9) )
	ROM_LOAD( "rom.2b", 0x080000, 0x80000, CRC(58a662be) SHA1(3e2fc167bdee74ebfa63c3b1b0d822e3d898c30c) )
	ROM_LOAD( "rom.2c", 0x100000, 0x80000, CRC(f96e0708) SHA1(e910970a4203b9b1943c853e3d869dd43cdfbc2d) )
	ROM_LOAD( "rom.2d", 0x180000, 0x80000, CRC(b47e27ec) SHA1(5a36e68eb7c868ce8ca9d11bd9bcaa7f101ee64f) )
	ROM_LOAD( "rom.2e", 0x200000, 0x80000, CRC(7c7beef6) SHA1(f8631aaec7cc01cc6478f3fc95fdac51c5b5d226) )

	ROM_REGION( 0x200000, "oki", ROMREGION_ERASE )	/* Samples */
	ROM_LOAD( "rom.1e", 0x080000, 0x40000, CRC(a49318df) SHA1(d952cab857a21e7710fad5b4977b11ff3794ac4d) )	// bank 2
	ROM_CONTINUE(       0x180000, 0x40000 )																	// bank 6
	ROM_LOAD( "rom.1f", 0x040000, 0x40000, CRC(9df4f029) SHA1(a8da9905f60910437756dede66a21c8653d98ca6) )	// bank 1
	ROM_CONTINUE(       0x140000, 0x40000 )																	// bank 5
ROM_END


/***************************************************************************

Don Den Lover Vol 1
(C) Dynax Inc 1995

CPU: TMP68HC000N-12
SND: OKI M6295, YM2413 (18 pin DIL), YMZ284-D (16 pin DIL. This chip is in place where a 40 pin chip is marked on PCB,
                                     possibly a replacement for some other 40 pin YM chip?)
OSC: 28.636MHz (near large GFX chip), 24.000MHz (near CPU)
DIPS: 1 x 8 Position switch. DIP info is in Japanese !
RAM: 1 x Toshiba TC5588-35, 2 x Toshiba TC55257-10, 5 x OKI M514262-70

OTHER:
Battery
RTC 72421B   4382 (18 pin DIL)
3 X PAL's (2 on daughter-board at locations 2E & 2D, 1 on main board near CPU at location 4C)
GFX Chip - NAKANIHON NL-005 (208 pin, square, surface-mounted)

***************************************************************************/

ROM_START( ddenlovr )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "1134h.1a", 0x000000, 0x040000, CRC(43accdff) SHA1(3023d4a071fc877f8e4325e95e586739077ccb02) )
	ROM_LOAD16_BYTE( "1133h.1c", 0x000001, 0x040000, CRC(361bf7b6) SHA1(1727112284cd1dcc1ed17ccba214cb0f8993650a) )

	ROM_REGION( 0x480000, "blitter", 0 )	/* blitter data */
	/* 000000-1fffff empty */
	ROM_LOAD( "1135h.3h", 0x200000, 0x080000, CRC(ee143d8e) SHA1(61a36c64d450209071e996b418adf416dfa68fd9) )
	ROM_LOAD( "1136h.3f", 0x280000, 0x080000, CRC(58a662be) SHA1(3e2fc167bdee74ebfa63c3b1b0d822e3d898c30c) )
	ROM_LOAD( "1137h.3e", 0x300000, 0x080000, CRC(f96e0708) SHA1(e910970a4203b9b1943c853e3d869dd43cdfbc2d) )
	ROM_LOAD( "1138h.3d", 0x380000, 0x080000, CRC(633cff33) SHA1(aaf9ded832ae8889f413d3734edfcde099f9c319) )
	ROM_LOAD( "1139h.3c", 0x400000, 0x080000, CRC(be1189ca) SHA1(34b4102c6341ade03a1d44b6049ffa15666c6bb6) )

	ROM_REGION( 0x200000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1131h.1f", 0x080000, 0x080000, CRC(32f68241) SHA1(585b5e0d2d959af8b57ecc0a277aeda27e5cae9c) )	// bank 2, 3
	ROM_LOAD( "1132h.1e", 0x100000, 0x080000, CRC(2de6363d) SHA1(2000328e41bc0261f19e02323434e9dfdc61013a) )	// bank 4, 5
ROM_END


/*
Don Den Lover (bootleg)

PCB Layout
----------

|------------------------------------|
|  ROM1               TC524258       |
|     PAL         PAL TC524258       |
|      M6295          TC524258       |
|      YM2413 6264    TC524258   PAL |
|J        28MHz       TC524258       |
|A  BATTERY                          |
|M                    ACTEL          |
|M                    A1020          |
|A                               PAL |
| 32MHz               ACTEL      ROM4|
| DSW1       PAL      A1020      ROM5|
|     62256  ROM2                ROM6|
|     62256  ROM3     ACTEL      ROM7|
|      68000          A1020      ROM8|
|------------------------------------|
Notes:
      68000 clock 14.00MHz [28/2]
      YM2413 clock 3.50MHz [28/8]
      M6295 clock 1.00MHz [32/32]
      HSync 15.30kHz
      VSync 60Hz
*/
ROM_START( ddenlovrb )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rom2", 0x000000, 0x040000, CRC(cabdf78f) SHA1(789d4754c7b84964ee317b8a618f26a417f50bcc) )
	ROM_LOAD16_BYTE( "rom3", 0x000001, 0x040000, CRC(36f8d05e) SHA1(78f75175541ebf377f5375ea30d80ea91f380971) )

	ROM_REGION( 0x280000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "rom4", 0x000000, 0x080000, CRC(ee143d8e) SHA1(61a36c64d450209071e996b418adf416dfa68fd9) )
	ROM_LOAD( "rom5", 0x080000, 0x080000, CRC(58a662be) SHA1(3e2fc167bdee74ebfa63c3b1b0d822e3d898c30c) )
	ROM_LOAD( "rom6", 0x100000, 0x080000, CRC(f96e0708) SHA1(e910970a4203b9b1943c853e3d869dd43cdfbc2d) )
	ROM_LOAD( "rom7", 0x180000, 0x080000, CRC(b47e27ec) SHA1(5a36e68eb7c868ce8ca9d11bd9bcaa7f101ee64f) )
	ROM_LOAD( "rom8", 0x200000, 0x080000, CRC(7c7beef6) SHA1(f8631aaec7cc01cc6478f3fc95fdac51c5b5d226) )

	ROM_REGION( 0x200000, "oki", 0 )	/* Samples */
	ROM_LOAD( "rom1", 0x000000, 0x080000, CRC(ba4723e8) SHA1(fd32b33bd43773fed083990b59a3994f4a631b04) )
ROM_END

static DRIVER_INIT( rongrong )
{
	/* Rong Rong seems to have a protection that works this way:
        - write 01 to port c2
        - write three times to f705 (a fixed command?)
        - write a parameter to f706
        - read the answer back from f601
        - write 00 to port c2
       The parameter is read from RAM location 60d4, and the answer
       is written back there. No matter what the protection device
       does, it seems that making 60d4 always read 0 is enough to
       bypass the protection. Actually, I'm wondering if this
       version of the game might be a bootleg with the protection
       patched. (both sets need this)
     */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->nop_read(0x60d4, 0x60d4);
}

/***************************************************************************

HANAKANZASHI
(c)1996 DYNAX.INC
CPU : Z-80 (TMPZ84C015BF-8)
SOUND : MSM6295 YM2413
REAL TIME CLOCK : MSM6242

***************************************************************************/

ROM_START( hanakanz )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "50720.5b",     0x00000, 0x80000, CRC(dc40fcfc) SHA1(32c8b3d23039ac47504c881552572f2c22afa585) )
	ROM_RELOAD(               0x10000, 0x80000 )

	ROM_REGION( 0x300000, "blitter", 0 )	/* blitter data */
	ROM_LOAD16_BYTE( "50740.8b",     0x000000, 0x80000, CRC(999e70ce) SHA1(421c137b43522fbf9f3f5aa86692dc563af86880) )
	ROM_LOAD16_BYTE( "50730.8c",     0x000001, 0x80000, CRC(54e1731d) SHA1(c3f60c4412665b379b4b630ead576691d7b2a598) )
	ROM_LOAD16_BYTE( "50760.10b",    0x100000, 0x80000, CRC(8fcb5da3) SHA1(86bd4f89e860cd476a026c21a87f34b7a208c539) )
	ROM_LOAD16_BYTE( "50750.10c",    0x100001, 0x80000, CRC(0e58bf9e) SHA1(5e04a637fc81fd48c6e1626ec06f2f1f4f52264a) )
	ROM_LOAD16_BYTE( "50780.12b",    0x200000, 0x80000, CRC(6dfd8a86) SHA1(4d0c9f2028533ebe51f2963cb776bde5c802883e) )
	ROM_LOAD16_BYTE( "50770.12c",    0x200001, 0x80000, CRC(118e6baf) SHA1(8e14baa967af87a74558f80584b7d483c98112be) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "50710.1c",     0x00000, 0x80000, CRC(72ae072f) SHA1(024af2ae6aa12b7f76d12a9c589f07ec7f47e395) )	// 2 banks
ROM_END


/***************************************************************************

Hana Kagerou
(c)1996 Nakanihon (Dynax)

CPU:    KL5C80A12

Sound:  YM2413
        M6295?

OSC:    20.000MHz
        28.63636MHz

Custom: (70C160F011)


NM5101.1C   samples

NM5102.5B   prg.

NM5103.8C   chr.
NM5104.8B
NM5105.10C
NM5106.10B
NM5107.12C
NM5108.12B

***************************************************************************/

ROM_START( hkagerou )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "nm5102.5b",    0x00000, 0x80000, CRC(c56c0856) SHA1(9b3c17c80498c9fa0ea91aa876aa4853c95ebb8c) )
	ROM_RELOAD(               0x10000, 0x80000 )

	ROM_REGION( 0xe80000, "blitter", 0 )	/* blitter data */

	ROM_LOAD16_BYTE( "nm5104.8b",    0xc00000, 0x080000, CRC(e91dd92b) SHA1(a4eb8a6237e63639da5fc1bc504c8dc2aee99ff5) )
	ROM_LOAD16_BYTE( "nm5103.8c",    0xc00001, 0x080000, CRC(4d4e248b) SHA1(f981ba8a05bac59c665fb0fd201ea8ff3bd87a3c) )
	ROM_LOAD16_BYTE( "nm5106.10b",   0xd00000, 0x080000, CRC(0853c32d) SHA1(120094d439f6bee05681e5d22998616639412011) )
	ROM_LOAD16_BYTE( "nm5105.10c",   0xd00001, 0x080000, CRC(f109ec10) SHA1(05b86f7e02329745b6208941d5ca02d392e8526f) )
	ROM_LOAD16_BYTE( "nm5108.12b",   0xe00000, 0x040000, CRC(d0a99b19) SHA1(555ba04f13e6f372f2b5fd6b6bafc9de65c78505) )
	ROM_LOAD16_BYTE( "nm5107.12c",   0xe00001, 0x040000, CRC(65a0ebbd) SHA1(81c108ed647b8f8c2903c4b01c8bc314ecfd9796) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "nm5101.1c",    0x00000, 0x80000, CRC(bf7a397e) SHA1(163dfe68873bfcdf28bf11f235b3ca17e8bbf02d) )	// 2 banks
ROM_END


/***************************************************************************

Mahjong Reach Ippatsu
(c)1998 Nihon System/Dynax

CPU:   KL5C80A12

Sound: YM2413
       M6295

OSC:   20.000MHz
       28.63636MHz
       32.768KHz

Custom: (70C160F011)
Others: M6242B (RTC)


52601.1C    samples

52602-N.5B  prg.

52603.8C    chr.
52604.8B
52605.10C
52606.10B
52607.12C
52608.12B

***************************************************************************/

ROM_START( mjreach1 )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "52602-n.5b",   0x00000, 0x80000, CRC(6bef7978) SHA1(56e38448fb03e868094d75e5b7de4e4f4a4e850a) )
	ROM_RELOAD(               0x10000, 0x80000 )

	ROM_REGION( 0x500000, "blitter", 0 )	/* blitter data */
	ROM_LOAD16_BYTE( "52604.8b",     0x000000, 0x100000, CRC(6ce01bb4) SHA1(800043d8203ab5560ed0b24e0a4e01c14b6a3ac0) )
	ROM_LOAD16_BYTE( "52603.8c",     0x000001, 0x100000, CRC(16d2c169) SHA1(3e50b1109c86d0e8f931ce5a3abf20d807ebabba) )
	ROM_LOAD16_BYTE( "52606.10b",    0x200000, 0x100000, CRC(07fe5dae) SHA1(221ec21c2d84497af5b769d7409f8775be933783) )
	ROM_LOAD16_BYTE( "52605.10c",    0x200001, 0x100000, CRC(b5d57163) SHA1(d6480904bd72d298d48fbcb251b902b0b994cab1) )
	ROM_LOAD16_BYTE( "52608.12b",    0x400000, 0x080000, CRC(2f93dde4) SHA1(8efaa920e485f50ef7f4396cc8c47dfbfc97bd01) )
	ROM_LOAD16_BYTE( "52607.12c",    0x400001, 0x080000, CRC(5e685c4d) SHA1(57c99fb791429d0edb7416cffb4d1d1eb34a2813) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "52601.1c",     0x00000, 0x80000, CRC(52666107) SHA1(1e1c17b1da7ded5fc52422c7e569ef02af1ee11d) )	// 2 banks
ROM_END

/***************************************************************************

Mahjong Chuukanejyo
Dynax, 1995

PCB Layout
----------
D11107218L1
|-----------------------------------------------|
|10WAY           18WAY          D12101 5.5V_BATT|
|          358     358        6606              |
|      VOL    6868A                             |
|                         16MHz                 |
|           95101                   62256       |
|                        TMPZ84C015F-6          |
|                                D12102         |
|2                                        3631  |
|8                                              |
|W                                  PAL         |
|A            28.322MHz                         |
|Y                                              |
|                          PAL                  |
|             70C160F009                        |
|                           D12103      D12104  |
|              TC524256Z-12                     |
|              TC524256Z-12 D12105      D12106  |
|DIP1     DIP2 TC524256Z-12                     |
|DIP3     DIP4 TC524256Z-12 D12107      D12108  |
|-----------------------------------------------|
Notes:
      Main CPU is Toshiba TMPZ84C015F-6 (QFP100)
      95101 - Unknown 40 pin DIP, maybe equivalent to AY-3-8910?
      6868A - Unknown 18 pin DIP, maybe some other sound related chip or a PIC?
      3631  - Unknown 18 pin DIP, maybe RTC or a PIC?
      6606  - Unknown QFP44, probably OKI M6295?
      70C160F009 - QFP208 Dynax Custom


***************************************************************************/

ROM_START( mjchuuka )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "d12102.5b", 0x00000, 0x80000, CRC(585a0a8e) SHA1(94b3eede36117fe0a34b61454484c72cd7f0ce6a) )
	ROM_RELOAD(            0x10000, 0x80000 )

	ROM_REGION( 0x300000, "blitter", ROMREGION_ERASEFF )	/* blitter data */
	ROM_LOAD16_BYTE( "d12103.11c", 0x000000, 0x080000, CRC(83bfc841) SHA1(36547e737244f95004c598adeb46cebce9ab3231) )
	ROM_LOAD16_BYTE( "d12104.11a", 0x000001, 0x080000, CRC(1bf6220a) SHA1(ea18fdf6e1298a3b4c91fbf6219b1edcfecaeca3) )
	ROM_LOAD16_BYTE( "d12105.12c", 0x100000, 0x080000, CRC(3424c8ac) SHA1(ee48622b478d39c6bdb5a18cab204e14f7d54f7a) )
	ROM_LOAD16_BYTE( "d12106.12a", 0x100001, 0x080000, CRC(9052bd09) SHA1(3e8e32dea6c0cea895b7f16883e500e487689e72) )
	ROM_LOAD16_BYTE( "d12107.13c", 0x280000, 0x020000, CRC(184afa94) SHA1(57566123a6dde661770740ad7a6c364c7ef5de86) )	// 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "d12108.13a", 0x280001, 0x020000, CRC(f8e8558a) SHA1(69e64c83945c6462b704b6d9d0250c9d98f66859) )	// 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "d12101.1b", 0x00000, 0x80000, CRC(9759c65e) SHA1(cf098c07616b6d2a2ba10ff6ae0006442b675326) )
ROM_END


/***************************************************************************

Mahjong The Dai Chuuka Ken (China Version)
Dynax, 1995

PCB Layout
----------

D11107218L1 DYNAX INC. NAGOYA JAPAN
|-----------------------------------------------------|
|10-WAY              18-WAY                  1    5.5V|
|                               6606         x        |
|   MB3712  VOL  358                                  |
|                358                                  |
|                               16MHz        43256    |
|                 6868A                               |
|              95101            Z84C015      2        |
|                                                     |
|2                                              3631  |
|8                                                    |
|W                                                    |
|A                                         PAL        |
|Y                     28.322MHz      PAL             |
|                                                     |
|                         |---------|                 |
|                         |NAKANIHON|                 |
|                         |70C160F009   3       4     |
|                 44C251  |         |                 |
|                 44C251  |         |   5       6     |
| DSW1     DSW2   44C251  |---------|                 |
| DSW3     DSW4   44C251                7       8     |
|-----------------------------------------------------|
Notes:
      PCB uses common 10-way/18-way and 28-way Mahjong pinouts
      5.5V    - Battery
      6606    - Compatible to M6295 (QFP44)
      6868A   - compatible to YM2413 (DIP18)
      3631    - Unknown DIP18 chip (maybe RTC?)
      Z84C015 - Toshiba TMPZ84C015BF-6 Z80 compatible CPU (clock input 16.0MHz)
      44C251  - Texas Instruments TMS44C251-12SD 256k x4 Dual Port VRAM (ZIP28)
      95101   - Winbond 95101, compatible to AY-3-8910 (DIP40)
      43256   - NEC D43256 32k x8 SRAM (DIP28)
      70C160F009 - Custom Dynax graphics generator (QFP160)
      All DIPSW's have 10 switches per DIPSW
      All ROMs are 27C040
                          1   - Sound samples
                          2   - Main program
                          3,4 - Graphics
                          5-8 - unused DIP32 sockets

      The same PCB is used with 'Mahjong Zhong Hua Er Nu', with ROM locations
      as follows....
                    1 - D1111-A.1B
                    2 - D12102.5B
                    3 - D12103.11C
                    4 - D12104.11A
                    5 - D12105.12C
                    6 - D12106.12A
                    7 - D12107.13C
                    8 - D12108.13A

***************************************************************************/

ROM_START( mjdchuka )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "2.5b", 0x00000, 0x80000, CRC(7957b4e7) SHA1(8b76c15694e42ff0b2ec5aeae059bf342f6bf476) )
	ROM_RELOAD(       0x10000, 0x80000 )

	ROM_REGION( 0x100000, "blitter", ROMREGION_ERASEFF )	/* blitter data */
	ROM_LOAD16_BYTE( "3.11c", 0x000000, 0x080000, CRC(c66553c3) SHA1(6e5380fdb97cc8b52986f3a3a8cac43c0f38cf54) )
	ROM_LOAD16_BYTE( "4.11a", 0x000001, 0x080000, CRC(972852fb) SHA1(157f0a772bf060efc39033b10e63a6cb1022edf6) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1.1b", 0x00000, 0x80000, CRC(9759c65e) SHA1(cf098c07616b6d2a2ba10ff6ae0006442b675326) )
ROM_END


/***************************************************************************

The First Funky Fighter
Nakanihon, 1994

PCB Layout
----------

N7403208L-2
|------------------------------------------------------------------|
|    VR1                7401          7402  32.768kHz M6242 3V_BATT|
|    VR2          358         PAL        M6295  TC55257  PAL       |
|       YM2413          TC5563                             16MHz   |
|                 358                          7403                |
|       YM2149          Z80                         TMPZ84C015BF-8 |
|                                                                  |
|J                                                                 |
|A                                                                 |
|M          NL-002      PAL                                        |
|M                                                                 |
|A                                                     DSW(10)     |
|                                                                  |
|                                                      DSW(10)     |
|                                                                  |
|       TC5588                           28.6363MHz                |
|                                          |-ROM-sub-board-N73RSUB-|
|                                          |                       |
| DSW(4)                                   |NL-005         PAL     |
|       SN75179                            |                       |         Sub-board contains 12 sockets.
|                                          |        7404   7411 |----------- Only these 3 are populated.
|                                          |        7405   7410 /  |
|DB9   OMRON              NL-006     TC524258BZ-10  7406   7409/   |
|      G6A-474P        TC524258BZ-10 TC524258BZ-10  7407           |
|                      TC524258BZ-10 TC524258BZ-10  7408           |
|DB9                                 TC524258BZ-10         PAL     |
|                                          |             (on sub)  |
|------------------------------------------|-----------------------|

the second halves of 7408.13b, 7409.4b, 7410.3b and 7411.2b are identical

***************************************************************************/

ROM_START( funkyfig )
	ROM_REGION( 0x90000 + 0x1000*8, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "7403.3c",      0x00000, 0x80000, CRC(ad0f5e14) SHA1(82de58d7ba35266f2d96503d72487796a9693996) )
	ROM_RELOAD(               0x10000, 0x80000 )

	ROM_REGION( 0x20000, "soundcpu", 0 )	/* Z80 Code */
	ROM_LOAD( "7401.1h",      0x00000, 0x20000, CRC(0f47d785) SHA1(d57733db6dcfb4c2cdaad04b5d3f0f569a0e7461) )	// 1xxxxxxxxxxxxxxxx = 0xFF

ROM_REGION( 0x500000, "blitter", ROMREGION_ERASE00 )	/* blitter data */
	ROM_LOAD( "7404.8b",      0x000000, 0x080000, CRC(aa4ddf32) SHA1(864890795a238ab34a85ca55a387d7e5efafccee) )			// \ 7e6f +
	ROM_LOAD( "7405.9b",      0x080000, 0x080000, CRC(fc125bd8) SHA1(150578f67d89be59eeeb811c159a789e5e9c993e) )			// / 35bb = b42a OK
	ROM_LOAD( "7406.10b",     0x100000, 0x080000, BAD_DUMP CRC(04a214b1) SHA1(af3e652377f5652377c7dedfad7c2677695eaf46) )	// \ af08 +
	ROM_LOAD( "7407.11b",     0x180000, 0x080000, BAD_DUMP CRC(635d4052) SHA1(7bc2f20d633c69352fc2d5634349c83055c99408) )	// / 6d64 = 1c6c ERR (should be 1c68!)
	ROM_LOAD( "7409.4b",      0x200000, 0x100000, CRC(064082c3) SHA1(26b0eec56b06365740b213b34e33a4b94ebc1d25) )			// \ 15bd +
	ROM_LOAD( "7410.3b",      0x280000, 0x100000, CRC(0ba67874) SHA1(3d984c77a843501e1075cadcc27820a35410ea3b) )			// / 2e4c = 4409 OK
	ROM_LOAD( "7408.13b",     0x300000, 0x100000, CRC(9efe4c60) SHA1(6462dca2af38517639bd2f182e68b7b1fc98a312) )			// 0f46 + 1825 = 276b OK
	ROM_LOAD( "7411.2b",      0x400000, 0x100000, CRC(1e9c73dc) SHA1(ba64de6168dc626dc89d38b3f9d8991163f5e63e) )			// f248 + 1825 OK (first half)

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7402.1e",      0x000000, 0x040000, CRC(5038cc34) SHA1(65618b232a6592ad36f4abbaa40625c208a015fd) )
ROM_END

/***************************************************************************

The Mysterious World
(c) 1994 DynaX

Board has a sticker labeled D7707308L1
The actual PCB is printed as D7107058L1-1

Most all chips are surface scratched

OSC: 24.000MHz, 14.318MHz
4 x 10 Switch Dipswitch
1 4 Switch Dipswitch
VR1, VR2 & Reset Switch
3.6V Ni/CD Battery
OKI M6242B - Real Time Clock

56 pin Non-JAMMA Connector
20 pin unknown Connector
36 pin unknown Connector

Sound Chips:
K-665 (OKI M6295)
YM2149F
YM2413

***************************************************************************/

ROM_START( mjmyster )
	ROM_REGION( 0x90000 + 0x1000*8, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "77t2.c3", 0x00000, 0x40000, CRC(b1427cce) SHA1(1640f5bb6275cce92e38cf3e0c788b4e65606459) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0x1a0000, "blitter", ROMREGION_ERASE00 )	/* blitter data */
	ROM_LOAD( "77t6.b12", 0x000000, 0x080000, CRC(a287589a) SHA1(58659dd7e019d1d32efeaec548c84a7ded637c50) )
	ROM_LOAD( "77t5.b11", 0x080000, 0x080000, CRC(a3475059) SHA1(ec86dcea3314b65d391a970680c021899c16449e) )
	ROM_LOAD( "77t4.b10", 0x100000, 0x080000, CRC(f45c24d6) SHA1(0eca68f2ca5722717f27ac0839359966daa2715b) )
	ROM_LOAD( "77t3.b9",  0x180000, 0x020000, CRC(8671165b) SHA1(23fad112909e82ac9d25dbb69bf6334f30fa6540) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "77t1.d1", 0x000000, 0x020000, CRC(09b7a9b2) SHA1(64d9ccbb726bb6c5b362afc92bca2e3db87fd454) )
ROM_END


/***************************************************************************

Mahjong The Mysterious World
Dynax, 1994

PCB Layout
----------

(no number)
|-------------------------------------------------------|
|MB3712  VOL                1.1E   M6242B               |
|                           K-665  32.768kHz PAL        |
|                    YM2413        62256                |
|                                  2.3D                 |
|           DSW5(8)    YM2149               TMPZ84C015  |
|                                                       |
|M                                                      |
|A                                                 16MHz|
|H                   PAL                                |
|J                                                      |
|O     NL-002                                           |
|N                   PAL                                |
|G                                                      |
|                                      28.636MHz        |
|DSW1(10)        CY7C185                                |
|                                                       |
|DSW2(10)  1108F0405       ***       1427F0071   3.10B  |
|                                                       |
|DSW3(10)                                        4.11B  |
|           TC524256Z-10  TC524256Z-10                  |
|DSW4(10)   TC524256Z-10  TC524256Z-10  PAL PAL  5.12B  |
|-------------------------------------------------------|
Notes:
      TMPZ84C015   - Toshiba TMPZ84C015F-6 (QFP100). Clocks 16.000MHz (pins 65 & 66), 8.000MHz (pins 68 & 69)
      62256        - 32k x8 SRAM (DIP28)
      TC524256Z-10 - Toshiba TC524256Z-10 256k x4 Dual Port VRAM (ZIP28)
      1427F0071    \ Dynax Custom ICs
      1108F0405    /
      NL-002       /
      ***          - Unknown QFP100 (surface scratched)
      K-665        - == Oki M6295 (QFP44). Clock 1.02272MHz [28.636/28]. pin 7 = high
      YM2149       - Clock 3.579545MHz [28.636/8]
      YM2413       - Clock 3.579545MHz [28.636/8]
      VSync        - 61Hz
      HSync        - 15.27kHz

***************************************************************************/

ROM_START( mjmywrld )
	ROM_REGION( 0x90000 + 0x1000*8, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "2.3d", 0x00000, 0x40000, CRC(a1ff31fa) SHA1(f132aaf59570cbbd2c4eff3ee7bd4cec26ce2fbb) )
	ROM_RELOAD(       0x10000, 0x40000 )

	ROM_REGION( 0x1a0000, "blitter", ROMREGION_ERASE00 )	/* blitter data */
	ROM_LOAD( "5.12b", 0x000000, 0x100000, CRC(a1f26722) SHA1(604780c1df622a1fb05ea8175acfa774cbe9f6e1) )
	ROM_LOAD( "4.11b", 0x100000, 0x080000, CRC(f45c24d6) SHA1(0eca68f2ca5722717f27ac0839359966daa2715b) )
	ROM_LOAD( "3.10b", 0x180000, 0x020000, CRC(8671165b) SHA1(23fad112909e82ac9d25dbb69bf6334f30fa6540) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1.1e", 0x00000, 0x20000, CRC(09b7a9b2) SHA1(64d9ccbb726bb6c5b362afc92bca2e3db87fd454) )
ROM_END


/***************************************************************************

Hanafuda Hana Ginga
Dynax, 1994

PCB Layout
----------

D8102048L1 (almost same PCB as The Mysterious World)
|-------------------------------------------------------|
|MB3712  VOL      DSW1(10)      M6242B  62256      SW   |
|                             K-665  32.768kHz          |
|                 DSW2(10)                      BATTERY |
|                                  8101.2B              |
| DSW5(4)         DSW3(10)    PAL           TMPZ84C015  |
|                                                       |
|M        YM2413  DSW4(10)                              |
|A                                                 16MHz|
|H        YM2149                                        |
|J                                                      |
|O     NL-002                                 PAL       |
|N                                                      |
|G                                            8102.9A   |
|                                 28.636MHz             |
|                CY7C185                      8103.10A  |
|                                                       |
|          1108F0405       ***     1427F0071  8104.11A  |
|                                                       |
|                                                       |
|         TC524256Z-10  TC524256Z-10                    |
|         TC524256Z-10  TC524256Z-10  PAL PAL           |
|-------------------------------------------------------|
Notes:
      TMPZ84C015   - Toshiba TMPZ84C015F-6 (QFP100). Clocks 16.000MHz (pins 65 & 66), 8.000MHz (pins 68 & 69)
      62256        - 32k x8 SRAM (DIP28)
      TC524256Z-10 - Toshiba TC524256Z-10 256k x4 Dual Port VRAM (ZIP28)
      1427F0071    \ Dynax Custom ICs
      1108F0405    /
      NL-002       /
      ***          - Unknown QFP100 (surface scratched)
      K-665        - == Oki M6295 (QFP44). Clock 1.02272MHz [28.636/28]. pin 7 = high
      YM2149       - Clock 3.579545MHz [28.636/8]
      YM2413       - Clock 3.579545MHz [28.636/8]
      VSync        - 61Hz
      HSync        - 15.27kHz
      SW           - This resets the PCB and also clears the RAM (clears credits etc)
                     It is used on almost all Mahjong games by Dynax and IGS

***************************************************************************/

ROM_START( hginga )
	ROM_REGION( 0x90000 + 0x1000*8, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "8101.2b", 0x00000, 0x40000, CRC(77a64b71) SHA1(3426998746c834435ff10a8d1c6502ea64a5f2e2) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0x180000, "blitter", ROMREGION_ERASEFF )	/* blitter data */
	ROM_LOAD( "8102.9a",  0x000000, 0x80000, CRC(0074af23) SHA1(39cd978bcc34b27fc896094cf2dd3b7d4596ab00) )
	ROM_LOAD( "8103.10a", 0x080000, 0x80000, CRC(a3a4ecb5) SHA1(08264cf131fd4c02d8b5925564cf8daa56e0bbc2) )
	ROM_LOAD( "8104.11a", 0x100000, 0x20000, CRC(24513af9) SHA1(ee1f440b64c1f8c1efc6f0c60e25cab257407865) )
	ROM_RELOAD(           0x120000, 0x20000 )
	ROM_RELOAD(           0x140000, 0x20000 )
	ROM_RELOAD(           0x160000, 0x20000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASEFF )	/* Samples - none */
ROM_END


/***************************************************************************

Mahjong The Mysterious Orient
Dynax, 1994

PCB Layout
----------

D7107058L1-1
|-------------------------------------------------------|
|MB3712  VOL             7101.1E   M6242B               |
|                           K-665  32.768kHz PAL        |
|                    YM2413        62256                |
|                                  7102.3D              |
|           DSW5(4)    YM2149               TMPZ84C015  |
|                                                       |
|M                                                      |
|A                                           14.31818MHz|
|H                   PAL                                |
|J                                                      |
|O     NL-002                                           |
|N                   PAL                                |
|G                                                      |
|                                                       |
|DSW1(10)                  6116                         |
|                                                       |
|DSW2(10)  1108F0405               1427F0071   7103.10B |
|                                                       |
|DSW3(10)                  24MHz               7104.11B |
|          TC524256Z-10  TC524256Z-10                   |
|DSW4(10)  TC524256Z-10  TC524256Z-10 PAL PAL  7105.12B |
|-------------------------------------------------------|
Notes:
      TMPZ84C015   - Toshiba TMPZ84C015F-6 (QFP100). Clocks 14.31818 (pins 65 & 66), 7.15909MHz (pins 68 & 69)
      62256        - 32k x8 SRAM (DIP28)
      TC524256Z-10 - Toshiba TC524256Z-10 256k x4 Dual Port VRAM (ZIP28)
      1427F0071    \ Dynax Custom ICs
      1108F0405    /
      NL-002       /
      K-665        - == Oki M6295 (QFP44). Clock 1.02272MHz [14.31818/14]. pin 7 = high
      YM2149       - Clock 1.7897725MHz [14.31818/8]
      YM2413       - Clock 3.579545MHz [14.31818/4]
      VSync        - 61Hz
      HSync        - 15.27kHz

***************************************************************************/

ROM_START( mjmyornt )
	ROM_REGION( 0x90000 + 0x1000*8, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "7102.3d", 0x00000, 0x40000, CRC(058f779b) SHA1(97253a86b2600c295f67e566ee3c0aa693ed117e) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0x1a0000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "7105.12b", 0x000000, 0x100000, CRC(35ca0079) SHA1(41b950b6fb9b216671d55dc399acb058ec80391f) )
	ROM_LOAD( "7104.11b", 0x100000, 0x080000, CRC(6d0fd29a) SHA1(9b31668acf1790d9aecd1f8e8c0cb52a7a625d2d) )
	ROM_LOAD( "7103.10b", 0x180000, 0x020000, CRC(88511487) SHA1(eae3008ecfcfa9aed667e69742b91d8e9f7302ec) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "7101.1e", 0x00000, 0x20000, CRC(09b7a9b2) SHA1(64d9ccbb726bb6c5b362afc92bca2e3db87fd454) )
ROM_END


/***************************************************************************

Mahjong The Mysterious Universe
Dynax, 1994

PCB Layout
----------

D7107058L1-1
|-------------------------------------------------------|
|MB3712  VOL                1.1E   M6242B               |
|                           K-665  32.768kHz PAL        |
|                    YM2413        62256                |
|                                  2.3D                 |
|           DSW5(8)    YM2149               TMPZ84C015  |
|                                                       |
|M                                                      |
|A                                           14.31818MHz|
|H                   PAL                                |
|J                                                      |
|O     NL-002                                           |
|N                   PAL                                |
|G                                                      |
|                                                       |
|DSW1(10)                  6116                  3.9B   |
|                                                       |
|DSW2(10)  1108F0405                 1427F0071   4.10B  |
|                                                       |
|DSW3(10)                  24MHz                 5.11B  |
|           TC524256Z-10  TC524256Z-10                  |
|DSW4(10)   TC524256Z-10  TC524256Z-10  PAL PAL  6.12B  |
|-------------------------------------------------------|
Notes:
      TMPZ84C015   - Toshiba TMPZ84C015F-6 (QFP100). Clocks 14.31818 (pins 65 & 66), 7.15909MHz (pins 68 & 69)
      62256        - 32k x8 SRAM (DIP28)
      TC524256Z-10 - Toshiba TC524256Z-10 256k x4 Dual Port VRAM (ZIP28)
      1427F0071    \ Dynax Custom ICs
      1108F0405    /
      NL-002       /
      K-665        - == Oki M6295 (QFP44). Clock 1.02272MHz [14.31818/14]. pin 7 = high
      YM2149       - Clock 1.7897725MHz [14.31818/8]
      YM2413       - Clock 3.579545MHz [14.31818/4]
      VSync        - 61Hz
      HSync        - 15.27kHz

***************************************************************************/

ROM_START( mjmyuniv )
	ROM_REGION( 0x90000 + 0x1000*8, "maincpu", 0 )	/* Z80 Code + space for banked RAM */
	ROM_LOAD( "2.3d", 0x00000, 0x40000, CRC(3284d714) SHA1(be2d5c5129ba9d689e030cb53bc30ed01c941703) )
	ROM_RELOAD(       0x10000, 0x40000 )

	ROM_REGION( 0x600000, "blitter", 0 )	/* blitter data */
	// gap
	ROM_LOAD( "5.11b", 0x400000, 0x80000, CRC(a287589a) SHA1(58659dd7e019d1d32efeaec548c84a7ded637c50) )
	ROM_LOAD( "6.12b", 0x480000, 0x80000, CRC(a3475059) SHA1(ec86dcea3314b65d391a970680c021899c16449e) )
	ROM_LOAD( "3.9b",  0x500000, 0x80000, CRC(f45c24d6) SHA1(0eca68f2ca5722717f27ac0839359966daa2715b) )
	ROM_LOAD( "4.10b", 0x580000, 0x80000, CRC(57fb94cc) SHA1(b0c7bd3fda19f877ab908c666aef79e208c5bfc3) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1.1e", 0x00000, 0x20000, CRC(09b7a9b2) SHA1(64d9ccbb726bb6c5b362afc92bca2e3db87fd454) )
ROM_END

/***************************************************************************

Panel & Variety Akamaru Q Jousyou Dont-R
(c)1996 Dynax (distributed by B&F)

CPU: TMP68HC000N-12
Sound: YM2413, YMZ284-D, M6295 (VRx2, 1 for BGM, 1 for Voice)
OSC: 24.00000MHz (near CPU), 28.63636MHz (near sound section)
RTC: 62421B
Custom: NAKANIHON NL-005
PLD: Actel A1010B (printed NM500)

ROMs (on subboard):
50101.1H (TMS 27C040)
50102.1G (TMS 27C040)
50103.1F (TMS 27C040)
50104.1E (TMS 27C040)
50105.1D (TMS 27C040)
50106.1C (TMS 27C040)
50107.1B (TMS 27C040)
50108.1A (TMS 27C040)
50109.2H (TC538000)
50110.3H (TC538000)
50111.4H (TMS 27C040)
50112.2D (TC538000)
50113.3D (TMS 27C040)
50114.4D (TMS 27C040)

***************************************************************************/

ROM_START( akamaru )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "50107.1b", 0x000000, 0x080000, CRC(8364d627) SHA1(ed445561b3a35e6445d1074000621554a6f26fc4) )
	ROM_LOAD16_BYTE( "50105.1d", 0x000001, 0x080000, CRC(42ff4bec) SHA1(4bab20706542056d39dfcd91314523bf0f7fff07) )
	ROM_LOAD16_BYTE( "50108.1a", 0x100000, 0x080000, CRC(1520ecad) SHA1(eaf44511148252eac0c7a7aab9bd689f87e5a40f) )
	ROM_LOAD16_BYTE( "50106.1c", 0x100001, 0x080000, CRC(8e081747) SHA1(ef7fb469455671ca7982c2455e8cb113c2750e30) )

	ROM_REGION( 0x480000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "50109.2h", 0x000000, 0x100000, CRC(cdff154d) SHA1(d9cdf159cd55fef0dd1abe07c2f19f05f63b2d1e) )
	ROM_LOAD( "50112.2d", 0x100000, 0x100000, CRC(7fdd9853) SHA1(d4ada01fa49b2932d02df5eb3d3e7eaad535df0b) )
	ROM_LOAD( "50110.3h", 0x200000, 0x100000, CRC(06ca4d87) SHA1(c80708df3613d36950ba5cc98d36336533ee7699) )
	ROM_LOAD( "50113.3d", 0x300000, 0x080000, CRC(c9a571cd) SHA1(6493d458a8adc0774d6c0ff67ba272a8f6e9ce07) )
	ROM_LOAD( "50111.4h", 0x380000, 0x080000, CRC(b2de7a3c) SHA1(12b3e59f09dbad35a7a98e55ba64041cdb341488) )
	ROM_LOAD( "50114.4d", 0x400000, 0x080000, CRC(2af39dfd) SHA1(61cfced3807a80be3233d1df3eedad25b796a9cf) )

	ROM_REGION( 0x300000, "oki", 0 )	/* Samples */
	// bank 0, 1
	ROM_LOAD( "50101.1h", 0x080000, 0x080000, CRC(6bef6cab) SHA1(bd1c7e06ac4fc9de368ad90f0a9fc602024eda35) )	// bank 2, 3
	ROM_LOAD( "50102.1g", 0x100000, 0x080000, CRC(056ac348) SHA1(26c34692dc858928268299743857c69e00eb969d) )	// bank 4, 5
	// bank 6, 7
	ROM_LOAD( "50104.1e", 0x200000, 0x080000, CRC(790f18c1) SHA1(72c1c6f711267e1b57dedad04bac44e3e0829725) )	// bank 8, 9
	ROM_LOAD( "50103.1f", 0x280000, 0x080000, CRC(aff3a753) SHA1(1ee4464107531d90a1decb85c5a0fb937dd6706b) )	// bank 10, 11
ROM_END

/***************************************************************************

Mahjong Fantasic Love
Nakanihon, 1996

PCB Layout
----------
no number (scratched off) Looks like Don Den Lover h/w
|--------------------------------------------------|
|UPC1242H  BATTERY                P1               |
|                 M6295      TC55257               |
|                                          *       |
|                72421B_RTC  TC55257               |
|YM2413     YMZ284-D               PAL             |
|                                                  |
|                                                  |
|J                                                 |
|A                                           24MHz |
|M           28.63636MHz                           |
|M                                     M514262     |
|A                                                 |
|                                      M514262     |
|                         |--------|               |
|                         |        |   M514262     |
|                         | NL-005 |               |
|             TC5588      |        |   M514262     |
|                         |        |               |
|                         |--------|   M514262     |
|DSW1(8)                          P2               |
|--------------------------------------------------|
Notes:
      *       - SDIP64 socket, not populated
      P1/P2   - Connector joining to ROM daughterboard
      TC55257 - 32k x8 SRAM
      TC5588  - 8k x8 SRAM
      M514262 - OKI M514262-70Z 262144-word x 4-bit Multiport DRAM
      YMZ284  - Yamaha YMZ284-D Software Controlled Melody and Effect Sound Generator (DIP16)
                Software compatible with YM2149. Clock 3.579545MHz [28.63636/8]
      YM2413  - OPLL FM Sound Generator (DIP18). Clock 3.579545MHz [28.63636/8]
      M6295   - Clock 1.02272MHz [28.63636/28]. Pin7 HIGH
      VSync   - 60Hz
      HSync   - 15.28kHz


Top Board
---------
NS5000101
|---------------------------------|
|                    16MHz        |
|       &                *        |
|                                 |
|                                 |
|                                 |
|                          DSW1(8)|
|                                 |
|50001   50003   50005   50007    |
|    50002   50004   50006        |
|                                 |
|                                 |
|50008           50010   50012    |
|    50009           50011   50013|
|                                 |
|                                 |
|---------------------------------|
Notes:
      &     - Unknown PLCC68 chip (surface scratched). Maybe CPLD/FPGA or custom? Doesn't
              look like 68000 as there is no measurable clock input on any pins.
      *     - Unknown QFP100 (surface scratched). Clock input of 16MHz. Possibly TMPZ84C015?

***************************************************************************/

ROM_START( mjflove )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "50004.2e", 0x00000, 0x80000, CRC(20afcdec) SHA1(b3e7d6083dab812a497b69a501e9d993a8ca86e7) )
	ROM_RELOAD(           0x10000, 0x80000 )

	ROM_REGION( 0x3c0000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "50005.2d", 0x000000, 0x80000, CRC(6494d5ad) SHA1(2313ee2f897320715c3de9a0de5c66e06b763a5f) )
	ROM_LOAD( "50006.2c", 0x080000, 0x80000, CRC(a1d61eb7) SHA1(4755c3843890f4682d5e5804153baba3d98ac2cc) )
	ROM_LOAD( "50007.2b", 0x100000, 0x80000, CRC(d79ea2f5) SHA1(9c783ee8fe1f646ad2402676c3b924678955a964) )
	ROM_LOAD( "50009.3h", 0x180000, 0x80000, CRC(cba17351) SHA1(22f901bcd7d1513a4fb56fb95f8568c842a3d42a) )
	ROM_LOAD( "50013.3a", 0x200000, 0x80000, CRC(ebb7c8bd) SHA1(16b4584d2a15c092ad7b2538850a39c81a5db753) )
	ROM_LOAD( "50010.3d", 0x280000, 0x80000, CRC(62c05df9) SHA1(da60ba77d2bcc560abfba9ca19586f90c07c4411) )
	ROM_LOAD( "50011.3c", 0x300000, 0x80000, CRC(7e05f586) SHA1(4dd17da3922365b9a1424f270ad07c5f6848558b) )
	ROM_LOAD( "50012.3b", 0x380000, 0x40000, CRC(9853e5e4) SHA1(8596459ab8614dbd2ddd068afb1b4655cbe3bb08) )

	ROM_REGION( 0x200000, "oki", 0 )	/* Samples */
	ROM_LOAD( "50002.2h", 0x000000, 0x80000, CRC(955034db) SHA1(190f37b77df0331243d52a60ddbd9c1398053f18) )	// 0,1
	ROM_LOAD( "50003.2f", 0x080000, 0x80000, CRC(8845734a) SHA1(c44d006cdf41da0187283faa8d060ed5d2d183fd) )	// 2,3
	ROM_LOAD( "50001.2j", 0x100000, 0x80000, CRC(6c0a93fd) SHA1(6a4359af79a0c18979ab15c8a2700880ec530192) )	// 4,5
	ROM_LOAD( "50008.3j", 0x180000, 0x40000, CRC(4a95b5eb) SHA1(02306cb11c889772c19f1635dbd34c0d03192af1) )	// 6
	ROM_RELOAD(           0x1c0000, 0x40000 )
ROM_END

/***************************************************************************

Super Hana Paradise
1995

PCB almost like Mysterious World

PCB Layout
----------
N8010178L1
|-------------------------------------------------------|
|MA1384  VOL             1011.2F                        |
|                          62256                        |
|                   M6295   1012.3E                     |
|        VOL                                     16MHz  |
|        YM2413                       TMPZ84C015        |
|                                                       |
|M                                                      |
|A     NL-002                                   DSW1(10)|
|H                                                      |
|J                                              DSW2(10)|
|O                                                      |
|N                                                      |
|G                                               DIP32  |
|                         28.636MHz                     |
|             TC5588                             DIP32  |
|                                                       |
|          1108F0406   4L02F2637     1427F0071   DIP32  |
|                                                       |
|                                                DIP32  |
|       TC524258Z-10  TC524258Z-10                      |
|       TC524258Z-10  TC524258Z-10               DIP32  |
|-------------------------------------------------------|
Notes:
      TMPZ84C015   - Toshiba TMPZ84C015F-6 (QFP100). Clocks 16.000MHz (pins 65 & 66), 8.000MHz (pins 68 & 69)
      62256        - 32k x8 SRAM (DIP28)
      TC5588       - 8k x8 SRAM (DIP28)
      TC524258Z-10 - Toshiba TC524258Z-10 256k x4 Dual Port VRAM (ZIP28)
      1427F0071    \
      1108F0406    | Dynax Custom ICs
      NL-002       |
      4L02F2637    /
      DIP32        - Empty sockets
      M6295        - Oki M6295 (QFP44). Clock 1.02272MHz [28.636/28]. pin 7 = high
      YM2413       - Clock 3.579545MHz [28.636/8]
      VSync        - 61Hz
      HSync        - 15.27kHz

ROM Daughterboard N73RSUB
Contains the remaining ROMS, 1 PAL and nothing else
1013, 1014, 1015, 1016

--

Super Hana Paradise
(c)1994 Dynax

D10110258L1

CPU: TMPZ84C015BF-8
Sound: YM2413 M6295
OSC: 28.6363MHz 16AKSS
Custom: NL-002
        1427F0071
        4L02F2637
        1108F0405

ROMs:
1011.2F
1012.3E

Subboard
1013.1A
1014.2A
1015.3A
1016.4A

***************************************************************************/

ROM_START( hparadis )
	ROM_REGION( 0x50000+8*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "1012.3e", 0x00000, 0x40000, CRC(bd3a3a8f) SHA1(35d70c2869a93192de7041b5c90b8a8a5e910946) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0x600000, "blitter", 0 )	/* blitter data */
	// unused
	ROM_LOAD( "1015.3a", 0x400000, 0x80000, CRC(ea7b282e) SHA1(4fb33f3a8fe0dd792bcdd90894b90f1ac09ef6a5) )
	ROM_LOAD( "1014.2a", 0x480000, 0x80000, CRC(5f057c13) SHA1(d17211c3b697b48012018b738cff22fc4743d607) )
	ROM_LOAD( "1016.4a", 0x500000, 0x80000, CRC(c8e7ffb6) SHA1(9a7df1d6b3723e56b69d56831ce32c5326764a68) )
	ROM_LOAD( "1013.1a", 0x580000, 0x80000, CRC(c26b0563) SHA1(a09209c7e44fde418f917aed3a436bdf515942eb) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1011.2f", 0x00000, 0x40000, CRC(8c852b1a) SHA1(39e3e037f441df1d7cc7a698fea3e7825f8f6984) )
ROM_END


/***************************************************************************

Hanafuda Hana Gokou
Dynax (Alba License), 1995

PCB almost like Mysterious World

PCB Layout
----------
N83061581L1
|-------------------------------------------------------|
|MB3714  VOL     DSW1(10)    M6242B   TC55257   BATTERY |
|                   32.768kHz 1081.2D  1082B.2B         |
|   VOL          DSW2(10)    M6295                      |
|   DSW5(4)                                      16MHz  |
|        YM2413  DSW3(10)        PAL         TMPZ84C015 |
|                                                       |
|M               DSW4(10)                               |
|A         YM2149                                       |
|H                          *                           |
|J     NL-002                           28.63636MHz     |
|O                                                 PAL  |
|N                                                      |
|G                                                      |
|2                                                      |
|8                                              1083.9A |
|                                                       |
|            &           %            #         1084.10A|
|                                                       |
|                      TC5588                    DIP32  |
|       TC524258Z-10  TC524258Z-10               DIP32  |
|       TC524258Z-10  TC524258Z-10               DIP32  |
|-------------------------------------------------------|
Notes:
      TMPZ84C015   - Toshiba TMPZ84C015F-6 (QFP100). Clocks 16.000MHz (pins 65 & 66), 8.000MHz (pins 68 & 69)
      TC55257      - 32k x8 SRAM (DIP28)
      TC5588       - 8k x8 SRAM (DIP28)
      TC524258Z-10 - Toshiba TC524258Z-10 256k x4 Dual Port VRAM (ZIP28)
      &            - Unknown QFP64. Possibly Dynax Custom 1108F0406
      %            - Unknown QFP100. Possibly Dynax Custom 4L02F2637
      #            - Unknown QFP100. Possibly Dynax Custom 1427F0071
      *            - Unknown PLCC44. Possibly MACH311 or similar CPLD
      DIP32        - Empty sockets
      M6295        - Oki M6295 (QFP44). Clock 1.02272MHz [28.63636/28]. pin 7 = HIGH
      YM2413       - Clock 3.579545MHz [28.63636/8]
      YM2149       - Clock 3.579545MHz [28.63636/8]
      VSync        - 60Hz
      HSync        - 15.36kHz

***************************************************************************/

ROM_START( hgokou )
	ROM_REGION( 0x90000+8*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "1082b.2b", 0x00000, 0x40000, CRC(e33bc5a5) SHA1(cb723cc81b914b45f89236812492c105c93c4e0b) )
	ROM_RELOAD(           0x10000, 0x40000 )

	ROM_REGION( 0x200000, "blitter", 0 )	/* blitter data */
	// unused
	ROM_LOAD( "1083.9a",  0x100000, 0x80000, CRC(054200c3) SHA1(7db457fa1f8639d15a6faa3e1e05d4302e7dd281) )
	ROM_LOAD( "1084.10a", 0x180000, 0x80000, CRC(49f657e8) SHA1(077c553f88a76f826495ad516350a53ce361c6da) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1081.2d",  0x00000, 0x40000, CRC(74dede40) SHA1(d148f9ab9223b4c0b2f457a6f0e7fa3d173ab12b) )
ROM_END

/***************************************************************************

Hanafuda Hana Gokou Bangaihen
Dynax 1995

PCB is almost identical to Hanafuda Hana Gokou by Dynax/Alba minus the CPLD

PCB Layout
----------
N10805078L1
|-------------------------------------------------------|
|MB3713  VOL     DSW1(10)    M6242B   TC55257   BATTERY |
|                   32.768kHz 1161.2D  1162.2B          |
|   VOL          DSW2(10)    M6295                      |
|   DSW5(4)                                      16MHz  |
|        YM2413  DSW3(10)        PAL         TMPZ84C015 |
|                                                       |
|M               DSW4(10)                               |
|A         YM2149                                       |
|H                                                      |
|J     NL-002                           28.63636MHz     |
|O                                                 PAL  |
|N                                                      |
|G                                                      |
|2                                                      |
|8                                              1163.9A |
|                                                       |
|            &           %            #         1164.10A|
|                                                       |
|                      TC5588                    DIP32  |
|       TC524258Z-10  TC524258Z-10               DIP32  |
|       TC524258Z-10  TC524258Z-10               DIP32  |
|-------------------------------------------------------|
Notes:
      TMPZ84C015   - Toshiba TMPZ84C015F-6 (QFP100). Clocks 16.000MHz (pins 65 & 66), 8.000MHz (pins 68 & 69)
      TC55257      - 32k x8 SRAM (DIP28)
      TC5588       - 8k x8 SRAM (DIP28)
      TC524258Z-10 - Toshiba TC524258Z-10 256k x4 Dual Port VRAM (ZIP28)
      &            - Unknown QFP64. Possibly Dynax Custom 1108F0406
      %            - Unknown QFP100. Possibly Dynax Custom 4L02F2637
      #            - Unknown QFP100. Possibly Dynax Custom 1427F0071
      DIP32        - Empty sockets
      M6295        - Oki M6295 (QFP44). Clock 1.02272MHz [28.63636/28]. pin 7 = HIGH
      YM2413       - Clock 3.579545MHz [28.63636/8]
      YM2149       - Clock 3.579545MHz [28.63636/8]
      VSync        - 60Hz
      HSync        - 15.36kHz

***************************************************************************/

ROM_START( hgokbang )
	ROM_REGION( 0x90000+8*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "1162.2b",  0x00000, 0x40000, CRC(02414b42) SHA1(00346d4c750c7cbf490f0a5bb90d1b2b3879c979) )
	ROM_RELOAD(           0x10000, 0x40000 )

	ROM_REGION( 0x500000, "blitter", 0 )	/* blitter data */
	// unused
	ROM_LOAD( "1163.9a",  0x400000, 0x80000, CRC(054200c3) SHA1(7db457fa1f8639d15a6faa3e1e05d4302e7dd281) )
	ROM_LOAD( "1164.10a", 0x480000, 0x80000, CRC(25b40754) SHA1(b660f174826a11cdcf9d61249012390f45f446e6) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "1161.2d",  0x00000, 0x40000, CRC(74dede40) SHA1(d148f9ab9223b4c0b2f457a6f0e7fa3d173ab12b) )
ROM_END

/***************************************************************************

Mahjong Jong-Tei
Dynax 1999

PCB Layout
----------

NM532-9902
|-----------------------------------------|
| TA7252        LM358  LM358  PST532 BATT |
|         VOL            M6295   32.768kHz|
|  DSW5(4)                        53201   |
|M                    YM2413 20MHz   M6242|
|A                                        |
|H            DSW1(10)       QFP100 62256 |
|J  ACTEL     DSW2(10)                    |
|O  A1010     DSW3(10)              53202 |
|N            DSW4(10)                    |
|G             28.63636MHz          53203 |
|                                         |
|                                   53204 |
|           M514262   4L10FXXXX           |
|           M524262                 53205 |
|           M514262                       |
|           M514262                 53206 |
|-----------------------------------------|
Notes:
      4L10FXXXX - Dynax graphics chip, surface scratch, but
                  it will be one of the usual ones
      QFP100    - Main CPU, surface scratched. Clock input 20.00MHz
      M6295     - Clock 1.022727143MHz [28.63636/28]. Pin 7 HIGH
      YM2413    - Clock 3.579545MHz [28.63636/8]
      VSync     - 60.7194Hz
      HSync     - 15.2443kHz

***************************************************************************/

ROM_START( jongtei )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "53202.5b", 0x00000, 0x80000, CRC(fa95a7f2) SHA1(bb67d74acb8908c222acdc92ee13d4a644358aef) )
	ROM_RELOAD(           0x10000, 0x80000 )

	ROM_REGION( 0x800000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "53203.7b",  0x000000, 0x200000, CRC(55d6522a) SHA1(47996be70481a98ead10211645566613d20b5880) )
	ROM_LOAD( "53204.8b",  0x200000, 0x200000, CRC(4f58a303) SHA1(2893e6b47c3098cb878cf5fa5957e9652559e420) )
	ROM_LOAD( "53205.10b", 0x400000, 0x200000, CRC(d69e0355) SHA1(f67688eaf7954619785040204368d2cb5fc64e6e) )
	ROM_LOAD( "53206.12b", 0x600000, 0x200000, CRC(f0652395) SHA1(286683728836c452b71c3b8c48bd0f7159b2a10c) )

	ROM_REGION( 0x200000, "oki", 0 )	/* Samples */
	ROM_LOAD( "53201.2a", 0x000000, 0x200000, CRC(c53d840c) SHA1(5a935320f48bdc8f3b9ed105dcdd0c6e33c3c38c) )
ROM_END

/***************************************************************************

Mahjong Seiryu Densetsu
Dynax 1996

PCB Layout
----------

NM5020403
|---------------------------------|
|MB3713     LM358 LM358   50201   |
|        YM2413    M6295     BATT |
|   DSW5     YMZ284  16MHz  PAL   |
|M                  QFP100 TC55257|
|A                        50202   |
|H                       RTC62421 |
|J       28.63636MHz              |
|O    PLCC68             TC524258 |
|N               QFP208  TC524258 |
|G                       TC524258 |
|            TC5588  PAL TC524258 |
|  DSW1 DSW2 50209  50206  50203  |
|  DSW3 DSW4 50210  50207  50204  |
|            50211  50208  50205  |
|---------------------------------|
Notes:
      YM2413 - Clock 3.579545MHz [28.63636/8]
      YMZ284 - Clock 3.579545MHz [28.63636/8]
      PLCC68 - Unknown PLCC68. Clock 1.7897725MHz [28.63636/16]
               Possibly FPGA/CPLD/Gate Array or custom Dynax chip etc.
      QFP100 - Unknown QFP100. Clock input 16MHz. Looks like TMPZ8400
      QFP208 - Unknown QFP208 custom Dynax chip
      DSW1-4 - 10-position DIP switches
      DSW5   - 4-position DIP switch
      VSync  - 60.8532Hz
      HSync  - 15.2790kHz

***************************************************************************/

ROM_START( sryudens )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "50202.5c", 0x00000, 0x80000, CRC(7072d3d9) SHA1(6a6605afd334d5adca0e8eed9758f2a2b37c389e) )
	ROM_RELOAD(           0x10000, 0x80000 )

	ROM_REGION( 0x480000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "50203.13b", 0x000000, 0x80000, CRC(4da60d60) SHA1(bd34f9471baf8242a32908bb3e2106e10ae3310c) )
	ROM_LOAD( "50204.14b", 0x080000, 0x80000, CRC(43d1d705) SHA1(ce7184a6dd04a10b7e0734039ae8c3dcf3819258) )
	ROM_LOAD( "50205.15b", 0x100000, 0x80000, CRC(7c727fc0) SHA1(3f7fe11fb5abb4a2a85b38b670ef4597cd42edc7) )
	ROM_LOAD( "50206.13d", 0x180000, 0x80000, CRC(c52396dd) SHA1(385dfda305011f4db0b783b861daeff4ee52ea9c) )
	ROM_LOAD( "50207.14d", 0x200000, 0x80000, CRC(cb600774) SHA1(321b0ac01e70d17006871ad7f88f98f53536ca8d) )
	ROM_LOAD( "50208.15d", 0x280000, 0x80000, CRC(0b30c780) SHA1(b2a263cc14e5a734eb9580451eb21dad980d18f0) )
	ROM_LOAD( "50209.13f", 0x300000, 0x80000, CRC(8f34a31c) SHA1(9b56a462f871d935806b6594f07fa1e4214f9186) )
	ROM_LOAD( "50210.14f", 0x380000, 0x80000, CRC(2fdd3b49) SHA1(db27d5d9f74f532ab4e9b8ffa81eef2fae2ef6fd) )
	ROM_LOAD( "50211.15f", 0x400000, 0x80000, CRC(39ad357a) SHA1(899e369d7396ed40803df7c575199a65b18c046e) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "50201.1c", 0x00000, 0x80000, CRC(5a8cd45c) SHA1(25ca573b8ba226fb3f2de48c57b5ced6884eaa63) )
ROM_END

/***************************************************************************

Mahjong Daimyojin
Dynax/Techno Top Ltd/Techno Planning, 2002

PCB Layout
----------

TSM015-0111
|--------------------------------------|
|TA7252    VOL     M6295  PST532  BATT |
|            LM358 LM358   T0171       |
|  DSW5      YM2413            32.768kHz
|  62003              20MHz     TC55257|
|M                       QFP100  M6242B|
|A           DSW1  DSW2                |
|H           DSW3  DSW4         P0172  |
|J                                     |
|O      PLCC68                T0173    |
|N               28.63636MHz           |
|G        KM424C256           P0174    |
|         KM424C256                    |
|         KM424C256 TSM100             |
|         KM424C256                    |
|         KM424C256                    |
|         KM424C256                    |
|--------------------------------------|
Notes:
      PLCC68 - Unknown PLCC68. Clock input 10MHz [20/2]. Possibly FPGA/CPLD/Gate Array or custom chip etc.
      QFP100 - Unknown QFP100. Clock input 20MHz. Looks like TMPZ8400
      TSM100 - Custom QFP208 GFX chip. Details....
               T-top SOFT
               TSM100
               70C160F011
               JAPAN 0210EAI
               D0002ZCA
      YM2413 - Clock 3.579545MHz [28.63636/8]
      M6295  - Clock 1.02272MHz [28.63636/28]. pin 7 high
      VSync  - 59.7922Hz
      HSync  - 15.4248kHz

***************************************************************************/

ROM_START( daimyojn )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "p0172.6b", 0x00000, 0x80000, CRC(478442bd) SHA1(50efe7e014a55a5e5ac359628438ad2963df181c) )
	ROM_RELOAD(           0x10000, 0x80000 )

	ROM_REGION( 0x400000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "t0173.7b", 0x000000, 0x200000, CRC(b54c7b02) SHA1(54a750708c91041caa89adb033d8133b409b0706) )
	ROM_LOAD( "p0174.8b", 0x200000, 0x200000, CRC(861de43f) SHA1(c5bc279f476902baa46e046800c26bd52255a525) )

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "t0171.2b", 0x00000, 0x80000, CRC(464be04c) SHA1(3532ac8d7eaadb2dc33e2c2d9731654176231184) )
ROM_END

ROM_START( momotaro )
	ROM_REGION( 0x90000+16*0x1000, "maincpu", 0 )	/* Z80 Code */
	ROM_LOAD( "r0272m1.6e", 0x00000, 0x80000, CRC(71c83332) SHA1(c949cb9e23e5cc77dbd64fc28e62a88f1dc811a3) )
	ROM_RELOAD(         	0x10000, 0x80000 )

	ROM_REGION( 0x400000, "blitter", 0 )	/* blitter data */
	ROM_LOAD( "t0273.7b", 0x000000, 0x200000, BAD_DUMP CRC(5ae90ae2) SHA1(975bae930d848987405dc3dd59de138b1f98b358) )	// FIXED BITS (xxxxx1xxxxxxxxx1)
	ROM_LOAD( "t0274.8b", 0x200000, 0x200000, BAD_DUMP CRC(78209778) SHA1(4054972e12115049322bb43381ff50a354c3cadf) )	// FIXED BITS (xxxxx1xxxxxxxxx1)

	ROM_REGION( 0x80000, "oki", 0 )	/* Samples */
	ROM_LOAD( "t0271.2b", 0x00000, 0x80000, CRC(c850d7b2) SHA1(8bb69bdea7035c5f8274927f07a4cdf6ed9b32fc) )
ROM_END

static DRIVER_INIT( momotaro )
{
	machine.device("maincpu")->memory().space(AS_IO)->install_legacy_read_handler(0xe0, 0xe0, FUNC(momotaro_protection_r) );
}

GAME( 1992, mmpanic,   0,        mmpanic,   mmpanic,  0,        ROT0, "Nakanihon / East Technology (Taito license)", "Monkey Mole Panic (USA)",                                         GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1993, funkyfig,  0,        funkyfig,  funkyfig, 0,        ROT0, "Nakanihon / East Technology (Taito license)", "The First Funky Fighter",                                         GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // scrolling, priority?
GAME( 1993, quizchq,   0,        quizchq,   quizchq,  0,        ROT0, "Nakanihon",                                   "Quiz Channel Question (Ver 1.00) (Japan)",                        GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1993, quizchql,  quizchq,  quizchq,   quizchq,  0,        ROT0, "Nakanihon (Laxan license)",                   "Quiz Channel Question (Ver 1.23) (Taiwan?)",                      GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1993, animaljr,  0,        mmpanic,   animaljr, 0,        ROT0, "Nakanihon / East Technology (Taito license)", "Exciting Animal Land Jr. (USA)",                                  GAME_NO_COCKTAIL | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1993, animaljrs, animaljr, mmpanic,   animaljr, 0,        ROT0, "Nakanihon / East Technology (Taito license)", "Animalandia Jr. (Spanish)",                                       GAME_NO_COCKTAIL | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1993, animaljrj, animaljr, mmpanic,   animaljr, 0,        ROT0, "Nakanihon / East Technology (Taito license)", "Waiwai Animal Land Jr. (Japan)",                                  GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1994, hginga,    0,        hginga,    hginga,   0,        ROT0, "Dynax",                                       "Hanafuda Hana Ginga",                                             GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1994, mjmyster,  0,        mjmyster,  mjmyster, 0,        ROT0, "Dynax",                                       "Mahjong The Mysterious World (set 1)",                            GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1994, mjmywrld,  mjmyster, mjmywrld,  mjmyster, 0,        ROT0, "Dynax",                                       "Mahjong The Mysterious World (set 2)",                            GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1994, mjmyornt,  0,        mjmyornt,  mjmyornt, 0,        ROT0, "Dynax",                                       "Mahjong The Mysterious Orient",                                   GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1994, mjmyuniv,  0,        mjmyuniv,  mjmyster, 0,        ROT0, "Dynax",                                       "Mahjong The Mysterious Universe",                                 GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1994, quiz365,   0,        quiz365,   quiz365,  0,        ROT0, "Nakanihon",                                   "Quiz 365 (Japan)",                                                GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1994, quiz365t,  quiz365,  quiz365,   quiz365,  0,        ROT0, "Nakanihon / Taito",                           "Quiz 365 (Hong Kong & Taiwan)",                                   GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1994, rongrong,  0,        rongrong,  rongrong, rongrong, ROT0, "Nakanihon (Activision license)",              "Puzzle Game Rong Rong (Europe)",                                  GAME_NO_COCKTAIL | GAME_IMPERFECT_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1994, rongrongj, rongrong, rongrong,  rongrong, rongrong, ROT0, "Nakanihon (Activision license)",              "Puzzle Game Rong Rong (Japan)",                                   GAME_NO_COCKTAIL | GAME_IMPERFECT_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1994, rongrongg, rongrong, rongrong,  rongrong, rongrong, ROT0, "Nakanihon (Activision license)",              "Puzzle Game Rong Rong (Germany)",                                 GAME_NO_COCKTAIL | GAME_IMPERFECT_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1994, hparadis,  0,        hparadis,  hparadis, 0,        ROT0, "Dynax",                                       "Super Hana Paradise (Japan)",                                     GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, hgokou,    0,        hgokou,    hgokou,   0,        ROT0, "Dynax (Alba license)",                        "Hanafuda Hana Gokou (Japan)",                                     GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, hgokbang,  hgokou,   hgokbang,  hgokou,   0,        ROT0, "Dynax",                                       "Hanafuda Hana Gokou Bangaihen (Japan)",                           GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, mjdchuka,  0,        mjchuuka,  mjchuuka, 0,        ROT0, "Dynax",                                       "Mahjong The Dai Chuuka Ken (China, v. D111)",                     GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, nettoqc,   0,        nettoqc,   nettoqc,  0,        ROT0, "Nakanihon",                                   "Nettoh Quiz Champion (Japan)",                                    GAME_NO_COCKTAIL | GAME_IMPERFECT_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1995, ddenlovj,  0,        ddenlovj,  ddenlovj, 0,        ROT0, "Dynax",                                       "Don Den Lover Vol. 1 - Shiro Kuro Tsukeyo! (Japan)",              GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, ddenlovrk, ddenlovj, ddenlovrk, ddenlovr, 0,        ROT0, "Dynax",                                       "Don Den Lover Vol. 1 - Heukbaeg-euro Jeonghaja (Korea)",          GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, ddenlovrb, ddenlovj, ddenlovr,  ddenlovr, 0,        ROT0, "bootleg",                                     "Don Den Lover Vol. 1 - Heukbaeg-euro Jeonghaja (Korea, bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1996, ddenlovr,  ddenlovj, ddenlovr,  ddenlovr, 0,        ROT0, "Dynax",                                       "Don Den Lover Vol. 1 (Hong Kong)",                                GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1996, hanakanz,  0,        hanakanz,  hanakanz, 0,        ROT0, "Dynax",                                       "Hana Kanzashi (Japan)",                                           GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1996, akamaru,   0,        akamaru,   akamaru,  0,        ROT0, "Dynax (Nakanihon license)",                   "Panel & Variety Akamaru Q Jousyou Dont-R",                        GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1996, sryudens,  0,        sryudens,  sryudens, 0,        ROT0, "Dynax / Face",                                "Mahjong Seiryu Densetsu (Japan, NM502)",                          GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1996, mjflove,   0,        mjflove,   mjflove,  0,        ROT0, "Nakanihon",                                   "Mahjong Fantasic Love (Japan)",                                   GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1997, hkagerou,  0,        hkagerou,  hkagerou, 0,        ROT0, "Nakanihon / Dynax",                           "Hana Kagerou [BET] (Japan)",                                      GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1998, mjchuuka,  0,        mjchuuka,  mjchuuka, 0,        ROT0, "Dynax",                                       "Mahjong Chuukanejyo (China)",                                     GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1998, mjreach1,  0,        mjreach1,  mjreach1, 0,        ROT0, "Nihon System",                                "Mahjong Reach Ippatsu (Japan)",                                   GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1999, jongtei,   0,        jongtei,   jongtei,  0,        ROT0, "Dynax",                                       "Mahjong Jong-Tei (Japan, ver. NM532-01)",                         GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 2002, daimyojn,  0,        daimyojn,  daimyojn, 0,        ROT0, "Dynax / Techno-Top / Techno-Planning",        "Mahjong Daimyojin (Japan, T017-PB-00)",                           GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 2004, momotaro,  0,        daimyojn,  daimyojn, momotaro, ROT0, "Techno-Top", 						         "Mahjong Momotarou (Japan)",                                       GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE | GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
