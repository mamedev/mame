// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    video/pdp1.c

    PDP1 video emulation.

    We emulate three display devices:
    * CRT screen
    * control panel
    * typewriter output

    For the actual emulation of these devices look at the machine/pdp1.c.  This
    file only includes the display routines.

    Raphael Nabet 2002-2004
    Based on earlier work by Chris Salomon
*/

#include <math.h>

#include "emu.h"
#include "cpu/pdp1/pdp1.h"
#include "includes/pdp1.h"




inline void pdp1_state::pdp1_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = color;
}

/*
    video init
*/
void pdp1_state::video_start()
{
	m_typewriter_color = color_typewriter_black;

	/* alloc bitmaps for our private fun */
	m_panel_bitmap.allocate(panel_window_width, panel_window_height, BITMAP_FORMAT_IND16);
	m_typewriter_bitmap.allocate(typewriter_window_width, typewriter_window_height, BITMAP_FORMAT_IND16);

	/* set up out bitmaps */
	pdp1_draw_panel_backdrop(m_panel_bitmap);

	const rectangle typewriter_bitmap_bounds(0, typewriter_window_width-1, 0, typewriter_window_height-1);
	m_typewriter_bitmap.fill(pen_typewriter_bg, typewriter_bitmap_bounds);
}


void pdp1_state::screen_eof_pdp1(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_crt->eof();
	}
}

/*
    schedule a pixel to be plotted
*/
void pdp1_state::pdp1_plot(int x, int y)
{
	/* compute pixel coordinates and plot */
	x = x*crt_window_width/01777;
	y = y*crt_window_height/01777;
	m_crt->plot(x, y);
}


/*
    video_update_pdp1: effectively redraw the screen
*/
UINT32 pdp1_state::screen_update_pdp1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pdp1_erase_lightpen(bitmap);
	m_crt->update(bitmap);
	pdp1_draw_lightpen(bitmap);

	pdp1_draw_panel(m_panel_bitmap);
	copybitmap(bitmap, m_panel_bitmap, 0, 0, panel_window_offset_x, panel_window_offset_y, cliprect);

	copybitmap(bitmap, m_typewriter_bitmap, 0, 0, typewriter_window_offset_x, typewriter_window_offset_y, cliprect);
	return 0;
}



/*
    Operator control panel code
*/

enum
{
	x_panel_col1_offset = 8,
	x_panel_col2_offset = x_panel_col1_offset+144+8,
	x_panel_col3_offset = x_panel_col2_offset+96+8
};

enum
{
	/* column 1: registers, test word, test address */
	y_panel_pc_offset = 0,
	y_panel_ma_offset = y_panel_pc_offset+2*8,
	y_panel_mb_offset = y_panel_ma_offset+2*8,
	y_panel_ac_offset = y_panel_mb_offset+2*8,
	y_panel_io_offset = y_panel_ac_offset+2*8,
	y_panel_ta_offset = y_panel_io_offset+2*8,  /* test address and extend switch */
	y_panel_tw_offset = y_panel_ta_offset+2*8,

	/* column 2: 1-bit indicators */
	y_panel_run_offset = 8,
	y_panel_cyc_offset = y_panel_run_offset+8,
	y_panel_defer_offset = y_panel_cyc_offset+8,
	y_panel_hs_cyc_offset = y_panel_defer_offset+8,
	y_panel_brk_ctr_1_offset = y_panel_hs_cyc_offset+8,
	y_panel_brk_ctr_2_offset = y_panel_brk_ctr_1_offset+8,
	y_panel_ov_offset = y_panel_brk_ctr_2_offset+8,
	y_panel_rim_offset = y_panel_ov_offset+8,
	y_panel_sbm_offset = y_panel_rim_offset+8,
	y_panel_exd_offset = y_panel_sbm_offset+8,
	y_panel_ioh_offset = y_panel_exd_offset+8,
	y_panel_ioc_offset = y_panel_ioh_offset+8,
	y_panel_ios_offset = y_panel_ioc_offset+8,

