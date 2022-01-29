// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp2640_tape.cpp

    Tape subsystem of HP264x terminals

    Tape subsystem is composed of two DC100 drives and two cards:
    02640-60137 CTU interface
    02640-60032 Read/write PCA

    Data is recorded on a single track with Manchester modulation.
    This modulation produces a maximum flux transition
    density of 1600 transitions per inch. Bytes are recorded from
    LSB to MSB. A "0" bit is recorded as 10, a "1" as 01.
    HP264x tapes are not compatible with HP9825, HP85 & HP9845 systems
    despite using the same DC100 cartridges. The latter systems record
    data with a different modulation (delta-distance) and use two tracks.

    Format of records on tape:
    0..3    Preamble = 00 00 00 80
    4..5    Length of record, MSB first
    6..x    Record data
    x+1     Checksum
    x+2..x+5 Postamble = 01 00 00 00 (which is just the preamble in
            reverse)

    Most significant bit of record length is set to 1 when starting
    a file header. In this case there's just one byte of data carrying
    the file number.

    Gaps of about 0.85" separate data records. A file header is
    surrounded by two gaps of about 1.7" length. End of data on tape
    is marked by 11" of gap.

    Reference docs:

    13255-91032, Cartridge tape module
    13255-91137, Extended CTU interface module

*********************************************************************/

#include "emu.h"
#include "hp2640_tape.h"

// Debugging
#include "logmacro.h"
#undef VERBOSE
#define VERBOSE 0
//#define VERBOSE (LOG_GENERAL)

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

// Timers
enum {
	GAP_TMR_ID,
	CELL_TMR_ID
};

// Constants
constexpr double FAST_SPEED = 60.0;             // Fast speed: 60 ips
constexpr double SLOW_SPEED = 10.0;             // Slow speed: 10 ips
constexpr double MOVING_THRESHOLD = 1.0;        // Tape is moving when speed > 1.0 ips
constexpr double ACCELERATION = 2000.0;         // Acceleration when speed set point is changed: 2000 ips^2
// 58.4 tachometer pulses per inch
constexpr hti_format_t::tape_pos_t TACH_TICK_LENGTH = static_cast<hti_format_t::tape_pos_t>(hti_format_t::ONE_INCH_POS / 58.4);
constexpr uint8_t MODULUS_RESET = 0b10110011;   // Reset value of modulus: -77
constexpr unsigned GAP_TIME_MS = 1;             // 1 ms to detect start/end of gaps

// Bits in command register
enum : unsigned {
	CMD_REG_LEFT_LIGHT_BIT = 7, // Light of left drive (1)
	CMD_REG_RIGHT_LIGHT_BIT = 6,// Light of right drive (1)
	CMD_REG_GAP_WRITE_BIT = 5,  // Write gap (1)
	CMD_REG_LEFT_SEL_BIT = 4,   // Select left (1) or right (0) drive
	CMD_REG_WRITE_BIT = 3,      // Read (0) or Write (1)
	CMD_REG_SPEED_BIT = 2,      // Slow (0) or Fast (1)
	CMD_REG_DIR_BIT = 1,        // Reverse (0) or Forward (1)
	CMD_REG_RUN_BIT = 0         // Stop (0) or Run (1)
};

// Bits in status register
enum : unsigned {
	STAT_REG_TACH_INT_BIT = 7,  // Tachometer tick interrupt (1)
	STAT_REG_BYTE_READY_BIT = 6,// Byte ready (1)
	STAT_REG_GAP_BIT = 5,       // Gap detected (1)
	STAT_REG_HOLE_INT_BIT = 4,  // Hole interrupt (1)
	STAT_REG_TACH_DIV2_BIT = 3, // Tach/2
	STAT_REG_RIP_BIT = 2,       // Record in progress (1)
	STAT_REG_RIGHT_CIN_BIT = 1, // Right cartridge in (1)
	STAT_REG_LEFT_CIN_BIT = 0   // Left cartridge in (1)
};

