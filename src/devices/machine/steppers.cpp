// license:BSD-3-Clause
// copyright-holders:James Wallace
///////////////////////////////////////////////////////////////////////////
//                                                                       //
// steppers.c steppermotor emulation                                     //
//                                                                       //
// Emulates : Stepper motors driven with full step or half step          //
//            also emulates the index optic                              //
//                                                                       //
// 26-05-2012: J. Wallace - Implemented proper phase alignment, we no    //
//                          longer need reverse interfaces here, the     //
//                          layout will suffice. Added belt reel handler.//
// 09-04-2012: J. Wallace - Studied some old reel motors and added a     //
//                          number of new stepper types. I am yet to     //
//                          add them to drivers, but barring some init   //
//                          stuff, they should work.                     //
// 15-01-2012: J. Wallace - Total internal rewrite to remove the table   //
//                          hoodoo that stops anyone but me actually     //
//                          updating this. In theory, we should be able  //
//                          to adapt the phase code to any reel type by  //
//                          studying a game's startup                    //
//                          Documentation is much better now.            //
// 04-04-2011: J. Wallace - Added reverse spin (this is necessary for    //
//                          accuracy), and improved wraparound logic     //
//    03-2011:              New 2D array to remove reel bounce and       //
//                          make more realistic                          //
// 26-01-2007: J. Wallace - Rewritten to make it more flexible           //
//                          and to allow indices to be set in drivers    //
// 29-12-2006: J. Wallace - Added state save support                     //
// 05-03-2004: Re-Animator                                               //
//                                                                       //
// TODO:  add further types of stepper motors if needed (Konami/IGT?)    //
//        200 Step reels can alter their relative opto tab position,     //
//        may be worth adding the phase setting to the interface         //
//        There are reports that some games use a pulse that is too short//
//        to give a 'judder' effect for holds, etc. We'll need to time   //
//        the pulses to keep tack of this without going out of sync.     //
//        Check 20RM and Starpoint 200 step                              //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include "emu.h"
#include "steppers.h"

#include <array>

DEFINE_DEVICE_TYPE(STEPPER, stepper_device, "stepper", "Stepper Motor")
DEFINE_DEVICE_TYPE(REEL, reel_device, "reel", "Fruit Machine Reel")

stepper_device::stepper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: stepper_device(mconfig, STEPPER, tag, owner, clock)
{
}

