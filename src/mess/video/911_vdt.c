/*
    TI 911 VDT core.  To be operated with the TI 990 line of computers (can be connected to
    any model, as communication uses the CRU bus).

    Raphael Nabet 2002

TODO:
    * add more flexibility, so that we can create multiple-terminal configurations.
    * support test mode???
*/


#include "emu.h"
#include "911_vdt.h"
#include "911_chr.h"
#include "911_key.h"
#include "sound/beep.h"



#define MAX_VDT 1

static const gfx_layout fontlayout_7bit =
{
	7, 10,			/* 7*10 characters */
	128,			/* 128 characters */
	1,				/* 1 bit per pixel */
	{ 0 },
	{ 1, 2, 3, 4, 5, 6, 7 },			/* straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	10*8			/* every char takes 10 consecutive bytes */
};

static const gfx_layout fontlayout_8bit =
{
	7, 10,			/* 7*10 characters */
	128,			/* 128 characters */
	1,				/* 1 bit per pixel */
	{ 0 },
	{ 1, 2, 3, 4, 5, 6, 7 },				/* straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	10*8			/* every char takes 10 consecutive bytes */
};

GFXDECODE_START( vdt911 )
	/* array must use same order as vdt911_model_t!!! */
	/* US */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_US_chr_offset, fontlayout_7bit, 0, 4 )
	/* UK */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_UK_chr_offset, fontlayout_7bit, 0, 4 )
	/* French */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_US_chr_offset, fontlayout_7bit, 0, 4 )
	/* German */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_german_chr_offset, fontlayout_7bit, 0, 4 )
	/* Swedish */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_swedish_chr_offset, fontlayout_7bit, 0, 4 )
	/* Norwegian */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_norwegian_chr_offset, fontlayout_7bit, 0, 4 )
	/* Japanese */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_japanese_chr_offset, fontlayout_8bit, 0, 4 )
	/* Arabic */
	/* GFXDECODE_ENTRY( vdt911_chr_region, vdt911_arabic_chr_offset, fontlayout_8bit, 0, 4 ) */
	/* FrenchWP */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_frenchWP_chr_offset, fontlayout_7bit, 0, 4 )
GFXDECODE_END

static const unsigned char vdt911_colors[] =
{
	0x00,0x00,0x00,	/* black */
	0xC0,0xC0,0xC0,	/* low intensity */
	0xFF,0xFF,0xFF	/* high intensity */
};

static const unsigned short vdt911_palette[] =
{
	0, 2,	/* high intensity */
	0, 1,	/* low intensity */
	2, 0,	/* high intensity, reverse */
	2, 1	/* low intensity, reverse */
};

typedef struct vdt_t
{
	vdt911_screen_size_t screen_size;	/* char_960 for 960-char, 12-line model; char_1920 for 1920-char, 24-line model */
	vdt911_model_t model;				/* country code */
	void (*int_callback)(running_machine &machine, int state);	/* interrupt callback, called when the state of irq changes */

	UINT8 data_reg;						/* vdt911 write buffer */
	UINT8 display_RAM[2048];			/* vdt911 char buffer (1kbyte for 960-char model, 2kbytes for 1920-char model) */

	unsigned int cursor_address;		/* current cursor address (controlled by the computer, affects both display and I/O protocol) */
	unsigned int cursor_address_mask;	/* 1023 for 960-char model, 2047 for 1920-char model */

	emu_timer *beep_timer;					/* beep clock (beeps ends when timer times out) */
	/*void *blink_clock;*/				/* cursor blink clock */

	UINT8 keyboard_data;				/* last code pressed on keyboard */
	unsigned int keyboard_data_ready : 1;		/* true if there is a new code in keyboard_data */
	unsigned int keyboard_interrupt_enable : 1;	/* true when keybord interrupts are enabled */

	unsigned int display_enable : 1;		/* screen is black when false */
	unsigned int dual_intensity_enable : 1;	/* if true, MSBit of ASCII codes controls character highlight */
	unsigned int display_cursor : 1;		/* if true, the current cursor location is displayed on screen */
	unsigned int blinking_cursor_enable : 1;/* if true, the cursor will blink when displayed */
	unsigned int blink_state : 1;			/* current cursor blink state */

	unsigned int word_select : 1;			/* CRU interface mode */
	unsigned int previous_word_select : 1;	/* value of word_select is saved here */

	UINT8 last_key_pressed;
	int last_modifier_state;
	char foreign_mode;
} vdt_t;

