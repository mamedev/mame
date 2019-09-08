// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp_dc100_tape.cpp

    HP DC100 tape cartridge & drive

    This driver emulates the DC100 tape drive as used by HP9825, 9845
    and 85 systems.

    Main features:
    - Handles HTI tape images
    - Supports 3 speed setpoints (stop, slow & fast) and forward/reverse
      tape direction
    - Applies a constant acceleration whenever a change of speed setpoint
      and/or direction occurs
    - Reports through a callback "motion" events such as the reaching/leaving
      of setpoint, the crossing of speed thresholds, etc.
    - Simulates tachometer ticks
    - Handles 3 different tape operations: reading, writing and erasing.
    - Simulates the serial reading and writing of bits from/to tape
      and their correct timing
    - Reports various tape conditions as the presence of cartridge,
      of tape holes, of the write-protection switch etc.
    - Checks for gaps (erased zones) of arbitrary length

*********************************************************************/

#include "emu.h"
#include "hp_dc100_tape.h"

// Debugging
#include "logmacro.h"
#define LOG_TMR_MASK (LOG_GENERAL << 1)
#define LOG_TMR(...) LOGMASKED(LOG_TMR_MASK, __VA_ARGS__)
#define LOG_DBG_MASK (LOG_TMR_MASK << 1)
#define LOG_DBG(...) LOGMASKED(LOG_DBG_MASK, __VA_ARGS__)
#define LOG_RW_MASK (LOG_DBG_MASK << 1)
#define LOG_RW(...) LOGMASKED(LOG_RW_MASK, __VA_ARGS__)
#undef VERBOSE
//#define VERBOSE (LOG_GENERAL | LOG_TMR_MASK | LOG_DBG_MASK | LOG_RW_MASK)
#define VERBOSE (LOG_GENERAL)

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Device type definition
DEFINE_DEVICE_TYPE(HP_DC100_TAPE, hp_dc100_tape_device, "hp_dc100_tape", "HP DC100 tape drive")

// Timers
enum {
	BIT_TMR_ID,
	TACHO_TMR_ID,
	HOLE_TMR_ID,
	MOTION_TMR_ID
};

// Constants
constexpr double MOTION_MARGIN = 1e-5;  // Margin to ensure motion events have passed when timer expires (10 µs)
constexpr hti_format_t::tape_pos_t TAPE_INIT_POS = 80 * hti_format_t::ONE_INCH_POS; // Initial tape position: 80" from beginning (just past the punched part)

hp_dc100_tape_device::hp_dc100_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP_DC100_TAPE , tag , owner , clock)
	, device_image_interface(mconfig , *this)
	, m_cart_out_handler(*this)
	, m_hole_handler(*this)
	, m_tacho_tick_handler(*this)
	, m_motion_handler(*this)
	, m_rd_bit_handler(*this)
	, m_wr_bit_handler(*this)
	, m_image()
	, m_image_dirty(false)
{
}

image_init_result hp_dc100_tape_device::call_load()
{
	return internal_load(false);
}

image_init_result hp_dc100_tape_device::call_create(int format_type, util::option_resolution *format_options)
{
	return internal_load(true);
}

void hp_dc100_tape_device::call_unload()
{
	LOG("call_unload dirty=%d\n" , m_image_dirty);

	device_reset();

	if (m_image_dirty) {
		io_generic io;
		io.file = (device_image_interface *)this;
		io.procs = &image_ioprocs;
		io.filler = 0;
		m_image.save_tape(&io);
		m_image_dirty = false;
	}

	m_image.clear_tape();
	set_tape_present(false);
}

