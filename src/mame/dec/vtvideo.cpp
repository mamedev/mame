// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Karl-Ludwig Deisenhofer
/**********************************************************************
DEC VT Terminal video emulation
[ DC012 and DC011 emulation ]

01/05/2009 Initial implementation [Miodrag Milanovic]
Enhancements (2013 - 2015) by Karl-Ludwig Deisenhofer.

DEC VIDEO : STATE AS OF JULY 2014
---------------------------------
Code developed with Rainbow-100 hardware in mind (a PC-like machine with a video circuit similar to a VT100 equipped w. AVO)
Details (termination characters, option hardware, MHFU watchdog) differ when compared to VT.

As of July 2014, the Rainbow video section is more mature than the 'VT100_VIDEO' part (essentially unchanged since 2009).
List of features not yet ported to VT: line doubler; 48 line mode; soft scroll; intensity / palette, AVO (ext.attributes)

FIXME: work out the differences and identify common code between VT and Rainbow. Unify different code trees.

- REQUIRED TODOS / TESTS :
  * do line and character attributes (plus combinations) match real hardware?
  * how does the AVO fit in?

- SCROLLING REGIONS / SPLIT SCREEN SCROLLING UNTESTED (if you open > 1 file with the VAX editor EDT)
  See VT100 Technical Manual: 4.7.4 Address Shuffling to 4.7.9 Split Screen Smooth Scrolling.
  More on scrolling regions: Rainbow 100 B technical documentation (QV069-GZ) April 1985 page 22

- NEW - INTERLACED MODE (Rainbow only):
  Vertical resolution increases from 240 to 480, while the refresh rate halves (flickers on CRTs).
  To accomplish this, the display controller repeats even lines in odd scans.
  VTVIDEO activates line doubling in 24 line, interlaced mode only.

  Although the DC12 has the ability to display 48 lines, most units are low on screen RAM and
  won't even show 80 x 48. -> REASON: (83 x 48 = 3984 Byte) > (screen RAM) minus 'scratch area'
  On a VT-180, BIOS scratch requires up to 700 bytes used for SETUP, flags, SILO, keyboard.

- POSSIBLE IMPROVEMENTS:

* exact colors for different VR201 monitors (for white, green and amber)

* ACCURATE VIDEO DELAYS:
  Position of the first visible scanline (relative to the vertical reset) depends on
  content of fill bytes at the beginning of screen RAM.

  Six invisible, linked lines are initially provided (at location $EE000+ on a Rainbow).
  Real-world DC hardware parses the (circular) chain until interrupted by blanking.
  BIOS assumes 2 lines are omitted @ 60 and 5 lines are at 50 Hertz (-> longer blank).

  VTVIDEO keeps it simple and skips up to 6 lines considered 'illegal' (offset < $12).
  Works even in cases where undocumented delays or lines are poked (SQUINT; VIDEO.PAS).

  Accurate timings for 4 modes (from VT-180 manual 6-30 on):
  Vertical frequency = (2 * HF = 31.468 Khz) / DIVIDER
  - 50 NI: (2 * HF = 31.468 Khz) / 630
  - 50 interlaced: (2 * HF = 31.468 Khz) / 629
  - 60 NI: (2 * HF = 31.468 Khz) / 524
  - 60 interlaced: (2 * HF = 31.468 Khz) / 525
**********************************************************************/

#include "emu.h"
#include "video/vtvideo.h"
#include "screen.h"

/***************************************************************************
PARAMETERS
***************************************************************************/

#define VERBOSE         1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT100_VIDEO, vt100_video_device, "vt100_video", "VT100 Video")
DEFINE_DEVICE_TYPE(RAINBOW_VIDEO, rainbow_video_device, "rainbow_video", "Rainbow Video")


vt100_video_device::vt100_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_read_ram(*this)
	, m_write_vert_freq_intr(*this)
	, m_write_lba3_lba4(*this)
	, m_write_lba7(*this)
	, m_char_rom(*this, finder_base::DUMMY_TAG)
	, m_palette(*this, "palette")
{
}