// device type definition
DEFINE_DEVICE_TYPE(HP2640_TAPE, hp2640_tape_device, "hp2640_tape" , "HP2640 tape subsystem")

hp2640_tape_device::hp2640_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP2640_TAPE, tag, owner, clock)
	, m_irq_handler(*this)
	, m_led0_handler(*this)
	, m_led1_handler(*this)
	, m_drives(*this , "unit%u" , 0)
{
}

void hp2640_tape_device::command_w(uint8_t cmd)
{
	auto cmd_reg_diff = m_cmd_reg ^ cmd;
	m_cmd_reg = cmd;

	LOG("CMD=%02x D=%02x\n" , m_cmd_reg , cmd_reg_diff);

	m_led0_handler(BIT(m_cmd_reg , CMD_REG_LEFT_LIGHT_BIT));
	m_led1_handler(BIT(m_cmd_reg , CMD_REG_RIGHT_LIGHT_BIT));

	if (BIT(cmd_reg_diff , CMD_REG_LEFT_SEL_BIT)) {
		// Drive selection changed, deselect old drive
		m_drives[ m_selected_drive ]->set_op(hp_dc100_tape_device::OP_IDLE);
		m_drives[ m_selected_drive ]->set_speed_setpoint(hp_dc100_tape_device::SP_STOP , false);
		m_selected_drive = BIT(m_cmd_reg , CMD_REG_LEFT_SEL_BIT) ? 0 : 1;
	}

	if (cmd_reg_diff &
		(BIT_MASK<uint8_t>(CMD_REG_RUN_BIT) |
		 BIT_MASK<uint8_t>(CMD_REG_DIR_BIT) |
		 BIT_MASK<uint8_t>(CMD_REG_SPEED_BIT) |
		 BIT_MASK<uint8_t>(CMD_REG_WRITE_BIT) |
		 BIT_MASK<uint8_t>(CMD_REG_LEFT_SEL_BIT) |
		 BIT_MASK<uint8_t>(CMD_REG_GAP_WRITE_BIT))) {
		// Drive selection and/or speed and/or r/w operation changed

		// ISF == true when running forward at slow speed
		m_isf = (m_cmd_reg & (BIT_MASK<uint8_t>(CMD_REG_RUN_BIT) | BIT_MASK<uint8_t>(CMD_REG_DIR_BIT) | BIT_MASK<uint8_t>(CMD_REG_SPEED_BIT))) ==
			(BIT_MASK<uint8_t>(CMD_REG_RUN_BIT) | BIT_MASK<uint8_t>(CMD_REG_DIR_BIT));

		if (!BIT(m_cmd_reg , CMD_REG_WRITE_BIT)) {
			m_current_op = hp_dc100_tape_device::OP_READ;
		} else if (m_isf && BIT(m_cmd_reg , CMD_REG_WRITE_BIT) && !BIT(m_cmd_reg , CMD_REG_GAP_WRITE_BIT)) {
			m_current_op = hp_dc100_tape_device::OP_WRITE;
		} else if (BIT(m_cmd_reg , CMD_REG_WRITE_BIT) && BIT(m_cmd_reg , CMD_REG_GAP_WRITE_BIT)) {
			m_current_op = hp_dc100_tape_device::OP_ERASE;
		} else {
			m_current_op = hp_dc100_tape_device::OP_IDLE;
		}

		if (!set_speed()) {
			start_rd_wr();
		}
	}
}

uint8_t hp2640_tape_device::status_r()
{
	uint8_t res = 0;

	if (m_tach_latch) {
		BIT_SET(res , 7);
	}
	if (m_byte_ready) {
		BIT_SET(res , 6);
	}
	if (m_gap) {
		BIT_SET(res , 5);
	}
	if (m_hole_latch) {
		BIT_SET(res , 4);
	}
	if (m_tach_div2) {
		BIT_SET(res , 3);
	}
	if (m_rip) {
		BIT_SET(res , 2);
	}
	if (!m_drives[ 1 ]->cart_out_r()) {
		BIT_SET(res , 1);
	}
	if (!m_drives[ 0 ]->cart_out_r()) {
		BIT_SET(res , 0);
	}

	m_tach_latch = false;
	m_hole_latch = false;
	update_irq();

	LOG("STS=%02x\n" , res);
	return res;
}

