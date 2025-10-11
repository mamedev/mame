// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Ensoniq Vacuum Fluorescent Displays (VFDs)
    Emulation by R. Belmont
*/

#include "emu.h"
#include "esqvfd.h"

#include "esq1by22.lh"
#include "esq2by40.lh"

DEFINE_DEVICE_TYPE(ESQ1X22,     esq1x22_device,     "esq1x22",     "Ensoniq 1x22 VFD")
DEFINE_DEVICE_TYPE(ESQ2X40,     esq2x40_device,     "esq2x40",     "Ensoniq 2x40 VFD")
DEFINE_DEVICE_TYPE(ESQ2X40_SQ1, esq2x40_sq1_device, "esq2x40_sq1", "Ensoniq 2x40 VFD (SQ-1 variant)")
DEFINE_DEVICE_TYPE(ESQ2X40_VFX, esq2x40_vfx_device, "esq2x40_vfx", "Ensoniq 2x40 VFD (VFX Family variant)")

// adapted from bfm_bd1, rearranged to work with ASCII data used by the Ensoniq h/w
static const uint16_t font[]=
{           // FEDC BA98 7654 3210
	0x0000, // 0000 0000 0000 0000 (space)
	0x0000, // 0000 0000 0000 0000 ! (not defined)
	0x0009, // 0000 0000 0000 1001 ".
	0xC62A, // 1100 0110 0010 1010 #.
	0xC62D, // 1100 0110 0010 1101 $.
	0x0000, // 0000 0000 0000 0000 % (not defined)
	0x0000, // 0000 0000 0000 0000 & (not defined)
	0x0040, // 0000 0000 1000 0000 '.
	0x0880, // 0000 1000 1000 0000 (.
	0x0050, // 0000 0000 0101 0000 ).
	0xCCD8, // 1100 1100 1101 1000 *.
	0xC408, // 1100 0100 0000 1000 +.
	0x0000, // 0000 0000 0000 0000 , (not defined)
	0xC000, // 1100 0000 0000 0000 -.
	0x1000, // 0001 0000 0000 0000 .
	0x0090, // 0000 0000 1001 0000 /
	0x22B7, // 0010 0010 1011 0111 0.
	0x0408, // 0000 0100 0000 1000 1.
	0xE206, // 1110 0010 0000 0110 2.
	0x4226, // 0100 0010 0010 0110 3.
	0xC023, // 1100 0000 0010 0011 4.
	0xC225, // 1100 0010 0010 0101 5.
	0xE225, // 1110 0010 0010 0101 6.
	0x0026, // 0000 0000 0010 0110 7.
	0xE227, // 1110 0010 0010 0111 8.
	0xC227, // 1100 0010 0010 0111 9.
	0x0000, // 0000 0000 0000 0000 : (not defined)
	0x0000, // 0000 0000 0000 0000 ; (not defined)
	0x0290, // 0000 0010 1001 0000 <.
	0xC200, // 1100 0010 0000 0000 =.
	0x0A40, // 0000 1010 0100 0000 >.
	0x0000, // 0000 0000 0000 0000 ? (not defined)
	0xA626, // 1010 0110 0010 0110 @.
	0xE027, // 1110 0000 0010 0111 A.
	0x462E, // 0100 0110 0010 1110 B.
	0x2205, // 0010 0010 0000 0101 C.
	0x062E, // 0000 0110 0010 1110 D.
	0xA205, // 1010 0010 0000 0101 E.
	0xA005, // 1010 0000 0000 0101 F.
	0x6225, // 0110 0010 0010 0101 G.
	0xE023, // 1110 0000 0010 0011 H.
	0x060C, // 0000 0110 0000 1100 I.
	0x2222, // 0010 0010 0010 0010 J.
	0xA881, // 1010 1000 1000 0001 K.
	0x2201, // 0010 0010 0000 0001 L.
	0x20E3, // 0010 0000 1110 0011 M.
	0x2863, // 0010 1000 0110 0011 N.
	0x2227, // 0010 0010 0010 0111 O.
	0xE007, // 1110 0000 0000 0111 P.
	0x2A27, // 0010 1010 0010 0111 Q.
	0xE807, // 1110 1000 0000 0111 R.
	0xC225, // 1100 0010 0010 0101 S.
	0x040C, // 0000 0100 0000 1100 T.
	0x2223, // 0010 0010 0010 0011 U.
	0x2091, // 0010 0000 1001 0001 V.
	0x2833, // 0010 1000 0011 0011 W.
	0x08D0, // 0000 1000 1101 0000 X.
	0x04C0, // 0000 0100 1100 0000 Y.
	0x0294, // 0000 0010 1001 0100 Z.
	0x2205, // 0010 0010 0000 0101 [.
	0x0840, // 0000 1000 0100 0000 \.
	0x0226, // 0000 0010 0010 0110 ].
	0x0810, // 0000 1000 0001 0000 ^.
	0x0200, // 0000 0010 0000 0000 _
	0x0040, // 0000 0000 0100 0000 `
	0xE027, // 1110 0000 0010 0111 A.
	0x462E, // 0100 0110 0010 1110 B.
	0x2205, // 0010 0010 0000 0101 C.
	0x062E, // 0000 0110 0010 1110 D.
	0xA205, // 1010 0010 0000 0101 E.
	0xA005, // 1010 0000 0000 0101 F.
	0x6225, // 0110 0010 0010 0101 G.
	0xE023, // 1110 0000 0010 0011 H.
	0x060C, // 0000 0110 0000 1100 I.
	0x2222, // 0010 0010 0010 0010 J.
	0xA881, // 1010 1000 1000 0001 K.
	0x2201, // 0010 0010 0000 0001 L.
	0x20E3, // 0010 0000 1110 0011 M.
	0x2863, // 0010 1000 0110 0011 N.
	0x2227, // 0010 0010 0010 0111 O.
	0xE007, // 1110 0000 0000 0111 P.
	0x2A27, // 0010 1010 0010 0111 Q.
	0xE807, // 1110 1000 0000 0111 R.
	0xC225, // 1100 0010 0010 0101 S.
	0x040C, // 0000 0100 0000 1100 T.
	0x2223, // 0010 0010 0010 0011 U.
	0x2091, // 0010 0000 1001 0001 V.
	0x2833, // 0010 1000 0011 0011 W.
	0x08D0, // 0000 1000 1101 0000 X.
	0x04C0, // 0000 0100 1100 0000 Y.
	0x0294, // 0000 0010 1001 0100 Z.
	0x2205, // 0010 0010 0000 0101 [.
	0x0408, // 0000 0100 0000 1000 |
	0x0226, // 0000 0010 0010 0110 ].
	0x0810, // 0000 1000 0001 0000 ~.
	0x0000, // 0000 0000 0000 0000 (DEL)
};