vt100_video_device::vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vt100_video_device(mconfig, VT100_VIDEO, tag, owner, clock)
{
}


rainbow_video_device::rainbow_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vt100_video_device(mconfig, RAINBOW_VIDEO, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vt100_video_device::device_start()
{
	/* resolve callbacks */
	m_read_ram.resolve_safe(0);
	m_write_vert_freq_intr.resolve_safe();
	m_write_lba3_lba4.resolve();
	m_write_lba7.resolve_safe();

	// LBA7 is scan line frequency update
	m_lba7_change_timer = timer_alloc(FUNC(vt100_video_device::lba7_change), this);
	m_lba7_change_timer->adjust(clocks_to_attotime(765), 0, clocks_to_attotime(765));

	// LBA3 and LBA4 are first two stages of divide-by-17 counter
	m_lba3_change_timer = timer_alloc(FUNC(vt100_video_device::lba3_change), this);

	screen().register_vblank_callback(vblank_state_delegate(&vt100_video_device::vblank_callback, this));

	save_item(NAME(m_lba7));
	save_item(NAME(m_scroll_latch));
	save_item(NAME(m_blink_flip_flop));
	save_item(NAME(m_reverse_field));
	save_item(NAME(m_basic_attribute));
	save_item(NAME(m_columns));
	save_item(NAME(m_height));
	save_item(NAME(m_height_MAX));
	save_item(NAME(m_fill_lines));
	save_item(NAME(m_is_50hz));
	save_item(NAME(m_interlaced));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vt100_video_device::device_reset()
{
	m_palette->set_pen_color(0, 0x00, 0x00, 0x00); // black
	m_palette->set_pen_color(1, 0xff - 100, 0xff - 100, 0xff - 100);  // WHITE (dim)
	m_palette->set_pen_color(2, 0xff - 50, 0xff - 50, 0xff - 50);     // WHITE NORMAL
	m_palette->set_pen_color(3, 0xff, 0xff, 0xff);              // WHITE (brighter)

	m_height = 25;
	m_height_MAX = 25;

	m_lba7 = 0;

	m_scroll_latch = 0;
	m_scroll_latch_valid = false;
	m_last_scroll = 0;

	m_blink_flip_flop = 0;
	m_reverse_field = 0;
	m_basic_attribute = 0;

	m_columns = 80;
	m_is_50hz = false;

	m_interlaced = true;
	m_fill_lines = 2; // for 60Hz
	recompute_parameters();
}

// ****** RAINBOW ******
// 4 color (= monochrome intensities) palette, 24 and 48 line modes.
void rainbow_video_device::device_reset()
{
	MHFU_FLAG = false;
	MHFU_counter = 0; // **** MHFU: OFF ON COLD BOOT ! ****

	// (rest of the palette is set in the main program)
	m_palette->set_pen_color(0, 0x00, 0x00, 0x00); // black

	m_height = 24;  // <---- DEC-100
	m_height_MAX = 48;

	m_lba7 = 0;

	m_scroll_latch = 0;
	m_scroll_latch_valid = false;
	m_last_scroll = 0;

	m_blink_flip_flop = 0;
	m_reverse_field = 0;
	m_basic_attribute = 0;

	m_columns = 80;

	m_is_50hz = false;

	m_interlaced = true;
	m_fill_lines = 2; // for 60Hz (not in use any longer -> detected)
	recompute_parameters();
}

/***************************************************************************
IMPLEMENTATION
***************************************************************************/

// Also used by Rainbow-100 ************
void vt100_video_device::recompute_parameters()
{
	// RAINBOW: 240 scan lines in non-interlaced mode (480 interlaced). VT-100 : same (?)
	m_linedoubler = false; // 24 "true" lines (240)  -OR-  48 lines with NO ghost lines (true 480)
	if ((m_interlaced) && (m_height == 24 || m_height == 25))
		m_linedoubler = true; // 24 lines with 'double scan' (false 480)

	int vert_pix_visible = m_height * (m_linedoubler ? 20 : 10);
	int vert_pix_total = m_is_50hz ? (m_interlaced ? 629 : 630/2) : (m_interlaced ? 525 : 524/2);
	attotime frame_period = clocks_to_attotime(vert_pix_total * 1530);

	// display 1 less filler pixel in 132 char. mode
	int horiz_pix_visible = m_columns * (m_columns == 132 ? 9 : 10);
	int horiz_pix_total = m_columns == 132 ? 1530 : 1020;

	// dot clock is divided by 1.5 in 80 column mode
	screen().set_unscaled_clock(m_columns == 132 ? clock() : clock() * 2 / 3);
	rectangle visarea(0, horiz_pix_visible - 1, 0, vert_pix_visible - 1);
	screen().configure(horiz_pix_total, vert_pix_total, visarea, frame_period.as_attoseconds());

	LOG("(RECOMPUTE) HPIX: %d (%d) - VPIX: %d (%d)\n", horiz_pix_visible, horiz_pix_total, vert_pix_visible, vert_pix_total);
	LOG("(RECOMPUTE) FREQUENCY: %f\n", frame_period.as_hz());
	if (m_interlaced)
		LOG("(RECOMPUTE) * INTERLACED *\n");
	if (m_linedoubler)
		LOG("(RECOMPUTE) * LINEDOUBLER *\n");
}

READ_LINE_MEMBER(vt100_video_device::lba7_r)
{
	return m_lba7;
}

void vt100_video_device::vblank_callback(screen_device &screen, bool state)
{
	if (state)
	{
		m_write_vert_freq_intr(ASSERT_LINE);
		notify_vblank(true);
	}
}

// Also used by Rainbow-100 ************
void vt100_video_device::dc012_w(offs_t offset, uint8_t data)
{
	// Writes to [10C] and [0C] are treated differently
	// - see 3.1.3.9.5 DC012 Programming Information (PC-100 spec)
	if ((offset & 0x100) ) // MHFU is disabled by writing a value to port 010C.
	{
//      if (MHFU_FLAG == true)
//                      LOG("MHFU  *** DISABLED *** \n");
		MHFU_FLAG = false;
	}
	else
	{
//      if (MHFU_FLAG == false)
//          LOG("MHFU  ___ENABLED___ %s \n", m_maincpu->pc());
		MHFU_FLAG = true;
		MHFU_counter = 0;
	}

	if (!(data & 0x08))
	{
		if (!(data & 0x04))
		{
			m_scroll_latch_valid = false;
			m_scroll_latch = data & 0x03; // LSB is written first.
		}
		else // set MSB of scroll_latch
		{
			m_scroll_latch = (m_scroll_latch & 0x03) | ((data & 0x03) << 2);
			m_scroll_latch_valid = true;
		}
	}
	else
	{
		switch (data & 0x0f)
		{
		case 0x08:
			// toggle blink flip flop
			m_blink_flip_flop = !(m_blink_flip_flop) ? 1 : 0;
			break;
		case 0x09:
			// clear vertical frequency interrupt;
			m_write_vert_freq_intr(CLEAR_LINE);
			notify_vblank(false);
			break;
		case 0x0a:
			// PDF: reverse field ON
			m_reverse_field = 0;
			break;
		case 0x0b:
			// PDF: reverse field OFF
			// SETUP: dark screen selected
			m_reverse_field = 1;
			break;

			//  Writing a 11XX bit combination clears the blink-flip flop (valid for 0x0C - 0x0F):
		case 0x0c:
			// set basic attribute to underline / blink flip-flop off
			m_blink_flip_flop = 0;
			m_basic_attribute = 0; // (VT-100 without AVO): reverse video is interpreted as underline (basic_attribute 0)
			break;

		case 0x0d:
			// (DEC Rainbow 100 DEFAULT) : reverse video with 24 lines / blink flip-flop off
			m_blink_flip_flop = 0;
			m_basic_attribute = 1; // (VT-100 without AVO): reverse video is interpreted as reverse (basic_attribute 1)

			if (m_height_MAX == 25) break; //  Abort on VT-100 for now.

			m_height = 24;  // (DEC Rainbow 100) : 24 line display
			recompute_parameters();
			break;

		case 0x0e:
			m_blink_flip_flop = 0;  // 'unsupported' DC012 command. Turns blink flip-flop off (11XX).
			break;

		case 0x0f:
			// (DEC Rainbow 100): reverse video with 48 lines / blink flip-flop off
			m_blink_flip_flop = 0;
			m_basic_attribute = 1;

			// 0x0f = 'reserved' on VT 100
			//  Abort on VT-100 for now.
			if (m_height_MAX == 25) break;

			m_height = 48;   // (DEC Rainbow 100) : 48 line display
			recompute_parameters();
			break;
		}
	}
}

// Writing to DC011 resets internal counters (& disturbs display) on real hardware.
void vt100_video_device::dc011_w(uint8_t data)
{
	if (!BIT(data, 5))
	{
		m_interlaced = true;

		if (!BIT(data, 4))
			m_columns = 80;
		else
			m_columns = 132;
	}
	else
	{
		m_interlaced = false;
		m_is_50hz = BIT(data, 4);
		m_fill_lines = m_is_50hz ? 5 : 2;
	}

	recompute_parameters();
}

void vt100_video_device::brightness_w(uint8_t data)
{
	//m_palette->set_pen_color(1, data, data, data);
}



void vt100_video_device::video_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t addr = 0;
	int line = 0;
	int xpos = 0;
	int ypos = 0;
	int x = 0;
	uint8_t scroll_region = 1; // binary 1
	uint8_t display_type = 3;  // binary 11
	uint16_t temp = 0;

	if (m_read_ram(0) != 0x7f)
		return;

	int fill_lines = m_fill_lines;
	int vert_charlines_MAX = m_height + fill_lines;
	if (m_linedoubler)
	{
		vert_charlines_MAX *= 2;
		fill_lines *= 2;
	}
	while (line < vert_charlines_MAX)
	{
		uint8_t code = m_read_ram(addr + xpos);
		if (code == 0x7f)
		{
			// end of line, fill empty till end of line
			if (line >= fill_lines)
			{
				for (x = xpos; x < ((display_type != 3) ? (m_columns / 2) : m_columns); x++)
				{
					display_char(bitmap, code, x, ypos, scroll_region, display_type, false, false, false, false, false);
				}
			}
			// move to new data
			temp = m_read_ram(addr + xpos + 1) * 256 + m_read_ram(addr + xpos + 2);
			addr = (temp)& 0x1fff;
			// if A12 is 1 then it is 0x2000 block, if 0 then 0x4000 (AVO - not actually implemented?)
			/*if (addr & 0x1000)*/ addr &= 0xfff; /*else addr |= 0x2000;*/
			scroll_region = (temp >> 15) & 1;
			display_type = bitswap<2>((temp >> 13) & 3, 0, 1);
			if (line >= fill_lines)
				ypos++;
			xpos = 0;
			line++;
			if (m_linedoubler)
				line++;
		}
		else
		{
			// display regular char
			if (line >= fill_lines)
			{
				uint8_t attr = m_read_ram(0x1000 + addr + xpos);
				display_char(bitmap, code & 0x7f, xpos, ypos, scroll_region, display_type, BIT(code, 7), !BIT(attr, 2), !BIT(attr, 0), !BIT(attr, 1), false);
			}
			xpos++;
			if (xpos > m_columns)
			{
				line++;
				xpos = 0;
			}
		}
	}
}

// ****** RAINBOW ******
// Display 10 scan lines (or 20 in interlaced mode) for one character @ position X/Y
// NOTE: X or Y indicate a character position!

// 5 possible CHARACTER STATES (normal, reverse, bold, blink, underline) are encoded into display_type.
// From the VT-180 specs, chapter 6-43 (where multiple attributes are described):
//  1) reverse characters [ XOR of reverse video and reverse screen (A) ] normally have dim backgrounds with black characters (B)
//  2) bold and reverse together give a background of normal intensity

//  3) blink controls intensity: normal chars vary between A) normal and dim  (B) bold chars vary between bright and normal
//  4) blink applied to a
//       A) reverse character causes it to alternate between normal and reverse video representation
//       B) non-rev. "        : alternate between usual intensity and the next lower intensity
//  5) underline causes the 9.th scan to be forced to
//       A) white of the same intensity as the characters (for nonreversed characters),
//       b) to black (for reverse characters)
// LINE ATTRIBUTE 'double_height' always is interpreted as 'double width + double height'

// ATTRIBUTES:   No attributes = 0x0E
// 1 = display char. in REVERSE   (encoded as 1 in attribute) HIGH ACTIVE
// 0 = display char. in BOLD      (encoded as 2 in attribute) LOW ACTIVE
// 0 = display char. w. BLINK     (encoded as 4 in attribute) LOW ACTIVE
// 0 = display char. w. UNDERLINE (encoded as 8 in attribute) LOW ACTIVE
void vt100_video_device::display_char(bitmap_ind16 &bitmap, uint8_t code, int x, int y, uint8_t scroll_region, uint8_t display_type, bool invert, bool bold, bool blink, bool underline, bool blank)
{
	uint16_t x_preset = x << 3; // x_preset = x * 9 (= 132 column mode)
	x_preset += x;

	if (m_columns == 80)
		x_preset += x; //        x_preset = x * 10 (80 column mode)

	uint16_t y_preset;

	uint16_t CHARPOS_y_preset = y << 3; // CHARPOS_y_preset = y * 10;
	CHARPOS_y_preset += y;
	CHARPOS_y_preset += y;

	uint16_t DOUBLE_x_preset = x_preset << 1; // 18 for 132 column mode, else 20 (x_preset * 2)

	uint8_t line = 0;
	int bit = 0, j = 0;
	int fg_intensity;
	int back_intensity, back_default_intensity;

	display_type = display_type & 3;

	// * SCREEN ATTRIBUTES (see VT-180 manual 6-30) *
	// 'reverse field' = reverse video over entire screen

	// For reference: a complete truth table can be taken from TABLE 4-6-4 / VT100 technical manual.
	// Following IF statements implement it in full. Code segments should not be shuffled.
	invert = invert ^ m_reverse_field ^ m_basic_attribute;

	fg_intensity = bold + 2;   // FOREGROUND (FG):  normal (2) or bright (3)

	back_intensity = 0; // DO NOT SHUFFLE CODE AROUND !!
	if ((blink != 0) && (m_blink_flip_flop != 0))
		fg_intensity -= 1; // normal => dim    bright => normal (when bold)

	// INVERSION: background gets foreground intensity (reduced by 1).
	// _RELIES ON_ on_ previous evaluation of the BLINK signal (fg_intensity).
	if (invert != 0)
	{
		back_intensity = fg_intensity - 1; // BG: normal => dim;  dim => OFF;   bright => normal

		if (back_intensity != 0)           //  FG: avoid 'black on black'
			fg_intensity = 0;
		else
			fg_intensity = fg_intensity + 1; // FG: dim => normal; normal => bright
	}

	// BG: DEFAULT for entire character (underline overrides this for 1 line) -
	back_default_intensity = back_intensity;

	bool double_width = (display_type != 3) ? true : false; // all except normal: double width
	bool double_height = (display_type & 1) ? false : true;  // 0,2 = double height

	int smooth_offset = 0;
	if (scroll_region != 0)
		smooth_offset = m_last_scroll; // valid after VBI

	int i = 0;
	int extra_scan_line = 0;
	for (int scan_line = 0; scan_line < (m_linedoubler ? 20 : 10); scan_line++)
	{
		y_preset = CHARPOS_y_preset + scan_line;

		// 'i' points to char-rom (plus scroll offset; if active) -
		// IF INTERLACED: odd lines = even lines
		i = (m_linedoubler ? (scan_line >> 1) : scan_line) + smooth_offset;

		if (i > 9) // handle everything in one loop (in case of smooth scroll):
		{
			extra_scan_line += 1;

			// Fetch appropriate character bitmap (one scan line) -
			// IF INTERLACED: no odd lines
			i = smooth_offset - (m_linedoubler ? (extra_scan_line >> 1) : extra_scan_line);

			if (CHARPOS_y_preset >= extra_scan_line) // If result not negative...
				y_preset = CHARPOS_y_preset - extra_scan_line; // correct Y pos.
			else
			{
				y_preset = (m_linedoubler ? 480 : 240) - extra_scan_line;
				i = 0; // blank line. Might not work with TCS or other charsets (FIXME)
			}
		}

		switch (display_type)
		{
		case 0:  // bottom half of 'double height, double width' char.
			j = (i >> 1) + 5;
			break;

		case 2:  // top half of 'double height, double width' char.
			j = (i >> 1);
			break;

		default: // 1: double width
			// 3: normal
			j = i;
			break;
		}

		// modify line since that is how it is stored in rom
		if (j == 0) j = 15; else j = j - 1;

		line = m_char_rom[(code << 4) + j]; // code * 16

		// UNDERLINED CHARACTERS (CASE 5 - different in 1 line):
		back_intensity = back_default_intensity; // 0, 1, 2
		if (underline != 0)
		{
			if (i == 8)
			{
				if (invert == 0)
					line = 0xff; // CASE 5 A)
				else
				{
					line = 0x00; // CASE 5 B)
					back_intensity = 0; // OVERRIDE: BLACK BACKGROUND
				}
			}
		}

		for (int b = 0; b < 8; b++) // 0..7
		{
			if (blank)
			{
				bit = m_reverse_field ^ m_basic_attribute;
			}
			else
			{
				bit = BIT((line << b), 7);

				if (bit > 0)
					bit = fg_intensity;
				else
					bit = back_intensity;
			}

			// Double, 'double_height + double_width', then normal.
			if (double_width)
			{
				bitmap.pix(y_preset, DOUBLE_x_preset + (b << 1) + 1) = bit;
				bitmap.pix(y_preset, DOUBLE_x_preset + (b << 1)) = bit;

				if (double_height)
				{
					bitmap.pix(1 + y_preset, DOUBLE_x_preset + (b << 1) + 1) = bit;
					bitmap.pix(1 + y_preset, DOUBLE_x_preset + (b << 1)) = bit;
				}
			}
			else
			{
				bitmap.pix(y_preset, x_preset + b) = bit;
			}
		} // for (8 bit)

		// char interleave (X) is filled with last bit
		if (double_width)
		{
			// double chars: 18 or 20 bits
			bitmap.pix(y_preset, DOUBLE_x_preset + 16) = bit;
			bitmap.pix(y_preset, DOUBLE_x_preset + 17) = bit;

			if (m_columns == 80)
			{
				bitmap.pix(y_preset, DOUBLE_x_preset + 18) = bit;
				bitmap.pix(y_preset, DOUBLE_x_preset + 19) = bit;
			}
		}
		else
		{   // normal chars: 9 or 10 bits
			bitmap.pix(y_preset, x_preset + 8) = bit;

			if (m_columns == 80)
				bitmap.pix(y_preset, x_preset + 9) = bit;
		}

		/* The DEC terminals use a single ROM bitmap font and
		 * dot-stretching to synthesize multiple variants that are not
		 * just nearest neighbor resampled. The result is the same
		 * as one would get by fake-bolding; the already doubled raster image;
		 * by rendering twice with 1px horizontal offset.
		 *
		 * For details see: https://vt100.net/dec/vt220/glyphs
		 */
		int prev_bit = back_intensity;
		int bits_width = 21;
		if (!double_width)
		{
			if (m_columns == 80)
				bits_width = 11;
			else
				bits_width = 10;
		}
		for (int b = 0; b < bits_width; b++)
		{
			if (double_width)
			{
				if (bitmap.pix(y_preset, DOUBLE_x_preset + b) == fg_intensity)
				{
					prev_bit = fg_intensity;
				}
				else
				{
					if (prev_bit == fg_intensity)
						bitmap.pix(y_preset, DOUBLE_x_preset + b) = fg_intensity;
					prev_bit = back_intensity;
				}
			}
			else
			{
				if (bitmap.pix(y_preset, x_preset + b) == fg_intensity)
				{
					prev_bit = fg_intensity;
				}
				else
				{
					if (prev_bit == fg_intensity)
						bitmap.pix(y_preset, x_preset + b) = fg_intensity;
					prev_bit = back_intensity;
				}
			}
		}
	} // for (scan_line)

}

// ****** RAINBOW ******
void rainbow_video_device::video_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t addr = 0;
	uint16_t attr_addr = 0;
	int line = 0;
	int xpos = 0;
	int ypos = 0;
	uint8_t code;
	int x = 0;
	uint8_t scroll_region = 0;  // DEFAULT TO 0 = NOT PART OF scroll_region
	uint8_t display_type = 3;  // NORMAL DISPLAY - binary 11
	uint16_t temp = 0;

	if (m_read_ram(0) != 0xff) // video uninitialized?
		return;

	// Skip fill (0xFF) lines and put result in ADDR.
	for (int xp = 1; xp <= 6; xp += 1) // beware of circular references
	{
		// Fetch LINE ATTRIBUTE before it is gone
		attr_addr = 0x1000 | ((addr + 1) & 0x0fff);

		temp = m_read_ram(addr + 2) * 256 + m_read_ram(addr + 1);
		addr = (temp)& 0x0fff;

		temp = m_read_ram(attr_addr);
		scroll_region = (temp)& 1;
		display_type = (temp >> 1) & 3;

		if (addr >= 0x12)
			break;
	}

	int vert_charlines_MAX = m_height;
	if (m_linedoubler)
		vert_charlines_MAX *= 2;
	while (line < vert_charlines_MAX)
	{
		code = m_read_ram(addr + xpos);

		bool force_blank;
		if (code == 0x00)        // TODO: investigate side effect on regular zero character!
			force_blank = true; // DEFAULT: filler chars (till end of line) and empty lines (00) will be blanked
		else
			force_blank = false; // else activate display.

		if (code == 0xff) // end of line, fill empty till end of line
		{
			// All except 3 is DOUBLE WIDTH 40 or 66 chars per line
			for (x = xpos; x < ((display_type != 3) ? (m_columns / 2) : m_columns); x++)
			{
					display_char(bitmap, code, x, ypos, scroll_region, display_type, false, false, false, false, true);
			}

			//  LINE ATTRIBUTE - valid for all chars on next line  ** DO NOT SHUFFLE **
			attr_addr = 0x1000 | ((addr + xpos + 1) & 0x0fff);

			// MOVE TO NEW DATA
			temp = m_read_ram(addr + xpos + 2) * 256 + m_read_ram(addr + xpos + 1);
			addr = (temp)& 0x0fff;

			temp = m_read_ram(attr_addr);
			scroll_region = (temp)& 1;
			display_type = (temp >> 1) & 3;

			ypos++;            // Y + 1 in non-interlaced mode
			if (m_linedoubler)
				ypos++;        // Y + 2 in 'double scan' mode (see -> 'display_char')

			if (ypos > vert_charlines_MAX) // prevent invalid Y pos.
				break;

			xpos = 0;
			line++;
		}
		else
		{
			attr_addr = 0x1000 | ((addr + xpos) & 0x0fff);
			temp = (m_read_ram(attr_addr) & 15); // get character attributes

			// see 'display_char' for an explanation of attribute encoding -
			display_char(bitmap, code, xpos, ypos, scroll_region, display_type, BIT(temp, 0), !BIT(temp, 1), !BIT(temp, 2), !BIT(temp, 3), force_blank);

			xpos++;
			if (xpos > m_columns)
			{
				xpos = 0;
				line++;
			}
		} // (else) valid char

	} // while

}

