// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Slick Shot input handling

    Unlike the other 8-bit Strata games, Slick Shot has an interesting
    and fairly complex input system. The actual cabinet has a good-sized
    gap underneath the monitor, from which a small pool table emerges.
    An actual cue ball and pool sticks were included with the game.

    To "control" the game, players actually put the cue ball on the pool
    table and shot the ball into the gap. Four sensors underneath the
    monitor would count how long they saw the ball, and from this data,
    the velocity and crossing point of the ball could be derived.

    In order to read these sensors, an extra Z80 was added to the board.
    The Z80 program is astoundingly simple: on reset, it writes a value of
    $00 to the output port, then waits for either sensor 0 or 1 to fire.
    As soon as one of those sensors fires, it begins counting how long it
    takes for the bits corresponding to those sensors, as well as sensors
    2 and 3, to return to their 0 state. It then writes a $ff to the
    output port to signal that data is ready and waits for the main CPU
    to clock the data through.

    On the main program side of things, the result from the Z80 is
    periodically polled. Once a $ff is seen, 3 words and 1 bytes' worth
    of data is read from the Z80, after which the Z80 goes into an
    infinite loop. When the main program is ready to read a result again,
    it resets the Z80 to start the read going again.

    The way the Z80 reads the data, is as follows:

        - write $00 to output
        - wait for sensor 0 or 1 to fire (go to the 1 state)
        - count how long that sensor takes to return to 0
        - count how long sensors 2 and 3 take to return to 0
        - write $ff to output
        - wait for data to be clocked through
        - return 3 words + 1 byte of data:
            - word 0 = (value of larger of sensor 2/3 counts) - (value of smaller)
            - word 1 = value of smaller of sensor 2/3 counts
            - word 2 = value of sensor 0/1
            - byte = beam data
                - bit 0 = 1 if sensor 0 fired; 0 if sensor 1 fired
                - bit 1 = 1 if sensor 3 value > sensor 2 value; 0 otherwise
        - enter infinite loop

    Once this data is read from the Z80, it is converted to an intermediate
    form, and then processed using 32-bit math (yes, on a 6809!) to produce
    the final velocity and X position of the crossing.

    Because it is not understood exactly where the sensors are placed and
    how to simulate the actual behavior, this module attempts to do the
    next best thing: given a velocity and X position, figure out raw
    sensor values that will travel from the Z80 to the main 6809 and
    through the calculations produce approximately the correct results.

    There are several stages of data:

        - sens0, sens1, sens2, sens3 = raw sensor values
        - word1, word2, word3, beam = values from the Z80 (beam = byte val)
        - inter1, inter2, inter3, beam = intermediate forms in the 6809
        - vx, vy, x = final X,Y velocities and X crossing point

    And all the functions here are designed to take you through the various
    stages, both forwards and backwards, replicating the operations in the
    6809 or reversing them.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/itech8.h"


#define MINDY           100


/*************************************
 *
 *  sensors_to_words
 *
 *  converts from raw sensor data to
 *  the three words + byte that the
 *  Z80 sends to the main 6809
 *
 *************************************/

#ifdef STANDALONE
static void sensors_to_words(UINT16 sens0, UINT16 sens1, UINT16 sens2, UINT16 sens3,
							UINT16 *word1, UINT16 *word2, UINT16 *word3, UINT8 *beams)
{
	/* word 1 contains the difference between the larger of sensors 2 & 3 and the smaller */
	*word1 = (sens3 > sens2) ? (sens3 - sens2) : (sens2 - sens3);

	/* word 2 contains the value of the smaller of sensors 2 & 3 */
	*word2 = (sens3 > sens2) ? sens2 : sens3;

	/* word 3 contains the value of sensor 0 or 1, depending on which fired */
	*word3 = sens0 ? sens0 : sens1;

	/* set the beams bits */
	*beams = 0;

	/* if sensor 1 fired first, set bit 0 */
	if (!sens0)
		*beams |= 1;

	/* if sensor 3 has the larger value, set bit 1 */
	if (sens3 > sens2)
		*beams |= 2;
}
#endif


/*************************************
 *
 *  words_to_inters
 *
 *  converts the three words + byte
 *  data from the Z80 into the three
 *  intermediate values used in the
 *  final calculations
 *
 *************************************/

