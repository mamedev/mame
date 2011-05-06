/*********************************************************************

    smsvdp.c

    Implementation of video hardware chip used by Sega Master System

**********************************************************************/

/*
    For more information, please see:
    - http://cgfm2.emuviews.com/txt/msvdp.txt
    - http://www.smspower.org/forums/viewtopic.php?p=44198

A scanline contains the following sections:
  - horizontal sync     1  ED      => HSYNC high/increment line counter/generate interrupts/etc
  - left blanking       2  ED-EE
  - color burst        14  EE-EF
  - left blanking       8  F5-F9
  - left border        13  F9-FF
  - active display    256  00-7F
  - right border       15  80-87
  - right blanking      8  87-8B
  - horizontal sync    25  8B-97   => HSYNC low


NTSC frame timing
                       256x192         256x224        256x240 (doesn't work on real hardware)
  - vertical blanking   3  D5-D7        3  E5-E7       3  EE-F0
  - top blanking       13  D8-E4       13  E8-F4      13  F1-FD
  - top border         27  E5-FF       11  F5-FF       3  FD-FF
  - active display    192  00-BF      224  00-DF     240  00-EF
  - bottom border      24  C0-D7        8  E0-E7       0  F0-F0
  - bottom blanking     3  D8-DA        3  E8-EA       3  F0-F2


PAL frame timing
                       256x192         256x224        256x240
  - vertical blanking   3  BA-BC        3  CA-CC       3  D2-D4
  - top blanking       13  BD-C9       13  CD-D9      13  D5-E1
  - top border         54  CA-FF       38  DA-FF      30  E2-FF
  - active display    192  00-BF      224  00-DF     240  00-EF
  - bottom border      48  C0-EF       32  E0-FF      24  F0-07
  - bottom blanking     3  F0-F2        3  00-02       3  08-0A

  TODO:
    - implement differences between SMS1_VDP & SMS_VDP/GG_VDP

*/

#include "emu.h"
#include "video/smsvdp.h"

#define IS_SMS1_VDP           (smsvdp->features & MODEL_315_5124)
#define IS_SMS2_VDP           (smsvdp->features & MODEL_315_5246)
#define IS_GAMEGEAR_VDP       (smsvdp->features & MODEL_315_5378)

#define STATUS_VINT           0x80	/* Pending vertical interrupt flag */
#define STATUS_SPROVR         0x40	/* Sprite overflow flag */
#define STATUS_SPRCOL         0x20	/* Object collision flag */
#define STATUS_HINT           0x02	/* Pending horizontal interrupt flag */

#define VINT_HPOS             23
#define HINT_HPOS             23
#define VCOUNT_CHANGE_HPOS    22
#define VINT_FLAG_HPOS        7
#define SPROVR_HPOS           6
#define SPRCOL_BASEHPOS       42
#define DISPLAY_CB_HPOS       5  /* fix X-Scroll latchtime (Flubba's VDPTest) */

#define GG_CRAM_SIZE          0x40	/* 32 colors x 2 bytes per color = 64 bytes */
#define SMS_CRAM_SIZE         0x20	/* 32 colors x 1 bytes per color = 32 bytes */
#define MAX_CRAM_SIZE         0x40

#define VRAM_SIZE             0x4000

#define PRIORITY_BIT          0x1000
#define BACKDROP_COLOR        ((smsvdp->vdp_mode == 4 ? 0x10 : 0x00) + (smsvdp->reg[0x07] & 0x0f))

#define NUM_OF_REGISTER       0x10  /* 16 registers */

#define INIT_VCOUNT           0
#define VERTICAL_BLANKING     1
#define TOP_BLANKING          2
#define TOP_BORDER            3
#define ACTIVE_DISPLAY_V      4
#define BOTTOM_BORDER         5
#define BOTTOM_BLANKING       6

static const UINT8 sms_ntsc_192[7] = { 0xd5, 3, 13, 27, 192, 24, 3 };
static const UINT8 sms_ntsc_224[7] = { 0xe5, 3, 13, 11, 224,  8, 3 };
static const UINT8 sms_ntsc_240[7] = { 0xee, 3, 13,  3, 240,  0, 3 };
static const UINT8 sms_pal_192[7]  = { 0xba, 3, 13, 54, 192, 48, 3 };
static const UINT8 sms_pal_224[7]  = { 0xca, 3, 13, 38, 224, 32, 3 };
static const UINT8 sms_pal_240[7]  = { 0xd2, 3, 13, 30, 240, 24, 3 };


typedef struct _smsvdp_t smsvdp_t;
struct _smsvdp_t
{
	UINT32           features;
	UINT8            reg[NUM_OF_REGISTER];     /* All the registers */
	UINT8            status;                   /* Status register */
	UINT8            reg9copy;                 /* Internal copy of register 9 */
	UINT8            addrmode;                 /* Type of VDP action */
	UINT16           addr;                     /* Contents of internal VDP address register */
	UINT8            cram_mask;                /* Mask to switch between SMS and GG CRAM sizes */
	int              cram_dirty;               /* Have there been any changes to the CRAM area */
	int              pending;
	UINT8            buffer;
	int              gg_sms_mode;              /* Shrunk SMS screen on GG lcd mode flag */
	int              irq_state;                /* The status of the IRQ line of the VDP */
	int              vdp_mode;                 /* Current mode of the VDP: 0,1,2,3,4 */
	int              y_pixels;                 /* 192, 224, 240 */
	UINT8            line_counter;
	UINT8            hcounter;
	memory_region            *VRAM;                    /* Pointer to VRAM */
	memory_region            *CRAM;                    /* Pointer to CRAM */
	const UINT8      *sms_frame_timing;
	bitmap_t         *tmpbitmap;
	UINT8            *collision_buffer;

	/* line_buffer will be used to hold 5 lines of line data. Line #0 is the regular blitting area.
       Lines #1-#4 will be used as a kind of cache to be used for vertical scaling in the gamegear
       sms compatibility mode. */
	int              *line_buffer;
	int              current_palette[32];
	smsvdp_int_cb    int_callback;
	smsvdp_pause_cb  pause_callback;
	emu_timer        *smsvdp_display_timer;
};

static TIMER_CALLBACK( smsvdp_display_callback );
static void sms_refresh_line( running_machine &machine, smsvdp_t *smsvdp, bitmap_t *bitmap, int offsetx, int offsety, int line );
static void sms_update_palette( smsvdp_t *smsvdp );


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE smsvdp_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SMSVDP);

	return (smsvdp_t *)downcast<legacy_device_base *>(device)->token();
}

INLINE const smsvdp_interface *get_interface(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == SMSVDP));
	return (const smsvdp_interface *) device->static_config();
}

/*************************************
 *
 *  Utilities
 *
 *************************************/