/**
 * Character data as used in the VFX family. Bit 0..13 are ordered to match
 * led14seg. Bit 14 activates the dot after a character; bit 15 the underline.
 * See layout/esq2by40_vfx.lay .
 */
static const uint16_t font_vfx[] = {
  0x0000, // 0000 0000 0000 0000 SPACE
  0x543f, // 0101 0100 0011 1111 '0.'
  0x0120, // 0000 0001 0010 0000 '"'
  0x4300, // 0100 0011 0000 0000 '1.'
  0x03ed, // 0000 0011 1110 1101 '$'
  0x40db, // 0100 0000 1101 1011 '2.'
  0x0000, // 0000 0000 0000 0000 '&'
  0x0800, // 0000 1000 0000 0000 '''
  0x40cf, // 0100 0000 1100 1111 '3.'
  0x40e6, // 0100 0000 1110 0110 '4.'
  0x3fc0, // 0011 1111 1100 0000 '*'
  0x03c0, // 0000 0011 1100 0000 '+'
  0x0000, // 0000 0000 0000 0000 ','
  0x00c0, // 0000 0000 1100 0000 '-'
  0x4000, // 0100 0000 0000 0000 '.'
  0x1400, // 0001 0100 0000 0000 '/'
  0x143f, // 0001 0100 0011 1111 '0'
  0x0300, // 0000 0011 0000 0000 '1'
  0x00db, // 0000 0000 1101 1011 '2'
  0x00cf, // 0000 0000 1100 1111 '3'
  0x00e6, // 0000 0000 1110 0110 '4'
  0x00ed, // 0000 0000 1110 1101 '5'
  0x00fd, // 0000 0000 1111 1101 '6'
  0x0007, // 0000 0000 0000 0111 '7'
  0x00ff, // 0000 0000 1111 1111 '8'
  0x00ef, // 0000 0000 1110 1111 '9'
  0x0000, // 0000 0000 0000 0000 ':'
  0x40fd, // 0100 0000 1111 1101 '6.'
  0x3000, // 0011 0000 0000 0000 '('
  0x00c8, // 0000 0000 1100 1000 '='
  0x0c00, // 0000 1100 0000 0000 ')'
  0x0000, // 0000 0000 0000 0000 '?'
  0x025f, // 0000 0010 0101 1111 '@'
  0x00f7, // 0000 0000 1111 0111 'A'
  0x038f, // 0000 0011 1000 1111 'B'
  0x0039, // 0000 0000 0011 1001 'C'
  0x030f, // 0000 0011 0000 1111 'D'
  0x00f9, // 0000 0000 1111 1001 'E'
  0x00f1, // 0000 0000 1111 0001 'F'
  0x00bd, // 0000 0000 1011 1101 'G'
  0x00f6, // 0000 0000 1111 0110 'H'
  0x0309, // 0000 0011 0000 1001 'I'
  0x001e, // 0000 0000 0001 1110 'J'
  0x3070, // 0011 0000 0111 0000 'K'
  0x0038, // 0000 0000 0011 1000 'L'
  0x1836, // 0001 1000 0011 0110 'M'
  0x2836, // 0010 1000 0011 0110 'N'
  0x003f, // 0000 0000 0011 1111 'O'
  0x00f3, // 0000 0000 1111 0011 'P'
  0x203f, // 0010 0000 0011 1111 'Q'
  0x20f3, // 0010 0000 1111 0011 'R'
  0x00ed, // 0000 0000 1110 1101 'S'
  0x0301, // 0000 0011 0000 0001 'T'
  0x003e, // 0000 0000 0011 1110 'U'
  0x1430, // 0001 0100 0011 0000 'V'
  0x2436, // 0010 0100 0011 0110 'W'
  0x3c00, // 0011 1100 0000 0000 'X'
  0x1a00, // 0001 1010 0000 0000 'Y'
  0x1409, // 0001 0100 0000 1001 'Z'
  0x0039, // 0000 0000 0011 1001 '['
  0x40ff, // 0100 0000 1111 1111 '8.'
  0x000f, // 0000 0000 0000 1111 ']'
  0x2400, // 0010 0100 0000 0000 '^'
  0x0008, // 0000 0000 0000 1000 '_'
  0x0800, // 0000 1000 0000 0000 '`'
  0x00f7, // 0000 0000 1111 0111 'a'
  0x038f, // 0000 0011 1000 1111 'b'
  0x0039, // 0000 0000 0011 1001 'c'
  0x030f, // 0000 0011 0000 1111 'd'
  0x00f9, // 0000 0000 1111 1001 'e'
  0x00f1, // 0000 0000 1111 0001 'f'
  0x00bd, // 0000 0000 1011 1101 'g'
  0x00f6, // 0000 0000 1111 0110 'h'
  0x0309, // 0000 0011 0000 1001 'i'
  0x001e, // 0000 0000 0001 1110 'j'
  0x3070, // 0011 0000 0111 0000 'k'
  0x0038, // 0000 0000 0011 1000 'l'
  0x1836, // 0001 1000 0011 0110 'm'
  0x2836, // 0010 1000 0011 0110 'n'
  0x003f, // 0000 0000 0011 1111 'o'
  0x00f3, // 0000 0000 1111 0011 'p'
  0x203f, // 0010 0000 0011 1111 'q'
  0x20f3, // 0010 0000 1111 0011 'r'
  0x00ed, // 0000 0000 1110 1101 's'
  0x0301, // 0000 0011 0000 0001 't'
  0x003e, // 0000 0000 0011 1110 'u'
  0x1430, // 0001 0100 0011 0000 'v'
  0x2436, // 0010 0100 0011 0110 'w'
  0x3c00, // 0011 1100 0000 0000 'x'
  0x1a00, // 0001 1010 0000 0000 'y'
  0x1409, // 0001 0100 0000 1001 'z'
  0x0039, // 0000 0000 0011 1001 '{'
  0x0300, // 0000 0011 0000 0000 '|'
  0x000f, // 0000 0000 0000 1111 '}'
  0x2400, // 0010 0100 0000 0000 '~'
  0x0000, // 0000 0000 0000 0000 DEL
};