std::string hp_dc100_tape_device::call_display()
{
	std::string buffer;
	// Mostly lifted from cassette_image_device::call_display ;)

	// Do not show anything if image not loaded or tape not moving
	if (cart_out_r() || !has_started()) {
		return buffer;
	}

	char track = m_track ? 'B' : 'A';
	char r_w = m_current_op == OP_WRITE || m_current_op == OP_ERASE ? 'W' : 'R';
	char m1;
	char m2;

	if (is_moving_fwd()) {
		m1 = '>';
		m2 = m_tape_speed == SP_FAST ? '>' : ' ';
	} else {
		m1 = '<';
		m2 = m_tape_speed == SP_FAST ? '<' : ' ';
	}

	int pos_in = get_approx_pos() / hti_format_t::ONE_INCH_POS;

	buffer = string_format("%c %c %c%c [%04d/1824]" , track , r_w , m1 , m2 , pos_in);

	return buffer;
}

const char *hp_dc100_tape_device::file_extensions() const
{
	return "hti";
}

void hp_dc100_tape_device::set_acceleration(double accel)
{
	m_acceleration = accel;
}

void hp_dc100_tape_device::set_set_points(double slow_sp , double fast_sp)
{
	m_slow_set_point = slow_sp;
	m_fast_set_point = fast_sp;
}

void hp_dc100_tape_device::set_tick_size(hti_format_t::tape_pos_t size)
{
	m_tick_size = size;
}

void hp_dc100_tape_device::set_bits_per_word(unsigned bits)
{
	m_image.set_bits_per_word(bits);
}

void hp_dc100_tape_device::set_go_threshold(double threshold)
{
	m_go_threshold = threshold;
}

void hp_dc100_tape_device::set_track_no(unsigned track)
{
	if (m_track != track) {
		LOG_DBG("Setting track %u (op=%d)\n" , track , static_cast<int>(m_current_op));
		auto saved_op = m_current_op;
		if (m_current_op != OP_IDLE) {
			// Close current op on old track
			set_op(OP_IDLE);
		}
		m_track = track;
		if (saved_op != OP_IDLE) {
			// Resume op on new track
			set_op(saved_op);
		}
	}
}

bool hp_dc100_tape_device::set_speed_setpoint(tape_speed_t speed , bool fwd)
{
	if (!m_present) {
		return false;
	}

	double new_setpoint = compute_set_point(speed, fwd);

	if (m_set_point != new_setpoint) {
		update_speed_pos();
		LOG_DBG("Speed SP changed %f->%f %.6f p=%d\n" , m_set_point , new_setpoint , machine().time().as_double() , m_tape_pos);
		m_tape_speed = speed;
		m_set_point = new_setpoint;
		// Speed set point changed, accelerate/decelerate
		m_accelerating = true;
		if (m_start_time.is_never()) {
			// Tape starting now
			start_tape();
			// When tape starts m_current_op is always OP_IDLE
			m_gap_detect_start = m_tape_pos;
		}
		adjust_tacho_timer();
		adjust_hole_timer();
		set_motion_timer();
		return true;
	} else {
		return false;
	}
}