static void set_display_settings( device_t *device )
{
	smsvdp_t *smsvdp = get_safe_token(device);

	screen_device *screen = device->machine().first_screen();
	int height = screen->height();
	int M1, M2, M3, M4;
	M1 = smsvdp->reg[0x01] & 0x10;
	M2 = smsvdp->reg[0x00] & 0x02;
	M3 = smsvdp->reg[0x01] & 0x08;
	M4 = smsvdp->reg[0x00] & 0x04;
	smsvdp->y_pixels = 192;
	if (M4)
	{
		/* mode 4 */
		smsvdp->vdp_mode = 4;
		if (M2 && (IS_SMS2_VDP || IS_GAMEGEAR_VDP))
		{
			if (M1 && !M3)
				smsvdp->y_pixels = 224;	/* 224-line display */
			else if (!M1 && M3)
				smsvdp->y_pixels = 240;	/* 240-line display */
		}
	}
	else
	{
		/* original TMS9918 mode */
		if (!M1 && !M2 && !M3)
		{
			smsvdp->vdp_mode = 0;
		}
		else
//      if (M1 && !M2 && !M3)
//      {
//          smsvdp->vdp_mode = 1;
//      }
//      else
		if (!M1 && M2 && !M3)
		{
			smsvdp->vdp_mode = 2;
//      }
//      else
//      if (!M1 && !M2 && M3)
//      {
//          smsvdp->vdp_mode = 3;
		}
		else
		{
			logerror("Unknown video mode detected (M1 = %c, M2 = %c, M3 = %c, M4 = %c)\n", M1 ? '1' : '0', M2 ? '1' : '0', M3 ? '1' : '0', M4 ? '1' : '0');
		}
	}

	switch (smsvdp->y_pixels)
	{
	case 192:
		smsvdp->sms_frame_timing = (height == PAL_Y_PIXELS) ? sms_pal_192 : sms_ntsc_192;
		break;

	case 224:
		smsvdp->sms_frame_timing = (height == PAL_Y_PIXELS) ? sms_pal_224 : sms_ntsc_224;
		break;

	case 240:
		smsvdp->sms_frame_timing = (height == PAL_Y_PIXELS) ? sms_pal_240 : sms_ntsc_240;
		break;
	}
	smsvdp->cram_dirty = 1;
}


READ8_DEVICE_HANDLER( sms_vdp_vcount_r )
{
	smsvdp_t *smsvdp = get_safe_token(device);
	int vpos = device->machine().primary_screen->vpos();

	if (device->machine().primary_screen->hpos() < VCOUNT_CHANGE_HPOS)
	{
		vpos--;
		if (vpos < 0)
			vpos += device->machine().primary_screen->height();
	}

	return (smsvdp->sms_frame_timing[INIT_VCOUNT] + vpos) & 0xff;
}


READ8_DEVICE_HANDLER( sms_vdp_hcount_latch_r )
{
	smsvdp_t *smsvdp = get_safe_token(device);

	return smsvdp->hcounter;
}

WRITE8_DEVICE_HANDLER( sms_vdp_hcount_latch_w )
{
	smsvdp_t *smsvdp = get_safe_token(device);

	smsvdp->hcounter = data;
}


void sms_vdp_set_ggsmsmode( device_t *device, int mode )
{
	smsvdp_t *smsvdp = get_safe_token(device);

	smsvdp->gg_sms_mode = mode;
	smsvdp->cram_mask = (IS_GAMEGEAR_VDP && !smsvdp->gg_sms_mode) ? (GG_CRAM_SIZE - 1) : (SMS_CRAM_SIZE - 1);
}


static TIMER_CALLBACK( smsvdp_set_status )
{
	smsvdp_t *smsvdp = (smsvdp_t *) ptr;

	smsvdp->status |= (UINT8) param;
}


static TIMER_CALLBACK( smsvdp_check_hint )
{
	smsvdp_t *smsvdp = (smsvdp_t *) ptr;

	if (smsvdp->line_counter == 0x00)
	{
		smsvdp->line_counter = smsvdp->reg[0x0a];
		smsvdp->status |= STATUS_HINT;
	}
	else
	{
		smsvdp->line_counter--;
	}

	if ((smsvdp->status & STATUS_HINT) && (smsvdp->reg[0x00] & 0x10))
	{
		smsvdp->irq_state = 1;

		if (smsvdp->int_callback)
			smsvdp->int_callback(machine, ASSERT_LINE);
	}
}


static TIMER_CALLBACK( smsvdp_check_vint )
{
	smsvdp_t *smsvdp = (smsvdp_t *) ptr;

	if ((smsvdp->status & STATUS_VINT) && (smsvdp->reg[0x01] & 0x20))
	{
		smsvdp->irq_state = 1;

		if (smsvdp->int_callback)
			smsvdp->int_callback(machine, ASSERT_LINE);
	}
}


