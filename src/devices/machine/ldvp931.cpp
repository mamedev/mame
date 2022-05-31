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
DEFINE_DEVICE_TYPE(PHILIPS_22VP931, philips_22vp931_device, "22vp931", "Philips 22VP931")



//**************************************************************************
//  22VP931 ROM AND MACHINE INTERFACES
//**************************************************************************

void philips_22vp931_device::vp931_portmap(address_map &map)
{
	map(0x00, 0x00).mirror(0xcf).rw(FUNC(philips_22vp931_device::i8049_keypad_r), FUNC(philips_22vp931_device::i8049_output0_w));
	map(0x10, 0x10).mirror(0xcf).rw(FUNC(philips_22vp931_device::i8049_unknown_r), FUNC(philips_22vp931_device::i8049_output1_w));
	map(0x20, 0x20).mirror(0xcf).rw(FUNC(philips_22vp931_device::i8049_datic_r), FUNC(philips_22vp931_device::i8049_lcd_w));
	map(0x30, 0x30).mirror(0xcf).rw(FUNC(philips_22vp931_device::i8049_from_controller_r), FUNC(philips_22vp931_device::i8049_to_controller_w));
}


ROM_START( vp931 )
	ROM_REGION( 0x800, "vp931", 0 )
	ROM_LOAD( "at-6-1_a.bin", 0x000, 0x800, CRC(e11b3c8d) SHA1(ea2d7f6a044ed085ce5e09d8b1b1a21c37f0e9b8) )
ROM_END



//**************************************************************************
//  PHILIPS 22VP931 IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  philips_22vp931_device - constructor
//-------------------------------------------------

philips_22vp931_device::philips_22vp931_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, PHILIPS_22VP931, tag, owner, clock),
		m_i8049_cpu(*this, "vp931"),
		m_process_vbi_timer(nullptr),
		m_irq_off_timer(nullptr),
		m_strobe_off_timer(nullptr),
		m_erp_off_timer(nullptr),
		m_track_timer(nullptr),
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

void philips_22vp931_device::reset_w(uint8_t data)
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

uint8_t philips_22vp931_device::data_r()
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

void philips_22vp931_device::device_start()
{
	// pass through to the parent
	laserdisc_device::device_start();

	// allocate timers
	m_initial_vbi_timer = timer_alloc(FUNC(philips_22vp931_device::process_vbi_data), this);
	m_process_vbi_timer = timer_alloc(FUNC(philips_22vp931_device::process_vbi_data), this);
	m_irq_off_timer = timer_alloc(FUNC(philips_22vp931_device::irq_off), this);
	m_strobe_off_timer = timer_alloc(FUNC(philips_22vp931_device::data_strobe_off), this);
	m_erp_off_timer = timer_alloc(FUNC(philips_22vp931_device::erp_off), this);
	m_track_timer = timer_alloc(FUNC(philips_22vp931_device::half_track_tick), this);
}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void philips_22vp931_device::device_reset()
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
//  process_vbi_data - process VBI data which was
//  fetched by the parent device
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(philips_22vp931_device::process_vbi_data)
{
	uint32_t line = param >> 2;
	int which = param & 3;
	uint32_t code = 0;

	// fetch the code and compute the DATIC latched value
	if (line >= LASERDISC_CODE_LINE16 && line <= LASERDISC_CODE_LINE18)
		code = get_field_code(laserdisc_field_code(line), false);

	// at the start of each line, signal an interrupt and use a timer to turn it off
	if (which == 0)
	{
		m_i8049_cpu->set_input_line(MCS48_INPUT_IRQ, ASSERT_LINE);
		m_irq_off_timer->adjust(attotime::from_nsec(5580));
	}

	// clock the data strobe on each subsequent callback
	else if (code != 0)
	{
		m_daticval = code >> (8 * (3 - which));
		m_datastrobe = 1;
		m_strobe_off_timer->adjust(attotime::from_nsec(5000));
	}

	// determine the next bit to fetch and reprime ourself
	if (++which == 4)
	{
		which = 0;
		line++;
	}
	if (line <= LASERDISC_CODE_LINE18 + 1)
		m_process_vbi_timer->adjust(screen().time_until_pos(line*2, which * 2 * screen().width() / 4), (line << 2) + which);
}


