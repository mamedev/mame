// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldvp931.c

    Philips 22VP931 laserdisc emulation.

**************************************************************************

    Still to do:

        * determine actual slow/fast speeds
        *

*************************************************************************/


#include "emu.h"
#include "ldvp931.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_COMMANDS                0
#define LOG_PORTS                   0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// scanning speeds
#define SCAN_SPEED                      (2000 / 30)         // 2000 frames/second
#define SCAN_FAST_SPEED                 (4000 / 30)         // 4000 frames/second



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type PHILLIPS_22VP931 = &device_creator<phillips_22vp931_device>;



//**************************************************************************
//  22VP931 ROM AND MACHINE INTERFACES
//**************************************************************************

static ADDRESS_MAP_START( vp931_portmap, AS_IO, 8, phillips_22vp931_device )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xcf) AM_READWRITE(i8049_keypad_r, i8049_output0_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0xcf) AM_READWRITE(i8049_unknown_r, i8049_output1_w)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0xcf) AM_READWRITE(i8049_datic_r, i8049_lcd_w)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0xcf) AM_READWRITE(i8049_from_controller_r, i8049_to_controller_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(i8049_port1_r, i8049_port1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(i8049_port2_r, i8049_port2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(i8049_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(i8049_t1_r)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( vp931 )
	MCFG_CPU_ADD("vp931", I8049, XTAL_11MHz)
	MCFG_CPU_IO_MAP(vp931_portmap)
MACHINE_CONFIG_END


ROM_START( vp931 )
	ROM_REGION( 0x800, "vp931", 0 )
	ROM_LOAD( "at-6-1_a.bin", 0x000, 0x800, CRC(e11b3c8d) SHA1(ea2d7f6a044ed085ce5e09d8b1b1a21c37f0e9b8) )
ROM_END



//**************************************************************************
//  PHILLIPS 22VP931 IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  phillips_22vp931_device - constructor
//-------------------------------------------------

phillips_22vp931_device::phillips_22vp931_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: laserdisc_device(mconfig, PHILLIPS_22VP931, "Phillips 22VP931", tag, owner, clock, "22vp931", __FILE__),
		m_i8049_cpu(*this, "vp931"),
		m_tracktimer(NULL),
		m_i8049_out0(0),
		m_i8049_out1(0),
		m_i8049_port1(0),
		m_daticval(0),
		m_daticerp(0),
		m_datastrobe(0),
		m_fromcontroller(0),
		m_fromcontroller_pending(false),
		m_tocontroller(0),
		m_tocontroller_pending(false),
		m_trackdir(0),
		m_trackstate(0),
		m_cmdcount(0),
		m_advanced(0)
{
}


//-------------------------------------------------
//  reset_w - write to the reset line
//-------------------------------------------------

void phillips_22vp931_device::reset_w(UINT8 data)
{
	// control the CPU state
	m_i8049_cpu->set_input_line(INPUT_LINE_RESET, data);

	// on an assert, reset the device state as well
	if (data == ASSERT_LINE)
		reset();
}


//-------------------------------------------------
//  data_r - handle a parallel data read from the
//  22VP931
//-------------------------------------------------

UINT8 phillips_22vp931_device::data_r()
{
	// if data is pending, clear the pending flag and notify any callbacks
	if (m_tocontroller_pending)
	{
		m_tocontroller_pending = false;
		if (!m_data_ready.isnull())
			m_data_ready(*this, false);
	}

	// also boost interleave for 4 scanlines to ensure proper communications
	machine().scheduler().boost_interleave(attotime::zero, screen().scan_period() * 4);
	return m_tocontroller;
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void phillips_22vp931_device::device_start()
{
	// pass through to the parent
	laserdisc_device::device_start();

	// allocate a timer
	m_tracktimer = timer_alloc(TID_HALF_TRACK);
}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void phillips_22vp931_device::device_reset()
{
	// pass through to the parent
	laserdisc_device::device_reset();

	// reset our state
	m_i8049_out0 = 0;
	m_i8049_out1 = 0;
	m_i8049_port1 = 0;

	m_daticval = 0;
	m_daticerp = 0;
	m_datastrobe = 0;

	m_fromcontroller = 0;
	m_fromcontroller_pending = false;
	m_tocontroller = 0;
	m_tocontroller_pending = false;

	m_trackdir = 0;
	m_trackstate = 0;

	m_cmdcount = 0;
	m_advanced = 0;
}


//-------------------------------------------------
//  device_timer - handle timers set by this
//  device
//-------------------------------------------------

void phillips_22vp931_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_VBI_DATA_FETCH:
		{
			UINT32 line = param >> 2;
			int which = param & 3;
			UINT32 code = 0;

			// fetch the code and compute the DATIC latched value
			if (line >= LASERDISC_CODE_LINE16 && line <= LASERDISC_CODE_LINE18)
				code = get_field_code(laserdisc_field_code(line), false);

			// at the start of each line, signal an interrupt and use a timer to turn it off
			if (which == 0)
			{
				m_i8049_cpu->set_input_line(MCS48_INPUT_IRQ, ASSERT_LINE);
				timer_set(attotime::from_nsec(5580), TID_IRQ_OFF);
			}

			// clock the data strobe on each subsequent callback
			else if (code != 0)
			{
				m_daticval = code >> (8 * (3 - which));
				m_datastrobe = 1;
				timer_set(attotime::from_nsec(5000), TID_DATA_STROBE_OFF);
			}

			// determine the next bit to fetch and reprime ourself
			if (++which == 4)
			{
				which = 0;
				line++;
			}
			if (line <= LASERDISC_CODE_LINE18 + 1)
				timer_set(screen().time_until_pos(line*2, which * 2 * screen().width() / 4), TID_VBI_DATA_FETCH, (line << 2) + which);
			break;
		}

		case TID_DEFERRED_DATA:
			// set the value and mark it pending
			if (LOG_COMMANDS && m_fromcontroller_pending)
				printf("Dropped previous command byte\n");
			m_fromcontroller = param;
			m_fromcontroller_pending = true;

			// track the commands for debugging purposes
			if (m_cmdcount < ARRAY_LENGTH(m_cmdbuf))
			{
				m_cmdbuf[m_cmdcount++ % 3] = param;
				if (LOG_COMMANDS && m_cmdcount % 3 == 0)
					printf("Cmd: %02X %02X %02X\n", m_cmdbuf[0], m_cmdbuf[1], m_cmdbuf[2]);
			}
			break;

		case TID_IRQ_OFF:
			m_i8049_cpu->set_input_line(MCS48_INPUT_IRQ, CLEAR_LINE);
			break;

		case TID_DATA_STROBE_OFF:
			m_datastrobe = 0;
			break;

		case TID_ERP_OFF:
			m_daticerp = 0;
			break;

		case TID_HALF_TRACK:
			// advance by the count and toggle the state
			m_trackstate ^= 1;
			if ((m_trackdir < 0 && !m_trackstate) || (m_trackdir > 0 && m_trackstate))
			{
				advance_slider(m_trackdir);
				m_advanced += m_trackdir;
			}
			break;

		// pass everything else onto the parent
		default:
			laserdisc_device::device_timer(timer, id, param, ptr);
			break;
	}
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const rom_entry *phillips_22vp931_device::device_rom_region() const
{
	return ROM_NAME(vp931);
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  our machine config fragment
//-------------------------------------------------

machine_config_constructor phillips_22vp931_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(vp931);
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void phillips_22vp931_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// reset our command counter (debugging only)
	m_cmdcount = 0;

	// set the ERP signal to 1 to indicate start of frame, and set a timer to turn it off
	m_daticerp = 1;
	timer_set(screen().time_until_pos(15*2), TID_ERP_OFF);
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

INT32 phillips_22vp931_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// set the first VBI timer to go at the start of line 16
	timer_set(screen().time_until_pos(16*2), TID_VBI_DATA_FETCH, LASERDISC_CODE_LINE16 << 2);

	// play forward by default
	return fieldnum;
}


//-------------------------------------------------
//  i8049_output0_w - controls audio/video squelch
//  and other bits
//-------------------------------------------------

WRITE8_MEMBER( phillips_22vp931_device::i8049_output0_w )
{
	/*
	    $80 = n/c
	    $40 = LED (?) -> C335
	    $20 = LED (?)
	    $10 = LED (?) -> CX
	    $08 = EJECT
	    $04 = inverted -> AUDIO MUTE II
	    $02 = inverted -> AUDIO MUTE I
	    $01 = inverted -> VIDEO MUTE
	*/

	if (LOG_PORTS && (m_i8049_out0 ^ data) & 0xff)
	{
		printf("%03X:out0:", space.device().safe_pc());
		if ( (data & 0x80)) printf(" ???");
		if ( (data & 0x40)) printf(" LED1");
		if ( (data & 0x20)) printf(" LED2");
		if ( (data & 0x10)) printf(" LED3");
		if ( (data & 0x08)) printf(" EJECT");
		if (!(data & 0x04)) printf(" AUDMUTE2");
		if (!(data & 0x02)) printf(" AUDMUTE1");
		if (!(data & 0x01)) printf(" VIDMUTE");
		printf("\n");
		m_i8049_out0 = data;
	}

	// update a/v squelch
	set_audio_squelch(!(data & 0x02), !(data & 0x04));
	set_video_squelch(!(data & 0x01));
}


//-------------------------------------------------
//  i8049_output1_w - controls scanning behaviors
//-------------------------------------------------

WRITE8_MEMBER( phillips_22vp931_device::i8049_output1_w )
{
	/*
	    $80 = n/c
	    $40 = n/c
	    $20 = n/c
	    $10 = n/c
	    $08 = inverted -> SMS
	    $04 = inverted -> SSS
	    $02 = inverted -> SCAN CMD
	    $01 = OSM
	*/

	INT32 speed = 0;

	if (LOG_PORTS && (m_i8049_out1 ^ data) & 0x08)
	{
		osd_printf_debug("%03X:out1:", space.device().safe_pc());
		if (!(data & 0x08)) osd_printf_debug(" SMS");
		osd_printf_debug("\n");
		m_i8049_out1 = data;
	}

	// speed is 0 unless SCAN CMD is clear
	speed = 0;
	if (!(data & 0x02))
	{
		// fast/slow is based on bit 2
		speed = (data & 0x04) ? SCAN_FAST_SPEED : SCAN_SPEED;

		// direction is based on bit 0
		if (data & 0x01)
			speed = -speed;
	}

	// update the speed
	set_slider_speed(speed);
}


//-------------------------------------------------
//  i8049_lcd_w - vestigial LCD frame display
//-------------------------------------------------

WRITE8_MEMBER( phillips_22vp931_device::i8049_lcd_w )
{
	/*
	    Frame number is written as 5 digits here; however, it is not actually
	    connected
	*/
}


//-------------------------------------------------
//  i8049_unknown_r - unknown input port
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_unknown_r )
{
	// only bit $80 is checked and its effects are minor
	return 0x00;
}


//-------------------------------------------------
//  i8049_keypad_r - vestigial keypad/button
//  controls
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_keypad_r )
{
	/*
	    From the code, this is apparently a vestigial keypad with basic controls:
	        $01 = play
	        $02 = still
	        $04 = jump 25 frames backward
	        $08 = jump 25 frames forward
	        $10 = search for frame 50(?)
	        $20 = search for frame 350(?)
	        $40 = reset
	        $80 = play reverse
	*/
	return 0x00;
}


//-------------------------------------------------
//  i8049_datic_r - read the latched value from the
//  DATIC circuit
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_datic_r )
{
	return m_daticval;
}


//-------------------------------------------------
//  i8049_from_controller_r - read the value the
//  external controller wrote
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_from_controller_r )
{
	// clear the pending flag and return the data
	m_fromcontroller_pending = false;
	return m_fromcontroller;
}


