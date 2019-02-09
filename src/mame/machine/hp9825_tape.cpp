// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp9825_tape.cpp

    HP9825 tape sub-system

*********************************************************************/

#include "emu.h"
#include "hp9825_tape.h"
#include "machine/rescap.h"

// Debugging
#include "logmacro.h"
#define LOG_REG_MASK (LOG_GENERAL << 1)
#define LOG_REG(...) LOGMASKED(LOG_REG_MASK, __VA_ARGS__)
#define LOG_TMR_MASK (LOG_REG_MASK << 1)
#define LOG_TMR(...) LOGMASKED(LOG_TMR_MASK, __VA_ARGS__)
#define LOG_DBG_MASK (LOG_TMR_MASK << 1)
#define LOG_DBG(...) LOGMASKED(LOG_DBG_MASK, __VA_ARGS__)
#undef VERBOSE
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

// Constants
constexpr double FAST_SPEED = 90.0;             // Fast speed: 90 ips
constexpr double SLOW_SPEED = 22.0;             // Slow speed: 22 ips
constexpr double MIN_RD_SPEED = 22.0;           // Minimum speed to read data off the tape
constexpr double MOVING_THRESHOLD = 2.0;        // Tape is moving (from MVG bit POV) when speed > 2.0 ips
constexpr double ACCELERATION = 1200.0;         // Acceleration when speed set point is changed: 1200 ips^2
constexpr unsigned TACH_TICKS_PER_INCH = 483;   // Tachometer pulses per inch
constexpr double INVERSION_MARGIN = 1e-5;       // Margin to ensure speed is away from 0 when motion is inverted (10 µs)
constexpr hti_format_t::tape_pos_t TACH_TICK_LENGTH = hti_format_t::ONE_INCH_POS / TACH_TICKS_PER_INCH; // Length of each tach tick

// Bits in command register
enum : unsigned {
	CMD_REG_MOTOR_BIT   = 7,    // Motor on (0)
	CMD_REG_WR_GATE_BIT = 6,    // Write gate (0)
	CMD_REG_SPEED_BIT = 5,      // Tape speed (1 = slow)
	CMD_REG_DIR_BIT = 4,        // Tape direction (1 = fwd)
	CMD_REG_FLG_SEL_BIT = 3,    // FLG selection (0 = tacho pulses, 1 = bit clock)
	CMD_REG_THRESHOLD_BIT = 2,  // Threshold selection
	CMD_REG_DMA_EN_BIT = 1,     // DMA enable (0)
	CMD_REG_TRACK_SEL_BIT = 0   // Track selection (1 = A)
};

// Bits in status register
enum : unsigned {
	STAT_REG_WPR_BIT = 7,   // Write protected (1)
	STAT_REG_DIR_BIT = 6,   // Tape direction (1 = rev)
	STAT_REG_MVG_BIT = 5,   // Tape moving (1)
	STAT_REG_GAP_BIT = 4,   // Gap (1) or data (0)
	STAT_REG_COUT_BIT = 2,  // Cartridge out (1)
	STAT_REG_SVF_BIT = 1,   // Servo failure (1)
	STAT_REG_EOT_BIT = 0    // End of tape (1)
};

// Timers
enum {
	BIT_TMR_ID,
	TACHO_TMR_ID,
	HOLE_TMR_ID,
	INV_TMR_ID
};

// Device type definition
DEFINE_DEVICE_TYPE(HP9825_TAPE, hp9825_tape_device, "hp9825_tape", "HP9825 tape sub-system")

hp9825_tape_device::hp9825_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig , HP9825_TAPE , tag , owner , clock)
	, device_image_interface(mconfig , *this)
	, m_flg_handler(*this)
	, m_sts_handler(*this)
	, m_dmar_handler(*this)
	, m_led_handler(*this)
	, m_short_gap_timer(*this , "short_tmr")
	, m_long_gap_timer(*this , "long_tmr")
	, m_image()
	, m_image_dirty(false)
{
}