	/* column 3: power, single step, single inst, sense, flags, instr... */
	y_panel_power_offset = 8,
	y_panel_sngl_step_offset = y_panel_power_offset+8,
	y_panel_sngl_inst_offset = y_panel_sngl_step_offset+8,
	y_panel_sep1_offset = y_panel_sngl_inst_offset+8,
	y_panel_ss_offset = y_panel_sep1_offset+8,
	y_panel_sep2_offset = y_panel_ss_offset+3*8,
	y_panel_pf_offset = y_panel_sep2_offset+8,
	y_panel_ir_offset = y_panel_pf_offset+2*8
};

/* draw a small 8*8 LED (or is this a lamp? ) */
void pdp1_state::pdp1_draw_led(bitmap_ind16 &bitmap, int x, int y, int state)
{
	int xx, yy;

	for (yy=1; yy<7; yy++)
		for (xx=1; xx<7; xx++)
			pdp1_plot_pixel(bitmap, x+xx, y+yy, state ? pen_lit_lamp : pen_unlit_lamp);
}

/* draw nb_bits leds which represent nb_bits bits in value */
void pdp1_state::pdp1_draw_multipleled(bitmap_ind16 &bitmap, int x, int y, int value, int nb_bits)
{
	while (nb_bits)
	{
		nb_bits--;

		pdp1_draw_led(bitmap, x, y, (value >> nb_bits) & 1);

		x += 8;
	}
}


/* draw a small 8*8 switch */
void pdp1_state::pdp1_draw_switch(bitmap_ind16 &bitmap, int x, int y, int state)
{
	int xx, yy;
	int i;

	/* erase area */
	for (yy=0; yy<8; yy++)
		for (xx=0; xx<8; xx++)
			pdp1_plot_pixel(bitmap, x+xx, y+yy, pen_panel_bg);


	/* draw nut (-> circle) */
	for (i=0; i<4;i++)
	{
		pdp1_plot_pixel(bitmap, x+2+i, y+1, pen_switch_nut);
		pdp1_plot_pixel(bitmap, x+2+i, y+6, pen_switch_nut);
		pdp1_plot_pixel(bitmap, x+1, y+2+i, pen_switch_nut);
		pdp1_plot_pixel(bitmap, x+6, y+2+i, pen_switch_nut);
	}
	pdp1_plot_pixel(bitmap, x+2, y+2, pen_switch_nut);
	pdp1_plot_pixel(bitmap, x+5, y+2, pen_switch_nut);
	pdp1_plot_pixel(bitmap, x+2, y+5, pen_switch_nut);
	pdp1_plot_pixel(bitmap, x+5, y+5, pen_switch_nut);

	/* draw button (->disc) */
	if (! state)
		y += 4;
	for (i=0; i<2;i++)
	{
		pdp1_plot_pixel(bitmap, x+3+i, y, pen_switch_button);
		pdp1_plot_pixel(bitmap, x+3+i, y+3, pen_switch_button);
	}
	for (i=0; i<4;i++)
	{
		pdp1_plot_pixel(bitmap, x+2+i, y+1, pen_switch_button);
		pdp1_plot_pixel(bitmap, x+2+i, y+2, pen_switch_button);
	}
}


/* draw nb_bits switches which represent nb_bits bits in value */
void pdp1_state::pdp1_draw_multipleswitch(bitmap_ind16 &bitmap, int x, int y, int value, int nb_bits)
{
	while (nb_bits)
	{
		nb_bits--;

		pdp1_draw_switch(bitmap, x, y, (value >> nb_bits) & 1);

		x += 8;
	}
}


/* write a single char on screen */
void pdp1_state::pdp1_draw_char(bitmap_ind16 &bitmap, char character, int x, int y, int color)
{
	m_gfxdecode->gfx(0)->transpen(bitmap,bitmap.cliprect(), character-32, color, 0, 0,
				x+1, y, 0);
}

/* write a string on screen */
void pdp1_state::pdp1_draw_string(bitmap_ind16 &bitmap, const char *buf, int x, int y, int color)
{
	while (* buf)
	{
		pdp1_draw_char(bitmap, *buf, x, y, color);

		x += 8;
		buf++;
	}
}


