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
#include "itech8.h"


// configurable logging
#define LOG_SENSOR     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_SENSOR)

#include "logmacro.h"

#define LOGSENSOR(...)     LOGMASKED(LOG_SENSOR,     __VA_ARGS__)



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
static void sensors_to_words(u16 sens0, u16 sens1, u16 sens2, u16 sens3,
							u16 &word1, u16 &word2, u16 &word3, u8 &beams)
{
	// word 1 contains the difference between the larger of sensors 2 & 3 and the smaller
	word1 = (sens3 > sens2) ? (sens3 - sens2) : (sens2 - sens3);

	// word 2 contains the value of the smaller of sensors 2 & 3
	word2 = (sens3 > sens2) ? sens2 : sens3;

	// word 3 contains the value of sensor 0 or 1, depending on which fired
	word3 = sens0 ? sens0 : sens1;

	// set the beams bits
	beams = 0;

	// if sensor 1 fired first, set bit 0
	if (!sens0)
		beams |= 1;

	// if sensor 3 has the larger value, set bit 1
	if (sens3 > sens2)
		beams |= 2;
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
static void words_to_inters(u16 word1, u16 word2, u16 word3, u8 beams,
							u16 &inter1, u16 &inter2, u16 &inter3)
{
	// word 2 is scaled up by 0x1.6553
	u16 word2mod = ((u64)word2 * 0x16553) >> 16;

	// intermediate values 1 and 2 are determined based on the beams bits
	switch (beams)
	{
		case 0:
			inter1 = word1 + word2mod;
			inter2 = word2mod + word3;
			break;

		case 1:
			inter1 = word1 + word2mod + word3;
			inter2 = word2mod;
			break;

		case 2:
			inter1 = word2mod;
			inter2 = word1 + word2mod + word3;
			break;

		case 3:
			inter1 = word2mod + word3;
			inter2 = word1 + word2mod;
			break;
	}

	// intermediate value 3 is always equal to the third word
	inter3 = word3;
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

void slikshot_state::inters_to_vels(u16 inter1, u16 inter2, u16 inter3, u8 beams,
							u8 &xres, u8 &vxres, u8 &vyres)
{
	u16 const xoffs = 0x0016;
	u8 const xscale = 0xe6;
	u16 x;

	// compute Vy
	u32 vy = inter1 ? (0x31c28 / inter1) : 0;

	// compute Vx
	u32 const _283a = inter2 ? (0x30f2e / inter2) : 0;
	u32 _27d8 = ((u64)vy * 0xfbd3) >> 16;
	u32 _27c2 = _283a - _27d8;
	u8 vxsgn = 0;
	if ((s32)_27c2 < 0)
	{
		vxsgn = 1;
		_27c2 = _27d8 - _283a;
	}
	u32 vx = ((u64)_27c2 * 0x58f8c) >> 16;

	// compute X
	_27d8 = ((u64)(inter3 << 16) * _283a) >> 16;
	u32 _283e = ((u64)_27d8 * 0x4a574b) >> 16;

	// adjust X based on the low bit of the beams
	if (beams & 1)
		x = 0x7a + (_283e >> 16) - xoffs;
	else
		x = 0x7a - (_283e >> 16) - xoffs;

	// apply a constant X scale
	if (xscale)
		x = ((xscale * (x & 0xff)) >> 8) & 0xff;

	// clamp if out of range
	if ((vx & 0xffff) >= 0x80)
		x = 0;

	// put the sign back in Vx
	vx &= 0xff;
	if (!vxsgn)
		vx = -vx;

	// clamp VY
	if ((vy & 0xffff) > 0x7f)
		vy = 0x7f;
	else
		vy &= 0xff;

	// copy the results
	xres = x;
	vxres = vx;
	vyres = vy;
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

void slikshot_state::vels_to_inters(u8 x, u8 vx, u8 vy,
							u16 &inter1, u16 &inter2, u16 &inter3, u8 &beams)
{
	u16 const xoffs = 0x0016;
	u8 const xscale = 0xe6;
	u8 x1, vx1, vy1;
	u8 x2, vx2, vy2;

	// inter1 comes from Vy
	inter1 = vy ? 0x31c28 / vy : 0;

	// inter2 can be derived from Vx and Vy
	u32 const _27d8 = ((u64)vy * 0xfbd3) >> 16;
	inter2 = 0x30f2e / (_27d8 + (((u32)abs((s8)vx) << 16) / 0x58f8c));
	u16 inter2a = 0x30f2e / (_27d8 - (((u32)abs((s8)vx) << 16) / 0x58f8c));

	// compute it back both ways and pick the closer
	inters_to_vels(inter1, inter2, 0, 0, x1, vx1, vy1);
	inters_to_vels(inter1, inter2a, 0, 0, x2, vx2, vy2);
	u8 const diff1 = (vx > vx1) ? (vx - vx1) : (vx1 - vx);
	u8 const diff2 = (vx > vx2) ? (vx - vx2) : (vx2 - vx);
	if (diff2 < diff1)
		inter2 = inter2a;

	// inter3: (beams & 1 == 1), inter3a: (beams & 1) == 0
	if (((x << 8) / xscale) + xoffs >= 0x7a)
	{
		beams = 1;
		inter3 = (((((((u64)(((x << 8) / xscale) + xoffs - 0x7a)) << 16) << 16) / 0x4a574b) << 16) / (0x30f2e / inter2)) >> 16;
	}
	else
	{
		beams = 0;
		inter3 = (((((((u64)(((x << 8) / xscale) + xoffs - 0x7a) * -1) << 16) << 16) / 0x4a574b) << 16) / (0x30f2e / inter2)) >> 16;
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

void slikshot_state::inters_to_words(u16 inter1, u16 inter2, u16 inter3, u8 &beams,
							u16 &word1, u16 &word2, u16 &word3)
{
	u16 word2mod;

	// intermediate value 3 is always equal to the third word
	word3 = inter3;

	// on input, it is expected that the low bit of beams has already been determined
	if (beams & 1)
	{
		// make sure we can do it
		if (inter3 <= inter1)
		{
			// always go back via case 3
			beams |= 2;

			// compute an appropriate value for the scaled version of word 2
			word2mod = inter1 - inter3;

			// compute the other values from that
			word1 = inter2 - word2mod;
			word2 = ((u64)word2mod << 16) / 0x16553;
		}
		else
			LOGSENSOR("inters_to_words: unable to convert %04x %04x %04x %02x\n",
					inter1, inter2, inter3, beams);
	}

	// handle the case where low bit of beams is 0
	else
	{
		// make sure we can do it
		if (inter3 <= inter2)
		{
			// always go back via case 0

			// compute an appropriate value for the scaled version of word 2
			word2mod = inter2 - inter3;

			// compute the other values from that
			word1 = inter1 - word2mod;
			word2 = ((u64)word2mod << 16) / 0x16553;
		}
		else
			LOGSENSOR("inters_to_words: unable to convert %04x %04x %04x %02x\n",
					inter1, inter2, inter3, beams);
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

void slikshot_state::words_to_sensors(u16 word1, u16 word2, u16 word3, u8 beams,
							u16 &sens0, u16 &sens1, u16 &sens2, u16 &sens3)
{
	// if bit 0 of the beams is set, sensor 1 fired first; otherwise sensor 0 fired
	if (beams & 1)
		sens0 = 0, sens1 = word3;
	else
		sens0 = word3, sens1 = 0;

	// if bit 1 of the beams is set, sensor 3 had a larger value
	if (beams & 2)
		sens3 = word2 + word1, sens2 = word2;
	else
		sens2 = word2 + word1, sens3 = word2;
}



/*************************************
 *
 *  compute_sensors
 *
 *************************************/

void slikshot_state::compute_sensors()
{
	u16 inter1, inter2, inter3;
	u16 word1 = 0, word2 = 0, word3 = 0;
	u8 beams;

	// skip if we're not ready
	if (m_sensor0 != 0 || m_sensor1 != 0 || m_sensor2 != 0 || m_sensor3 != 0)
		return;

	// reverse map the inputs
	vels_to_inters(m_curx, m_curvx, m_curvy, inter1, inter2, inter3, beams);
	inters_to_words(inter1, inter2, inter3, beams, word1, word2, word3);
	words_to_sensors(word1, word2, word3, beams, m_sensor0, m_sensor1, m_sensor2, m_sensor3);

	LOGSENSOR("%15f: Sensor values: %04x %04x %04x %04x\n", machine().time().as_double(), m_sensor0, m_sensor1, m_sensor2, m_sensor3);
}



/*************************************
 *
 *  z80_port_r
 *
 *************************************/

u8 slikshot_state::z80_port_r()
{
	// if we have nothing, return 0x03
	if (!m_sensor0 && !m_sensor1 && !m_sensor2 && !m_sensor3)
		return 0x03 | (m_z80_clear_to_send << 7);

	u8 result = 0;

	// 1 bit for each sensor
	if (m_sensor0)
	{
		result |= 1;
		if (!machine().side_effects_disabled())
			m_sensor0--;
	}
	if (m_sensor1)
	{
		result |= 2;
		if (!machine().side_effects_disabled())
			m_sensor1--;
	}
	if (m_sensor2)
	{
		result |= 4;
		if (!machine().side_effects_disabled())
			m_sensor2--;
	}
	if (m_sensor3)
	{
		result |= 8;
		if (!machine().side_effects_disabled())
			m_sensor3--;
	}
	result |= m_z80_clear_to_send << 7;

	return result;
}



/*************************************
 *
 *  z80_port_w
 *
 *************************************/

void slikshot_state::z80_port_w(u8 data)
{
	m_z80_port_val = data;
	m_z80_clear_to_send = 0;
}



/*************************************
 *
 *  z80_r
 *
 *************************************/

u8 slikshot_state::z80_r()
{
	// allow the Z80 to send us stuff now
	if (!machine().side_effects_disabled())
		m_z80_clear_to_send = 1;
	return m_z80_port_val;
}



/*************************************
 *
 *  z80_control_r
 *
 *************************************/

u8 slikshot_state::z80_control_r()
{
	return m_z80_ctrl;
}



/*************************************
 *
 *  z80_control_w
 *
 *************************************/

TIMER_CALLBACK_MEMBER(slikshot_state::delayed_z80_control_w)
{
	u8 const data = param;

	// bit 4 controls the reset line on the Z80

	// this is a big kludge: only allow a reset if the Z80 is stopped
	// at its endpoint; otherwise, we never get a result from the Z80
	if ((data & 0x10) || m_subcpu->state_int(Z80_PC) == 0x13a)
	{
		m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

		// on the rising edge, make the crosshair visible again
		if ((data & 0x10) && !(m_z80_ctrl & 0x10))
			m_crosshair_vis = 1;
	}

	// boost the interleave whenever this is written to
	machine().scheduler().perfect_quantum(attotime::from_usec(100));

	// stash the new value
	m_z80_ctrl = data;
}


void slikshot_state::z80_control_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(slikshot_state::delayed_z80_control_w), this), data);
}



void slikshot_state::machine_start()
{
	itech8_state::machine_start();

	m_z80_ctrl = 0;
	m_z80_port_val = 0;
	m_z80_clear_to_send = 0;

	m_sensor0 = m_sensor1 = m_sensor2 = m_sensor3 = 0;
	m_curvx = 0, m_curvy = 1, m_curx = 0;
	m_ybuffer_next = 0;
	m_curxpos = 0;
	m_last_ytotal = 0;
	m_crosshair_vis = 0;

	save_item(NAME(m_z80_ctrl));
	save_item(NAME(m_z80_port_val));
	save_item(NAME(m_z80_clear_to_send));
	save_item(NAME(m_sensor0));
	save_item(NAME(m_sensor1));
	save_item(NAME(m_sensor2));
	save_item(NAME(m_sensor3));
	save_item(NAME(m_curvx));
	save_item(NAME(m_curvy));
	save_item(NAME(m_curx));
	save_item(NAME(m_xbuffer));
	save_item(NAME(m_ybuffer));
	save_item(NAME(m_ybuffer_next));
	save_item(NAME(m_curxpos));
	save_item(NAME(m_last_ytotal));
	save_item(NAME(m_crosshair_vis));
}


/*************************************
 *
 *  slikshot_state::screen_update
 *
 *************************************/

u32 slikshot_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// draw the normal video first
	screen_update_2page(screen, bitmap, cliprect);

	// add the current X,Y positions to the list
	m_xbuffer[m_ybuffer_next % YBUFFER_COUNT] = m_fakex->read();
	m_ybuffer[m_ybuffer_next % YBUFFER_COUNT] = m_fakey->read();
	m_ybuffer_next++;

	// determine where to draw the starting point
	m_curxpos = std::clamp<s32>(m_curxpos + m_xbuffer[(m_ybuffer_next + 1) % YBUFFER_COUNT], -0x80, 0x80);

	// compute the total X/Y movement
	s32 totaldx = 0, totaldy = 0;
	for (int i = 0; i < YBUFFER_COUNT - 1; i++)
	{
		totaldx += m_xbuffer[(m_ybuffer_next + i + 1) % YBUFFER_COUNT];
		totaldy += m_ybuffer[(m_ybuffer_next + i + 1) % YBUFFER_COUNT];
	}

	// if the shoot button is pressed, fire away
	if (totaldy < m_last_ytotal && m_last_ytotal > 50 && m_crosshair_vis)
	{
		m_curvx = std::clamp<int>(totaldx, -0x7f, 0x7f);
		m_curvy = std::clamp<int>(m_last_ytotal - 50, 0x10, 0x7f);
		m_curx = std::clamp<int>(0x60 + ((m_curxpos * 3) >> 3), 0x30, 0x90);

		compute_sensors();
//      popmessage("V=%02x,%02x  X=%02x", m_curvx, m_curvy, m_curx);
		m_crosshair_vis = 0;
	}
	m_last_ytotal = totaldy;

	// clear the buffer while the crosshair is not visible
	if (!m_crosshair_vis)
	{
		memset(m_xbuffer, 0, sizeof(m_xbuffer));
		memset(m_ybuffer, 0, sizeof(m_ybuffer));
	}

	return 0;
}