#ifdef STANDALONE
static void words_to_inters(UINT16 word1, UINT16 word2, UINT16 word3, UINT8 beams,
							UINT16 *inter1, UINT16 *inter2, UINT16 *inter3)
{
	/* word 2 is scaled up by 0x1.6553 */
	UINT16 word2mod = ((UINT64)word2 * 0x16553) >> 16;

	/* intermediate values 1 and 2 are determined based on the beams bits */
	switch (beams)
	{
		case 0:
			*inter1 = word1 + word2mod;
			*inter2 = word2mod + word3;
			break;

		case 1:
			*inter1 = word1 + word2mod + word3;
			*inter2 = word2mod;
			break;

		case 2:
			*inter1 = word2mod;
			*inter2 = word1 + word2mod + word3;
			break;

		case 3:
			*inter1 = word2mod + word3;
			*inter2 = word1 + word2mod;
			break;
	}

	/* intermediate value 3 is always equal to the third word */
	*inter3 = word3;
}
#endif


/*************************************
 *
 *  inters_to_vels
 *
 *  converts the three intermediate
 *  values to the final velocity and
 *  X position values
 *
 *************************************/

void itech8_state::inters_to_vels(UINT16 inter1, UINT16 inter2, UINT16 inter3, UINT8 beams,
							UINT8 *xres, UINT8 *vxres, UINT8 *vyres)
{
	UINT32 _27d8, _27c2;
	UINT32 vx, vy, _283a, _283e;
	UINT8 vxsgn;
	UINT16 xoffs = 0x0016;
	UINT8 xscale = 0xe6;
	UINT16 x;

	/* compute Vy */
	vy = inter1 ? (0x31c28 / inter1) : 0;

	/* compute Vx */
	_283a = inter2 ? (0x30f2e / inter2) : 0;
	_27d8 = ((UINT64)vy * 0xfbd3) >> 16;
	_27c2 = _283a - _27d8;
	vxsgn = 0;
	if ((INT32)_27c2 < 0)
	{
		vxsgn = 1;
		_27c2 = _27d8 - _283a;
	}
	vx = ((UINT64)_27c2 * 0x58f8c) >> 16;

	/* compute X */
	_27d8 = ((UINT64)(inter3 << 16) * _283a) >> 16;
	_283e = ((UINT64)_27d8 * 0x4a574b) >> 16;

	/* adjust X based on the low bit of the beams */
	if (beams & 1)
		x = 0x7a + (_283e >> 16) - xoffs;
	else
		x = 0x7a - (_283e >> 16) - xoffs;

	/* apply a constant X scale */
	if (xscale)
		x = ((xscale * (x & 0xff)) >> 8) & 0xff;

	/* clamp if out of range */
	if ((vx & 0xffff) >= 0x80)
		x = 0;

	/* put the sign back in Vx */
	vx &= 0xff;
	if (!vxsgn)
		vx = -vx;

	/* clamp VY */
	if ((vy & 0xffff) > 0x7f)
		vy = 0x7f;
	else
		vy &= 0xff;

	/* copy the results */
	*xres = x;
	*vxres = vx;
	*vyres = vy;
}



/*************************************
 *
 *  vels_to_inters
 *
 *  converts from the final velocity
 *  and X position values back to
 *  three intermediate values that
 *  will produce the desired result
 *
 *************************************/

void itech8_state::vels_to_inters(UINT8 x, UINT8 vx, UINT8 vy,
							UINT16 *inter1, UINT16 *inter2, UINT16 *inter3, UINT8 *beams)
{
	UINT32 _27d8;
	UINT16 xoffs = 0x0016;
	UINT8 xscale = 0xe6;
	UINT8 x1, vx1, vy1;
	UINT8 x2, vx2, vy2;
	UINT8 diff1, diff2;
	UINT16 inter2a;

	/* inter1 comes from Vy */
	*inter1 = vy ? 0x31c28 / vy : 0;

	/* inter2 can be derived from Vx and Vy */
	_27d8 = ((UINT64)vy * 0xfbd3) >> 16;
	*inter2 = 0x30f2e / (_27d8 + ((abs((INT8)vx) << 16) / 0x58f8c));
	inter2a = 0x30f2e / (_27d8 - ((abs((INT8)vx) << 16) / 0x58f8c));

	/* compute it back both ways and pick the closer */
	inters_to_vels(*inter1, *inter2, 0, 0, &x1, &vx1, &vy1);
	inters_to_vels(*inter1, inter2a, 0, 0, &x2, &vx2, &vy2);
	diff1 = (vx > vx1) ? (vx - vx1) : (vx1 - vx);
	diff2 = (vx > vx2) ? (vx - vx2) : (vx2 - vx);
	if (diff2 < diff1)
		*inter2 = inter2a;

	/* inter3: (beams & 1 == 1), inter3a: (beams & 1) == 0 */
	if (((x << 8) / xscale) + xoffs >= 0x7a)
	{
		*beams = 1;
		*inter3 = (((((((UINT64)(((x << 8) / xscale) + xoffs - 0x7a)) << 16) << 16) / 0x4a574b) << 16) / (0x30f2e / *inter2)) >> 16;
	}
	else
	{
		*beams = 0;
		*inter3 = (((((((UINT64)(((x << 8) / xscale) + xoffs - 0x7a) * -1) << 16) << 16) / 0x4a574b) << 16) / (0x30f2e / *inter2)) >> 16;
	}
}