void hp9825_tape_device::device_add_mconfig(machine_config &config)
{
	TTL74123(config , m_short_gap_timer , 0);
	m_short_gap_timer->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_short_gap_timer->set_resistor_value(RES_K(37.9));
	m_short_gap_timer->set_capacitor_value(CAP_N(10));
	m_short_gap_timer->set_a_pin_value(0);
	m_short_gap_timer->set_clear_pin_value(1);
	m_short_gap_timer->out_cb().set(FUNC(hp9825_tape_device::short_gap_w));

	TTL74123(config , m_long_gap_timer , 0);
	m_long_gap_timer->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_long_gap_timer->set_resistor_value(RES_K(28.7));
	m_long_gap_timer->set_capacitor_value(CAP_U(0.22));
	m_long_gap_timer->set_clear_pin_value(1);
	m_long_gap_timer->out_cb().set(FUNC(hp9825_tape_device::long_gap_w));
}

void hp9825_tape_device::device_start()
{
	m_flg_handler.resolve_safe();
	m_sts_handler.resolve_safe();
	m_dmar_handler.resolve_safe();
	m_led_handler.resolve_safe();

	m_bit_timer = timer_alloc(BIT_TMR_ID);
	m_tacho_timer = timer_alloc(TACHO_TMR_ID);
	m_hole_timer = timer_alloc(HOLE_TMR_ID);
	m_inv_timer = timer_alloc(INV_TMR_ID);

	save_item(NAME(m_cmd_reg));
	save_item(NAME(m_stat_reg));
	save_item(NAME(m_flg));
	save_item(NAME(m_sts));
	save_item(NAME(m_data_out));
	save_item(NAME(m_data_in));
	save_item(NAME(m_exception));
	save_item(NAME(m_search_complete));
	save_item(NAME(m_dma_req));
	save_item(NAME(m_in_gap));
	save_item(NAME(m_no_go));
	save_item(NAME(m_present));
	save_item(NAME(m_valid_bits));
	save_item(NAME(m_trans_cnt));
	save_item(NAME(m_short_gap_out));
	save_item(NAME(m_long_gap_out));
	save_item(NAME(m_image_dirty));
	save_item(NAME(m_tape_pos));
	save_item(NAME(m_next_bit_pos));
	save_item(NAME(m_next_tacho_pos));
	save_item(NAME(m_next_hole_pos));
	save_item(NAME(m_speed));
	save_item(NAME(m_start_time));
	save_item(NAME(m_accelerating));
	save_item(NAME(m_rw_stat));
	save_item(NAME(m_rd_it_valid));
	save_item(NAME(m_rw_word));
	save_item(NAME(m_bit_idx));
	save_item(NAME(m_gap_start));
}

void hp9825_tape_device::device_reset()
{
	clear_state();
}

void hp9825_tape_device::clear_state()
{
	m_cmd_reg = ~0;
	m_stat_reg = 0;
	// Not actually reset in real hw
	m_flg = false;
	m_sts = true;
	m_data_out = false;
	m_data_in = false;
	m_exception = false;
	m_search_complete = false;
	m_dma_req = false;
	m_in_gap = true;
	m_no_go = false;
	m_valid_bits = false;
	m_trans_cnt = 0;
	m_tape_pos = 80 * hti_format_t::ONE_INCH_POS;
	m_speed = 0.0;
	m_start_time = attotime::never;
	m_accelerating = false;
	m_rw_stat = RW_IDLE;
	m_rd_it_valid = false;
	m_gap_start = hti_format_t::NULL_TAPE_POS;

	m_short_gap_timer->b_w(0);
	m_long_gap_timer->a_w(1);
	m_long_gap_timer->b_w(0);

	m_flg_handler(false);
	m_sts_handler(true);
	m_dmar_handler(false);
	m_led_handler(false);

	m_bit_timer->reset();
	m_tacho_timer->reset();
	m_hole_timer->reset();
	m_inv_timer->reset();

	set_tape_present(false);
	set_tape_present(is_loaded());
}

