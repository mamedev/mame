// license:GPL-2.0+
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

/***************************************************************************
PARAMETERS
***************************************************************************/

#define VERBOSE         1

#define LOG(x)      do { if (VERBOSE) logerror x; } while (0)


const device_type VT100_VIDEO = &device_creator<vt100_video_device>;
const device_type RAINBOW_VIDEO = &device_creator<rainbow_video_device>;


vt100_video_device::vt100_video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
device_video_interface(mconfig, *this),
m_read_ram(*this),
m_write_clear_video_interrupt(*this),
m_char_rom_tag(""),
m_palette(*this, "palette")
{
}


vt100_video_device::vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, VT100_VIDEO, "VT100 Video", tag, owner, clock, "vt100_video", __FILE__),
device_video_interface(mconfig, *this),
m_read_ram(*this),
m_write_clear_video_interrupt(*this),
m_char_rom_tag(""),
m_palette(*this, "palette")
{
}


rainbow_video_device::rainbow_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: vt100_video_device(mconfig, RAINBOW_VIDEO, "Rainbow Video", tag, owner, clock, "rainbow_video", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vt100_video_device::device_start()
{
	/* resolve callbacks */
	m_read_ram.resolve_safe(0);
	m_write_clear_video_interrupt.resolve_safe();

	m_gfx = machine().root_device().memregion(m_char_rom_tag)->base();
	assert(m_gfx != nullptr);

	// LBA7 is scan line frequency update
	machine().scheduler().timer_pulse(attotime::from_nsec(31778), timer_expired_delegate(FUNC(vt100_video_device::lba7_change), this));

	save_item(NAME(m_lba7));
	save_item(NAME(m_scroll_latch));
	save_item(NAME(m_blink_flip_flop));
	save_item(NAME(m_reverse_field));
	save_item(NAME(m_basic_attribute));
	save_item(NAME(m_columns));
	save_item(NAME(m_height));
	save_item(NAME(m_height_MAX));
	save_item(NAME(m_fill_lines));
	save_item(NAME(m_frequency));
	save_item(NAME(m_interlaced));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vt100_video_device::device_reset()
{
	m_palette->set_pen_color(0, 0x00, 0x00, 0x00); // black
	m_palette->set_pen_color(1, 0xff, 0xff, 0xff); // white

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
	m_frequency = 60;

	m_interlaced = 1;
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

	m_frequency = 60;

	m_interlaced = 1;
	m_fill_lines = 2; // for 60Hz (not in use any longer -> detected)
	recompute_parameters();
}

/***************************************************************************
IMPLEMENTATION
***************************************************************************/

// Also used by Rainbow-100 ************
void vt100_video_device::recompute_parameters()
{
	rectangle visarea;
	int horiz_pix_total = 0;

	// RAINBOW: 240 scan lines in non-interlaced mode (480 interlaced). VT-100 : same (?)
	m_linedoubler = false; // 24 "true" lines (240)  -OR-  48 lines with NO ghost lines (true 480)
	if ((m_interlaced) && (m_height == 24))
		m_linedoubler = true; // 24 lines with 'double scan' (false 480)

	int vert_pix_total = ((m_linedoubler == false) ? m_height : m_height_MAX) * 10;

	if (m_columns == 132)
		horiz_pix_total = m_columns * 9; // display 1 less filler pixel in 132 char. mode
	else
		horiz_pix_total = m_columns * 10; // normal 80 character mode.

	visarea.set(0, horiz_pix_total - 1, 0, vert_pix_total - 1);
	machine().first_screen()->configure(horiz_pix_total, vert_pix_total, visarea, HZ_TO_ATTOSECONDS((m_interlaced == 0) ? m_frequency : (m_frequency/2) ));

	if (VERBOSE)
	{
		printf("\n(RECOMPUTE) HPIX: %d - VPIX: %d", horiz_pix_total, vert_pix_total);
		printf("\n(RECOMPUTE) FREQUENCY: %d", (m_interlaced == 0) ? m_frequency : (m_frequency / 2));
		if (m_interlaced)
			printf("\n(RECOMPUTE) * INTERLACED *");
		if (m_linedoubler)
			printf("\n(RECOMPUTE) * LINEDOUBLER *");
	}
}


READ8_MEMBER(vt100_video_device::lba7_r)
{
	return m_lba7;
}


// Also used by Rainbow-100 ************
WRITE8_MEMBER(vt100_video_device::dc012_w)
{
	// Writes to [10C] and [0C] are treated differently
	// - see 3.1.3.9.5 DC012 Programming Information (PC-100 spec)

	// MHFU is disabled by writing 00 to port 010C.

	// Code recognition is abysmal - sorry for that.
	if (data == 0)
	{
		UINT8 *rom = machine().root_device().memregion("maincpu")->base();
		if (rom != nullptr)
		{
			UINT32 PC = space.device().safe_pc();
			if ((rom[ PC - 1] == 0xe6) &&
				(rom[ PC    ] == 0x0c)
				)
			{
				// OUT 0C,al  < DO NOTHING >
			}
			else
			{
				//UINT8 magic1= rom[PC - 1];
				//printf("\n PC %05x - MHFU MAGIC -1 %02x\n", PC,  magic1);
				//UINT8 magic2 = rom[PC - 2];
				//printf("\n PC %05x - MHFU MAGIC -2 %02x\n", PC, magic2);
				//if (VERBOSE)

				//if(1  )
				if ((rom[PC - 2] == 0x0C) &&
					(rom[PC - 1] == 0x01)
					)
				{
					if (MHFU_FLAG == true)
						printf("MHFU  *** DISABLED *** %05x \n", PC);

					MHFU_FLAG = false;
					MHFU_counter = 0;
				}
			}

		} // DATA == 0 ONLY ....

	}
	else
	{
		//if (VERBOSE)
		if (MHFU_FLAG == false)
			printf("MHFU  ___ENABLED___ %05x \n", space.device().safe_pc());

		// RESET
		MHFU_FLAG = true;
		MHFU_counter = 0;
	}

	if (!(data & 0x08))
	{
		if (!(data & 0x04))
		{
			m_scroll_latch_valid = false; // LSB is written first.
			// set lower part scroll
			m_scroll_latch = data & 0x03;
		}
		else
		{
			// set higher part scroll
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
			m_write_clear_video_interrupt(0);
			break;
		case 0x0a:
			// set reverse field on
			m_reverse_field = 1;
			break;
		case 0x0b:
			// set reverse field off
			m_reverse_field = 0;
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
WRITE8_MEMBER(vt100_video_device::dc011_w)
{
	if (!BIT(data, 5))
	{
		m_interlaced = 1;

		if (!BIT(data, 4))
			m_columns = 80;
		else
			m_columns = 132;
	}
	else
	{
		m_interlaced = 0;

		if (!BIT(data, 4))
		{
			m_frequency = 60;
			m_fill_lines = 2;
		}
		else
		{
			m_frequency = 50;
			m_fill_lines = 5;
		}
	}

	recompute_parameters();
}

WRITE8_MEMBER(vt100_video_device::brightness_w)
{
	//m_palette->set_pen_color(1, data, data, data);
}



void vt100_video_device::display_char(bitmap_ind16 &bitmap, UINT8 code, int x, int y, UINT8 scroll_region, UINT8 display_type)
{
	UINT8 line = 0;
	int bit = 0, prevbit, invert = 0, j;
	int double_width = (display_type == 2) ? 1 : 0;

	for (int i = 0; i < 10; i++)
	{
		switch (display_type)
		{
		case 0: // bottom half, double height
			j = (i >> 1) + 5; break;
		case 1: // top half, double height
			j = (i >> 1); break;
		case 2: // double width
		case 3: // normal
			j = i;  break;
		default: j = 0; break;
		}
		// modify line since that is how it is stored in rom
		if (j == 0) j = 15; else j = j - 1;

		line = m_gfx[(code & 0x7f) * 16 + j];

		if (m_basic_attribute == 1)
		{
			if ((code & 0x80) == 0x80)
				invert = 1;
			else
				invert = 0;
		}

		for (int b = 0; b < 8; b++)
		{
			prevbit = bit;
			bit = BIT((line << b), 7);
			if (double_width)
			{
				bitmap.pix16(y * 10 + i, x * 20 + b * 2) = (bit | prevbit) ^ invert;
				bitmap.pix16(y * 10 + i, x * 20 + b * 2 + 1) = bit ^ invert;
			}
			else
			{
				bitmap.pix16(y * 10 + i, x * 10 + b) = (bit | prevbit) ^ invert;
			}
		}
		prevbit = bit;
		// char interleave is filled with last bit
		if (double_width)
		{
			bitmap.pix16(y * 10 + i, x * 20 + 16) = (bit | prevbit) ^ invert;
			bitmap.pix16(y * 10 + i, x * 20 + 17) = bit ^ invert;
			bitmap.pix16(y * 10 + i, x * 20 + 18) = bit ^ invert;
			bitmap.pix16(y * 10 + i, x * 20 + 19) = bit ^ invert;
		}
		else
		{
			bitmap.pix16(y * 10 + i, x * 10 + 8) = (bit | prevbit) ^ invert;
			bitmap.pix16(y * 10 + i, x * 10 + 9) = bit ^ invert;
		}
	}
}

void vt100_video_device::video_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 addr = 0;
	int line = 0;
	int xpos = 0;
	int ypos = 0;
	UINT8 code;
	int x = 0;
	UINT8 scroll_region = 1; // binary 1
	UINT8 display_type = 3;  // binary 11
	UINT16 temp = 0;

	if (m_read_ram(0) != 0x7f)
		return;

	while (line < (m_height + m_fill_lines))
	{
		code = m_read_ram(addr + xpos);
		if (code == 0x7f)
		{
			// end of line, fill empty till end of line
			if (line >= m_fill_lines)
			{
				for (x = xpos; x < ((display_type == 2) ? (m_columns / 2) : m_columns); x++)
				{
					display_char(bitmap, code, x, ypos, scroll_region, display_type);
				}
			}
			// move to new data
			temp = m_read_ram(addr + xpos + 1) * 256 + m_read_ram(addr + xpos + 2);
			addr = (temp)& 0x1fff;
			// if A12 is 1 then it is 0x2000 block, if 0 then 0x4000 (AVO)
			if (addr & 0x1000) addr &= 0xfff; else addr |= 0x2000;
			scroll_region = (temp >> 15) & 1;
			display_type = (temp >> 13) & 3;
			if (line >= m_fill_lines)
			{
				ypos++;
			}
			xpos = 0;
			line++;
		}
		else
		{
			// display regular char
			if (line >= m_fill_lines)
			{
				display_char(bitmap, code, xpos, ypos, scroll_region, display_type);
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
// 1 = display char. in REVERSE   (encoded as 8 in display_type)  HIGH ACTIVE
// 0 = display char. in BOLD      (encoded as 16 in display_type) LOW ACTIVE
// 0 = display char. w. BLINK     (encoded as 32 in display_type) LOW ACTIVE
// 0 = display char. w. UNDERLINE (encoded as 64 in display_type) LOW ACTIVE
void rainbow_video_device::display_char(bitmap_ind16 &bitmap, UINT8 code, int x, int y, UINT8 scroll_region, UINT8 display_type)
{
	UINT16 x_preset = x << 3; // x_preset = x * 9 (= 132 column mode)
	x_preset += x;

	if (m_columns == 80)
		x_preset += x; //        x_preset = x * 10 (80 column mode)

	UINT16 y_preset;

	UINT16 CHARPOS_y_preset = y << 3; // CHARPOS_y_preset = y * 10;
	CHARPOS_y_preset += y;
	CHARPOS_y_preset += y;

	UINT16 DOUBLE_x_preset = x_preset << 1; // 18 for 132 column mode, else 20 (x_preset * 2)

	UINT8 line = 0;
	int bit = 0, j = 0;
	int fg_intensity;
	int back_intensity, back_default_intensity;

	int invert = (display_type & 8) ? 1 : 0; // REVERSE
	int bold  = (display_type & 16) ? 0 : 1; // BIT 4
	int blink = (display_type & 32) ? 0 : 1; // BIT 5
	int underline = (display_type & 64) ? 0 : 1; // BIT 6
	bool blank = (display_type & 128) ? true : false; // BIT 7

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

		line = m_gfx[(code << 4) + j]; // code * 16

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
				bitmap.pix16(y_preset, DOUBLE_x_preset + (b << 1) + 1) = bit;
				bitmap.pix16(y_preset, DOUBLE_x_preset + (b << 1)) = bit;

				if (double_height)
				{
					bitmap.pix16(1 + y_preset, DOUBLE_x_preset + (b << 1) + 1) = bit;
					bitmap.pix16(1 + y_preset, DOUBLE_x_preset + (b << 1)) = bit;
				}
			}
			else
			{
				bitmap.pix16(y_preset, x_preset + b) = bit;
			}
		} // for (8 bit)

		// char interleave (X) is filled with last bit
		if (double_width)
		{
			// double chars: 18 or 20 bits
			bitmap.pix16(y_preset, DOUBLE_x_preset + 16) = bit;
			bitmap.pix16(y_preset, DOUBLE_x_preset + 17) = bit;

			if (m_columns == 80)
			{
				bitmap.pix16(y_preset, DOUBLE_x_preset + 18) = bit;
				bitmap.pix16(y_preset, DOUBLE_x_preset + 19) = bit;
			}
		}
		else
		{   // normal chars: 9 or 10 bits
			bitmap.pix16(y_preset, x_preset + 8) = bit;

			if (m_columns == 80)
				bitmap.pix16(y_preset, x_preset + 9) = bit;
		}
	} // for (scan_line)

}

// ****** RAINBOW ******
void rainbow_video_device::video_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 addr = 0;
	UINT16 attr_addr = 0;
	int line = 0;
	int xpos = 0;
	int ypos = 0;
	UINT8 code;
	int x = 0;
	UINT8 scroll_region = 0;  // DEFAULT TO 0 = NOT PART OF scroll_region
	UINT8 display_type = 3;  // NORMAL DISPLAY - binary 11
	UINT16 temp = 0;

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

	int vert_charlines_MAX = ((m_linedoubler == false) ? m_height : m_height_MAX);
	while (line < vert_charlines_MAX )
	{
		code = m_read_ram(addr + xpos);

		if (code == 0x00)        // TODO: investigate side effect on regular zero character!
			display_type |= 0x80; // DEFAULT: filler chars (till end of line) and empty lines (00) will be blanked
		else
			display_type &= 0x7f; // else activate display.

		if (code == 0xff) // end of line, fill empty till end of line
		{
			// HINT: display_type is already shifted! All except 3 is DOUBLE WIDTH 40 or 66 chars per line
			for (x = xpos; x < ((display_type != 3) ? (m_columns / 2) : m_columns); x++)
			{
					display_char(bitmap, code, x, ypos, scroll_region, display_type | 0x80);
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
			temp = (m_read_ram(attr_addr) & 15) << 3; // get character attributes

			// see 'display_char' for an explanation of attribute encoding -
			display_char(bitmap, code, xpos, ypos, scroll_region, display_type | temp);

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



int rainbow_video_device::MHFU(int ASK)
{
	switch (ASK)
	{
	case 1:         // "true": RETURN BOOLEAN (MHFU disabled or enabled?)
		return MHFU_FLAG;

	case -1:        // -1: increment IF ENABLED, return counter value (=> Rainbow.c)
		if (MHFU_FLAG == true)
			MHFU_counter++;
		return MHFU_counter;

	case -100:          // -100 : RESET and ENABLE MHFU counter
		MHFU_counter = 0;
		if(1) //if (VERBOSE)
			printf("-100 MHFU  * reset and ENABLE * \n");

		if(1) // if (VERBOSE)
		{
			if (MHFU_FLAG == false)
				printf("-100 MHFU  ___ENABLED___\n");
		}
		MHFU_FLAG = true;

		return -100;

	case -200:          // -200 : RESET and DISABLE MHFU
		MHFU_counter = 0;

		if(1) //if (VERBOSE)
		{
			if (MHFU_FLAG == true)
				printf("MHFU  *** DISABLED *** \n");
		}
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
}

static MACHINE_CONFIG_FRAGMENT(vt100_video)
MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor vt100_video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(vt100_video);
}

static MACHINE_CONFIG_FRAGMENT(rainbow_video)
MCFG_PALETTE_ADD("palette", 4)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor rainbow_video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(rainbow_video);
}
