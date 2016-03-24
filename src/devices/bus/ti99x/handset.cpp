// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/4 Handset support (TI99/4 only)

    The ti99/4 was intended to support some so-called "IR remote handsets".
    This feature was canceled at the last minute (reportedly ten minutes before
    the introductory press conference in June 1979), but the first thousands of
    99/4 units had the required port, and the support code was seemingly not
    deleted from ROMs until the introduction of the ti99/4a in 1981.  You could
    connect up to 4 20-key keypads, and up to 4 joysticks with a maximum
    resolution of 15 levels on each axis.

    The keyboard DSR was able to couple two 20-key keypads together to emulate
    a full 40-key keyboard.  Keyboard modes 0, 1 and 2 would select either the
    console keyboard with its two wired remote controllers (i.e. joysticks), or
    remote handsets 1 and 2 with their associated IR remote controllers (i.e.
    joysticks), according to which was currently active.

    Originally written by R. Nabet

    **************************************************

    TI-99/4(A) Twin Joystick

    This file also contains the implementation of the twin joystick;
    actually, no big deal, as it contains no logic but only switches.

    **************************************************

    Michael Zapf
    2010-10-24 Rewriten as device

    January 2012: Rewritten as class

*****************************************************************************/

#include "handset.h"
#include "machine/tms9901.h"

#define LOG logerror
#define VERBOSE 1

static const char *const joynames[2][4] =
{
	{ "JOY0", "JOY2", "JOY4", "JOY6" },     // X axis
	{ "JOY1", "JOY3", "JOY5", "JOY7" }      // Y axis
};

static const char *const keynames[] = { "KP0", "KP1", "KP2", "KP3", "KP4" };

ti99_handset_device::ti99_handset_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: joyport_attached_device(mconfig, HANDSET, "TI-99/4 IR handset", tag, owner, clock, "handset", __FILE__), m_ack(0), m_clock_high(false), m_buf(0), m_buflen(0), m_delay_timer(nullptr)
{
}

#define POLL_TIMER 1
#define DELAY_TIMER 2

/*****************************************************************************/

/*
    Return the status of the handset via the joystick port.
    B = bus
    C = clock
    I = int (neg logic)

    answer = |0|I|C|1|B|B|B|B|
*/
UINT8 ti99_handset_device::read_dev()
{
	return (m_buf & 0xf) | 0x10 | (m_clock_high? 0x20:0) | ( m_buflen==3? 0x00:0x40);
}

void ti99_handset_device::write_dev(UINT8 data)
{
	if (VERBOSE>7) LOG("ti99_handset_device: Set ack %d\n", data);
	set_acknowledge(data);
}

/*
    Handle data acknowledge sent by the ti-99/4 handset ISR (through tms9901
    line P0).  This function is called by a delayed timer 30us after the state
    of P0 is changed, because, in one occasion, the ISR asserts the line before
    it reads the data, so we need to delay the acknowledge process.
*/
void ti99_handset_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_clock_high = !m_clock_high;
	m_buf >>= 4;
	m_buflen--;

	// Clear the INT12 line
	m_joyport->set_interrupt(CLEAR_LINE);

	if (m_buflen == 1)
	{
		// Unless I am missing something, the third and last nibble of the
		// message is not acknowledged by the DSR in any way, and the first nibble
		// of next message is not requested for either, so we need to decide on
		// our own when we can post a new event.  Currently, we wait for 1000us
		// after the DSR acknowledges the second nybble.
		m_delay_timer->adjust(attotime::from_usec(1000));
	}

	if (m_buflen == 0)
		/* See if we need to post a new event */
	do_task();
}

/*
    Handler for tms9901 P0 pin (handset data acknowledge)
*/
void ti99_handset_device::set_acknowledge(int data)
{
	if ((m_buflen !=0) && (data != m_ack))
	{
		m_ack = data;
		if ((data!=0) == m_clock_high)
		{
			// I don't know what the real delay is, but 30us apears to be enough
			m_delay_timer->adjust(attotime::from_usec(30));
		}
	}
}