void hp9825_tape_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG_TMR("%.6f TMR %d s=%.3f p=%d a=%d\n" , machine().time().as_double() , id , m_speed , m_tape_pos , m_accelerating);
	update_speed_pos();

	switch (id) {
	case BIT_TMR_ID:
		m_tape_pos = m_next_bit_pos;
		if (m_rw_stat == RW_READING) {
			// Reading
			// Tape pos here is aligned with the beginning of bit cell. It'd be more correct
			// to align with end of cell, though. This solution is a lot simpler and
			// it's basically harmless.

			// 17th bit is sync (always 1)
			bool bit = m_bit_idx == 16 ? true : BIT(m_rd_it->second , 15 - m_bit_idx);
			rd_bit(bit);
			if (is_moving_fwd()) {
				m_bit_idx++;
				if (m_bit_idx >= 17) {
					m_rd_it_valid = m_image.adv_it(current_track() , true , m_rd_it) != hti_format_t::ADV_NO_MORE_DATA;
					load_rd_word();
				} else {
					time_to_distance(bit_size(bit), m_next_bit_pos, m_bit_timer);
				}
			} else {
				if (m_bit_idx > 0) {
					m_bit_idx--;
					time_to_distance(-bit_size(bit), m_next_bit_pos, m_bit_timer);
				} else {
					m_rd_it_valid = m_image.adv_it(current_track() , false , m_rd_it) != hti_format_t::ADV_NO_MORE_DATA;
					load_rd_word();
				}
			}
		} else if (m_rw_stat == RW_WRITING) {
			// Writing
			// Tape pos is aligned with beginning of bit cell
			bool bit = m_data_in;
			if (BIT(m_cmd_reg , CMD_REG_FLG_SEL_BIT)) {
				set_flg(true);
			}
			wr_bit(bit);
			time_to_distance(bit_size(bit), m_next_bit_pos, m_bit_timer);
		}
		break;

	case TACHO_TMR_ID:
		m_tape_pos = m_next_tacho_pos;
		if (!BIT(m_cmd_reg , CMD_REG_FLG_SEL_BIT)) {
			set_flg(true);
		}
		adjust_tacho_timer();
		break;

	case HOLE_TMR_ID:
		m_tape_pos = m_next_hole_pos;
		BIT_SET(m_stat_reg , STAT_REG_EOT_BIT);
		update_sts();
		adjust_hole_timer();
		break;

	case INV_TMR_ID:
		// In itself it does nothing (all work is in update_speed_pos)
		break;

	default:
		break;
	}
	LOG_TMR("%.6f TMR %d s=%.3f p=%d a=%d\n" , machine().time().as_double() , id , m_speed , m_tape_pos , m_accelerating);
}

image_init_result hp9825_tape_device::internal_load(bool is_create)
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

image_init_result hp9825_tape_device::call_load()
{
	return internal_load(false);
}

image_init_result hp9825_tape_device::call_create(int format_type, util::option_resolution *format_options)
{
	return internal_load(true);
}

void hp9825_tape_device::call_unload()
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

std::string hp9825_tape_device::call_display()
{
	// TODO:
	return std::string();
}

const char *hp9825_tape_device::file_extensions() const
{
	return "hti";
}

READ16_MEMBER(hp9825_tape_device::tape_r)
{
	uint16_t res = 0;

	switch (offset) {
	case 0:
		// R4: read data out
		if (m_data_out) {
			BIT_SET(res , 0);
		}
		if (!m_no_go) {
			set_flg(false);
		}
		break;

	case 1:
		// R5: read status
		res = m_stat_reg;
		break;

	case 2:
		// R6: clear EOT
		BIT_CLR(m_stat_reg , STAT_REG_EOT_BIT);
		update_sts();
		break;

	default:
		logerror("Reading @ offset %u\n" , offset);
		break;
	}

	LOG_REG("R R%u=%02x\n" , offset + 4 , res);
	return res;
}

