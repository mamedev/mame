// license:BSD-3-Clause
// copyright-holders:F. Ulivi

/*********************************************************************

    Intel 1 Mbit magnetic bubble memory subsystem

    **** TODO ****
    - Commands to be tested: WR_BLR, WR_BL, RD_FSA_STAT, RCD
    - More than 1 MBM

*********************************************************************/

#include "emu.h"
#include "i7110.h"

#include "machine/timer.h"

#include <bitset>

// Debugging
#define LOG_FSM         (1U << 1)
#define LOG_REG         (1U << 2)
#define LOG_IRQ         (1U << 3)
#define LOG_DRQ         (1U << 4)
#define LOG_FSA_IO      (1U << 5)
#define LOG_CMD         (1U << 6)
#define LOG_FIFO        (1U << 7)
#define LOG_FSA         (1U << 8)

#undef VERBOSE
#define VERBOSE 0
// #define VERBOSE (LOG_GENERAL | LOG_FSM | LOG_REG | LOG_IRQ | LOG_DRQ | LOG_FSA_IO | LOG_CMD | LOG_FIFO | LOG_FSA)
#include "logmacro.h"

#define LOGFSM(...)     LOGMASKED(LOG_FSM,    __VA_ARGS__)
#define LOGREG(...)     LOGMASKED(LOG_REG,    __VA_ARGS__)
#define LOGIRQ(...)     LOGMASKED(LOG_IRQ,    __VA_ARGS__)
#define LOGDRQ(...)     LOGMASKED(LOG_DRQ,    __VA_ARGS__)
#define LOGFSA_IO(...)  LOGMASKED(LOG_FSA_IO, __VA_ARGS__)
#define LOGCMD(...)     LOGMASKED(LOG_CMD,    __VA_ARGS__)
#define LOGFIFO(...)    LOGMASKED(LOG_FIFO,   __VA_ARGS__)
#define LOGFSA(...)     LOGMASKED(LOG_FSA,    __VA_ARGS__)

// Device type definition
DEFINE_DEVICE_TYPE(FSA_CHANNEL, fsa_channel_device, "fsa_channel", "i7242 FSA channel")
DEFINE_DEVICE_TYPE(IBUBBLE, ibubble_device, "intel_mbm", "Intel i7110 bubble chipset")
DEFINE_DEVICE_TYPE(I7220_1, i7220_1_device, "i7220_1", "Intel i7220-1 bubble memory controller")

namespace {
	// Bit manipulation
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

// +--------------------+
// | fsa_channel_device |
// +--------------------+
fsa_channel_device::fsa_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FSA_CHANNEL, tag, owner, clock)
	, m_timer(*this, "tmr")
{
}

void fsa_channel_device::field_rotate()
{
	rotate(m_bootloop, true);
	for (unsigned i = 0; i < QUADS_PER_CH; i++) {
		for (unsigned j = 0; j < LOOPS_PER_QUAD; j++) {
			rotate(m_data_loops[ i ][ j ], true);
		}
	}
	rotate(m_even_out, false);
	rotate(m_odd_out, false);
	rotate(m_even_in, false);
	rotate(m_odd_in, false);
}

void fsa_channel_device::cmd_w(bool select, fsa_channel_device::fsa_cmd cmd)
{
	m_selected = select;
	bool enable = false;

	if (m_selected || cmd == FSA_CMD_NOP || cmd == FSA_CMD_SET_EC) {
		switch (cmd) {
		case FSA_CMD_NOP:
			reset_pointers();
			break;

		case FSA_CMD_SWRESET:
			reset_pointers();
			clear_errors();
			reset_error_corr();
			BIT_CLR(m_status, STAT_ECF);
			cmd = FSA_CMD_NOP;
			break;

		case FSA_CMD_INIT:
			reset_pointers();
			clear_errors();
			reset_error_corr();
			enable = true;
			break;

		case FSA_CMD_WRITE_DATA:
			reset_pointers();
			reset_error_corr();
			enable = true;
			break;

		case FSA_CMD_READ_DATA:
			reset_pointers();
			reset_error_corr();
			enable = true;
			break;

		case FSA_CMD_ICD:
			if (BIT(m_status, STAT_ECF)) {
				reset_pointers();
				// FIFO is actually full, even after resetting the pointers
				BIT_CLR(m_status, STAT_FIFOMT);
				BIT_SET(m_status, STAT_FIFOFL);
				// Clear ERRFLG
				BIT_CLR(m_status, STAT_CORRERR);
				BIT_CLR(m_status, STAT_UNCORRERR);
				pre_error_trapping();
				m_timer->adjust(clocks_to_attotime(ICD_CLOCKS));
				enable = true;
			} else {
				cmd = FSA_CMD_NOP;
			}
			break;

		case FSA_CMD_RCD:
			if (BIT(m_status, STAT_ECF)) {
				reset_pointers();
				// FIFO is actually full, even after resetting the pointers
				BIT_CLR(m_status, STAT_FIFOMT);
				BIT_SET(m_status, STAT_FIFOFL);
				// Clear ERRFLG
				BIT_CLR(m_status, STAT_CORRERR);
				BIT_CLR(m_status, STAT_UNCORRERR);
				pre_error_trapping();
				enable = true;
			} else {
				cmd = FSA_CMD_NOP;
			}
			break;

		case FSA_CMD_WRITE_BLR:
			// BLR is written through dio_w
			reset_pointers();
			break;

		case FSA_CMD_READ_BLR:
			// BLR is read through dio_r
			reset_pointers();
			break;

		case FSA_CMD_SET_ENABLE:
			reset_pointers();
			enable = true;
			cmd = FSA_CMD_NOP;
			break;

		case FSA_CMD_READ_ERRFLG:
			// Errflg is read through dio_r
			break;

		case FSA_CMD_SET_EC:
			if (m_selected) {
				BIT_SET(m_status, STAT_ECF);
			} else {
				BIT_CLR(m_status, STAT_ECF);
			}
			reset_pointers();
			cmd = FSA_CMD_NOP;
			break;

		case FSA_CMD_READ_STATUS:
			// Status is actually read by calling status_r
			break;

		default:
			LOG("Invalid cmd %d\n", unsigned(cmd));
			cmd = FSA_CMD_NOP;
			reset_pointers();
			break;
		}
		m_curr_cmd = cmd;
	}
	LOGFSA("cmd %x %x sel %d en %d\n", unsigned(cmd), unsigned(m_curr_cmd), select, enable);
	set_enable(enable);
}

void fsa_channel_device::dio_w(int data)
{
	if (!m_selected) {
		return;
	}

	LOGFSA("DIOW %x=%d\n", unsigned(m_curr_cmd), data);

	switch (m_curr_cmd) {
	case FSA_CMD_WRITE_DATA:
		if (!BIT(m_status, STAT_ECF) || m_fifo_in_idx < PAYLOAD_BITS) {
			fire_code_enc(data != 0);
			fifo_enqueue(data != 0);
		} else {
			// data is ignored, bits come from Fire code
			fifo_enqueue(BIT(m_code_accum, FIRE_CODE_BITS - 1));
			m_code_accum = (m_code_accum << 1) & FIRE_MASK;
		}
		break;

	case FSA_CMD_WRITE_BLR:
		m_blr[ m_blr_idx ] = data != 0;
		m_blr_idx++;
		if (m_blr_idx == BLR_BITS) {
			m_blr_idx = 0;
			m_curr_cmd = FSA_CMD_NOP;
		}
		break;

	default:
		LOG("Cmd %d invalid in dio_w\n", m_curr_cmd);
		m_curr_cmd = FSA_CMD_NOP;
		break;
	}
}

int fsa_channel_device::dio_r()
{
	int res = 0;

	if (m_selected) {
		switch (m_curr_cmd) {
		case FSA_CMD_INIT:
			res = m_data_out;
			break;

		case FSA_CMD_READ_DATA:
			res = fifo_dequeue();
			break;

		case FSA_CMD_RCD:
			res = correct_one_bit();
			if (BIT(m_status, STAT_FIFOMT)) {
				if (m_ec_state == ec_state::EC_NO_ERROR) {
					LOG("RCD done, no error correction\n");
				} else if (m_ec_state == ec_state::EC_WAIT_TRAP) {
					LOG("RCD done, uncorrectable errors detected\n");
					BIT_SET(m_status, STAT_UNCORRERR);
				} else {
					LOG("RCD done, correctable error(s) detected & corrected\n");
					BIT_SET(m_status, STAT_CORRERR);
				}
				m_curr_cmd = FSA_CMD_NOP;
			}
			break;

		case FSA_CMD_READ_BLR:
			res = m_blr[ m_blr_idx ];
			m_blr_idx++;
			if (m_blr_idx == BLR_BITS) {
				m_blr_idx = 0;
				m_curr_cmd = FSA_CMD_NOP;
			}
			break;

		case FSA_CMD_READ_ERRFLG:
			res = errflg_r();
			m_curr_cmd = FSA_CMD_NOP;
			break;

		default:
			LOG("Cmd %d invalid in dio_r\n", m_curr_cmd);
			m_curr_cmd = FSA_CMD_NOP;
			break;
		}
	}
	LOGFSA("DIOR %x=%d\n", unsigned(m_curr_cmd), res);
	return res;
}

uint8_t fsa_channel_device::status_r()
{
	uint8_t res = 0;

	if (m_selected && m_curr_cmd == FSA_CMD_READ_STATUS) {
		res = m_status;
		clear_errors();
		m_curr_cmd = FSA_CMD_NOP;
	}

	return res;
}

void fsa_channel_device::shiftclk()
{
	if (!m_selected) {
		return;
	}

	switch (m_curr_cmd) {
	case FSA_CMD_INIT:
		m_data_out = detector_r();
		break;

	case FSA_CMD_WRITE_DATA: {
		bool gen = false;
		// Writing to a good loop?
		// LOGFSA("WR blr @%u=%u\n", get_blr_idx(), m_blr[ get_blr_idx() ]);
		if (m_blr[ get_blr_idx() ]) {
			gen = fifo_dequeue();
		}
		m_blr_idx++;
		if (m_blr_idx == BLR_BITS * 2) {
			m_blr_idx = 0;
		}
		generate(gen);
	}
		break;

	case FSA_CMD_READ_DATA: {
		bool bit = detector_r();
		// Reading from a good loop?
		// LOGFSA("RD blr @%u=%u\n", get_blr_idx(), m_blr[ get_blr_idx() ]);
		if (m_blr[ get_blr_idx() ]) {
			fifo_enqueue(bit);
			if (BIT(m_status, STAT_ECF)) {
				fire_code_syn(bit);
				LOGFSA("RD %d %u %04x %02x\n", bit, m_fifo_in_idx, m_code_accum, m_status);
				if (m_fifo_in_idx == 0 && m_code_accum != 0) {
					// When reading, any error is first reported as correctable
					// ICD or RCD command is then used to tell correctable errors from uncorrectable ones
					LOG("Error, syn=%04x\n", m_code_accum);
					BIT_SET(m_status, STAT_CORRERR);
				}
			}
		}
		m_blr_idx++;
		if (m_blr_idx == BLR_BITS * 2) {
			m_blr_idx = 0;
		}
	}
		break;

	default:
		LOG("Cmd %d invalid in shiftclk\n", m_curr_cmd);
		m_curr_cmd = FSA_CMD_NOP;
		break;
	}
}

void fsa_channel_device::bubble_replicate()
{
	// Even quad
	for (unsigned i = 0; i < LOOPS_PER_QUAD; ++i) {
		m_even_out[ EVEN_OUT_LOOP80_POS - i * DATA_BIT_OUT_DIST ] = m_data_loops[ EVEN_QUAD ][ i ][ LOOP_RD_POS ];
	}
	// Odd quad
	for (unsigned i = 0; i < LOOPS_PER_QUAD; ++i) {
		m_odd_out[ ODD_OUT_LOOP80_POS - i * DATA_BIT_OUT_DIST ] = m_data_loops[ ODD_QUAD ][ i ][ LOOP_RD_POS ];
	}
}

void fsa_channel_device::bootloop_replicate()
{
	auto bl = m_bootloop[ LOOP_RD_POS ];
	m_even_out[ EVEN_OUT_BL_POS ] = bl;
}

