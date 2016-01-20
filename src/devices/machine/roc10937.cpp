// license:BSD-3-Clause
// copyright-holders:James Wallace
/**********************************************************************

    Rockwell 10937/10957 interface and emulation by J.Wallace
    OKI MSC1937 is a clone of this chip

**********************************************************************/

#include "emu.h"
#include "roc10937.h"

/*
   Rockwell 10937 16 segment charset lookup table
     0     1
    ---- ----
   |\   |   /|
 7 | \F |8 /9| 2
   |  \ | /  |
    -E-- --A-
   |  / | \  |
 6 | /D |C \B| 3
   |/   |   \|
    ---- ----  .11
      5    4   ,10

In 14 segment mode, 0 represents the whole top line,
and 5 the bottom line, allowing both modes to share
a charset.

Note that, although we call this a '16 segment' display,
we actually have 18 segments, including the semicolon portions.
16-bit tables are used to hold the main characters, the rest are OR'd in
*/

static const UINT16 roc10937charset[]=
{           // FEDC BA98 7654 3210
	0x507F, // 0101 0000 0111 1111 @.
	0x44CF, // 0100 0100 1100 1111 A.
	0x153F, // 0001 0101 0011 1111 B.
	0x00F3, // 0000 0000 1111 0011 C.
	0x113F, // 0001 0001 0011 1111 D.
	0x40F3, // 0100 0000 1111 0011 E.
	0x40C3, // 0100 0000 1100 0011 F.
	0x04FB, // 0000 0100 1111 1011 G.
	0x44CC, // 0100 0100 1100 1100 H.
	0x1133, // 0001 0001 0011 0011 I.
	0x007C, // 0000 0000 0111 1100 J.
	0x4AC0, // 0100 1010 1100 0000 K.
	0x00F0, // 0000 0000 1111 0000 L.
	0x82CC, // 1000 0010 1100 1100 M.
	0x88CC, // 1000 1000 1100 1100 N.
	0x00FF, // 0000 0000 1111 1111 O.
	0x44C7, // 0100 0100 1100 0111 P.
	0x08FF, // 0000 1000 1111 1111 Q.
	0x4CC7, // 0100 1100 1100 0111 R.
	0x44BB, // 0100 0100 1011 1011 S.
	0x1103, // 0001 0001 0000 0011 T.
	0x00FC, // 0000 0000 1111 1100 U.
	0x22C0, // 0010 0010 1100 0000 V.
	0x28CC, // 0010 1000 1100 1100 W.
	0xAA00, // 1010 1010 0000 0000 X.
	0x9200, // 1001 0010 0000 0000 Y.
	0x2233, // 0010 0010 0011 0011 Z.
	0x00E1, // 0000 0000 1110 0001 [.
	0x8800, // 1000 1000 0000 0000 \.
	0x001E, // 0000 0000 0001 1110 ].
	0x2800, // 0010 1000 0000 0000 ^.
	0x0030, // 0000 0000 0011 0000 _.
	0x0000, // 0000 0000 0000 0000 dummy.
	0x8121, // 1000 0001 0010 0001 !.
	0x0180, // 0000 0001 1000 0000 ".
	0x553C, // 0101 0101 0011 1100 #.
	0x55BB, // 0101 0101 1011 1011 $.
	0x7799, // 0111 0111 1001 1001 %.
	0xC979, // 1100 1001 0111 1001 &.
	0x0200, // 0000 0010 0000 0000 '.
	0x0A00, // 0000 1010 0000 0000 (.
	0xA050, // 1010 0000 0000 0000 ).
	0xFF00, // 1111 1111 0000 0000 *.
	0x5500, // 0101 0101 0000 0000 +.
	0x0000, // 0000 0000 0000 0000 ;. (Set separately)
	0x4400, // 0100 0100 0000 0000 --.
	0x0000, // 0000 0000 0000 0000 . .(Set separately)
	0x2200, // 0010 0010 0000 0000 /.
	0x22FF, // 0010 0010 1111 1111 0.
	0x1100, // 0001 0001 0000 0000 1.
	0x4477, // 0100 0100 0111 0111 2.
	0x443F, // 0100 0100 0011 1111 3.
	0x448C, // 0100 0100 1000 1100 4.
	0x44BB, // 0100 0100 1011 1011 5.
	0x44FB, // 0100 0100 1111 1011 6.
	0x000F, // 0000 0000 0000 1111 7.
	0x44FF, // 0100 0100 1111 1111 8.
	0x44BF, // 0100 0100 1011 1111 9.
	0x0021, // 0000 0000 0010 0001 -
			//                     -.
	0x2001, // 0010 0000 0000 0001 -
			//                     /.
	0x2230, // 0010 0010 0011 0000 <.
	0x4430, // 0100 0100 0011 0000 =.
	0x8830, // 1000 1000 0011 0000 >.
	0x1407, // 0001 0100 0000 0111 ?.
};