/*************************************
 *
 *  inters_to_words
 *
 *  converts the intermediate values
 *  used in the final calculations
 *  back to the three words + byte
 *  data from the Z80
 *
 *************************************/

void itech8_state::inters_to_words(UINT16 inter1, UINT16 inter2, UINT16 inter3, UINT8 *beams,
							UINT16 *word1, UINT16 *word2, UINT16 *word3)
{
	UINT16 word2mod;

	/* intermediate value 3 is always equal to the third word */
	*word3 = inter3;

	/* on input, it is expected that the low bit of beams has already been determined */
	if (*beams & 1)
	{
		/* make sure we can do it */
		if (inter3 <= inter1)
		{
			/* always go back via case 3 */
			*beams |= 2;

			/* compute an appropriate value for the scaled version of word 2 */
			word2mod = inter1 - inter3;

			/* compute the other values from that */
			*word1 = inter2 - word2mod;
			*word2 = ((UINT64)word2mod << 16) / 0x16553;
		}
		else
			logerror("inters_to_words: unable to convert %04x %04x %04x %02x\n",
					(UINT32)inter1, (UINT32)inter2, (UINT32)inter3, (UINT32)*beams);
	}

	/* handle the case where low bit of beams is 0 */
	else
	{
		/* make sure we can do it */
		if (inter3 <= inter2)
		{
			/* always go back via case 0 */

			/* compute an appropriate value for the scaled version of word 2 */
			word2mod = inter2 - inter3;

			/* compute the other values from that */
			*word1 = inter1 - word2mod;
			*word2 = ((UINT64)word2mod << 16) / 0x16553;
		}
		else
			logerror("inters_to_words: unable to convert %04x %04x %04x %02x\n",
					(UINT32)inter1, (UINT32)inter2, (UINT32)inter3, (UINT32)*beams);
	}
}



/*************************************
 *
 *  words_to_sensors
 *
 *  converts from the three words +
 *  byte that the Z80 sends to the
 *  main 6809 back to raw sensor data
 *
 *************************************/

void itech8_state::words_to_sensors(UINT16 word1, UINT16 word2, UINT16 word3, UINT8 beams,
							UINT16 *sens0, UINT16 *sens1, UINT16 *sens2, UINT16 *sens3)
{
	/* if bit 0 of the beams is set, sensor 1 fired first; otherwise sensor 0 fired */
	if (beams & 1)
		*sens0 = 0, *sens1 = word3;
	else
		*sens0 = word3, *sens1 = 0;

	/* if bit 1 of the beams is set, sensor 3 had a larger value */
	if (beams & 2)
		*sens3 = word2 + word1, *sens2 = word2;
	else
		*sens2 = word2 + word1, *sens3 = word2;
}



/*************************************
 *
 *  compute_sensors
 *
 *************************************/

void itech8_state::compute_sensors()
{
	UINT16 inter1, inter2, inter3;
	UINT16 word1 = 0, word2 = 0, word3 = 0;
	UINT8 beams;

	/* skip if we're not ready */
	if (m_sensor0 != 0 || m_sensor1 != 0 || m_sensor2 != 0 || m_sensor3 != 0)
		return;

	/* reverse map the inputs */
	vels_to_inters(m_curx, m_curvx, m_curvy, &inter1, &inter2, &inter3, &beams);
	inters_to_words(inter1, inter2, inter3, &beams, &word1, &word2, &word3);
	words_to_sensors(word1, word2, word3, beams, &m_sensor0, &m_sensor1, &m_sensor2, &m_sensor3);

	logerror("%15f: Sensor values: %04x %04x %04x %04x\n", machine().time().as_double(), m_sensor0, m_sensor1, m_sensor2, m_sensor3);
}