WRITE16_MEMBER(hp9825_tape_device::tape_w)
{
	LOG_REG("W R%u=%02x\n" , offset + 4 , data);

	switch (offset) {
	case 0:
		// R4: write data in
		m_data_in = BIT(data , 0);
		m_dma_req = false;
		update_dmar();
		if (!m_no_go) {
			set_flg(false);
		}
		break;

	case 1:
		// R5: write command
		{
			double old_set_point = get_speed_set_point();
			auto old_cmd_reg = m_cmd_reg;
			m_cmd_reg = data;
			check_for_speed_change(old_set_point);
			// Direction bit is mirrored (inverted) in status register
			if (BIT(m_cmd_reg , CMD_REG_DIR_BIT)) {
				BIT_CLR(m_stat_reg , STAT_REG_DIR_BIT);
			} else {
				BIT_SET(m_stat_reg , STAT_REG_DIR_BIT);
			}
			if (BIT(m_cmd_reg , CMD_REG_DMA_EN_BIT)) {
				// DMA disabled
				m_search_complete = false;
			}
			update_sts();
			update_dmar();

			if ((old_cmd_reg ^ m_cmd_reg) &
				(BIT_MASK<uint8_t>(CMD_REG_WR_GATE_BIT) | BIT_MASK<uint8_t>(CMD_REG_THRESHOLD_BIT))) {
				// Something changed in Wr gate or threshold bit, start rd/wr
				update_speed_pos();
				start_rd_wr();
			}
		}
		break;

	case 2:
		// R6: clear DMA
		if (!BIT(m_cmd_reg , CMD_REG_DMA_EN_BIT)) {
			m_search_complete = true;
			update_sts();
		}
		m_dma_req = false;
		update_dmar();
		break;

	case 3:
		// R7: reset status
		if (m_present) {
			BIT_CLR(m_stat_reg , STAT_REG_COUT_BIT);
			update_sts();
		}
		break;

	default:
		logerror("Writing @ offset %u\n" , offset);
		break;
	}
}

WRITE_LINE_MEMBER(hp9825_tape_device::short_gap_w)
{
	LOG_DBG("Short gap %d\n" , state);
	m_short_gap_out = state;
	if (!m_short_gap_out) {
		m_valid_bits = false;
		m_data_out = false;
		m_trans_cnt = 0;
		m_in_gap = true;
		m_long_gap_timer->a_w(m_in_gap);
		update_sts();
	}
}

WRITE_LINE_MEMBER(hp9825_tape_device::long_gap_w)
{
	LOG_DBG("Long gap %d\n" , state);
	if (m_long_gap_out && !state && !BIT(m_cmd_reg , CMD_REG_DMA_EN_BIT)) {
		m_dma_req = true;
		update_dmar();
	}
	m_long_gap_out = state;
	if (m_long_gap_out) {
		BIT_CLR(m_stat_reg , STAT_REG_GAP_BIT);
	} else {
		BIT_SET(m_stat_reg , STAT_REG_GAP_BIT);
	}
}

void hp9825_tape_device::set_flg(bool state)
{
	if (state != m_flg) {
		m_flg = state;
		m_flg_handler(m_flg);
	}
}

void hp9825_tape_device::update_sts()
{
	// Inputs to STS computation:
	// CMD_REG_MOTOR_BIT
	// CMD_REG_DMA_EN_BIT
	// STAT_REG_EOT_BIT
	// STAT_REG_COUT_BIT
	// m_search_complete
	// m_in_gap
	auto prev_set_point = get_speed_set_point();
	auto prev_exception = m_exception;

	m_exception =
		BIT(m_stat_reg , STAT_REG_EOT_BIT) ||
		BIT(m_stat_reg , STAT_REG_COUT_BIT) ||
		m_search_complete;

	if (prev_exception != m_exception) {
		check_for_speed_change(prev_set_point);
	}

	m_no_go = m_exception && !BIT(m_cmd_reg , CMD_REG_MOTOR_BIT);
	// U6-6
	bool sts_2 = m_in_gap && BIT(m_cmd_reg , CMD_REG_DMA_EN_BIT);

	bool new_sts = m_no_go || sts_2;
	if (new_sts != m_sts) {
		m_sts = new_sts;
		m_sts_handler(m_sts);
	}

	if (m_no_go) {
		set_flg(true);
	}
}

void hp9825_tape_device::update_dmar()
{
	m_dmar_handler(m_dma_req && !BIT(m_cmd_reg , CMD_REG_DMA_EN_BIT));
}

void hp9825_tape_device::set_tape_present(bool present)
{
	m_present = present;
	if (present) {
		if (is_readonly()) {
			BIT_SET(m_stat_reg, STAT_REG_WPR_BIT);
		} else {
			BIT_CLR(m_stat_reg, STAT_REG_WPR_BIT);
		}
		// STAT_REG_COUT_BIT is cleared by a write to R7
	} else {
		BIT_SET(m_stat_reg, STAT_REG_COUT_BIT);
		BIT_SET(m_stat_reg, STAT_REG_WPR_BIT);
		update_sts();
	}
}