//-------------------------------------------------
//  process_deferred_data -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(philips_22vp931_device::process_deferred_data)
{
	// set the value and mark it pending
	if (LOG_COMMANDS && m_fromcontroller_pending)
		printf("Dropped previous command byte\n");
	m_fromcontroller = param;
	m_fromcontroller_pending = true;

	// track the commands for debugging purposes
	if (m_cmdcount < std::size(m_cmdbuf))
	{
		m_cmdbuf[m_cmdcount++ % 3] = param;
		if (LOG_COMMANDS && m_cmdcount % 3 == 0)
			printf("Cmd: %02X %02X %02X\n", m_cmdbuf[0], m_cmdbuf[1], m_cmdbuf[2]);
	}
}


//-------------------------------------------------
//  irq_off -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(philips_22vp931_device::irq_off)
{
	m_i8049_cpu->set_input_line(MCS48_INPUT_IRQ, CLEAR_LINE);
}


//-------------------------------------------------
//  data_strobe_off -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(philips_22vp931_device::data_strobe_off)
{
	m_datastrobe = 0;
}


//-------------------------------------------------
//  erp_off -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(philips_22vp931_device::erp_off)
{
	m_daticerp = 0;
}


//-------------------------------------------------
//  half_track_tick - advance the slider by the
//  current count and toggle the track state
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(philips_22vp931_device::half_track_tick)
{
	m_trackstate ^= 1;
	if ((m_trackdir < 0 && !m_trackstate) || (m_trackdir > 0 && m_trackstate))
	{
		advance_slider(m_trackdir);
		m_advanced += m_trackdir;
	}
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const tiny_rom_entry *philips_22vp931_device::device_rom_region() const
{
	return ROM_NAME(vp931);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void philips_22vp931_device::device_add_mconfig(machine_config &config)
{
	I8049(config, m_i8049_cpu, XTAL(11'000'000));
	m_i8049_cpu->set_addrmap(AS_IO, &philips_22vp931_device::vp931_portmap);
	m_i8049_cpu->p1_in_cb().set(FUNC(philips_22vp931_device::i8049_port1_r));
	m_i8049_cpu->p1_out_cb().set(FUNC(philips_22vp931_device::i8049_port1_w));
	m_i8049_cpu->p2_in_cb().set(FUNC(philips_22vp931_device::i8049_port2_r));
	m_i8049_cpu->p2_out_cb().set(FUNC(philips_22vp931_device::i8049_port2_w));
	m_i8049_cpu->t0_in_cb().set(FUNC(philips_22vp931_device::i8049_t0_r));
	m_i8049_cpu->t1_in_cb().set(FUNC(philips_22vp931_device::i8049_t1_r));
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void philips_22vp931_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// reset our command counter (debugging only)
	m_cmdcount = 0;

	// set the ERP signal to 1 to indicate start of frame, and set a timer to turn it off
	m_daticerp = 1;
	m_erp_off_timer->adjust(screen().time_until_pos(15*2));
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

int32_t philips_22vp931_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// player_update is invoked by the parent device at line 16, so call our VBI processing timer directly
	m_initial_vbi_timer->adjust(screen().time_until_pos(16*2), LASERDISC_CODE_LINE16 << 2);
	//process_vbi_data(LASERDISC_CODE_LINE16 << 2);

	// play forward by default
	return fieldnum;
}


//-------------------------------------------------
//  i8049_output0_w - controls audio/video squelch
//  and other bits
//-------------------------------------------------

void philips_22vp931_device::i8049_output0_w(uint8_t data)
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
		std::string flags;
		if ( (data & 0x80)) flags += " ???";
		if ( (data & 0x40)) flags += " LED1";
		if ( (data & 0x20)) flags += " LED2";
		if ( (data & 0x10)) flags += " LED3";
		if ( (data & 0x08)) flags += " EJECT";
		if (!(data & 0x04)) flags += " AUDMUTE2";
		if (!(data & 0x02)) flags += " AUDMUTE1";
		if (!(data & 0x01)) flags += " VIDMUTE";

		logerror("out0: %s %s\n", flags, machine().describe_context());
		m_i8049_out0 = data;
	}

	// update a/v squelch
	set_audio_squelch(!(data & 0x02), !(data & 0x04));
	set_video_squelch(!(data & 0x01));
}


//-------------------------------------------------
//  i8049_output1_w - controls scanning behaviors
//-------------------------------------------------