void fsa_channel_device::bubble_swap()
{
	// Even quad
	for (unsigned i = 0; i < LOOPS_PER_QUAD; i++) {
		unsigned idx = EVEN_IN_LOOP80_POS - i * DATA_BIT_IN_DIST;
		auto tmp = m_data_loops[ EVEN_QUAD ][ i ][ LOOP_WR_POS ];
		m_data_loops[ EVEN_QUAD ][ i ][ LOOP_WR_POS ] = m_even_in[ idx ];
		m_even_in[ idx ] = tmp;
	}
	// Odd quad
	for (unsigned i = 0; i < LOOPS_PER_QUAD; i++) {
		unsigned idx = ODD_IN_LOOP80_POS - i * DATA_BIT_IN_DIST;
		auto tmp = m_data_loops[ ODD_QUAD ][ i ][ LOOP_WR_POS ];
		m_data_loops[ ODD_QUAD ][ i ][ LOOP_WR_POS ] = m_odd_in[ idx ];
		m_odd_in[ idx ] = tmp;
	}
}

void fsa_channel_device::bootloop_swap()
{
	auto tmp = m_bootloop[ LOOP_WR_POS ];
	m_bootloop[ LOOP_WR_POS ] = m_even_in[ EVEN_IN_BL_POS ];
	m_even_in[ EVEN_IN_BL_POS ] = tmp;
}

bool fsa_channel_device::errflg_r() const
{
	LOGFSA_IO("ERRFLG %d %02x\n", m_selected, m_status);
	return m_selected &&
		(BIT(m_status, STAT_UNCORRERR) != 0 ||
		 BIT(m_status, STAT_CORRERR) != 0 ||
		 BIT(m_status, STAT_TIMERR) != 0);
}

void fsa_channel_device::load_image(const std::vector<uint8_t>& img)
{
	if (img.size() < IMAGE_SIZE) {
		return;
	}

	auto it = img.cbegin();

	decode_loop(it, m_bootloop);
	for (unsigned i = 0; i < QUADS_PER_CH; i++) {
		for (unsigned j = 0; j < LOOPS_PER_QUAD; j++) {
			decode_loop(it, m_data_loops[ i ][ j ]);
		}
	}
	m_even_out.reset();
	m_odd_out.reset();
	m_even_in.reset();
	m_odd_in.reset();
}

// Default bootloop
// It comes from https://hpmuseum.net/images/98259A_BubbleMemory-50.jpg
// There are 138 good loops in each channel
static const uint8_t default_bl[ fsa_channel_device::BL_BOOTLOOP_BITS / 8 ] = {
	0xb7, 0xbb, 0xb3, 0xfb, 0x7f, 0xff, 0x37, 0x9f,
	0xfb, 0xff, 0xf7, 0x7f, 0xbd, 0x9b, 0xfb, 0xff,
	0xf5, 0xff, 0xfb, 0xff, 0xff, 0xff, 0x97, 0xff,
	0xbf, 0x5f, 0xbb, 0xdd, 0xef, 0xbf, 0xdf, 0xa5,
	0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void fsa_channel_device::create_image()
{
	unsigned idx = 0;
	for (unsigned i = 0; i < BL_PRE_SYNC_BITS; ++i) {
		m_bootloop[ idx++ ] = 1;
	}
	for (unsigned i = 0; i < BL_SYNC_BITS; ++i) {
		m_bootloop[ idx++ ] = 0;
	}
	for (unsigned i = 0; i < BL_PATTERN_BITS; ++i) {
		m_bootloop[ idx++ ] = (i & 1) == 0;
	}
	const uint8_t *bl = &default_bl[ 0 ];
	for (unsigned i = 0; i < BL_BOOTLOOP_BITS; i += 8) {
		uint8_t tmp = *bl++;
		for (unsigned j = 0; j < 8; j++) {
			m_bootloop[ idx++ ] = BIT(tmp, 7 - j);
		}
	}
	for (unsigned i = 0; i < BL_PAD_BITS; ++i) {
		m_bootloop[ idx++ ] = 0;
	}

	for (unsigned i = 0; i < QUADS_PER_CH; i++) {
		for (unsigned j = 0; j < LOOPS_PER_QUAD; j++) {
			m_data_loops[ i ][ j ].reset();
		}
	}
	m_even_out.reset();
	m_odd_out.reset();
	m_even_in.reset();
	m_odd_in.reset();
}

std::vector<uint8_t> fsa_channel_device::save_image() const
{
	std::vector<uint8_t> out;
	out.reserve(IMAGE_SIZE);

	encode_loop(m_bootloop, out);
	for (unsigned i = 0; i < QUADS_PER_CH; i++) {
		for (unsigned j = 0; j < LOOPS_PER_QUAD; j++) {
			encode_loop(m_data_loops[ i ][ j ], out);
		}
	}
	// Even though they are as non-volatile as data/boot loops, there's no need to save input/output tracks
	// They are overwritten at first read/write operation
	return out;
}

ALLOW_SAVE_TYPE(fsa_channel_device::fsa_cmd);
ALLOW_SAVE_TYPE(fsa_channel_device::ec_state);

void fsa_channel_device::device_start()
{
	save_item(NAME(m_selected));
	save_item(NAME(m_enable));
	save_item(NAME(m_data_out));
	save_item(NAME(m_status));
	save_item(NAME(m_blr_idx));
	save_item(NAME(m_fifo_in_idx));
	save_item(NAME(m_fifo_out_idx));
	save_item(NAME(m_code_accum));
	save_item(NAME(m_code_cnt));
	save_item(NAME(m_curr_cmd));
	save_item(NAME(m_ec_state));
}

void fsa_channel_device::device_reset()
{
	m_selected = false;
	m_enable = false;
	m_data_out = false;
	m_status = 0;
	reset_pointers();
	reset_error_corr();
	m_curr_cmd = fsa_cmd::FSA_CMD_NOP;
}

void fsa_channel_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, m_timer).configure_generic(FUNC(fsa_channel_device::timer_to));
}

TIMER_DEVICE_CALLBACK_MEMBER(fsa_channel_device::timer_to)
{
	if (m_curr_cmd == FSA_CMD_ICD) {
		// Scan all bits in FIFO and apply corrections
		do {
			bool bit = correct_one_bit();
			fifo_enqueue(bit);
		} while (m_fifo_in_idx != 0);

		if (m_ec_state == ec_state::EC_NO_ERROR) {
			LOG("ICD done, no error correction\n");
		} else if (m_ec_state == ec_state::EC_WAIT_TRAP) {
			LOG("ICD done, uncorrectable errors detected\n");
			BIT_SET(m_status, STAT_UNCORRERR);
		} else {
			LOG("ICD done, correctable error(s) detected & corrected\n");
			BIT_SET(m_status, STAT_CORRERR);
		}
		m_curr_cmd = FSA_CMD_NOP;
	} else {
		LOG("Unexpected cmd (%d) in timer_to\n", m_curr_cmd);
		m_curr_cmd = FSA_CMD_NOP;
	}
}

void fsa_channel_device::reset_pointers()
{
	m_blr_idx = 0;
	m_fifo_in_idx = 0;
	m_fifo_out_idx = 0;
	BIT_CLR(m_status, STAT_FIFOFL);
	BIT_SET(m_status, STAT_FIFOMT);
}

unsigned fsa_channel_device::get_blr_idx() const
{
	return ((m_blr_idx >> 1) & ~1U) | (m_blr_idx & 1);
}

void fsa_channel_device::clear_errors()
{
	BIT_CLR(m_status, STAT_UNCORRERR);
	BIT_CLR(m_status, STAT_CORRERR);
	BIT_CLR(m_status, STAT_TIMERR);
}

void fsa_channel_device::reset_error_corr()
{
	BIT_CLR(m_status, STAT_UNCORRERR);
	BIT_CLR(m_status, STAT_CORRERR);
	m_code_accum = 0;
	m_code_cnt = 0;
	m_ec_state = ec_state::EC_NO_ERROR;
}

void fsa_channel_device::set_enable(bool en)
{
	m_enable = en;
}

template<std::size_t N> void fsa_channel_device::rotate(std::bitset<N>& bits, bool loop)
{
	bool first = loop && bits[ 0 ];
	bits >>= 1;
	if (first) {
		bits.set(bits.size() - 1);
	}
}

bool fsa_channel_device::detector_r() const
{
	return m_even_out[ EVEN_OUT_DET_POS ] || m_odd_out[ ODD_OUT_DET_POS ];
}

void fsa_channel_device::generate(bool bit)
{
	m_even_in[ EVEN_IN_GEN_POS ] = bit;
	m_odd_in[ ODD_IN_GEN_POS ] = bit;
}

unsigned fsa_channel_device::fifo_size() const
{
	return BIT(m_status, STAT_ECF) ? EC_BITS : NO_EC_BITS;
}

void fsa_channel_device::fifo_enqueue(bool bit)
{
	if (BIT(m_status, STAT_FIFOFL)) {
		// FIFO overflow
		BIT_SET(m_status, STAT_TIMERR);
	} else {
		m_fifo[ m_fifo_in_idx ] = bit;
		m_fifo_in_idx++;
		if (m_fifo_in_idx >= fifo_size()) {
			m_fifo_in_idx = 0;
		}
		if (m_fifo_in_idx == m_fifo_out_idx) {
			BIT_SET(m_status, STAT_FIFOFL);
		}
		BIT_CLR(m_status, STAT_FIFOMT);
	}
}

bool fsa_channel_device::fifo_dequeue()
{
	bool res = false;

	if (BIT(m_status, STAT_FIFOMT)) {
		// FIFO underflow
		BIT_SET(m_status, STAT_TIMERR);
	} else {
		res = m_fifo[ m_fifo_out_idx ];
		m_fifo_out_idx++;
		if (m_fifo_out_idx >= fifo_size()) {
			m_fifo_out_idx = 0;
		}
		if (m_fifo_in_idx == m_fifo_out_idx) {
			BIT_SET(m_status, STAT_FIFOMT);
		}
		BIT_CLR(m_status, STAT_FIFOFL);
	}

	return res;
}

void fsa_channel_device::fire_code_enc(bool bit)
{
	if (bit ^ BIT(m_code_accum, FIRE_CODE_BITS - 1)) {
		m_code_accum = (m_code_accum << 1) ^ FIRE_POLY;
	} else {
		m_code_accum <<= 1;
	}
	m_code_accum &= FIRE_MASK;
}

void fsa_channel_device::fire_code_syn(bool bit)
{
	fire_code_enc(false);
	m_code_accum ^= bit;
}

void fsa_channel_device::pre_error_trapping()
{
  if (m_code_accum != 0 || m_ec_state != ec_state::EC_NO_ERROR) {
	m_ec_state = ec_state::EC_WAIT_TRAP;
	for (unsigned i = 0; i < FIRE_CODE_BITS; ++i) {
	  if ((m_code_accum & ERR_TRAP_MASK) == 0) {
		// Correctable error(s) in Fire code
		LOG("Error in check code @ %u, syn=%04x\n", i, m_code_accum);
		m_code_cnt = i + PAYLOAD_BITS + 1;
		m_ec_state = ec_state::EC_WAIT_TO_START;
		break;
	  } else {
		fire_code_syn(false);
	  }
	}
	for (unsigned i = 0; i < FIRE_CHOPPED_BITS; ++i) {
	  if (m_ec_state == ec_state::EC_WAIT_TRAP) {
		if ((m_code_accum & ERR_TRAP_MASK) == 0) {
		  m_ec_state = ec_state::EC_CORRECTING;
		} else {
		  fire_code_syn(false);
		}
	  }
	  if (m_ec_state == ec_state::EC_CORRECTING) {
		m_code_accum = (m_code_accum << 1) & FIRE_MASK;
		if (m_code_accum == 0) {
		  m_ec_state = ec_state::EC_DONE;
		}
	  }
	}
  }
}

bool fsa_channel_device::correct_one_bit()
{
	// m_fifo_out_idx = bits corrected so far
	if (m_ec_state == ec_state::EC_WAIT_TRAP && m_fifo_out_idx < PAYLOAD_BITS) {
		if ((m_code_accum & ERR_TRAP_MASK) == 0) {
			LOG("Trapped @ %u, syn=%04x\n", m_fifo_out_idx, m_code_accum);
			m_ec_state = ec_state::EC_CORRECTING;
		} else {
			fire_code_syn(false);
		}
	}
	if (m_ec_state == ec_state::EC_WAIT_TO_START && --m_code_cnt == 0) {
		m_ec_state = ec_state::EC_CORRECTING;
	}
	bool err;
	if (m_ec_state == ec_state::EC_CORRECTING) {
		err = BIT(m_code_accum, FIRE_CODE_BITS - 1);
		m_code_accum = (m_code_accum << 1) & FIRE_MASK;
		if (m_code_accum == 0) {
			m_ec_state = ec_state::EC_DONE;
		}
	} else {
		err = false;
	}
	return fifo_dequeue() ^ err;
}