static TIMER_CALLBACK( smsvdp_display_callback )
{
	smsvdp_t *smsvdp = (smsvdp_t *) ptr;

	rectangle rec;
	int vpos = machine.primary_screen->vpos();
	int vpos_limit = smsvdp->sms_frame_timing[VERTICAL_BLANKING] + smsvdp->sms_frame_timing[TOP_BLANKING]
	               + smsvdp->sms_frame_timing[TOP_BORDER] + smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V]
	               + smsvdp->sms_frame_timing[BOTTOM_BORDER] + smsvdp->sms_frame_timing[BOTTOM_BLANKING];

	rec.min_y = rec.max_y = vpos;

	/* Check if we're on the last line of a frame */
	if (vpos == vpos_limit - 1)
	{
		smsvdp->line_counter = smsvdp->reg[0x0a];
		if (smsvdp->pause_callback)
			smsvdp->pause_callback(machine);

		return;
	}

	vpos_limit -= smsvdp->sms_frame_timing[BOTTOM_BLANKING];

	/* Check if we're below the bottom border */
	if (vpos >= vpos_limit)
	{
		smsvdp->line_counter = smsvdp->reg[0x0a];
		return;
	}

	vpos_limit -= smsvdp->sms_frame_timing[BOTTOM_BORDER];

	/* Check if we're in the bottom border area */
	if (vpos >= vpos_limit)
	{
		if (vpos == vpos_limit)
		{
			machine.scheduler().timer_set(machine.primary_screen->time_until_pos(vpos, HINT_HPOS), FUNC(smsvdp_check_hint),0 ,smsvdp);
		}
		else
		{
			smsvdp->line_counter = smsvdp->reg[0x0a];
		}

		if (vpos == vpos_limit + 1)
		{
			machine.scheduler().timer_set(machine.primary_screen->time_until_pos(vpos, VINT_FLAG_HPOS), FUNC(smsvdp_set_status), (int)STATUS_VINT, smsvdp);
			machine.scheduler().timer_set(machine.primary_screen->time_until_pos(vpos, VINT_HPOS), FUNC(smsvdp_check_vint), 0, smsvdp);
		}

		sms_update_palette(smsvdp);

		/* Draw left border */
		rec.min_x = LBORDER_START;
		rec.max_x = LBORDER_START + LBORDER_X_PIXELS - 1;
		bitmap_fill(smsvdp->tmpbitmap, &rec, machine.pens[smsvdp->current_palette[BACKDROP_COLOR]]);

		/* Draw right border */
		rec.min_x = LBORDER_START + LBORDER_X_PIXELS + 256;
		rec.max_x = rec.min_x + RBORDER_X_PIXELS - 1;
		bitmap_fill(smsvdp->tmpbitmap, &rec, machine.pens[smsvdp->current_palette[BACKDROP_COLOR]]);

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function so it will */
		/* be included in the gamegear scaling functions */
		sms_refresh_line(machine, smsvdp, smsvdp->tmpbitmap, LBORDER_START + LBORDER_X_PIXELS, vpos_limit - smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V], vpos - (vpos_limit - smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V]));
		return;
	}

	vpos_limit -= smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V];

	/* Check if we're in the active display area */
	if (vpos >= vpos_limit)
	{
		if (vpos == vpos_limit)
		{
			smsvdp->reg9copy = smsvdp->reg[0x09];
		}

		machine.scheduler().timer_set(machine.primary_screen->time_until_pos(vpos, HINT_HPOS), FUNC(smsvdp_check_hint),0, smsvdp);

		sms_update_palette(smsvdp);

		/* Draw left border */
		rec.min_x = LBORDER_START;
		rec.max_x = LBORDER_START + LBORDER_X_PIXELS - 1;
		bitmap_fill(smsvdp->tmpbitmap, &rec, machine.pens[smsvdp->current_palette[BACKDROP_COLOR]]);

		/* Draw right border */
		rec.min_x = LBORDER_START + LBORDER_X_PIXELS + 256;
		rec.max_x = rec.min_x + RBORDER_X_PIXELS - 1;
		bitmap_fill(smsvdp->tmpbitmap, &rec, machine.pens[smsvdp->current_palette[BACKDROP_COLOR]]);

		sms_refresh_line(machine, smsvdp, smsvdp->tmpbitmap, LBORDER_START + LBORDER_X_PIXELS, vpos_limit, vpos - vpos_limit);

		return;
	}

	vpos_limit -= smsvdp->sms_frame_timing[TOP_BORDER];

	/* Check if we're in the top border area */
	if (vpos >= vpos_limit)
	{
		smsvdp->line_counter = smsvdp->reg[0x0a];
		sms_update_palette(smsvdp);

		/* Draw left border */
		rec.min_x = LBORDER_START;
		rec.max_x = LBORDER_START + LBORDER_X_PIXELS - 1;
		bitmap_fill(smsvdp->tmpbitmap, &rec, machine.pens[smsvdp->current_palette[BACKDROP_COLOR]]);

		/* Draw right border */
		rec.min_x = LBORDER_START + LBORDER_X_PIXELS + 256;
		rec.max_x = rec.min_x + RBORDER_X_PIXELS - 1;
		bitmap_fill(smsvdp->tmpbitmap, &rec, machine.pens[smsvdp->current_palette[BACKDROP_COLOR]]);

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function so it will */
		/* be included in the gamegear scaling functions */
		sms_refresh_line(machine, smsvdp, smsvdp->tmpbitmap, LBORDER_START + LBORDER_X_PIXELS, vpos_limit + smsvdp->sms_frame_timing[TOP_BORDER], vpos - (vpos_limit + smsvdp->sms_frame_timing[TOP_BORDER]));
		return;
	}

	/* we're in the vertical or top blanking area */
	smsvdp->line_counter = smsvdp->reg[0x0a];
}


READ8_DEVICE_HANDLER( sms_vdp_data_r )
{
	smsvdp_t *smsvdp = get_safe_token(device);
	UINT8 temp;

	/* SMS 2 & GG behaviour. Seems like the latched data is passed straight through */
	/* to the address register when in the middle of doing a command.               */
	/* Cosmic Spacehead needs this, among others                                    */
	/* Clear pending write flag */
	smsvdp->pending = 0;

	/* Return read buffer contents */
	temp = smsvdp->buffer;

	/* Load read buffer */
	smsvdp->buffer = smsvdp->VRAM->u8((smsvdp->addr & 0x3fff));

	/* Bump internal address register */
	smsvdp->addr += 1;
	return temp;
}


READ8_DEVICE_HANDLER( sms_vdp_ctrl_r )
{
	smsvdp_t *smsvdp = get_safe_token(device);

	UINT8 temp = smsvdp->status;

	/* Clear pending write flag */
	smsvdp->pending = 0;

	smsvdp->status &= ~(STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL | STATUS_HINT);

	if (smsvdp->irq_state == 1)
	{
		smsvdp->irq_state = 0;

		if (smsvdp->int_callback)
			smsvdp->int_callback(device->machine(), CLEAR_LINE);
	}

	/* low 5 bits return non-zero data (it fixes PGA Tour Golf course map introduction) */
	return temp | 0x1f;
}


WRITE8_DEVICE_HANDLER( sms_vdp_data_w )
{
	smsvdp_t *smsvdp = get_safe_token(device);
	int address;

	/* SMS 2 & GG behaviour. Seems like the latched data is passed straight through */
	/* to the address register when in the middle of doing a command.               */
	/* Cosmic Spacehead needs this, among others                                    */
	/* Clear pending write flag */
	smsvdp->pending = 0;

	switch(smsvdp->addrmode)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			address = (smsvdp->addr & 0x3fff);
			smsvdp->VRAM->u8(address) = data;
			break;

		case 0x03:
			address = smsvdp->addr & smsvdp->cram_mask;
			if (data != smsvdp->CRAM->u8(address))
			{
				smsvdp->CRAM->u8(address) = data;
				smsvdp->cram_dirty = 1;
			}
			break;
	}

	smsvdp->buffer = data;
	smsvdp->addr += 1;
}


WRITE8_DEVICE_HANDLER( sms_vdp_ctrl_w )
{
	smsvdp_t *smsvdp = get_safe_token(device);

	int reg_num;

	if (smsvdp->pending == 0)
	{
		smsvdp->addr = (smsvdp->addr & 0xff00) | data;
		smsvdp->pending = 1;
	}
	else
	{
		/* Clear pending write flag */
		smsvdp->pending = 0;

		smsvdp->addrmode = (data >> 6) & 0x03;
		smsvdp->addr = (data << 8) | (smsvdp->addr & 0xff);
		switch (smsvdp->addrmode)
		{
		case 0:		/* VRAM reading mode */
			smsvdp->buffer = smsvdp->VRAM->u8(smsvdp->addr & 0x3fff);
			smsvdp->addr += 1;
			break;

		case 1:		/* VRAM writing mode */
			break;

		case 2:		/* VDP register write */
			reg_num = data & 0x0f;
			smsvdp->reg[reg_num] = smsvdp->addr & 0xff;
			if (reg_num == 0 && smsvdp->addr & 0x02)
				logerror("overscan enabled.\n");

			if (reg_num == 0 || reg_num == 1)
				set_display_settings(device);

			if (reg_num == 1)
			{
				device->machine().scheduler().timer_set(device->machine().primary_screen->time_until_pos(device->machine().primary_screen->vpos(), VINT_HPOS), FUNC(smsvdp_check_vint),0 ,smsvdp);
			}
			smsvdp->addrmode = 0;
			break;

		case 3:		/* CRAM writing mode */
			break;
		}
	}
}