//-------------------------------------------------
//  i8049_to_controller_w - write a value back to
//  the external controller
//-------------------------------------------------

WRITE8_MEMBER( phillips_22vp931_device::i8049_to_controller_w )
{
	// set the pending flag and stash the data
	m_tocontroller_pending = TRUE;
	m_tocontroller = data;

	// signal to the callback if provided
	if (!m_data_ready.isnull())
		m_data_ready(*this, true);

	// also boost interleave for 4 scanlines to ensure proper communications
	machine().scheduler().boost_interleave(attotime::zero, screen().scan_period() * 4);
}


//-------------------------------------------------
//  i8049_port1_r - read the 8048 I/O port 1
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_port1_r )
{
	/*
	    $80 = P17 = (in) unsure
	    $40 = P16 = (in) /ERP from datic circuit
	    $20 = P15 = (in) D105
	*/

	UINT8 result = 0x00;
	if (!m_daticerp)
		result |= 0x40;
	return result;
}


//-------------------------------------------------
//  i8049_port1_w - write the 8048 I/O port 1
//-------------------------------------------------

WRITE8_MEMBER( phillips_22vp931_device::i8049_port1_w )
{
	/*
	    $10 = P14 = (out) D104 -> /SPEED
	    $08 = P13 = (out) D103 -> /TIMER ENABLE
	    $04 = P12 = (out) D102 -> /REV
	    $02 = P11 = (out) D101 -> /FORW
	    $01 = P10 = (out) D100 -> some op-amp then to C334, B56, B332
	*/

	if (LOG_PORTS && (m_i8049_port1 ^ data) & 0x1f)
	{
		printf("%03X:port1:", space.device().safe_pc());
		if (!(data & 0x10)) printf(" SPEED");
		if (!(data & 0x08)) printf(" TIMENABLE");
		if (!(data & 0x04)) printf(" REV");
		if (!(data & 0x02)) printf(" FORW");
		if (!(data & 0x01)) printf(" OPAMP");
		printf("\n");
	}

	// if bit 0 is set, we are not tracking
	if (data & 0x01)
		m_trackdir = 0;

	// if bit 0 is clear and we weren't tracking before, initialize the state
	else if (m_trackdir == 0)
	{
		m_advanced = 0;

		// if bit 2 is clear, we are moving backwards
		if (!(data & 0x04))
		{
			m_trackdir = -1;
			m_trackstate = 1;
		}

		// if bit 1 is clear, we are moving forward
		else if (!(data & 0x02))
		{
			m_trackdir = 1;
			m_trackstate = 0;
		}
	}

	// if we have a timer, adjust it
	if (m_tracktimer != NULL)
	{
		// turn it off if we're not tracking
		if (m_trackdir == 0)
			m_tracktimer->reset();

		// if we just started tracking, or if the speed was changed, reprime the timer
		else if (((m_i8049_port1 ^ data) & 0x11) != 0)
		{
			// speeds here are just guesses, but work with the player logic; this is the time per half-track
			attotime speed = (data & 0x10) ? attotime::from_usec(60) : attotime::from_usec(10);

			// always start with an initial long delay; the code expects this
			m_tracktimer->adjust(attotime::from_usec(100), 0, speed);
		}
	}

	m_i8049_port1 = data;
}


