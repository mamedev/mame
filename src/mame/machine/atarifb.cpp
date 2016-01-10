// license:???
// copyright-holders:Mike Balfour, Patrick Lawrence, Brad Oliver
/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "emu.h"
#include "includes/atarifb.h"
#include "sound/discrete.h"


WRITE8_MEMBER(atarifb_state::atarifb_out1_w)
{
	m_CTRLD = data;

	m_discrete->write(space, ATARIFB_WHISTLE_EN, data & 0x01);     // Whistle
	m_discrete->write(space, ATARIFB_HIT_EN, data & 0x02);         // Hit
	m_discrete->write(space, ATARIFB_ATTRACT_EN, data & 0x10);     // Attract
	m_discrete->write(space, ATARIFB_NOISE_EN, data & 0x04);           // Noise Enable / Kicker
}


WRITE8_MEMBER(atarifb_state::atarifb4_out1_w)
{
	m_CTRLD = data;

	m_discrete->write(space, ATARIFB_WHISTLE_EN, data & 0x01);     // Whistle
	m_discrete->write(space, ATARIFB_HIT_EN, data & 0x02);         // Hit
	m_discrete->write(space, ATARIFB_ATTRACT_EN, data & 0x10);     // Attract
	m_discrete->write(space, ATARIFB_NOISE_EN, data & 0x04);           // Noise Enable / Kicker

	machine().bookkeeping().coin_counter_w(1, data & 0x80);
}


WRITE8_MEMBER(atarifb_state::abaseb_out1_w)
{
	m_CTRLD = data;

	m_discrete->write(space, ATARIFB_WHISTLE_EN, data & 0x01);     // Whistle
	m_discrete->write(space, ATARIFB_HIT_EN, data & 0x02);         // Hit
	m_discrete->write(space, ATARIFB_ATTRACT_EN, data & 0x10);     // Attract
	m_discrete->write(space, ATARIFB_NOISE_EN, data & 0x04);           // Noise Enable / Kicker

	if (data & 0x80)
	{
		/* Invert video */
		m_palette->set_pen_color(1, rgb_t(0x00,0x00,0x00)); /* black  */
		m_palette->set_pen_color(0, rgb_t(0xff,0xff,0xff)); /* white  */
	}
	else
	{
		/* Regular video */
		m_palette->set_pen_color(0, rgb_t(0x00,0x00,0x00)); /* black  */
		m_palette->set_pen_color(1, rgb_t(0xff,0xff,0xff)); /* white  */
	}
}


WRITE8_MEMBER(atarifb_state::soccer_out1_w)
{
	m_CTRLD = data;

	/* bit 0 = whistle */
	/* bit 1 = hit */
	/* bit 2 = kicker */
	/* bit 3 = unused */
	/* bit 4 = 2/4 Player LED */    // Say what?
	/* bit 5-6 = trackball CTRL bits */
	/* bit 7 = Rule LED */

	m_discrete->write(space, ATARIFB_WHISTLE_EN, data & 0x01);     // Whistle
	m_discrete->write(space, ATARIFB_HIT_EN, data & 0x02);         // Hit
	m_discrete->write(space, ATARIFB_ATTRACT_EN, data & 0x10);     // Attract
	m_discrete->write(space, ATARIFB_NOISE_EN, data & 0x04);           // Noise Enable / Kicker

//  output().set_led_value(0, data & 0x10);  // !!!!!!!!!! Is this correct????
	output().set_led_value(1, data & 0x80);
}


WRITE8_MEMBER(atarifb_state::atarifb_out2_w)
{
	m_discrete->write(space, ATARIFB_CROWD_DATA, data & 0x0f); // Crowd

	machine().bookkeeping().coin_counter_w(0, data & 0x10);
}


WRITE8_MEMBER(atarifb_state::soccer_out2_w)
{
	m_discrete->write(space, ATARIFB_CROWD_DATA, data & 0x0f); // Crowd

	machine().bookkeeping().coin_counter_w(0, data & 0x10);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
	machine().bookkeeping().coin_counter_w(2, data & 0x40);
}



/*************************************
 *
 *  LED control
 *
 *************************************/

/* FIXME: almost surely not tied with screen vertical position */
WRITE8_MEMBER(atarifb_state::atarifb_out3_w)
{
	int loop = m_screen->vpos() >= 123;

	switch (loop)
	{
		case 0x00:
			/* Player 1 play select lamp */
			output().set_value("ledleft0", (data >> 0) & 1);
			output().set_value("ledleft1", (data >> 1) & 1);
			output().set_value("ledleft2", (data >> 2) & 1);
			output().set_value("ledleft3", (data >> 3) & 1);
			output().set_value("ledleft4", (data >> 4) & 1);
			break;
		case 0x01:
			/* Player 2 play select lamp */
			output().set_value("ledright0", (data >> 0) & 1);
			output().set_value("ledright1", (data >> 1) & 1);
			output().set_value("ledright2", (data >> 2) & 1);
			output().set_value("ledright3", (data >> 3) & 1);
			output().set_value("ledright4", (data >> 4) & 1);
			break;
	}
//  logerror("out3_w, %02x:%02x\n", loop, data);
}