static void sms_refresh_line_mode4( smsvdp_t *smsvdp, int *line_buffer, int *priority_selected, int line )
{
	int tile_column;
	int x_scroll, y_scroll, x_scroll_start_column;
	int pixel_x, pixel_plot_x;
	int bit_plane_0, bit_plane_1, bit_plane_2, bit_plane_3;
	int scroll_mod;
	UINT16 name_table_address;
	UINT8 *name_table;

	if (smsvdp->y_pixels != 192)
	{
		name_table_address = ((smsvdp->reg[0x02] & 0x0c) << 10) | 0x0700;
		scroll_mod = 256;
	}
	else
	{
		name_table_address = (smsvdp->reg[0x02] << 10) & 0x3800;
		scroll_mod = 224;
	}

	if (IS_SMS1_VDP)
		name_table_address = name_table_address & (((smsvdp->reg[0x02] & 0x01) << 10) | 0x3bff);

	/* if top 2 rows of screen not affected by horizontal scrolling, then x_scroll = 0 */
	/* else x_scroll = reg[0x08]                                                       */
	x_scroll = (((smsvdp->reg[0x00] & 0x40) && (line < 16)) ? 0 : 0x0100 - smsvdp->reg[0x08]);

	x_scroll_start_column = (x_scroll >> 3);			 /* x starting column tile */

	/* Draw background layer */
	for (tile_column = 0; tile_column < 33; tile_column++)
	{
		UINT16 tile_data;
		int tile_selected, palette_selected, horiz_selected, vert_selected, priority_select;
		int tile_line;

		/* Rightmost 8 columns for SMS (or 2 columns for GG) not affected by */
		/* vertical scrolling when bit 7 of reg[0x00] is set */
		y_scroll = ((smsvdp->reg[0x00] & 0x80) && (tile_column > 23)) ? 0 : smsvdp->reg9copy;

		name_table = smsvdp->VRAM->base() + name_table_address + ((((line + y_scroll) % scroll_mod) >> 3) << 6);

		tile_line = ((tile_column + x_scroll_start_column) & 0x1f) * 2;
		tile_data = name_table[tile_line] | (name_table[tile_line + 1] << 8);

		tile_selected = (tile_data & 0x01ff);
		priority_select = tile_data & PRIORITY_BIT;
		palette_selected = (tile_data >> 11) & 0x01;
		vert_selected = (tile_data >> 10) & 0x01;
		horiz_selected = (tile_data >> 9) & 0x01;

		tile_line = line - ((0x07 - (y_scroll & 0x07)) + 1);
		if (vert_selected)
			tile_line = 0x07 - tile_line;

		bit_plane_0 = smsvdp->VRAM->u8(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x00);
		bit_plane_1 = smsvdp->VRAM->u8(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x01);
		bit_plane_2 = smsvdp->VRAM->u8(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x02);
		bit_plane_3 = smsvdp->VRAM->u8(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x03);

		for (pixel_x = 0; pixel_x < 8 ; pixel_x++)
		{
			UINT8 pen_bit_0, pen_bit_1, pen_bit_2, pen_bit_3;
			UINT8 pen_selected;

			pen_bit_0 = (bit_plane_0 >> (7 - pixel_x)) & 0x01;
			pen_bit_1 = (bit_plane_1 >> (7 - pixel_x)) & 0x01;
			pen_bit_2 = (bit_plane_2 >> (7 - pixel_x)) & 0x01;
			pen_bit_3 = (bit_plane_3 >> (7 - pixel_x)) & 0x01;

			pen_selected = (pen_bit_3 << 3 | pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0);
			if (palette_selected)
				pen_selected |= 0x10;


			if (!horiz_selected)
			{
				pixel_plot_x = pixel_x;
			}
			else
			{
				pixel_plot_x = 7 - pixel_x;
			}
			pixel_plot_x = (0 - (x_scroll & 0x07) + (tile_column << 3) + pixel_plot_x);
			if (pixel_plot_x >= 0 && pixel_plot_x < 256)
			{
//              logerror("%x %x\n", pixel_plot_x + pixel_offset_x, pixel_plot_y);
				line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];
				priority_selected[pixel_plot_x] = priority_select | (pen_selected & 0x0f);
			}
		}
	}
}

