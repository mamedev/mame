// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldv1000.c

    Pioneer LD-V1000 laserdisc emulation.

**************************************************************************

    Still to do:

        * fix issues
        * add OSD

*************************************************************************/


#include "emu.h"
#include "ldv1000.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_PORT_IO                 0
#define LOG_STATUS_CHANGES          0
#define LOG_FRAMES_SEEN             0
#define LOG_COMMANDS                0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define SCAN_SPEED                      (2000 / 30)         // 2000 frames/second
#define SEEK_FAST_SPEED                 (4000 / 30)         // 4000 frames/second

#define MULTIJUMP_TRACK_TIME            attotime::from_usec(50)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(PIONEER_LDV1000, pioneer_ldv1000_device, "ldv1000", "Pioneer LD-V1000")



//**************************************************************************
//  LD-V1000 ROM AND MACHINE INTERFACES
//**************************************************************************

void pioneer_ldv1000_device::ldv1000_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).rom();
	map(0x8000, 0x87ff).mirror(0x3800).ram();
	map(0xc000, 0xc003).mirror(0x1ff0).rw("ldvppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc004, 0xc007).mirror(0x1ff0).rw("ldvppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


void pioneer_ldv1000_device::ldv1000_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).mirror(0x38).rw(FUNC(pioneer_ldv1000_device::z80_decoder_display_port_r), FUNC(pioneer_ldv1000_device::z80_decoder_display_port_w));
	map(0x40, 0x40).mirror(0x3f).r(FUNC(pioneer_ldv1000_device::z80_controller_r));
	map(0x80, 0x80).mirror(0x3f).w(FUNC(pioneer_ldv1000_device::z80_controller_w));
	map(0xc0, 0xc3).mirror(0x3c).rw(m_z80_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


static const z80_daisy_config daisy_chain[] =
{
	{ "ldvctc" },
	{ nullptr }
};


ROM_START( ldv1000 )
	ROM_REGION( 0x2000, "ldv1000", 0 )
	ROM_LOAD( "z03_1001_vyw-053_v1-0.bin", 0x0000, 0x2000, CRC(31ec4687) SHA1(52f91c304a878ba02b2fa1cda1a9489d6dd5a34f) )
ROM_END



//**************************************************************************
//  PIONEER LD-V1000 IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  pioneer_ldv1000_device - constructor
//-------------------------------------------------

pioneer_ldv1000_device::pioneer_ldv1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, PIONEER_LDV1000, tag, owner, clock),
		m_z80_cpu(*this, "ldv1000"),
		m_z80_ctc(*this, "ldvctc"),
		m_multitimer(nullptr),
		m_command_strobe_cb(*this),
		m_command(0),
		m_status(0),
		m_vsync(false),
		m_counter_start(0),
		m_counter(0),
		m_portc0(0),
		m_portb1(0),
		m_portc1(0),
		m_portselect(0),
		m_dispindex(0),
		m_vbiready(false),
		m_vbiindex(0)
{
}


//-------------------------------------------------
//  data_w - handle a parallel data write to the
//  LD-V1000
//-------------------------------------------------

void pioneer_ldv1000_device::data_w(uint8_t data)
{
	m_command = data;
	if (LOG_COMMANDS)
		logerror("-> COMMAND = %02X (%s)\n", data, (m_portc1 & 0x10) ? "valid" : "invalid");
}


//-------------------------------------------------
//  enter_w - set the state of the ENTER strobe
//-------------------------------------------------