void hp2640_tape_device::data_w(uint8_t data)
{
	LOG("DATA W=%02x\n" , data);
	m_data_rd = data;
	m_byte_ready = false;
	update_irq();
}

uint8_t hp2640_tape_device::data_r()
{
	m_byte_ready = false;
	update_irq();

	LOG("DATA R=%02x\n" , m_data_rd);
	return m_data_rd;
}

uint8_t hp2640_tape_device::poll_r() const
{
	return m_irq ? 0x80 : 0x00;
}

void hp2640_tape_device::device_add_mconfig(machine_config &config)
{
	for (unsigned i = 0; i < 2; i++) {
		auto& finder = m_drives[ i ];

		HP_DC100_TAPE(config , finder , 0);
		// Acceleration: 2000 in/s^2
		finder->set_acceleration(ACCELERATION);
		// Slow speed: 10 ips, Fast speed: 60 ips
		finder->set_set_points(SLOW_SPEED , FAST_SPEED);
		// 58.4 ticks per inch
		finder->set_tick_size(TACH_TICK_LENGTH);
		// Manchester encoded data
		finder->set_image_format(hti_format_t::HTI_MANCHESTER_MOD);
		// Moving when speed is >1 ips
		finder->set_go_threshold(MOVING_THRESHOLD);
		// Unit name: U0/U1
		finder->set_name(string_format("U%u" , i));

		finder->hole().set([this , i](int state) { hole_w(i , state); });
		finder->tacho_tick().set([this , i](int state) { tacho_tick_w(i , state); });
		finder->motion_event().set([this , i](int state) { motion_w(i , state); });
		finder->rd_bit().set([this , i](int state) { rd_bit_w(i , state); });
		finder->wr_bit().set([this , i](int state) { return wr_bit_r(i); });
	}
}

void hp2640_tape_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_led0_handler.resolve_safe();
	m_led1_handler.resolve_safe();

	m_gap_timer = timer_alloc(GAP_TMR_ID);
	m_cell_timer = timer_alloc(CELL_TMR_ID);

	save_item(NAME(m_selected_drive));
	save_item(NAME(m_cmd_reg));
	save_item(NAME(m_data_rd));
	save_item(NAME(m_data_sr));
	save_item(NAME(m_modulus));
	save_item(NAME(m_cell_cnt));
	save_item(NAME(m_bit_cnt));
	save_item(NAME(m_tach_div2));
	save_item(NAME(m_tach_latch));
	save_item(NAME(m_hole_latch));
	save_item(NAME(m_byte_ready));
	save_item(NAME(m_irq));
	save_item(NAME(m_bit_sync));
	save_item(NAME(m_wr_bit));
	save_item(NAME(m_last_rd_bit));
	save_item(NAME(m_isf));
	save_item(NAME(m_gap));
	save_item(NAME(m_prev_gap));
	save_item(NAME(m_rip));
}

void hp2640_tape_device::device_reset()
{
	m_selected_drive = 0;

	// All bits set to 1 but 3 & 0
	m_cmd_reg = ~0;
	BIT_CLR(m_cmd_reg , CMD_REG_WRITE_BIT);
	BIT_CLR(m_cmd_reg , CMD_REG_RUN_BIT);

	m_modulus = MODULUS_RESET;
	m_cell_cnt = 6;
	m_bit_cnt = 8;
	m_tach_div2 = false;
	m_tach_latch = false;
	m_hole_latch = false;
	m_byte_ready = false;
	m_irq = true;
	m_bit_sync = false;
	m_isf = false;
	m_gap = true;
	m_prev_gap = true;
	m_rip = false;
	m_current_op = hp_dc100_tape_device::OP_READ;
	update_irq();

	m_gap_timer->reset();
	m_cell_timer->reset();
}

