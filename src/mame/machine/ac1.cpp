// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        AC1 video driver by Miodrag Milanovic

        15/01/2009 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "includes/ac1.h"

READ8_MEMBER(ac1_state::ac1_port_b_r)
{
	UINT8 data = 0x7f;

	if (m_cassette->input() > 0.03)
		data |= 0x80;

	return data;
}

#define BNOT(x) ((x) ? 0 : 1)

READ8_MEMBER(ac1_state::ac1_port_a_r)
{
	UINT8 line0 = m_io_line[0]->read();
	UINT8 line1 = m_io_line[1]->read();
	UINT8 line2 = m_io_line[2]->read();
	UINT8 line3 = m_io_line[3]->read();
	UINT8 line4 = m_io_line[4]->read();
	UINT8 line5 = m_io_line[5]->read();
	UINT8 line6 = m_io_line[6]->read();

	UINT8 SH    = BNOT(BIT(line6,0));
	UINT8 CTRL  = BNOT(BIT(line6,1));
	UINT8 SPACE = BIT(line6,2);
	UINT8 ENTER = BIT(line6,3);
	UINT8 BACK  = BIT(line6,4);

	UINT8 all = line0 | line1 | line2 | line3 | line4 | line5;
	UINT8 s1 = BNOT(BIT(all,0));UINT8 z1 = (line0 !=0) ? 0 : 1;
	UINT8 s2 = BNOT(BIT(all,1));UINT8 z2 = (line1 !=0) ? 0 : 1;
	UINT8 s3 = BNOT(BIT(all,2));UINT8 z3 = (line2 !=0) ? 0 : 1;
	UINT8 s4 = BNOT(BIT(all,3));UINT8 z4 = (line3 !=0) ? 0 : 1;
	UINT8 s5 = BNOT(BIT(all,4));UINT8 z5 = (line4 !=0) ? 0 : 1;
	UINT8 s6 = BNOT(BIT(all,5));UINT8 z6 = (line5 !=0) ? 0 : 1;
	UINT8 s7 = BNOT(BIT(all,6));
	UINT8 s8 = BNOT(BIT(all,7));
	UINT8 tast,td0,td1,td2,td3,td4,td5,td6,dg5;

	/* Additional double keys */
	if (SPACE) {
		z1 = 0; s1 = 0; SH = 0;
	}
	if (ENTER) {
		z4 = 0; s6 = 0; CTRL = 0;
	}
	if (BACK) {
		z4 = 0; s1 = 0; CTRL = 0;
	}

	tast = BNOT(s1 & s2 & s3 & s4 & s5 &s6 & s7 & s8);
	td0  = BNOT(s2 & s4 & s6 & s8);
	td1  = BNOT(s3 & s4 & s7 & s8);
	td2  = BNOT(s5 & s6 & s7 & s8);
	td3  = BNOT(z2 & z4 & z6);
	td4  = BNOT(BNOT(BNOT(z1 & z2) & SH) & z5 & z6);
	dg5  = BNOT(z3 & z4 & z5 & z6);
	td5  = BNOT(BNOT(dg5 & BNOT(SH)) & z1 & z2);
	td6  = (dg5 & CTRL);
	return td0 + (td1 << 1) +(td2 << 2) +(td3 << 3) +(td4 << 4) +(td5 << 5) +(td6 << 6) +(tast << 7);
}

WRITE8_MEMBER(ac1_state::ac1_port_a_w)
{
}

WRITE8_MEMBER(ac1_state::ac1_port_b_w)
{
	/*

	    bit     description

	    0
	    1       RTTY receive
	    2       RTTY transmit
	    3       RTTY PTT
	    4
	    5
	    6       cassette out
	    7       cassette in

	*/
	m_cassette->output((data & 0x40) ? -1.0 : +1.0);
}

/* Driver initialization */
DRIVER_INIT_MEMBER(ac1_state,ac1)
{
}

void ac1_state::machine_reset()
{
}