void hp_dc100_tape_device::set_op(tape_op_t op , bool force)
{
	if (!m_present || m_start_time.is_never()) {
		return;
	}
	if (!m_in_set_op && (op != m_current_op || force)) {
		LOG_DBG("Op %d->%d (f=%d)\n" , m_current_op , op , force);
		m_in_set_op = true;
		update_speed_pos();
		auto prev_op = m_current_op;
		// Close current operating state
		stop_op();
		m_in_set_op = false;
		m_current_op = op;

		switch (op) {
		case OP_IDLE:
			m_gap_detect_start = m_tape_pos;
			break;

		case OP_READ:
			if (prev_op == OP_WRITE) {
				LOG("Starting RD after WR?\n");
			}
			if (fabs(m_speed) < m_slow_set_point) {
				LOG("Starting RD at %f speed?\n" , m_speed);
			}
			m_rd_it_valid = m_image.next_data(get_track_no() , m_tape_pos , is_moving_fwd() , false , m_rd_it);
			load_rd_word();
			m_gap_detect_start = m_tape_pos;
			break;

		case OP_WRITE:
			if (m_accelerating || fabs(m_speed) != m_slow_set_point) {
				LOG("Starting WR at %f speed (acc=%d)?\n" , m_speed , m_accelerating);
			}
			if (prev_op == OP_READ) {
				// Switching from RD to WR
				// Clear out the part of m_rw_word that is going to be written
				LOG_RW("Switch RD->WR @%d, idx=%d, w=%04x\n" , m_rw_pos , m_bit_idx , m_rw_word);
				if (is_moving_fwd()) {
					if (--m_bit_idx >= 0) {
						m_rw_word &= 0xffffU << (m_bit_idx + 1);
					} else {
						m_bit_idx = 15;
						m_rw_pos = m_next_bit_pos;
						m_rw_word = 0;
					}
				} else {
					if (++m_bit_idx < 16) {
						m_rw_word &= 0xffffU >> (16 - m_bit_idx);
					} else {
						m_bit_idx = 0;
						m_rw_pos = m_next_bit_pos;
						m_rw_word = 0;
					}
				}
				time_to_distance(m_next_bit_pos - m_tape_pos, m_next_bit_pos, m_bit_timer);
			} else {
				m_rw_word = 0;
				if (is_moving_fwd()) {
					m_bit_idx = 15;
				} else {
					m_bit_idx = 0;
				}
				m_next_bit_pos = m_rw_pos = m_tape_pos;
				m_bit_timer->adjust(attotime::zero);
			}
			break;

		case OP_ERASE:
			LOG_DBG("Start GAP @%d\n" , m_tape_pos);
			m_rw_pos = m_tape_pos;
			break;

		default:
			LOG("Invalid op!\n");
			break;
		}
	}
}

void hp_dc100_tape_device::update_speed_pos()
{
	if (m_start_time.is_never()) {
		// Tape stopped
		return;
	}

	attotime delta_time{machine().time() - m_start_time};
	if (delta_time.is_zero()) {
		return;
	}
	m_start_time = machine().time();
	double delta_time_double = delta_time.as_double();

	// Space in const A phase
	double space_const_a;
	// Time in const V phase
	double time_const_v;
	// Report a motion event
	bool motion_event = false;
	// Direction inverted
	bool inverted = false;

	if (m_accelerating) {
		auto prev_speed = m_speed;
		double time_to_const_v = fabs(m_set_point - m_speed) / m_acceleration;
		double acceleration = m_set_point > m_speed ? m_acceleration : -m_acceleration;
		bool retrigger_motion = false;
		if (delta_time_double < time_to_const_v) {
			space_const_a = const_a_space(acceleration , delta_time_double);
			m_speed += delta_time_double * acceleration;
			time_const_v = 0.0;
		} else {
			space_const_a = const_a_space(acceleration , time_to_const_v);
			time_const_v = delta_time_double - time_to_const_v;
			LOG_DBG("Acceleration ends\n");
			m_accelerating = false;
			m_speed = m_set_point;
			motion_event = true;
			retrigger_motion = true;
			if (m_speed == 0.0) {
				// Tape stops
				stop_tape();
			}
		}
		if ((fabs(prev_speed) - m_slow_set_point) * (fabs(m_speed) - m_slow_set_point) <= 0.0 ||
			(fabs(prev_speed) - m_go_threshold) * (fabs(m_speed) - m_go_threshold) < 0.0) {
			// Slow speed threshold crossed
			// In-motion threshold crossed
			LOG_DBG("Thr crossed %f->%f\n" , prev_speed , m_speed);
			motion_event = true;
			retrigger_motion = true;
		}
		if (prev_speed * m_speed < 0.0) {
			// Direction inverted (speed sign flipped)
			LOG_DBG("Dir inverted s=%f\n" , m_speed);
			inverted = true;
			retrigger_motion = true;
		}
		if (retrigger_motion) {
			set_motion_timer();
		}
	} else {
		space_const_a = 0.0;
		time_const_v = delta_time_double;
	}

	hti_format_t::tape_pos_t delta_pos = (hti_format_t::tape_pos_t)((space_const_a + m_speed * time_const_v) * hti_format_t::ONE_INCH_POS);
	LOG_DBG("dp=%d\n" , delta_pos);
	if (!hti_format_t::pos_offset(m_tape_pos , true , delta_pos)) {
		LOG("Tape unspooled!\n");
	}

	if (inverted) {
		adjust_tacho_timer();
		adjust_hole_timer();
		if (m_gap_detect_start != hti_format_t::NULL_TAPE_POS) {
			m_gap_detect_start = m_tape_pos;
		}
	}
	if (motion_event) {
		// Must be done here or pos & speed won't be correct
		m_motion_handler(1);
	}
}