/*************************************
 *
 *  slikz80_port_r
 *
 *************************************/

READ8_MEMBER(itech8_state::slikz80_port_r )
{
	int result = 0;

	/* if we have nothing, return 0x03 */
	if (!m_sensor0 && !m_sensor1 && !m_sensor2 && !m_sensor3)
		return 0x03 | (m_z80_clear_to_send << 7);

	/* 1 bit for each sensor */
	if (m_sensor0)
		result |= 1, m_sensor0--;
	if (m_sensor1)
		result |= 2, m_sensor1--;
	if (m_sensor2)
		result |= 4, m_sensor2--;
	if (m_sensor3)
		result |= 8, m_sensor3--;
	result |= m_z80_clear_to_send << 7;

	return result;
}



/*************************************
 *
 *  slikz80_port_w
 *
 *************************************/

WRITE8_MEMBER(itech8_state::slikz80_port_w )
{
	m_z80_port_val = data;
	m_z80_clear_to_send = 0;
}



/*************************************
 *
 *  slikshot_z80_r
 *
 *************************************/

READ8_MEMBER(itech8_state::slikshot_z80_r )
{
	/* allow the Z80 to send us stuff now */
	m_z80_clear_to_send = 1;
	return m_z80_port_val;
}



/*************************************
 *
 *  slikshot_z80_control_r
 *
 *************************************/

READ8_MEMBER(itech8_state::slikshot_z80_control_r )
{
	return m_z80_ctrl;
}



/*************************************
 *
 *  slikshot_z80_control_w
 *
 *************************************/

TIMER_CALLBACK_MEMBER( itech8_state::delayed_z80_control_w )
{
	int data = param;

	/* bit 4 controls the reset line on the Z80 */

	/* this is a big kludge: only allow a reset if the Z80 is stopped */
	/* at its endpoint; otherwise, we never get a result from the Z80 */
	if ((data & 0x10) || m_subcpu->state_int(Z80_PC) == 0x13a)
	{
		m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

		/* on the rising edge, make the crosshair visible again */
		if ((data & 0x10) && !(m_z80_ctrl & 0x10))
			m_crosshair_vis = 1;
	}

	/* boost the interleave whenever this is written to */
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));

	/* stash the new value */
	m_z80_ctrl = data;
}


WRITE8_MEMBER(itech8_state::slikshot_z80_control_w )
{
	synchronize(TIMER_DELAYED_Z80_CONTROL, data);
}



VIDEO_START_MEMBER(itech8_state,slikshot)
{
	itech8_state::video_start();

	m_z80_ctrl = 0;
	m_z80_port_val = 0;
	m_z80_clear_to_send = 0;

	m_sensor0 = m_sensor1 = m_sensor2 = m_sensor3 = 0;
	m_curvx = 0, m_curvy = 1, m_curx = 0;
	m_ybuffer_next = 0;
	m_curxpos = 0;
	m_last_ytotal = 0;
	m_crosshair_vis = 0;
}


/*************************************
 *
 *  SCREEN_UPDATE( slikshot )
 *
 *************************************/