static void sms_refresh_mode4_sprites( running_machine &machine, smsvdp_t *smsvdp, int *line_buffer, int *priority_selected, int pixel_plot_y, int line )
{
	int sprite_index;
	int pixel_x, pixel_plot_x;
	int sprite_x, sprite_y, sprite_line, sprite_tile_selected, sprite_height, sprite_zoom;
	int sprite_col_occurred, sprite_col_x;
	int sprite_buffer[8], sprite_buffer_count, sprite_buffer_index;
	int bit_plane_0, bit_plane_1, bit_plane_2, bit_plane_3;
	UINT8 *sprite_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x05] << 7) & 0x3f00);

	/* Draw sprite layer */
	sprite_height = (smsvdp->reg[0x01] & 0x02 ? 16 : 8);
	sprite_zoom = 1;

	if (smsvdp->reg[0x01] & 0x01)		/* sprite doubling */
		sprite_zoom = 2;

	sprite_buffer_count = 0;
	for (sprite_index = 0; (sprite_index < 64) && (sprite_table[sprite_index] != 0xd0 || smsvdp->y_pixels != 192) && (sprite_buffer_count < 9); sprite_index++)
	{
		sprite_y = sprite_table[sprite_index] + 1; /* sprite y position starts at line 1 */

		if (sprite_y > 240)
			sprite_y -= 256; /* wrap from top if y position is > 240 */

		if ((line >= sprite_y) && (line < (sprite_y + sprite_height * sprite_zoom)))
		{
			if (sprite_buffer_count < 8)
			{
				sprite_buffer[sprite_buffer_count] = sprite_index;
			}
			else if (line >= 0 && line < smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V])
			{
				/* Too many sprites per line */
				machine.scheduler().timer_set(machine.primary_screen->time_until_pos(pixel_plot_y + line, SPROVR_HPOS), FUNC(smsvdp_set_status), (int)STATUS_SPROVR, smsvdp);
			}
			sprite_buffer_count++;
		}
	}

	/* Check if display is disabled */
	if (!(smsvdp->reg[0x01] & 0x40))
		return;

	if (sprite_buffer_count > 8)
		sprite_buffer_count = 8;

	memset(smsvdp->collision_buffer, 0, SMS_X_PIXELS);
	sprite_buffer_count--;

	for (sprite_buffer_index = sprite_buffer_count; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		sprite_index = sprite_buffer[sprite_buffer_index];
		sprite_y = sprite_table[sprite_index] + 1; /* sprite y position starts at line 1 */

		if (sprite_y > 240)
			sprite_y -= 256; /* wrap from top if y position is > 240 */

		sprite_x = sprite_table[0x80 + (sprite_index << 1)];

		if (smsvdp->reg[0x00] & 0x08)
			sprite_x -= 0x08;	 /* sprite shift */

		sprite_tile_selected = sprite_table[0x81 + (sprite_index << 1)];

		if (smsvdp->reg[0x06] & 0x04)
			sprite_tile_selected += 256; /* pattern table select */

		if (smsvdp->reg[0x01] & 0x02)
			sprite_tile_selected &= 0x01fe; /* force even index */

		sprite_line = (line - sprite_y) / sprite_zoom;

		if (sprite_line > 0x07)
			sprite_tile_selected += 1;

		bit_plane_0 = smsvdp->VRAM->u8(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x00);
		bit_plane_1 = smsvdp->VRAM->u8(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x01);
		bit_plane_2 = smsvdp->VRAM->u8(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x02);
		bit_plane_3 = smsvdp->VRAM->u8(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x03);

		sprite_col_occurred = 0;
		sprite_col_x = 0;

		for (pixel_x = 0; pixel_x < 8 ; pixel_x++)
		{
			UINT8 pen_bit_0, pen_bit_1, pen_bit_2, pen_bit_3;
			int pen_selected;

			pen_bit_0 = (bit_plane_0 >> (7 - pixel_x)) & 0x01;
			pen_bit_1 = (bit_plane_1 >> (7 - pixel_x)) & 0x01;
			pen_bit_2 = (bit_plane_2 >> (7 - pixel_x)) & 0x01;
			pen_bit_3 = (bit_plane_3 >> (7 - pixel_x)) & 0x01;

			pen_selected = (pen_bit_3 << 3 | pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0) | 0x10;

			if (pen_selected == 0x10)		/* Transparent palette so skip draw */
			{
				continue;
			}

			if (smsvdp->reg[0x01] & 0x01)
			{
				/* sprite doubling is enabled */
				pixel_plot_x = sprite_x + (pixel_x << 1);

				/* check to prevent going outside of active display area */
				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (!(priority_selected[pixel_plot_x] & PRIORITY_BIT))
				{
					line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];
					line_buffer[pixel_plot_x + 1] = smsvdp->current_palette[pen_selected];
				}
				else
				{
					if (priority_selected[pixel_plot_x] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];
					}
					if (priority_selected[pixel_plot_x + 1] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x + 1] = smsvdp->current_palette[pen_selected];
					}
				}
				if (smsvdp->collision_buffer[pixel_plot_x] != 1)
				{
					smsvdp->collision_buffer[pixel_plot_x] = 1;
				}
				else
				{
					if (!sprite_col_occurred)
					{
						sprite_col_occurred = 1;
						sprite_col_x = pixel_plot_x;
					}
				}
				if (smsvdp->collision_buffer[pixel_plot_x + 1] != 1)
				{
					smsvdp->collision_buffer[pixel_plot_x + 1] = 1;
				}
				else
				{
					if (!sprite_col_occurred)
					{
						sprite_col_occurred = 1;
						sprite_col_x = pixel_plot_x;
					}
				}
			}
			else
			{
				pixel_plot_x = sprite_x + pixel_x;

				/* check to prevent going outside of active display area */
				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (!(priority_selected[pixel_plot_x] & PRIORITY_BIT))
				{
					line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];
				}
				else
				{
					if (priority_selected[pixel_plot_x] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];
					}
				}
				if (smsvdp->collision_buffer[pixel_plot_x] != 1)
				{
					smsvdp->collision_buffer[pixel_plot_x] = 1;
				}
				else
				{
					if (!sprite_col_occurred)
					{
						sprite_col_occurred = 1;
						sprite_col_x = pixel_plot_x;
					}
				}
			}
		}
		if (sprite_col_occurred)
			machine.scheduler().timer_set(machine.primary_screen->time_until_pos(pixel_plot_y + line, SPRCOL_BASEHPOS + sprite_col_x), FUNC(smsvdp_set_status), (int)STATUS_SPRCOL, smsvdp);
	}

	/* Fill column 0 with overscan color from reg[0x07] */
	if (smsvdp->reg[0x00] & 0x20)
	{
		line_buffer[0] = smsvdp->current_palette[BACKDROP_COLOR];
		line_buffer[1] = smsvdp->current_palette[BACKDROP_COLOR];
		line_buffer[2] = smsvdp->current_palette[BACKDROP_COLOR];
		line_buffer[3] = smsvdp->current_palette[BACKDROP_COLOR];
		line_buffer[4] = smsvdp->current_palette[BACKDROP_COLOR];
		line_buffer[5] = smsvdp->current_palette[BACKDROP_COLOR];
		line_buffer[6] = smsvdp->current_palette[BACKDROP_COLOR];
		line_buffer[7] = smsvdp->current_palette[BACKDROP_COLOR];
	}
}