bool hp9825_tape_device::is_moving_fwd() const
{
	return BIT(m_cmd_reg , CMD_REG_DIR_BIT);
}

bool hp9825_tape_device::is_speed_fast() const
{
	return !BIT(m_cmd_reg , CMD_REG_SPEED_BIT);
}

bool hp9825_tape_device::is_actual_dir_fwd() const
{
	// Actual direction can be different from commanded direction in accelerated phases (e.g. when tape
	// direction is turned around)
	return m_speed == 0.0 ? is_moving_fwd() : m_speed > 0.0;
}

double hp9825_tape_device::get_speed_set_point() const
{
	if (m_exception || BIT(m_cmd_reg , CMD_REG_MOTOR_BIT)) {
		// Stop
		return 0.0;
	} else {
		double c;
		if (is_speed_fast()) {
			// HS
			c = FAST_SPEED;
		} else {
			// LS
			c = SLOW_SPEED;
		}
		if (!is_moving_fwd()) {
			// Reverse
			c = -c;
		}
		return c;
	}
}

void hp9825_tape_device::start_tape()
{
	LOG_DBG("Tape started %.6f p=%d\n" , machine().time().as_double() , m_tape_pos);
	m_start_time = machine().time();
	m_accelerating = true;
	m_speed = 0;
}

void hp9825_tape_device::stop_tape()
{
	LOG_DBG("Tape stops %.6f p=%d\n" , machine().time().as_double() , m_tape_pos);
	m_start_time = attotime::never;
	m_accelerating = false;
	m_speed = 0;
	m_tacho_timer->reset();
	m_hole_timer->reset();
	m_inv_timer->reset();
	stop_rd_wr();
}

void hp9825_tape_device::check_for_speed_change(double prev_set_point)
{
	double set_point = get_speed_set_point();
	if (prev_set_point != set_point) {
		update_speed_pos();
		LOG_DBG("Speed SP changed %f->%f %.6f p=%d\n" , prev_set_point , set_point , machine().time().as_double() , m_tape_pos);
		// Speed set point changed, accelerate/decelerate
		m_accelerating = true;
		if (m_start_time.is_never()) {
			// Tape starting now
			start_tape();
		}
		start_rd_wr(true);
		adjust_tacho_timer();
		adjust_hole_timer();
		set_inv_timer();
	}
}

void hp9825_tape_device::update_speed_pos()
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
	// Do R/W start/stop
	bool rw_start_stop = false;

	if (m_accelerating) {
		double set_point = get_speed_set_point();
		double time_to_const_v = fabs(set_point - m_speed) / ACCELERATION;
		double acceleration = set_point > m_speed ? ACCELERATION : -ACCELERATION;
		if (delta_time_double < time_to_const_v) {
			space_const_a = const_a_space(acceleration , delta_time_double);
			auto prev_speed = m_speed;
			m_speed += delta_time_double * acceleration;
			rw_start_stop = (fabs(prev_speed) >= MIN_RD_SPEED) != (fabs(m_speed) >= MIN_RD_SPEED);
			if (prev_speed * m_speed < 0.0) {
				// Direction inverted (speed sign flipped)
				LOG_DBG("Dir inverted s=%f\n" , m_speed);
				adjust_tacho_timer();
				adjust_hole_timer();
			}
			set_inv_timer();
			time_const_v = 0.0;
		} else {
			space_const_a = const_a_space(acceleration , time_to_const_v);
			time_const_v = delta_time_double - time_to_const_v;
			LOG_DBG("Acceleration ends\n");
			m_accelerating = false;
			m_speed = set_point;
			m_inv_timer->reset();
			if (m_speed == 0.0) {
				// Tape stops
				stop_tape();
			} else {
				rw_start_stop = true;
			}
		}
	} else {
		space_const_a = 0.0;
		time_const_v = delta_time_double;
	}

	// Update MVG bit
	if (fabs(m_speed) >= MOVING_THRESHOLD) {
		if (!BIT(m_stat_reg , STAT_REG_MVG_BIT)) {
			BIT_SET(m_stat_reg , STAT_REG_MVG_BIT);
			m_led_handler(true);
		}
	} else {
		if (BIT(m_stat_reg , STAT_REG_MVG_BIT)) {
			BIT_CLR(m_stat_reg , STAT_REG_MVG_BIT);
			m_led_handler(false);
		}
	}

	hti_format_t::tape_pos_t delta_pos = (hti_format_t::tape_pos_t)((space_const_a + m_speed * time_const_v) * hti_format_t::ONE_INCH_POS);
	LOG_DBG("dp=%d\n" , delta_pos);
	if (!hti_format_t::pos_offset(m_tape_pos , true , delta_pos)) {
		LOG("Tape unspooled!\n");
	}

	if (rw_start_stop) {
		start_rd_wr();
	}
}

