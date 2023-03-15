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
#define LOG_DBG_MASK (LOG_REG_MASK << 1)
#define LOG_DBG(...) LOGMASKED(LOG_DBG_MASK, __VA_ARGS__)
#undef VERBOSE
//#define VERBOSE (LOG_GENERAL | LOG_REG_MASK | LOG_DBG_MASK)
//#define VERBOSE (LOG_GENERAL)
#define VERBOSE (LOG_GENERAL | LOG_DBG_MASK)

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
constexpr double MOVING_THRESHOLD = 2.0;        // Tape is moving (from MVG bit POV) when speed > 2.0 ips
constexpr double ACCELERATION = 1200.0;         // Acceleration when speed set point is changed: 1200 ips^2
constexpr unsigned TACH_TICKS_PER_INCH = 483;   // Tachometer pulses per inch
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

// Device type definition
DEFINE_DEVICE_TYPE(HP9825_TAPE, hp9825_tape_device, "hp9825_tape", "HP9825 tape sub-system")

hp9825_tape_device::hp9825_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig , HP9825_TAPE , tag , owner , clock)
	, m_flg_handler(*this)
	, m_sts_handler(*this)
	, m_dmar_handler(*this)
	, m_led_handler(*this)
	, m_cart_in_handler(*this)
	, m_tape(*this , "drive")
	, m_short_gap_timer(*this , "short_tmr")
	, m_long_gap_timer(*this , "long_tmr")
{
}

void hp9825_tape_device::device_add_mconfig(machine_config &config)
{
	HP_DC100_TAPE(config , m_tape , 0);
	m_tape->set_acceleration(ACCELERATION);
	m_tape->set_set_points(SLOW_SPEED , FAST_SPEED);
	m_tape->set_tick_size(TACH_TICK_LENGTH);
	m_tape->set_image_format(hti_format_t::HTI_DELTA_MOD_17_BITS);
	m_tape->set_go_threshold(MOVING_THRESHOLD);
	m_tape->cart_out().set(FUNC(hp9825_tape_device::cart_out_w));
	m_tape->hole().set(FUNC(hp9825_tape_device::hole_w));
	m_tape->tacho_tick().set(FUNC(hp9825_tape_device::tacho_tick_w));
	m_tape->motion_event().set(FUNC(hp9825_tape_device::motion_w));
	m_tape->rd_bit().set(FUNC(hp9825_tape_device::rd_bit_w));
	m_tape->wr_bit().set(FUNC(hp9825_tape_device::wr_bit_r));

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
	LOG_DBG("start\n");
	m_flg_handler.resolve_safe();
	m_sts_handler.resolve_safe();
	m_dmar_handler.resolve_safe();
	m_led_handler.resolve_safe();
	m_cart_in_handler.resolve_safe();

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
	save_item(NAME(m_valid_bits));
	save_item(NAME(m_trans_cnt));
	save_item(NAME(m_short_gap_out));
	save_item(NAME(m_long_gap_out));
}

void hp9825_tape_device::device_reset()
{
	LOG_DBG("reset\n");
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

	m_short_gap_timer->b_w(0);
	m_long_gap_timer->a_w(1);
	m_long_gap_timer->b_w(0);

	m_flg_handler(false);
	m_sts_handler(true);
	m_dmar_handler(false);
	m_led_handler(false);
}

uint16_t hp9825_tape_device::tape_r(offs_t offset)
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

