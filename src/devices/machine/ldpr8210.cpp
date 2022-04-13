// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldpr8210.c

    Pioneer PR-8210 laserdisc emulation.

**************************************************************************

    Still to do:

        * implement SLOW TRG
        * figure out Simutrek without jump hack
        * figure out serial protocol issues (current hack works nicely)
        * determine actual slow/fast speeds

*************************************************************************/


#include "emu.h"
#include "ldpr8210.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_VBLANK_VBI              0
#define LOG_SERIAL                  0
#define LOG_SIMUTREK                0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// Overlay constants, related to 720-pixel wide capture
#define OVERLAY_GROUP0_X                (82.0f / 720.0f)
#define OVERLAY_GROUP1_X                (162.0f / 720.0f)
#define OVERLAY_GROUP2_X                (322.0f / 720.0f)
#define OVERLAY_GROUP3_X                (483.0f / 720.0f)
#define OVERLAY_Y                       (104/2)
#define OVERLAY_PIXEL_WIDTH             (4.5f / 720.0f)
#define OVERLAY_PIXEL_HEIGHT            2
#define OVERLAY_X_PIXELS                5
#define OVERLAY_Y_PIXELS                7

// scanning speeds
#define SCAN_SPEED                      (2000 / 30)         // 2000 frames/second
#define SEEK_FAST_SPEED                 (4000 / 30)         // 4000 frames/second

// serial timing, mostly from the service manual, derived from the XTAL
#define SERIAL_CLOCK                    XTAL(455'000)
#define SERIAL_0_BIT_TIME               attotime::from_hz(SERIAL_CLOCK / 512)
#define SERIAL_1_BIT_TIME               attotime::from_hz(SERIAL_CLOCK / 1024)
#define SERIAL_MIDPOINT_TIME            attotime::from_hz(SERIAL_CLOCK / 600)
#define SERIAL_MAX_BIT_TIME             attotime::from_hz(SERIAL_CLOCK / 4096)
#define SERIAL_MAX_WORD_TIME            attotime::from_hz(SERIAL_CLOCK / 11520)
#define SERIAL_REJECT_DUPLICATE_TIME    attotime::from_hz(SERIAL_CLOCK / 11520 / 4)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(PIONEER_PR8210,   pioneer_pr8210_device,   "pr8210",   "Pioneer PR-8210")
DEFINE_DEVICE_TYPE(SIMUTREK_SPECIAL, simutrek_special_device, "simutrek", "Simutrek Modified PR-8210")


// bitmaps for the characters
static const uint8_t text_bitmap[0x40][7] =
{
	{ 0 },                                  // @
	{ 0x20,0x50,0x88,0x88,0xf8,0x88,0x88 }, // A
	{ 0 },                                  // B
	{ 0x70,0x88,0x80,0x80,0x80,0x88,0x70 }, // C
	{ 0 },                                  // D
	{ 0xf8,0x80,0x80,0xf0,0x80,0x80,0xf8 }, // E
	{ 0xf8,0x80,0x80,0xf0,0x80,0x80,0x80 }, // F
	{ 0 },                                  // G
	{ 0x88,0x88,0x88,0xf8,0x88,0x88,0x88 }, // H
	{ 0 },                                  // I
	{ 0 },                                  // J
	{ 0 },                                  // K
	{ 0 },                                  // L
	{ 0x88,0xd8,0xa8,0xa8,0xa8,0x88,0x88 }, // M
	{ 0 },                                  // N
	{ 0 },                                  // O
	{ 0xf0,0x88,0x88,0xf0,0x80,0x80,0x80 }, // P
	{ 0 },                                  // Q
	{ 0xf0,0x88,0x88,0xf0,0xa0,0x90,0x88 }, // R
	{ 0x70,0x88,0x80,0x70,0x08,0x88,0x70 }, // S
	{ 0 },                                  // T
	{ 0 },                                  // U
	{ 0 },                                  // V
	{ 0 },                                  // W
	{ 0 },                                  // X
	{ 0 },                                  // Y
	{ 0 },                                  // Z
	{ 0 },                                  // [
	{ 0 },                                  // <backslash>
	{ 0 },                                  // ]
	{ 0 },                                  // ^
	{ 0 },                                  // _

	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // <space>
	{ 0 },                                  // !
	{ 0 },                                  // "
	{ 0 },                                  // #
	{ 0 },                                  // $
	{ 0 },                                  // %
	{ 0 },                                  // &
	{ 0 },                                  // '
	{ 0 },                                  // (
	{ 0 },                                  // )
	{ 0 },                                  // *
	{ 0 },                                  // +
	{ 0 },                                  // ,
	{ 0 },                                  // -
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x40 }, // .
	{ 0 },                                  // /
	{ 0x70,0x88,0x88,0x88,0x88,0x88,0x70 }, // 0
	{ 0x20,0x60,0x20,0x20,0x20,0x20,0x70 }, // 1
	{ 0x70,0x88,0x08,0x70,0x80,0x80,0xf8 }, // 2
	{ 0xf8,0x08,0x10,0x30,0x08,0x88,0x70 }, // 3
	{ 0x10,0x30,0x50,0x90,0xf8,0x10,0x10 }, // 4
	{ 0xf8,0x80,0xf0,0x08,0x08,0x88,0x70 }, // 5
	{ 0x78,0x80,0x80,0xf0,0x88,0x88,0x70 }, // 6
	{ 0xf8,0x08,0x08,0x10,0x20,0x40,0x80 }, // 7
	{ 0x70,0x88,0x88,0x70,0x88,0x88,0x70 }, // 8
	{ 0x70,0x88,0x88,0x78,0x08,0x08,0xf0 }, // 9
	{ 0 },                                  // :
	{ 0 },                                  // ;
	{ 0 },                                  // <
	{ 0 },                                  // =
	{ 0 },                                  // >
	{ 0 }                                   // ?
};