/*
    draw the operator control panel (fixed backdrop)
*/
void pdp1_state::pdp1_draw_panel_backdrop(bitmap_ind16 &bitmap)
{
	/* fill with black */
	const rectangle panel_bitmap_bounds(0, panel_window_width-1,    0, panel_window_height-1);
	m_panel_bitmap.fill(pen_panel_bg, panel_bitmap_bounds);

	/* column 1: registers, test word, test address */
	pdp1_draw_string(bitmap, "program counter", x_panel_col1_offset, y_panel_pc_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "memory address", x_panel_col1_offset, y_panel_ma_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "memory buffer", x_panel_col1_offset, y_panel_mb_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "accumulator", x_panel_col1_offset, y_panel_ac_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "in-out", x_panel_col1_offset, y_panel_io_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "extend address", x_panel_col1_offset, y_panel_ta_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "test word", x_panel_col1_offset, y_panel_tw_offset, color_panel_caption);

	/* column separator */
	bitmap.plot_box(x_panel_col2_offset-4, panel_window_offset_y+8, 1, 96, pen_panel_caption);

	/* column 2: 1-bit indicators */
	pdp1_draw_string(bitmap, "run", x_panel_col2_offset+8, y_panel_run_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "cycle", x_panel_col2_offset+8, y_panel_cyc_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "defer", x_panel_col2_offset+8, y_panel_defer_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "h. s. cycle", x_panel_col2_offset+8, y_panel_hs_cyc_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "brk. ctr. 1", x_panel_col2_offset+8, y_panel_brk_ctr_1_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "brk. ctr. 2", x_panel_col2_offset+8, y_panel_brk_ctr_2_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "overflow", x_panel_col2_offset+8, y_panel_ov_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "read in", x_panel_col2_offset+8, y_panel_rim_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "seq. break", x_panel_col2_offset+8, y_panel_sbm_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "extend", x_panel_col2_offset+8, y_panel_exd_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "i-o halt", x_panel_col2_offset+8, y_panel_ioh_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "i-o com'ds", x_panel_col2_offset+8, y_panel_ioc_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "i-o sync", x_panel_col2_offset+8, y_panel_ios_offset, color_panel_caption);

	/* column separator */
	bitmap.plot_box(x_panel_col3_offset-4, panel_window_offset_y+8, 1, 96, pen_panel_caption);

	/* column 3: power, single step, single inst, sense, flags, instr... */
	pdp1_draw_string(bitmap, "power", x_panel_col3_offset+16, y_panel_power_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "single step", x_panel_col3_offset+16, y_panel_sngl_step_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "single inst.", x_panel_col3_offset+16, y_panel_sngl_inst_offset, color_panel_caption);
	/* separator */
	bitmap.plot_box(x_panel_col3_offset+8, y_panel_sep1_offset+4, 96, 1, pen_panel_caption);
	pdp1_draw_string(bitmap, "sense switches", x_panel_col3_offset, y_panel_ss_offset, color_panel_caption);
	/* separator */
	bitmap.plot_box(x_panel_col3_offset+8, y_panel_sep2_offset+4, 96, 1, pen_panel_caption);
	pdp1_draw_string(bitmap, "program flags", x_panel_col3_offset, y_panel_pf_offset, color_panel_caption);
	pdp1_draw_string(bitmap, "instruction", x_panel_col3_offset, y_panel_ir_offset, color_panel_caption);
}