void fsa_channel_device::encode_loop(const std::bitset<BITS_PER_LOOP>& loop, std::vector<uint8_t>& out)
{
	for (unsigned i = 0; i < BITS_PER_LOOP; i += 8) {
		uint8_t tmp = 0;
		for (unsigned j = 0; j < 8; j++) {
			if (loop[ i + j ]) {
				BIT_SET(tmp, j);
			}
		}
		out.push_back(tmp);
	}
}

void fsa_channel_device::decode_loop(std::vector<uint8_t>::const_iterator& in, std::bitset<BITS_PER_LOOP>& loop)
{
	for (unsigned i = 0; i < BITS_PER_LOOP; i += 8) {
		uint8_t tmp = *in++;
		for (unsigned j = 0; j < 8; j++) {
			loop[ i + j ] = BIT(tmp, j);
		}
	}
}

// +----------------+
// | ibubble_device |
// +----------------+
ibubble_device::ibubble_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IBUBBLE, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_chA(*this, "cha")
	, m_chB(*this, "chb")
	, select_w_cb(*this)
	, dio_w_cb(*this)
	, dio_r_cb(*this, 0)
	, status_r_cb(*this, 0)
	, errflg_r_cb(*this, 0)
	, m_cs(0)
	, m_select_a(false)
	, m_select_b(false)
	, m_dirty(false)
{
}

void ibubble_device::field_rotate(int state)
{
	// "Enable A" enables rotation of whole memory
	if (m_chA->enable_r()) {
		m_chA->field_rotate();
		m_chB->field_rotate();
	}
}

void ibubble_device::select_w(uint16_t data)
{
	m_select_a = BIT(data, 0);
	m_select_b = BIT(data, 1);
	select_w_cb(data >> 2);
}

void ibubble_device::cmd_w(uint8_t data)
{
	auto cmd = static_cast<fsa_channel_device::fsa_cmd>(data);
	if (m_cs == 0) {
		m_chA->cmd_w(m_select_a, cmd);
		m_chB->cmd_w(m_select_b, cmd);
	} else {
		m_chA->cmd_w(false, cmd);
		m_chB->cmd_w(false, cmd);
	}
}

void ibubble_device::dio_w(uint16_t data)
{
	m_chA->dio_w(BIT(data, 0));
	m_chB->dio_w(BIT(data, 1));
	dio_w_cb(data >> 2);
}

uint16_t ibubble_device::dio_r()
{
	uint16_t res = dio_r_cb() << 2;

	if (m_chA->dio_r()) {
		BIT_SET(res, 0);
	}
	if (m_chB->dio_r()) {
		BIT_SET(res, 1);
	}
	return res;
}

uint8_t ibubble_device::status_r()
{
	uint8_t res = status_r_cb();

	// No more than 1 channel should be selected or DIO bus contention occurs
	res |= m_chA->status_r();
	res |= m_chB->status_r();

	return res;
}

void ibubble_device::shiftclk(int state)
{
	m_chA->shiftclk();
	m_chB->shiftclk();
}

void ibubble_device::bubble_replicate(int state)
{
	if (m_chA->enable_r()) {
		m_chA->bubble_replicate();
		m_chB->bubble_replicate();
	}
}

void ibubble_device::bootloop_replicate(int state)
{
	if (m_chA->enable_r()) {
		m_chA->bootloop_replicate();
		m_chB->bootloop_replicate();
	}
}

void ibubble_device::bubble_swap(int state)
{
	if (m_chA->enable_r()) {
		m_chA->bubble_swap();
		m_chB->bubble_swap();
		m_dirty = true;
	}
}

void ibubble_device::bootloop_swap(int state)
{
	if (m_chA->enable_r()) {
		m_chA->bootloop_swap();
		m_chB->bootloop_swap();
		m_dirty = true;
	}
}

int ibubble_device::errflg_r()
{
	// ERRFLG is an open-drain signal
	int res = errflg_r_cb();
	if (m_chA->errflg_r()) {
		res = 1;
	}
	if (m_chB->errflg_r()) {
		res = 1;
	}

	return res;
}

void ibubble_device::cs_w(int state)
{
	m_cs = state;
}

std::pair<std::error_condition, std::string> ibubble_device::call_load()
{
	if (length() < (fsa_channel_device::IMAGE_SIZE * 2)) {
		return std::make_pair(std::error_condition(image_error::INVALIDLENGTH), std::string("Image too short"));
	}

	std::vector<uint8_t> buff(fsa_channel_device::IMAGE_SIZE);

	if (fread(buff.data(), fsa_channel_device::IMAGE_SIZE) != fsa_channel_device::IMAGE_SIZE) {
		return std::make_pair(std::errc::io_error, std::string("Error reading file"));
	}
	m_chA->load_image(buff);

	if (fread(buff.data(), fsa_channel_device::IMAGE_SIZE) != fsa_channel_device::IMAGE_SIZE) {
		return std::make_pair(std::errc::io_error, std::string("Error reading file"));
	}
	m_chB->load_image(buff);

	m_dirty = false;

	return std::make_pair(std::error_condition(), std::string());
}

std::pair<std::error_condition, std::string> ibubble_device::call_create(int format_type, util::option_resolution *format_options)
{
	m_chA->create_image();
	m_chB->create_image();

	m_dirty = true;

	return std::make_pair(std::error_condition(), std::string());
}

void ibubble_device::call_unload()
{
	if (m_dirty) {
		fseek(0, SEEK_SET);

		{
			auto tmp = m_chA->save_image();
			fwrite(tmp.data(), fsa_channel_device::IMAGE_SIZE);
		}
		{
			auto tmp = m_chB->save_image();
			fwrite(tmp.data(), fsa_channel_device::IMAGE_SIZE);
		}
	}
}

void ibubble_device::device_start()
{
	save_item(NAME(m_cs));
	save_item(NAME(m_select_a));
	save_item(NAME(m_select_b));
	save_item(NAME(m_dirty));
}

void ibubble_device::device_reset()
{
	m_cs = 0;
	m_select_a = false;
	m_select_b = false;
}

void ibubble_device::device_add_mconfig(machine_config &config)
{
	FSA_CHANNEL(config, m_chA, clock());
	FSA_CHANNEL(config, m_chB, clock());
}