//**************************************************************************
//  PR-8210 ROM AND MACHINE INTERFACES
//**************************************************************************

void pioneer_pr8210_device::pr8210_portmap(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(pioneer_pr8210_device::i8049_pia_r), FUNC(pioneer_pr8210_device::i8049_pia_w));
}

ROM_START( pr8210 )
	ROM_REGION( 0x800, "pr8210", 0 )
	ROM_LOAD( "pr-8210_mcu_ud6005a.bin", 0x000, 0x800, CRC(120fa83b) SHA1(b514326ca1f52d6d89056868f9d17eabd4e3f31d) )
ROM_END



//**************************************************************************
//  PIONEER PR-8210 IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  pioneer_pr8210_device - constructor
//-------------------------------------------------

pioneer_pr8210_device::pioneer_pr8210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pioneer_pr8210_device(mconfig, PIONEER_PR8210, tag, owner, clock)
{
}

pioneer_pr8210_device::pioneer_pr8210_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, type, tag, owner, clock),
		m_audio1(*this, "pr8210_audio1"),
		m_audio2(*this, "pr8210_audio2"),
		m_clv(*this, "pr8210_clv"),
		m_cav(*this, "pr8210_cav"),
		m_srev(*this, "pr8210_srev"),
		m_sfwd(*this, "pr8210_sfwd"),
		m_play(*this, "pr8210_play"),
		m_step(*this, "pr8210_step"),
		m_pause(*this, "pr8210_pause"),
		m_standby(*this, "pr8210_standby"),
		m_control(0),
		m_lastcommand(0),
		m_accumulator(0),
		m_lastcommandtime(attotime::zero),
		m_lastbittime(attotime::zero),
		m_firstbittime(attotime::zero),
		m_i8049_cpu(*this, "pr8210"),
		m_slowtrg(attotime::zero),
		m_vsync(false),
		m_i8049_port1(0),
		m_i8049_port2(0)
{
}


//-------------------------------------------------
//  control_w - write callback when the CONTROL
//  line is toggled
//-------------------------------------------------