void rainbow_video_device::notify_vblank(bool v)
{
	static bool v_last;
	m_notify_vblank = v;

	if (m_scroll_latch_valid)
	{
		// Line linking / unlinking is done during VBI (see 4.7.4 and up in VT manual).
		if ((v == false) && (v_last == true))
			m_last_scroll = m_scroll_latch;
	}

	v_last = v;
}

void rainbow_video_device::palette_select(int choice)
{
	switch (choice)
	{
	default:
	case 0x01:
		m_palette->set_pen_color(1, 0xff - 100, 0xff - 100, 0xff - 100);  // WHITE (dim)
		m_palette->set_pen_color(2, 0xff - 50, 0xff - 50, 0xff - 50);     // WHITE NORMAL
		m_palette->set_pen_color(3, 0xff, 0xff, 0xff);              // WHITE (brighter)
		break;

	case 0x02:
		m_palette->set_pen_color(1, 35, 200 - 55, 75);        // GREEN (dim)
		m_palette->set_pen_color(2, 35 + 55, 200, 75 + 55);        // GREEN (NORMAL)
		m_palette->set_pen_color(3, 35 + 110, 200 + 55, 75 + 110);         // GREEN (brighter)
		break;

	case 0x03:
		m_palette->set_pen_color(1, 213 - 47, 146 - 47, 82 - 47); // AMBER (dim)
		m_palette->set_pen_color(2, 213, 146, 82); // AMBER (NORMAL)
		m_palette->set_pen_color(3, 255, 193, 129); // AMBER (brighter)
		break;
	}
}