///////////////////////////////////////////////////////////////////////////
static const int roc10937poslut[]=
{
	1,//0
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	0//15
};

const device_type ROC10937 = &device_creator<roc10937_t>;

rocvfd_t::rocvfd_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
	m_port_val=0;
}


void rocvfd_t::static_set_value(device_t &device, int val)
{
	rocvfd_t &roc = downcast<rocvfd_t &>(device);
	roc.m_port_val = val;
}

void rocvfd_t::device_start()
{
	save_item(NAME(m_port_val));
	save_item(NAME(m_cursor_pos));
	save_item(NAME(m_window_size));
	save_item(NAME(m_shift_count));
	save_item(NAME(m_shift_data));
	save_item(NAME(m_pcursor_pos));
	save_item(NAME(m_chars));
	save_item(NAME(m_outputs));
	save_item(NAME(m_brightness));
	save_item(NAME(m_count));
	save_item(NAME(m_sclk));
	save_item(NAME(m_data));
	save_item(NAME(m_duty));
	save_item(NAME(m_disp));


	device_reset();
}

void rocvfd_t::device_reset()
{
	m_cursor_pos = 0;
	m_window_size = 16;
	m_shift_count = 0;
	m_shift_data = 0;
	m_pcursor_pos = 0;
	m_brightness =31;
	m_count=0;
	m_duty=31;
	m_disp = 0;
	m_sclk = 0;
	m_data = 0;

	memset(m_chars, 0, sizeof(m_chars));
	memset(m_outputs, 0, sizeof(m_outputs));
}

///////////////////////////////////////////////////////////////////////////
UINT32 rocvfd_t::set_display(UINT32 segin)
{
	return BITSWAP32(segin, 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,11,9,15,13,12,8,10,14,7,6,5,4,3,2,1,0);

}

///////////////////////////////////////////////////////////////////////////
void rocvfd_t::device_post_load()
{
	update_display();
}

void rocvfd_t::update_display()
{
	for (int i =0; i<16; i++)
	{
		m_outputs[i] = set_display(m_chars[i]);
		machine().output().set_indexed_value("vfd", (m_port_val*16) + i, m_outputs[i]);
	}
}

WRITE_LINE_MEMBER( rocvfd_t::sclk )
{
	shift_clock(state);
}

WRITE_LINE_MEMBER( rocvfd_t::data )
{
	m_data = state;
}

WRITE_LINE_MEMBER( rocvfd_t::por )
{
	//If line goes low, reset mode is engaged, until such a time as it goes high again.
	if (!state)
	{
		reset();
	}
}


void rocvfd_t::shift_clock(int state)
{
	if (m_sclk != state)
	{
		//Clock data on FALLING edge
		if (!m_sclk)
		{
			m_shift_data <<= 1;

			if ( m_data ) m_shift_data |= 1;

			if ( ++m_shift_count >= 8 )
			{
				write_char(m_shift_data);
				m_shift_count = 0;
				m_shift_data  = 0;
			}
			update_display();

		}
	}
	m_sclk = state;
}

///////////////////////////////////////////////////////////////////////////
roc10937_t::roc10937_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: rocvfd_t(mconfig, ROC10937, "Rockwell 10937 VFD controller and compatible", tag, owner, clock, "roc10937", __FILE__)
{
	m_port_val=0;
}

const device_type MSC1937 = &device_creator<msc1937_t>;