//
// **** Pseudo-code of BMC commands ****
//
// 4 clocks = 1 μs
// 1 field rotation = 80 clocks = 20 μs
//
// fn cmd_start()
//   BIT_CLR(m_str, STAT_CORRERR)
//   BIT_CLR(m_str, STAT_UNCORRERR)
//   BIT_CLR(m_str, STAT_TIMERR)
//   BIT_CLR(m_str, STAT_OPFAIL)
//   BIT_CLR(m_str, STAT_OPCOMPLETE)
//   BIT_SET(m_str, STAT_BUSY)
//   clr_irq()
// fn leave()
//   if in_a_subcmd() && !BIT(m_str, STAT_OPFAIL)
//     ret_subcmd()
//   else
//     if interrupt needs to be set
//       set_irq()
//     BIT_CLR(m_str, STAT_BUSY)
//     exit
// fn set_op_complete()
//   if !in_a_subcmd()
//     BIT_SET(m_str, STAT_OPCOMPLETE);
// fn set_op_complete_n_leave()
//   set_op_complete()
//   leave()
// fn set_op_fail()
//   BIT_CLR(m_str, STAT_OPCOMPLETE)
//   BIT_SET(m_str, STAT_OPFAIL)
// fn set_op_fail_n_leave()
//   set_op_fail()
//   leave()
// fn cond_assert(bool c)
//   if !c
//     set_op_fail_n_leave()
// **********************************
// * Write bootloop register masked *
// **********************************
//   # 2 FSA channels
//   # 900 μs duration
//   @S0
//   cond_assert(initialized)
//   cond_assert(nfc==1)
//   cnt1 = ec_enabled() ? 135 : 136
//   cnt2 = cnt1
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_WRITE_BLR) + delay @S1
//   for _ in range(160)
//     bitA, bitB = fifo_dequeue_bits(2)
//     if cnt1 && bitA
//       cnt1--
//     else
//       bitA = 0
//     if cnt2 && bitB
//       cnt2--
//     else
//       bitB = 0
//     tmp2 = ((bitB << 1) | bitA) << fsa_1st_ch(nfc, mbm_select)
//     dio_w(tmp2)
//     delay @S2
//   Final delay to 3600 clocks @S3
//   set_op_complete_n_leave()
// **************
// * Initialize *
// **************
//   # 1..8 FSAs, 2 channels each
//   # Best case duration: (350 + 85200 N) μs
//   # Worst case duration: (350 + 164740 N) μs
//   # N = # of FSAs
//   @S0
//   sub command: Abort @S1
//   sub command: MBM Purge @S2
//   sub command: Reset FIFO @S3
//   cond_assert(nfc==1)
//   for mbm_select in range(mbm_select + 1)
//     @S4
//     sub command: read bootloop @S5
//     sub command: write bootloop register masked @S6
//   set_op_complete_n_leave()
// ********************
// * Read bubble data *
// ********************
//   # 2/4/8/16 FSA channels (technically it's also possible to read from a single channel)
//   # Channels must be <= num_mbm * 2
//   # Execution time: it's complicated.. from AP157:
//   # Single page (MFBTR=0): tseek + 8690 μs
//   # Single page (MFBTR=1): tseek + 12770 μs
//   # N pages (MFBTR=0): tseek + 8690 μs + 7500(N-1) μs
//   # N pages (MFBTR=1): tseek + 12770 μs + 7500(N-1) μs
//   # (With the assumption that no error correction takes place)
//   @S0
//   cond_assert(initialized)
//   @S1
//   read_mbm_changed:
//   mbm_select_changed = true
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S2
//   cmd_w(ec_enabled() ? selected_mbms : 0, FSA_CMD_SET_EC) + delay @S3
//   cmd_w(selected_mbms, FSA_CMD_READ_DATA) + delay
//   read_loop: @S4
//   read_seek() @S5
//   bubble_replicate()
//   field_rotate() + delay @S6
//   field_rotate() + delay @S7
//   bubble_replicate()
//   rotate(38) @S8
//   for cnt1 in range(320)
//     if !mbm_select_changed && cnt1 < ec_enabled() ? 270 : 272
//       w = dio_r()
//       if cnt1 < ec_enabled() ? 256 : 272
//         fifo_enqueue_bits(fsa_n_channels(nfc), w >> fsa_1st_ch(nfc, mbm_select))
//     shiftclk()
//     field_rotate() + delay @S9
//   rotate(5)
//   n_pages = (n_pages - 1) & 0x7ff
//   if ec_enabled() && errflg_r()
//     if BIT(m_en, EN_RCD)
//       # Level 1 error correction
//       cmd_w(selected_mbms, FSA_CMD_RCD) + delay @S11
//       fsa_to_bmc_xfer() @S12
//       read_ch_status() @S13
//       if !BIT(m_str, STAT_UNCORRERR)
//         ar = (ar + 1) & 0x7fff
//     else
//       # Level 2 & 3 error correction
//       cmd_w(selected_mbms, FSA_CMD_ICD) + delay @S14
//       while !errflg_r()
//         delay  @S14
//       read_ch_status() @S15
//       if !BIT(m_str, STAT_UNCORRERR)
//         # Level 3
//         if BIT(m_en, EN_INT_ERR)
//           leave_read_bubble()
//         else
//           cmd_w(selected_mbms, FSA_CMD_RCD) + delay @S16
//           fsa_to_bmc_xfer() @S17
//           ar = (ar + 1) & 0x7fff
//     if n_pages == 0 @S18
//       if BIT(m_en, EN_RCD) || !BIT(m_str, STAT_UNCORRERR)
//         set_op_complete()
//       else
//         set_op_fail()
//       leave_read_bubble()
//     else if BIT(m_str, STAT_UNCORRERR)
//       set_op_fail()
//       leave_read_bubble()
//     else
//       goto read_mbm_changed
//   else
//     ar = (ar + 1) & 0x7fff
//     mbm_select_changed = selected_mbms != fsa_ch_mask(nfc,mbm_select)
//     if n_pages == 0
//       set_op_complete()
//       fsa_to_bmc_xfer()
//       leave_read_bubble()
//     else if mbm_select_changed
//       fsa_to_bmc_xfer() @S19
//       cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S1
//       goto read_mbm_changed
//     else
//       delay 320 clocks @S4
//       goto read_loop
//   fn read_seek()
//     @S0
//     first_mbm = fsa_1st_mbm(nfc, mbm_select)
//     while ar != m_la[ first_mbm ] || m_skip[ first_mbm ] != 0
//       field_rotate() + delay @S0
//   fn read_ch_status()
//     @S0
//     cmd_w(selected_mbms, FSA_CMD_READ_ERRFLG) + delay @S1
//     w = dio_r()
//     for each channel in w @S2
//       cmd_w(channel, FSA_CMD_READ_STATUS) + delay @S3
//       st = status_r()
//       if BIT(st, STAT_UNCORRERR)
//         BIT_SET(m_str, STAT_UNCORRERR)
//       if BIT(st, STAT_CORRERR)
//         BIT_SET(m_str, STAT_CORRERR)
//   fn fsa_to_bmc_xfer()
//     @S0
//     for cnt2 in range(ec_enabled() ? 270 : 272)
//        w = dio_r()
//        if cnt2 < ec_enabled() ? 256 : 272
//          fifo_enqueue_bits(fsa_n_channels(nfc), w >> fsa_1st_ch(nfc, mbm_select))
//        delay BIT(m_en, EN_MFBTR) ? 80 clocks : 20 clocks @S1
//   fn leave_read_bubble()
//     @S20
//     cmd_w(selected_mbms, FSA_CMD_NOP) + delay @S21
//     leave()
//   fn rotate(cnt1)
//     @S0
//     for _ in range(cnt1)
//       field_rotate() + delay @S0
// *********************
// * Write bubble data *
// *********************
//   # 2/4/8/16 FSA channels
//   # Channels must be <= num_mbm * 2
//   # Execution time:
//   # Single page: tseek + 7450 μs
//   # N pages: tseek + 7450 μs + 7500(N-1) μs
//   @S0
//   cond_assert(initialized)
//   @S1
//   write_mbm_changed:
//   mbm_select_changed = true
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S2
//   cmd_w(ec_enabled() ? selected_mbms : 0, FSA_CMD_SET_EC) + delay @S3
//   cmd_w(selected_mbms, FSA_CMD_WRITE_DATA) + delay
//   write_loop: @S4
//   write_seek() @S5
//   if mbm_select_changed
//     do
//       delay @S6
//     while FIFO has less than 2 bytes
//   w = fifo_dequeue_bits(fsa_n_channels(nfc)) @S7
//   dio_w(w << fsa_1st_ch(nfc, mbm_select))
//   delay @S8
//   for cnt2 in range(320)
//     if ec_enabled()
//       if cnt2 < 256-1
//         w = fifo_dequeue_bits(fsa_n_channels(nfc))
//         dio_w(w << fsa_1st_ch(nfc, mbm_select))
//       else if cnt2 < 270-1
//         dio_w(0)
//     else if cnt2 < 272-1
//       w = fifo_dequeue_bits(fsa_n_channels(nfc))
//       dio_w(w << fsa_1st_ch(nfc, mbm_select))
//     shiftclk()
//     field_rotate() + delay @S8
//   rotate(20) @S9
//   bubble_swap()
//   rotate(2) @S10
//   bubble_swap()
//   rotate(23) @S11
//   n_pages = (n_pages - 1) & 0x7ff
//   ar = (ar + 1) & 0x7fff
//   mbm_select_changed = selected_mbms != fsa_ch_mask(nfc,mbm_select)
//   if n_pages == 0
//     cmd_w(selected_mbms, FSA_CMD_NOP) + delay @S12
//     set_op_complete_n_leave()
//   else if mbm_select_changed
//     cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S1
//     goto write_mbm_changed
//   else
//     goto write_loop
//   fn write_seek()
//     @S0
//     first_mbm = fsa_1st_mbm(nfc, mbm_select)
//     while (ar + 0x706) & 0x7ff != m_la[ first_mbm ] || m_skip[ first_mbm ] != 0
//       field_rotate() + delay @S0
// *************
// * Read seek *
// *************
//   # 2/4/8/16 FSA channels
//   # Execution time:
//   # Best case: 7350 μs
//   # Worst case: 89250 μs
//   @S0
//   cond_assert(initialized)
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_SET_ENABLE) + delay @S1
//   read_seek() @S2
//   rotate(BIT(ar, 0) ? 366 : 364) @S3
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S4
//   set_op_complete_n_leave()
// **************************
// * Read bootloop register *
// **************************
//   # 1/2/4/8/16 FSA channels (2 typical)
//   # Execution time: 900 μs
//   @S0
//   cond_assert(initialized)
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_READ_BLR) + delay @S1
//   for _ in range(160)
//     w = dio_r()
//     fifo_enqueue_bits(fsa_n_channels(nfc), w >> fsa_1st_ch(nfc,mbm_select))
//     delay 20 clks @S1
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S2
//   set_op_complete_n_leave()
// ***************************
// * Write bootloop register *
// ***************************
//   # 1/2/4/8/16 FSA channels (2 typical)
//   # Execution time: 900 μs
//   @S0
//   cond_assert(initialized)
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_WRITE_BLR) + delay @S1
//   for _ in range(160)
//     w = fifo_dequeue_bits(fsa_n_channels(nfc))
//     dio_w(w << fsa_1st_ch(nfc,mbm_select))
//     delay 20 clks @S1
//   # Extract and discard dummy byte at end of FIFO
//   fifo_dequeue_bits(8)
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S2
//   set_op_complete_n_leave()
// ******************
// * Write bootloop *
// ******************
//   # 2 FSA channels
//   # Execution time: 82850 μs
//   @S0
//   cond_assert(initialized)
//   cond_assert(nfc==1)
//   if !BIT(m_en, EN_WR_BL)
//     BIT_SET(m_str, STAT_TIMERR)
//     set_op_fail_n_leave()
//   select_chs()
//   # This is used to turn error correction off
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S1
//   cmd_w(selected_mbms, FSA_CMD_WRITE_DATA) + delay @S2
//   for _ in range(18)
//     dio_w(0xffff)
//     shiftclk()
//     field_rotate() + delay @S3
//   for _ in range(3140)
//     bootloop_swap()
//     dio_w(0xffff)
//     shiftclk()
//     field_rotate() + delay @S4
//   for _ in range(242)
//     bootloop_swap()
//     dio_w(0)
//     shiftclk()
//     field_rotate() + delay @S5
//   for cnt1 in range(14)
//     bootloop_swap()
//     dio_w(BIT(cnt1, 0) ? 0 : 0xffff)
//     shiftclk()
//     field_rotate() + delay @S6
//   for cnt1 in range(320)
//     bootloop_swap()
//     b = fifo_dequeue_bits(1)
//     dio_w(b ? 0xffff : 0)
//     shiftclk()
//     field_rotate() + delay @S7
//   for _ in range(362)
//     bootloop_swap()
//     dio_w(0)
//     shiftclk()
//     field_rotate() + delay @S8
//   for _ in range(18)
//     bootloop_swap()
//     field_rotate() + delay @S9
//   # Extract and discard dummy byte at end of FIFO
//   fifo_dequeue_bits(8)
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S10
//   initialized = false
//   set_op_complete_n_leave()
// *******************
// * Read FSA status *
// *******************
//   # Number of FSAs (N) = num_mbm
//   # Execution time: 75 μs + 40 (N-1) μs
//   @S0
//   cond_assert(initialized)
//   for i in range(num_mbm) @S1
//     cmd_w(fsa_1ch_mask(i*2), FSA_CMD_READ_STATUS) + delay 80 clks @S2
//     b = status_r()
//     fifo_enqueue_byte(b)
//     cmd_w(fsa_1ch_mask(i*2+1), FSA_CMD_READ_STATUS) + delay 80 clks @S3
//     b = status_r()
//     fifo_enqueue_byte(b)
//   delay 140 clks @S4
//   set_op_complete_n_leave()
// *********
// * Abort *
// *********
//   # Number of FSAs (N) = num_mbm
//   # (100 + 40 (N-1)) μs duration
//   @S0
//   if aborting a running command
//     for i in range(num_mbm) @S1
//       cmd_w(fsa_1ch_mask(i*2), FSA_CMD_NOP) + delay 40 clks @S2
//       cmd_w(fsa_1ch_mask(i*2), FSA_CMD_READ_STATUS) + delay 40 clks @S3
//       status_r()
//       cmd_w(fsa_1ch_mask(i*2+1), FSA_CMD_NOP) + delay 40 clks @S4
//       cmd_w(fsa_1ch_mask(i*2+1), FSA_CMD_READ_STATUS) + delay 40 clks @S5
//       status_r()
//     initialized = false
//   clear FIFO & status @S6
//   delay 240 clks @S7
//   set_op_complete_n_leave()
// **************
// * Write seek *
// **************
//   # 2/4/8/16 FSA channels
//   # Execution time:
//   # Best case: 7350 μs
//   # Worst case: 89250 μs
//   @S0
//   cond_assert(initialized)
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_SET_ENABLE) + delay @S1
//   write_seek() @S2
//   for _ in range(BIT(ar, 0) ? 366 : 364)
//     field_rotate() + delay @S3
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S4
//   set_op_complete_n_leave()
// *****************
// * Read bootloop *
// *****************
//   # 1 FSA channel (BL is identical in the 2 halves/channels of a FSA)
//   # Best case duration: 86000 μs
//   # Worst case duration: 165000 μs
//   @S0
//   cond_assert(initialized)
//   cond_assert(nfc==1)
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_SWRESET) + delay @S1
//   cmd_w(selected_mbms, FSA_CMD_INIT) + delay @S2
//   for _ in range(362)
//     bootloop_replicate()
//     field_rotate() + delay @S2
//   # Max rotations
//   cnt1 = 7829
//   # Min rotations
//   cnt2 = 3937
//   state = 0
//   loop
//     shiftclk()
//     bit = BIT(dio_r(), fsa_1st_ch(nfc, mbm_select))
//     switch state
//       case 0: @S3
//         if bit
//           cnt3 = 3158
//           state = 1
//       case 1: @S4
//         if bit
//           if --cnt3 == 0
//             state = 2
//         else
//           state = 0
//       case 2: @S5
//         if !bit
//           cnt3 = 256
//           state = 3
//       case 3: @S6
//         exp_bit = cnt3 > 14 ? 0 : !BIT(cnt3, 0)
//         if bit == exp_bit
//           if --cnt3 == 0
//             cnt3 = 320
//             state = 4
//         else
//           state = 0
//       case 4: @S7
//         fifo_enqueue_bits(1, bit)
//         if --cnt3 == 0
//           mbm = fsa_1st_mbm(nfc, mbm_select)
//           m_la[ mbm ] = 0
//           m_skip[ mbm ] = 0
//           while cnt2 > 0
//             delay 80 clocks @S8
//             cnt2--
//           set_op_complete_n_leave()
//     if --cnt1 == 0
//       initialized = false
//       BIT_SET(m_str, STAT_TIMERR)
//       set_op_fail_n_leave()
//     if cnt2 > 0
//       cnt2--
//     bootloop_replicate()
//     field_rotate() + delay
// ***********************
// * Read corrected data *
// ***********************
//   # 1/2/4/8/16 FSA channels
//   # Execution time: 1400 μs
//   @S0
//   cond_assert(initialized)
//   cond_assert(BIT(m_en, EN_ICD))
//   select_chs()
//   cmd_w(selected_mbms, FSA_CMD_RCD) + delay @S1
//   for cnt1 in range(270)
//      w = dio_r()
//      if cnt1 < 256
//        fifo_enqueue_bits(fsa_n_channels(nfc), w >> fsa_1st_ch(nfc, mbm_select))
//      delay 20 clocks @S2
//   delay @S3
//   set_op_complete_n_leave()
// **************
// * Reset FIFO *
// **************
//   # 50 μs duration
//   @S0
//   clear FIFO
//   delay 200 clks @S1
//   set_op_complete_n_leave()
// *************
// * MBM purge *
// *************
//   # 150 μs duration
//   @S0
//   initialized = true
//   clear AR
//   clear LA & skip
//   cmd_w(0xffff, FSA_CMD_SWRESET) + delay 40 @S1
//   num_mbm = 0
//   for cnt1 in range(8)
//     cmd_w(fsa_1ch_mask(cnt1*2), FSA_CMD_READ_STATUS) + delay 40 @S2
//     w = status_r()
//     if BIT(w, STAT_FIFOMT)
//       num_mbm++
//   delay 240 clks @S3
//   set_op_complete_n_leave()
// ******************
// * Software reset *
// ******************
//   # 50 μs duration
//   @S0
//   cond_assert(initialized)
//   clear AR
//   clear n_pages
//   cmd_w(0xffff, FSA_CMD_SWRESET)
//   delay 200 clks @S1
//   set_op_complete_n_leave()
//