hti_format_t::tape_pos_t hp_dc100_tape_device::get_approx_pos() const
{
	if (m_start_time.is_never()) {
		// Tape not moving
		return m_tape_pos;
	}

	attotime delta_time{ machine().time() - m_start_time };
	hti_format_t::tape_pos_t delta_pos = (hti_format_t::tape_pos_t)(delta_time.as_double() * m_speed * hti_format_t::ONE_INCH_POS);
	auto tape_pos = m_tape_pos;
	hti_format_t::pos_offset(tape_pos , true , delta_pos);
	return tape_pos;
}

bool hp_dc100_tape_device::gap_reached(hti_format_t::tape_pos_t min_gap_size)
{
	update_speed_pos();

	if (m_gap_detect_start != hti_format_t::NULL_TAPE_POS &&
		abs(m_gap_detect_start - m_tape_pos) >= min_gap_size) {
		auto tmp = m_tape_pos;
		hti_format_t::pos_offset(tmp , is_moving_fwd() , -min_gap_size);
		if (m_image.just_gap(get_track_no() , tmp , m_tape_pos)) {
			return true;
		}
	}
	return false;
}

void hp_dc100_tape_device::time_to_next_gap(hti_format_t::tape_pos_t min_gap_size , bool new_gap , emu_timer *target_timer)
{
	update_speed_pos();

	bool fwd = is_moving_fwd();
	hti_format_t::tape_pos_t tmp = m_tape_pos;
	bool found = true;
	if (new_gap) {
		hti_format_t::track_iterator_t it;
		found = m_image.next_data(get_track_no() , tmp , fwd , true , it);
		if (found) {
			tmp = hti_format_t::farthest_end(it , !fwd);
		}
	}
	if (found && m_image.next_gap(get_track_no() , tmp , fwd , min_gap_size)) {
		hti_format_t::tape_pos_t dummy;
		LOG_DBG("TTNG T%u S%d N%d %d->%d\n" , get_track_no() , min_gap_size , new_gap , m_tape_pos , tmp);
		time_to_distance(tmp - m_tape_pos, dummy, target_timer);
	} else {
		LOG_DBG("TTNG T%u S%d N%d %d->X\n" , get_track_no() , min_gap_size , new_gap , m_tape_pos);
		target_timer->reset();
	}
}