/*
    draw the operator control panel (dynamic elements)
*/
void pdp1_state::pdp1_draw_panel(bitmap_ind16 &bitmap)
{
	/* column 1: registers, test word, test address */
	pdp1_draw_multipleled(bitmap, x_panel_col1_offset+16, y_panel_pc_offset+8,m_maincpu->state_int(PDP1_PC), 16);
	pdp1_draw_multipleled(bitmap, x_panel_col1_offset+16, y_panel_ma_offset+8,m_maincpu->state_int(PDP1_MA), 16);
	pdp1_draw_multipleled(bitmap, x_panel_col1_offset, y_panel_mb_offset+8,m_maincpu->state_int(PDP1_MB), 18);
	pdp1_draw_multipleled(bitmap, x_panel_col1_offset, y_panel_ac_offset+8,m_maincpu->state_int(PDP1_AC), 18);
	pdp1_draw_multipleled(bitmap, x_panel_col1_offset, y_panel_io_offset+8,m_maincpu->state_int(PDP1_IO), 18);
	pdp1_draw_switch(bitmap, x_panel_col1_offset, y_panel_ta_offset+8,m_maincpu->state_int(PDP1_EXTEND_SW));
	pdp1_draw_multipleswitch(bitmap, x_panel_col1_offset+16, y_panel_ta_offset+8,m_maincpu->state_int(PDP1_TA), 16);
	pdp1_draw_multipleswitch(bitmap, x_panel_col1_offset, y_panel_tw_offset+8,m_maincpu->state_int(PDP1_TW), 18);

	/* column 2: 1-bit indicators */
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_run_offset,m_maincpu->state_int(PDP1_RUN));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_cyc_offset,m_maincpu->state_int(PDP1_CYC));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_defer_offset,m_maincpu->state_int(PDP1_DEFER));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_hs_cyc_offset, 0);  /* not emulated */
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_brk_ctr_1_offset,m_maincpu->state_int(PDP1_BRK_CTR) & 1);
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_brk_ctr_2_offset,m_maincpu->state_int(PDP1_BRK_CTR) & 2);
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_ov_offset,m_maincpu->state_int(PDP1_OV));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_rim_offset,m_maincpu->state_int(PDP1_RIM));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_sbm_offset,m_maincpu->state_int(PDP1_SBM));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_exd_offset,m_maincpu->state_int(PDP1_EXD));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_ioh_offset,m_maincpu->state_int(PDP1_IOH));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_ioc_offset,m_maincpu->state_int(PDP1_IOC));
	pdp1_draw_led(bitmap, x_panel_col2_offset, y_panel_ios_offset,m_maincpu->state_int(PDP1_IOS));

	/* column 3: power, single step, single inst, sense, flags, instr... */
	pdp1_draw_led(bitmap, x_panel_col3_offset, y_panel_power_offset, 1);   /* always on */
	pdp1_draw_switch(bitmap, x_panel_col3_offset+8, y_panel_power_offset, 1);  /* always on */
	pdp1_draw_led(bitmap, x_panel_col3_offset, y_panel_sngl_step_offset,m_maincpu->state_int(PDP1_SNGL_STEP));
	pdp1_draw_switch(bitmap, x_panel_col3_offset+8, y_panel_sngl_step_offset,m_maincpu->state_int(PDP1_SNGL_STEP));
	pdp1_draw_led(bitmap, x_panel_col3_offset, y_panel_sngl_inst_offset,m_maincpu->state_int(PDP1_SNGL_INST));
	pdp1_draw_switch(bitmap, x_panel_col3_offset+8, y_panel_sngl_inst_offset,m_maincpu->state_int(PDP1_SNGL_INST));
	pdp1_draw_multipleled(bitmap, x_panel_col3_offset, y_panel_ss_offset+8,m_maincpu->state_int(PDP1_SS), 6);
	pdp1_draw_multipleswitch(bitmap, x_panel_col3_offset, y_panel_ss_offset+16,m_maincpu->state_int(PDP1_SS), 6);
	pdp1_draw_multipleled(bitmap, x_panel_col3_offset, y_panel_pf_offset+8,m_maincpu->state_int(PDP1_PF), 6);
	pdp1_draw_multipleled(bitmap, x_panel_col3_offset, y_panel_ir_offset+8,m_maincpu->state_int(PDP1_IR), 5);
}


/*
    Typewriter code
*/




enum
{
	typewriter_line_height = 8,
	typewriter_write_offset_y = typewriter_window_height-typewriter_line_height,
	typewriter_scroll_step = typewriter_line_height
};

enum
{
	tab_step = 8
};


void pdp1_state::pdp1_typewriter_linefeed()
{
	UINT8 buf[typewriter_window_width];
	int y;

	for (y=0; y<typewriter_window_height-typewriter_scroll_step; y++)
	{
		extract_scanline8(m_typewriter_bitmap, 0, y+typewriter_scroll_step, typewriter_window_width, buf);
		draw_scanline8(m_typewriter_bitmap, 0, y, typewriter_window_width, buf, m_palette->pens());
	}

	const rectangle typewriter_scroll_clear_window(0, typewriter_window_width-1,    typewriter_window_height-typewriter_scroll_step, typewriter_window_height-1);
	m_typewriter_bitmap.fill(pen_typewriter_bg, typewriter_scroll_clear_window);
}