esqvfd_device::esqvfd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, dimensions_param &&dimensions) :
	device_t(mconfig, type, tag, owner, clock),
	m_vfds(std::move(std::get<0>(dimensions))),
	m_rows(std::get<1>(dimensions)),
	m_cols(std::get<2>(dimensions))
{
}

void esqvfd_device::device_start()
{
	m_vfds->resolve();
}

void esqvfd_device::device_reset()
{
	m_cursx = m_cursy = 0;
	m_savedx = m_savedy = 0;
	m_curattr = AT_NORMAL;
	m_lastchar = 0;
	memset(m_chars, 0, sizeof(m_chars));
	memset(m_attrs, 0, sizeof(m_attrs));
	memset(m_dirty, 1, sizeof(m_attrs));
}

// generic display update; can override from child classes if not good enough
void esqvfd_device::update_display()
{
	for (int row = 0; row < m_rows; row++)
	{
		for (int col = 0; col < m_cols; col++)
		{
			if (m_dirty[row][col])
			{
				uint32_t segdata = conv_segments(font[m_chars[row][col]]);

				// digits:
				m_vfds->set((row * m_cols) + col, segdata);

				// underlines:
				m_vfds->set((row * m_cols) + col + (m_rows * m_cols), (m_attrs[row][col] & AT_UNDERLINE) ? 1 : 0);

				m_dirty[row][col] = 0;
			}
		}
	}
}