/*
    Macros for model features
*/
/* TRUE for japanese and arabic terminals, which use 8-bit charcodes and keyboard shift modes */
#define USES_8BIT_CHARCODES(vdt) ((vdt->model == vdt911_model_Japanese) /*|| (vdt->model == vdt911_model_Arabic)*/)
/* TRUE for keyboards which have this extra key (on the left of TAB/SKIP)
    (Most localized keyboards have it) */
#define HAS_EXTRA_KEY_67(vdt) (! ((vdt->model == vdt911_model_US) || (vdt->model == vdt911_model_UK) || (vdt->model == vdt911_model_French)))
/* TRUE for keyboards which have this extra key (on the right of space),
    AND do not use it as a modifier */
#define HAS_EXTRA_KEY_91(vdt) ((vdt->model == vdt911_model_German) || (vdt->model == vdt911_model_Swedish) || (vdt->model == vdt911_model_Norwegian))

static TIMER_CALLBACK(blink_callback);
static TIMER_CALLBACK(beep_callback);

/*
    Initialize vdt911 palette
*/
PALETTE_INIT( vdt911 )
{
	UINT8 i, r, g, b;

	machine.colortable = colortable_alloc(machine, 3);

	for ( i = 0; i < 3; i++ )
	{
		r = vdt911_colors[i*3]; g = vdt911_colors[i*3+1]; b = vdt911_colors[i*3+2];
		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	for(i=0;i<8;i++)
		colortable_entry_set_value(machine.colortable, i, vdt911_palette[i]);
}

/*
    Copy a character bitmap array to another location in memory
*/
static void copy_character_matrix_array(const UINT8 char_array[128][10], UINT8 *dest)
{
	int i, j;

	for (i=0; i<128; i++)
		for (j=0; j<10; j++)
			*(dest++) = char_array[i][j];
}

/*
    Patch a character bitmap array according to an array of char_override_t
*/
static void apply_char_overrides(int nb_char_overrides, const char_override_t char_overrides[], UINT8 *dest)
{
	int i, j;

	for (i=0; i<nb_char_overrides; i++)
	{
		for (j=0; j<10; j++)
			dest[char_overrides[i].char_index*10+j] = char_defs[char_overrides[i].symbol_index][j];
	}
}

/*
    Initialize the 911 vdt core
*/
void vdt911_init(running_machine &machine)
{
	UINT8 *base;
	UINT8 *chr = machine.root_device().memregion(vdt911_chr_region)->base();

	/* set up US character definitions */
	base = chr+vdt911_US_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);

	/* set up UK character definitions */
	base = chr+vdt911_UK_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(UK_overrides)/sizeof(char_override_t), UK_overrides, base);

	/* French character set is identical to US character set */

	/* set up German character definitions */
	base = chr+vdt911_german_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(german_overrides)/sizeof(char_override_t), german_overrides, base);

	/* set up Swedish/Finnish character definitions */
	base = chr+vdt911_swedish_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(swedish_overrides)/sizeof(char_override_t), swedish_overrides, base);

	/* set up Norwegian/Danish character definitions */
	base = chr+vdt911_norwegian_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(norwegian_overrides)/sizeof(char_override_t), norwegian_overrides, base);

	/* set up Katakana Japanese character definitions */
	base = chr+vdt911_japanese_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(japanese_overrides)/sizeof(char_override_t), japanese_overrides, base);
	base = chr+vdt911_japanese_chr_offset+128*vdt911_single_char_len;
	copy_character_matrix_array(char_defs+char_defs_katakana_base, base);

#if 0
	/* set up Arabic character definitions */
	base = chr+vdt911_arabic_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(arabic_overrides)/sizeof(char_override_t), arabic_overrides, base);
	base = chr+vdt911_arabic_chr_offset+128*vdt911_single_char_len;
	copy_character_matrix_array(char_defs+char_defs_arabic_base, base);
