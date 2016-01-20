// license:???
// copyright-holders:Paul Daniels, Colin Howell, R. Belmont
/***************************************************************************

  apple1.c

  Functions to emulate the video hardware of the Apple I.

  The Apple I video hardware was basically a dumb video terminal; in
  fact it was based on Steve Wozniak's own design for a simple video
  terminal.  It had 40 columns by 24 lines of uppercase-only text.
  Text could only be output at 60 characters per second, one character
  per video frame.  The cursor (a blinking @) could only be advanced
  using spaces or carriage returns.  Carriage returns were the only
  control characters recognized.  Previously written text could not be
  altered, only scrolled off the top of the screen.

  The video memory used seven 1k-bit dynamic shift registers.  Six of
  these held the 6-bit visible character codes, and one stored the
  cursor location as a simple bitmap--the bit for the cursor position
  was set to 0, and all the other bits were 1.

  These shift registers were continuously recirculated, completing one
  cycle per video frame.  As a new line of characters was about to be
  scanned by the video beam, that character line would be recirculated
  into the shift registers and would simultaneously be stored into a
  6x40-bit line buffer (also a shift register).  At this point, if the
  cursor location was in this line, a new character could be written
  into that location in the shift registers and the cursor could be
  advanced.  (Carriage returns were not written into the shift
  registers; they only advanced the cursor.)

  The characters in the line buffer were recirculated 7 times to
  display the 8 scan lines of the characters, before being replaced by
  a new line of characters from the main shift registers.

  Cursor blinking was performed by a Signetics 555 timer IC whose
  output was gated into the character code signals as they passed into
  the line buffer.

  Character images were provided by a Signetics 2513 character
  generator ROM, a chip also used in computer terminals such as the
  ADM-3A.  This ROM had 9 address lines and 5 data lines; it contained
  64 character images, each 5 pixels wide by 8 pixels high, with one
  line of pixels being blank for vertical separation.  The video
  circuitry added the 2 pixels of horizontal separation for each
  character.

  A special CLEAR SCREEN switch on the keyboard, directly connected to
  the video hardware, could be used to clear the video memory and
  return the cursor to the home position.  This was completely
  independent of the processor.

  A schematic of the Apple I video hardware can be found in the
  Apple-1 Operation Manual; look for the schematic titled "Terminal
  Section".  Most of the functionality modeled here was determined by
  reading this schematic.  Many of the chips used were standard 74xx
  TTL chips, but the shift registers used for the video memory and
  line buffer were Signetics 25xx PMOS ICs.  These were already
  becoming obsolete when the Apple I was built, and detailed
  information on them is very hard to find today.

***************************************************************************/

#include "emu.h"
#include "includes/apple1.h"


/***************************************************************************

    Terminal code

***************************************************************************/

TILE_GET_INFO_MEMBER(apple1_state::terminal_gettileinfo)
{
	int ch, gfxfont, code, color;

	ch = m_current_terminal->mem[tile_index];
	code = ch & ((1 << m_current_terminal->char_bits) - 1);
	color = ch >> m_current_terminal->char_bits;
	gfxfont = m_current_terminal->gfx;

	if ((tile_index == m_current_terminal->cur_offset) && !m_current_terminal->cur_hidden && m_current_terminal->getcursorcode)
		code = m_current_terminal->getcursorcode(code);

	SET_TILE_INFO_MEMBER(gfxfont,    /* gfx */
		code,       /* character */
		color,      /* color */
		0);         /* flags */
}

void apple1_state::terminal_draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, terminal_t *terminal)
{
	m_current_terminal = terminal;
	terminal->tm->draw(screen, dest, cliprect, 0, 0);
	m_current_terminal = nullptr;
}

void apple1_state::verify_coords(terminal_t *terminal, int x, int y)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < terminal->num_cols);
	assert(y < terminal->num_rows);
}

void apple1_state::terminal_putchar(terminal_t *terminal, int x, int y, int ch)
{
	int offs;

	verify_coords(terminal, x, y);

	offs = y * terminal->num_cols + x;
	if (terminal->mem[offs] != ch)
	{
		terminal->mem[offs] = ch;
		terminal->tm->mark_tile_dirty(offs);
	}
}

int apple1_state::terminal_getchar(terminal_t *terminal, int x, int y)
{
	int offs;

	verify_coords(terminal, x, y);
	offs = y * terminal->num_cols + x;
	return terminal->mem[offs];
}

