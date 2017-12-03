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
#define VERBOSE 0
#include "logmacro.h"

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
// One tachometer tick every 1/32 of inch
static constexpr hti_format_t::tape_pos_t TACH_TICK_LEN = hti_format_t::ONE_INCH_POS / 32;
static constexpr hti_format_t::tape_pos_t TAPE_INIT_POS = hti_format_t::ONE_INCH_POS * 80;
// Slow tape speed (10 ips)
static constexpr unsigned TACH_FREQ_SLOW = hti_format_t::ONE_INCH_POS * 10;
// Fast tape speed (60 ips)
static constexpr unsigned TACH_FREQ_FAST = hti_format_t::ONE_INCH_POS * 60;
// Minimum gap size (totally made up)
static constexpr hti_format_t::tape_pos_t MIN_GAP_SIZE = TACH_TICK_LEN;
// Braking distance at fast speed: 1.5 in (this value assumes the deceleration is 1200 in/s^2, the same as in TACO)
static constexpr hti_format_t::tape_pos_t FAST_BRAKE_DIST = hti_format_t::ONE_INCH_POS * 1.5;
// Braking distance at slow speed: 1/24 in
static constexpr hti_format_t::tape_pos_t SLOW_BRAKE_DIST = hti_format_t::ONE_INCH_POS / 24;

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
	: device_t(mconfig , HP_1MA6 , tag , owner , clock),
	  device_image_interface(mconfig , *this),
	  m_image(),
	  m_image_dirty(false)
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
		update_tape_pos();
		if (m_cmd_state == CMD_IDLE) {
			BIT_SET(m_status_reg , STS_READY_BIT);
		} else if (m_cmd_state == CMD_STOPPING ||
				   m_cmd_state == CMD_FAST_FWD_REV ||
				   m_cmd_state == CMD_WR_GAP) {
			BIT_CLR(m_status_reg , STS_READY_BIT);
		}
		if ((m_control_reg & (BIT_MASK(CTL_WRITE_SYNC_BIT) |
							  BIT_MASK(CTL_WRITE_GAP_BIT))) != 0 &&
			(!m_cartridge_in || !is_readonly())) {
			BIT_SET(m_status_reg , STS_WRITE_EN_BIT);
		} else {
			BIT_CLR(m_status_reg , STS_WRITE_EN_BIT);
		}
		// Gap detection
		if (m_cmd_state == CMD_IDLE) {
			BIT_SET(m_status_reg , STS_GAP_BIT);
		} else if (m_gap_detect_start != hti_format_t::NULL_TAPE_POS) {
			if (abs(m_gap_detect_start - m_tape_pos) >= MIN_GAP_SIZE) {
				auto tmp = m_tape_pos;
				hti_format_t::pos_offset(tmp , is_moving_fwd() , -MIN_GAP_SIZE);
				if (m_image.just_gap(current_track() , tmp , m_tape_pos)) {
					BIT_SET(m_status_reg , STS_GAP_BIT);
				}
			} else {
				BIT_SET(m_status_reg , STS_GAP_BIT);
			}
		}

		res = m_status_reg;
		// Clear latching bits
		BIT_CLR(m_status_reg , STS_HOLE_BIT);
		BIT_CLR(m_status_reg , STS_GAP_BIT);
		BIT_CLR(m_status_reg , STS_TACH_BIT);
		BIT_CLR(m_status_reg , STS_READY_BIT);
		if (m_cartridge_in) {
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

void hp_1ma6_device::device_start()
{
	save_item(NAME(m_data_reg));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_control_reg));
	save_item(NAME(m_tape_pos));
	save_item(NAME(m_prev_pos));
	save_item(NAME(m_start_time));
	save_item(NAME(m_image_dirty));
	save_item(NAME(m_rw_pos));
	save_item(NAME(m_next_word));
	save_item(NAME(m_rd_it_valid));
	save_item(NAME(m_gap_detect_start));

	m_tape_timer = timer_alloc(TAPE_TMR_ID);
	m_hole_timer = timer_alloc(HOLE_TMR_ID);
}

void hp_1ma6_device::device_reset()
{
	clear_state();

	m_tape_timer->reset();
	m_hole_timer->reset();
}