#endif

	/* set up French word processing character definitions */
	base = chr+vdt911_frenchWP_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(frenchWP_overrides)/sizeof(char_override_t), frenchWP_overrides, base);
}

static TIMER_CALLBACK(setup_beep)
{
	beep_set_frequency(machine.device(BEEPER_TAG), 2000);
}


INLINE vdt_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == VDT911);

	return (vdt_t *)downcast<vdt911_device *>(device)->token();
}

/*
    Initialize one 911 vdt controller/terminal
*/
static DEVICE_START( vdt911 )
{
	vdt_t *vdt = get_safe_token(device);
	const vdt911_init_params_t *params = (const vdt911_init_params_t *)device->static_config();
	vdt->last_key_pressed = 0x80;
	vdt->screen_size = params->screen_size;
	vdt->model = params->model;
	vdt->int_callback = params->int_callback;

	if (vdt->screen_size == char_960)
		vdt->cursor_address_mask = 0x3ff;	/* 1kb of RAM */
	else
		vdt->cursor_address_mask = 0x7ff;	/* 2 kb of RAM */

	device->machine().scheduler().timer_set(attotime::zero, FUNC(setup_beep), 0, vdt);

	/* set up cursor blink clock.  2Hz frequency -> .25s half-period. */
	/*vdt->blink_clock =*/ device->machine().scheduler().timer_pulse(attotime::from_msec(250), FUNC(blink_callback), 0, vdt);

	/* alloc beep timer */
	vdt->beep_timer = device->machine().scheduler().timer_alloc(FUNC(beep_callback));
}

const device_type VDT911 = &device_creator<vdt911_device>;