/*
    post_message()

    Post a 12-bit message: trigger an interrupt on the tms9901, and store the
    message in the I/R receiver buffer so that the handset ISR will read this
    message.

    message: 12-bit message to post (only the 12 LSBits are meaningful)
*/
void ti99_handset_device::post_message(int message)
{
	/* post message and assert interrupt */
	m_clock_high = true;
	m_buf = ~message;
	m_buflen = 3;
	if (VERBOSE>5) LOG("ti99_handset_device: trigger interrupt\n");
	m_joyport->set_interrupt(ASSERT_LINE);
}

/*
    poll_keyboard()
    Poll the current state of one given handset keypad.
    num: number of the keypad to poll (0-3)
    Returns TRUE if the handset state has changed and a message was posted.
*/
bool ti99_handset_device::poll_keyboard(int num)
{
	UINT32 key_buf;
	UINT8 current_key;
	int i;

	/* read current key state */
	key_buf = (ioport(keynames[num])->read() | (ioport(keynames[num + 1])->read() << 16) ) >> (4*num);

	// If a key was previously pressed, this key was not shift, and this key is
	// still down, then don't change the current key press.
	if (previous_key[num] !=0 && (previous_key[num] != 0x24)
		&& (key_buf & (1 << (previous_key[num] & 0x1f))))
	{
		/* check the shift modifier state */
		if (((previous_key[num] & 0x20) != 0) == ((key_buf & 0x0008) != 0))
			/* the shift modifier state has not changed */
			return false;
		else
		{
			// The shift modifier state has changed: we need to update the
			// keyboard state
			if (key_buf & 0x0008)
			{   /* shift has been pressed down */
				previous_key[num] = current_key = previous_key[num] | 0x20;
			}
			else
			{
				previous_key[num] = current_key = previous_key[num] & ~0x20;
			}
			/* post message */
			post_message((((unsigned)current_key) << 4) | (num << 1));
			return true;
		}
	}

	current_key = 0;    /* default value if no key is down */
	for (i=0; i<20; i++)
	{
		if (key_buf & (1 << i))
		{
			current_key = i + 1;
			if (key_buf & 0x0008)
				current_key |= 0x20;    /* set shift flag */

			if (current_key != 0x24)
				// If this is the shift key, any other key we may find will
				// have higher priority; otherwise, we may exit the loop and keep
				// the key we have just found.
				break;
		}
	}

	if (current_key != previous_key[num])
	{
		previous_key[num] = current_key;

		/* post message */
		post_message((((unsigned) current_key) << 4) | (num << 1));
		return true;
	}
	return false;
}

/*
    poll_joystick()

    Poll the current state of one given handset joystick.
    num: number of the joystick to poll (0-3)
    Returns TRUE if the handset state has changed and a message was posted.
*/
bool ti99_handset_device::poll_joystick(int num)
{
	UINT8 current_joy;
	int current_joy_x, current_joy_y;
	int message;
	/* read joystick position */
	current_joy_x = ioport(joynames[0][num])->read();
	current_joy_y = ioport(joynames[1][num])->read();

	/* compare with last saved position */
	current_joy = current_joy_x | (current_joy_y << 4);
	if (current_joy != previous_joy[num])
	{
		/* save position */
		previous_joy[num] = current_joy;

		/* transform position to signed quantity */
		current_joy_x -= 7;
		current_joy_y -= 7;

		message = 0;

		/* set sign */
		// note that we set the sign if the joystick position is 0 to work
		// around a bug in the ti99/4 ROMs
		if (current_joy_x <= 0)
		{
			message |= 0x040;
			current_joy_x = - current_joy_x;
		}

		if (current_joy_y <= 0)
		{
			message |= 0x400;
			current_joy_y = - current_joy_y;
		}

		/* convert absolute values to Gray code and insert in message */
		if (current_joy_x & 4)
			current_joy_x ^= 3;
		if (current_joy_x & 2)
			current_joy_x ^= 1;
		message |= current_joy_x << 3;

		if (current_joy_y & 4)
			current_joy_y ^= 3;
		if (current_joy_y & 2)
			current_joy_y ^= 1;
		message |= current_joy_y << 7;

		/* set joystick address */
		message |= (num << 1) | 0x1;

		/* post message */
		post_message(message);
		return true;
	}
	return false;
}

