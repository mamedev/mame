// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Orao machine driver by Miodrag Milanovic

        23/02/2008 Sound support added.
        22/02/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "imagedev/cassette.h"
#include "includes/orao.h"


/* Driver initialization */
DRIVER_INIT_MEMBER(orao_state,orao)
{
	memset(m_memory,0xff,0x6000);
}

DRIVER_INIT_MEMBER(orao_state,orao103)
{
	memset(m_memory,0xff,0x6000);
}

void orao_state::machine_reset()
{
}

READ8_MEMBER(orao_state::orao_io_r)
{
	double level;

		switch(offset) {
		/* Keyboard*/
		case 0x07FC : return ioport("LINE0")->read();
		case 0x07FD : return ioport("LINE1")->read();
		case 0x07FA : return ioport("LINE2")->read();
		case 0x07FB : return ioport("LINE3")->read();
		case 0x07F6 : return ioport("LINE4")->read();
		case 0x07F7 : return ioport("LINE5")->read();
		case 0x07EE : return ioport("LINE6")->read();
		case 0x07EF : return ioport("LINE7")->read();
		case 0x07DE : return ioport("LINE8")->read();
		case 0x07DF : return ioport("LINE9")->read();
		case 0x07BE : return ioport("LINE10")->read();
		case 0x07BF : return ioport("LINE11")->read();
		case 0x077E : return ioport("LINE12")->read();
		case 0x077F : return ioport("LINE13")->read();
		case 0x06FE : return ioport("LINE14")->read();
		case 0x06FF : return ioport("LINE15")->read();
		case 0x05FE : return ioport("LINE16")->read();
		case 0x05FF : return ioport("LINE17")->read();
		case 0x03FE : return ioport("LINE18")->read();
		case 0x03FF : return ioport("LINE19")->read();
		/* Tape */
		case 0x07FF :
					level = m_cassette->input();
					if (level <  0) {
						return 0x00;
					}
					return 0xff;
		}


		return 0xff;
}


WRITE8_MEMBER(orao_state::orao_io_w)
{
	if (offset == 0x0800)
	{
		m_dac->write_unsigned8(data); //beeper
	}
}
