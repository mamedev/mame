/***************************************************************************

        AC1 video driver by Miodrag Milanovic

        15/01/2009 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "includes/ac1.h"

static READ8_DEVICE_HANDLER (ac1_port_b_r)
{
	ac1_state *state = space.machine().driver_data<ac1_state>();
	UINT8 data = 0x7f;

	if (state->m_cassette->input() > 0.03)
		data |= 0x80;

	return data;
}

#define BNOT(x) ((x) ? 0 : 1)

static READ8_DEVICE_HANDLER (ac1_port_a_r)
{
	UINT8 line0 = space.machine().root_device().ioport("LINE0")->read();
	UINT8 line1 = space.machine().root_device().ioport("LINE1")->read();
	UINT8 line2 = space.machine().root_device().ioport("LINE2")->read();
	UINT8 line3 = space.machine().root_device().ioport("LINE3")->read();
	UINT8 line4 = space.machine().root_device().ioport("LINE4")->read();
	UINT8 line5 = space.machine().root_device().ioport("LINE5")->read();
	UINT8 line6 = space.machine().root_device().ioport("LINE6")->read();

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

static WRITE8_DEVICE_HANDLER (ac1_port_a_w)
{
}

static WRITE8_DEVICE_HANDLER (ac1_port_b_w)
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
	ac1_state *state = space.machine().driver_data<ac1_state>();
	state->m_cassette->output((data & 0x40) ? -1.0 : +1.0);
}

Z80PIO_INTERFACE( ac1_z80pio_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_HANDLER(ac1_port_a_r),
	DEVCB_HANDLER(ac1_port_a_w),
	DEVCB_NULL,
	DEVCB_HANDLER(ac1_port_b_r),
	DEVCB_HANDLER(ac1_port_b_w),
	DEVCB_NULL
};

/* Driver initialization */
DRIVER_INIT_MEMBER(ac1_state,ac1)
{
}

void ac1_state::machine_reset()
{
	m_cassette = machine().device<cassette_image_device>(CASSETTE_TAG);
}