void apple1_state::terminal_putblank(terminal_t *terminal, int x, int y)
{
	terminal_putchar(terminal, x, y, terminal->blank_char);
}

void apple1_state::terminal_dirtycursor(terminal_t *terminal)
{
	if (terminal->cur_offset >= 0)
		terminal->tm->mark_tile_dirty(terminal->cur_offset);
}

void apple1_state::terminal_setcursor(terminal_t *terminal, int x, int y)
{
	terminal_dirtycursor(terminal);
	terminal->cur_offset = y * terminal->num_cols + x;
	terminal_dirtycursor(terminal);
}

void apple1_state::terminal_hidecursor(terminal_t *terminal)
{
	terminal->cur_hidden = 1;
	terminal_dirtycursor(terminal);
}

void apple1_state::terminal_showcursor(terminal_t *terminal)
{
	terminal->cur_hidden = 0;
	terminal_dirtycursor(terminal);
}

void apple1_state::terminal_getcursor(terminal_t *terminal, int *x, int *y)
{
	*x = terminal->cur_offset % terminal->num_cols;
	*y = terminal->cur_offset / terminal->num_cols;
}

void apple1_state::terminal_fill(terminal_t *terminal, int val)
{
	int i;
	for (i = 0; i < terminal->num_cols * terminal->num_rows; i++)
		terminal->mem[i] = val;
	terminal->tm->mark_all_dirty();
}

void apple1_state::terminal_clear(terminal_t *terminal)
{
	terminal_fill(terminal, terminal->blank_char);
}

terminal_t *apple1_state::terminal_create(
	int gfx, int blank_char, int char_bits,
	int (*getcursorcode)(int original_code),
	int num_cols, int num_rows)
{
	terminal_t *term;
	int char_width, char_height;

	char_width = m_gfxdecode->gfx(gfx)->width();
	char_height = m_gfxdecode->gfx(gfx)->height();

	term = (terminal_t *) auto_alloc_array(machine(), char, sizeof(terminal_t) - sizeof(term->mem)
		+ (num_cols * num_rows * sizeof(termchar_t)));

	term->tm = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(apple1_state::terminal_gettileinfo),this), TILEMAP_SCAN_ROWS,
		char_width, char_height, num_cols, num_rows);

	term->gfx = gfx;
	term->blank_char = blank_char;
	term->char_bits = char_bits;
	term->num_cols = num_cols;
	term->num_rows = num_rows;
	term->getcursorcode = getcursorcode;
	term->cur_offset = -1;
	term->cur_hidden = 0;
	terminal_clear(term);
	return term;
}


/**************************************************************************/



/* The cursor blinking is generated by a free-running timer with a
   0.52-second period.  It is on for 2/3 of this period and off for
   1/3. */
#define CURSOR_OFF_LENGTH           (0.52/3)

/**************************************************************************/

static int apple1_getcursorcode(int original_code)
{
	/* Cursor uses symbol 0 (an @ sign) in the character generator ROM. */
	return 0;
}

/**************************************************************************/

void apple1_state::video_start()
{
	m_blink_on = 1;     /* cursor is visible initially */
	m_terminal = terminal_create(
		0,          /* graphics font 0 (the only one we have) */
		32,         /* Blank character is symbol 32 in the ROM */
		8,          /* use 8 bits for the character code */
		apple1_getcursorcode,
		40, 24);    /* 40 columns, 24 rows */

	terminal_setcursor(m_terminal, 0, 0);
}

