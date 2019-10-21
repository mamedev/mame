// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    1ma6.cpp

    HP-85 tape controller (1MA6-0001)

    *Status of tape tests of service ROM*

    Test Code   Description     Status
    ==================================
    I           Write protect   OK
    P           Status test     OK
    Q           Speed test      OK
    R           Hole detection  Fails with "HOLE D" error (*1)
    S           Write test      OK
    T           Read test       OK
    U           Record test     OK (*2)

    *1 Hole test fails because it depends on the diameter of the
       holes being correct (this driver doesn't emulate it).
    *2 Record test is buggy (byte @74ab in service ROM should be 0x54).
       Test succeeds if this bug is corrected first.

*********************************************************************/

#include "emu.h"
#include "1ma6.h"

// Debugging
#include "logmacro.h"
#define LOG_DBG_MASK (LOG_GENERAL << 1)
#define LOG_DBG(...) LOGMASKED(LOG_DBG_MASK, __VA_ARGS__)
#define LOG_RW_MASK (LOG_DBG_MASK << 1)
#define LOG_RW(...) LOGMASKED(LOG_RW_MASK, __VA_ARGS__)
#undef VERBOSE
//#define VERBOSE (LOG_GENERAL | LOG_DBG_MASK | LOG_RW_MASK)
#define VERBOSE 0

// Device type definition
DEFINE_DEVICE_TYPE(HP_1MA6, hp_1ma6_device, "hp_1ma6", "HP 1MA6")

// Bit manipulation
namespace {
	static constexpr unsigned BIT_MASK(unsigned n)
	{
		return 1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~(T)BIT_MASK(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= (T)BIT_MASK(n);
	}

	template<typename T> void COPY_BIT(bool bit , T& w , unsigned n)
	{
		if (bit) {
			BIT_SET(w , n);
		} else {
			BIT_CLR(w , n);
		}
	}
}

// **** Constants ****
constexpr double FAST_SPEED = 60.0;             // Fast speed: 60 ips
constexpr double SLOW_SPEED = 10.0;             // Slow speed: 10 ips
constexpr double MOVING_THRESHOLD = 2.0;        // Tape is moving when speed > 2.0 ips
constexpr double ACCELERATION = 1200.0;         // Acceleration when speed set point is changed: 1200 ips^2
// One tachometer tick every 1/32 of inch
constexpr hti_format_t::tape_pos_t TACH_TICK_LEN = hti_format_t::ONE_INCH_POS / 32;
// Minimum gap size (totally made up)
constexpr hti_format_t::tape_pos_t MIN_GAP_SIZE = TACH_TICK_LEN;

// Bits in control register
enum control_bits : unsigned
{
	CTL_TRACK_NO_BIT = 0,     // Track selection
	CTL_POWER_UP_BIT = 1,     // Tape controller power up
	CTL_MOTOR_ON_BIT = 2,     // Motor control
	CTL_DIR_FWD_BIT = 3,      // Tape direction = forward
	CTL_FAST_BIT = 4,         // Speed = fast
	CTL_WRITE_DATA_BIT = 5,   // Write data
	CTL_WRITE_SYNC_BIT = 6,   // Write SYNC
	CTL_WRITE_GAP_BIT = 7     // Write gap
};

// Bits in status register
enum status_bits : unsigned
{
	STS_CASSETTE_IN_BIT = 0,  // Cassette in
	STS_STALL_BIT = 1,        // Tape stalled
	STS_ILIM_BIT = 2,         // Overcurrent
	STS_WRITE_EN_BIT = 3,     // Write enabled
	STS_HOLE_BIT = 4,         // Hole detected
	STS_GAP_BIT = 5,          // Gap detected
	STS_TACH_BIT = 6,         // Tachometer tick
	STS_READY_BIT = 7         // Ready
};

// Timers
enum {
	TAPE_TMR_ID,
	HOLE_TMR_ID
};

hp_1ma6_device::hp_1ma6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP_1MA6 , tag , owner , clock)
	, m_tape(*this , "drive")
{
	clear_state();
}

WRITE8_MEMBER(hp_1ma6_device::reg_w)
{
	LOG("WR %u=%02x\n" , offset , data);
	switch(offset) {
	case 0:
		// Control register
		start_cmd_exec(data);
		break;

	case 1:
		// Data register
		m_data_reg = data;
		break;
	}
}

READ8_MEMBER(hp_1ma6_device::reg_r)
{
	uint8_t res = 0;

	switch (offset) {
	case 0:
		// Status register
		m_tape->update_speed_pos();
		if (m_cmd_state == CMD_IDLE) {
			BIT_SET(m_status_reg , STS_READY_BIT);
		} else if (m_cmd_state == CMD_STOPPING ||
				   m_cmd_state == CMD_FAST_FWD_REV ||
				   m_cmd_state == CMD_WR_GAP) {
			BIT_CLR(m_status_reg , STS_READY_BIT);
		}
		if ((m_control_reg & (BIT_MASK(CTL_WRITE_SYNC_BIT) |
							  BIT_MASK(CTL_WRITE_GAP_BIT))) != 0 &&
			(m_tape->cart_out_r() || !m_tape->wpr_r())) {
			BIT_SET(m_status_reg , STS_WRITE_EN_BIT);
		} else {
			BIT_CLR(m_status_reg , STS_WRITE_EN_BIT);
		}
		// Gap detection
		if (m_cmd_state == CMD_IDLE ||
			m_tape->gap_reached(MIN_GAP_SIZE)) {
			BIT_SET(m_status_reg , STS_GAP_BIT);
		}

		res = m_status_reg;
		// Clear latching bits
		BIT_CLR(m_status_reg , STS_HOLE_BIT);
		BIT_CLR(m_status_reg , STS_GAP_BIT);
		BIT_CLR(m_status_reg , STS_TACH_BIT);
		BIT_CLR(m_status_reg , STS_READY_BIT);
		if (!m_tape->cart_out_r()) {
			BIT_SET(m_status_reg , STS_CASSETTE_IN_BIT);
		}
		break;

	case 1:
		// Data register
		res = m_data_reg;
		break;
	}
	LOG("RD %u=%02x\n" , offset , res);
	return res;
}

WRITE_LINE_MEMBER(hp_1ma6_device::cart_out_w)
{
	LOG_DBG("cart_out_w %d\n" , state);
	if (state) {
		// STS_CASSETTE_IN_BIT is set by reading status register
		BIT_CLR(m_status_reg , STS_CASSETTE_IN_BIT);
	}
}

WRITE_LINE_MEMBER(hp_1ma6_device::hole_w)
{
	if (state) {
		LOG_DBG("hole_w\n");
		BIT_SET(m_status_reg , STS_HOLE_BIT);
	}
}

WRITE_LINE_MEMBER(hp_1ma6_device::tacho_tick_w)
{
	if (state) {
		LOG_DBG("tacho_tick_w\n");
		BIT_SET(m_status_reg , STS_TACH_BIT);
	}
}

WRITE_LINE_MEMBER(hp_1ma6_device::motion_w)
{
	if (state) {
		LOG_DBG("motion_w @%.6f st=%d\n" , machine().time().as_double() , m_cmd_state);
		switch (m_cmd_state) {
		case CMD_STOPPING:
			if (!m_tape->is_moving()) {
				m_cmd_state = CMD_IDLE;
			}
			break;

		case CMD_STARTING:
			{
				hp_dc100_tape_device::tape_op_t op = hp_dc100_tape_device::OP_IDLE;

				switch (m_control_reg & (BIT_MASK(CTL_FAST_BIT) |
										 BIT_MASK(CTL_WRITE_DATA_BIT) |
										 BIT_MASK(CTL_WRITE_SYNC_BIT) |
										 BIT_MASK(CTL_WRITE_GAP_BIT))) {
				case 0:
					if (m_tape->is_above_threshold()) {
						// Start RD
						m_cmd_state = CMD_RD_WAIT_SYNC;
						op = hp_dc100_tape_device::OP_READ;
					}
					break;

				case BIT_MASK(CTL_FAST_BIT):
				case BIT_MASK(CTL_WRITE_GAP_BIT):
				case BIT_MASK(CTL_WRITE_GAP_BIT) | BIT_MASK(CTL_WRITE_DATA_BIT):
					// Start simple movement
					m_cmd_state = CMD_FAST_FWD_REV;
					break;

				case BIT_MASK(CTL_WRITE_DATA_BIT):
					if (m_tape->is_above_threshold()) {
						// Start re-writing
						m_cmd_state = CMD_WR_WAIT_SYNC;
						// Need to achieve sync first (hence RD op)
						op = hp_dc100_tape_device::OP_READ;
					}
					break;

				case BIT_MASK(CTL_WRITE_SYNC_BIT):
					if (m_tape->is_above_threshold()) {
						// Start WR
						load_wr_word();
						m_cmd_state = CMD_WR_PREAMBLE;
						op = hp_dc100_tape_device::OP_WRITE;
					}
					break;

				case BIT_MASK(CTL_WRITE_SYNC_BIT) | BIT_MASK(CTL_WRITE_GAP_BIT):
					if (m_tape->is_above_threshold()) {
						// Start erasing (gap writing)
						m_cmd_state = CMD_WR_GAP;
						op = hp_dc100_tape_device::OP_ERASE;
					}
					break;

				default:
					break;
				}
				m_tape->set_op(op);
			}
			break;

		default:
			break;
		}
	}
}

WRITE_LINE_MEMBER(hp_1ma6_device::rd_bit_w)
{
	LOG_RW("RD bit %d (st=%d,sr=%02x,i=%u)\n" , state , m_cmd_state , m_data_sr , m_bit_idx);
	switch (m_cmd_state) {
	case CMD_RD_WAIT_SYNC:
		if (state) {
			// Got sync (bit is 1)
			LOG_RW("RD synced\n");
			m_cmd_state = CMD_RD;
			m_bit_idx = 7;
			m_data_sr = 0;
		}
		break;

	case CMD_RD:
		if (state) {
			BIT_SET(m_data_sr , m_bit_idx);
		}
		if (m_bit_idx) {
			m_bit_idx--;
		} else {
			LOG_RW("RD byte %02x\n" , m_data_sr);
			m_data_reg = m_data_sr;
			m_bit_idx = 7;
			m_data_sr = 0;
			BIT_SET(m_status_reg , STS_READY_BIT);
		}
		break;

	case CMD_WR_WAIT_SYNC:
		m_cmd_state = CMD_WR_PREAMBLE;
		load_wr_word();
		m_tape->set_op(hp_dc100_tape_device::OP_WRITE);
		break;

	default:
		break;
	}
}

READ_LINE_MEMBER(hp_1ma6_device::wr_bit_r)
{
	bool bit = m_cmd_state == CMD_WR_PREAMBLE ? false : BIT(m_data_sr , m_bit_idx);
	if (m_bit_idx) {
		m_bit_idx--;
	} else {
		if (m_cmd_state == CMD_WR) {
			load_wr_word();
		}
		m_cmd_state = CMD_WR;
		m_bit_idx = 7;
	}
	LOG_RW("WR bit %d (sr=%02x,i=%u)\n" , bit , m_data_sr , m_bit_idx);
	return bit;
}

void hp_1ma6_device::device_add_mconfig(machine_config &config)
{
	HP_DC100_TAPE(config , m_tape , 0);
	m_tape->set_acceleration(ACCELERATION);
	m_tape->set_set_points(SLOW_SPEED , FAST_SPEED);
	m_tape->set_tick_size(TACH_TICK_LEN);
	m_tape->set_bits_per_word(16);
	m_tape->set_go_threshold(MOVING_THRESHOLD);
	m_tape->cart_out().set(FUNC(hp_1ma6_device::cart_out_w));
	m_tape->hole().set(FUNC(hp_1ma6_device::hole_w));
	m_tape->tacho_tick().set(FUNC(hp_1ma6_device::tacho_tick_w));
	m_tape->motion_event().set(FUNC(hp_1ma6_device::motion_w));
	m_tape->rd_bit().set(FUNC(hp_1ma6_device::rd_bit_w));
	m_tape->wr_bit().set(FUNC(hp_1ma6_device::wr_bit_r));
}

void hp_1ma6_device::device_start()
{
	save_item(NAME(m_data_reg));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_control_reg));
	save_item(NAME(m_bit_idx));
	save_item(NAME(m_data_sr));
}