// ****** RAINBOW ******
void rainbow_video_device::video_blanking(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// 'In reverse screen mode, termination forces the beam to the screen background intensity'
	// Background intensity means 'dim' (1) according to one source.
	bitmap.fill(((m_reverse_field ^ m_basic_attribute) ? 1 : 0), cliprect);
}

#define MHFU_IS_ENABLED 1
#define MHFU_COUNT -1
#define MHFU_VALUE -2
#define MHFU_RESET_and_ENABLE   -100
#define MHFU_RESET_and_DISABLE  -200
#define MHFU_RESET              -250

int rainbow_video_device::MHFU(int ASK)
{
	switch (ASK)
	{
	case MHFU_IS_ENABLED:       // "true": RETURN BOOLEAN (MHFU disabled or enabled?)
		return MHFU_FLAG;

	case MHFU_COUNT:        // -1: increment IF ENABLED, return counter value (=> Rainbow.c)
		if (MHFU_FLAG == true)
			if (MHFU_counter < 254)
				MHFU_counter++;
		[[fallthrough]];
	case MHFU_VALUE:
		return MHFU_counter;

	case MHFU_RESET:        // -250 : RESET counter (NOTHING ELSE!)
		MHFU_counter = 0;
		return MHFU_FLAG;

	case MHFU_RESET_and_ENABLE: // -100 : RESET and ENABLE MHFU counter
		MHFU_counter = 0;
		MHFU_FLAG = true;

		return -100;

	case MHFU_RESET_and_DISABLE:    // -200 : RESET and DISABLE MHFU
		MHFU_counter = 0;
		MHFU_FLAG = false;

		return -200;

	default:
		assert(1);
		return -255;
	} // switch
}

TIMER_CALLBACK_MEMBER(vt100_video_device::lba7_change)
{
	m_lba7 = (m_lba7) ? 0 : 1;
	m_write_lba7(m_lba7);

	if (!m_write_lba3_lba4.isnull())
	{
		// The first of every eight low periods of LBA 3 is twice as long
		m_write_lba3_lba4(2);
		m_lba3_change_timer->adjust(clocks_to_attotime(90), 3);
	}
}

TIMER_CALLBACK_MEMBER(vt100_video_device::lba3_change)
{
	m_write_lba3_lba4(param & 3);
	if (param <= 16)
		m_lba3_change_timer->adjust(clocks_to_attotime(45), param + 1);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vt100_video_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_entries(4);
}