void hp_dc100_tape_device::device_start()
{
	m_cart_out_handler.resolve_safe();
	m_hole_handler.resolve_safe();
	m_tacho_tick_handler.resolve_safe();
	m_motion_handler.resolve_safe();
	m_rd_bit_handler.resolve_safe();
	m_wr_bit_handler.resolve_safe(0);

	save_item(NAME(m_acceleration));
	save_item(NAME(m_slow_set_point));
	save_item(NAME(m_fast_set_point));
	save_item(NAME(m_tick_size));
	save_item(NAME(m_go_threshold));
	save_item(NAME(m_tape_pos));
	save_item(NAME(m_set_point));
	save_item(NAME(m_speed));
	save_item(NAME(m_start_time));
	save_item(NAME(m_accelerating));
	save_item(NAME(m_track));
	save_item(NAME(m_in_set_op));
	save_item(NAME(m_present));
	save_item(NAME(m_rd_it_valid));
	save_item(NAME(m_rw_word));
	save_item(NAME(m_bit_idx));
	save_item(NAME(m_rw_pos));
	save_item(NAME(m_gap_detect_start));
	save_item(NAME(m_next_bit_pos));
	save_item(NAME(m_next_tacho_pos));
	save_item(NAME(m_next_hole_pos));
	save_item(NAME(m_image_dirty));

	m_bit_timer = timer_alloc(BIT_TMR_ID);
	m_tacho_timer = timer_alloc(TACHO_TMR_ID);
	m_hole_timer = timer_alloc(HOLE_TMR_ID);
	m_motion_timer = timer_alloc(MOTION_TMR_ID);
}

void hp_dc100_tape_device::device_reset()
{
	clear_state();
}

void hp_dc100_tape_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG_TMR("%.6f TMR %d p=%d s=%.3f(%.3f) a=%d\n" , machine().time().as_double() , id , m_tape_pos , m_speed , m_set_point , m_accelerating);
	update_speed_pos();

	switch (id) {
	case BIT_TMR_ID:
		m_tape_pos = m_next_bit_pos;
		if (m_current_op == OP_READ) {
			bool bit = BIT(m_rd_it->second , m_bit_idx);
			m_rd_bit_handler(bit);
			if (is_moving_fwd()) {
				if (--m_bit_idx >= 0) {
					time_to_distance(m_image.bit_length(BIT(m_rd_it->second , m_bit_idx)), m_next_bit_pos, m_bit_timer);
				} else {
					m_rd_it_valid = m_image.adv_it(get_track_no() , true , m_rd_it) != hti_format_t::ADV_NO_MORE_DATA;
					load_rd_word();
				}
			} else {
				if (++m_bit_idx < 16) {
					time_to_distance(-m_image.bit_length(BIT(m_rd_it->second , m_bit_idx)), m_next_bit_pos, m_bit_timer);
				} else {
					m_rd_it_valid = m_image.adv_it(get_track_no() , false , m_rd_it) != hti_format_t::ADV_NO_MORE_DATA;
					load_rd_word();
				}
			}
		} else if (m_current_op == OP_WRITE) {
			bool bit = m_wr_bit_handler();
			hti_format_t::tape_pos_t bit_len = m_image.bit_length(bit);
			if (bit) {
				BIT_SET(m_rw_word , m_bit_idx);
			}
			if (is_moving_fwd()) {
				if (--m_bit_idx < 0) {
					store_wr_word();
				}
			} else {
				if (++m_bit_idx >= 16) {
					store_wr_word();
				}
				bit_len = -bit_len;
			}
			time_to_distance(bit_len, m_next_bit_pos, m_bit_timer);
		}
		break;

	case TACHO_TMR_ID:
		m_tape_pos = m_next_tacho_pos;
		m_tacho_tick_handler(1);
		adjust_tacho_timer();
		break;

	case HOLE_TMR_ID:
		m_tape_pos = m_next_hole_pos;
		m_hole_handler(1);
		adjust_hole_timer();
		break;

	case MOTION_TMR_ID:
		// In itself it does nothing (all work is in update_speed_pos)
		break;

	default:
		break;
	}
}

void hp_dc100_tape_device::clear_state()
{
	m_tape_pos = TAPE_INIT_POS;
	m_tape_speed = SP_STOP;
	m_set_point = 0.0;
	m_speed = 0.0;
	m_start_time = attotime::never;
	m_accelerating = false;
	m_track = 0;
	m_current_op = OP_IDLE;
	m_in_set_op = false;
	m_present = true;
	m_rd_it_valid = false;
	m_rw_word = 0;
	m_bit_idx = 0;
	m_rw_pos = hti_format_t::NULL_TAPE_POS;
	m_gap_detect_start = hti_format_t::NULL_TAPE_POS;
	m_bit_timer->reset();
	m_tacho_timer->reset();
	m_hole_timer->reset();
	m_motion_timer->reset();

	set_tape_present(false);
	set_tape_present(is_loaded());
}