void hp2640_tape_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	LOG("TMR %d @%s\n" , id , machine().time().to_string());

	switch (id) {
	case GAP_TMR_ID:
		m_gap = !m_gap;
		if (m_gap) {
			set_gap();
		} else {
			LOG("GAP ENDS\n");
			load_gap_timer();
			if (m_isf) {
				load_modulus();
			}
		}
		break;

	case CELL_TMR_ID:
		if (m_current_op == hp_dc100_tape_device::OP_READ) {
			m_cell_cnt++;
			if (m_cell_cnt == 15) {
				restart_cell_cnt();
			} else {
				load_modulus();
			}
		}
		break;

	default:
		break;
	}
}

void hp2640_tape_device::hole_w(unsigned drive , int state)
{
	if (state && drive == m_selected_drive) {
		LOG("HOLE\n");
		m_hole_latch = true;
		BIT_CLR(m_cmd_reg, CMD_REG_RUN_BIT);
		set_speed();
		update_irq();
	}
}

void hp2640_tape_device::tacho_tick_w(unsigned drive , int state)
{
	if (state && drive == m_selected_drive) {
		m_tach_div2 = !m_tach_div2;
		if (m_tach_div2) {
			LOG("HALF TICK\n");
			m_tach_latch = true;
			update_irq();
		}
	}
}

void hp2640_tape_device::motion_w(unsigned drive , int state)
{
	if (state && drive == m_selected_drive) {
		LOG("MOTION\n");
		start_rd_wr();
	}
}

void hp2640_tape_device::rd_bit_w(unsigned drive , int state)
{
	if (drive == m_selected_drive) {
		bool rd_bit = state != 0;
		bool transition = rd_bit != m_last_rd_bit;
		m_last_rd_bit = rd_bit;

		LOG("RD BIT %d TR %d GP %d IS %d CC %u MD %u @%s\n" , rd_bit , transition , m_gap , m_isf , m_cell_cnt , m_modulus , machine().time().to_string());

		if (transition) {
			if (m_gap) {
				if (m_prev_gap) {
					m_prev_gap = false;
					load_gap_timer();
				}
			} else {
				load_gap_timer();

				if (m_isf) {
					if (BIT(m_cell_cnt , 3)) {
						// Adjust modulus by +/- 1
						if (BIT(m_cell_cnt , 1)) {
							m_modulus--;
						} else {
							m_modulus++;
						}
					}
					bool prev_bit_sync = m_bit_sync;
					if ((m_cell_cnt & 0xc) == 0xc) {
						if (m_bit_sync) {
							// Shift in read bit
							LOG("BIT %d CNT %u\n" , rd_bit , m_bit_cnt);
							m_data_sr >>= 1;
							if (rd_bit) {
								BIT_SET(m_data_sr, 7);
							}
							m_bit_cnt++;
							if (m_bit_cnt == 16) {
								LOG("RD DATA=%02x\n" , m_data_sr);
								m_bit_cnt = 8;
								m_data_rd = m_data_sr;
								m_byte_ready = true;
								update_irq();
							}
						} else if (rd_bit) {
							// Sync achieved
							LOG("SYNC\n");
							m_bit_sync = true;
							m_bit_cnt = 8;
							m_byte_ready = true;
							update_irq();
						}
					}
					if (BIT(m_cell_cnt , 2) || !prev_bit_sync) {
						restart_cell_cnt();
					}
				}
			}
		}
	}
}

