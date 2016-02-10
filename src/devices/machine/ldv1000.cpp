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
#include "cpu/z80/z80daisy.h"



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
const device_type PIONEER_LDV1000 = &device_creator<pioneer_ldv1000_device>;



//**************************************************************************
//  LD-V1000 ROM AND MACHINE INTERFACES
//**************************************************************************

static ADDRESS_MAP_START( ldv1000_map, AS_PROGRAM, 8, pioneer_ldv1000_device )
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x6000) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x3800) AM_RAM
	AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x9ff0) AM_DEVREADWRITE("ldvppi0", i8255_device, read, write)
	AM_RANGE(0xc004, 0xc007) AM_MIRROR(0x9ff0) AM_DEVREADWRITE("ldvppi1", i8255_device, read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ldv1000_portmap, AS_IO, 8, pioneer_ldv1000_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x07) AM_MIRROR(0x38) AM_READWRITE(z80_decoder_display_port_r, z80_decoder_display_port_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(z80_controller_r)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3f) AM_WRITE(z80_controller_w)
	AM_RANGE(0xc0, 0xc3) AM_MIRROR(0x3c) AM_DEVREADWRITE("ldvctc", z80ctc_device, read, write)
ADDRESS_MAP_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ldvctc" },
	{ nullptr }
};


static MACHINE_CONFIG_FRAGMENT( ldv1000 )
	MCFG_CPU_ADD("ldv1000", Z80, XTAL_5MHz/2)
	MCFG_CPU_CONFIG(daisy_chain)
	MCFG_CPU_PROGRAM_MAP(ldv1000_map)
	MCFG_CPU_IO_MAP(ldv1000_portmap)

	MCFG_DEVICE_ADD("ldvctc", Z80CTC, XTAL_5MHz/2)
	MCFG_Z80CTC_INTR_CB(WRITELINE(pioneer_ldv1000_device, ctc_interrupt))

	MCFG_DEVICE_ADD("ldvppi0", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pioneer_ldv1000_device, ppi0_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(pioneer_ldv1000_device, ppi0_portb_r))
	MCFG_I8255_IN_PORTC_CB(READ8(pioneer_ldv1000_device, ppi0_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pioneer_ldv1000_device, ppi0_portc_w))

	MCFG_DEVICE_ADD("ldvppi1", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(pioneer_ldv1000_device, ppi1_porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(pioneer_ldv1000_device, ppi1_portb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pioneer_ldv1000_device, ppi1_portc_w))
MACHINE_CONFIG_END


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

pioneer_ldv1000_device::pioneer_ldv1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: laserdisc_device(mconfig, PIONEER_LDV1000, "Pioneer LD-V1000", tag, owner, clock, "ldv1000", __FILE__),
		m_z80_cpu(*this, "ldv1000"),
		m_z80_ctc(*this, "ldvctc"),
		m_multitimer(nullptr),
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

void pioneer_ldv1000_device::data_w(UINT8 data)
{
	m_command = data;
	if (LOG_COMMANDS)
		printf("-> COMMAND = %02X (%s)\n", data, (m_portc1 & 0x10) ? "valid" : "invalid");
}


//-------------------------------------------------
//  enter_w - set the state of the ENTER strobe
//-------------------------------------------------

void pioneer_ldv1000_device::enter_w(UINT8 data)
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

void pioneer_ldv1000_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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
			UINT32 lines[3];
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
					UINT8 *dest = &m_vbi[line * 7];
					UINT32 data = lines[line];

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
			laserdisc_device::device_timer(timer, id, param, ptr);
			break;
	}
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const rom_entry *pioneer_ldv1000_device::device_rom_region() const
{
	return ROM_NAME(ldv1000);
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  our machine config fragment
//-------------------------------------------------

machine_config_constructor pioneer_ldv1000_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(ldv1000);
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

INT32 pioneer_ldv1000_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	if (LOG_FRAMES_SEEN)
	{
		int frame = frame_from_metadata(vbi);
		if (frame != FRAME_NOT_PRESENT) printf("== %d\n", frame);
	}
	return fieldnum;
}


//-------------------------------------------------
//  ctc_interrupt - called when the CTC triggers
//  an interrupt in the daisy chain
//-------------------------------------------------