void hp9825_tape_device::tape_w(offs_t offset, uint16_t data)
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
			auto old_cmd_reg = m_cmd_reg;
			m_cmd_reg = data;
			check_for_speed_change();
			// Direction bit is mirrored (inverted) in status register
			if (BIT(m_cmd_reg , CMD_REG_DIR_BIT)) {
				BIT_CLR(m_stat_reg , STAT_REG_DIR_BIT);
			} else {
				BIT_SET(m_stat_reg , STAT_REG_DIR_BIT);
			}
			m_tape->set_track_no(!BIT(m_cmd_reg , CMD_REG_TRACK_SEL_BIT));
			if (BIT(m_cmd_reg , CMD_REG_DMA_EN_BIT)) {
				// DMA disabled
				m_search_complete = false;
			}
			update_sts();
			update_dmar();

			if ((old_cmd_reg ^ m_cmd_reg) &
				(BIT_MASK<uint8_t>(CMD_REG_WR_GATE_BIT) | BIT_MASK<uint8_t>(CMD_REG_THRESHOLD_BIT))) {
				// Something changed in Wr gate or threshold bit, start rd/wr
				m_tape->update_speed_pos();
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
		if (!m_tape->cart_out_r()) {
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

WRITE_LINE_MEMBER(hp9825_tape_device::cart_out_w)
{
	LOG_DBG("cart_out_w %d\n" , state);
	if (state) {
		// STAT_REG_COUT_BIT is cleared by a write to R7
		BIT_SET(m_stat_reg, STAT_REG_COUT_BIT);
	}

	if (m_tape->wpr_r()) {
		BIT_SET(m_stat_reg, STAT_REG_WPR_BIT);
	} else {
		BIT_CLR(m_stat_reg, STAT_REG_WPR_BIT);
	}

	m_cart_in_handler(!state);

	update_sts();
}

WRITE_LINE_MEMBER(hp9825_tape_device::hole_w)
{
	if (state) {
		LOG_DBG("hole_w\n");
		BIT_SET(m_stat_reg , STAT_REG_EOT_BIT);
		update_sts();
	}
}

WRITE_LINE_MEMBER(hp9825_tape_device::tacho_tick_w)
{
	if (state) {
		LOG_DBG("tacho_tick_w\n");
		if (!BIT(m_cmd_reg , CMD_REG_FLG_SEL_BIT)) {
			set_flg(true);
		}
	}
}

WRITE_LINE_MEMBER(hp9825_tape_device::motion_w)
{
	if (state) {
		LOG_DBG("motion_w\n");
		// Update MVG bit
		if (m_tape->is_moving()) {
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
		start_rd_wr();
	}
}

WRITE_LINE_MEMBER(hp9825_tape_device::rd_bit_w)
{
	m_short_gap_timer->b_w(1);
	m_short_gap_timer->b_w(0);
	m_long_gap_timer->b_w(1);
	m_long_gap_timer->b_w(0);
	m_data_out = m_valid_bits && state;
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

READ_LINE_MEMBER(hp9825_tape_device::wr_bit_r)
{
	if (BIT(m_cmd_reg , CMD_REG_FLG_SEL_BIT)) {
		set_flg(true);
	}
	return m_data_in;
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
	auto prev_exception = m_exception;

	m_exception =
		BIT(m_stat_reg , STAT_REG_EOT_BIT) ||
		BIT(m_stat_reg , STAT_REG_COUT_BIT) ||
		m_search_complete;

	if (prev_exception != m_exception) {
		check_for_speed_change();
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

bool hp9825_tape_device::is_moving_fwd() const
{
	return BIT(m_cmd_reg , CMD_REG_DIR_BIT);
}

bool hp9825_tape_device::is_speed_fast() const
{
	return !BIT(m_cmd_reg , CMD_REG_SPEED_BIT);
}

void hp9825_tape_device::check_for_speed_change()
{
	hp_dc100_tape_device::tape_speed_t new_speed;

	if (m_exception || BIT(m_cmd_reg , CMD_REG_MOTOR_BIT)) {
		// Stop
		new_speed = hp_dc100_tape_device::SP_STOP;
	} else {
		new_speed = is_speed_fast() ? hp_dc100_tape_device::SP_FAST :
			hp_dc100_tape_device::SP_SLOW;
	}

	bool changed = m_tape->set_speed_setpoint(new_speed , is_moving_fwd());

	if (changed) {
		start_rd_wr(true);
	}
}

void hp9825_tape_device::start_rd_wr(bool recalc)
{
	if (m_tape->is_above_threshold() && BIT(m_cmd_reg , CMD_REG_WR_GATE_BIT)) {
		// Reading
		m_tape->set_op(hp_dc100_tape_device::OP_READ , recalc);
	} else if (!m_tape->is_accelerating() && !BIT(m_cmd_reg , CMD_REG_WR_GATE_BIT) && BIT(m_cmd_reg , CMD_REG_THRESHOLD_BIT)) {
		// Data writing
		m_tape->set_op(hp_dc100_tape_device::OP_WRITE);
	} else if (!BIT(m_cmd_reg , CMD_REG_WR_GATE_BIT) && !BIT(m_cmd_reg , CMD_REG_THRESHOLD_BIT)) {
		// Gap writing
		m_tape->set_op(hp_dc100_tape_device::OP_ERASE);
	} else {
		m_tape->set_op(hp_dc100_tape_device::OP_IDLE);
	}
}