// +----------------+
// | i7220_1_device |
// +----------------+
i7220_1_device::i7220_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, I7220_1, tag, owner, clock)
	, intrq_cb(*this)
	, drq_cb(*this)
	, field_rotate_cb(*this)
	, select_w_cb(*this)
	, cmd_w_cb(*this)
	, dio_w_cb(*this)
	, dio_r_cb(*this, 0)
	, status_r_cb(*this, 0)
	, shiftclk_cb(*this)
	, bubble_replicate_cb(*this)
	, bootloop_replicate_cb(*this)
	, bubble_swap_cb(*this)
	, bootloop_swap_cb(*this)
	, errflg_r_cb(*this, 0)
	, m_timer(*this, "tmr")
{
}

uint8_t i7220_1_device::read(offs_t offset)
{
	uint8_t res = 0;

	if ((offset & 1) == 0) {
		// Register addressed by RAC
		switch (m_rac) {
		case REG_IDX_FIFO:
			if (!m_fifo.empty()) {
				res = m_fifo.dequeue();
				update_drq();
			}
			break;

		case REG_IDX_UR:
			res = m_ur;
			break;

		case REG_IDX_AR_LSB:
			res = uint8_t(m_ar);
			break;

		case REG_IDX_AR_MSB:
			res = uint8_t(m_ar >> 8);
			break;
		}

		LOGREG("Rd reg=%02x RAC=%u\n", res, m_rac);
		// Increment RAC
		if (m_rac != REG_IDX_FIFO) {
			m_rac = (m_rac + 1) & RAC_MASK;
		}
	} else {
		// Status register
		if (get_busy_state()) {
			BIT_SET(res, STAT_BUSY);
		}
		if (!is_executing()) {
			res |= m_str;
		}

		if (m_rac != REG_IDX_FIFO ||
			((!is_executing() || m_read_cmd) && !m_fifo.empty()) ||
			(is_executing() && !m_read_cmd && !m_fifo.full())) {
			BIT_SET(res, STAT_FIFO_RDY);
		}
		LOGREG("Rd sts=%02x\n", res);
	}

	return res;
}

void i7220_1_device::write(offs_t offset, uint8_t data)
{
	if ((offset & 1) == 0) {
		// Register addressed by RAC
		LOGREG("Wr reg=%02x RAC=%u\n", data, m_rac);
		switch (m_rac) {
		case REG_IDX_FIFO:
			if (!m_fifo.full()) {
				m_fifo.enqueue(data);
				update_drq();
			}
			break;

		case REG_IDX_UR:
			m_ur = data;
			break;

		case REG_IDX_BLR_LSB:
			m_blr = (m_blr & 0xff00) | data;
			break;

		case REG_IDX_BLR_MSB:
			m_blr = (uint16_t(data) << 8) | (m_blr & 0x00ff);
			break;

		case REG_IDX_ER:
			m_en = data;
			break;

		case REG_IDX_AR_LSB:
			m_ar = (m_ar & 0xff00) | data;
			break;

		case REG_IDX_AR_MSB:
			m_ar = (uint16_t(data) << 8) | (m_ar & 0x00ff);
			break;
		}

		// Increment RAC
		if (m_rac != REG_IDX_FIFO) {
			m_rac = (m_rac + 1) & RAC_MASK;
		}
	} else {
		// Command/RAC register
		LOGREG("Wr cmd/rac=%02x\n", data);
		if (BIT(data, 4)) {
			cmd_start(data & CMD_MASK);
		} else {
			uint8_t new_rac = data & RAC_MASK;
			// Check RAC validity
			if (new_rac == REG_IDX_FIFO ||
				new_rac >= REG_IDX_UR) {
				m_rac = new_rac;
				if (BIT(data, 5)) {
					// Shouldn't inhibit it during cmd execution?
					clr_status();
				}
			}
		}
	}
}

ALLOW_SAVE_TYPE(i7220_1_device::fsm_state);
ALLOW_SAVE_TYPE(i7220_1_device::cmds);

void i7220_1_device::device_start()
{
	save_item(NAME(m_initialized));
	save_item(NAME(m_drq));
	save_item(NAME(m_irq));
	save_item(NAME(m_mbm_select_changed));
	save_item(NAME(m_aborted));
	save_item(NAME(m_read_cmd));
	save_item(NAME(m_rac));
	save_item(NAME(m_cmdr));
	save_item(NAME(m_str));
	save_item(NAME(m_en));
	save_item(NAME(m_ur));
	save_item(NAME(m_fifo_accum));
	save_item(NAME(m_fifo_mask));
	save_item(NAME(m_blr));
	save_item(NAME(m_cnt1));
	save_item(NAME(m_cnt2));
	save_item(NAME(m_cnt3));
	save_item(NAME(m_cnt4));
	save_item(NAME(m_ar));
	save_item(NAME(m_selected_chs));
	save_item(NAME(m_la));
	save_item(NAME(m_skip));
	save_item(NAME(m_mbm_count));
	save_item(NAME(m_fsm_state));
	save_item(NAME(m_ret_state));
	save_item(NAME(m_sub_cmd));
}

void i7220_1_device::device_reset()
{
	m_initialized = false;
	m_drq = false;
	m_irq = false;
	m_read_cmd = false;
	m_rac = 0;
	m_str = 0;
	m_en = 0;
	m_blr = 0;
	m_ar = 0;
	m_fsm_state = IDLE;
	m_ret_state = IDLE;
	m_fifo.clear();
	intrq_cb(0);
	drq_cb(0);
}

void i7220_1_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, m_timer).configure_generic(FUNC(i7220_1_device::timer_to));
}

TIMER_DEVICE_CALLBACK_MEMBER(i7220_1_device::timer_to)
{
	run_fsm();
}

void i7220_1_device::field_rotate(fsm_state new_state)
{
	LOGFSA_IO("ROT %s\n", machine().time().as_string(6));
	field_rotate_cb(1);
	for (unsigned i = 0; i < MAX_MBM; i++) {
		// Update logical address
		if (m_selected_chs & fsa_ch_mask(2, i)) {
			LOGFSM("ROT bfr %u la %03x sk %u\n", i, m_la[ i ], m_skip[ i ]);
			if (m_skip[ i ] == 0) {
				if (BIT(m_la[ i ], 0)) {
					m_skip[ i ] = 1;
				} else {
					m_la[ i ] = (m_la[ i ] + LA_DIST) & LA_MASK;
				}
			} else if (m_skip[ i ] == 1) {
				m_skip[ i ]++;
			} else {
				m_skip[ i ] = 0;
				m_la[ i ] = (m_la[ i ] + LA_DIST) & LA_MASK;
			}
			LOGFSM("ROT aft %u la %03x sk %u\n", i, m_la[ i ], m_skip[ i ]);
		}
	}
	delay(CLKS_ROTATE, new_state);
}

void i7220_1_device::cmd_w(uint16_t mask, fsa_channel_device::fsa_cmd cmd)
{
	LOGFSA_IO("CMD %x %04x\n", unsigned(cmd), mask);
	select_w_cb(mask);
	cmd_w_cb(uint8_t(cmd));
}

void i7220_1_device::dio_w(uint16_t data)
{
	LOGFSA_IO("DIOW %04x\n", data);
	// Mapping:
	// b15  MBM #7, ch B
	// b14  MBM #7, ch A
	// ...
	// b1   MBM #0, ch B
	// b0   MBM #0, ch A
	dio_w_cb(data);
}

uint16_t i7220_1_device::dio_r()
{
	// Mapping: see dio_w
	auto res = dio_r_cb();
	LOGFSA_IO("DIOR %04x\n", res);
	return res;
}

uint8_t i7220_1_device::status_r()
{
	auto res = status_r_cb();
	LOGFSA_IO("STSR %02x\n", res);
	return res;
}

void i7220_1_device::shiftclk()
{
	LOGFSA_IO("SHIFTCLK\n");
	shiftclk_cb(1);
}

void i7220_1_device::bubble_replicate()
{
	LOGFSA_IO("BUBREP\n");
	bubble_replicate_cb(1);
}

void i7220_1_device::bootloop_replicate()
{
	LOGFSA_IO("BLREP\n");
	bootloop_replicate_cb(1);
}

void i7220_1_device::bubble_swap()
{
	LOGFSA_IO("BUBSWAP\n");
	bubble_swap_cb(1);
}

void i7220_1_device::bootloop_swap()
{
	LOGFSA_IO("BLSWAP\n");
	bootloop_swap_cb(1);
}

bool i7220_1_device::errflg_r()
{
	auto res = errflg_r_cb() != 0;
	LOGFSA_IO("ERRFLG %d\n", res);
	return res;
}

bool i7220_1_device::ec_enabled() const
{
	return BIT(m_en, EN_RCD) || BIT(m_en, EN_ICD);
}

unsigned i7220_1_device::bits_per_xfer() const
{
	return ec_enabled() ? fsa_channel_device::EC_BITS : fsa_channel_device::NO_EC_BITS;
}

void i7220_1_device::clr_irq()
{
	if (m_irq) {
		LOGIRQ("IRQ 0\n");
		m_irq = false;
		intrq_cb(0);
	}
}

void i7220_1_device::set_irq()
{
	if (!m_irq) {
		LOGIRQ("IRQ 1\n");
		m_irq = true;
		intrq_cb(1);
	}
}

void i7220_1_device::update_drq()
{
	bool new_drq;

	if (get_busy_state()) {
		if (BIT(m_en, EN_DMA)) {
			if (m_read_cmd) {
				new_drq = !m_fifo.empty();
			} else {
				new_drq = !m_fifo.full();
			}
		} else {
			if (m_read_cmd) {
				new_drq = m_fifo.queue_length() >= 22;
			} else {
				new_drq = m_fifo.queue_length() <= 20;
			}
		}
	} else {
		new_drq = false;
	}
	if (new_drq != m_drq) {
		m_drq = new_drq;
		LOGDRQ("DRQ %d\n", m_drq);
		drq_cb(m_drq);
	}
}

void i7220_1_device::clr_status()
{
	BIT_CLR(m_str, STAT_PARERR);
	BIT_CLR(m_str, STAT_UNCORRERR);
	BIT_CLR(m_str, STAT_CORRERR);
	BIT_CLR(m_str, STAT_TIMERR);
	BIT_CLR(m_str, STAT_OPFAIL);
	BIT_CLR(m_str, STAT_OPCOMPLETE);
	clr_irq();
}

bool i7220_1_device::get_busy_state() const
{
	return is_executing() ||
		(m_read_cmd && m_drq);
}

bool i7220_1_device::is_executing() const
{
	return m_fsm_state != IDLE;
}

void i7220_1_device::cmd_start(uint8_t new_cmd)
{
	if (is_executing()) {
		if (new_cmd == bmc_cmds::CMD_ABORT) {
			m_timer->reset();
			m_aborted = true;
		} else {
			LOGCMD("Cmd %x ignored!\n", new_cmd);
			return;
		}
	} else {
		m_aborted = false;
	}
	LOGCMD("Cmd started %x ABRT=%d @%s\n", new_cmd, m_aborted, machine().time().as_string(6));
	m_cmdr = new_cmd;
	clr_status();
	m_fsm_state = S0;
	m_ret_state = IDLE;
	m_read_cmd = m_cmdr == bmc_cmds::CMD_INITIALIZE ||
		m_cmdr == bmc_cmds::CMD_RD_DATA ||
		m_cmdr == bmc_cmds::CMD_RD_BLR ||
		m_cmdr == bmc_cmds::CMD_RD_FSA_STAT ||
		m_cmdr == bmc_cmds::CMD_RD_BL ||
		m_cmdr == bmc_cmds::CMD_RCD;
	update_drq();
	run_fsm();
}

void i7220_1_device::call_subcmd(cmds sub_cmd, fsm_state ret_state)
{
	m_ret_state = ret_state;
	m_fsm_state = S0;
	m_sub_cmd = sub_cmd;
}

void i7220_1_device::ret_subcmd()
{
	m_fsm_state = m_ret_state;
	m_ret_state = IDLE;
}

bool i7220_1_device::in_a_subcmd() const
{
	return m_ret_state != IDLE;
}