void philips_22vp931_device::i8049_output1_w(uint8_t data)
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

	int32_t speed;

	if (LOG_PORTS && (m_i8049_out1 ^ data) & 0x08)
	{
		std::string flags;
		if (!(data & 0x08)) flags += " SMS";
		logerror("out1: %s %s\n", flags, machine().describe_context());
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

void philips_22vp931_device::i8049_lcd_w(uint8_t data)
{
	/*
	    Frame number is written as 5 digits here; however, it is not actually
	    connected
	*/
}


//-------------------------------------------------
//  i8049_unknown_r - unknown input port
//-------------------------------------------------

uint8_t philips_22vp931_device::i8049_unknown_r()
{
	// only bit $80 is checked and its effects are minor
	return 0x00;
}


//-------------------------------------------------
//  i8049_keypad_r - vestigial keypad/button
//  controls
//-------------------------------------------------

uint8_t philips_22vp931_device::i8049_keypad_r()
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

uint8_t philips_22vp931_device::i8049_datic_r()
{
	return m_daticval;
}


//-------------------------------------------------
//  i8049_from_controller_r - read the value the
//  external controller wrote
//-------------------------------------------------

uint8_t philips_22vp931_device::i8049_from_controller_r()
{
	// clear the pending flag and return the data
	m_fromcontroller_pending = false;
	return m_fromcontroller;
}


//-------------------------------------------------
//  i8049_to_controller_w - write a value back to
//  the external controller
//-------------------------------------------------

void philips_22vp931_device::i8049_to_controller_w(uint8_t data)
{
	// set the pending flag and stash the data
	m_tocontroller_pending = true;
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

uint8_t philips_22vp931_device::i8049_port1_r()
{
	/*
	    $80 = P17 = (in) unsure
	    $40 = P16 = (in) /ERP from datic circuit
	    $20 = P15 = (in) D105
	*/

	uint8_t result = 0x00;
	if (!m_daticerp)
		result |= 0x40;
	return result;
}


//-------------------------------------------------
//  i8049_port1_w - write the 8048 I/O port 1
//-------------------------------------------------

void philips_22vp931_device::i8049_port1_w(uint8_t data)
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
		std::string flags;
		if (!(data & 0x10)) flags += " SPEED";
		if (!(data & 0x08)) flags += " TIMENABLE";
		if (!(data & 0x04)) flags += " REV";
		if (!(data & 0x02)) flags += " FORW";
		if (!(data & 0x01)) flags += " OPAMP";
		logerror("port1: %s %s\n", flags, machine().describe_context());
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

	// turn off the track timer if we're not tracking
	if (m_trackdir == 0)
		m_track_timer->reset();

	// if we just started tracking, or if the speed was changed, reprime the timer
	else if (((m_i8049_port1 ^ data) & 0x11) != 0)
	{
		// speeds here are just guesses, but work with the player logic; this is the time per half-track
		attotime speed = (data & 0x10) ? attotime::from_usec(60) : attotime::from_usec(10);

		// always start with an initial long delay; the code expects this
		m_track_timer->adjust(attotime::from_usec(100), 0, speed);
	}

	m_i8049_port1 = data;
}


//-------------------------------------------------
//  i8049_port2_r - read from the 8048 I/O port 2
//-------------------------------------------------

uint8_t philips_22vp931_device::i8049_port2_r()
{
	/*
	    $80 = P27 = (in) set/reset latch; set by FOC LS, reset by IGR
	    $20 = P25 = (in) D125 -> 0 when data written to controller is preset, reset to 1 when read
	    $10 = P24 = (in) D124 -> 0 when data from controller is present, reset to 1 on a read
	*/

	uint8_t result = 0x00;
	if (!m_tocontroller_pending)
		result |= 0x20;
	if (!m_fromcontroller_pending)
		result |= 0x10;
	return result;
}


//-------------------------------------------------
//  i8049_port2_w - write the 8048 I/O port 2
//-------------------------------------------------

void philips_22vp931_device::i8049_port2_w(uint8_t data)
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

int philips_22vp931_device::i8049_t0_r()
{
	return m_datastrobe;
}


//-------------------------------------------------
//  i8049_t1_r - return the T1 line status, which
//  is connected to the tracking state and is used
//  to count the number of tracks advanced
//-------------------------------------------------

int philips_22vp931_device::i8049_t1_r()
{
	return m_trackstate;
}