image_init_result hp_dc100_tape_device::internal_load(bool is_create)
{
	LOG("load %d\n" , is_create);

	device_reset();

	io_generic io;
	io.file = (device_image_interface *)this;
	io.procs = &image_ioprocs;
	io.filler = 0;
	if (is_create) {
		m_image.clear_tape();
		m_image.save_tape(&io);
	} else if (!m_image.load_tape(&io)) {
		LOG("load failed\n");
		seterror(IMAGE_ERROR_INVALIDIMAGE , "Wrong format");
		set_tape_present(false);
		return image_init_result::FAIL;
	}
	LOG("load OK\n");

	m_image_dirty = false;

	set_tape_present(true);
	return image_init_result::PASS;
}

void hp_dc100_tape_device::set_tape_present(bool present)
{
	if (m_present != present) {
		m_present = present;
		m_cart_out_handler(!m_present);
	}
}

double hp_dc100_tape_device::compute_set_point(tape_speed_t speed , bool fwd) const
{
	double sp;

	if (speed == SP_SLOW) {
		sp = m_slow_set_point;
	} else if (speed == SP_FAST) {
		sp = m_fast_set_point;
	} else {
		sp = 0.0;
	}

	if (!fwd) {
		sp = -sp;
	}

	return sp;
}

void hp_dc100_tape_device::start_tape()
{
	LOG_DBG("Tape started %.6f p=%d\n" , machine().time().as_double() , m_tape_pos);
	m_start_time = machine().time();
	m_accelerating = true;
	m_speed = 0;
}

void hp_dc100_tape_device::stop_tape()
{
	LOG_DBG("Tape stops %.6f p=%d\n" , machine().time().as_double() , m_tape_pos);
	m_start_time = attotime::never;
	m_accelerating = false;
	m_speed = 0;
	m_tacho_timer->reset();
	m_hole_timer->reset();
	m_motion_timer->reset();
	stop_op();
}

double hp_dc100_tape_device::const_a_space(double a , double t) const
{
	// Space traveled in time 't' at constant acceleration 'a' starting with 'm_speed' speed
	return t * (m_speed + a / 2 * t);
}

attotime hp_dc100_tape_device::time_to_threshold(double threshold , bool zero_allowed) const
{
	attotime time{ attotime::never };

	auto delta_sp = m_set_point - m_speed;
	auto delta_t  = threshold - m_speed;
	LOG_DBG("Dsp=%.6f D+th=%.6f\n" , delta_sp , delta_t);

	if ((delta_sp * delta_t > 0.0 && fabs(delta_t) <= fabs(delta_sp)) ||
		(zero_allowed && delta_t == 0.0)) {
		time = attotime::from_double(fabs(delta_t) / m_acceleration);
		LOG_DBG("Time to +th: %.6f\n" , time.as_double());
	}

	delta_t = -threshold - m_speed;
	LOG_DBG("Dsp=%.6f D-th=%.6f\n" , delta_sp , delta_t);
	if ((delta_sp * delta_t > 0.0 && fabs(delta_t) <= fabs(delta_sp)) ||
		(zero_allowed && delta_t == 0.0)) {
		double tm = fabs(delta_t) / m_acceleration;
		if (tm < time.as_double()) {
			time = attotime::from_double(tm);
			LOG_DBG("Time to -th: %.6f\n" , time.as_double());
		}
	}
	return time;
}