void pioneer_ldv1000_device::enter_w(uint8_t data)
{
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void pioneer_ldv1000_device::device_start()
{
	// pass through to the parent
	laserdisc_device::device_start();

	// allocate timers
	m_multitimer = timer_alloc(TID_MULTIJUMP);

	m_command_strobe_cb.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void pioneer_ldv1000_device::device_reset()
{
	// pass through to the parent
	laserdisc_device::device_reset();

	// reset our state
	m_command = 0;
	m_status = 0;
	m_vsync = false;
	m_counter_start = 0;
	m_counter = 0;
	m_portc0 = 0;
	m_portb1 = 0;
	m_portc1 = 0;
	m_portselect = 0;
	m_dispindex = 0;
	m_vbiready = false;
	m_vbiindex = 0;
}


//-------------------------------------------------
//  device_timer - handle timers set by this
//  device
//-------------------------------------------------

void pioneer_ldv1000_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TID_MULTIJUMP:
		{
			// bit 5 of port B on PPI 1 selects the direction of slider movement
			int direction = (m_portb1 & 0x20) ? 1 : -1;
			advance_slider(direction);

			// update down counter and reschedule
			if (--m_counter != 0)
				timer.adjust(MULTIJUMP_TRACK_TIME);
			break;
		}

		case TID_VSYNC_OFF:
			m_vsync = false;
			break;

		case TID_VBI_DATA_FETCH:
		{
			// appears to return data in reverse order
			uint32_t lines[3];
			lines[0] = get_field_code(LASERDISC_CODE_LINE1718, false);
			lines[1] = get_field_code(LASERDISC_CODE_LINE17, false);
			lines[2] = get_field_code(LASERDISC_CODE_LINE16, false);

			// fill in the details
			memset(m_vbi, 0, sizeof(m_vbi));
			if (focus_on() && laser_on())
			{
				// loop over lines
				for (int line = 0; line < 3; line++)
				{
					uint8_t *dest = &m_vbi[line * 7];
					uint32_t data = lines[line];

					// the logic only processes leadin/leadout/frame number codes
					if (data == VBI_CODE_LEADIN || data == VBI_CODE_LEADOUT || (data & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
					{
						*dest++ = 0x09 | (((data & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE) ? 0x02 : 0x00);
						*dest++ = 0x08;
						*dest++ = (data >> 16) & 0x0f;
						*dest++ = (data >> 12) & 0x0f;
						*dest++ = (data >>  8) & 0x0f;
						*dest++ = (data >>  4) & 0x0f;
						*dest++ = (data >>  0) & 0x0f;
					}
				}
			}

			// signal that data is ready and reset the readback index
			m_vbiready = true;
			m_vbiindex = 0;
			break;
		}

		// pass everything else onto the parent
		default:
			laserdisc_device::device_timer(timer, id, param);
			break;
	}
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const tiny_rom_entry *pioneer_ldv1000_device::device_rom_region() const
{
	return ROM_NAME(ldv1000);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pioneer_ldv1000_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80_cpu, XTAL(5'000'000)/2);
	m_z80_cpu->set_daisy_config(daisy_chain);
	m_z80_cpu->set_addrmap(AS_PROGRAM, &pioneer_ldv1000_device::ldv1000_map);
	m_z80_cpu->set_addrmap(AS_IO, &pioneer_ldv1000_device::ldv1000_portmap);

	Z80CTC(config, m_z80_ctc, XTAL(5'000'000)/2);
	m_z80_ctc->intr_callback().set(FUNC(pioneer_ldv1000_device::ctc_interrupt));

	i8255_device &ldvppi0(I8255(config, "ldvppi0"));
	ldvppi0.out_pa_callback().set(FUNC(pioneer_ldv1000_device::ppi0_porta_w));
	ldvppi0.in_pb_callback().set(FUNC(pioneer_ldv1000_device::ppi0_portb_r));
	ldvppi0.in_pc_callback().set(FUNC(pioneer_ldv1000_device::ppi0_portc_r));
	ldvppi0.out_pc_callback().set(FUNC(pioneer_ldv1000_device::ppi0_portc_w));

	i8255_device &ldvppi1(I8255(config, "ldvppi1"));
	ldvppi1.in_pa_callback().set(FUNC(pioneer_ldv1000_device::ppi1_porta_r));
	ldvppi1.out_pb_callback().set(FUNC(pioneer_ldv1000_device::ppi1_portb_w));
	ldvppi1.out_pc_callback().set(FUNC(pioneer_ldv1000_device::ppi1_portc_w));
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void pioneer_ldv1000_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// generate interrupts if we hit the edges
	slider_position sliderpos = get_slider_position();
	m_z80_ctc->trg1(sliderpos == SLIDER_MINIMUM);
	m_z80_ctc->trg2(sliderpos == SLIDER_MAXIMUM);

	// signal VSYNC and set a timer to turn it off
	m_vsync = true;
	timer_set(screen().scan_period() * 4, TID_VSYNC_OFF);

	// also set a timer to fetch the VBI data when it is ready
	timer_set(screen().time_until_pos(19*2), TID_VBI_DATA_FETCH);

	// boost interleave for the first 1ms to improve communications
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_msec(1));
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

int32_t pioneer_ldv1000_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	if (LOG_FRAMES_SEEN)
	{
		int frame = frame_from_metadata(vbi);
		if (frame != FRAME_NOT_PRESENT) logerror("== %d\n", frame);
	}
	return fieldnum;
}


//-------------------------------------------------
//  ctc_interrupt - called when the CTC triggers
//  an interrupt in the daisy chain
//-------------------------------------------------

void pioneer_ldv1000_device::ctc_interrupt(int state)
{
	m_z80_cpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  z80_decoder_display_port_w - handle writes to
//  the decoder/display chips
//-------------------------------------------------

void pioneer_ldv1000_device::z80_decoder_display_port_w(offs_t offset, uint8_t data)
{
	/*
	    TX/RX = /A0 (A0=0 -> TX, A0=1 -> RX)

	    Display is 6-bit
	    Decoder is 4-bit
	*/

	// writes to offset 0 select the target for reads/writes of actual data
	if (offset == 0)
	{
		m_portselect = data;
		m_dispindex = 0;
	}

	// writes to offset 2 constitute actual writes targeted toward the display and decoder chips
	else if (offset == 2)
	{
		// selections 0 and 1 represent the two display lines; only 6 bits are transferred
		if (m_portselect < 2)
			m_display[m_portselect][m_dispindex++ % 20] = data & 0x3f;
	}
}


//-------------------------------------------------
//  z80_decoder_display_port_r - handle reads from the
//  decoder/display chips
//-------------------------------------------------

uint8_t pioneer_ldv1000_device::z80_decoder_display_port_r(offs_t offset)
{
	// reads from offset 3 constitute actual reads from the display and decoder chips
	uint8_t result = 0;
	if (offset == 3)
	{
		// selection 4 represents the VBI data reading
		if (m_portselect == 4)
		{
			m_vbiready = false;
			result = m_vbi[m_vbiindex++ % std::size(m_vbi)];
		}
	}
	return result;
}


//-------------------------------------------------
//  z80_controller_r - handle read of the data from
//  the controlling system
//-------------------------------------------------

uint8_t pioneer_ldv1000_device::z80_controller_r()
{
	// note that this is a cheesy implementation; the real thing relies on exquisite timing
	uint8_t result = m_command ^ 0xff;
	m_command = 0xff;
	return result;
}


//-------------------------------------------------
//  z80_controller_w - handle status latch writes
//-------------------------------------------------

void pioneer_ldv1000_device::z80_controller_w(uint8_t data)
{
	if (LOG_STATUS_CHANGES && data != m_status)
		logerror("%s:CONTROLLER.W=%02X\n", machine().describe_context(), data);
	m_status = data;
}


//-------------------------------------------------
//  ppi0_porta_w - handle writes to port A of
//  PPI #0
//-------------------------------------------------

void pioneer_ldv1000_device::ppi0_porta_w(uint8_t data)
{
	m_counter_start = data;
	if (LOG_PORT_IO)
		logerror("%s:PORTA.0=%02X\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  ppi0_portb_r - handle reads from port B of
//  PPI #0
//-------------------------------------------------

uint8_t pioneer_ldv1000_device::ppi0_portb_r()
{
	return m_counter;
}


//-------------------------------------------------
//  ppi0_portc_r - handle reads from port C of
//  PPI #0
//-------------------------------------------------

uint8_t pioneer_ldv1000_device::ppi0_portc_r()
{
	/*
	    $10 = /VSYNC
	    $20 = IRQ from decoder chip
	    $40 = TRKG LOOP (N24-1)
	    $80 = DUMP (N20-1) -- code reads the state and waits for it to change
	*/

	uint8_t result = 0x00;
	if (!m_vsync)
		result |= 0x10;
	if (!m_vbiready)
		result |= 0x20;
	return result;
}


//-------------------------------------------------
//  ppi0_portc_w - handle writes to port C of
//  PPI #0
//-------------------------------------------------

void pioneer_ldv1000_device::ppi0_portc_w(uint8_t data)
{
	/*
	    $01 = preload on up/down counters
	    $02 = /MULTI JUMP TRIG
	    $04 = SCAN MODE
	    $08 = n/c
	*/

	// set the new value
	uint8_t prev = m_portc0;
	m_portc0 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0x0f) != 0)
	{
		logerror("%s:PORTC.0=%02X%s%s%s\n", machine().describe_context(), data,
			(data & 0x01) ? " PRELOAD" : "",
			!(data & 0x02) ? " /MULTIJUMP" : "",
			(data & 0x04) ? " SCANMODE" : "");
	}

	// on the rising edge of bit 0, clock the down counter load
	if ((data & 0x01) && !(prev & 0x01))
		m_counter = m_counter_start;

	// on the falling edge of bit 1, start the multi-jump timer
	if (!(data & 0x02) && (prev & 0x02))
		m_multitimer->adjust(MULTIJUMP_TRACK_TIME);
}


//-------------------------------------------------
//  ppi1_porta_r - handle reads from port A of
//  PPI #1
//-------------------------------------------------

uint8_t pioneer_ldv1000_device::ppi1_porta_r()
{
	/*
	    $01 = /FOCS LOCK
	    $02 = /SPDL LOCK
	    $04 = INSIDE
	    $08 = OUTSIDE
	    $10 = MOTOR STOP
	    $20 = +5V/test point
	    $40 = /INT LOCK
	    $80 = 8 INCH CHK
	*/

	slider_position sliderpos = get_slider_position();
	uint8_t result = 0x00;

	// bit 0: /FOCUS LOCK
	if (!focus_on())
		result |= 0x01;

	// bit 1: /SPDL LOCK
	if (!spdl_on())
		result |= 0x02;

	// bit 2: INSIDE signal
	if (sliderpos == SLIDER_MINIMUM)
		result |= 0x04;

	// bit 3: OUTSIDE signal
	if (sliderpos == SLIDER_MAXIMUM)
		result |= 0x08;

	// bit 4: MOTOR STOP

	// bit 5: +5V/test point
	result |= 0x20;

	// bit 6: /INT LOCK

	// bit 7: 8 INCH CHK

	return result;
}


//-------------------------------------------------
//  ppi1_portb_w - handle writes to port B of
//  PPI #1
//-------------------------------------------------

void pioneer_ldv1000_device::ppi1_portb_w(uint8_t data)
{
	/*
	    $01 = /FOCS ON
	    $02 = /SPDL RUN
	    $04 = /JUMP TRIG
	    $08 = /SCAN A
	    $10 = SCAN B
	    $20 = SCAN C
	    $40 = /LASER ON
	    $80 = /SYNC ST0
	*/

	// set the new value
	uint8_t prev = m_portb1;
	m_portb1 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0xff) != 0)
	{
		logerror("%s:PORTB.1=%02X: %s%s%s%s%s%s\n", machine().describe_context(), data,
			!(data & 0x01) ? " FOCSON" : "",
			!(data & 0x02) ? " SPDLRUN" : "",
			!(data & 0x04) ? " JUMPTRIG" : "",
			!(data & 0x08) ? string_format(" SCANA (%c %c)", (data & 0x10) ? 'L' : 'H', (data & 0x20) ? 'F' : 'R') : "",
			(data & 0x40) ? " LASERON" : "",
			!(data & 0x80) ? " SYNCST0" : "");
	}

	// bit 5 selects the direction of slider movement for JUMP TRG and scanning
	int direction = (data & 0x20) ? 1 : -1;

	// on the falling edge of bit 2, jump one track in either direction
	if (!(data & 0x04) && (prev & 0x04))
		advance_slider(direction);

	// bit 3 low enables scanning
	if (!(data & 0x08))
	{
		// bit 4 selects the speed
		int delta = (data & 0x10) ? SCAN_SPEED : SEEK_FAST_SPEED;
		set_slider_speed(delta * direction);
	}

	// bit 3 high stops scanning
	else
		set_slider_speed(0);
}


//-------------------------------------------------
//  ppi1_portc_w - handle writes to port C of
//  PPI #1
//-------------------------------------------------

void pioneer_ldv1000_device::ppi1_portc_w(uint8_t data)
{
	/*
	    $01 = AUD 1
	    $02 = AUD 2
	    $04 = AUDIO ENABLE
	    $08 = /VIDEO SQ
	    $10 = COMMAND
	    $20 = STATUS
	    $40 = SIZE 8/12
	    $80 = /LED CAV
	*/

	// set the new value
	uint8_t prev = m_portc1;
	m_portc1 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0xcf) != 0)
	{
		logerror("%s:PORTC.1=%02X%s%s%s%s%s%s%s%s\n", machine().describe_context(), data,
			(data & 0x01) ? " AUD1" : "",
			(data & 0x02) ? " AUD2" : "",
			(data & 0x04) ? " AUDEN" : "",
			!(data & 0x08) ? " VIDEOSQ" : "",
			(data & 0x10) ? " COMMAND" : "",
			(data & 0x20) ? " STATUS" : "",
			(data & 0x40) ? " SIZE8" : "",
			!(data & 0x80) ? " CAV" : "");
	}

	// bit 4 sends a command strobe signal to Host CPU
	m_command_strobe_cb(bool(data & 0x10));

	// video squelch is controlled by bit 3
	set_video_squelch((data & 0x08) == 0);

	// audio squelch is controlled by bits 0-2
	set_audio_squelch(!(data & 0x04) || !(data & 0x01), !(data & 0x04) || !(data & 0x02));
}