void hp9825_tape_device::set_inv_timer()
{
	// Set inversion timer to go off at the moment the tape inverts its motion
	// (i.e. when m_speed crosses 0)
	if (m_speed == 0.0 || (get_speed_set_point() * m_speed) > 0.0) {
		LOG_DBG("Inv tmr reset\n");
		// No inversion in sight
		m_inv_timer->reset();
	} else {
		// INVERSION_MARGIN is added to ensure that m_speed has already cleared the
		// 0-crossing when inv timer goes off
		LOG_DBG("Inv tmr set %.6f %f\n" , machine().time().as_double() , m_speed);
		m_inv_timer->adjust(attotime::from_double(fabs(m_speed) / ACCELERATION + INVERSION_MARGIN));
	}
}

void hp9825_tape_device::time_to_distance(hti_format_t::tape_pos_t distance , hti_format_t::tape_pos_t& target_pos , emu_timer *target_timer) const
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
	double set_point = get_speed_set_point();
	double time_const_a;

	if (m_accelerating) {
		// Time to reach constant V phase
		double time_to_const_v = fabs(set_point - m_speed) / ACCELERATION;
		// Signed acceleration
		double acceleration = set_point > m_speed ? ACCELERATION : -ACCELERATION;
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
		if (set_point == 0.0) {
			target_timer->reset();
			return;
		} else {
			time_const_v = space / set_point;
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

double hp9825_tape_device::const_a_space(double a , double t) const
{
	// Space traveled in time 't' at constant acceleration 'a' starting with 'm_speed' speed
	return t * (m_speed + a / 2 * t);
}

hti_format_t::tape_pos_t hp9825_tape_device::get_next_hole() const
{
	return hti_format_t::next_hole(m_tape_pos , is_actual_dir_fwd()) - m_tape_pos;
}

void hp9825_tape_device::adjust_tacho_timer()
{
	hti_format_t::tape_pos_t tick = TACH_TICK_LENGTH;
	if (!is_actual_dir_fwd()) {
		tick = -tick;
	}
	time_to_distance(tick , m_next_tacho_pos , m_tacho_timer);
}

void hp9825_tape_device::adjust_hole_timer()
{
	time_to_distance(get_next_hole() , m_next_hole_pos , m_hole_timer);
}

unsigned hp9825_tape_device::current_track() const
{
	return !BIT(m_cmd_reg , CMD_REG_TRACK_SEL_BIT);
}

constexpr hti_format_t::tape_pos_t hp9825_tape_device::bit_size(bool bit)
{
	return bit ? hti_format_t::ONE_BIT_LEN : hti_format_t::ZERO_BIT_LEN;
}

void hp9825_tape_device::start_rd_wr(bool recalc)
{
	if (fabs(m_speed) >= MIN_RD_SPEED && BIT(m_cmd_reg , CMD_REG_WR_GATE_BIT)) {
		// Reading
		if (m_rw_stat != RW_READING || recalc) {
			stop_rd_wr();
			LOG_DBG("Start RD @%d s=%f t=%u\n" , m_tape_pos , m_speed , current_track());
			m_rd_it_valid = m_image.next_data(current_track() , m_tape_pos , is_actual_dir_fwd() , false , m_rd_it);
			m_rw_stat = RW_READING;
			load_rd_word();
		}
	} else if (!m_accelerating && !BIT(m_cmd_reg , CMD_REG_WR_GATE_BIT) && BIT(m_cmd_reg , CMD_REG_THRESHOLD_BIT)) {
		// Data writing
		if (m_rw_stat != RW_WRITING) {
			stop_rd_wr();
			// Start WR, only LS FWD is allowed
			if (m_speed == SLOW_SPEED) {
				LOG_DBG("Start WR @%d\n" , m_tape_pos);
				// Looking for sync
				m_bit_idx = 17;
				m_rw_word = 0;
				time_to_distance(bit_size(true) , m_next_bit_pos , m_bit_timer);
				m_rw_stat = RW_WRITING;
			} else {
				LOG("Starting WR s=%f ???\n" , m_speed);
			}
		}
	} else if (!BIT(m_cmd_reg , CMD_REG_WR_GATE_BIT) && !BIT(m_cmd_reg , CMD_REG_THRESHOLD_BIT)) {
		// Gap writing
		if (m_rw_stat != RW_WRITING_GAP) {
			stop_rd_wr();
			LOG_DBG("Start GAP @%d\n" , m_tape_pos);
			m_gap_start = m_tape_pos;
			m_rw_stat = RW_WRITING_GAP;
		}
	} else {
		stop_rd_wr();
	}
}

void hp9825_tape_device::load_rd_word()
{
	if (m_rd_it_valid) {
		if (is_moving_fwd()) {
			m_bit_idx = 0;
		} else {
			m_bit_idx = 16;
		}
		// This is actually the nearest end (dir is inverted)
		m_next_bit_pos = hti_format_t::farthest_end(m_rd_it , !is_actual_dir_fwd());
		LOG_DBG("Valid np=%d\n" , m_next_bit_pos);
		time_to_distance(m_next_bit_pos - m_tape_pos , m_next_bit_pos , m_bit_timer);
	} else {
		LOG_DBG("Invalid\n");
		stop_rd_wr();
	}
}

void hp9825_tape_device::rd_bit(bool bit)
{
	m_short_gap_timer->b_w(1);
	m_short_gap_timer->b_w(0);
	m_long_gap_timer->b_w(1);
	m_long_gap_timer->b_w(0);
	m_data_out = m_valid_bits && bit;
	if (BIT(m_cmd_reg , CMD_REG_FLG_SEL_BIT)) {
		set_flg(true);
	}
	m_trans_cnt++;
	LOG_DBG("TC %u IG %d VB %d\n" , m_trans_cnt , m_in_gap , m_valid_bits);
	if ((m_trans_cnt & 0x0c) == 0x0c) {
		m_valid_bits = true;
	}
	if (BIT(m_trans_cnt , 2) && m_in_gap) {
		m_in_gap = false;
		m_long_gap_timer->a_w(m_in_gap);
		update_sts();
	}
}

void hp9825_tape_device::wr_bit(bool bit)
{
	LOG_DBG("WR %d idx=%u\n" , bit , m_bit_idx);
	m_rw_word <<= 1;
	m_rw_word |= bit;

	// A bit of heuristic here to achieve synchronization (i.e. find word boundary in bit stream)
	if (m_bit_idx >= 17 && m_bit_idx < (17 + 16)) {
		m_bit_idx++;
	} else if (m_bit_idx == (17 + 16) && (m_rw_word & 3) == 3) {
		m_bit_idx = 16;
	}
	if (m_bit_idx < 16) {
		m_bit_idx++;
	} else if (m_bit_idx == 16) {
		// Write word
		if (!BIT(m_rw_word , 0)) {
			LOG_DBG("Sync lost! w=%05x\n" , m_rw_word & 0x1ffff);
		} else {
			hti_format_t::tape_word_t w = (hti_format_t::tape_word_t)(m_rw_word >> 1);
			hti_format_t::tape_pos_t wr_pos = m_tape_pos - hti_format_t::word_length(w) + hti_format_t::ONE_BIT_LEN;
			LOG_DBG("Wr word %04x @%d\n" , w , wr_pos);
			m_image.write_word(current_track() , wr_pos , w , wr_pos);
			m_image_dirty = true;
			m_bit_idx = 0;
		}
	}
}

void hp9825_tape_device::stop_rd_wr()
{
	if (m_rw_stat == RW_WRITING_GAP) {
		LOG_DBG("Wr gap from %d to %d\n" , m_gap_start , m_tape_pos);
		m_image.write_gap(current_track() , m_gap_start , m_tape_pos);
		m_image_dirty = true;
	}
	m_bit_timer->reset();
	m_rw_stat = RW_IDLE;
}