void hp_1ma6_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	update_tape_pos();

	switch (id) {
	case TAPE_TMR_ID:
		{
			LOG("TMR %.6f %d %d\n" , machine().time().as_double() , m_tape_pos , m_cmd_state);
			attotime cmd_duration = attotime::never;
			switch (m_cmd_state) {
			case CMD_STOPPING:
				m_cmd_state = CMD_IDLE;
				stop_tape();
				break;

			case CMD_RD_WAIT_SYNC:
				m_tape_pos = m_rw_pos;
				LOG("RD WS @ %d = %04x\n" , m_tape_pos , m_rd_it->second);
				if (m_rd_it->second != 0xffff) {
					// Sync achieved
					m_cmd_state = CMD_RD_MSB;
				}
				cmd_duration = advance_rd_state();
				break;

			case CMD_RD_MSB:
				m_tape_pos = m_rw_pos;
				m_next_word = m_rd_it->second;
				LOG("RD T%u @ %d = %04x\n" , current_track() , m_rw_pos , m_next_word);
				m_data_reg = (uint8_t)(m_next_word >> 8);
				BIT_SET(m_status_reg , STS_READY_BIT);
				// Time to reach LSB: half the length of a 0 word
				// Actually it varies basing on MSB content
				cmd_duration = time_to_distance(hti_format_t::word_length(0) / 2);
				m_cmd_state = CMD_RD_LSB;
				break;

			case CMD_RD_LSB:
				m_data_reg = (uint8_t)m_next_word;
				BIT_SET(m_status_reg , STS_READY_BIT);
				m_cmd_state = CMD_RD_MSB;
				cmd_duration = advance_rd_state();
				break;

			case CMD_WR_SYNC:
				m_tape_pos = m_rw_pos;
				if (m_rd_it->second != 0xffff) {
					// Got preamble
					m_next_word = (hti_format_t::tape_word_t)m_data_reg << 8;
					BIT_SET(m_status_reg , STS_READY_BIT);
					m_cmd_state = CMD_WR_LSB;
					cmd_duration = time_to_distance(hti_format_t::word_length(0) / 2);
				} else {
					cmd_duration = advance_rd_state();
				}
				break;

			case CMD_WR_MSB:
				// **** FWD ****
				// | m_rw_pos   --> m_next_word
				// | m_tape_pos --> (start of next word)
				// V
				//
				// **** REV ****
				// ^ m_tape_pos --> m_next_word
				// | m_rw_pos   --> (start of previous word)
				// |
				{
					auto length = hti_format_t::word_length(m_next_word);
					if (!is_moving_fwd()) {
						hti_format_t::pos_offset(m_rw_pos , false , length);
					}
					LOG("WR T%u @ %d = %04x\n" , current_track() , m_rw_pos , m_next_word);
					if (m_cartridge_in) {
						m_image.write_word(current_track() , m_rw_pos , m_next_word , length , is_moving_fwd());
						m_image_dirty = true;
					}
					if (is_moving_fwd()) {
						hti_format_t::pos_offset(m_rw_pos , true , length);
					}
					m_tape_pos = m_rw_pos;

					m_next_word = (hti_format_t::tape_word_t)m_data_reg << 8;
					BIT_SET(m_status_reg , STS_READY_BIT);
					m_cmd_state = CMD_WR_LSB;
					cmd_duration = time_to_distance(hti_format_t::word_length(0) / 2);
				}
				break;

			case CMD_WR_LSB:
				{
					m_next_word = (m_next_word & 0xff00) | m_data_reg;
					BIT_SET(m_status_reg , STS_READY_BIT);
					m_cmd_state = CMD_WR_MSB;
					auto length = hti_format_t::word_length(m_next_word);
					auto tmp = m_rw_pos;
					hti_format_t::pos_offset(tmp , is_moving_fwd() , length);
					cmd_duration = time_to_target(tmp);
				}
				break;

			default:
				break;
			}
			m_tape_timer->adjust(cmd_duration);
		}
		break;

	case HOLE_TMR_ID:
		BIT_SET(m_status_reg , STS_HOLE_BIT);
		m_hole_timer->adjust(time_to_next_hole());
		break;

	default:
		break;
	}
}