void i7220_1_device::leave()
{
	if (in_a_subcmd() && !BIT(m_str, STAT_OPFAIL)) {
		ret_subcmd();
	} else {
		LOGCMD("Cmd ended %x, ST=%02x @%s\n", m_cmdr, m_str, machine().time().as_string(6));
		// Raise an error interrupt?
		if (BIT(m_str, STAT_TIMERR) &&
			(BIT(m_en, EN_ICD) || BIT(m_en, EN_RCD) || BIT(m_en, EN_INT_ERR))) {
			set_irq();
		}
		if (BIT(m_str, STAT_UNCORRERR) &&
			(BIT(m_en, EN_ICD) || BIT(m_en, EN_RCD))) {
			set_irq();
		}
		if (BIT(m_str, STAT_CORRERR) &&
			BIT(m_en, EN_INT_ERR) &&
			(BIT(m_en, EN_ICD) || BIT(m_en, EN_RCD))) {
			set_irq();
		}
		if (BIT(m_str, STAT_OPCOMPLETE) &&
			BIT(m_en, EN_INT_NORM)) {
			// Normal termination interrupt
			set_irq();
		}
		m_fsm_state = IDLE;
		m_ret_state = IDLE;
		update_drq();
	}
}

void i7220_1_device::set_op_complete()
{
	LOGFSM("op complete\n");
	if (!in_a_subcmd()) {
		BIT_SET(m_str, STAT_OPCOMPLETE);
	}
}

void i7220_1_device::set_op_complete_n_leave()
{
	set_op_complete();
	leave();
}

void i7220_1_device::set_op_fail()
{
	LOGFSM("op fail\n");
	BIT_CLR(m_str, STAT_OPCOMPLETE);
	BIT_SET(m_str, STAT_OPFAIL);
}

void i7220_1_device::set_op_fail_n_leave()
{
	set_op_fail();
	leave();
}

bool i7220_1_device::cond_assert(bool c)
{
	if (!c) {
		LOGCMD("Assertion failed\n");
		set_op_fail_n_leave();
	}
	return c;
}

void i7220_1_device::run_fsm()
{
	while (m_fsm_state != IDLE &&
		   (!m_timer->enabled() || m_timer->expire().is_never())) {
		LOGFSM("FSM %d %d %x %x\n", m_fsm_state, m_ret_state, m_cmdr, m_sub_cmd);
		if (m_ret_state == IDLE) {
			// Executing a main command
			one_cmd_step(static_cast<cmds>(m_cmdr));
		} else {
			// Executing a sub-command
			one_cmd_step(m_sub_cmd);
		}
	}
}

void i7220_1_device::one_cmd_step(cmds cmd)
{
	switch(cmd) {
	case cmds::CMDS_WR_BLR_MASKED:
		do_wr_blr_masked();
		break;

	case cmds::CMDS_INITIALIZE:
		do_initialize();
		break;

	case cmds::CMDS_RD_DATA:
		do_rd_data();
		break;

	case cmds::CMDS_WR_DATA:
		do_wr_data();
		break;

	case cmds::CMDS_RD_SEEK:
		do_rd_seek();
		break;

	case cmds::CMDS_RD_BLR:
		do_rd_blr();
		break;

	case cmds::CMDS_WR_BLR:
		do_wr_blr();
		break;

	case cmds::CMDS_WR_BL:
		do_wr_bl();
		break;

	case cmds::CMDS_RD_FSA_STAT:
		do_rd_fsa_stat();
		break;

	case cmds::CMDS_ABORT:
		do_abort();
		break;

	case cmds::CMDS_WR_SEEK:
		do_wr_seek();
		break;

	case cmds::CMDS_RD_BL:
		do_rd_bl();
		break;

	case cmds::CMDS_RCD:
		do_rcd();
		break;

	case cmds::CMDS_RESET_FIFO:
		do_reset_fifo();
		break;

	case cmds::CMDS_MBM_PURGE:
		do_mbm_purge();
		break;

	case cmds::CMDS_SWRESET:
		do_swreset();
		break;

	case cmds::SUBCMD_READ_SEEK:
		do_read_seek();
		break;

	case cmds::SUBCMD_READ_CH_STAT:
		do_read_ch_stat();
		break;

	case cmds::SUBCMD_FSA_BMC_XFER:
		do_fsa_bmc_xfer();
		break;

	case cmds::SUBCMD_WRITE_SEEK:
		do_write_seek();
		break;

	case cmds::SUBCMD_ROTATE:
		do_rotate();
		break;
	}
}

void i7220_1_device::delay(unsigned clks, fsm_state new_state)
{
	m_timer->adjust(clocks_to_attotime(clks));
	m_fsm_state = new_state;
}

void i7220_1_device::clr_fifo_enqueue()
{
	m_fifo_mask = BIT_MASK<uint8_t>(7);
	m_fifo_accum = 0;
}

bool i7220_1_device::fifo_enqueue_bits(unsigned n_bits, uint16_t data)
{
	for (unsigned i = 0; i < n_bits; ++i) {
		if (BIT(data, i)) {
			m_fifo_accum |= m_fifo_mask;
		}
		m_fifo_mask >>= 1;
		if (m_fifo_mask == 0) {
			if (!fifo_enqueue_byte(m_fifo_accum)) {
				return false;
			}
			clr_fifo_enqueue();
		}
	}

	return true;
}

bool i7220_1_device::fifo_enqueue_byte(uint8_t data)
{
	if (m_fifo.full()) {
		// FIFO overflow
		LOGFIFO("FIFO OVERFLOW\n");
		BIT_SET(m_str, STAT_TIMERR);
		set_op_fail_n_leave();
		return false;
	} else {
		LOGFIFO("ENQ %02x\n", data);
		m_fifo.enqueue(data);
		update_drq();
		return true;
	}
}

void i7220_1_device::clr_fifo_dequeue()
{
	m_fifo_mask = 0;
}

bool i7220_1_device::fifo_dequeue_bits(unsigned n_bits, uint16_t &data)
{
	uint16_t res = 0;
	for (unsigned i = 0; i < n_bits; ++i) {
		if (m_fifo_mask == 0) {
			if (m_fifo.empty()) {
				// FIFO underflow
				LOGFIFO("FIFO UNDERFLOW\n");
				BIT_SET(m_str, STAT_TIMERR);
				set_op_fail_n_leave();
				return false;
			}
			m_fifo_accum = m_fifo.dequeue();
			LOGFIFO("DEQ %02x\n", m_fifo_accum);
			update_drq();
			m_fifo_mask = BIT_MASK<uint8_t>(7);
		}
		if (m_fifo_mask & m_fifo_accum) {
			BIT_SET(res, i);
		}
		m_fifo_mask >>= 1;
	}

	data = res;
	return true;
}