/*
    ti99_handset_task()
    Manage handsets, posting an event if the state of any handset has changed.
*/
void ti99_handset_device::do_task()
{
	int i;

	if (m_buflen == 0)
	{
		/* poll every handset */
		for (i=0; i < MAX_HANDSETS; i++)
			if (poll_joystick(i)==true) return;
		for (i=0; i < MAX_HANDSETS; i++)
			if (poll_keyboard(i)==true) return;
	}
	else if (m_buflen == 3)
	{   /* update messages after they have been posted */
		if (m_buf & 1)
		{   /* keyboard */
			poll_keyboard((~(m_buf >> 1)) & 0x3);
		}
		else
		{   /* joystick */
			poll_joystick((~(m_buf >> 1)) & 0x3);
		}
	}
}

void ti99_handset_device::pulse_clock()
{
	logerror("handset: pulse_clock\n");
	do_task();
}

void ti99_handset_device::device_start(void)
{
	m_delay_timer = timer_alloc(DELAY_TIMER);
}

void ti99_handset_device::device_reset(void)
{
	if (VERBOSE>5) LOG("ti99_handset_device: Reset\n");
	m_delay_timer->enable(true);
	m_buf = 0;
	m_buflen = 0;
	m_clock_high = false;
	m_ack = 0;
}

#define JOYSTICK_DELTA          10
#define JOYSTICK_SENSITIVITY    100