static void sms_refresh_tms9918_sprites( running_machine &machine, smsvdp_t *smsvdp, int *line_buffer, int pixel_plot_y, int line )
{
	int pixel_plot_x;
	int sprite_col_occurred, sprite_col_x;
	int sprite_height, sprite_buffer_count, sprite_index, sprite_buffer[5], sprite_buffer_index;
	UINT8 *sprite_table, *sprite_pattern_table;

	/* Draw sprite layer */
	sprite_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x05] & 0x7f) << 7);
	sprite_pattern_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x06] & 0x07) << 11);
	sprite_height = 8;

	if (smsvdp->reg[0x01] & 0x02)                         /* Check if SI is set */
		sprite_height = sprite_height * 2;
	if (smsvdp->reg[0x01] & 0x01)                         /* Check if MAG is set */
		sprite_height = sprite_height * 2;

	sprite_buffer_count = 0;
	for (sprite_index = 0; (sprite_index < 32 * 4) && (sprite_table[sprite_index] != 0xd0) && (sprite_buffer_count < 5); sprite_index += 4)
	{
		int sprite_y = sprite_table[sprite_index] + 1;

		if (sprite_y > 240)
			sprite_y -= 256;

		if ((line >= sprite_y) && (line < (sprite_y + sprite_height)))
		{
			if (sprite_buffer_count < 5)
			{
				sprite_buffer[sprite_buffer_count] = sprite_index;
			}
			else if (line >= 0 && line < smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V])
			{
				/* Too many sprites per line */
				machine.scheduler().timer_set(machine.primary_screen->time_until_pos(pixel_plot_y + line, SPROVR_HPOS), FUNC(smsvdp_set_status), (int)STATUS_SPROVR, smsvdp);
			}
			sprite_buffer_count++;
		}
	}

	/* Check if display is disabled */
	if (!(smsvdp->reg[0x01] & 0x40))
		return;

	if (sprite_buffer_count > 4)
		sprite_buffer_count = 4;

	memset(smsvdp->collision_buffer, 0, SMS_X_PIXELS);
	sprite_buffer_count--;

	for (sprite_buffer_index = sprite_buffer_count; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		int pen_selected;
		int sprite_line, pixel_x, sprite_x, sprite_tile_selected;
		int sprite_y;
		UINT8 pattern;

		sprite_index = sprite_buffer[sprite_buffer_index];
		sprite_y = sprite_table[sprite_index] + 1;

		if (sprite_y > 240)
			sprite_y -= 256;

		sprite_x = sprite_table[sprite_index + 1];
		pen_selected = sprite_table[sprite_index + 3] & 0x0f;

		if (IS_GAMEGEAR_VDP)
			pen_selected |= 0x10;

		if (sprite_table[sprite_index + 3] & 0x80)
			sprite_x -= 32;

		sprite_tile_selected = sprite_table[sprite_index + 2];
		sprite_line = line - sprite_y;

		if (smsvdp->reg[0x01] & 0x01)
			sprite_line >>= 1;

		if (smsvdp->reg[0x01] & 0x02)
		{
			sprite_tile_selected &= 0xfc;

			if (sprite_line > 0x07)
			{
				sprite_tile_selected += 1;
				sprite_line -= 8;
			}
		}

		pattern = sprite_pattern_table[sprite_tile_selected * 8 + sprite_line];

		sprite_col_occurred = 0;
		sprite_col_x = 0;

		for (pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			if (smsvdp->reg[0x01] & 0x01)
			{
				pixel_plot_x = sprite_x + pixel_x * 2;
				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (pen_selected && (pattern & (1 << (7 - pixel_x))))
				{
					line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];

					if (smsvdp->collision_buffer[pixel_plot_x] != 1)
					{
						smsvdp->collision_buffer[pixel_plot_x] = 1;
					}
					else
					{
						if (!sprite_col_occurred)
						{
							sprite_col_occurred = 1;
							sprite_col_x = pixel_plot_x;
						}
					}

					line_buffer[pixel_plot_x+1] = smsvdp->current_palette[pen_selected];

					if (smsvdp->collision_buffer[pixel_plot_x + 1] != 1)
					{
						smsvdp->collision_buffer[pixel_plot_x + 1] = 1;
					}
					else
					{
						if (!sprite_col_occurred)
						{
							sprite_col_occurred = 1;
							sprite_col_x = pixel_plot_x;
						}
					}
				}
			}
			else
			{
				pixel_plot_x = sprite_x + pixel_x;

				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (pen_selected && (pattern & (1 << (7 - pixel_x))))
				{
					line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];

					if (smsvdp->collision_buffer[pixel_plot_x] != 1)
					{
						smsvdp->collision_buffer[pixel_plot_x] = 1;
					}
					else
					{
						if (!sprite_col_occurred)
						{
							sprite_col_occurred = 1;
							sprite_col_x = pixel_plot_x;
						}
					}
				}
			}
		}

		if (smsvdp->reg[0x01] & 0x02)
		{
			sprite_tile_selected += 2;
			pattern = sprite_pattern_table[sprite_tile_selected * 8 + sprite_line];
			sprite_x += (smsvdp->reg[0x01] & 0x01 ? 16 : 8);

			for (pixel_x = 0; pixel_x < 8; pixel_x++)
			{
				if (smsvdp->reg[0x01] & 0x01)
				{
					pixel_plot_x = sprite_x + pixel_x * 2;

					if (pixel_plot_x < 0 || pixel_plot_x > 255)
					{
						continue;
					}

					if (pen_selected && (pattern & (1 << (7 - pixel_x))))
					{
						line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];

						if (smsvdp->collision_buffer[pixel_plot_x] != 1)
						{
							smsvdp->collision_buffer[pixel_plot_x] = 1;
						}
						else
						{
							if (!sprite_col_occurred)
							{
								sprite_col_occurred = 1;
								sprite_col_x = pixel_plot_x;
							}
						}

						line_buffer[pixel_plot_x+1] = smsvdp->current_palette[pen_selected];

						if (smsvdp->collision_buffer[pixel_plot_x + 1] != 1)
						{
							smsvdp->collision_buffer[pixel_plot_x + 1] = 1;
						}
						else
						{
							if (!sprite_col_occurred)
							{
								sprite_col_occurred = 1;
								sprite_col_x = pixel_plot_x;
							}
						}
					}
				}
				else
				{
					pixel_plot_x = sprite_x + pixel_x;

					if (pixel_plot_x < 0 || pixel_plot_x > 255)
					{
						continue;
					}

					if (pen_selected && (pattern & (1 << (7 - pixel_x))))
					{
						line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];

						if (smsvdp->collision_buffer[pixel_plot_x] != 1)
						{
							smsvdp->collision_buffer[pixel_plot_x] = 1;
						}
						else
						{
							if (!sprite_col_occurred)
							{
								sprite_col_occurred = 1;
								sprite_col_x = pixel_plot_x;
							}
						}
					}
				}
			}
		}
		if (sprite_col_occurred)
			machine.scheduler().timer_set(machine.primary_screen->time_until_pos(pixel_plot_y + line, SPRCOL_BASEHPOS + sprite_col_x), FUNC(smsvdp_set_status), (int)STATUS_SPRCOL, smsvdp);
	}
}


static void sms_refresh_line_mode2( smsvdp_t *smsvdp, int *line_buffer, int line )
{
	int tile_column;
	int pixel_x, pixel_plot_x;
	UINT8 *name_table, *color_table, *pattern_table;
	int pattern_mask, color_mask, pattern_offset;

	/* Draw background layer */
	name_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x02] & 0x0f) << 10) + ((line >> 3) * 32);
	color_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x03] & 0x80) << 6);
	color_mask = ((smsvdp->reg[0x03] & 0x7f) << 3) | 0x07;
	pattern_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x04] & 0x04) << 11);
	pattern_mask = ((smsvdp->reg[0x04] & 0x03) << 8) | 0xff;
	pattern_offset = (line & 0xc0) << 2;

	for (tile_column = 0; tile_column < 32; tile_column++)
	{
		UINT8 pattern;
		UINT8 colors;

		pattern = pattern_table[(((pattern_offset + name_table[tile_column]) & pattern_mask) * 8) + (line & 0x07)];
		colors = color_table[(((pattern_offset + name_table[tile_column]) & color_mask) * 8) + (line & 0x07)];

		for (pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			UINT8 pen_selected;

			if (pattern & (1 << (7 - pixel_x)))
			{
				pen_selected = colors >> 4;
			}
			else
			{
				pen_selected = colors & 0x0f;
			}

			if (!pen_selected)
				pen_selected = BACKDROP_COLOR;

			pixel_plot_x = (tile_column << 3) + pixel_x;

			if (IS_GAMEGEAR_VDP)
				pen_selected |= 0x10;

			line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];
		}
	}
}