image_init_result hp_1ma6_device::call_load()
{
	LOG("call_load\n");
	return internal_load(false);
}

image_init_result hp_1ma6_device::call_create(int format_type, util::option_resolution *format_options)
{
	LOG("call_create\n");
	return internal_load(true);
}

void hp_1ma6_device::call_unload()
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
	set_cartridge_in(false);
}

std::string hp_1ma6_device::call_display()
{
	std::string buffer;
	// Mostly lifted from cassette_image_device::call_display ;)
	// See also hp_taco_device::call_display

	// Do not show anything if image not loaded or tape not moving
	if (!exists() || m_start_time.is_never()) {
		return buffer;
	}

	char track = BIT(m_control_reg , CTL_TRACK_NO_BIT) ? 'B' : 'A';
	char r_w = m_cmd_state == CMD_WR_SYNC ||
		m_cmd_state == CMD_WR_MSB ||
		m_cmd_state == CMD_WR_LSB ||
		m_cmd_state == CMD_WR_GAP ? 'W' : 'R';
	char m1;
	char m2;

	if (is_moving_fwd()) {
		m1 = '>';
		m2 = BIT(m_control_reg , CTL_FAST_BIT) ? '>' : ' ';
	} else {
		m1 = '<';
		m2 = BIT(m_control_reg , CTL_FAST_BIT) ? '<' : ' ';
	}

	int pos_in = current_tape_pos() / hti_format_t::ONE_INCH_POS;

	buffer = string_format("%c %c %c%c [%04d/1824]" , track , r_w , m1 , m2 , pos_in);

	return buffer;
}

const char *hp_1ma6_device::file_extensions() const
{
	return "hti";
}

void hp_1ma6_device::clear_state()
{
	m_data_reg = 0;
	m_status_reg = 0;
	m_control_reg = 0;
	m_tape_pos = TAPE_INIT_POS;
	m_prev_pos = TAPE_INIT_POS;
	m_start_time = attotime::never;
	m_rw_pos = hti_format_t::NULL_TAPE_POS;
	m_next_word = 0;
	m_rd_it_valid = false;
	m_gap_detect_start = hti_format_t::NULL_TAPE_POS;
	m_cmd_state = CMD_IDLE;
	set_cartridge_in(false);
	set_cartridge_in(is_loaded());
}

unsigned hp_1ma6_device::current_track() const
{
	return BIT(m_control_reg , CTL_TRACK_NO_BIT);
}

unsigned hp_1ma6_device::speed_to_tick_freq(void) const
{
	unsigned freq = BIT(m_control_reg , CTL_FAST_BIT) ? TACH_FREQ_FAST : TACH_FREQ_SLOW;

	if (m_cmd_state == CMD_STOPPING) {
		// Braking
		freq /= 2;
	}

	return freq;
}

attotime hp_1ma6_device::time_to_distance(hti_format_t::tape_pos_t distance) const
{
	// +1 for rounding
	return attotime::from_ticks(distance + 1 , speed_to_tick_freq());
}

attotime hp_1ma6_device::time_to_target(hti_format_t::tape_pos_t target) const
{
	return time_to_distance(abs(target - m_tape_pos));
}

attotime hp_1ma6_device::time_to_next_hole(void) const
{
	auto pos = hti_format_t::next_hole(m_tape_pos , is_moving_fwd());

	if (pos == hti_format_t::NULL_TAPE_POS) {
		return attotime::never;
	} else {
		return time_to_target(pos);
	}
}

attotime hp_1ma6_device::time_to_rd_next_word(hti_format_t::tape_pos_t& word_rd_pos) const
{
	if (m_rd_it_valid) {
		word_rd_pos = hti_format_t::farthest_end(m_rd_it , is_moving_fwd());
		return time_to_target(word_rd_pos);
	} else {
		return attotime::never;
	}
}