int hp2640_tape_device::wr_bit_r(unsigned drive)
{
	if (drive == m_selected_drive) {
		// IRL the m_cell_cnt counts in [6..13] range
		if (m_cell_cnt == 6) {
			m_cell_cnt = 13;
			m_wr_bit = !BIT(m_data_sr , 0);
		} else if (m_cell_cnt == 13) {
			m_cell_cnt = 6;
			m_wr_bit = !m_wr_bit;
			m_bit_cnt++;
			if (m_bit_cnt == 16) {
				m_data_sr = m_data_rd;
				m_bit_cnt = 8;
				m_byte_ready = true;
				update_irq();
			} else {
				m_data_sr >>= 1;
			}
		}
		return m_wr_bit;
	} else {
		return 0;
	}
}

void hp2640_tape_device::update_irq()
{
	bool new_irq = m_tach_latch || m_hole_latch || m_byte_ready;

	if (new_irq != m_irq) {
		LOG("IRQ %d\n" , new_irq);
		m_irq = new_irq;
		m_irq_handler(m_irq);
	}
}

bool hp2640_tape_device::set_speed()
{
	hp_dc100_tape_device::tape_speed_t sp;

	if (!BIT(m_cmd_reg , CMD_REG_RUN_BIT)) {
		sp = hp_dc100_tape_device::SP_STOP;
	} else if (BIT(m_cmd_reg , CMD_REG_SPEED_BIT)) {
		sp = hp_dc100_tape_device::SP_FAST;
	} else {
		sp = hp_dc100_tape_device::SP_SLOW;
	}

	bool changed = m_drives[ m_selected_drive ]->set_speed_setpoint(sp , BIT(m_cmd_reg , CMD_REG_DIR_BIT));

	if (changed) {
		start_rd_wr(true);
	}

	return changed;
}

void hp2640_tape_device::start_rd_wr(bool recalc)
{
	hp_dc100_tape_device& drive = *m_drives[ m_selected_drive ];

	m_rip = false;

	if (drive.is_above_threshold() && m_current_op == hp_dc100_tape_device::OP_READ) {
		// Reading
		LOG("START RD GP %d IS %d\n" , m_gap , m_isf);
		drive.set_op(hp_dc100_tape_device::OP_READ , recalc);
		if (m_gap) {
			m_modulus = MODULUS_RESET;
			m_bit_sync = false;
			stop_cell_cnt();
		}
		if (!m_isf) {
			stop_cell_cnt();
		}
	} else if (!drive.is_accelerating() && !drive.wpr_r() && m_current_op == hp_dc100_tape_device::OP_WRITE) {
		// Data writing
		LOG("START WR\n");
		stop_cell_cnt();
		drive.set_op(hp_dc100_tape_device::OP_WRITE);
	} else if (!drive.wpr_r() && m_current_op == hp_dc100_tape_device::OP_ERASE) {
		// Gap writing
		LOG("START ERASE\n");
		m_rip = true;
		m_data_sr = 0;
		m_bit_cnt = 8;
		stop_cell_cnt();
		drive.set_op(hp_dc100_tape_device::OP_ERASE);
	} else {
		LOG("IDLE\n");
		stop_cell_cnt();
		drive.set_op(hp_dc100_tape_device::OP_IDLE);
	}
}

void hp2640_tape_device::load_modulus()
{
	m_cell_timer->adjust(clocks_to_attotime(256U - m_modulus));
}

void hp2640_tape_device::restart_cell_cnt()
{
	m_cell_cnt = 6;
	load_modulus();
}

void hp2640_tape_device::stop_cell_cnt()
{
	m_cell_cnt = 6;
	m_cell_timer->reset();
}

void hp2640_tape_device::set_gap()
{
	LOG("GAP START\n");
	m_gap = true;
	m_prev_gap = true;
	if (m_drives[ m_selected_drive ]->get_op() == hp_dc100_tape_device::OP_READ) {
		m_modulus = MODULUS_RESET;
		m_bit_sync = false;
		stop_cell_cnt();
	}
}

void hp2640_tape_device::load_gap_timer()
{
	m_gap_timer->adjust(attotime::from_msec(GAP_TIME_MS));
}