static void sms_refresh_line_mode0( smsvdp_t *smsvdp, int *line_buffer, int line)
{
	int tile_column;
	int pixel_x, pixel_plot_x;
	UINT8 *name_table, *color_table, *pattern_table;

	/* Draw background layer */
	name_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x02] & 0x0f) << 10) + ((line >> 3) * 32);
	color_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x03] << 6) & (VRAM_SIZE - 1));
	pattern_table = smsvdp->VRAM->base() + ((smsvdp->reg[0x04] << 11) & (VRAM_SIZE - 1));

	for (tile_column = 0; tile_column < 32; tile_column++)
	{
		UINT8 pattern;
		UINT8 colors;

		pattern = pattern_table[(name_table[tile_column] * 8) + (line & 0x07)];
		colors = color_table[name_table[tile_column] >> 3];

		for (pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			int pen_selected;

			if (pattern & (1 << (7 - pixel_x)))
				pen_selected = colors >> 4;
			else
				pen_selected = colors & 0x0f;

			if (IS_GAMEGEAR_VDP)
				pen_selected |= 0x10;

			pixel_plot_x = (tile_column << 3) + pixel_x;
			line_buffer[pixel_plot_x] = smsvdp->current_palette[pen_selected];
		}
	}
}


static void sms_refresh_line( running_machine &machine, smsvdp_t *smsvdp, bitmap_t *bitmap, int pixel_offset_x, int pixel_plot_y, int line )
{
	int x;
	int *blitline_buffer = smsvdp->line_buffer;
	int priority_selected[256];

	switch( smsvdp->vdp_mode )
	{
	case 0:
		if (line >= 0 && line < smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V])
			sms_refresh_line_mode0(smsvdp, blitline_buffer, line);
		sms_refresh_tms9918_sprites(machine, smsvdp, blitline_buffer, pixel_plot_y, line);
		break;

	case 2:
		if (line >= 0 && line < smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V])
			sms_refresh_line_mode2(smsvdp, blitline_buffer, line);
		sms_refresh_tms9918_sprites(machine, smsvdp, blitline_buffer, pixel_plot_y, line);
		break;

	case 4:
	default:
		memset(priority_selected, 0, sizeof(priority_selected));
		if (line >= 0 && line < smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V])
			sms_refresh_line_mode4(smsvdp, blitline_buffer, priority_selected, line);
		sms_refresh_mode4_sprites(machine, smsvdp, blitline_buffer, priority_selected, pixel_plot_y, line);
		break;
	}

	/* Check if display is disabled or we're below/above active area */
	if (!(smsvdp->reg[0x01] & 0x40) || line < 0 || line >= smsvdp->sms_frame_timing[ACTIVE_DISPLAY_V])
	{
		for (x = 0; x < 256; x++)
		{
			blitline_buffer[x] = smsvdp->current_palette[BACKDROP_COLOR];
		}
	}

	if (IS_GAMEGEAR_VDP && smsvdp->gg_sms_mode)
	{
		int *combineline_buffer = smsvdp->line_buffer + ((line & 0x03) + 1) * 256;
		int plot_x = 48;

		/* Do horizontal scaling */
		for (x = 8; x < 248;)
		{
			int combined;

			/* Take red and green from first pixel, and blue from second pixel */
			combined = (blitline_buffer[x] & 0x00ff) | (blitline_buffer[x + 1] & 0x0f00);
			combineline_buffer[plot_x] = combined;

			/* Take red from second pixel, and green and blue from third pixel */
			combined = (blitline_buffer[x + 1] & 0x000f) | (blitline_buffer[x + 2] & 0x0ff0);
			combineline_buffer[plot_x + 1] = combined;
			x += 3;
			plot_x += 2;
		}

		/* Do vertical scaling for a screen with 192 or 224 lines
           Lines 0-2 and 221-223 have no effect on the output on the GG screen.
           We will calculate the gamegear lines as follows:
           GG_0 = 1/6 * SMS_3 + 1/3 * SMS_4 + 1/3 * SMS_5 + 1/6 * SMS_6
           GG_1 = 1/6 * SMS_4 + 1/3 * SMS_5 + 1/3 * SMS_6 + 1/6 * SMS_7
           GG_2 = 1/6 * SMS_6 + 1/3 * SMS_7 + 1/3 * SMS_8 + 1/6 * SMS_9
           GG_3 = 1/6 * SMS_7 + 1/3 * SMS_8 + 1/3 * SMS_9 + 1/6 * SMS_10
           GG_4 = 1/6 * SMS_9 + 1/3 * SMS_10 + 1/3 * SMS_11 + 1/6 * SMS_12
           .....
           GG_142 = 1/6 * SMS_216 + 1/3 * SMS_217 + 1/3 * SMS_218 + 1/6 * SMS_219
           GG_143 = 1/6 * SMS_217 + 1/3 * SMS_218 + 1/3 * SMS_219 + 1/6 * SMS_220
        */
		{
			int gg_line;
			int my_line = pixel_plot_y + line - (TBORDER_START + NTSC_224_TBORDER_Y_PIXELS);
			int *line1, *line2, *line3, *line4;

			/* First make sure there's enough data to draw anything */
			/* We need one more line of data if we're on line 8, 11, 14, 17, etc */
			if (my_line < 6 || my_line > 220 || ((my_line - 8) % 3 == 0))
			{
				return;
			}

			gg_line = ((my_line - 6) / 3) * 2;

			/* If we're on SMS line 7, 10, 13, etc we're on an odd GG line */
			if (my_line % 3)
			{
				gg_line++;
			}

			/* Calculate the line we will be drawing on */
			pixel_plot_y = TBORDER_START + NTSC_192_TBORDER_Y_PIXELS + 24 + gg_line;

			/* Setup our source lines */
			line1 = smsvdp->line_buffer + (((my_line - 3) & 0x03) + 1) * 256;
			line2 = smsvdp->line_buffer + (((my_line - 2) & 0x03) + 1) * 256;
			line3 = smsvdp->line_buffer + (((my_line - 1) & 0x03) + 1) * 256;
			line4 = smsvdp->line_buffer + (((my_line - 0) & 0x03) + 1) * 256;

			for (x = 0 + 48; x < 160 + 48; x++)
			{
				rgb_t	c1 = machine.pens[line1[x]];
				rgb_t	c2 = machine.pens[line2[x]];
				rgb_t	c3 = machine.pens[line3[x]];
				rgb_t	c4 = machine.pens[line4[x]];
				*BITMAP_ADDR32(bitmap, pixel_plot_y, pixel_offset_x + x) =
					MAKE_RGB((RGB_RED(c1) / 6 + RGB_RED(c2) / 3 + RGB_RED(c3) / 3 + RGB_RED(c4) / 6 ),
						(RGB_GREEN(c1) / 6 + RGB_GREEN(c2) / 3 + RGB_GREEN(c3) / 3 + RGB_GREEN(c4) / 6 ),
						(RGB_BLUE(c1) / 6 + RGB_BLUE(c2) / 3 + RGB_BLUE(c3) / 3 + RGB_BLUE(c4) / 6 ) );
			}
			return;
		}
		blitline_buffer = combineline_buffer;
	}

	for (x = 0; x < 256; x++)
	{
		*BITMAP_ADDR32(bitmap, pixel_plot_y + line, pixel_offset_x + x) = machine.pens[blitline_buffer[x]];
	}
}