INPUT_PORTS_START( handset )
	/* 13 pseudo-ports for IR remote handsets */

	/* 8 pseudo-ports for the 4 IR joysticks */
	PORT_START("JOY0")  /* joystick 1, X axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(1)

	PORT_START("JOY1")  /* joystick 1, Y axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("JOY2")  /* joystick 2, X axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(2)

	PORT_START("JOY3")  /* joystick 2, Y axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("JOY4")  /* joystick 3, X axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(3)

	PORT_START("JOY5")  /* joystick 3, Y axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(3) PORT_REVERSE

	PORT_START("JOY6")  /* joystick 4, X axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(4)

	PORT_START("JOY7")  /* joystick 4, Y axis */
		PORT_BIT( 0xf, 0x7,  IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0,0xe) PORT_PLAYER(4) PORT_REVERSE

	/* 5 pseudo-ports for the 4 IR remote keypads */
	PORT_START("KP0")   /* keypad 1, keys 1 to 16 */
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: CLR") PORT_CODE(KEYCODE_1) PORT_PLAYER(1)
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: GO") PORT_CODE(KEYCODE_Q) PORT_PLAYER(1)
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: SET") PORT_CODE(KEYCODE_SPACE) PORT_PLAYER(1)
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: NEXT") PORT_CODE(KEYCODE_LSHIFT) PORT_PLAYER(1)
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 7") PORT_CODE(KEYCODE_2) PORT_PLAYER(1)
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 4") PORT_CODE(KEYCODE_W) PORT_PLAYER(1)
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 1") PORT_CODE(KEYCODE_A) PORT_PLAYER(1)
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: STOP") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 8") PORT_CODE(KEYCODE_3) PORT_PLAYER(1)
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 5") PORT_CODE(KEYCODE_E) PORT_PLAYER(1)
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 2") PORT_CODE(KEYCODE_S) PORT_PLAYER(1)
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 0") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 9") PORT_CODE(KEYCODE_4) PORT_PLAYER(1)
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 6") PORT_CODE(KEYCODE_R) PORT_PLAYER(1)
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: 3") PORT_CODE(KEYCODE_D) PORT_PLAYER(1)
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: E =") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)

	PORT_START("KP1")   /* keypad 1, keys 17 to 20 */
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: (div)") PORT_CODE(KEYCODE_5) PORT_PLAYER(1)
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: (mul)") PORT_CODE(KEYCODE_T) PORT_PLAYER(1)
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: NO -") PORT_CODE(KEYCODE_F) PORT_PLAYER(1)
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1: YES +") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)
				/* keypad 2, keys 1 to 12 */
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: CLR") PORT_CODE(KEYCODE_6) PORT_PLAYER(2)
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: GO") PORT_CODE(KEYCODE_Y) PORT_PLAYER(2)
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: SET") PORT_CODE(KEYCODE_G) PORT_PLAYER(2)
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: NEXT") PORT_CODE(KEYCODE_B) PORT_PLAYER(2)
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 7") PORT_CODE(KEYCODE_7) PORT_PLAYER(2)
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 4") PORT_CODE(KEYCODE_U) PORT_PLAYER(2)
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 1") PORT_CODE(KEYCODE_H) PORT_PLAYER(2)
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: STOP") PORT_CODE(KEYCODE_N) PORT_PLAYER(2)
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 8") PORT_CODE(KEYCODE_8) PORT_PLAYER(2)
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 5") PORT_CODE(KEYCODE_I) PORT_PLAYER(2)
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 2") PORT_CODE(KEYCODE_J) PORT_PLAYER(2)
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 0") PORT_CODE(KEYCODE_M) PORT_PLAYER(2)

	PORT_START("KP2")   /* keypad 2, keys 13 to 20 */
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 9") PORT_CODE(KEYCODE_9) PORT_PLAYER(2)
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 6") PORT_CODE(KEYCODE_O) PORT_PLAYER(2)
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: 3") PORT_CODE(KEYCODE_K) PORT_PLAYER(2)
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: E =") PORT_CODE(KEYCODE_STOP) PORT_PLAYER(2)
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: (div)") PORT_CODE(KEYCODE_0) PORT_PLAYER(2)
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: (mul)") PORT_CODE(KEYCODE_P) PORT_PLAYER(2)
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: NO -") PORT_CODE(KEYCODE_L) PORT_PLAYER(2)
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2: YES +") PORT_CODE(KEYCODE_ENTER) PORT_PLAYER(2)
				/* keypad 3, keys 1 to 8 */
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: CLR") PORT_CODE(KEYCODE_1) PORT_PLAYER(3)
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: GO") PORT_CODE(KEYCODE_Q) PORT_PLAYER(3)
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: SET") PORT_CODE(KEYCODE_SPACE) PORT_PLAYER(3)
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: NEXT") PORT_CODE(KEYCODE_LSHIFT) PORT_PLAYER(3)
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 7") PORT_CODE(KEYCODE_2) PORT_PLAYER(3)
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 4") PORT_CODE(KEYCODE_W) PORT_PLAYER(3)
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 1") PORT_CODE(KEYCODE_A) PORT_PLAYER(3)
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: STOP") PORT_CODE(KEYCODE_Z) PORT_PLAYER(3)

	PORT_START("KP3")   /* keypad 3, keys 9 to 20 */
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 8") PORT_CODE(KEYCODE_3) PORT_PLAYER(3)
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 5") PORT_CODE(KEYCODE_E) PORT_PLAYER(3)
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 2") PORT_CODE(KEYCODE_S) PORT_PLAYER(3)
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 0") PORT_CODE(KEYCODE_X) PORT_PLAYER(3)
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 9") PORT_CODE(KEYCODE_4) PORT_PLAYER(3)
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 6") PORT_CODE(KEYCODE_R) PORT_PLAYER(3)
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: 3") PORT_CODE(KEYCODE_D) PORT_PLAYER(3)
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: E =") PORT_CODE(KEYCODE_C) PORT_PLAYER(3)
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: (div)") PORT_CODE(KEYCODE_5) PORT_PLAYER(3)
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: (mul)") PORT_CODE(KEYCODE_T) PORT_PLAYER(3)
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: NO -") PORT_CODE(KEYCODE_F) PORT_PLAYER(3)
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3: YES +") PORT_CODE(KEYCODE_V) PORT_PLAYER(3)
				/* keypad 4, keys 1 to 4 */
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: CLR") PORT_CODE(KEYCODE_6) PORT_PLAYER(4)
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: GO") PORT_CODE(KEYCODE_Y) PORT_PLAYER(4)
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: SET") PORT_CODE(KEYCODE_G) PORT_PLAYER(4)
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: NEXT") PORT_CODE(KEYCODE_B) PORT_PLAYER(4)

	PORT_START("KP4")   /* keypad 4, keys 5 to 20 */
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 7") PORT_CODE(KEYCODE_7) PORT_PLAYER(4)
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 4") PORT_CODE(KEYCODE_U) PORT_PLAYER(4)
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 1") PORT_CODE(KEYCODE_H) PORT_PLAYER(4)
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: STOP") PORT_CODE(KEYCODE_N) PORT_PLAYER(4)
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 8") PORT_CODE(KEYCODE_8) PORT_PLAYER(4)
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 5") PORT_CODE(KEYCODE_I) PORT_PLAYER(4)
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 2") PORT_CODE(KEYCODE_J) PORT_PLAYER(4)
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 0") PORT_CODE(KEYCODE_M) PORT_PLAYER(4)
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 9") PORT_CODE(KEYCODE_9) PORT_PLAYER(4)
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 6") PORT_CODE(KEYCODE_O) PORT_PLAYER(4)
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: 3") PORT_CODE(KEYCODE_K) PORT_PLAYER(4)
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: E =") PORT_CODE(KEYCODE_STOP) PORT_PLAYER(4)
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: (div)") PORT_CODE(KEYCODE_0) PORT_PLAYER(4)
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: (mul)") PORT_CODE(KEYCODE_P) PORT_PLAYER(4)
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: NO -") PORT_CODE(KEYCODE_L) PORT_PLAYER(4)
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4: YES +") PORT_CODE(KEYCODE_ENTER) PORT_PLAYER(4)
INPUT_PORTS_END