hti_format_t::tape_pos_t hp_1ma6_device::current_tape_pos() const
{
	if (m_start_time.is_never()) {
		// Tape not moving
		return m_tape_pos;
	}

	attotime delta_time(machine().time() - m_start_time);
	hti_format_t::tape_pos_t delta_tach = (hti_format_t::tape_pos_t)(delta_time.as_ticks(speed_to_tick_freq()));

	auto tape_pos = m_tape_pos;
	if (!hti_format_t::pos_offset(tape_pos , is_moving_fwd() , delta_tach)) {
		LOG("Tape unspooled!\n");
	}

	return tape_pos;
}

void hp_1ma6_device::update_tape_pos()
{
	if (m_start_time.is_never()) {
		// Tape not moving
		return;
	}

	m_tape_pos = current_tape_pos();
	m_start_time = machine().time();

	// Tachometer ticks
	if ((m_prev_pos / TACH_TICK_LEN) != (m_tape_pos / TACH_TICK_LEN)) {
		BIT_SET(m_status_reg , STS_TACH_BIT);
	}
	m_prev_pos = m_tape_pos;
}

bool hp_1ma6_device::is_reading() const
{
	return m_cmd_state >= CMD_RD_WAIT_SYNC &&
		m_cmd_state <= CMD_RD_LSB;
}

bool hp_1ma6_device::is_writing() const
{
	return m_cmd_state >= CMD_WR_SYNC &&
		m_cmd_state <= CMD_WR_GAP;
}

bool hp_1ma6_device::is_moving_fwd() const
{
	return BIT(m_control_reg , CTL_DIR_FWD_BIT);
}

void hp_1ma6_device::set_cartridge_in(bool in)
{
	m_cartridge_in = in;
	if (!m_cartridge_in) {
		BIT_CLR(m_status_reg , STS_CASSETTE_IN_BIT);
	}
}

void hp_1ma6_device::start_tape()
{
	m_start_time = machine().time();
	m_prev_pos = m_tape_pos;
	if (!is_writing()) {
		m_gap_detect_start = m_tape_pos;
	} else {
		m_gap_detect_start = hti_format_t::NULL_TAPE_POS;
	}
	m_hole_timer->adjust(time_to_next_hole());
}

void hp_1ma6_device::stop_tape()
{
	m_start_time = attotime::never;
	m_gap_detect_start = hti_format_t::NULL_TAPE_POS;
	m_hole_timer->reset();
	m_tape_timer->reset();
}

attotime hp_1ma6_device::advance_rd_state()
{
	auto res = m_image.adv_it(current_track() , is_moving_fwd() , m_rd_it);
	if (res == hti_format_t::ADV_NO_MORE_DATA) {
		m_rd_it_valid = false;
	} else if (res == hti_format_t::ADV_DISCONT_DATA && is_reading()) {
		// Discontinuous data: move back to sync search (probably..)
		m_cmd_state = CMD_RD_WAIT_SYNC;
	}
	return time_to_rd_next_word(m_rw_pos);
}

