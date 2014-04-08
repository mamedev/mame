/**********************************************************************

    DEC VT Terminal video emulation
    [ DC012 and DC011 emulation ]

    01/05/2009 Initial implementation [Miodrag Milanovic]
    Portions (2013, 2014) by Karl-Ludwig Deisenhofer.

    DEC VIDEO : STATE AS OF MARCH 2014
    ----------------------------------
    - TESTS REQUIRED : do line and character attributes (plus combinations) match real hardware?

    - JUMPY SOFT SCROLL : Soft scroll *should* be synced with beam or DMA (line linking/unlinking is done during VBI, in less than 550ms).
                          See 4.7.4 and up in VT manual.

    - UNDOCUMENTED FEATURES of DC011 / DC012 (CLUES WANTED)
       A. (VT 100): DEC VT terminals are said to have a feature that doubles the number of lines
                    (50 real lines or just interlaced mode with 500 instead of 250 scanlines?)

       B. (DEC-100-B) fun PD program SQUEEZE.COM _compresses_ display to X/2 and Y/2
          - so picture takes a quarter of the original screen. How does it accomplish this?

    - IMPROVEMENTS:
        - exact colors for different VR201 monitors ('paper white', green and amber)

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "video/vtvideo.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define VERBOSE         0

#define LOG(x)      do { if (VERBOSE) logerror x; } while (0)


const device_type VT100_VIDEO = &device_creator<vt100_video_device>;
const device_type RAINBOW_VIDEO = &device_creator<rainbow_video_device>;


vt100_video_device::vt100_video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_video_interface(mconfig, *this),
						m_read_ram(*this),
						m_write_clear_video_interrupt(*this),
						m_palette(*this, "palette")
{
}


vt100_video_device::vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, VT100_VIDEO, "VT100 Video", tag, owner, clock, "vt100_video", __FILE__),
						device_video_interface(mconfig, *this),
						m_read_ram(*this),
						m_write_clear_video_interrupt(*this),
						m_palette(*this, "palette")
{
}


rainbow_video_device::rainbow_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: vt100_video_device(mconfig, RAINBOW_VIDEO, "Rainbow Video", tag, owner, clock, "rainbow_video", __FILE__)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vt100_video_device::device_config_complete()
{
	// inherit a copy of the static data
	const vt_video_interface *intf = reinterpret_cast<const vt_video_interface *>(static_config());
	if (intf != NULL)
		*static_cast<vt_video_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_char_rom_tag = "";
	}
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
	assert(m_gfx != NULL);

	// LBA7 is scan line frequency update
	machine().scheduler().timer_pulse(attotime::from_nsec(31778), timer_expired_delegate(FUNC(vt100_video_device::lba7_change),this));

	save_item(NAME(m_lba7));
	save_item(NAME(m_scroll_latch));
	save_item(NAME(m_blink_flip_flop));
	save_item(NAME(m_reverse_field));
	save_item(NAME(m_basic_attribute));
	save_item(NAME(m_columns));
	save_item(NAME(m_height));
	save_item(NAME(m_height_MAX));
	save_item(NAME(m_skip_lines));
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
	m_scroll_latch_valid = 0;

	m_blink_flip_flop = 0;
	m_reverse_field = 0;
	m_basic_attribute = 0;

	m_columns = 80;
	m_frequency = 60;
	m_interlaced = 1;
	m_skip_lines = 2; // for 60Hz
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
	m_scroll_latch_valid = 0;

	m_blink_flip_flop = 0;
	m_reverse_field = 0;
	m_basic_attribute = 0;

	m_columns = 80;
	m_frequency = 60;
	m_interlaced = 1;
	m_skip_lines = 2; // for 60Hz
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// Also used by Rainbow-100 ************
void vt100_video_device::recompute_parameters()
{
	int horiz_pix_total = 0;
	int vert_pix_total = 0;
	rectangle visarea;

	vert_pix_total  = m_height  * 10;

	if (m_columns == 132) {
		horiz_pix_total = m_columns * 9; // display 1 less filler pixel in 132 char. mode (DEC-100)
	} else {
		horiz_pix_total = m_columns * 10; // normal 80 character mode.
	}

	visarea.set(0, horiz_pix_total - 1, 0, vert_pix_total - 1);

	m_screen->configure(horiz_pix_total, vert_pix_total, visarea, m_screen->frame_period().attoseconds);
}


READ8_MEMBER( vt100_video_device::lba7_r )
{
	return m_lba7;
}


// Also used by Rainbow-100 ************
WRITE8_MEMBER( vt100_video_device::dc012_w )
{
	// TODO: writes to 10C/0C should be treated differently (emulation disables the watchdog too often).
	if (data == 0) // MHFU is disabled by writing 00 to port 010C.
	{
				//if (MHFU_FLAG == true)
				//  printf("MHFU  *** DISABLED *** \n");
				MHFU_FLAG = false;
				MHFU_counter = 0;
	}
	else
	{           // RESET
				//if (MHFU_FLAG == false)
				//  printf("MHFU  ___ENABLED___ \n");
				MHFU_FLAG = true;

				MHFU_counter = 0;
	}

	if (!(data & 0x08))
	{
		// The scroll offset put in 'm_scroll_latch' is a decimal offset controlling 10 scan lines.
		// The BIOS first writes the least significant bits, then the 2 most significant bits.

		// If scrolling up (incrementing the scroll latch), the additional line is linked in at the bottom.
		// When the scroll latch is incremented back to 0, the top line of the scrolling region must be unlinked.

		// When scrolling down (decrementing the scroll latch), new lines must be linked in at the top of the scroll region
		// and unlinked down at the bottom.

		// Note that the scroll latch value will be used during the next frame rather than the current frame.
		// All line linking/unlinking is done during the vertical blanking interval (< 550ms).

		// More on scrolling regions: Rainbow 100 B technical documentation (QV069-GZ) April 1985 page 22
		// Also see VT100 Technical Manual: 4.7.4 Address Shuffling to 4.7.9 Split Screen Smooth Scrolling.
		if (!(data & 0x04))
		{
			m_scroll_latch_valid = 0; // LSB is written first.

			// set lower part scroll
			m_scroll_latch = data & 0x03;
		}
		else
		{
			// set higher part scroll
			m_scroll_latch = (m_scroll_latch & 0x03) | ((data & 0x03) << 2);

			m_scroll_latch_valid = 1; // MSB is written last.
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

				if (m_height != 24)
				{
					m_height = 24;  // (DEC Rainbow 100) : 24 line display
					recompute_parameters();
				}
				break;

			case 0x0e:
				m_blink_flip_flop = 0;  // 'unsupported' DC012 command. Turn blink flip-flop off.
				break;

			case 0x0f:
				// (DEC Rainbow 100): reverse video with 48 lines / blink flip-flop off
				m_blink_flip_flop = 0;
				m_basic_attribute = 1;

				// 0x0f = 'reserved' on VT 100
				//  Abort on VT-100 for now.
				if (m_height_MAX == 25) break;

				if (m_height != 48)
				{
					m_height = 48;   // (DEC Rainbow 100) : 48 line display
					recompute_parameters();
				}
				break;
		}
	}
}


WRITE8_MEMBER( vt100_video_device::dc011_w )
{
	if (!BIT(data, 5))
	{
		UINT8 col = m_columns;
		if (!BIT(data, 4))
		{
			m_columns = 80;
		}
		else
		{
			m_columns = 132;
		}
		if (col != m_columns)
		{
			recompute_parameters();
		}
		m_interlaced = 1;
	}
	else
	{
		if (!BIT(data, 4))
		{
			m_frequency = 60;
			m_skip_lines = 2;
		}
		else
		{
			m_frequency = 50;
			m_skip_lines = 5;
		}
		m_interlaced = 0;
	}
}

WRITE8_MEMBER( vt100_video_device::brightness_w )
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
			case 0 : // bottom half, double height
						j = (i >> 1) + 5; break;
			case 1 : // top half, double height
						j = (i >> 1); break;
			case 2 : // double width
			case 3 : // normal
						j = i;  break;
			default : j = 0; break;
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
				bitmap.pix16(y * 10 + i, x * 20 + b * 2)     =  (bit | prevbit) ^ invert;
				bitmap.pix16(y * 10 + i, x * 20 + b * 2 + 1) =  bit ^ invert;
			}
			else
			{
				bitmap.pix16(y * 10 + i, x * 10 + b) =  (bit | prevbit) ^ invert;
			}
		}
		prevbit = bit;
		// char interleave is filled with last bit
		if (double_width)
		{
			bitmap.pix16(y * 10 + i, x * 20 + 16) =  (bit | prevbit) ^ invert;
			bitmap.pix16(y * 10 + i, x * 20 + 17) =  bit ^ invert;
			bitmap.pix16(y * 10 + i, x * 20 + 18) =  bit ^ invert;
			bitmap.pix16(y * 10 + i, x * 20 + 19) =  bit ^ invert;
		}
		else
		{
			bitmap.pix16(y * 10 + i, x * 10 + 8) =  (bit | prevbit) ^ invert;
			bitmap.pix16(y * 10 + i, x * 10 + 9) =  bit ^ invert;
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

	while (line < (m_height + m_skip_lines))
	{
		code = m_read_ram(addr + xpos);
		if (code == 0x7f)
		{
			// end of line, fill empty till end of line
			if (line >= m_skip_lines)
			{
				for (x = xpos; x < ((display_type == 2) ? (m_columns / 2) : m_columns); x++)
				{
					display_char(bitmap, code, x, ypos, scroll_region, display_type);
				}
			}
			// move to new data
			temp = m_read_ram(addr + xpos + 1) * 256 + m_read_ram(addr + xpos + 2);
			addr = (temp) & 0x1fff;
			// if A12 is 1 then it is 0x2000 block, if 0 then 0x4000 (AVO)
			if (addr & 0x1000) addr &= 0xfff; else addr |= 0x2000;
			scroll_region = (temp >> 15) & 1;
			display_type  = (temp >> 13) & 3;
			if (line >= m_skip_lines)
			{
				ypos++;
			}
			xpos = 0;
			line++;
		}
		else
		{
			// display regular char
			if (line >= m_skip_lines)
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
void rainbow_video_device::display_char(bitmap_ind16 &bitmap, UINT8 code, int x, int y, UINT8 scroll_region, UINT8 display_type)
{
	UINT16 y_preset;
	UINT16 x_preset, d_x_preset;
	if (m_columns == 132)
	{     x_preset   = x * 9;
			d_x_preset = x * 18;
	} else
	{
			x_preset   = x * 10;
			d_x_preset = x * 20;
	}

	UINT8 line = 0;
	int bit = 0, j = 0;
	int fg_intensity;
	int back_intensity, back_default_intensity;

	int invert = (display_type &  8) >> 3; // BIT 3 indicates REVERSE
	int bold = (display_type & 16) >> 4;   // BIT 4 indicates BOLD
	int blink  = display_type &  32;       // BIT 5 indicates BLINK
	int underline = display_type & 64;     // BIT 6 indicates UNDERLINE
	bool blank = (display_type & 0x80) ? true : false; // BIT 7 indicates BLANK

	display_type = display_type & 3;

	static int old_scroll_region;

	// * SCREEN ATTRIBUTES (see VT-180 manual 6-30) *
	// 'reverse field' = reverse video over entire screen

	// For reference: a complete truth table can be taken from TABLE 4-6-4 / VT100 technical manual.
	// Following simple IF statements implement it in full. Code segments should not be shuffled.
	invert = invert ^ m_reverse_field ^ m_basic_attribute;

	fg_intensity = bold + 2;   // FOREGROUND (FG):  normal (2) or bright (3)

	back_intensity = 0; // DO NOT SHUFFLE CODE AROUND !!
	if ( (blink != 0) && ( m_blink_flip_flop != 0 ) )
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

	bool double_width  = (display_type != 3) ? true  : false; // all except normal: double width
	bool double_height = (display_type &  1) ? false : true;  // 0,2 = double height

	for (int scan_line = 0; scan_line < 10; scan_line++)
	{
		y_preset = y * 10 + scan_line;

		// Offset to bitmap in char-rom (used later below)
		int i = scan_line;

		// Affects 'i'  / 'y_preset' / 'scan_line' (= LOOP VARIABLE)
		if ( m_scroll_latch_valid == 1)
		{
			// **** START IF SCROLL REGION:
			if ( (old_scroll_region == 0) && (scroll_region == 1) )
			{
			if (scan_line == 0)  // * EXECUTED ONCE *
			{
				scan_line = m_scroll_latch; // write less lines  ! SIDE EFFECT ON LOOP !
				i = m_scroll_latch;         // set hard offset to char-rom
			}
			}

			// **** MIDDLE OF REGION:
			if ( (old_scroll_region == 1) && (scroll_region == 1) )
			{
			if (    ( y * 10 + scan_line - m_scroll_latch ) >=   0 )
						y_preset = y * 10 + scan_line - m_scroll_latch;
			}

			// **** END OF SCROLL REGION:
			if ( (old_scroll_region == 1) && (scroll_region == 0) )
			{
			if (i > (9 - m_scroll_latch) )
			{   old_scroll_region = m_scroll_latch_valid; // keep track
				return;                         // WHAT HAPPENS WITH THE REST OF THE LINE (BLANK ?)
			}
			}
		} // (IF) scroll latch valid

		switch (display_type)
		{
			case 0 :  // bottom half of 'double height, double width' char.
						j = (i >> 1) + 5;
						break;

			case 2 :  // top half of 'double height, double width' char.
						j = (i >> 1);
						break;

			default : // 1: double width
						// 3: normal
						j = i;
						break;
		}

		// modify line since that is how it is stored in rom
		if (j == 0) j = 15; else j = j - 1;

		line = m_gfx[ (code << 4) + j]; // code * 16

		// UNDERLINED CHARACTERS (CASE 5 - different in 1 line):
		back_intensity = back_default_intensity; // 0, 1, 2
		if ( underline != 0 )
		{
			if ( i == 8 )
			{
					if (invert == 0)
						line = 0xff; // CASE 5 A)
					else
					{    line = 0x00; // CASE 5 B)
						back_intensity = 0; // OVERRIDE: BLACK BACKGROUND
					}
			}
		}

		for (int b = 0; b < 8; b++) // 0..7
		{
			if (blank)
			{       bit = m_reverse_field ^ m_basic_attribute;
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
				bitmap.pix16( y_preset, d_x_preset + (b << 1) + 1) = bit;
				bitmap.pix16( y_preset, d_x_preset + (b << 1)    ) = bit;

				if (double_height)
				{
						bitmap.pix16( 1 + y_preset, d_x_preset + (b << 1) + 1) = bit;
						bitmap.pix16( 1 + y_preset, d_x_preset + (b << 1)    ) = bit;
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
			bitmap.pix16(y_preset, d_x_preset + 16) = bit;
			bitmap.pix16(y_preset, d_x_preset + 17) = bit;

			if (m_columns == 80)
			{   bitmap.pix16(y_preset, d_x_preset + 18) = bit;
				bitmap.pix16(y_preset, d_x_preset + 19) = bit;
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
	UINT8 scroll_region = 1;  // DEFAULT TO 1 = PART OF scroll_region
	UINT8 display_type = 3;  // binary 11
	UINT16 temp = 0;

	while (line < (m_height + m_skip_lines))
	{
		code = m_read_ram(addr + xpos);

		if ( code == 0x00 )        // TODO: investigate side effect on regular zero character!
				display_type |= 0x80; // DEFAULT: filler chars (till end of line) and empty lines (00) will be blanked
		else
				display_type &= 0x7f; // else activate display.

		if ( code == 0xff )
		{
			// end of line, fill empty till end of line
			if (line >= m_skip_lines)
			{
				// NOTE: display_type is already shifted! All except 3 is DOUBLE WIDTH 40 or 66 chars per line
				for (x = xpos; x < ( (display_type != 3) ? (m_columns / 2) : m_columns );  x++)
				{
					display_char(bitmap, code, x, ypos, scroll_region, display_type | 0x80);
				}

			}

			//  LINE ATTRIBUTE - valid for all chars on next line  ** DO NOT SHUFFLE **
			attr_addr = 0x1000 | ( (addr + xpos + 1) & 0x0fff );

			// MOVE TO NEW DATA
			temp = m_read_ram(addr + xpos + 2) * 256 + m_read_ram(addr + xpos + 1);
			addr = (temp) & 0x0fff;

			temp = m_read_ram(attr_addr);
			scroll_region = (temp) & 1;
			display_type  = (temp >> 1) & 3;

			if (line >= m_skip_lines)
			{
				ypos++;
			}
			xpos = 0;
			line++;
		}
		else
		{
			// display regular char
			if (line >= m_skip_lines)
			{
				attr_addr = 0x1000 | ( (addr + xpos) & 0x0fff );
				temp = m_read_ram(attr_addr); // get character attribute

				// CONFIRMED: Reverse active on 1.  No attributes = 0x0E
				// 1 = display char. in REVERSE   (encoded as 8)
				// 0 = display char. in BOLD      (encoded as 16)
				// 0 = display char. w. BLINK     (encoded as 32)
				// 0 = display char. w. UNDERLINE (encoded as 64).
				display_char(bitmap, code, xpos, ypos, scroll_region, display_type | (   (    (temp & 1)) << 3 )
																						| ( (2-(temp & 2)) << 3 )
																						| ( (4-(temp & 4)) << 3 )
																						| ( (8-(temp & 8)) << 3 )
																					);

			}
			xpos++;

			if (xpos > m_columns )
			{
				xpos = 0;
				line++;
			}
		} // (else) valid char

	} // while


}


void rainbow_video_device::palette_select ( int choice )
{
	switch(choice)
	{
			default:
			case 0x01:
						m_palette->set_pen_color(1, 0xff-100, 0xff-100, 0xff-100);  // WHITE (dim)
						m_palette->set_pen_color(2, 0xff-50, 0xff-50, 0xff-50);     // WHITE NORMAL
						m_palette->set_pen_color(3, 0xff, 0xff, 0xff);              // WHITE (brighter)
						break;

			case 0x02:
						m_palette->set_pen_color(1, 0 , 205 -50, 70 - 50);        // GREEN (dim)
						m_palette->set_pen_color(2, 0 , 205,     70     );        // GREEN (NORMAL)
						m_palette->set_pen_color(3, 0,  205 +50, 70 + 50);        // GREEN (brighter)
						break;

			case 0x03:
						m_palette->set_pen_color(1, 213 - 47, 146 - 47, 82 - 47); // AMBER (dim)
						m_palette->set_pen_color(2, 213,      146,      82     ); // AMBER (NORMAL)
						m_palette->set_pen_color(3, 255,      193,      129    ); // AMBER (brighter)
						break;
	}
}

// ****** RAINBOW ******
void rainbow_video_device::video_blanking(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// 'In reverse screen mode, termination forces the beam to the screen background intensity'
	// Background intensity means 'dim' (1) according to one source.
	bitmap.fill( ((m_reverse_field ^ m_basic_attribute) ? 1 : 0) , cliprect);
}



int rainbow_video_device::MHFU(int ASK)
{
	switch (ASK)
	{
			case 1:         // "true": RETURN BOOLEAN (MHFU disabled or enabled?)
				return MHFU_FLAG;

			case -1:        // -1: increment, return counter value (=> Rainbow.c)
					if (MHFU_FLAG == true)
					MHFU_counter++;
				return MHFU_counter;

			case -100:          // -100 : RESET and ENABLE MHFU counter
				//printf("-100 MHFU  * reset and ENABLE * \n");
				MHFU_counter = 0;

				//if (MHFU_FLAG == false)
				//  printf("-100 MHFU  ___ENABLED___\n");
				MHFU_FLAG = true;

				return -100;

			default:
				assert(1);
				return -255;
	} // switch
}

TIMER_CALLBACK_MEMBER( vt100_video_device::lba7_change )
{
	m_lba7 = (m_lba7) ? 0 : 1;
}

static MACHINE_CONFIG_FRAGMENT( vt100_video )
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor vt100_video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vt100_video );
}

static MACHINE_CONFIG_FRAGMENT( rainbow_video )
	MCFG_PALETTE_ADD("palette", 4)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor rainbow_video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( rainbow_video );
}