/* 2x40 VFD display used in the ESQ-1, VFX-SD, SD-1, and others */

void esq2x40_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_esq2by40);
}

void esq2x40_device::write_char(uint8_t data)
{
	// ESQ-1 sends (cursor move) 0xfa 0xYY to mark YY characters as underlined at the current cursor location
	if (m_lastchar == 0xfa)
	{
		for (uint8_t j = 0; j < m_rows; j++)
		{
			for (uint8_t i = 0; i < m_cols; i++)
			{
				if (m_cursy == j && i >= m_cursx && i < m_cursx + data)
					m_attrs[j][i] |= AT_UNDERLINE;
				else
					m_attrs[j][i] &= ~AT_UNDERLINE;

				m_dirty[j][i] = 1;
			}
		}

		m_lastchar = 0;
		update_display();
		return;
	}

	m_lastchar = data;

	if ((data >= 0x80) && (data < 0xd0))
	{
		m_cursy = ((data & 0x7f) >= 40) ? 1 : 0;
		m_cursx = (data & 0x7f) % 40;
	}
	else if (data >= 0xd0)
	{
		switch (data)
		{
			case 0xd0:  // blink start
				m_curattr |= AT_BLINK;
				break;

			case 0xd1:  // blink stop (cancel all attribs on VFX+)
				m_curattr = 0; //&= ~AT_BLINK;
				break;

			case 0xd2:  // blinking underline on VFX
				m_curattr |= AT_BLINK | AT_UNDERLINE;
				break;

			case 0xd3:  // start underline
				m_curattr |= AT_UNDERLINE;
				break;

			case 0xd6:  // clear screen
				m_cursx = m_cursy = 0;
				memset(m_chars, 0, sizeof(m_chars));
				memset(m_attrs, 0, sizeof(m_attrs));
				memset(m_dirty, 1, sizeof(m_dirty));
				break;

			case 0xf5:  // save cursor position
				m_savedx = m_cursx;
				m_savedy = m_cursy;
				break;

			case 0xf6:  // restore cursor position
				m_cursx = m_savedx;
				m_cursy = m_savedy;
				m_curattr = m_attrs[m_cursy][m_cursx];
				break;

			default:
//                printf("Unknown control code %02x\n", data);
				break;
		}
	}
	else
	{
		if ((data >= 0x20) && (data <= 0x5f))
		{
			m_chars[m_cursy][m_cursx] = data - ' ';
			m_attrs[m_cursy][m_cursx] = m_curattr;
			m_dirty[m_cursy][m_cursx] = 1;
			m_cursx++;

			if (m_cursx >= 39)
			{
				m_cursx = 39;
			}
		}
	}

	update_display();
}

esq2x40_device::esq2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqvfd_device(mconfig, ESQ2X40, tag, owner, clock, make_dimensions<2, 40>(*this))
{
}

/*
 * VFX-family specifics:
 * This device leavs the handling of commands to the esqpanel subclass that uses this vfd,
 * and instead simply focuses on storing and displaying the characters with their attributes.
 * 
 */ 

esq2x40_vfx_device::esq2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqvfd_device(mconfig, ESQ2X40_VFX, tag, owner, clock, make_dimensions<2, 40>(*this))
{
}

void esq2x40_vfx_device::set_char(uint8_t row, uint8_t column, uint8_t c, uint8_t attr) {
	m_chars[row][column] = (' ' <= c && c < 128) ? c - ' ' : 0;
	m_attrs[row][column] = attr;
	m_dirty[row][column] = true;

	update_display();
}

