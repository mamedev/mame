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

void esqvfd_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
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

				// force bottom bar on all underlined chars
				if (m_attrs[row][col] & AT_UNDERLINE)
					segdata |= 0x0008;

				m_vfds->set((row * m_cols) + col, segdata);

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

void esq2x40_device::write_char(int data)
{
	// ESQ-1 sends (cursor move) 0xfa 0xYY to mark YY characters as underlined at the current cursor location
	if (m_lastchar == 0xfa)
	{
		if ((m_cursx + data) > m_rows)
			data = m_rows - m_cursx;
		for (int i = 0; i < data; i++)
		{
			m_attrs[m_cursy][m_cursx + i] |= AT_UNDERLINE;
			m_dirty[m_cursy][m_cursx + i] = 1;
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

bool esq2x40_device::write_contents(std::ostream &o)
{
	o.put((char) 0xd6); // clear screen

	uint8_t attrs = 0;
	for (int row = 0; row < 2; row++)
	{
		o.put((char) (0x80 + (40 * row))); // move to first column this row

		for (int col = 0; col < 40; col++)
		{
			if (m_attrs[row][col] != attrs)
			{
				attrs = m_attrs[row][col];

				o.put((char) 0xd1); // all attributes off

				if (attrs & AT_BLINK)
				{
					o.put((char) 0xd0); // blink on
				}

				if (attrs & AT_UNDERLINE)
				{
					o.put((char) 0xd3); // underline
				}
			}

			o.put((char) (m_chars[row][col] + ' '));
		}
	}
	return true;
}


esq2x40_device::esq2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	esqvfd_device(mconfig, ESQ2X40, tag, owner, clock, make_dimensions<2, 40>(*this))
{
}

/* 1x22 display from the VFX (not right, but it'll do for now) */

void esq1x22_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_esq1by22);
}


void esq1x22_device::write_char(int data)
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

void esq2x40_sq1_device::write_char(int data)
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