void pioneer_pr8210_device::control_w(uint8_t data)
{
	// set the new value and remember the last
	uint8_t prev = m_control;
	m_control = data;

	// handle rising edge
	if (prev != ASSERT_LINE && data == ASSERT_LINE)
	{
		// get the time difference from the last assert
		// and update our internal command time
		attotime curtime = machine().time();
		attotime delta = curtime - m_lastbittime;
		m_lastbittime = curtime;

		// if we timed out since the first bit, reset the accumulator
		attotime overalldelta = curtime - m_firstbittime;
		if (overalldelta > SERIAL_MAX_WORD_TIME || delta > SERIAL_MAX_BIT_TIME)
		{
			m_firstbittime = curtime;
			m_accumulator = 0x5555;
			if (LOG_SERIAL)
				logerror("Reset accumulator\n");
		}

		// 0 bit delta is 1.05 msec, 1 bit delta is 2.11 msec
		int longpulse = (delta < SERIAL_MIDPOINT_TIME) ? 0 : 1;
		m_accumulator = (m_accumulator << 1) | longpulse;

		// log the deltas for debugging
		if (LOG_SERIAL)
		{
			int usecdiff = (int)(delta.attoseconds() / ATTOSECONDS_IN_USEC(1));
			logerror("bitdelta = %5d (%d) - accum = %04X\n", usecdiff, longpulse, m_accumulator);
		}

		// if we have a complete command, signal it
		// a complete command is 0,0,1 followed by 5 bits, followed by 0,0
		if ((m_accumulator & 0x383) == 0x80)
		{
			// data is stored to the PIA in bit-reverse order
			uint8_t newcommand = (m_accumulator >> 2) & 0x1f;
			m_pia.porta = bitswap<8>(newcommand, 0,1,2,3,4,5,6,7);

			// the MCU logic requires a 0 to execute many commands; however, nobody
			// consistently sends a 0, whereas they do tend to send duplicate commands...
			// if we assume that each duplicate causes a 0, we get the correct results
			attotime rejectuntil = m_lastcommandtime + SERIAL_REJECT_DUPLICATE_TIME;
			m_lastcommandtime = curtime;
			if (m_pia.porta == m_lastcommand && curtime < rejectuntil)
				m_pia.porta = 0x00;
			else
				m_lastcommand = m_pia.porta;

			// log the command and wait for a keypress
			if (LOG_SERIAL)
				logerror("--- Command = %02X\n", m_pia.porta >> 3);

			// reset the first bit time so that the accumulator clears on the next write
			m_firstbittime = curtime - SERIAL_MAX_WORD_TIME;
		}
	}
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void pioneer_pr8210_device::device_start()
{
	// resolve outputs
	m_audio1.resolve();
	m_audio2.resolve();
	m_clv.resolve();
	m_cav.resolve();
	m_srev.resolve();
	m_sfwd.resolve();
	m_play.resolve();
	m_step.resolve();
	m_pause.resolve();
	m_standby.resolve();

	// pass through to the parent
	laserdisc_device::device_start();
}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void pioneer_pr8210_device::device_reset()
{
	// pass through to the parent
	laserdisc_device::device_reset();

	// reset our state
	attotime curtime = machine().time();
	m_lastcommandtime = curtime;
	m_firstbittime = curtime;
	m_lastbittime = curtime;
	m_slowtrg = curtime;
}


//-------------------------------------------------
//  device_timer - handle timers set by this
//  device
//-------------------------------------------------

void pioneer_pr8210_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		// update the VBI data in the PIA as soon as it is ready;
		// this must happen early in the frame because the player
		// logic relies on fetching it here
		case TID_VBI_DATA_FETCH:

			// logging
			if (LOG_VBLANK_VBI)
			{
				uint32_t line1718 = get_field_code(LASERDISC_CODE_LINE1718, false);
				if ((line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
					logerror("%3d:VBI(%05d)\n", screen().vpos(), VBI_CAV_PICTURE(line1718));
				else
					logerror("%3d:VBI()\n", screen().vpos());
			}

			// update PIA registers based on vbi code
			m_pia.vbi1 = 0xff;
			m_pia.vbi2 = 0xff;
			if (focus_on() && laser_on())
			{
				uint32_t line16 = get_field_code(LASERDISC_CODE_LINE16, false);
				uint32_t line1718 = get_field_code(LASERDISC_CODE_LINE1718, false);
				if (line1718 == VBI_CODE_LEADIN)
					m_pia.vbi1 &= ~0x01;
				if (line1718 == VBI_CODE_LEADOUT)
					m_pia.vbi1 &= ~0x02;
				if (line16 == VBI_CODE_STOP)
					m_pia.vbi1 &= ~0x04;
				// unsure what this bit means: m_pia.vbi1 &= ~0x08;
				if ((line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
				{
					m_pia.vbi1 &= ~0x10;
					m_pia.frame[2] = 0xf0 | ((line1718 >> 16) & 0x07);
					m_pia.frame[3] = 0xf0 | ((line1718 >> 12) & 0x0f);
					m_pia.frame[4] = 0xf0 | ((line1718 >>  8) & 0x0f);
					m_pia.frame[5] = 0xf0 | ((line1718 >>  4) & 0x0f);
					m_pia.frame[6] = 0xf0 | ((line1718 >>  0) & 0x0f);
				}
				if ((line1718 & VBI_MASK_CHAPTER) == VBI_CODE_CHAPTER)
				{
					m_pia.vbi2 &= ~0x01;
					m_pia.frame[0] = 0xf0 | ((line1718 >> 16) & 0x07);
					m_pia.frame[1] = 0xf0 | ((line1718 >> 12) & 0x0f);
				}
			}
			break;

		// clear the VSYNC flag
		case TID_VSYNC_OFF:
			m_vsync = false;
			break;

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

const tiny_rom_entry *pioneer_pr8210_device::device_rom_region() const
{
	return ROM_NAME(pr8210);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pioneer_pr8210_device::device_add_mconfig(machine_config &config)
{
	I8049(config, m_i8049_cpu, XTAL(4'410'000));
	m_i8049_cpu->set_addrmap(AS_IO, &pioneer_pr8210_device::pr8210_portmap);
	m_i8049_cpu->bus_in_cb().set(FUNC(pioneer_pr8210_device::i8049_bus_r));
	m_i8049_cpu->p1_out_cb().set(FUNC(pioneer_pr8210_device::i8049_port1_w));
	m_i8049_cpu->p2_out_cb().set(FUNC(pioneer_pr8210_device::i8049_port2_w));
	m_i8049_cpu->t0_in_cb().set(FUNC(pioneer_pr8210_device::i8049_t0_r));
	m_i8049_cpu->t1_in_cb().set(FUNC(pioneer_pr8210_device::i8049_t1_r));
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void pioneer_pr8210_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// logging
	if (LOG_VBLANK_VBI)
	{
		if ((vbi.line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
			logerror("%3d:VSYNC(%d,%05d)\n", screen().vpos(), fieldnum, VBI_CAV_PICTURE(vbi.line1718));
		else
			logerror("%3d:VSYNC(%d)\n", screen().vpos(), fieldnum);
	}

	// signal VSYNC and set a timer to turn it off
	m_vsync = true;
	timer_set(screen().scan_period() * 4, TID_VSYNC_OFF);

	// also set a timer to fetch the VBI data when it is ready
	timer_set(screen().time_until_pos(19*2), TID_VBI_DATA_FETCH);
}


//-------------------------------------------------
//  player_update - update callback, called on the
//  first visible line of the frame
//-------------------------------------------------

int32_t pioneer_pr8210_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// logging
	if (LOG_VBLANK_VBI)
		logerror("%3d:Update(%d)\n", screen().vpos(), fieldnum);

	// if the spindle is on, we advance by 1 track after completing field #1
	return spdl_on() ? fieldnum : 0;
}


//-------------------------------------------------
//  player_overlay - overlay callback, called
//  during frame processing in update to overlay
//  player data
//-------------------------------------------------

void pioneer_pr8210_device::player_overlay(bitmap_yuy16 &bitmap)
{
	// custom display
	if (m_pia.display)
	{
		overlay_draw_group(bitmap, &m_pia.text[2], 5, OVERLAY_GROUP1_X);
		overlay_draw_group(bitmap, &m_pia.text[7], 5, OVERLAY_GROUP2_X);
		overlay_draw_group(bitmap, &m_pia.text[12], 5, OVERLAY_GROUP3_X);
	}

	// chapter/frame display
	else
	{
		// frame display
		if (m_pia.latchdisplay & 2)
			overlay_draw_group(bitmap, &m_pia.text[2], 5, OVERLAY_GROUP1_X);

		// chapter overlay
		if (m_pia.latchdisplay & 1)
			overlay_draw_group(bitmap, &m_pia.text[0], 2, OVERLAY_GROUP0_X);
	}
	m_pia.latchdisplay = 0;
}


//-------------------------------------------------
//  i8049_pia_r - handle reads from the mystery
//  Pioneer PIA
//-------------------------------------------------

uint8_t pioneer_pr8210_device::i8049_pia_r(offs_t offset)
{
	uint8_t result = 0xff;
	switch (offset)
	{
		// (20-26) 7 characters for the chapter/frame
		case 0x20:  case 0x21:
		case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
			result = m_pia.frame[offset - 0x20];
			break;

		// (1D-1F,27) invalid read but normal
		case 0x1d:  case 0x1e:  case 0x1f:
		case 0x27:
			break;

		// (A0) port A value (from serial decoder)
		case 0xa0:
			result = m_pia.porta;
			break;

		// (C0) VBI decoding state 1
		case 0xc0:
			if (LOG_VBLANK_VBI)
				logerror("%3d:PIA(C0)\n", screen().vpos());
			result = m_pia.vbi1;
			break;

		// (E0) VBI decoding state 2
		case 0xe0:
			if (LOG_VBLANK_VBI)
				logerror("%3d:PIA(E0)\n", screen().vpos());
			result = m_pia.vbi2;
			break;

		default:
			logerror("%s Unknown PR-8210 PIA read from offset %02X\n", machine().describe_context(), offset);
			break;
	}
	return result;
}


//-------------------------------------------------
//  i8049_pia_w - handle writes to the mystery
//  Pioneer PIA
//-------------------------------------------------

void pioneer_pr8210_device::i8049_pia_w(offs_t offset, uint8_t data)
{
	uint8_t value;
	switch (offset)
	{
		// (20-30) 17 characters for the display
		case 0x20:  case 0x21:
		case 0x22:  case 0x23:  case 0x24:  case 0x25:  case 0x26:
		case 0x27:  case 0x28:  case 0x29:  case 0x2a:  case 0x2b:
		case 0x2c:  case 0x2d:  case 0x2e:  case 0x2f:  case 0x30:
			m_pia.text[offset - 0x20] = data;
			break;

		// (40) control lines
		case 0x40:

			// toggle bit 0 to latch chapter number into display area
			if (!(data & 0x01) && (m_pia.control & 0x01))
			{
				memcpy(&m_pia.text[0], &m_pia.frame[0], 2);
				m_pia.latchdisplay |= 1;
			}

			// toggle bit 1 to latch frame number into display area
			if (!(data & 0x02) && (m_pia.control & 0x02))
			{
				memcpy(&m_pia.text[2], &m_pia.frame[2], 5);
				m_pia.latchdisplay |= 2;
			}
			m_pia.control = data;
			break;

		// (60) port B value (LEDs)
		case 0x60:

			// these 4 are direct-connect
			m_audio1 = BIT(data, 0);
			m_audio2 = BIT(data, 1);
			m_clv = BIT(data, 2);
			m_cav = BIT(data, 3);

			// remaining 3 bits select one of 5 LEDs via a mux
			value = ((data & 0x40) >> 6) | ((data & 0x20) >> 4) | ((data & 0x10) >> 2);
			m_srev = (value == 0);
			m_sfwd = (value == 1);
			m_play = (value == 2);
			m_step = (value == 3);
			m_pause = (value == 4);

			m_pia.portb = data;
			update_audio_squelch();
			break;

		// (80) display enable
		case 0x80:
			m_pia.display = data & 0x01;
			break;

		// no other writes known
		default:
			logerror("%s Unknown PR-8210 PIA write to offset %02X = %02X\n", machine().describe_context(), offset, data);
			break;
	}
}


//-------------------------------------------------
//  i8049_bus_r - handle reads from the 8049 BUS
//  input, which is enabled via the PIA above
//-------------------------------------------------

uint8_t pioneer_pr8210_device::i8049_bus_r()
{
	/*
	   $80 = n/c
	   $40 = (in) slider pot interrupt source (slider position limit detector, inside and outside)
	   $20 = n/c
	   $10 = (in) /FOCUS LOCK
	   $08 = (in) /SPDL LOCK
	   $04 = (in) SIZE 8/12
	   $02 = (in) FG via op-amp (spindle motor stop detector)
	   $01 = (in) SLOW TIMER OUT
	*/

	uint8_t result = 0x00;

	// bus bit 6: slider position limit detector, inside and outside
	slider_position sliderpos = get_slider_position();
	if (sliderpos != SLIDER_MINIMUM && sliderpos != SLIDER_MAXIMUM)
		result |= 0x40;

	// bus bit 4: /FOCUS LOCK
	if (!focus_on())
		result |= 0x10;

	// bus bit 3: /SPDL LOCK
	if (!spdl_on())
		result |= 0x08;

	// bus bit 1: spindle motor stop detector
	if (!spdl_on())
		result |= 0x02;

	// bus bit 0: SLOW TIMER OUT

	// loop at beginning waits for $40=0, $02=1
	return result;
}


//-------------------------------------------------
//  i8049_port1_w - handle writes to the 8049
//  port #1
//-------------------------------------------------

void pioneer_pr8210_device::i8049_port1_w(uint8_t data)
{
	/*
	   $80 = (out) SCAN C (F/R)
	   $40 = (out) AUDIO SQ
	   $20 = (out) VIDEO SQ
	   $10 = (out) /SPDL ON
	   $08 = (out) /FOCUS ON
	   $04 = (out) SCAN B (L/H)
	   $02 = (out) SCAN A (/SCAN)
	   $01 = (out) JUMP TRG (jump back trigger, clock on high->low)
	*/

	// set the new value
	uint8_t prev = m_i8049_port1;
	m_i8049_port1 = data;

	// bit 7 selects the direction of slider movement for JUMP TRG and scanning
	int direction = (data & 0x80) ? 1 : -1;

	// on the falling edge of bit 0, jump one track in either direction
	if (!(data & 0x01) && (prev & 0x01))
	{
		// special override for the Simutrek, which takes over control of this is some situations
		if (!override_control())
		{
			if (LOG_SIMUTREK)
				logerror("%3d:JUMP TRG\n", screen().vpos());
			advance_slider(direction);
		}
		else if (LOG_SIMUTREK)
			logerror("%3d:Skipped JUMP TRG\n", screen().vpos());
	}

	// bit 1 low enables scanning
	if (!(data & 0x02))
	{
		// bit 2 selects the speed
		int delta = (data & 0x04) ? SCAN_SPEED : SEEK_FAST_SPEED;
		set_slider_speed(delta * direction);
	}

	// bit 1 high stops scanning
	else
		set_slider_speed(0);

	// video squelch is controlled by bit 5; audio squelch is controlled by bit 6
	update_video_squelch();
	update_audio_squelch();
}


//-------------------------------------------------
//  i8049_port2_w - handle writes to the 8049
//  port #2
//-------------------------------------------------

void pioneer_pr8210_device::i8049_port2_w(uint8_t data)
{
	/*
	   $80 = (out) /CS on PIA
	   $40 = (out) 0 to self-generate IRQ
	   $20 = (out) SLOW TRG
	   $10 = (out) STANDBY LED
	   $08 = (out) TP2
	   $04 = (out) TP1
	   $02 = (out) ???
	   $01 = (out) LASER ON
	*/

	// set the new value
	uint8_t prev = m_i8049_port2;
	m_i8049_port2 = data;

	// on the falling edge of bit 5, start the slow timer
	if (!BIT(data, 5) && BIT(prev, 5))
		m_slowtrg = machine().time();

	// bit 6 when low triggers an IRQ on the MCU
	m_i8049_cpu->set_input_line(MCS48_INPUT_IRQ, BIT(data, 6) ? CLEAR_LINE : ASSERT_LINE);

	// standby LED is set accordingly to bit 4
	m_standby = BIT(data, 4);
}


//-------------------------------------------------
//  i8049_t0_r - return the state of the 8049
//  T0 input (connected to VSYNC)
//-------------------------------------------------

int pioneer_pr8210_device::i8049_t0_r()
{
	// returns VSYNC state
	return !m_vsync;
}


//-------------------------------------------------
//  i8049_t1_r - return the state of the 8049
//  T1 input (pulled high)
//-------------------------------------------------

int pioneer_pr8210_device::i8049_t1_r()
{
	return 1;
}


//-------------------------------------------------
//  overlay_draw_group - draw a single group of
//  characters
//-------------------------------------------------

void pioneer_pr8210_device::overlay_draw_group(bitmap_yuy16 &bitmap, const uint8_t *text, int count, float xstart)
{
	// rease the background
	overlay_erase(bitmap, xstart, xstart + ((OVERLAY_X_PIXELS + 1) * count + 1) * OVERLAY_PIXEL_WIDTH);

	// draw each character, suppressing leading 0's
	bool skip = true;
	for (int x = 0; x < count; x++)
		if (!skip || x == count - 1 || (text[x] & 0x3f) != 0x30)
		{
			skip = false;
			overlay_draw_char(bitmap, text[x], xstart + ((OVERLAY_X_PIXELS + 1) * x + 1) * OVERLAY_PIXEL_WIDTH);
		}
}


//-------------------------------------------------
//  overlay_erase - erase the background area
//  where the text overlay will be displayed
//-------------------------------------------------

void pioneer_pr8210_device::overlay_erase(bitmap_yuy16 &bitmap, float xstart, float xend)
{
	uint32_t xmin = uint32_t(xstart * 256.0f * float(bitmap.width()));
	uint32_t xmax = uint32_t(xend * 256.0f * float(bitmap.width()));

	for (uint32_t y = OVERLAY_Y; y < (OVERLAY_Y + (OVERLAY_Y_PIXELS + 2) * OVERLAY_PIXEL_HEIGHT); y++)
	{
		uint16_t *dest = &bitmap.pix(y, xmin >> 8);
		uint16_t ymax = *dest >> 8;
		uint16_t ymin = ymax * 3 / 8;
		uint16_t yres = ymin + ((ymax - ymin) * (xmin & 0xff)) / 256;
		*dest = (yres << 8) | (*dest & 0xff);
		dest++;

		for (uint32_t x = (xmin | 0xff) + 1; x < xmax; x += 0x100)
		{
			yres = (*dest >> 8) * 3 / 8;
			*dest = (yres << 8) | (*dest & 0xff);
			dest++;
		}

		ymax = *dest >> 8;
		ymin = ymax * 3 / 8;
		yres = ymin + ((ymax - ymin) * (~xmax & 0xff)) / 256;
		*dest = (yres << 8) | (*dest & 0xff);
		dest++;
	}
}


//-------------------------------------------------
//  overlay_draw_char - draw a single character
//  of the text overlay
//-------------------------------------------------

void pioneer_pr8210_device::overlay_draw_char(bitmap_yuy16 &bitmap, uint8_t ch, float xstart)
{
	uint32_t xminbase = uint32_t(xstart * 256.0f * float(bitmap.width()));
	uint32_t xsize = uint32_t(OVERLAY_PIXEL_WIDTH * 256.0f * float(bitmap.width()));

	// iterate over pixels
	const uint8_t *chdataptr = &text_bitmap[ch & 0x3f][0];
	for (uint32_t y = 0; y < OVERLAY_Y_PIXELS; y++)
	{
		uint8_t chdata = *chdataptr++;

		for (uint32_t x = 0; x < OVERLAY_X_PIXELS; x++, chdata <<= 1)
			if (chdata & 0x80)
			{
				uint32_t xmin = xminbase + x * xsize;
				uint32_t xmax = xmin + xsize;
				for (uint32_t yy = 0; yy < OVERLAY_PIXEL_HEIGHT; yy++)
				{
					uint16_t *dest = &bitmap.pix(OVERLAY_Y + (y + 1) * OVERLAY_PIXEL_HEIGHT + yy, xmin >> 8);
					uint16_t ymax = 0xff;
					uint16_t ymin = *dest >> 8;
					uint16_t yres = ymin + ((ymax - ymin) * (~xmin & 0xff)) / 256;
					*dest = (yres << 8) | (*dest & 0xff);
					dest++;

					for (uint32_t xx = (xmin | 0xff) + 1; xx < xmax; xx += 0x100)
						*dest++ = 0xf080;

					ymax = 0xff;
					ymin = *dest >> 8;
					yres = ymin + ((ymax - ymin) * (xmax & 0xff)) / 256;
					*dest = (yres << 8) | (*dest & 0xff);
					dest++;
				}
			}
	}
}



//**************************************************************************
//  SIMUTREK ROM AND MACHINE INTERFACES
//**************************************************************************

void simutrek_special_device::simutrek_portmap(address_map &map)
{
	map(0x00, 0xff).r(FUNC(simutrek_special_device::i8748_data_r));
}


ROM_START( simutrek )
	ROM_REGION( 0x800, "pr8210", 0 )
	ROM_LOAD( "pr-8210_mcu_ud6005a.bin", 0x000, 0x800, CRC(120fa83b) SHA1(b514326ca1f52d6d89056868f9d17eabd4e3f31d) )

	ROM_REGION( 0x400, "simutrek", 0)
	ROM_LOAD( "laser_player_interface_d8748_a308.bin", 0x0000, 0x0400, CRC(eed3e728) SHA1(1eb3467f1c41553375b2c21952cd593b167f5416) )
ROM_END



//**************************************************************************
//  SIMUTREK IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
// simutrek_special_device - constructor
//-------------------------------------------------

simutrek_special_device::simutrek_special_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pioneer_pr8210_device(mconfig, SIMUTREK_SPECIAL, tag, owner, clock),
		m_i8748_cpu(*this, "simutrek"),
		m_audio_squelch(0),
		m_data(0),
		m_data_ready(false),
		m_i8748_port2(0),
		m_controlnext(0),
		m_controlthis(0)
{
}


//-------------------------------------------------
//  data_w - write callback when the parallel data
//  port is written to
//-------------------------------------------------

void simutrek_special_device::data_w(uint8_t data)
{
	synchronize(TID_LATCH_DATA, data);
	if (LOG_SIMUTREK)
		logerror("%03d:**** Simutrek Command = %02X\n", screen().vpos(), data);
}


//-------------------------------------------------
//  set_external_audio_squelch - Simutrek-specific
//  command to enable/disable audio squelch
//-------------------------------------------------

void simutrek_special_device::set_external_audio_squelch(int state)
{
	if (LOG_SIMUTREK && m_audio_squelch != (state == 0))
		logerror("--> audio squelch = %d\n", state == 0);
	m_audio_squelch = (state == 0);
	update_audio_squelch();
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void simutrek_special_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// latch the control state after the second field
	if (fieldnum == 1)
	{
		m_controlthis = m_controlnext;
		m_controlnext = 0;
	}

	// call the parent
	if (LOG_SIMUTREK)
		logerror("%3d:VSYNC(%d)\n", screen().vpos(), fieldnum);
	pioneer_pr8210_device::player_vsync(vbi, fieldnum, curtime);

	// process data
	if (m_data_ready)
	{
		if (LOG_SIMUTREK)
			logerror("%3d:VSYNC IRQ\n", screen().vpos());
		m_i8748_cpu->set_input_line(MCS48_INPUT_IRQ, ASSERT_LINE);
		timer_set(screen().scan_period(), TID_IRQ_OFF);
	}
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void simutrek_special_device::device_start()
{
	// pass through to the parent
	pioneer_pr8210_device::device_start();
}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void simutrek_special_device::device_reset()
{
	// standard PR-8210 initialization
	pioneer_pr8210_device::device_reset();

	// initialize the Simutrek state
	// for proper synchronization of initial attract mode, this needs to be set
	m_data_ready = true;
}


//-------------------------------------------------
//  device_timer - handle timers set by this
//  device
//-------------------------------------------------

void simutrek_special_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		// clear the 8748 IRQ
		case TID_IRQ_OFF:
			m_i8748_cpu->set_input_line(MCS48_INPUT_IRQ, CLEAR_LINE);
			break;

		// latch data
		case TID_LATCH_DATA:
			m_data = param;
			m_data_ready = true;
			break;

		// pass everything else onto the parent
		default:
			pioneer_pr8210_device::device_timer(timer, id, param);
			break;
	}
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const tiny_rom_entry *simutrek_special_device::device_rom_region() const
{
	return ROM_NAME(simutrek);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void simutrek_special_device::device_add_mconfig(machine_config &config)
{
	i8748_device &special(I8748(config, "simutrek", XTAL(6'000'000)));
	special.set_addrmap(AS_IO, &simutrek_special_device::simutrek_portmap);
	special.p2_in_cb().set(FUNC(simutrek_special_device::i8748_port2_r));
	special.p2_out_cb().set(FUNC(simutrek_special_device::i8748_port2_w));
	special.t0_in_cb().set(FUNC(simutrek_special_device::i8748_t0_r));

	pioneer_pr8210_device::device_add_mconfig(config);
}


//-------------------------------------------------
//  i8748_port2_r - handle reads from the 8748
//  port #2
//-------------------------------------------------

uint8_t simutrek_special_device::i8748_port2_r()
{
	// bit $80 is the pr8210 video squelch
	return (m_i8049_port1 & 0x20) ? 0x00 : 0x80;
}


//-------------------------------------------------
//  i8748_port2_w - handle writes to the 8748
//  port #2
//-------------------------------------------------

void simutrek_special_device::i8748_port2_w(uint8_t data)
{
	// update stat
	uint8_t prev = m_i8748_port2;
	m_i8748_port2 = data;

	// bit $20 goes to the serial line
	if ((data ^ prev) & 0x20)
		pioneer_pr8210_device::control_w((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	// bit $10 goes to JUMP TRG
	// bit $08 controls direction
	if (!(data & 0x10) && (prev & 0x10))
	{
		int direction = (data & 0x08) ? 1 : -1;
		if (LOG_SIMUTREK)
			logerror("%3d:JUMP TRG %s\n", screen().vpos(), machine().describe_context());
		advance_slider(direction);
	}

	// bit $04 controls who owns the JUMP TRG command
	if (LOG_SIMUTREK && ((data ^ prev) & 0x04))
		logerror("%3d:Simutrek ownership line = %d %s\n", screen().vpos(), (data >> 2) & 1, machine().describe_context());
	m_controlnext = (~data >> 2) & 1;

	// bits $03 control something (status?)
	if (LOG_SIMUTREK && ((data ^ prev) & 0x03))
		logerror("Simutrek Status = %d\n", data & 0x03);
}


//-------------------------------------------------
//  i8748_data_r - handle external 8748 data reads
//-------------------------------------------------

uint8_t simutrek_special_device::i8748_data_r()
{
	// acknowledge the read and clear the data ready flag
	m_data_ready = false;
	return m_data;
}


//-------------------------------------------------
//  i8748_t0_r - return the status of the 8748
//  T0 input
//-------------------------------------------------

int simutrek_special_device::i8748_t0_r()
{
	// return 1 if data is waiting from main CPU
	return m_data_ready;
}