void hp_1ma6_device::device_reset()
{
	clear_state();
}

void hp_1ma6_device::clear_state()
{
	m_data_reg = 0;
	m_status_reg = 0;
	m_control_reg = 0;
	m_bit_idx = 0;
	m_data_sr = 0;
	m_cmd_state = CMD_IDLE;
}

void hp_1ma6_device::load_wr_word()
{
	m_bit_idx = 7;
	m_data_sr = m_data_reg;
	LOG_RW("WR byte %02x\n" , m_data_sr);
	BIT_SET(m_status_reg , STS_READY_BIT);
}

void hp_1ma6_device::start_cmd_exec(uint8_t new_ctl_reg)
{
	m_control_reg = new_ctl_reg;

	if (!BIT(new_ctl_reg , CTL_POWER_UP_BIT) ||
		!BIT(new_ctl_reg , CTL_MOTOR_ON_BIT)) {
		m_cmd_state = CMD_STOPPING;
	} else {
		m_cmd_state = CMD_STARTING;
	}

	m_tape->set_op(hp_dc100_tape_device::OP_IDLE);
	m_tape->set_track_no(BIT(m_control_reg , CTL_TRACK_NO_BIT));

	hp_dc100_tape_device::tape_speed_t new_speed = hp_dc100_tape_device::SP_STOP;
	if (BIT(new_ctl_reg , CTL_POWER_UP_BIT) &&
		BIT(new_ctl_reg , CTL_MOTOR_ON_BIT)) {
		new_speed = BIT(new_ctl_reg , CTL_FAST_BIT) ? hp_dc100_tape_device::SP_FAST :
			hp_dc100_tape_device::SP_SLOW;
	}

	m_tape->set_speed_setpoint(new_speed , BIT(new_ctl_reg , CTL_DIR_FWD_BIT));
	motion_w(1);
}