READ8_MEMBER(atarifb_state::atarifb_in0_r)
{
	if ((m_CTRLD & 0x20) == 0x00)
	{
		int val;

		val = (m_sign_y_2 >> 7) |
				(m_sign_x_2 >> 6) |
				(m_sign_y_1 >> 5) |
				(m_sign_x_1 >> 4) |
				ioport("IN0")->read();
		return val;
	}
	else
	{
		int new_x, new_y;

		/* Read player 1 trackball */
		new_x = ioport("IN3")->read();
		if (new_x != m_counter_x_in0)
		{
			m_sign_x_1 = (new_x - m_counter_x_in0) & 0x80;
			m_counter_x_in0 = new_x;
		}

		new_y = ioport("IN2")->read();
		if (new_y != m_counter_y_in0)
		{
			m_sign_y_1 = (new_y - m_counter_y_in0) & 0x80;
			m_counter_y_in0 = new_y;
		}

		return (((m_counter_y_in0 & 0x0f) << 4) | (m_counter_x_in0 & 0x0f));
	}
}


READ8_MEMBER(atarifb_state::atarifb_in2_r)
{
	if ((m_CTRLD & 0x20) == 0x00)
	{
		return ioport("IN1")->read();
	}
	else
	{
		int new_x, new_y;

		/* Read player 2 trackball */
		new_x = ioport("IN5")->read();
		if (new_x != m_counter_x_in2)
		{
			m_sign_x_2 = (new_x - m_counter_x_in2) & 0x80;
			m_counter_x_in2 = new_x;
		}

		new_y = ioport("IN4")->read();
		if (new_y != m_counter_y_in2)
		{
			m_sign_y_2 = (new_y - m_counter_y_in2) & 0x80;
			m_counter_y_in2 = new_y;
		}

		return (((m_counter_y_in2 & 0x0f) << 4) | (m_counter_x_in2 & 0x0f));
	}
}

READ8_MEMBER(atarifb_state::atarifb4_in0_r)
{
	/* LD1 and LD2 low, return sign bits */
	if ((m_CTRLD & 0x60) == 0x00)
	{
		int val;

		val = (m_sign_x_4 >> 7) |
				(m_sign_y_4 >> 6) |
				(m_sign_x_2 >> 5) |
				(m_sign_y_2 >> 4) |
				(m_sign_x_3 >> 3) |
				(m_sign_y_3 >> 2) |
				(m_sign_x_1 >> 1) |
				(m_sign_y_1 >> 0);
		return val;
	}
	else if ((m_CTRLD & 0x60) == 0x60)
	/* LD1 and LD2 both high, return Team 1 right player (player 1) */
	{
		int new_x, new_y;

		/* Read player 1 trackball */
		new_x = ioport("IN3")->read();
		if (new_x != m_counter_x_in0)
		{
			m_sign_x_1 = (new_x - m_counter_x_in0) & 0x80;
			m_counter_x_in0 = new_x;
		}

		new_y = ioport("IN2")->read();
		if (new_y != m_counter_y_in0)
		{
			m_sign_y_1 = (new_y - m_counter_y_in0) & 0x80;
			m_counter_y_in0 = new_y;
		}

		return (((m_counter_y_in0 & 0x0f) << 4) | (m_counter_x_in0 & 0x0f));
	}
	else if ((m_CTRLD & 0x60) == 0x40)
	/* LD1 high, LD2 low, return Team 1 left player (player 2) */
	{
		int new_x, new_y;

		/* Read player 2 trackball */
		new_x = ioport("IN5")->read();
		if (new_x != m_counter_x_in0b)
		{
			m_sign_x_2 = (new_x - m_counter_x_in0b) & 0x80;
			m_counter_x_in0b = new_x;
		}

		new_y = ioport("IN4")->read();
		if (new_y != m_counter_y_in0b)
		{
			m_sign_y_2 = (new_y - m_counter_y_in0b) & 0x80;
			m_counter_y_in0b = new_y;
		}

		return (((m_counter_y_in0b & 0x0f) << 4) | (m_counter_x_in0b & 0x0f));
	}

	else return 0;
}


READ8_MEMBER(atarifb_state::atarifb4_in2_r)
{
	if ((m_CTRLD & 0x40) == 0x00)
	{
		return ioport("IN1")->read();
	}
	else if ((m_CTRLD & 0x60) == 0x60)
	/* LD1 and LD2 both high, return Team 2 right player (player 3) */
	{
		int new_x, new_y;

		/* Read player 3 trackball */
		new_x = ioport("IN7")->read();
		if (new_x != m_counter_x_in2)
		{
			m_sign_x_3 = (new_x - m_counter_x_in2) & 0x80;
			m_counter_x_in2 = new_x;
		}

		new_y = ioport("IN6")->read();
		if (new_y != m_counter_y_in2)
		{
			m_sign_y_3 = (new_y - m_counter_y_in2) & 0x80;
			m_counter_y_in2 = new_y;
		}

		return (((m_counter_y_in2 & 0x0f) << 4) | (m_counter_x_in2 & 0x0f));
	}
	else if ((m_CTRLD & 0x60) == 0x40)
	/* LD1 high, LD2 low, return Team 2 left player (player 4) */
	{
		int new_x, new_y;

		/* Read player 4 trackball */
		new_x = ioport("IN9")->read();
		if (new_x != m_counter_x_in2b)
		{
			m_sign_x_4 = (new_x - m_counter_x_in2b) & 0x80;
			m_counter_x_in2b = new_x;
		}

		new_y = ioport("IN8")->read();
		if (new_y != m_counter_y_in2b)
		{
			m_sign_y_4 = (new_y - m_counter_y_in2b) & 0x80;
			m_counter_y_in2b = new_y;
		}

		return (((m_counter_y_in2b & 0x0f) << 4) | (m_counter_x_in2b & 0x0f));
	}

	else return 0;
}