vdt911_device::vdt911_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VDT911, "911 VDT", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(vdt_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vdt911_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vdt911_device::device_start()
{
	DEVICE_START_NAME( vdt911 )(this);
}



/*
    timer callback to toggle blink state
*/
static TIMER_CALLBACK(blink_callback)
{
	vdt_t *vdt = (vdt_t *)ptr;
	vdt->blink_state = !vdt->blink_state;
}

/*
    timer callback to stop beep generator
*/
static TIMER_CALLBACK(beep_callback)
{
	beep_set_state(machine.device(BEEPER_TAG), 0);
}

/*
    CRU interface read
*/
READ8_DEVICE_HANDLER( vdt911_cru_r )
{
	vdt_t *vdt = get_safe_token(device);
	int reply=0;

	offset &= 0x1;

	if (! vdt->word_select)
	{	/* select word 0 */
		switch (offset)
		{
		case 0:
			reply = vdt->display_RAM[vdt->cursor_address];
			break;

		case 1:
			reply = vdt->keyboard_data & 0x7f;
			if (vdt->keyboard_data_ready)
				reply |= 0x80;
			break;
		}
	}
	else
	{	/* select word 1 */
		switch (offset)
		{
		case 0:
			reply = vdt->cursor_address & 0xff;
			break;

		case 1:
			reply = (vdt->cursor_address >> 8) & 0x07;
			if (vdt->keyboard_data & 0x80)
				reply |= 0x08;
			/*if (! vdt->terminal_ready)
                reply |= 0x10;*/
			if (vdt->previous_word_select)
				reply |= 0x20;
			/*if (vdt->keyboard_parity_error)
                reply |= 0x40;*/
			if (vdt->keyboard_data_ready)
				reply |= 0x80;
			break;
		}
	}

	return reply;
}

/*
    CRU interface write
*/
WRITE8_DEVICE_HANDLER( vdt911_cru_w )
{
	vdt_t *vdt = get_safe_token(device);
	offset &= 0xf;

	if (! vdt->word_select)
	{	/* select word 0 */
		switch (offset)
		{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
			/* display memory write data */
			if (data)
				vdt->data_reg |= (1 << offset);
			else
				vdt->data_reg &= ~ (1 << offset);
			break;

		case 0x8:
			/* write data strobe */
			 vdt->display_RAM[vdt->cursor_address] = vdt->data_reg;
			break;

		case 0x9:
			/* test mode */
			/* ... */
			break;

		case 0xa:
			/* cursor move */
			if (data)
				vdt->cursor_address--;
			else
				vdt->cursor_address++;
			vdt->cursor_address &= vdt->cursor_address_mask;
			break;

		case 0xb:
			/* blinking cursor enable */
			vdt->blinking_cursor_enable = data;
			break;

		case 0xc:
			/* keyboard interrupt enable */
			vdt->keyboard_interrupt_enable = data;
			(*vdt->int_callback)(device->machine(), vdt->keyboard_interrupt_enable && vdt->keyboard_data_ready);
			break;

		case 0xd:
			/* dual intensity enable */
			vdt->dual_intensity_enable = data;
			break;

		case 0xe:
			/* display enable */
			vdt->display_enable = data;
			break;

		case 0xf:
			/* select word */
			vdt->previous_word_select = vdt->word_select;
			vdt->word_select = data;
			break;
		}
	}
	else
	{	/* select word 1 */
		switch (offset)
		{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
			/* cursor address */
			if (data)
				vdt->cursor_address |= (1 << offset);
			else
				vdt->cursor_address &= ~ (1 << offset);
			vdt->cursor_address &= vdt->cursor_address_mask;
			break;

		case 0xb:
			/* not used */
			break;

		case 0xc:
			/* display cursor */
			vdt->display_cursor = data;
			break;

		case 0xd:
			/* keyboard acknowledge */
			if (vdt->keyboard_data_ready)
			{
				vdt->keyboard_data_ready = 0;
				if (vdt->keyboard_interrupt_enable)
					(*vdt->int_callback)(device->machine(), 0);
			}
			/*vdt->keyboard_parity_error = 0;*/
			break;

		case 0xe:
			/* beep enable strobe - not tested */
			beep_set_state(device->machine().device(BEEPER_TAG), 1);

			vdt->beep_timer->adjust(attotime::from_usec(300));
			break;

		case 0xf:
			/* select word */
			vdt->previous_word_select = vdt->word_select;
			vdt->word_select = data;
			break;
		}
	}
}

/*
    Video refresh
*/
void vdt911_refresh(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y)
{
	vdt_t *vdt = get_safe_token(device);
	gfx_element *gfx = device->machine().gfx[vdt->model];
	int height = (vdt->screen_size == char_960) ? 12 : /*25*/24;
	int use_8bit_charcodes = USES_8BIT_CHARCODES(vdt);
	int address = 0;
	int i, j;
	int cur_char;
	int color;

	/*if (use_8bit_charcodes)
        color = vdt->dual_intensity_enable ? 1 : 0;*/

	if (! vdt->display_enable)
	{
		rectangle my_rect(x, x + 80*7 - 1, y, y + height*10 - 1);

		bitmap.fill(0, my_rect);
	}
	else
		for (i=0; i<height; i++)
		{
			for (j=0; j<80; j++)
			{
				cur_char = vdt->display_RAM[address];
				/* does dual intensity work with 8-bit character set? */
				color = (vdt->dual_intensity_enable && (cur_char & 0x80)) ? 1 : 0;
				if (! use_8bit_charcodes)
					cur_char &= 0x7f;

				/* display cursor in reverse video */
				if ((address == vdt->cursor_address) && vdt->display_cursor
						&& ((! vdt->blinking_cursor_enable) || vdt->blink_state))
					color += 2;

				address++;

				drawgfx_opaque(bitmap, cliprect, gfx, cur_char, color, 0, 0,
						x+j*7, y+i*10);
			}
		}
}

static const unsigned char (*const key_translate[])[91] =
{	/* array must use same order as vdt911_model_t!!! */
	/* US */
	US_key_translate,
	/* UK */
	US_key_translate,
	/* French */
	French_key_translate,
	/* German */
	German_key_translate,
	/* Swedish */
	Swedish_key_translate,
	/* Norwegian */
	Norwegian_key_translate,
	/* Japanese */
	Japanese_key_translate,
	/* Arabic */
	/*Arabic_key_translate,*/
	/* FrenchWP */
	FrenchWP_key_translate
};

/*
    keyboard handler: should be called regularly by machine code, for instance
    every Video Blank Interrupt.
*/
void vdt911_keyboard(device_t *device)
{
	vdt_t *vdt = get_safe_token(device);

	typedef enum
	{
		/* states for western keyboards and katakana/arabic keyboards in romaji/latin mode */
		lower_case = 0, upper_case, shift, control,
		/* states for katakana/arabic keyboards in katakana/arabic mode */
		foreign, foreign_shift,
		/* special value to stop repeat if the modifier state changes */
		special_debounce = -1
	} modifier_state_t;

	static unsigned char repeat_timer;
	enum { repeat_delay = 5 /* approx. 1/10s */ };

	UINT16 key_buf[6];
	int i, j;
	modifier_state_t modifier_state;
	int repeat_mode;

	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	/* read current key state */
	for (i = 0; i < 6; i++)
	{
		key_buf[i] = device->machine().root_device().ioport(keynames[i])->read();
	}

	/* parse modifier keys */
	if ((USES_8BIT_CHARCODES(vdt))
		&& ((key_buf[5] & 0x0400) || ((!(key_buf[5] & 0x0100)) && vdt->foreign_mode)))
	{	/* we are in katakana/arabic mode */
		vdt->foreign_mode = TRUE;

		if ((key_buf[4] & 0x0400) || (key_buf[5] & 0x0020))
			modifier_state = foreign_shift;
		else
			modifier_state = foreign;
	}
	else
	{	/* we are using a western keyboard, or a katakana/arabic keyboard in
        romaji/latin mode */
		vdt->foreign_mode = FALSE;

		if (key_buf[3] & 0x0040)
			modifier_state = control;
		else if ((key_buf[4] & 0x0400) || (key_buf[5] & 0x0020))
			modifier_state = shift;
		else if ((key_buf[0] & 0x2000))
			modifier_state = upper_case;
		else
			modifier_state = lower_case;
	}


	/* test repeat key */
	repeat_mode = key_buf[2] & 0x0002;


	/* remove modifier keys */
	key_buf[0] &= ~0x2000;
	key_buf[2] &= ~0x0002;
	key_buf[3] &= ~0x0040;
	key_buf[4] &= ~0x0400;
	key_buf[5] &= ~0x0120;

	/* remove unused keys */
	if (! HAS_EXTRA_KEY_91(vdt))
		key_buf[5] &= ~0x0400;

	if (! HAS_EXTRA_KEY_67(vdt))
		key_buf[4] &= ~0x0004;


	if (! repeat_mode)
		/* reset REPEAT timer if the REPEAT key is not pressed */
		repeat_timer = 0;

	if (! (vdt->last_key_pressed & 0x80) && (key_buf[vdt->last_key_pressed >> 4] & (1 << (vdt->last_key_pressed & 0xf))))
	{
		/* last key has not been released */
		if (modifier_state == vdt->last_modifier_state)
		{
			/* handle REPEAT mode if applicable */
			if ((repeat_mode) && (++repeat_timer == repeat_delay))
			{
				if (vdt->keyboard_data_ready)
				{	/* keyboard buffer full */
					repeat_timer--;
				}
				else
				{	/* repeat current key */
					vdt->keyboard_data_ready = 1;
					repeat_timer = 0;
				}
			}
		}
		else
		{
			repeat_timer = 0;
			vdt->last_modifier_state = special_debounce;
		}
	}
	else
	{
		vdt->last_key_pressed = 0x80;

		if (vdt->keyboard_data_ready)
		{	/* keyboard buffer full */
			/* do nothing */
		}
		else
		{
			for (i=0; i<6; i++)
			{
				for (j=0; j<16; j++)
				{
					if (key_buf[i] & (1 << j))
					{
						vdt->last_key_pressed = (i << 4) | j;
						vdt->last_modifier_state = modifier_state;

						vdt->keyboard_data = (int)key_translate[vdt->model][modifier_state][vdt->last_key_pressed];
						vdt->keyboard_data_ready = 1;
						if (vdt->keyboard_interrupt_enable)
							(*vdt->int_callback)(device->machine(), 1);
						return;
					}
				}
			}
		}
	}
}