void hp_dc100_tape_device::set_motion_timer()
{
	if (!m_accelerating) {
		m_motion_timer->reset();
	} else {
		// There are 4 possible future "motion" events:
		// 1. Slow speed threshold is crossed
		// 2. The "in motion" threshold is crossed
		// 3. Tape direction reverses
		// 4. Set point is reached
		// Motion timer is set to expire at the event that occurs first
		attotime time{ time_to_threshold(m_slow_set_point , true) };

		attotime tmp{ time_to_threshold(m_go_threshold , false) };
		if (tmp < time) {
			time = tmp;
		}

		// Time to the moment when tape inverts its motion
		// (i.e. when m_speed crosses 0)
		tmp = time_to_threshold(0.0 , false);
		if (tmp < time) {
			time = tmp;
		}

		// Time to reach set point
		tmp = attotime::from_double(fabs(m_speed - m_set_point) / m_acceleration);
		if (tmp < time) {
			time = tmp;
		}

		if (time.is_never()) {
			// Should never get here
			m_motion_timer->reset();
		} else {
			// Add margin
			time += attotime::from_double(MOTION_MARGIN);

			m_motion_timer->adjust(time);
		}
	}
}

void hp_dc100_tape_device::time_to_distance(hti_format_t::tape_pos_t distance , hti_format_t::tape_pos_t& target_pos , emu_timer *target_timer) const
{
	if (m_start_time.is_never()) {
		// If tape is stopped we'll never get there..
		target_timer->reset();
		return;
	}

	target_pos = m_tape_pos;
	if (!hti_format_t::pos_offset(target_pos , true , distance)) {
		// Beyond end of tape
		target_timer->reset();
		return;
	}

	double space = double(distance) / hti_format_t::ONE_INCH_POS;
	double time_const_a;

	if (m_accelerating) {
		// Time to reach constant V phase
		double time_to_const_v = fabs(m_set_point - m_speed) / m_acceleration;
		// Signed acceleration
		double acceleration = m_set_point > m_speed ? m_acceleration : -m_acceleration;
		// Compute time to cover distance with constant acceleration
		// It's the smallest non-negative root of this quadratic equation:
		// 1/2*acceleration*t^2+m_speed*t=space
		double delta = m_speed * m_speed + 2 * acceleration * space;
		bool has_root = delta >= 0.0;
		double time_in_const_a = 0.0;
		if (has_root) {
			double time_in_const_a_pos = (sqrt(delta) - m_speed) / acceleration;
			double time_in_const_a_neg = -(sqrt(delta) + m_speed) / acceleration;
			LOG_DBG("TTD %.6f %.6f\n" , time_in_const_a_pos , time_in_const_a_neg);
			if (time_in_const_a_pos >= 0.0) {
				if (time_in_const_a_neg >= 0.0) {
					// pos + neg +
					time_in_const_a = std::min(time_in_const_a_pos , time_in_const_a_neg);
				} else {
					// pos + neg -
					time_in_const_a = time_in_const_a_pos;
				}
			} else {
				if (time_in_const_a_neg >= 0.0) {
					// pos - neg +
					time_in_const_a = time_in_const_a_neg;
				} else {
					// pos - neg -
					has_root = false;
				}
			}
		}
		LOG_DBG("TTD %d %d %.6f %.6f %.6f\n" , distance , has_root , m_speed , time_to_const_v , time_in_const_a);
		if (has_root && time_in_const_a <= time_to_const_v) {
			// Entirely in the constant A phase
			time_const_a = time_in_const_a;
			space = 0.0;
		} else {
			// Partly in const A & partly in const V
			double space_in_const_a = const_a_space(acceleration , time_to_const_v);
			space -= space_in_const_a;
			time_const_a = time_to_const_v;
		}
	} else {
		// Entirely in const V
		time_const_a = 0.0;
	}

	// Time in constant V
	double time_const_v;
	if (space != 0.0) {
		if (m_set_point == 0.0) {
			target_timer->reset();
			return;
		} else {
			time_const_v = space / m_set_point;
			if (time_const_v < 0.0) {
				target_timer->reset();
				return;
			}
		}
	} else {
		time_const_v = 0.0;
	}
	LOG_DBG("TTD %.6f %.6f\n" , time_const_a , time_const_v);

	target_timer->adjust(attotime::from_double(time_const_a + time_const_v));
}