stepper_device::stepper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_max_steps(48*2)
	, m_optic_cb(*this)
	, m_output_pos(*this, "pos")
	, m_output_scroll(*this, "scroll")
{
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::update_optic()
{
	int pos   = m_step_pos,
		start = m_index_start,
		end = m_index_end;

	const bool pattern_ok =
		(m_pattern == m_index_patt || m_index_patt == 0);

	if (!pattern_ok)
	{
		m_optic = 0;
	}
	else if (start == end)
	{
		// Single-point optic test mode:
		// assert the optic only when the wrapped reel position matches exactly.
		// This is useful for mechanisms that behave more like a narrow tab / edge
		// than a multi-step window.
		m_optic = (pos == start) ? 1 : 0;
	}
	else if (start > end)	// cope with index patterns that wrap around
	{
		if ((pos > start) || (pos < end))
			m_optic = 1;
		else
			m_optic = 0;
	}
	else
	{
		if ((pos > start) && (pos < end))
			m_optic = 1;
		else
			m_optic = 0;
	}

	m_optic_cb(m_optic);
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::device_start()
{
	// register for state saving
	save_item(NAME(m_index_start));
	save_item(NAME(m_index_end));
	save_item(NAME(m_index_patt));
	save_item(NAME(m_initphase));
	save_item(NAME(m_phase));
	save_item(NAME(m_old_phase));
	save_item(NAME(m_pattern));
	save_item(NAME(m_old_pattern));
	save_item(NAME(m_step_pos));
	save_item(NAME(m_abs_step_pos));
	save_item(NAME(m_max_steps));
	save_item(NAME(m_zero_hold));
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::device_reset()
{
	m_step_pos     = 0x00;
	m_abs_step_pos = 0x00;
	m_pattern      = 0x00;
	m_old_pattern  = 0x00;
	m_phase        = m_initphase;
	m_old_phase    = m_initphase;
	m_zero_hold    = false;
	update_optic();
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::device_post_load()
{
	update_optic();
	draw();
}

///////////////////////////////////////////////////////////////////////////

int stepper_device::update(uint8_t pattern)
{
	int changed = 0;

	if ((m_flags & STEPPER_IGNORE_ZERO) && pattern == 0x00)
	{
		m_zero_hold = true;
		return 0;
	}

	// This code probably makes more sense if you visualise what is being emulated, namely
	// a spinning drum with two electromagnets inside. Essentially, the CPU
	// activates a pair of windings on these magnets leads as necessary to attract and repel the drum to pull it round and
	// display as appropriate. To attempt to visualise the rotation effect, take a look at the compass rose below, representing a side on view of the reel,
	// the numbers indicate the phase information as used
	// 
	//     7
	//     N
	// 1 W   E 5
	//     S
	//     3
	// 
	// For sake of accuracy, we're representing all possible phases of the motor, effectively moving the motor one half step at a time, so a 48 step motor becomes
	// 96 half steps. This is necessary because of some programs running the wiring in series with a distinct delay between the pair being completed. This causes
	// a small movement that may trigger the optic tab.

	const bool zero_hold_restart = m_zero_hold;
	m_zero_hold = false;

	m_pattern = pattern;
	advance_phase();

	int steps = m_old_phase - m_phase;
	if (steps < -4)
	{
		steps = steps +8;
	}
	if (steps > 4)
	{
		steps = steps -8;
	}

	m_old_phase   = m_phase;
	m_old_pattern = m_pattern;

	int max = m_max_steps;
	int pos = m_step_pos;

	if (max!=0)
	{
		if (zero_hold_restart && (m_flags & STEPPER_CLAMP_OPPOSITE_PHASE_START))
		{
			int dir;

			if (m_flags & STEPPER_KICK_DIR_NEGATIVE)
				dir = -1;
			else if (m_flags & STEPPER_KICK_DIR_POSITIVE)
				dir = 1;
			else
				dir = (steps > 0) ? 1 : -1;

			m_abs_step_pos += dir;
			pos = (m_step_pos + dir + max) % max;
		}
		else if ((m_flags & STEPPER_ALLOW_MULTISTEP) && (steps > 1 || steps < -1))
		{
			int dir;

			// Opposite-phase transition (4 half-steps on the 8-phase ring) is direction-ambiguous.
			// When requested, force a fixed kick direction so start-up movement is deterministic.
			if ((steps == 4 || steps == -4) && (m_flags & (STEPPER_KICK_DIR_POSITIVE | STEPPER_KICK_DIR_NEGATIVE)))
				dir = (m_flags & STEPPER_KICK_DIR_NEGATIVE) ? -1 : 1;
			else
				dir = (steps > 0) ? 1 : -1;

			for (int i = 0; i < ((steps < 0) ? -steps : steps); i++)
			{
				m_abs_step_pos += dir;
				pos = (pos + dir + max) % max;
			}
		}
		else
		{
			m_abs_step_pos += steps;
			pos = (m_step_pos + steps + max) % max;
		}
	}

	if (pos != m_step_pos)
	{
		changed++;
	}

	m_step_pos = pos;
	update_optic();

	return changed;
}

///////////////////////////////////////////////////////////////////////////

int stepper_device::write(uint8_t pattern)
{
	int changed = update(pattern);
	draw();

	return changed;
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::restore_position(int abs_pos)
{
	// absolute position
	m_abs_step_pos = abs_pos;

	// visible wrapped position
	if (m_max_steps > 0)
	{
		m_step_pos = abs_pos % m_max_steps;
		if (m_step_pos < 0)
			m_step_pos += m_max_steps;
	}
	else
	{
		m_step_pos = 0;
	}

	// phase/pattern cannot be reconstructed exactly from position alone
	// unless the drive state is also known.  For now, keep them in a
	// neutral state and resynchronise the optic/output state from position.
	m_phase = m_initphase;
	m_old_phase = m_initphase;
	m_pattern = 0;
	m_old_pattern = 0;
	m_zero_hold = false;

	update_optic();
	draw();
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::advance_phase()
{
	// Standard drive table is 2,6,4,5,1,9,8,a
	// NOTE: This runs through the stator patterns in such a way as to drive the reel forward (downwards from the player's view, clockwise on our rose)
	// The Heber 'Pluto' controller runs this in reverse, this needs checking on real hardware
	switch (m_pattern)
	{			// Black  Blue  Red  Yellow
		case 0x02:	//  0     0     1     0
		m_phase = 7;
		break;
		case 0x06:	//  0     1     1     0
		m_phase = 6;
		break;
		case 0x04:	//  0     1     0     0
		m_phase = 5;
		break;
		case 0x05:	//  0     1     0     1
		m_phase = 4;
		break;
		case 0x01:	//  0     0     0     1
		m_phase = 3;
		break;
		case 0x09:	//  1     0     0     1
		m_phase = 2;
		break;
		case 0x08:	//  1     0     0     0
		m_phase = 1;
		break;
		case 0x0A:	//  1     0     1     0
		m_phase = 0;
		break;
				// Black  Blue  Red  Yellow
		case 0x03:	//  0     0     1     1
		{
			if ((m_old_phase ==6)||(m_old_phase == 0))	// if the previous pattern had the drum in the northern quadrant, it will point north now
			{
				m_phase = 7;
			}
			else	// otherwise it will line up due south
			{
				m_phase = 3;
			}
		}
		break;
		case 0x0C:	//  1     1     0     0
		{
			if ((m_old_phase ==6)||(m_old_phase == 4))	// if the previous pattern had the drum in the eastern quadrant, it will point east now
			{
				m_phase = 5;
			}
			else	//otherwise it will line up due west
			{
				m_phase = 1;
			}
		}
		break;
	}
}

namespace
{

struct reel_200step_desc
{
	int16_t max_steps;
	uint8_t flags;
	std::array<uint8_t, 16> pattern_to_phase;
	std::array<uint8_t, 16> settleback_phase;
};

static constexpr uint8_t INVALID_PHASE = 0xff;

std::array<uint8_t, 16> make_pattern_table(
	uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3,
	uint8_t p4, uint8_t p5, uint8_t p6, uint8_t p7)
{
	std::array<uint8_t, 16> table{};
	table.fill(INVALID_PHASE);
	table[p0] = 7;
	table[p1] = 6;
	table[p2] = 5;
	table[p3] = 4;
	table[p4] = 3;
	table[p5] = 2;
	table[p6] = 1;
	table[p7] = 0;
	return table;
}

std::array<uint8_t, 16> make_settleback_table(
	uint8_t from0 = 0xff, uint8_t to0 = 0xff,
	uint8_t from1 = 0xff, uint8_t to1 = 0xff,
	uint8_t from2 = 0xff, uint8_t to2 = 0xff,
	uint8_t from3 = 0xff, uint8_t to3 = 0xff)
{
	std::array<uint8_t, 16> table{};
	table.fill(INVALID_PHASE);
	if (from0 != 0xff)
		table[from0] = to0;
	if (from1 != 0xff)
		table[from1] = to1;
	if (from2 != 0xff)
		table[from2] = to2;
	if (from3 != 0xff)
		table[from3] = to3;
	return table;
}

const reel_200step_desc s_universal_200step_desc {
	200 * 2,
	STEPPER_IGNORE_ZERO | STEPPER_0F_SETTLEBACK,
	make_pattern_table(0x09, 0x08, 0x0c, 0x04, 0x06, 0x02, 0x03, 0x01),
	make_settleback_table(0x03, 0, 0x06, 2, 0x0c, 4, 0x09, 6)
};

const reel_200step_desc s_yamasa_200step_desc {
	200 * 2,
	STEPPER_IGNORE_ZERO | STEPPER_0F_SETTLEBACK,
	make_pattern_table(0x09, 0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08),
	make_settleback_table(0x09, 0, 0x03, 6, 0x06, 4, 0x0c, 2)
};

const reel_200step_desc s_yamasa_252step_desc {
	252 * 2,
	STEPPER_IGNORE_ZERO | STEPPER_0F_SETTLEBACK,
	make_pattern_table(0x09, 0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08),
	make_settleback_table(0x09, 6, 0x03, 4, 0x06, 2, 0x0c, 0)
};

const reel_200step_desc s_sammy_200step_desc {
	200 * 2,
	STEPPER_IGNORE_ZERO | STEPPER_ALLOW_MULTISTEP | STEPPER_0F_SETTLEBACK,
	make_pattern_table(0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09),
	make_settleback_table(0x09, 1, 0x03, 7, 0x06, 5, 0x0c, 3)
};

const reel_200step_desc s_pioneer_200step_desc {
	200 * 2,
	STEPPER_IGNORE_ZERO | STEPPER_0F_SETTLEBACK,
	make_pattern_table(0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09),
	make_settleback_table()
};

const reel_200step_desc s_arktechnico_200step_desc {
	200 * 2,
	STEPPER_IGNORE_ZERO | STEPPER_0F_SETTLEBACK,
	make_pattern_table(0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09),
	make_settleback_table()
};

const reel_200step_desc s_olympia_200step_desc {
	200 * 2,
	STEPPER_IGNORE_ZERO | STEPPER_0F_SETTLEBACK,
	make_pattern_table(0x09, 0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08),
	make_settleback_table(0x09, 6, 0x03, 4, 0x06, 2, 0x0c, 0)
};

const reel_200step_desc *get_200step_desc(uint8_t type)
{
	switch (type)
	{
	case UNIVERSAL_200STEP_REEL:
		return &s_universal_200step_desc;
	case YAMASA_200STEP_REEL:
		return &s_yamasa_200step_desc;
	case YAMASA_252STEP_REEL:
		return &s_yamasa_252step_desc;
	case SAMMY_200STEP_REEL:
		return &s_sammy_200step_desc;
	case PIONEER_200STEP_REEL:
		return &s_pioneer_200step_desc;
	case ARKTECHNICO_200STEP_REEL:
		return &s_arktechnico_200step_desc;
	case OLYMPIA_200STEP_REEL:
		return &s_olympia_200step_desc;
	default:
		return nullptr;
	}
}

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////

void reel_device::set_reel_type(uint8_t type)
{
	m_type = type;
	m_flags = 0;

	if (const reel_200step_desc *const desc = get_200step_desc(type))
	{
		m_max_steps = desc->max_steps;
		m_flags = desc->flags;
		return;
	}

	switch (type)
	{
	default:
	case STARPOINT_48STEP_REEL:	// STARPOINT RMxxx
	case BARCREST_48STEP_REEL:	// Barcrest Reel unit
	case MPU3_48STEP_REEL:
	case GAMESMAN_48STEP_REEL:	// Gamesman GMxxxx
	case PROJECT_48STEP_REEL:
		m_max_steps = (48 * 2);
		break;
	case GAMESMAN_100STEP_REEL:
		m_max_steps = (100 * 2);
		break;
	case STARPOINT_144STEP_DICE:	// STARPOINT 1DCU DICE mechanism
		// Dice reels are 48 step motors, but complete three full cycles between opto updates
		m_max_steps = ((48 * 3) * 2);
		break;
	case STARPOINT_200STEP_REEL:
	case GAMESMAN_200STEP_REEL:
	case ECOIN_200STEP_REEL:
	case SRU_200STEP_REEL:
		m_max_steps = (200 * 2);
		break;
	}
}

void reel_device::advance_phase()
{
	switch (m_type)
	{
		default:
		logerror("No reel type specified!\n");
		break;
		case BASIC_STEPPER:
		case STARPOINT_48STEP_REEL:	// STARPOINT RMxxx
		case GAMESMAN_200STEP_REEL:	// Gamesman GMxxxx
		case STARPOINT_144STEP_DICE:	// STARPOINT 1DCU DICE mechanism
		case STARPOINT_200STEP_REEL:
		case SYS5_100STEP_REEL:
		stepper_device::advance_phase();
		break;

		case BARCREST_48STEP_REEL:
		case GAMESMAN_48STEP_REEL:
		case GAMESMAN_100STEP_REEL:
		// Standard drive table is 1,3,2,6,4,C,8,9
		// Gamesman 48 step uses this pattern shifted one place forward, though this shouldn't matter
		switch (m_pattern)
		{
					// Yellow   Brown  Orange Black
			case 0x01:	//  0        0      0      1
			m_phase = 7;
			break;
			case 0x03:	//  0        0      1      1
			m_phase = 6;
			break;
			case 0x02:	//  0        0      1      0
			m_phase = 5;
			break;
			case 0x06:	//  0        1      1      0
			m_phase = 4;
			break;
			case 0x04:	//  0        1      0      0
			m_phase = 3;
			break;
			case 0x0C:	//  1        1      0      0
			m_phase = 2;
			break;
			case 0x08:	//  1        0      0      0
			m_phase = 1;
			break;	// YOLB
			case 0x09:	//  1        0      0      1
			m_phase = 0;
			break;

			// The below values should not be used by anything sane, as they effectively ignore one stator side entirely
					// Yellow   Brown  Orange Black
			case 0x05:	//   0       1       0     1
			{
				if ((m_old_phase == 6)||(m_old_phase == 0))	// if the previous pattern had the drum in the northern quadrant, it will point north now
				{
					m_phase = 7;
				}
				else	// otherwise it will line up due south
				{
					m_phase = 3;
				}
			}
			break;

			case 0x0A:	//   1       0       1     0
			{
				if ((m_old_phase ==6)||(m_old_phase == 4))	// if the previous pattern had the drum in the eastern quadrant, it will point east now
				{
					m_phase = 5;
				}
				else	// otherwise it will line up due west
				{
					m_phase = 1;
				}
			}
			break;
		}
		break;

		case MPU3_48STEP_REEL:
		// The MPU3 harness is actually the same as the MPU4 setup, but with two active lines instead of four, and a slight change to the windings.
		// Inverters are used so if a pin is low, the higher bit of the pair is activated, and if high the lower bit is activated.

		switch (m_pattern)
		{
					// Grey(1)  Yellow(2) Grey (2) Yellow (2)
			case 0x02:	//   0        1        0        1
			m_phase = 6;
			break;
			case 0x03:	//   0        1        1        0
			m_phase = 4;
			break;
			case 0x01:	//   1        0        1        0
			m_phase = 2;
			break;
			case 0x00:	//   1        0        0        1
			m_phase = 0;
			break;
		}
		break;

		case ECOIN_200STEP_REEL:
		// While the 48 and 100 step models appear to be reverse driven Starpoint reels, the 200 step model seems bespoke, certainly in terms of wiring.
		// On a Proconn machine this same pattern is seen but running in reverse
		// Standard drive table is 8,c,4,6,2,3,1,9
		switch (m_pattern)
		{
			case 0x08:	//  0     0     1     0
			m_phase = 7;
			break;
			case 0x0c:	//  0     1     1     0
			m_phase = 6;
			break;
			case 0x04:	//  0     1     0     0
			m_phase = 5;
			break;
			case 0x06:	//  0     1     0     1
			m_phase = 4;
			break;
			case 0x02:	//  0     0     0     1
			m_phase = 3;
			break;
			case 0x03:	//  1     0     0     1
			m_phase = 2;
			break;
			case 0x01:	//  1     0     0     0
			m_phase = 1;
			break;
			case 0x09:	//  1     0     1     0
			m_phase = 0;
			break;
			case 0x0a:	//  0     0     1     1
			{
				if ((m_old_phase == 6)||(m_old_phase == 0))	// if the previous pattern had the drum in the northern quadrant, it will point north now
				{
					m_phase = 7;
				}
				else	//otherwise it will line up due south
				{
					m_phase = 3;
				}
			}
			break;
			case 0x07:	//  1     1     0     0
			{
				if ((m_old_phase == 6)||(m_old_phase == 4))	// if the previous pattern had the drum in the eastern quadrant, it will point east now
				{
					m_phase = 5;
				}
				else	//otherwise it will line up due west
				{
					m_phase = 1;
				}
			}
			break;
		}
		break;

		case SRU_200STEP_REEL:
		// Standard drive table is 2,3,1,9,8,c,4,6
		// Starpoint mechanism, custom for JPM?
		switch (m_pattern)
		{
			case 0x02:
			m_phase = 7;
			break;
			case 0x03:
			m_phase = 6;
			break;
			case 0x01:
			m_phase = 5;
			break;
			case 0x09:
			m_phase = 4;
			break;
			case 0x08:
			m_phase = 3;
			break;
			case 0x0c:
			m_phase = 2;
			break;
			case 0x04:
			m_phase = 1;
			break;
			case 0x06:
			m_phase = 0;
			break;
		}
		break;

		case PROJECT_48STEP_REEL:
		// Standard drive table is 8,c,4,5,1,3,2,a
		// This appears to be basically a rewired Gamesman (the reel PCB looks like it does some shuffling)
		// TODO: Not sure if this should be represented as a type here, or by defining it as a Gamesman in the driver and bitswapping.
		switch (m_pattern)
		{
			case 0x08:	//  0     0     1     0
			m_phase = 7;
			break;
			case 0x0c:	//  0     1     1     0
			m_phase = 6;
			break;
			case 0x04:	//  0     1     0     0
			m_phase = 5;
			break;
			case 0x05:	//  0     1     0     1
			m_phase = 4;
			break;
			case 0x01:	//  0     0     0     1
			m_phase = 3;
			break;
			case 0x03:	//  1     0     0     1
			m_phase = 2;
			break;
			case 0x02:	//  1     0     0     0
			m_phase = 1;
			break;
			case 0x0a:	//  1     0     1     0
			m_phase = 0;
			break;
			case 0x09:	//  0     0     1     1
			{
				if ((m_old_phase == 6)||(m_old_phase == 0))	// if the previous pattern had the drum in the northern quadrant, it will point north now
				{
					m_phase = 7;
				}
				else	//otherwise it will line up due south
				{
					m_phase = 3;
				}
			}
			break;
			case 0x06:	//  1     1     0     0
			{
				if ((m_old_phase == 6)||(m_old_phase == 4))	// if the previous pattern had the drum in the eastern quadrant, it will point east now
				{
					m_phase = 5;
				}
				else	// otherwise it will line up due west
				{
					m_phase = 1;
				}
			}
			break;
		}
		break;

		case UNIVERSAL_200STEP_REEL:
		case YAMASA_200STEP_REEL:
		case YAMASA_252STEP_REEL:
		case SAMMY_200STEP_REEL:
		case PIONEER_200STEP_REEL:
		case ARKTECHNICO_200STEP_REEL:
		case OLYMPIA_200STEP_REEL:
		{
			const reel_200step_desc *const desc = get_200step_desc(m_type);
			assert(desc != nullptr);

			if (m_pattern == 0x0f)
			{
				if (desc->flags & STEPPER_0F_SETTLEBACK)
				{
					const uint8_t phase = desc->settleback_phase[m_old_pattern & 0x0f];
					if (phase != INVALID_PHASE)
						m_phase = phase;
				}
				break;
			}

			const uint8_t phase = desc->pattern_to_phase[m_pattern & 0x0f];
			if (phase != INVALID_PHASE)
				m_phase = phase;
			break;
		}
		break;
	}
}

///////////////////////////////////////////////////////////////////////////

reel_device::reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t type, int16_t start_index, int16_t end_index
	, int16_t index_pattern, uint8_t init_phase, int16_t max_steps)
	: stepper_device(mconfig, tag, owner)
{
	set_reel_type(type);
	set_start_index(start_index);
	set_end_index(end_index);
	set_index_pattern(index_pattern);
	set_init_phase(init_phase);
	set_max_steps(max_steps);
}

///////////////////////////////////////////////////////////////////////////

reel_device::reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: stepper_device(mconfig, REEL, tag, owner, clock)
{
}

///////////////////////////////////////////////////////////////////////////

void reel_device::device_start()
{
	stepper_device::device_start();

	save_item(NAME(m_type));
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::draw()
{
	int reelpos = get_position();
	if (reelpos != m_output_pos)
	{
		m_output_pos = reelpos;

		/* update scroll output if configured */
		if (get_max())
		{
			int sreelpos = (reelpos * 0x10000) / get_max();
			m_output_scroll = sreelpos;
		}
	}
}