ioport_constructor ti99_handset_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( handset );
}

const device_type HANDSET = &device_creator<ti99_handset_device>;

/******************************************************************************
    Twin Joystick
******************************************************************************/

/* col 6: "wired handset 1" (= joystick 1) */
/* col 7: "wired handset 2" (= joystick 2) */

INPUT_PORTS_START( joysticks )
	PORT_START("JOY1")
		PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP/*, "(1UP)", CODE_NONE, OSD_JOY_UP*/) PORT_PLAYER(1)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN/*, "(1DOWN)", CODE_NONE, OSD_JOY_DOWN, 0*/) PORT_PLAYER(1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT/*, "(1RIGHT)", CODE_NONE, OSD_JOY_RIGHT, 0*/) PORT_PLAYER(1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT/*, "(1LEFT)", CODE_NONE, OSD_JOY_LEFT, 0*/) PORT_PLAYER(1)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1/*, "(1FIRE)", CODE_NONE, OSD_JOY_FIRE, 0*/) PORT_PLAYER(1)

	PORT_START("JOY2")
		PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP/*, "(2UP)", CODE_NONE, OSD_JOY2_UP, 0*/) PORT_PLAYER(2)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN/*, "(2DOWN)", CODE_NONE, OSD_JOY2_DOWN, 0*/) PORT_PLAYER(2)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT/*, "(2RIGHT)", CODE_NONE, OSD_JOY2_RIGHT, 0*/) PORT_PLAYER(2)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT/*, "(2LEFT)", CODE_NONE, OSD_JOY2_LEFT, 0*/) PORT_PLAYER(2)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1/*, "(2FIRE)", CODE_NONE, OSD_JOY2_FIRE, 0*/) PORT_PLAYER(2)
INPUT_PORTS_END

ti99_twin_joystick::ti99_twin_joystick(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: joyport_attached_device(mconfig, HANDSET, "TI-99/4(A) Twin Joystick", tag, owner, clock, "twinjoy", __FILE__), m_joystick(0)
{
}

void ti99_twin_joystick::device_start(void)
{
}

/*
    Return the status of the joysticks.

    answer = |0|0|0|U|D|R|L|B|
*/
UINT8 ti99_twin_joystick::read_dev()
{
	UINT8 value;
	if (m_joystick==1) value = ioport("JOY1")->read();
	else
	{
		if (m_joystick==2) value = ioport("JOY2")->read();
		else value = 0xff;
	}
	if (VERBOSE>6) LOG("ti99_twin_joystick: joy%d = %02x\n", m_joystick, value);
	return value;
}

void ti99_twin_joystick::write_dev(UINT8 data)
{
	if (VERBOSE>7) LOG("ti99_twin_joystick: Select joystick %d\n", data);
	m_joystick = data & 0x03;
}

ioport_constructor ti99_twin_joystick::device_input_ports() const
{
	return INPUT_PORTS_NAME( joysticks );
}

const device_type TI99_JOYSTICK = &device_creator<ti99_twin_joystick>;