//-------------------------------------------------
//  i8049_port2_r - read from the 8048 I/O port 2
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_port2_r )
{
	/*
	    $80 = P27 = (in) set/reset latch; set by FOC LS, reset by IGR
	    $20 = P25 = (in) D125 -> 0 when data written to controller is preset, reset to 1 when read
	    $10 = P24 = (in) D124 -> 0 when data from controller is present, reset to 1 on a read
	*/

	UINT8 result = 0x00;
	if (!m_tocontroller_pending)
		result |= 0x20;
	if (!m_fromcontroller_pending)
		result |= 0x10;
	return result;
}


//-------------------------------------------------
//  i8049_port2_w - write the 8048 I/O port 2
//-------------------------------------------------

WRITE8_MEMBER( phillips_22vp931_device::i8049_port2_w )
{
	/*
	    $40 = P26 = (out) cleared while data is sent back & forth; set afterwards
	                [Not actually connected, but this is done in the code]
	*/
}


//-------------------------------------------------
//  i8049_t0_r - return the T0 line status, which is
//  connected to the DATIC's data strobe line
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_t0_r )
{
	return m_datastrobe;
}


//-------------------------------------------------
//  i8049_t1_r - return the T1 line status, which
//  is connected to the tracking state and is used
//  to count the number of tracks advanced
//-------------------------------------------------

READ8_MEMBER( phillips_22vp931_device::i8049_t1_r )
{
	return m_trackstate;
}