void hp_dc100_tape_device::adjust_tacho_timer()
{
	auto tick_fract = m_tape_pos % m_tick_size;
	hti_format_t::tape_pos_t dist_to_next;
	if (is_moving_fwd()) {
		// Distance to next tick in the fwd direction
		dist_to_next = m_tick_size - tick_fract;
	} else if (tick_fract) {
		// Distance to next tick in the rev direction when we're not exactly
		// on a tick (tick_fract != 0)
		dist_to_next = -tick_fract;
	} else {
		// Distance to next tick in the rev direction when we're exactly
		// on a tick (tick_fract == 0)
		dist_to_next = -m_tick_size;
	}
	LOG_DBG("Next tick @%d (pos=%d)\n" , dist_to_next , m_tape_pos);
	time_to_distance(dist_to_next, m_next_tacho_pos, m_tacho_timer);
}

void hp_dc100_tape_device::adjust_hole_timer()
{
	auto hole_pos = m_image.next_hole(m_tape_pos , is_moving_fwd());
	if (hole_pos == hti_format_t::NULL_TAPE_POS) {
		m_hole_timer->reset();
	} else {
		time_to_distance(hole_pos - m_tape_pos, m_next_hole_pos, m_hole_timer);
	}
}

void hp_dc100_tape_device::stop_op()
{
	if (m_current_op == OP_WRITE) {
		store_wr_word();
	} else if (m_current_op == OP_ERASE) {
		LOG_DBG("Wr gap from %d to %d\n" , m_rw_pos , m_tape_pos);
		m_image.write_gap(get_track_no() , m_rw_pos , m_tape_pos);
		m_image_dirty = true;
	}
	m_bit_timer->reset();
	m_current_op = OP_IDLE;
	m_gap_detect_start = hti_format_t::NULL_TAPE_POS;
}

void hp_dc100_tape_device::load_rd_word()
{
	if (m_rd_it_valid) {
		bool fwd = is_moving_fwd();
		if (fwd) {
			m_bit_idx = 15;
		} else {
			m_bit_idx = 0;
		}
		// This is actually the nearest end (dir is inverted)
		m_rw_pos = m_next_bit_pos = hti_format_t::farthest_end(m_rd_it , !fwd);
		// Compute end of bit cell
		hti_format_t::tape_pos_t bit_len = m_image.bit_length(BIT(m_rd_it->second , m_bit_idx));
		if (!fwd) {
			bit_len = -bit_len;
		}
		time_to_distance(m_next_bit_pos + bit_len - m_tape_pos , m_next_bit_pos , m_bit_timer);
		LOG_RW("RD %04x @%d\n" , m_rd_it->second , m_next_bit_pos);
	} else {
		LOG_RW("End of RD data @%d\n" , m_tape_pos);
		stop_op();
		m_gap_detect_start = m_tape_pos;
	}
}

void hp_dc100_tape_device::store_wr_word()
{
	bool fwd = is_moving_fwd();
	if ((fwd && m_bit_idx == 15) ||
		(!fwd && m_bit_idx == 0)) {
		return;
	}
	hti_format_t::tape_pos_t word_length = m_image.word_length(m_rw_word);
	if (!fwd) {
		m_rw_pos -= word_length;
	}
	LOG_RW("WR %04x @%d\n" , m_rw_word , m_rw_pos);
	m_image.write_word(get_track_no() , m_rw_pos , m_rw_word , word_length , fwd);
	m_image_dirty = true;
	m_rw_word = 0;
	if (fwd) {
		m_bit_idx = 15;
		m_rw_pos += word_length;
	} else {
		m_bit_idx = 0;
	}
}