void pdp1_state::pdp1_typewriter_drawchar(int character)
{
	static const char ascii_table[2][64] =
	{   /* n-s = non-spacing */
		{   /* lower case */
			' ',                '1',                '2',                '3',
			'4',                '5',                '6',                '7',
			'8',                '9',                '*',                '*',
			'*',                '*',                '*',                '*',
			'0',                '/',                's',                't',
			'u',                'v',                'w',                'x',
			'y',                'z',                '*',                ',',
			'*',/*black*/       '*',/*red*/         '*',/*Tab*/         '*',
			'\200',/*n-s middle dot*/'j',           'k',                'l',
			'm',                'n',                'o',                'p',
			'q',                'r',                '*',                '*',
			'-',                ')',                '\201',/*n-s overstrike*/'(',
			'*',                'a',                'b',                'c',
			'd',                'e',                'f',                'g',
			'h',                'i',                '*',/*Lower Case*/  '.',
			'*',/*Upper Case*/  '*',/*Backspace*/   '*',                '*'/*Carriage Return*/
		},
		{   /* upper case */
			' ',                '"',                '\'',               '~',
			'\202',/*implies*/  '\203',/*or*/       '\204',/*and*/      '<',
			'>',                '\205',/*up arrow*/ '*',                '*',
			'*',                '*',                '*',                '*',
			'\206',/*right arrow*/'?',              'S',                'T',
			'U',                'V',                'W',                'X',
			'Y',                'Z',                '*',                '=',
			'*',/*black*/       '*',/*red*/         '*',/*Tab*/         '*',
			'_',/*n-s???*/      'J',                'K',                'L',
			'M',                'N',                'O',                'P',
			'Q',                'R',                '*',                '*',
			'+',                ']',                '|',/*n-s???*/      '[',
			'*',                'A',                'B',                'C',
			'D',                'E',                'F',                'G',
			'H',                'I',                '*',/*Lower Case*/  '\207',/*multiply*/
			'*',/*Upper Case*/  '*',/*Backspace*/   '*',                '*'/*Carriage Return*/
		}
	};



	character &= 0x3f;

	switch (character)
	{
	case 034:
		/* Black */
		m_typewriter_color = color_typewriter_black;
		break;

	case 035:
		/* Red */
		m_typewriter_color = color_typewriter_red;
		break;

	case 036:
		/* Tab */
		m_pos = m_pos + tab_step - (m_pos % tab_step);
		break;

	case 072:
		/* Lower case */
		m_case_shift = 0;
		break;

	case 074:
		/* Upper case */
		m_case_shift = 1;
		break;

	case 075:
		/* Backspace */
		if (m_pos)
			m_pos--;
		break;

	case 077:
		/* Carriage Return */
		m_pos = 0;
		pdp1_typewriter_linefeed();
		break;

	default:
		/* Any printable character... */

		if (m_pos >= 80)
		{   /* if past right border, wrap around. (Right???) */
			pdp1_typewriter_linefeed();  /* next line */
			m_pos = 0;                   /* return to start of line */
		}

		/* print character (lookup ASCII equivalent in table) */
		pdp1_draw_char(m_typewriter_bitmap, ascii_table[m_case_shift][character],
						8*m_pos, typewriter_write_offset_y,
						m_typewriter_color); /* print char */

		if ((character!= 040) && (character!= 056)) /* 040 and 056 are non-spacing characters */
			m_pos++;     /* step carriage forward */

		break;
	}
}



/*
    lightpen code
*/

void pdp1_state::pdp1_update_lightpen_state(const lightpen_t *new_state)
{
	m_lightpen_state = *new_state;
}