void i7220_1_device::do_wr_blr_masked()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized) &&
			cond_assert(get_nfc() == 1)) {
			clr_fifo_dequeue();
			select_chs();
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_WRITE_BLR);
			delay(CLKS_CMD, S1);
		}
		break;

	case S1:
		m_cnt1 = m_cnt2 = bits_per_xfer() / 2;
		m_cnt3 = fsa_channel_device::BLR_BITS;
		m_fsm_state = S2;
		[[fallthrough]];

	case S2:
		if (m_cnt3) {
			m_cnt3--;
			uint16_t w;
			if (!fifo_dequeue_bits(2, w)) {
				return;
			}
			if (m_cnt1 != 0 && BIT(w, 0)) {
				m_cnt1--;
			} else {
				BIT_CLR(w, 0);
			}
			if (m_cnt2 != 0 && BIT(w, 1)) {
				m_cnt2--;
			} else {
				BIT_CLR(w, 1);
			}
			LOGFSA_IO("BLMSK %u %u %u\n", m_cnt1, m_cnt2, m_cnt3);
			dio_w(w << fsa_1st_ch());
			delay(CLKS_FAST_IO, S2);
		} else {
			delay(360, S3);
		}
		break;

	case S3:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_wr_blr_masked", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_initialize()
{
	switch (m_fsm_state) {
	case S0:
		call_subcmd(cmds::CMDS_ABORT, S1);
		break;

	case S1:
		call_subcmd(cmds::CMDS_MBM_PURGE, S2);
		break;

	case S2:
		call_subcmd(cmds::CMDS_RESET_FIFO, S3);
		break;

	case S3:
		if (cond_assert(get_nfc() == 1)) {
			// Step through all MBMs, attempt to read BL
			m_cnt4 = get_mbm_select() + 1;
			set_mbm_select(0);
			m_fsm_state = S4;
		}
		break;

	case S4:
		// Read BL into FIFO
		call_subcmd(cmds::CMDS_RD_BL, S5);
		break;

	case S5:
		// Write BL from FIFO to both channels
		call_subcmd(cmds::CMDS_WR_BLR_MASKED, S6);
		break;

	case S6:
		if (--m_cnt4) {
			// Move to next MBM
			set_mbm_select(get_mbm_select() + 1);
			m_fsm_state = S4;
		} else {
			set_op_complete_n_leave();
		}
		break;

	default:
		LOG("Unexpected state %d in do_initialize", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_rd_data()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			clr_fifo_enqueue();
			m_fsm_state = S1;
		}
		break;

	case S1:
		m_mbm_select_changed = true;
		select_chs();
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
		delay(CLKS_CMD, S2);
		break;

	case S2:
		cmd_w(ec_enabled() ? m_selected_chs : 0, fsa_channel_device::FSA_CMD_SET_EC);
		delay(CLKS_CMD, S3);
		break;

	case S3:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_READ_DATA);
		delay(CLKS_CMD, S4);
		break;

	case S4:
		call_subcmd(cmds::SUBCMD_READ_SEEK, S5);
		break;

	case S5:
		bubble_replicate();
		field_rotate(S6);
		break;

	case S6:
		field_rotate(S7);
		break;

	case S7:
		bubble_replicate();
		m_cnt1 = 38;
		call_subcmd(cmds::SUBCMD_ROTATE, S8);
		break;

	case S8:
		m_cnt1 = 0;
		m_fsm_state = S9;
		break;

	case S9:
		if (m_cnt1 < fsa_channel_device::RAW_BITS) {
			if (!m_mbm_select_changed && m_cnt1 < bits_per_xfer()) {
				uint16_t w = dio_r();
				if ((!ec_enabled() || m_cnt1 < fsa_channel_device::PAYLOAD_BITS) &&
					!fifo_enqueue_bits(fsa_n_channels(), w >> fsa_1st_ch())) {
					return;
				}
			}
			m_cnt1++;
			shiftclk();
			field_rotate(S9);
		} else {
			m_cnt1 = 5;
			call_subcmd(cmds::SUBCMD_ROTATE, S10);
		}
		break;

	case S10:
		dec_n_pages();
		LOGFSM("Pages=%u\n", get_n_pages());
		if (ec_enabled() && errflg_r()) {
			// Error detected
			if (BIT(m_en, EN_RCD)) {
				// Level 1 error correction
				cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_RCD);
				delay(CLKS_CMD, S11);
			} else {
				// Level 2 & 3 error correction
				cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_ICD);
				delay(CLKS_CMD, S14);
			}
		} else {
			m_mbm_select_changed = inc_ar();
			if (get_n_pages() == 0) {
				set_op_complete();
				call_subcmd(cmds::SUBCMD_FSA_BMC_XFER, S20);
			} else if (m_mbm_select_changed) {
				call_subcmd(cmds::SUBCMD_FSA_BMC_XFER, S19);
			} else {
				delay(320, S4);
			}
		}
		break;

	case S11:
		call_subcmd(cmds::SUBCMD_FSA_BMC_XFER, S12);
		break;

	case S12:
		call_subcmd(cmds::SUBCMD_READ_CH_STAT, S13);
		break;

	case S13:
		if (!BIT(m_str, STAT_UNCORRERR)) {
			inc_ar();
		}
		m_fsm_state = S18;
		break;

	case S14:
		if (!errflg_r()) {
			delay(20, S14);
		} else {
			call_subcmd(cmds::SUBCMD_READ_CH_STAT, S15);
		}
		break;

	case S15:
		if (!BIT(m_str, STAT_UNCORRERR)) {
			if (BIT(m_en, EN_INT_ERR)) {
				m_fsm_state = S20;
			} else {
				cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_RCD);
				delay(CLKS_CMD, S16);
			}
		} else {
			m_fsm_state = S18;
		}
		break;

	case S16:
		call_subcmd(cmds::SUBCMD_FSA_BMC_XFER, S17);
		break;

	case S17:
		inc_ar();
		m_fsm_state = S18;
		break;

	case S18:
		LOGFSM("S18 %u %02x %02x\n", get_n_pages(), m_en, m_str);
		if (get_n_pages() == 0) {
			if (BIT(m_en, EN_RCD) || !BIT(m_str, STAT_UNCORRERR)) {
				set_op_complete();
			} else {
				set_op_fail();
			}
			m_fsm_state = S20;
		} else if (BIT(m_str, STAT_UNCORRERR)) {
			set_op_fail();
			m_fsm_state = S20;
		} else {
			m_fsm_state = S1;
		}
		break;

	case S19:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
		delay(CLKS_CMD, S1);
		break;

	case S20:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_NOP);
		delay(CLKS_CMD, S21);
		break;

	case S21:
		leave();
		break;

	default:
		LOG("Unexpected state %d in do_rd_datad", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_wr_data()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			clr_fifo_dequeue();
			m_fsm_state = S1;
		}
		break;

	case S1:
		m_mbm_select_changed = true;
		select_chs();
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
		delay(CLKS_CMD, S2);
		break;

	case S2:
		cmd_w(ec_enabled() ? m_selected_chs : 0, fsa_channel_device::FSA_CMD_SET_EC);
		delay(CLKS_CMD, S3);
		break;

	case S3:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_WRITE_DATA);
		delay(CLKS_CMD, S4);
		break;

	case S4:
		call_subcmd(cmds::SUBCMD_WRITE_SEEK, S5);
		break;

	case S5:
		if (m_mbm_select_changed) {
			delay(100, S6);
		} else {
			m_fsm_state = S7;
		}
		break;

	case S6:
		if (m_fifo.queue_length() >= 2) {
			m_fsm_state = S7;
		} else {
			m_fsm_state = S5;
		}
		break;

	case S7: {
		uint16_t w;
		if (!fifo_dequeue_bits(fsa_n_channels(), w)) {
			return;
		}
		dio_w(w << fsa_1st_ch());
		m_cnt1 = 0;
		delay(CLKS_SLOW_IO, S8);
	}
		break;

	case S8:
		if (m_cnt1 < fsa_channel_device::RAW_BITS) {
			if (ec_enabled()) {
				if (m_cnt1 < fsa_channel_device::PAYLOAD_BITS - 1) {
					uint16_t w;
					if (!fifo_dequeue_bits(fsa_n_channels(), w)) {
						return;
					}
					dio_w(w << fsa_1st_ch());
				} else if (m_cnt1 < fsa_channel_device::EC_BITS - 1) {
					dio_w(0);
				}
			} else if (m_cnt1 < fsa_channel_device::NO_EC_BITS - 1) {
				uint16_t w;
				if (!fifo_dequeue_bits(fsa_n_channels(), w)) {
					return;
				}
				dio_w(w << fsa_1st_ch());
			}
			m_cnt1++;
			shiftclk();
			field_rotate(S8);
		} else {
			m_cnt1 = 20;
			call_subcmd(cmds::SUBCMD_ROTATE, S9);
		}
		break;

	case S9:
		bubble_swap();
		m_cnt1 = 2;
		call_subcmd(cmds::SUBCMD_ROTATE, S10);
		break;

	case S10:
		bubble_swap();
		m_cnt1 = 23;
		call_subcmd(cmds::SUBCMD_ROTATE, S11);
		break;

	case S11:
		dec_n_pages();
		m_mbm_select_changed = inc_ar();
		if (get_n_pages() == 0) {
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_NOP);
			delay(CLKS_CMD, S12);
		} else if (m_mbm_select_changed) {
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
			delay(CLKS_CMD, S1);
		} else {
			m_fsm_state = S4;
		}
		break;

	case S12:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_wrdata", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_rd_seek()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			select_chs();
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SET_ENABLE);
			delay(CLKS_CMD, S1);
		}
		break;

	case S1:
		call_subcmd(cmds::SUBCMD_READ_SEEK, S2);
		break;

	case S2:
		m_cnt1 = BIT(get_la(), 0) ? 366 : 364;
		call_subcmd(cmds::SUBCMD_ROTATE, S3);
		break;

	case S3:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
		delay(CLKS_CMD, S4);
		break;

	case S4:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_rd_seek", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_rd_blr()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			clr_fifo_enqueue();
			select_chs();
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_READ_BLR);
			m_cnt1 = fsa_channel_device::BLR_BITS;
			delay(CLKS_CMD + 20, S1);
		}
		break;

	case S1:
		if (m_cnt1) {
			m_cnt1--;
			auto w = dio_r();
			if (!fifo_enqueue_bits(fsa_n_channels(), w >> fsa_1st_ch())) {
				return;
			}
			delay(CLKS_FAST_IO, S1);
		} else {
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
			delay(CLKS_CMD + 300, S2);
		}
		break;

	case S2:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_rd_blr", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_wr_blr()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			clr_fifo_dequeue();
			select_chs();
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_WRITE_BLR);
			m_cnt1 = fsa_channel_device::BLR_BITS;
			delay(CLKS_CMD + 20, S1);
		}
		break;

	case S1:
		if (m_cnt1) {
			m_cnt1--;
			uint16_t w;
			if (!fifo_dequeue_bits(fsa_n_channels(), w)) {
				return;
			}
			dio_w(w << fsa_1st_ch());
			delay(CLKS_FAST_IO, S1);
		} else {
			// Discard a dummy byte from FIFO
			uint16_t dummy;
			if (!fifo_dequeue_bits(8, dummy)) {
				return;
			}
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
			delay(CLKS_CMD + 300, S2);
		}
		break;

	case S2:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_wr_blr", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_wr_bl()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized) &&
			cond_assert(get_nfc() == 1)) {
			if (!BIT(m_en, EN_WR_BL)) {
				BIT_SET(m_str, STAT_TIMERR);
				set_op_fail_n_leave();
			} else {
				clr_fifo_dequeue();
				select_chs();
				cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
				delay(CLKS_CMD, S1);
			}
		}
		break;

	case S1:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_WRITE_DATA);
		delay(CLKS_CMD, S2);
		break;

	case S2:
		m_cnt1 = 18;
		m_fsm_state = S3;
		break;

	case S3:
		if (m_cnt1) {
			m_cnt1--;
			dio_w(0xffff);
			shiftclk();
			field_rotate(S3);
		} else {
			m_cnt1 = fsa_channel_device::BL_PRE_SYNC_BITS - 18;
			m_fsm_state = S4;
		}
		break;

	case S4:
		if (m_cnt1) {
			m_cnt1--;
			bootloop_swap();
			dio_w(0xffff);
			shiftclk();
			field_rotate(S4);
		} else {
			m_cnt1 = fsa_channel_device::BL_SYNC_BITS;
			m_fsm_state = S5;
		}
		break;

	case S5:
		if (m_cnt1) {
			m_cnt1--;
			bootloop_swap();
			dio_w(0);
			shiftclk();
			field_rotate(S5);
		} else {
			m_cnt1 = fsa_channel_device::BL_PATTERN_BITS;
			m_fsm_state = S6;
		}
		break;

	case S6:
		if (m_cnt1) {
			m_cnt1--;
			bootloop_swap();
			dio_w(BIT(m_cnt1, 0) ? 0xffff : 0);
			shiftclk();
			field_rotate(S6);
		} else {
			m_cnt1 = fsa_channel_device::BL_BOOTLOOP_BITS;
			m_fsm_state = S7;
		}
		break;

	case S7:
		if (m_cnt1) {
			m_cnt1--;
			bootloop_swap();
			uint16_t w;
			if (!fifo_dequeue_bits(1, w)) {
				return;
			}
			dio_w(w ? 0xffff : 0);
			shiftclk();
			field_rotate(S7);
		} else {
			m_cnt1 = fsa_channel_device::BL_PAD_BITS;
			m_fsm_state = S8;
		}
		break;

	case S8:
		if (m_cnt1) {
			m_cnt1--;
			bootloop_swap();
			dio_w(0);
			shiftclk();
			field_rotate(S8);
		} else {
			m_cnt1 = 18;
			m_fsm_state = S9;
		}
		break;

	case S9:
		if (m_cnt1) {
			m_cnt1--;
			bootloop_swap();
			field_rotate(S9);
		} else {
			// Extract and discard dummy byte at end of FIFO
			uint16_t dummy;
			if (!fifo_dequeue_bits(8, dummy)) {
				return;
			}
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
			delay(1960, S10);
		}
		break;

	case S10:
		m_initialized = false;
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_wr_bl", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_rd_fsa_stat()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			clr_fifo_enqueue();
			m_cnt1 = 0;
			m_fsm_state = S1;
		}
		break;

	case S1:
		if (m_cnt1 < m_mbm_count) {
			cmd_w(fsa_1ch_mask(m_cnt1 * 2), fsa_channel_device::FSA_CMD_READ_STATUS);
			delay(CLKS_CMD * 2, S2);
		} else {
			delay(140, S4);
		}
		break;

	case S2: {
		uint8_t s = status_r();
		if (!fifo_enqueue_byte(s)) {
			return;
		}
		cmd_w(fsa_1ch_mask(m_cnt1 * 2 + 1), fsa_channel_device::FSA_CMD_READ_STATUS);
		delay(CLKS_CMD * 2, S3);
	}
		break;

	case S3: {
		uint8_t s = status_r();
		if (!fifo_enqueue_byte(s)) {
			return;
		}
		m_cnt1++;
		m_fsm_state = S1;
	}
		break;

	case S4:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_rd_fsa_stat", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_abort()
{
	switch (m_fsm_state) {
	case S0:
		if (m_aborted) {
			m_cnt1 = 0;
			m_fsm_state = S1;
		} else {
			m_fsm_state = S6;
		}
		break;

	case S1:
		if (m_cnt1 < m_mbm_count) {
			cmd_w(fsa_1ch_mask(m_cnt1 * 2), fsa_channel_device::FSA_CMD_NOP);
			delay(CLKS_CMD, S2);
		} else {
			m_initialized = false;
			m_fsm_state = S6;
		}
		break;

	case S2:
		cmd_w(fsa_1ch_mask(m_cnt1 * 2), fsa_channel_device::FSA_CMD_READ_STATUS);
		delay(CLKS_CMD, S3);
		break;

	case S3:
		status_r();
		cmd_w(fsa_1ch_mask(m_cnt1 * 2 + 1), fsa_channel_device::FSA_CMD_NOP);
		delay(CLKS_CMD, S4);
		break;

	case S4:
		cmd_w(fsa_1ch_mask(m_cnt1 * 2 + 1), fsa_channel_device::FSA_CMD_READ_STATUS);
		delay(CLKS_CMD, S5);
		break;

	case S5:
		status_r();
		m_cnt1++;
		m_fsm_state = S1;
		break;

	case S6:
		m_fifo.clear();
		clr_status();
		delay(240, S7);
		break;

	case S7:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_abort", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_wr_seek()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			select_chs();
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SET_ENABLE);
			delay(CLKS_CMD, S1);
		}
		break;

	case S1:
		call_subcmd(cmds::SUBCMD_WRITE_SEEK, S2);
		break;

	case S2:
		m_cnt1 = BIT(get_la(), 0) ? 366 : 364;
		call_subcmd(cmds::SUBCMD_ROTATE, S3);
		break;

	case S3:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
		delay(CLKS_CMD, S4);
		break;

	case S4:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_wr_seek", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_rd_bl()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized) &&
			cond_assert(get_nfc() == 1)) {
			clr_fifo_enqueue();
			select_chs();
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_SWRESET);
			delay(CLKS_CMD, S1);
		}
		break;

	case S1:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_INIT);
		m_cnt1 = 362;
		delay(CLKS_CMD, S2);
		break;

	case S2:
		if (m_cnt1) {
			m_cnt1--;
			bootloop_replicate();
			field_rotate(S2);
		} else {
			m_cnt1 = fsa_channel_device::BL_PRE_SYNC_BITS +
				fsa_channel_device::BL_SYNC_BITS +
				fsa_channel_device::BL_PATTERN_BITS +
				fsa_channel_device::BL_BOOTLOOP_BITS +
				fsa_channel_device::BITS_PER_LOOP - 1;
			m_cnt2 = 3937;
			m_fsm_state = S3;
		}
		break;

	case S8:
		if (m_cnt2) {
			delay(CLKS_ROTATE * m_cnt2, S8);
			m_cnt2 = 0;
		} else {
			set_op_complete_n_leave();
		}
		break;

	default: {
		shiftclk();
		// Get 1 BL bit from channel A (no idea what to do with the one from channel B)
		bool bit = BIT(dio_r(), fsa_1st_ch());
		switch (m_fsm_state) {
		case S3:
			// Wait for pre-sync
			if (bit) {
				m_cnt3 = fsa_channel_device::BL_PRE_SYNC_BITS - 1;
				m_fsm_state = S4;
			}
			break;

		case S4:
			// Wait for pre-sync to meet min length
			if (bit) {
				if (--m_cnt3 == 0) {
					m_fsm_state = S5;
				}
			} else {
				m_fsm_state = S3;
			}
			break;

		case S5:
			// Wait for pre-sync to end
			if (!bit) {
				m_cnt3 = fsa_channel_device::BL_SYNC_BITS + fsa_channel_device::BL_PATTERN_BITS - 1;
				m_fsm_state = S6;
			}
			break;

		case S6: {
			// Wait for sync & pattern bits
			bool exp_bit = m_cnt3 > fsa_channel_device::BL_PATTERN_BITS ? false : !BIT(m_cnt3, 0);
			if (bit == exp_bit) {
				if (--m_cnt3 == 0) {
					m_cnt3 = fsa_channel_device::BL_BOOTLOOP_BITS;
					m_fsm_state = S7;
				}
			} else {
				m_fsm_state = S3;
			}
		}
			break;

		case S7:
			// Read BL bits
			if (!fifo_enqueue_bits(1, bit)) {
				return;
			}
			if (--m_cnt3 == 0) {
				// Synchronization achieved
				// Padding is not checked, its purpose is to keep even output track clean (filled with 0's) after synchronization
				auto mbm = fsa_1st_mbm();
				LOGCMD("SYNC %u %u %u\n", mbm, m_cnt1, m_cnt2);
				m_la[ mbm ] = 0;
				m_skip[ mbm ] = 0;
				m_fsm_state = S8;
				return;
			}
			break;

		default:
			LOG("Unexpected state %d in do_rd_bl", m_fsm_state);
			set_op_fail_n_leave();
			return;
		}
		if (--m_cnt1 == 0) {
			// No valid BL found
			m_initialized = false;
			BIT_SET(m_str, STAT_TIMERR);
			set_op_fail_n_leave();
		} else {
			if (m_cnt2) {
				m_cnt2--;
			}
			bootloop_replicate();
			field_rotate(m_fsm_state);
		}
	}
		break;
	}
}

