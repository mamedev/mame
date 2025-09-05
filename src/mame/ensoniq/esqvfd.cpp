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
#include "nec_fip80b5r.lh"

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
 * The font used by the VFX family on the NEC FIP80B5R display, including 
 * VFX-family-specific characters sich as digits followed by a period / decimal point.
 * Arranged to match this, fairly arbitrarily chosen, order of segments:
 * 
 *     ---- 0 ----
 *    |\    |    /|
 *    | \   |   / |
 *    5  4  3  2  1
 *    |   \ | /   |
 *    |    \|/    |
 *     --7-- --6--
 *    |    /|\    |
 *    |   / | \   |
 *   12 11 10  9  8
 *    | /   |   \ |
 *    |/    |    \|
 *     ----13-----  *14
 * 
 *    -----15-----
 * 
 */
static const uint16_t font_vfx[] = {
	0x0000, //  0000 0000 0000 0000 SPACE
	0x7927, //  0011 1001 0010 0111 '0.'
	0x0028, //  0000 0000 0010 1000 '"'
	0x4408, //  0000 0100 0000 1000 '1.'
	0x25e9, //  0010 0101 1110 1001 '$'
	0x70c3, //  0011 0000 1100 0011 '2.'
	0x0000, //  0000 0000 0000 0000 '&'
	0x0010, //  0000 0000 0001 0000 '''
	0x61c3, //  0010 0001 1100 0011 '3.'
	0x41e2, //  0000 0001 1110 0010 '4.'
	0x0edc, //  0000 1110 1101 1100 '*'
	0x04c8, //  0000 0100 1100 1000 '+'
	0x0000, //  0000 0000 0000 0000 ','
	0x00c0, //  0000 0000 1100 0000 '-'
	0x4000, //  0100 0000 0000 0000 '.'
	0x0804, //  0000 1000 0000 0100 '/'
	0x3927, //  0011 1001 0010 0111 '0'
	0x0408, //  0000 0100 0000 1000 '1'
	0x30c3, //  0011 0000 1100 0011 '2'
	0x21c3, //  0010 0001 1100 0011 '3'
	0x01e2, //  0000 0001 1110 0010 '4'
	0x21e1, //  0010 0001 1110 0001 '5'
	0x31e1, //  0011 0001 1110 0001 '6'
	0x0103, //  0000 0001 0000 0011 '7'
	0x31e3, //  0011 0001 1110 0011 '8'
	0x21e3, //  0010 0001 1110 0011 '9'
	0x0000, //  0000 0000 0000 0000 ':'
	0x71e1, //  0011 0001 1110 0001 '6.'
	0x0204, //  0000 0010 0000 0100 '('
	0x20c0, //  0010 0000 1100 0000 '='
	0x0810, //  0000 1000 0001 0000 ')'
	0x0000, //  0000 0000 0000 0000 '?'
	0x3583, //  0011 0101 1000 0011 '@'
	0x11e3, //  0001 0001 1110 0011 'A'
	0x254b, //  0010 0101 0100 1011 'B'
	0x3021, //  0011 0000 0010 0001 'C'
	0x250b, //  0010 0101 0000 1011 'D'
	0x30e1, //  0011 0000 1110 0001 'E'
	0x10e1, //  0001 0000 1110 0001 'F'
	0x3161, //  0011 0001 0110 0001 'G'
	0x11e2, //  0001 0001 1110 0010 'H'
	0x2409, //  0010 0100 0000 1001 'I'
	0x3102, //  0011 0001 0000 0010 'J'
	0x12a4, //  0001 0010 1010 0100 'K'
	0x3020, //  0011 0000 0010 0000 'L'
	0x1136, //  0001 0001 0011 0110 'M'
	0x1332, //  0001 0011 0011 0010 'N'
	0x3123, //  0011 0001 0010 0011 'O'
	0x10e3, //  0001 0000 1110 0011 'P'
	0x3323, //  0011 0011 0010 0011 'Q'
	0x12e3, //  0001 0010 1110 0011 'R'
	0x21e1, //  0010 0001 1110 0001 'S'
	0x0409, //  0000 0100 0000 1001 'T'
	0x3122, //  0011 0001 0010 0010 'U'
	0x1824, //  0001 1000 0010 0100 'V'
	0x1b22, //  0001 1011 0010 0010 'W'
	0x0a14, //  0000 1010 0001 0100 'X'
	0x0414, //  0000 0100 0001 0100 'Y'
	0x2805, //  0010 1000 0000 0101 'Z'
	0x3021, //  0011 0000 0010 0001 '['
	0x71e3, //  0011 0001 1110 0011 '8.'
	0x2103, //  0010 0001 0000 0011 ']'
	0x0a00, //  0000 1010 0000 0000 '^'
	0x2000, //  0010 0000 0000 0000 '_'
	0x0010, //  0000 0000 0001 0000 '`'
	0x11e3, //  0001 0001 1110 0011 'a'
	0x254b, //  0010 0101 0100 1011 'b'
	0x3021, //  0011 0000 0010 0001 'c'
	0x250b, //  0010 0101 0000 1011 'd'
	0x30e1, //  0011 0000 1110 0001 'e'
	0x10e1, //  0001 0000 1110 0001 'f'
	0x3161, //  0011 0001 0110 0001 'g'
	0x11e2, //  0001 0001 1110 0010 'h'
	0x2409, //  0010 0100 0000 1001 'i'
	0x3102, //  0011 0001 0000 0010 'j'
	0x12a4, //  0001 0010 1010 0100 'k'
	0x3020, //  0011 0000 0010 0000 'l'
	0x1136, //  0001 0001 0011 0110 'm'
	0x1332, //  0001 0011 0011 0010 'n'
	0x3123, //  0011 0001 0010 0011 'o'
	0x10e3, //  0001 0000 1110 0011 'p'
	0x3323, //  0011 0011 0010 0011 'q'
	0x12e3, //  0001 0010 1110 0011 'r'
	0x21e1, //  0010 0001 1110 0001 's'
	0x0409, //  0000 0100 0000 1001 't'
	0x3122, //  0011 0001 0010 0010 'u'
	0x1824, //  0001 1000 0010 0100 'v'
	0x1b22, //  0001 1011 0010 0010 'w'
	0x0a14, //  0000 1010 0001 0100 'x'
	0x0414, //  0000 0100 0001 0100 'y'
	0x2805, //  0010 1000 0000 0101 'z'
	0x3021, //  0011 0000 0010 0001 '{'
	0x0408, //  0000 0100 0000 1000 '|'
	0x2103, //  0010 0001 0000 0011 '}'
	0x0a00, //  0000 1010 0000 0000 '~'
	0x0000, //  0000 0000 0000 0000 DEL
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
	esqvfd_device(mconfig, ESQ2X40, tag, owner, clock, make_dimensions<2, 40>(*this))
{
}

void esq2x40_vfx_device::set_char(uint8_t row, uint8_t column, uint8_t c, uint8_t attr) {
	m_chars[row][column] = c;
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
				uint32_t segdata = font_vfx[m_chars[row][col]];
				auto attr = m_attrs[row][col];
				auto underline = (attr & AT_UNDERLINE) && (!(attr & AT_BLINK) || m_blink_on);

				// digits:
				m_vfds->set((row * m_cols) + col, segdata | (underline ? 0x8000 : 0));

				m_dirty[row][col] = 0;
			}
		}
	}
}

void esq2x40_vfx_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_nec_fip80b5r);
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
