// license:BSD-3-Clause
// copyright-holders:James Wallace
/**********************************************************************

    Rockwell 10937/10957 interface and emulation by J.Wallace
    OKI MSC1937 is a clone of this chip, used in many displays

**********************************************************************/

#include "emu.h"
#include "roc10937.h"

#include <algorithm>


/*
   Rockwell 10937 16 segment charset lookup table (from MCU)
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
16-bit tables are used to hold the main characters in the MCU memory,
the other characters come in separate to this lookup.
*/

static const uint16_t roc10937charset[]=
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

rocvfd_device::rocvfd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_outputs(),
	m_cursor_pos(0),
	m_port_val(0)
{
}


void rocvfd_device::device_start()
{
	m_outputs = std::make_unique<output_finder<16> >(*this, "vfd%u", unsigned(m_port_val * 16));
	m_outputs->resolve();

	m_brightness = std::make_unique<output_finder<1> >(*this, "vfdduty%u", unsigned(m_port_val));
	m_brightness->resolve();

	m_sclk = 0;
	m_data = 0;
	m_por = 0;

	save_item(NAME(m_cursor_pos));
	save_item(NAME(m_window_size));
	save_item(NAME(m_shift_count));
	save_item(NAME(m_shift_data));
	save_item(NAME(m_pcursor_pos));
	save_item(NAME(m_chars));
	save_item(NAME(m_count));
	save_item(NAME(m_sclk));
	save_item(NAME(m_data));
	save_item(NAME(m_por));
	save_item(NAME(m_duty));

	std::fill(std::begin(m_chars), std::end(m_chars), 0);
	std::fill(std::begin(*m_outputs), std::end(*m_outputs), 0);

}

void rocvfd_device::device_reset()
{
	//We don't clear the buffers on reset as JPM games rely on the buffers being intact after POR
	//On real hardware, garbage patterns can appear unless specifically cleared, so this makes sense.
	m_cursor_pos = 0;
	m_window_size = 16;
	m_shift_count = 0;
	m_shift_data = 0;
	m_pcursor_pos = 0;
	m_count = 0;
	m_duty = 0;

	(*m_brightness)[0] = 0;
}

///////////////////////////////////////////////////////////////////////////
uint32_t rocvfd_device::set_display(uint32_t segin)
{
	return bitswap<32>(segin, 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,11,9,15,13,12,8,10,14,7,6,5,4,3,2,1,0);
}

///////////////////////////////////////////////////////////////////////////
void rocvfd_device::device_post_load()
{
	update_display();
}

void rocvfd_device::update_display()
{
	std::transform(std::begin(m_chars), std::end(m_chars), std::begin(*m_outputs), set_display);
	(*m_brightness)[0] = m_duty;
}

void rocvfd_device::sclk(int state)
{
	shift_clock(state);
}

void rocvfd_device::data(int state)
{
	if (state)
	{
		m_data = 1;
	}
	else
	{
		m_data = 0;
	}
}

void rocvfd_device::por(int state)
{
	//If line goes low, reset mode is engaged, until such a time as it goes high again.
	if (!state)
	{
		reset();
	}
	m_por = state;
}


void rocvfd_device::shift_clock(int state)
{
	if (m_sclk != state)
	{
		//Clock data on FALLING edge
		if (!m_sclk && m_por)
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
		m_sclk = state;
	}
}

///////////////////////////////////////////////////////////////////////////
DEFINE_DEVICE_TYPE(ROC10937, roc10937_device, "roc10937", "Rockwell 10937 VFD controller") // and compatible
DEFINE_DEVICE_TYPE(MSC1937,  msc1937_device,  "msc1937",  "OKI MSC1937 VFD controller")
DEFINE_DEVICE_TYPE(MIC10937, mic10937_device, "mic10937", "Micrel MIC10937 VFD controller")
DEFINE_DEVICE_TYPE(ROC10957, roc10957_device, "roc10957", "Rockwell 10957 VFD controller") // and compatible
DEFINE_DEVICE_TYPE(S16LF01,  s16lf01_device,  "s16lf01",  "Samsung 16LF01 Series VFD") // and compatible, basically the MSC1937 on a 16 seg display

roc10937_device::roc10937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rocvfd_device(mconfig, ROC10937, tag, owner, clock)
{
}

msc1937_device::msc1937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rocvfd_device(mconfig, MSC1937, tag, owner, clock)
{
}

mic10937_device::mic10937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rocvfd_device(mconfig, MIC10937, tag, owner, clock)
{
}

s16lf01_device::s16lf01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rocvfd_device(mconfig, S16LF01, tag, owner, clock)
{
}

void rocvfd_device::write_char(int data)
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
		{ // 111x xxxx Set duty cycle ( power to display )
			m_duty = (data & 0x1F);
		}
		else if ( (data & 0xE0) == 0x80 ) // 100x ---
		{ // 100x xxxx Test mode
			popmessage("TEST MODE ENABLED!");
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

roc10957_device::roc10957_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rocvfd_device(mconfig, ROC10957, tag, owner, clock)
{
}

void roc10957_device::write_char(int data)
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
		{ // 111x xxxx Set duty cycle ( power to display )
			m_duty = (data & 0x1F);
		}
		else if ( (data & 0xE0) == 0x80 ) // 100x ---
		{ // 100x xxxx Test mode
			popmessage("TEST MODE ENABLED!");
			m_duty = 4;
		}
	}
	else
	{ // Display data.  Bit 6 is a "don't care" bit except for PNT and TAIL.
		data &= 0x7F;

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
			m_chars[m_pcursor_pos] |= (1<<17);//,
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