void hp_1ma6_device::start_cmd_exec(uint8_t new_ctl_reg)
{
	if (new_ctl_reg == m_control_reg) {
		return;
	}

	cmd_state_t new_state;

	if (!BIT(new_ctl_reg , CTL_POWER_UP_BIT)) {
		new_state = CMD_IDLE;
	} else if (!BIT(new_ctl_reg , CTL_MOTOR_ON_BIT)) {
		new_state = CMD_STOPPING;
	} else switch (new_ctl_reg & (BIT_MASK(CTL_FAST_BIT) |
								  BIT_MASK(CTL_WRITE_DATA_BIT) |
								  BIT_MASK(CTL_WRITE_SYNC_BIT) |
								  BIT_MASK(CTL_WRITE_GAP_BIT))) {
		case 0:
			new_state = CMD_RD_WAIT_SYNC;
			break;

		case BIT_MASK(CTL_FAST_BIT):
		case BIT_MASK(CTL_WRITE_GAP_BIT):
		case BIT_MASK(CTL_WRITE_GAP_BIT) | BIT_MASK(CTL_WRITE_DATA_BIT):
			new_state = CMD_FAST_FWD_REV;
			break;

		case BIT_MASK(CTL_WRITE_DATA_BIT):
			new_state = CMD_WR_SYNC;
			break;

		case BIT_MASK(CTL_WRITE_SYNC_BIT):
			new_state = CMD_WR_MSB;
			break;

		case BIT_MASK(CTL_WRITE_SYNC_BIT) | BIT_MASK(CTL_WRITE_GAP_BIT):
			new_state = CMD_WR_GAP;
			break;

		default:
			LOG("Unknown command %02x\n" , new_ctl_reg);
			return;
	}

	update_tape_pos();
	LOG("State %d -> %d pos = %d\n" , m_cmd_state , new_state , m_tape_pos);

	if (m_cmd_state == CMD_WR_GAP) {
		// Finish gap writing
		LOG("T%u: Gap from %d to %d\n" , current_track() , m_rw_pos , m_tape_pos);
		if (m_cartridge_in) {
			m_image.write_gap(current_track() , m_rw_pos , m_tape_pos);
			m_image_dirty = true;
		}
	}

	uint8_t saved_dir_speed = m_control_reg & (BIT_MASK(CTL_DIR_FWD_BIT) | BIT_MASK(CTL_FAST_BIT));

	m_control_reg = new_ctl_reg;

	attotime cmd_duration = attotime::never;

	switch (new_state) {
	case CMD_IDLE:
		m_cmd_state = CMD_IDLE;
		stop_tape();
		break;

	case CMD_STOPPING:
		if (m_cmd_state != CMD_IDLE &&
			m_cmd_state != CMD_STOPPING) {
			m_cmd_state = CMD_STOPPING;
			// Keep speed & direction from before the STOP command
			m_control_reg = (m_control_reg & ~(BIT_MASK(CTL_DIR_FWD_BIT) | BIT_MASK(CTL_FAST_BIT))) | saved_dir_speed;
			cmd_duration = time_to_distance(BIT(m_control_reg , CTL_FAST_BIT) ? FAST_BRAKE_DIST : SLOW_BRAKE_DIST);
		}
		break;

	case CMD_RD_WAIT_SYNC:
		m_cmd_state = CMD_RD_WAIT_SYNC;
		start_tape();
		m_rd_it_valid = m_image.next_data(current_track() , m_tape_pos , is_moving_fwd() , false , m_rd_it);
		cmd_duration = time_to_rd_next_word(m_rw_pos);
		// TODO: ??
		BIT_CLR(m_status_reg , STS_READY_BIT);
		break;

	case CMD_WR_SYNC:
		m_cmd_state = CMD_WR_SYNC;
		start_tape();
		m_rd_it_valid = m_image.next_data(current_track() , m_tape_pos , is_moving_fwd() , false , m_rd_it);
		cmd_duration = time_to_rd_next_word(m_rw_pos);
		BIT_SET(m_status_reg , STS_READY_BIT);
		break;

	case CMD_WR_MSB:
		m_cmd_state = CMD_WR_MSB;
		start_tape();
		m_rw_pos = m_tape_pos;
		// SYNC word: MSB = 00, LSB = data register
		m_next_word = (hti_format_t::tape_word_t)m_data_reg;
		BIT_SET(m_status_reg , STS_READY_BIT);
		cmd_duration = time_to_distance(hti_format_t::word_length(m_next_word));
		break;

	case CMD_WR_GAP:
		m_cmd_state = CMD_WR_GAP;
		start_tape();
		m_rw_pos = m_tape_pos;
		break;

	case CMD_FAST_FWD_REV:
		m_cmd_state = CMD_FAST_FWD_REV;
		start_tape();
		break;

	default:
		break;
	}

	m_tape_timer->adjust(cmd_duration);
}

image_init_result hp_1ma6_device::internal_load(bool is_create)
{
	device_reset();

	io_generic io;
	io.file = (device_image_interface *)this;
	io.procs = &image_ioprocs;
	io.filler = 0;
	if (is_create) {
		m_image.clear_tape();
		m_image.save_tape(&io);
	} else if (!m_image.load_tape(&io)) {
		seterror(IMAGE_ERROR_INVALIDIMAGE , "Wrong format");
		set_cartridge_in(false);
		return image_init_result::FAIL;
	}

	m_image_dirty = false;
	set_cartridge_in(true);

	return image_init_result::PASS;
}