void i7220_1_device::do_rcd()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized) &&
			cond_assert(ec_enabled())) {
			clr_fifo_enqueue();
			select_chs();
			cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_RCD);
			m_cnt1 = 0;
			delay(CLKS_CMD, S1);
		}
		break;

	case S1:
		if (m_cnt1 < fsa_channel_device::EC_BITS) {
			auto w = dio_r();
			if (m_cnt1 < fsa_channel_device::PAYLOAD_BITS &&
				!fifo_enqueue_bits(fsa_n_channels(), w >> fsa_1st_ch())) {
				return;
			}
			m_cnt1++;
			delay(CLKS_FAST_IO, S1);
		} else {
			delay(160, S2);
		}
		break;

	case S2:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_rcd", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_reset_fifo()
{
	switch (m_fsm_state) {
	case S0:
		m_fifo.clear();
		delay(200, S1);
		break;

	case S1:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_reset_fifo", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_mbm_purge()
{
	switch (m_fsm_state) {
	case S0:
		m_initialized = true;
		for (unsigned i = 0; i < MAX_MBM; i++) {
			m_la[ i ] = 0;
			m_skip[ i ] = 0;
		}
		m_ar &= ~LA_MASK;
		m_cnt1 = 0;
		m_mbm_count = 0;
		cmd_w(0xffff, fsa_channel_device::FSA_CMD_SWRESET);
		delay(CLKS_CMD, S1);
		break;

	case S1:
		if (m_cnt1 < MAX_MBM) {
			cmd_w(fsa_1ch_mask(m_cnt1 * 2), fsa_channel_device::FSA_CMD_READ_STATUS);
			delay(CLKS_CMD, S2);
		} else {
			LOGCMD("%u MBM(s) found\n", m_mbm_count);
			delay(240, S3);
		}
		break;

	case S2:
		if (BIT(status_r(), fsa_channel_device::STAT_FIFOMT)) {
			// 1 MBM found
			m_mbm_count++;
		}
		m_cnt1++;
		m_fsm_state = S1;
		break;

	case S3:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_mbm_purge", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_swreset()
{
	switch (m_fsm_state) {
	case S0:
		if (cond_assert(m_initialized)) {
			m_ar &= ~LA_MASK;
			m_blr &= ~N_PAGE_MASK;
			cmd_w(0xffff, fsa_channel_device::FSA_CMD_SWRESET);
			delay(200, S1);
		}
		break;

	case S1:
		set_op_complete_n_leave();
		break;

	default:
		LOG("Unexpected state %d in do_swreset", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_seek(uint16_t offset)
{
	auto first_mbm = fsa_1st_mbm();
	if (((get_la() + offset) & LA_MASK) == m_la[ first_mbm ] && m_skip[ first_mbm ] == 0) {
		// LA reached
		ret_subcmd();
	} else {
		// LA not reached, keep rotating
		field_rotate(S0);
	}
}

void i7220_1_device::do_read_seek()
{
	// Just S0
	do_seek(0);
}

void i7220_1_device::do_read_ch_stat()
{
	switch (m_fsm_state) {
	case S0:
		cmd_w(m_selected_chs, fsa_channel_device::FSA_CMD_READ_ERRFLG);
		delay(CLKS_CMD + CLKS_FAST_IO, S1);
		break;

	case S1:
		m_selected_chs = dio_r();
		m_cnt1 = 0;
		m_fsm_state = S2;
		break;

	case S2:
		if (m_cnt1 < MAX_CH) {
			uint16_t ch_mask = fsa_1ch_mask(m_cnt1);
			if (ch_mask & m_selected_chs) {
				cmd_w(ch_mask, fsa_channel_device::FSA_CMD_READ_STATUS);
				delay(CLKS_CMD, S3);
			} else {
				m_cnt1++;
			}
		} else {
			select_chs();
			ret_subcmd();
		}
		break;

	case S3: {
		uint8_t status = status_r();
		if (BIT(status, fsa_channel_device::STAT_UNCORRERR)) {
			BIT_SET(m_str, STAT_UNCORRERR);
		}
		if (BIT(status, fsa_channel_device::STAT_CORRERR)) {
			BIT_SET(m_str, STAT_CORRERR);
		}
		m_cnt1++;
		m_fsm_state = S2;
	}
		break;

	default:
		LOG("Unexpected state %d in do_read_ch_stat", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_fsa_bmc_xfer()
{
	switch (m_fsm_state) {
	case S0:
		m_cnt1 = 0;
		m_fsm_state = S1;
		[[fallthrough]];

	case S1:
		if (m_cnt1 < bits_per_xfer()) {
			auto w = dio_r();
			if ((!ec_enabled() || m_cnt1 < fsa_channel_device::PAYLOAD_BITS) &&
				!fifo_enqueue_bits(fsa_n_channels(), w >> fsa_1st_ch())) {
				return;
			}
			m_cnt1++;
			delay(BIT(m_en, EN_MFBTR) ? CLKS_SLOW_IO : CLKS_FAST_IO, S1);
		} else {
			ret_subcmd();
		}
		break;

	default:
		LOG("Unexpected state %d in do_fsa_bmc_xfer", m_fsm_state);
		set_op_fail_n_leave();
	}
}

void i7220_1_device::do_write_seek()
{
	// Just S0
	do_seek(WR_OFF);
}

void i7220_1_device::do_rotate()
{
	// Just S0
	if (m_cnt1) {
		m_cnt1--;
		field_rotate(S0);
	} else {
		ret_subcmd();
	}
}

uint8_t i7220_1_device::get_nfc() const
{
	return (m_blr >> NFC_SHIFT) & NFC_MASK;
}

void i7220_1_device::set_nfc(uint8_t n)
{
	m_blr = (m_blr & ~(uint16_t(NFC_MASK) << NFC_SHIFT)) | (uint16_t(n & NFC_MASK) << NFC_SHIFT);
}

uint8_t i7220_1_device::get_mbm_select() const
{
	return (m_ar >> MBM_SHIFT) & MBM_MASK;
}

void i7220_1_device::set_mbm_select(uint8_t m)
{
	m_ar = (m_ar & ~(uint16_t(MBM_MASK) << MBM_SHIFT)) | (uint16_t(m & MBM_MASK) << MBM_SHIFT);
}

uint16_t i7220_1_device::get_n_pages() const
{
	return m_blr & N_PAGE_MASK;
}

void i7220_1_device::dec_n_pages()
{
	uint16_t pages = get_n_pages();

	m_blr = (m_blr & ~N_PAGE_MASK) | ((pages - 1) & N_PAGE_MASK);
}

uint16_t i7220_1_device::get_la() const
{
	return m_ar & LA_MASK;
}

bool i7220_1_device::inc_ar()
{
	auto old_mbm = get_mbm_select();
	m_ar = (m_ar + 1) & AR_MASK;
	LOGFSM("AR: %04x\n", m_ar);
	return old_mbm != get_mbm_select();
}

void i7220_1_device::select_chs()
{
	m_selected_chs = fsa_ch_mask();
}

unsigned i7220_1_device::fsa_n_channels() const
{
	uint8_t nfc = get_nfc();
	// Valid values of nfc: 0,1,2,4,8
	if (BIT(nfc, 3)) {
		return 16;
	} else if (BIT(nfc, 2)) {
		return 8;
	} else if (BIT(nfc, 1)) {
		return 4;
	} else if (BIT(nfc, 0)) {
		return 2;
	} else {
		return 1;
	}
}

unsigned i7220_1_device::fsa_1st_ch() const
{
	return get_mbm_select() * fsa_n_channels();
}

unsigned i7220_1_device::fsa_1st_mbm() const
{
	return fsa_1st_ch() / 2;
}

uint16_t i7220_1_device::fsa_ch_mask() const
{
	return fsa_ch_mask(fsa_n_channels(), get_mbm_select());
}

uint16_t i7220_1_device::fsa_1ch_mask(uint8_t mbm_select)
{
	return fsa_ch_mask(1, mbm_select);
}

uint16_t i7220_1_device::fsa_ch_mask(unsigned n_ch, uint8_t mbm_select)
{
	// Valid values of nfc: 0,1,2,4,8
	// |                  | nfc=0        | nfc=1           | nfc=2           | nfc=4         | nfc=8         |
	// | fsa_n_channels → | 1            | 2               | 4               | 8             | 16            |
	// |    ↓  mbm_select |              |                 |                 |               |               |
	// |------------------+--------------+-----------------+-----------------+---------------+---------------|
	// |                0 | 0 (0,0001)   | 0-1 (0,0003)    | 0-3 (0,000f)    | 0-7 (0,00ff)  | 0-15 (0,ffff) |
	// |                1 | 1 (1,0002)   | 2-3 (2,000c)    | 4-7 (4,00f0)    | 8-15 (8,ff00) |               |
	// |                2 | 2 (2,0004)   | 4-5 (4,0030)    | 8-11 (8,0f00)   |               |               |
	// |                3 | 3 (3,0008)   | 6-7 (6,00c0)    | 12-15 (12,f000) |               |               |
	// |                4 | 4 (4,0010)   | 8-9 (8,0300)    |                 |               |               |
	// |                5 | 5 (5,0020)   | 10-11 (10,0c00) |                 |               |               |
	// |                6 | 6 (6,0040)   | 12-13 (12,3000) |                 |               |               |
	// |                7 | 7 (7,0080)   | 14-15 (14,c000) |                 |               |               |
	// |                8 | 8 (8,0100)   |                 |                 |               |               |
	// |                9 | 9 (9,0200)   |                 |                 |               |               |
	// |               10 | 10 (10,0400) |                 |                 |               |               |
	// |               11 | 11 (11,0800) |                 |                 |               |               |
	// |               12 | 12 (12,1000) |                 |                 |               |               |
	// |               13 | 13 (13,2000) |                 |                 |               |               |
	// |               14 | 14 (14,4000) |                 |                 |               |               |
	// |               15 | 15 (15,8000) |                 |                 |               |               |
	//
	// Each cell: x-y (f,m)
	// Where x-y is range of selected channels
	// 'f' is first channel (fsa_1st_ch)
	// 'm' is channel mask (fsa_ch_mask)

	uint16_t res = make_bitmask<uint16_t>(n_ch);
	res <<= (mbm_select * n_ch);
	return res;
}