// This is only used by Light Phaser. Should it be moved elsewhere?
int sms_vdp_check_brightness(device_t *device, int x, int y)
{
	/* brightness of the lightgray color in the frame drawn by Light Phaser games */
	const UINT8 sensor_min_brightness = 0x7f;

	/* TODO: Check how Light Phaser behaves for border areas. For Gangster Town, should */
	/* a shot at right border (HC~=0x90) really appear at active scr, near to left border? */
	if (x < LBORDER_START + LBORDER_X_PIXELS || x >= LBORDER_START + LBORDER_X_PIXELS + 256)
		return 0;

	smsvdp_t *smsvdp = get_safe_token(device);
	rgb_t color = *BITMAP_ADDR32(smsvdp->tmpbitmap, y, x);

	/* reference: http://www.w3.org/TR/AERT#color-contrast */
	UINT8 brightness = (RGB_RED(color) * 0.299) + (RGB_GREEN(color) * 0.587) + (RGB_BLUE(color) * 0.114);
	//printf ("color brightness: %2X for x %d y %d\n", brightness, x, y);

	return (brightness >= sensor_min_brightness) ? 1 : 0;
}


static void sms_update_palette( smsvdp_t *smsvdp )
{
	int i;

	/* Exit if palette is has no changes */
	if (smsvdp->cram_dirty == 0)
	{
		return;
	}
	smsvdp->cram_dirty = 0;

	if (smsvdp->vdp_mode != 4 && ! IS_GAMEGEAR_VDP)
	{
		for(i = 0; i < 16; i++)
		{
			smsvdp->current_palette[i] = 64 + i;
		}
		return;
	}

	if (IS_GAMEGEAR_VDP)
	{
		if (smsvdp->gg_sms_mode)
		{
			for (i = 0; i < 32; i++)
			{
				smsvdp->current_palette[i] = ((smsvdp->CRAM->u8(i) & 0x30) << 6) | ((smsvdp->CRAM->u8(i) & 0x0c ) << 4) | ((smsvdp->CRAM->u8(i) & 0x03) << 2);
			}
		}
		else
		{
			for (i = 0; i < 32; i++)
			{
				smsvdp->current_palette[i] = ((smsvdp->CRAM->u8(i * 2 + 1) << 8) | smsvdp->CRAM->u8(i * 2)) & 0x0fff;
			}
		}
	}
	else
	{
		for (i = 0; i < 32; i++)
		{
			smsvdp->current_palette[i] = smsvdp->CRAM->u8(i) & 0x3f;
		}
	}
}


UINT32 sms_vdp_update( device_t *device, bitmap_t *bitmap, const rectangle *cliprect )
{
	smsvdp_t *smsvdp = get_safe_token(device);
	copybitmap(bitmap, smsvdp->tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( smsvdp )
{
	smsvdp_t *smsvdp = get_safe_token(device);
	const smsvdp_interface *intf = get_interface(device);

	screen_device *screen = device->machine().first_screen();
	int width = screen->width();
	int height = screen->height();

	smsvdp->features = intf->model;
	smsvdp->int_callback = intf->int_callback;
	smsvdp->pause_callback = intf->pause_callback;

	/* Allocate video RAM */
	smsvdp->VRAM = device->machine().region_alloc("vdp_vram", VRAM_SIZE, 1, ENDIANNESS_LITTLE);
	smsvdp->CRAM = device->machine().region_alloc("vdp_cram", MAX_CRAM_SIZE, 1, ENDIANNESS_LITTLE);
	smsvdp->line_buffer = auto_alloc_array(device->machine(), int, 256 * 5);

	smsvdp->collision_buffer = auto_alloc_array(device->machine(), UINT8, SMS_X_PIXELS);
	smsvdp->sms_frame_timing = auto_alloc_array(device->machine(), UINT8, 7);

	/* Make temp bitmap for rendering */
	smsvdp->tmpbitmap = auto_bitmap_alloc(device->machine(), width, height, BITMAP_FORMAT_INDEXED32);

	smsvdp->smsvdp_display_timer = device->machine().scheduler().timer_alloc(FUNC(smsvdp_display_callback), smsvdp);
	smsvdp->smsvdp_display_timer->adjust(screen->time_until_pos(0, DISPLAY_CB_HPOS), 0, screen->scan_period());

	device->save_item(NAME(smsvdp->status));
	device->save_item(NAME(smsvdp->reg9copy));
	device->save_item(NAME(smsvdp->addrmode));
	device->save_item(NAME(smsvdp->addr));
	device->save_item(NAME(smsvdp->cram_mask));
	device->save_item(NAME(smsvdp->cram_dirty));
	device->save_item(NAME(smsvdp->pending));
	device->save_item(NAME(smsvdp->buffer));
	device->save_item(NAME(smsvdp->gg_sms_mode));
	device->save_item(NAME(smsvdp->irq_state));
	device->save_item(NAME(smsvdp->vdp_mode));
	device->save_item(NAME(smsvdp->y_pixels));
	device->save_item(NAME(smsvdp->line_counter));
	device->save_item(NAME(smsvdp->hcounter));
	device->save_item(NAME(smsvdp->reg));
	device->save_item(NAME(smsvdp->current_palette));
	device->save_pointer(NAME(smsvdp->line_buffer), 256 * 5);
	device->save_pointer(NAME(smsvdp->collision_buffer), SMS_X_PIXELS);
	device->save_item(NAME(*smsvdp->tmpbitmap));
}

static DEVICE_RESET( smsvdp )
{
	smsvdp_t *smsvdp = get_safe_token(device);
	int i;

	/* Most register are 0x00 at power-up */
	for (i = 0; i < NUM_OF_REGISTER; i++)
		smsvdp->reg[i] = 0x00;

	smsvdp->reg[0x02] = 0x0e;
	smsvdp->reg[0x0a] = 0xff;

	smsvdp->status = 0;
	smsvdp->reg9copy = 0;
	smsvdp->addrmode = 0;
	smsvdp->addr = 0;
	smsvdp->gg_sms_mode = 0;
	smsvdp->cram_mask = (IS_GAMEGEAR_VDP && !smsvdp->gg_sms_mode) ? (GG_CRAM_SIZE - 1) : (SMS_CRAM_SIZE - 1);
	smsvdp->cram_dirty = 1;
	smsvdp->pending = 0;
	smsvdp->buffer = 0;
	smsvdp->irq_state = 0;
	smsvdp->line_counter = 0;
	smsvdp->hcounter = 0;

	for (i = 0; i < 0x20; i++)
		smsvdp->current_palette[i] = 0;

	set_display_settings(device);

	/* Clear RAM */
	memset(smsvdp->VRAM->base(), 0, VRAM_SIZE);
	memset(smsvdp->CRAM->base(), 0, MAX_CRAM_SIZE);
	memset(smsvdp->line_buffer, 0, 256 * 5 * sizeof(int));
}

DEVICE_GET_INFO( smsvdp )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(smsvdp_t);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(smsvdp);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(smsvdp);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega Master SYstem / Game Gear VDP");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega MS VDP");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME / MESS Team");			break;
	}
}

DEFINE_LEGACY_DEVICE(SMSVDP, smsvdp);