UINT32 itech8_state::screen_update_slikshot(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int totaldy, totaldx;
	int temp, i;

	/* draw the normal video first */
	screen_update_2page(screen, bitmap, cliprect);

	/* add the current X,Y positions to the list */
	m_xbuffer[m_ybuffer_next % YBUFFER_COUNT] = ioport("FAKEX")->read_safe(0);
	m_ybuffer[m_ybuffer_next % YBUFFER_COUNT] = ioport("FAKEY")->read_safe(0);
	m_ybuffer_next++;

	/* determine where to draw the starting point */
	m_curxpos += m_xbuffer[(m_ybuffer_next + 1) % YBUFFER_COUNT];
	if (m_curxpos < -0x80) m_curxpos = -0x80;
	if (m_curxpos >  0x80) m_curxpos =  0x80;

	/* compute the total X/Y movement */
	totaldx = totaldy = 0;
	for (i = 0; i < YBUFFER_COUNT - 1; i++)
	{
		totaldx += m_xbuffer[(m_ybuffer_next + i + 1) % YBUFFER_COUNT];
		totaldy += m_ybuffer[(m_ybuffer_next + i + 1) % YBUFFER_COUNT];
	}

	/* if the shoot button is pressed, fire away */
	if (totaldy < m_last_ytotal && m_last_ytotal > 50 && m_crosshair_vis)
	{
		/* compute the updated values */
		temp = totaldx;
		if (temp <= -0x80) temp = -0x7f;
		if (temp >=  0x80) temp =  0x7f;
		m_curvx = temp;

		temp = m_last_ytotal - 50;
		if (temp <=  0x10) temp =  0x10;
		if (temp >=  0x7f) temp =  0x7f;
		m_curvy = temp;

		temp = 0x60 + (m_curxpos * 0x30 / 0x80);
		if (temp <=  0x30) temp =  0x30;
		if (temp >=  0x90) temp =  0x90;
		m_curx = temp;

		compute_sensors();
//      popmessage("V=%02x,%02x  X=%02x", m_curvx, m_curvy, m_curx);
		m_crosshair_vis = 0;
	}
	m_last_ytotal = totaldy;

	/* clear the buffer while the crosshair is not visible */
	if (!m_crosshair_vis)
	{
		memset(m_xbuffer, 0, sizeof(m_xbuffer));
		memset(m_ybuffer, 0, sizeof(m_ybuffer));
	}

	return 0;
}



/*************************************
 *
 *  main
 *
 *  uncomment this to make a stand
 *  alone version for testing
 *
 *************************************/

#ifdef STANDALONE

int main(int argc, char *argv[])
{
	UINT16 word1, word2, word3;
	UINT16 inter1, inter2, inter3;
	UINT8 beams, x, vx, vy;

	if (argc == 5)
	{
		UINT32 sens0, sens1, sens2, sens3;

		sscanf(argv[1], "%x", &sens0);
		sscanf(argv[2], "%x", &sens1);
		sscanf(argv[3], "%x", &sens2);
		sscanf(argv[4], "%x", &sens3);
		osd_printf_debug("sensors: %04x %04x %04x %04x\n", sens0, sens1, sens2, sens3);
		if (sens0 && sens1)
		{
			osd_printf_debug("error: sensor 0 or 1 must be 0\n");
			return 1;
		}

		sensors_to_words(sens0, sens1, sens2, sens3, &word1, &word2, &word3, &beams);
		osd_printf_debug("word1 = %04x  word2 = %04x  word3 = %04x  beams = %d\n",
				(UINT32)word1, (UINT32)word2, (UINT32)word3, (UINT32)beams);

		words_to_inters(word1, word2, word3, beams, &inter1, &inter2, &inter3);
		osd_printf_debug("inter1 = %04x  inter2 = %04x  inter3 = %04x\n", (UINT32)inter1, (UINT32)inter2, (UINT32)inter3);

		inters_to_vels(inter1, inter2, inter3, beams, &x, &vx, &vy);
		osd_printf_debug("x = %02x  vx = %02x  vy = %02x\n", (UINT32)x, (UINT32)vx, (UINT32)vy);
	}
	else if (argc == 4)
	{
		UINT32 xin, vxin, vyin;
		UINT16 sens0, sens1, sens2, sens3;

		sscanf(argv[1], "%x", &xin);
		sscanf(argv[2], "%x", &vxin);
		sscanf(argv[3], "%x", &vyin);
		x = xin;
		vx = vxin;
		vy = vyin;
		osd_printf_debug("x = %02x  vx = %02x  vy = %02x\n", (UINT32)x, (UINT32)vx, (UINT32)vy);

		vels_to_inters(x, vx, vy, &inter1, &inter2, &inter3, &beams);
		osd_printf_debug("inter1 = %04x  inter2 = %04x  inter3 = %04x  beams = %d\n", (UINT32)inter1, (UINT32)inter2, (UINT32)inter3, (UINT32)beams);

		inters_to_words(inter1, inter2, inter3, &beams, &word1, &word2, &word3);
		osd_printf_debug("word1 = %04x  word2 = %04x  word3 = %04x  beams = %d\n",
				(UINT32)word1, (UINT32)word2, (UINT32)word3, (UINT32)beams);

		words_to_sensors(word1, word2, word3, beams, &sens0, &sens1, &sens2, &sens3);
		osd_printf_debug("sensors: %04x %04x %04x %04x\n", sens0, sens1, sens2, sens3);
	}

	return 0;
}

#endif