/* This function handles all writes to the video display. */
void apple1_state::apple1_vh_dsp_w (int data)
{
	int x, y;
	int cursor_x, cursor_y;

	/* While CLEAR SCREEN is being held down, the hardware is forced
	   to clear the video memory, so video writes have no effect. */
	if (m_vh_clrscrn_pressed)
		return;

	/* The video display port only accepts the 7 lowest bits of the char. */
	data &= 0x7f;

	terminal_getcursor(m_terminal, &cursor_x, &cursor_y);

	if (data == '\r') {
		/* Carriage-return moves the cursor to the start of the next
		   line. */
		cursor_x = 0;
		cursor_y++;
	}
	else if (data < ' ') {
		/* Except for carriage-return, the video hardware completely
		   ignores all control characters. */
		return;
	}
	else {
		/* For visible characters, only 6 bits of the ASCII code are
		   used, because the 2513 character generator ROM only
		   contains 64 symbols.  The low 5 bits of the ASCII code are
		   used directly.  Bit 6 is ignored, since it is the same for
		   all the available characters in the ROM.  Bit 7 is inverted
		   before being used as the high bit of the 6-bit ROM symbol
		   index, because the block of 32 ASCII symbols containing the
		   uppercase letters comes first in the ROM. */

		int romindx = (data & 0x1f) | (((data ^ 0x40) & 0x40) >> 1);

		terminal_putchar(m_terminal, cursor_x, cursor_y, romindx);
		if (cursor_x < 39)
		{
			cursor_x++;
		}
		else
		{
			cursor_x = 0;
			cursor_y++;
		}
	}

	/* If the cursor went past the bottom line, scroll the text up one line. */
	if (cursor_y == 24)
	{
		for (y = 1; y < 24; y++)
			for (x = 0; x < 40; x++)
				terminal_putchar(m_terminal, x, y-1,
									terminal_getchar(m_terminal, x, y));

		for (x = 0; x < 40; x++)
			terminal_putblank(m_terminal, x, 23);

		cursor_y--;
	}

	terminal_setcursor(m_terminal, cursor_x, cursor_y);
}

/* This function handles clearing the video display on cold-boot or in
   response to a press of the CLEAR SCREEN switch. */
void apple1_state::apple1_vh_dsp_clr ()
{
	terminal_setcursor(m_terminal, 0, 0);
	terminal_clear(m_terminal);
}

/* Calculate how long it will take for the display to assert the RDA
   signal in response to a video display write.  This signal indicates
   the display has completed the write and is ready to accept another
   write. */
attotime  apple1_state::apple1_vh_dsp_time_to_ready ()
{
	int cursor_x, cursor_y;
	int cursor_scanline;
	double scanline_period = m_screen->scan_period().as_double();
	double cursor_hfrac;

	/* The video hardware refreshes the screen by reading the
	   character codes from its circulating shift-register memory.
	   Because of the way this memory works, a new character can only
	   be written into the cursor location at the moment this location
	   is about to be read.  This happens during the first scanline of
	   the cursor's character line, when the beam reaches the cursor's
	   horizontal position. */

	terminal_getcursor(m_terminal, &cursor_x, &cursor_y);
	cursor_scanline = cursor_y * apple1_charlayout.height;

	/* Each scanline is composed of 455 pixel times.  The first 175 of
	   these are the horizontal blanking period; the remaining 280 are
	   for the visible part of the scanline. */
	cursor_hfrac = (175 + cursor_x * apple1_charlayout.width) / 455;

	if (m_screen->vpos() == cursor_scanline) {
		/* video_screen_get_hpos() doesn't account for the horizontal
		   blanking interval; it acts as if the scanline period is
		   entirely composed of visible pixel times.  However, we can
		   still use it to find what fraction of the current scanline
		   period has elapsed. */
		double current_hfrac = m_screen->hpos() /
								m_screen->width();
		if (current_hfrac < cursor_hfrac)
			return attotime::from_double(scanline_period * (cursor_hfrac - current_hfrac));
	}

	return attotime::from_double(
		m_screen->time_until_pos(cursor_scanline, 0).as_double() +
		scanline_period * cursor_hfrac);
}

/* Blink the cursor on or off, as appropriate. */
void apple1_state::apple1_vh_cursor_blink ()
{
	int new_blink_on;

	/* The cursor is on for 2/3 of its blink period and off for 1/3.
	   This is most easily handled by dividing the total elapsed time
	   by the length of the off-portion of the cycle, giving us the
	   number of one-third-cycles elapsed, then checking the result
	   modulo 3. */

	if (((int) (machine().time().as_double() / CURSOR_OFF_LENGTH)) % 3 < 2)
		new_blink_on = 1;
	else
		new_blink_on = 0;

	if (new_blink_on != m_blink_on) {        /* have we changed state? */
		if (new_blink_on)
			terminal_showcursor(m_terminal);
		else
			terminal_hidecursor(m_terminal);
		m_blink_on = new_blink_on;
	}
}

UINT32 apple1_state::screen_update_apple1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	apple1_vh_cursor_blink();
	terminal_draw(screen, bitmap, cliprect, m_terminal);
	return 0;
}