WRITE_LINE_MEMBER( pioneer_ldv1000_device::ctc_interrupt )
{
	m_z80_cpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  z80_decoder_display_port_w - handle writes to
//  the decoder/display chips
//-------------------------------------------------

WRITE8_MEMBER( pioneer_ldv1000_device::z80_decoder_display_port_w )
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

READ8_MEMBER( pioneer_ldv1000_device::z80_decoder_display_port_r )
{
	// reads from offset 3 constitute actual reads from the display and decoder chips
	UINT8 result = 0;
	if (offset == 3)
	{
		// selection 4 represents the VBI data reading
		if (m_portselect == 4)
		{
			m_vbiready = false;
			result = m_vbi[m_vbiindex++ % ARRAY_LENGTH(m_vbi)];
		}
	}
	return result;
}


//-------------------------------------------------
//  z80_controller_r - handle read of the data from
//  the controlling system
//-------------------------------------------------

READ8_MEMBER( pioneer_ldv1000_device::z80_controller_r )
{
	// note that this is a cheesy implementation; the real thing relies on exquisite timing
	UINT8 result = m_command ^ 0xff;
	m_command = 0xff;
	return result;
}


//-------------------------------------------------
//  z80_controller_w - handle status latch writes
//-------------------------------------------------

WRITE8_MEMBER( pioneer_ldv1000_device::z80_controller_w )
{
	if (LOG_STATUS_CHANGES && data != m_status)
		printf("%04X:CONTROLLER.W=%02X\n", space.device().safe_pc(), data);
	m_status = data;
}


//-------------------------------------------------
//  ppi0_porta_w - handle writes to port A of
//  PPI #0
//-------------------------------------------------

WRITE8_MEMBER( pioneer_ldv1000_device::ppi0_porta_w )
{
	m_counter_start = data;
	if (LOG_PORT_IO)
		printf("%s:PORTA.0=%02X\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  ppi0_portb_r - handle reads from port B of
//  PPI #0
//-------------------------------------------------

READ8_MEMBER( pioneer_ldv1000_device::ppi0_portb_r )
{
	return m_counter;
}


//-------------------------------------------------
//  ppi0_portc_r - handle reads from port C of
//  PPI #0
//-------------------------------------------------

READ8_MEMBER( pioneer_ldv1000_device::ppi0_portc_r )
{
	/*
	    $10 = /VSYNC
	    $20 = IRQ from decoder chip
	    $40 = TRKG LOOP (N24-1)
	    $80 = DUMP (N20-1) -- code reads the state and waits for it to change
	*/

	UINT8 result = 0x00;
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

WRITE8_MEMBER( pioneer_ldv1000_device::ppi0_portc_w )
{
	/*
	    $01 = preload on up/down counters
	    $02 = /MULTI JUMP TRIG
	    $04 = SCAN MODE
	    $08 = n/c
	*/

	// set the new value
	UINT8 prev = m_portc0;
	m_portc0 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0x0f) != 0)
	{
		printf("%s:PORTC.0=%02X", machine().describe_context(), data);
		if (data & 0x01) printf(" PRELOAD");
		if (!(data & 0x02)) printf(" /MULTIJUMP");
		if (data & 0x04) printf(" SCANMODE");
		printf("\n");
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

READ8_MEMBER( pioneer_ldv1000_device::ppi1_porta_r )
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
	UINT8 result = 0x00;

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

WRITE8_MEMBER( pioneer_ldv1000_device::ppi1_portb_w )
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
	UINT8 prev = m_portb1;
	m_portb1 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0xff) != 0)
	{
		printf("%s:PORTB.1=%02X:", machine().describe_context(), data);
		if (!(data & 0x01)) printf(" FOCSON");
		if (!(data & 0x02)) printf(" SPDLRUN");
		if (!(data & 0x04)) printf(" JUMPTRIG");
		if (!(data & 0x08)) printf(" SCANA (%c %c)", (data & 0x10) ? 'L' : 'H', (data & 0x20) ? 'F' : 'R');
		if ( (data & 0x40)) printf(" LASERON");
		if (!(data & 0x80)) printf(" SYNCST0");
		printf("\n");
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

WRITE8_MEMBER( pioneer_ldv1000_device::ppi1_portc_w )
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
	UINT8 prev = m_portc1;
	m_portc1 = data;
	if (LOG_PORT_IO && ((data ^ prev) & 0xcf) != 0)
	{
		printf("%s:PORTC.1=%02X", machine().describe_context(), data);
		if (data & 0x01) printf(" AUD1");
		if (data & 0x02) printf(" AUD2");
		if (data & 0x04) printf(" AUDEN");
		if (!(data & 0x08)) printf(" VIDEOSQ");
		if (data & 0x10) printf(" COMMAND");
		if (data & 0x20) printf(" STATUS");
		if (data & 0x40) printf(" SIZE8");
		if (!(data & 0x80)) printf(" CAV");
		printf("\n");
	}

	// video squelch is controlled by bit 3
	set_video_squelch((data & 0x08) == 0);

	// audio squelch is controlled by bits 0-2
	set_audio_squelch(!(data & 0x04) || !(data & 0x01), !(data & 0x04) || !(data & 0x02));
}
