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
#define SERIAL_CLOCK                    XTAL_455kHz
#define SERIAL_0_BIT_TIME               attotime::from_hz((double)SERIAL_CLOCK / 512)
#define SERIAL_1_BIT_TIME               attotime::from_hz((double)SERIAL_CLOCK / 1024)
#define SERIAL_MIDPOINT_TIME            attotime::from_hz((double)SERIAL_CLOCK / 600)
#define SERIAL_MAX_BIT_TIME             attotime::from_hz((double)SERIAL_CLOCK / 4096)
#define SERIAL_MAX_WORD_TIME            attotime::from_hz((double)SERIAL_CLOCK / 11520)
#define SERIAL_REJECT_DUPLICATE_TIME    attotime::from_hz((double)SERIAL_CLOCK / 11520 / 4)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type PIONEER_PR8210 = &device_creator<pioneer_pr8210_device>;
const device_type SIMUTREK_SPECIAL = &device_creator<simutrek_special_device>;


// bitmaps for the characters
static const UINT8 text_bitmap[0x40][7] =
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

static ADDRESS_MAP_START( pr8210_portmap, AS_IO, 8, pioneer_pr8210_device )
	AM_RANGE(0x00, 0xff) AM_READWRITE(i8049_pia_r, i8049_pia_w)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(i8049_bus_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(i8049_port1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(i8049_port2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(i8049_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(i8049_t1_r)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( pr8210 )
	MCFG_CPU_ADD("pr8210", I8049, XTAL_4_41MHz)
	MCFG_CPU_IO_MAP(pr8210_portmap)
MACHINE_CONFIG_END


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

pioneer_pr8210_device::pioneer_pr8210_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: laserdisc_device(mconfig, PIONEER_PR8210, "Pioneer PR-8210", tag, owner, clock, "pr8210", __FILE__),
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

pioneer_pr8210_device::pioneer_pr8210_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: laserdisc_device(mconfig, type, name, tag, owner, clock, shortname, source),
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

void pioneer_pr8210_device::control_w(UINT8 data)
{
	// set the new value and remember the last
	UINT8 prev = m_control;
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
				printf("Reset accumulator\n");
		}

		// 0 bit delta is 1.05 msec, 1 bit delta is 2.11 msec
		int longpulse = (delta < SERIAL_MIDPOINT_TIME) ? 0 : 1;
		m_accumulator = (m_accumulator << 1) | longpulse;

		// log the deltas for debugging
		if (LOG_SERIAL)
		{
			int usecdiff = (int)(delta.attoseconds() / ATTOSECONDS_IN_USEC(1));
			printf("bitdelta = %5d (%d) - accum = %04X\n", usecdiff, longpulse, m_accumulator);
		}

		// if we have a complete command, signal it
		// a complete command is 0,0,1 followed by 5 bits, followed by 0,0
		if ((m_accumulator & 0x383) == 0x80)
		{
			// data is stored to the PIA in bit-reverse order
			UINT8 newcommand = (m_accumulator >> 2) & 0x1f;
			m_pia.porta = BITSWAP8(newcommand, 0,1,2,3,4,5,6,7);

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
				printf("--- Command = %02X\n", m_pia.porta >> 3);

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

void pioneer_pr8210_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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
				UINT32 line1718 = get_field_code(LASERDISC_CODE_LINE1718, FALSE);
				if ((line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
					printf("%3d:VBI(%05d)\n", screen().vpos(), VBI_CAV_PICTURE(line1718));
				else
					printf("%3d:VBI()\n", screen().vpos());
			}

			// update PIA registers based on vbi code
			m_pia.vbi1 = 0xff;
			m_pia.vbi2 = 0xff;
			if (focus_on() && laser_on())
			{
				UINT32 line16 = get_field_code(LASERDISC_CODE_LINE16, FALSE);
				UINT32 line1718 = get_field_code(LASERDISC_CODE_LINE1718, FALSE);
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
			laserdisc_device::device_timer(timer, id, param, ptr);
			break;
	}
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const rom_entry *pioneer_pr8210_device::device_rom_region() const
{
	return ROM_NAME(pr8210);
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  our machine config fragment
//-------------------------------------------------

machine_config_constructor pioneer_pr8210_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(pr8210);
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
			printf("%3d:VSYNC(%d,%05d)\n", screen().vpos(), fieldnum, VBI_CAV_PICTURE(vbi.line1718));
		else
			printf("%3d:VSYNC(%d)\n", screen().vpos(), fieldnum);
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

INT32 pioneer_pr8210_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// logging
	if (LOG_VBLANK_VBI)
		printf("%3d:Update(%d)\n", screen().vpos(), fieldnum);

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

READ8_MEMBER( pioneer_pr8210_device::i8049_pia_r )
{
	UINT8 result = 0xff;
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
				printf("%3d:PIA(C0)\n", screen().vpos());
			result = m_pia.vbi1;
			break;

		// (E0) VBI decoding state 2
		case 0xe0:
			if (LOG_VBLANK_VBI)
				printf("%3d:PIA(E0)\n", screen().vpos());
			result = m_pia.vbi2;
			break;

		default:
			osd_printf_debug("%03X:Unknown PR-8210 PIA read from offset %02X\n", space.device().safe_pc(), offset);
			break;
	}
	return result;
}


//-------------------------------------------------
//  i8049_pia_w - handle writes to the mystery
//  Pioneer PIA
//-------------------------------------------------

WRITE8_MEMBER( pioneer_pr8210_device::i8049_pia_w )
{
	UINT8 value;
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
			machine().output().set_value("pr8210_audio1", (data & 0x01) != 0);
			machine().output().set_value("pr8210_audio2", (data & 0x02) != 0);
			machine().output().set_value("pr8210_clv", (data & 0x04) != 0);
			machine().output().set_value("pr8210_cav", (data & 0x08) != 0);

			// remaining 3 bits select one of 5 LEDs via a mux
			value = ((data & 0x40) >> 6) | ((data & 0x20) >> 4) | ((data & 0x10) >> 2);
			machine().output().set_value("pr8210_srev", (value == 0));
			machine().output().set_value("pr8210_sfwd", (value == 1));
			machine().output().set_value("pr8210_play", (value == 2));
			machine().output().set_value("pr8210_step", (value == 3));
			machine().output().set_value("pr8210_pause", (value == 4));

			m_pia.portb = data;
			update_audio_squelch();
			break;

		// (80) display enable
		case 0x80:
			m_pia.display = data & 0x01;
			break;

		// no other writes known
		default:
			osd_printf_debug("%03X:Unknown PR-8210 PIA write to offset %02X = %02X\n", space.device().safe_pc(), offset, data);
			break;
	}
}


//-------------------------------------------------
//  i8049_bus_r - handle reads from the 8049 BUS
//  input, which is enabled via the PIA above
//-------------------------------------------------

READ8_MEMBER( pioneer_pr8210_device::i8049_bus_r )
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

	UINT8 result = 0x00;

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

WRITE8_MEMBER( pioneer_pr8210_device::i8049_port1_w )
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
	UINT8 prev = m_i8049_port1;
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
				printf("%3d:JUMP TRG\n", screen().vpos());
			advance_slider(direction);
		}
		else if (LOG_SIMUTREK)
			printf("%3d:Skipped JUMP TRG\n", screen().vpos());
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

WRITE8_MEMBER( pioneer_pr8210_device::i8049_port2_w )
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
	UINT8 prev = m_i8049_port2;
	m_i8049_port2 = data;

	// on the falling edge of bit 5, start the slow timer
	if (!(data & 0x20) && (prev & 0x20))
		m_slowtrg = machine().time();

	// bit 6 when low triggers an IRQ on the MCU
	m_i8049_cpu->set_input_line(MCS48_INPUT_IRQ, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

	// standby LED is set accordingl to bit 4
	machine().output().set_value("pr8210_standby", (data & 0x10) != 0);
}


//-------------------------------------------------
//  i8049_t0_r - return the state of the 8049
//  T0 input (connected to VSYNC)
//-------------------------------------------------

READ8_MEMBER( pioneer_pr8210_device::i8049_t0_r )
{
	// returns VSYNC state
	return !m_vsync;
}


//-------------------------------------------------
//  i8049_t1_r - return the state of the 8049
//  T1 input (pulled high)
//-------------------------------------------------

READ8_MEMBER( pioneer_pr8210_device::i8049_t1_r )
{
	return 1;
}


//-------------------------------------------------
//  overlay_draw_group - draw a single group of
//  characters
//-------------------------------------------------

void pioneer_pr8210_device::overlay_draw_group(bitmap_yuy16 &bitmap, const UINT8 *text, int count, float xstart)
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
	UINT32 xmin = (UINT32)(xstart * 256.0f * float(bitmap.width()));
	UINT32 xmax = (UINT32)(xend * 256.0f * float(bitmap.width()));

	for (UINT32 y = OVERLAY_Y; y < (OVERLAY_Y + (OVERLAY_Y_PIXELS + 2) * OVERLAY_PIXEL_HEIGHT); y++)
	{
		UINT16 *dest = &bitmap.pix16(y, xmin >> 8);
		UINT16 ymax = *dest >> 8;
		UINT16 ymin = ymax * 3 / 8;
		UINT16 yres = ymin + ((ymax - ymin) * (xmin & 0xff)) / 256;
		*dest = (yres << 8) | (*dest & 0xff);
		dest++;

		for (UINT32 x = (xmin | 0xff) + 1; x < xmax; x += 0x100)
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

void pioneer_pr8210_device::overlay_draw_char(bitmap_yuy16 &bitmap, UINT8 ch, float xstart)
{
	UINT32 xminbase = (UINT32)(xstart * 256.0f * float(bitmap.width()));
	UINT32 xsize = (UINT32)(OVERLAY_PIXEL_WIDTH * 256.0f * float(bitmap.width()));

	// iterate over pixels
	const UINT8 *chdataptr = &text_bitmap[ch & 0x3f][0];
	for (UINT32 y = 0; y < OVERLAY_Y_PIXELS; y++)
	{
		UINT8 chdata = *chdataptr++;

		for (UINT32 x = 0; x < OVERLAY_X_PIXELS; x++, chdata <<= 1)
			if (chdata & 0x80)
			{
				UINT32 xmin = xminbase + x * xsize;
				UINT32 xmax = xmin + xsize;
				for (UINT32 yy = 0; yy < OVERLAY_PIXEL_HEIGHT; yy++)
				{
					UINT16 *dest = &bitmap.pix16(OVERLAY_Y + (y + 1) * OVERLAY_PIXEL_HEIGHT + yy, xmin >> 8);
					UINT16 ymax = 0xff;
					UINT16 ymin = *dest >> 8;
					UINT16 yres = ymin + ((ymax - ymin) * (~xmin & 0xff)) / 256;
					*dest = (yres << 8) | (*dest & 0xff);
					dest++;

					for (UINT32 xx = (xmin | 0xff) + 1; xx < xmax; xx += 0x100)
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

static ADDRESS_MAP_START( simutrek_portmap, AS_IO, 8, simutrek_special_device )
	AM_RANGE(0x00, 0xff) AM_READ(i8748_data_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(i8748_port2_r, i8748_port2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(i8748_t0_r)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( simutrek )
	MCFG_CPU_ADD("simutrek", I8748, XTAL_6MHz)
	MCFG_CPU_IO_MAP(simutrek_portmap)

	MCFG_FRAGMENT_ADD(pr8210)
MACHINE_CONFIG_END


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

simutrek_special_device::simutrek_special_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: pioneer_pr8210_device(mconfig, SIMUTREK_SPECIAL, "Simutrek Modified PR-8210", tag, owner, clock, "simutrek", __FILE__),
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

void simutrek_special_device::data_w(UINT8 data)
{
	synchronize(TID_LATCH_DATA, data);
	if (LOG_SIMUTREK)
		printf("%03d:**** Simutrek Command = %02X\n", screen().vpos(), data);
}


//-------------------------------------------------
//  set_external_audio_squelch - Simutrek-specific
//  command to enable/disable audio squelch
//-------------------------------------------------

void simutrek_special_device::set_external_audio_squelch(int state)
{
	if (LOG_SIMUTREK && m_audio_squelch != (state == 0))
		printf("--> audio squelch = %d\n", state == 0);
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
		printf("%3d:VSYNC(%d)\n", screen().vpos(), fieldnum);
	pioneer_pr8210_device::player_vsync(vbi, fieldnum, curtime);

	// process data
	if (m_data_ready)
	{
		if (LOG_SIMUTREK)
			printf("%3d:VSYNC IRQ\n", screen().vpos());
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

void simutrek_special_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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
			pioneer_pr8210_device::device_timer(timer, id, param, ptr);
			break;
	}
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const rom_entry *simutrek_special_device::device_rom_region() const
{
	return ROM_NAME(simutrek);
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  our machine config fragment
//-------------------------------------------------

machine_config_constructor simutrek_special_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(simutrek);
}


//-------------------------------------------------
//  i8748_port2_r - handle reads from the 8748
//  port #2
//-------------------------------------------------

READ8_MEMBER( simutrek_special_device::i8748_port2_r )
{
	// bit $80 is the pr8210 video squelch
	return (m_i8049_port1 & 0x20) ? 0x00 : 0x80;
}


//-------------------------------------------------
//  i8748_port2_w - handle writes to the 8748
//  port #2
//-------------------------------------------------

WRITE8_MEMBER( simutrek_special_device::i8748_port2_w )
{
	// update stat
	UINT8 prev = m_i8748_port2;
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
			printf("%3d:JUMP TRG (Simutrek PC=%03X)\n", screen().vpos(), space.device().safe_pc());
		advance_slider(direction);
	}

	// bit $04 controls who owns the JUMP TRG command
	if (LOG_SIMUTREK && ((data ^ prev) & 0x04))
		printf("%3d:Simutrek ownership line = %d (Simutrek PC=%03X)\n", screen().vpos(), (data >> 2) & 1, space.device().safe_pc());
	m_controlnext = (~data >> 2) & 1;

	// bits $03 control something (status?)
	if (LOG_SIMUTREK && ((data ^ prev) & 0x03))
		printf("Simutrek Status = %d\n", data & 0x03);
}


//-------------------------------------------------
//  i8748_data_r - handle external 8748 data reads
//-------------------------------------------------

READ8_MEMBER( simutrek_special_device::i8748_data_r )
{
	// acknowledge the read and clear the data ready flag
	m_data_ready = false;
	return m_data;
}


//-------------------------------------------------
//  i8748_t0_r - return the status of the 8748
//  T0 input
//-------------------------------------------------

READ8_MEMBER( simutrek_special_device::i8748_t0_r )
{
	// return 1 if data is waiting from main CPU
	return m_data_ready;
}