#if 1
void pdp1_state::pdp1_draw_circle(bitmap_ind16 &bitmap, int x, int y, int radius, int color_)
{
	int interval;
	int a;

	x = x*crt_window_width/01777;
	y = y*crt_window_width/01777;
	radius = radius*crt_window_width/01777;

	interval = ceil(radius/sqrt(2.));

	for (a=0; a<=interval; a++)
	{
		int b = sqrt((double)radius*radius-a*a) + .5;

		if ((x-a >= 0) && (y-b >= 0))
			pdp1_plot_pixel(bitmap, x-a, y-b, color_);
		if ((x-a >= 0) && (y+b <= crt_window_height-1))
			pdp1_plot_pixel(bitmap, x-a, y+b, color_);
		if ((x+a <= crt_window_width-1) && (y-b >= 0))
			pdp1_plot_pixel(bitmap, x+a, y-b, color_);
		if ((x+a <= crt_window_width-1) && (y+b <= crt_window_height-1))
			pdp1_plot_pixel(bitmap, x+a, y+b, color_);

		if ((x-b >= 0) && (y-a >= 0))
			pdp1_plot_pixel(bitmap, x-b, y-a, color_);
		if ((x-b >= 0) && (y+a <= crt_window_height-1))
			pdp1_plot_pixel(bitmap, x-b, y+a, color_);
		if ((x+b <= crt_window_width-1) && (y-a >= 0))
			pdp1_plot_pixel(bitmap, x+b, y-a, color_);
		if ((x+b <= crt_window_width-1) && (y+a <= crt_window_height-1))
			pdp1_plot_pixel(bitmap, x+b, y+a, color_);
	}
}
#else
void pdp1_state::pdp1_draw_circle(bitmap_ind16 &bitmap, int x, int y, int radius, int color)
{
	float fx, fy;
	float interval;


	fx = (float)x*crt_window_width/01777;
	fy = (float)y*crt_window_height/01777;

	interval = radius/sqrt(2.);

	for (x=/*ceil*/(fx-interval); x<=fx+interval; x++)
	{
		float dy = sqrt(radius*radius-(x-fx)*(x-fx));

		if ((x >= 0) && (x <= crt_window_width-1) && (fy-dy >= 0))
			pdp1_plot_pixel(bitmap, x, fy-dy, color);
		if ((x >= 0) && (x <= crt_window_width-1) && (y+dy <= crt_window_height-1))
			pdp1_plot_pixel(bitmap, x, fy+dy, color);
	}
	for (y=/*ceil*/(fy-interval); y<=fy+interval; y++)
	{
		float dx = sqrt(radius*radius-(y-fy)*(y-fy));

		if ((fx-dx >= 0) && (y >= 0) && (y <= crt_window_height-1))
			pdp1_plot_pixel(bitmap, fx-dx, y, color);
		if ((fx+dx <= crt_window_width-1) && (y >= 0) && (y <= crt_window_height-1))
			pdp1_plot_pixel(bitmap, fx+dx, y, color);
	}
}
#endif

void pdp1_state::pdp1_erase_lightpen(bitmap_ind16 &bitmap)
{
	if (m_previous_lightpen_state.active)
	{
#if 0
		if (m_previous_lightpen_state.x>0)
			pdp1_plot_pixel(bitmap, m_previous_lightpen_state.x/2-1, m_previous_lightpen_state.y/2, pen_black);
		if (m_previous_lightpen_state.x<1023)
			pdp1_plot_pixel(bitmap, m_previous_lightpen_state.x/2+1, m_previous_lightpen_state.y/2, pen_black);
		if (m_previous_lightpen_state.y>0)
			pdp1_plot_pixel(bitmap, m_previous_lightpen_state.x/2, m_previous_lightpen_state.y/2-1, pen_black);
		if (m_previous_lightpen_state.y<1023)
			pdp1_plot_pixel(bitmap, m_previous_lightpen_state.x/2, m_previous_lightpen_state.y/2+1, pen_black);
#endif
		pdp1_draw_circle(bitmap, m_previous_lightpen_state.x, m_previous_lightpen_state.y, m_previous_lightpen_state.radius, pen_black);
	}
}

void pdp1_state::pdp1_draw_lightpen(bitmap_ind16 &bitmap)
{
	if (m_lightpen_state.active)
	{
		int color_ = m_lightpen_state.down ? pen_lightpen_pressed : pen_lightpen_nonpressed;
#if 0
		if (m_lightpen_state.x>0)
			pdp1_plot_pixel(bitmap, m_lightpen_state.x/2-1, m_lightpen_state.y/2, color);
		if (m_lightpen_state.x<1023)
			pdp1_plot_pixel(bitmap, m_lightpen_state.x/2+1, m_lightpen_state.y/2, color);
		if (m_lightpen_state.y>0)
			pdp1_plot_pixel(bitmap, m_lightpen_state.x/2, m_lightpen_state.y/2-1, color);
		if (m_lightpen_state.y<1023)
			pdp1_plot_pixel(bitmap, m_lightpen_state.x/2, m_lightpen_state.y/2+1, color);
#endif
		pdp1_draw_circle(bitmap, m_lightpen_state.x, m_lightpen_state.y, m_lightpen_state.radius, color_);
	}
	m_previous_lightpen_state = m_lightpen_state;
}