msc1937_t::msc1937_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: rocvfd_t(mconfig, MSC1937, "OKI MSC1937 VFD controller", tag, owner, clock, "msc1937", __FILE__)
{
	m_port_val=0;
}

void rocvfd_t::write_char(int data)
{
	if ( data & 0x80 )
	{ // Control data received
		if ( (data & 0xF0) == 0xA0 ) // 1010 xxxx
		{ // 1 010 xxxx Buffer Pointer control
			m_cursor_pos = roc10937poslut[data & 0x0F];
		}
		else if ( (data & 0xF0) == 0xC0 ) // 1100 xxxx
		{ // 1100 xxxx Set number of digits
			data &= 0x0F;

			if ( data == 0 ) m_window_size = 16;
			else             m_window_size = data;
		}
		else if ( (data & 0xE0) == 0xE0 ) // 111x xxxx
		{ // 111x xxxx Set duty cycle ( brightness )
			m_brightness = (data & 0x1F);
		}
		else if ( (data & 0xE0) == 0x80 ) // 100x ---
		{ // 100x xxxx Test mode
			m_duty =4;
		}
	}
	else
	{ // Display data
//      data &= 0x3F;

		switch ( data )
		{
			case 0x2C: // ;
			m_chars[m_pcursor_pos] |= (1<<16);//.
			m_chars[m_pcursor_pos] |= (1<<17);//,
			break;
			case 0x2E: //
			m_chars[m_pcursor_pos] |= (1<<16);//.
			break;
			default :
			m_pcursor_pos = m_cursor_pos;
			m_chars[m_cursor_pos] = roc10937charset[data & 0x3F];

			m_cursor_pos++;
			if (  m_cursor_pos > (m_window_size -1) )
			{
				m_cursor_pos = 0;
			}
			break;
		}
	}
}

const device_type ROC10957 = &device_creator<roc10957_t>;

roc10957_t::roc10957_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: rocvfd_t(mconfig, ROC10957, "Rockwell 10957 VFD controller and compatible", tag, owner, clock, "roc10957", __FILE__)
{
	m_port_val=0;
}

void roc10957_t::write_char(int data)
{
	if ( data & 0x80 )
	{ // Control data received
		if ( (data & 0xF0) == 0xA0 ) // 1010 xxxx
		{ // 1 010 xxxx Buffer Pointer control
			m_cursor_pos = roc10937poslut[data & 0x0F];
		}
		else if ( (data & 0xF0) == 0xC0 ) // 1100 xxxx
		{ // 1100 xxxx Set number of digits
			data &= 0x0F;

			if ( data == 0 ) m_window_size = 16;
			else             m_window_size = data;
		}
		else if ( (data & 0xE0) == 0xE0 ) // 111x xxxx
		{ // 111x xxxx Set duty cycle ( brightness )
			m_brightness = (data & 0x1F);
		}
		else if ( (data & 0xE0) == 0x80 ) // 100x ---
		{ // 100x xxxx Test mode
			popmessage("TEST MODE ENABLED!");
			m_duty = 4;
		}
	}
	else
	{ // Display data
		data &= 0x3F;

		switch ( data )
		{
			case 0x2C: // ;
			m_chars[m_pcursor_pos] |= (1<<16);//.
			m_chars[m_pcursor_pos] |= (1<<17);//,
			break;
			case 0x2E: //
			m_chars[m_pcursor_pos] |= (1<<16);//.
			break;
			case 0x6C: // ;
			m_chars[m_pcursor_pos] |= (1<<16);//.
			break;
			case 0x6E: //
			{
				m_chars[m_pcursor_pos] = 0;
			}
			break;
			default :
			m_pcursor_pos = m_cursor_pos;
			m_chars[m_cursor_pos] = roc10937charset[data & 0x3F];

			m_cursor_pos++;
			if (  m_cursor_pos > (m_window_size -1) )
			{
				m_cursor_pos = 0;
			}
		break;
		}
	}
}

const device_type S16LF01 = &device_creator<s16lf01_t>;

s16lf01_t::s16lf01_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: rocvfd_t(mconfig, S16LF01, "Samsung 16LF01 Series VFD controller and compatible", tag, owner, clock, "s16lf01", __FILE__)
{
	m_port_val=0;
}