void esq2x40_vfx_device::clear() {
	memset(m_chars, 0, sizeof(m_chars));
	memset(m_attrs, 0, sizeof(m_attrs));
	memset(m_dirty, 1, sizeof(m_dirty));

	update_display();
}

void esq2x40_vfx_device::update_display()
{
	for (int row = 0; row < m_rows; row++)
	{
		for (int col = 0; col < m_cols; col++)
		{
			if (m_dirty[row][col])
			{
				uint32_t char_segments = font_vfx[m_chars[row][col]];
				auto attr = m_attrs[row][col];
				uint32_t segments;

				if ((attr & AT_BLINK) && !m_blink_on)  // something is blinked off
				{
					if (attr & AT_UNDERLINE) // blink the underline off
						segments = char_segments;
					else // there is no underline, blink the entire character
						segments = 0;
				}
				else
				{
					if (attr & AT_UNDERLINE) 
						segments = char_segments | 0x8000;
					else
						segments = char_segments;
				}

				m_vfds->set((row * m_cols) + col, segments);

				m_dirty[row][col] = 0;
			}
		}
	}
}

void esq2x40_vfx_device::device_add_mconfig(machine_config &config)
{
	// Do not set a default layout. This display must be used
	// within a layout that includes the VFD elements, such as
	// vfx.lay, vfxsd.lay or sd1.lay.
}

void esq2x40_vfx_device::set_blink_on(bool blink_on) {
	m_blink_on = blink_on;

	for (int row = 0; row < m_rows; row++)
	{
		for (int col = 0; col < m_cols; col++)
		{
			m_dirty[row][col] |= m_attrs[row][col] & AT_BLINK;
		}
	}
	update_display();
}

/* 1x22 display from the VFX (not right, but it'll do for now) */

void esq1x22_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_esq1by22);
}

void esq1x22_device::write_char(uint8_t data)
{
	if (data >= 0x60)
	{
		switch (data)
		{
			case 'f':   // clear screen
				m_cursx = m_cursy = 0;
				memset(m_chars, 0, sizeof(m_chars));
				memset(m_attrs, 0, sizeof(m_attrs));
				memset(m_dirty, 1, sizeof(m_dirty));
				break;

			default:
				printf("Unhandled control code %02x\n", data);
				break;
		}
	}
	else
	{
		if ((data >= 0x20) && (data <= 0x5f))
		{
			m_chars[0][m_cursx] = data - ' ';
			m_dirty[0][m_cursx] = 1;
			m_cursx++;

			if (m_cursx >= 23)
			{
				m_cursx = 23;
			}
		}
	}

	update_display();
}

esq1x22_device::esq1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqvfd_device(mconfig, ESQ1X22, tag, owner, clock, make_dimensions<1, 22>(*this))
{
}

/* SQ-1 display, I think it's really an LCD but we'll deal with it for now */
void esq2x40_sq1_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_esq2by40);  // we use the normal 2x40 layout
}

void esq2x40_sq1_device::write_char(uint8_t data)
{
	if (data == 0x09)   // musical note
	{
		data = '^'; // approximate for now
	}

	if (m_wait87shift)
	{
		m_cursy = (data >> 4) & 0xf;
		m_cursx = data & 0xf;
		m_wait87shift = false;
	}
	else if (m_wait88shift)
	{
		m_wait88shift = false;
	}
	else if ((data >= 0x20) && (data <= 0x7f))
	{
		m_chars[m_cursy][m_cursx] = data - ' ';
		m_attrs[m_cursy][m_cursx] = m_curattr;
		m_dirty[m_cursy][m_cursx] = 1;
		m_cursx++;

		if (m_cursx >= 39)
		{
			m_cursx = 39;
		}

		update_display();
	}
	else if (data == 0x83)
	{
		m_cursx = m_cursy = 0;
		memset(m_chars, 0, sizeof(m_chars));
		memset(m_attrs, 0, sizeof(m_attrs));
		memset(m_dirty, 1, sizeof(m_dirty));
	}
	else if (data == 0x87)
	{
		m_wait87shift = true;
	}
	else if (data == 0x88)
	{
		m_wait88shift = true;
	}
	else
	{
//        printf("SQ-1 unhandled display char %02x\n", data);
	}
}

esq2x40_sq1_device::esq2x40_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqvfd_device(mconfig, ESQ2X40_SQ1, tag, owner, clock, make_dimensions<2, 40>(*this))
{
	m_wait87shift = false;
	m_wait88shift = false;
}
