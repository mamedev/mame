// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp_taco.cpp

    HP TApe COntroller (5006-3012)

*********************************************************************/

// This a complete re-write of my previous driver of TACO chip.
// The old driver was entirely based on reverse-engineering of the chip as very little
// documentation was available. Then, in late 2018, a HP internal doc on TACO
// was published by Dyke Shaffer on https://groups.io/g/VintHPcom. This doc, of course,
// was a total game changer for what concerns the TACO emulation for various reasons:
// it comes from HP, it is very detailed and it appears to be an exhaustive description
// of the behavior of the chip.
// It was time to throw out my mostly-correct, hard-won reverse engineered driver and
// re-write it.

// Documentation I used:
// [1]  HP, TACO external reference specification, dec 76: *THE* document about TACO
// [2]  HP, manual 09845-10201, apr 81 rev. - General Utility Routines. This manual
//      describes the SIF format and related utility tools.

// What's in the new driver:
// - All commands used by test ROM, by system firmware and by SIF utilities are implemented
//   according to flow charts in [1]
// - Handling of error conditions, R/W bits, tachometer ticks and gaps according to [1]
// What's not in:
// - Commands that are not used at all by the software I analyzed. They could be added easily,
//   though.
// - Accurate execution times of commands
// - Handling of FLG signal: the emulated chip always reports itself as ready for writing.
// - Read threshold is ignored. Real tapes could be read with either a low or high threshold.
// Where I filled the gaps in [1]:
// - Command 0D (001101) is not documented at all but it's used by test ROM and system fw.
//   I carried this command over from my old TACO driver: it's implemented as NOP + IRQ.
// - [1] is not very clear about the gap size that's used to detect the so-called "gap in read"
//   error condition. I use a 0.132" gap (GIR_GAP_LENGTH) because it's the minimum that
//   allows the SIF utilities to run correctly.

// This is an overview of the TACO/CPU interface.
//
// Reg. | R/W | Content
// =====================
// R4   | R/W | Data register: words read/written to/from tape pass through this register
// R5   | R/W | Command and status register (see below)
// R6   | R/W | Tachometer register. Writing it sets a pulse counter that counts up on either tachometer pulses or gaps, depending
//      |     | on command. When the counter rolls over from 0xffff to 0 it typically ends the command.
//      |     | Current counter value is returned when reading from this register.
// R7   | R   | Checksum register. Reading it clears it next time the checksum is updated.
// R7   | W   | Threshold register. It sets the duration of bits when reading and writing. It
//      |     | also controls the pre-compensation when writing. This driver ignores it.
//
// Format of TACO command/status register (R5)
// Bit    R/W Content
// ===============
// 15     RW  Tape direction (1 = forward)
// 14..9  RW  Command (see the "enum" below)
//  8     RW  Minimum size of gaps (1 = 1.5", 0 = 0.017")
//  7     RW  Speed of tape (1 = 90 ips, 0 = 22 ips)
//  6     RW  MOD bit. Most of the commands use it to select read threshold (0 = low, 1 = high).
//  5     R   Current track (1 = B)
//  4     R   Gap detected (1)
//  3     R   Write protection (1)
//  2     R   Servo failure (1)
//  1     R   Cartridge out (1)
//  0     R   Hole detected (1)

// Here's a summary of the on-tape format of HP9845 systems.
// * A tape has two independent tracks (A & B).
// * Each track holds 426 records.
// * Each record has an header and 256 bytes of payload (see below)
// * Records are separated by gaps of uniform magnetization called IRG (Inter-Record Gap)
//   or IFG (Inter-File Gap), depending on the length. In HP9845 all records are separated
//   by IRGs except records 0 & 1, which are separated by 1 IFG.
// * The basic unit of data I/O are 16-bit words
// * Bits are encoded by different distances between magnetic flux reversals
// * The structure of tracks is:
//   - Begin of tape holes
//   - The deadzone: 350x 0xffff words
//   - 1" of IRG
//   - Record #0 (track A) or #426 (track B)
//   - 1" of IRG (2.5" of IFG on track A)
//   - Record #1 (track A) or #427 (track B)
//   - 1" of IRG
//   - Record #2 (track A) or #428 (track B)
//   - ...and so on up to record #425/#851
//   - EVD gap (End of Valid Data): at least 6"
//   - End of tape holes
// * Even though the tape format is not SIF (HP's own Standard Interchange Format), it is
//   clearly based on SIF itself. The whole tape content is stored according to SIF,
//   specifically it is "encapsulated" inside file #1.
// * Record #0 is not used. It serves as SIF "file identifier record" (i.e. it identifies
//   that the rest of tape is inside file #1 from SIF point of view). The IFG between
//   record #0 and #1 is placed according to SIF specification.
// * Records #1 and #2 hold the first copy of tape directory
// * Records #3 and #4 hold the second/backup copy of tape directory
// * User data are stored starting from record #5
// * There is no "fragmentation" map (like file allocation table in FAT filesystem): a file
//   spanning more than 1 record always occupy a single block of contiguous records.
//
// A record is structured like this (see description of SIF in [2], pg 655 and following):
// Word 0:      Invisible preamble word: it's made of a string of 0s terminated by a single 1.
//              It can be longer than 16 bits. This word is used to synchronize with word
//              boundary. It's always written as 0x0001. Record re-writing appears to add
//              a few 0s at the beginning so that preamble looks longer than 16 bits.
//              This is caused by re-writing starting just after reading a few bits of the
//              previous preamble in order to synchronize with record. This driver typically
//              leaves 2 bits of the previous preamble so that the new one seems to be 18
//              bit long (17x 0s and a 1).
// Word 1:      File word: file identifier bit, empty record indicator and file number
// Word 2:      Record word: record number and "free field pattern".
//              Free field pattern is 4-bit long and it's used as a kind of source
//              ID. As far as I can tell, HP9845 systems set it to 0, HP85 sets it to 1.
// Word 3:      Length word: bytes available and used in record
// Word 4:      Checksum (sum of words 1..3)
// Words 5..132:        Payload
// Word 133:    Checksum (sum of words 5..132)
//
// This is how TACO encodes words on tape:
// - the unit of encoding are 16-bit words
// - each word is encoded from MSB to LSB
// - tape is read/written at slow speed only (21.98 ips)
// - a 0 is encoded with a distance between flux reversals of 1/35200 s
//   (giving a maximum density of about 1600 reversals per inch)
// - a 1 is encoded with a distance that's 1.75 times that of a 0
// - when reading, word boundary is recovered by looking for the "1" that terminates the
//   preamble (see above)
//
// HP9825 encodes words in a slightly different way: each word has a 17th "1" at the end.
// This is added to gain some time to process read words. It carries no information and
// it's discarded when reading the tape. TACO has a special version of read & write commands
// to handle HP9825 encoding.
//
// Acknowledgments:
// Dyke Shaffer for publishing the HP-internal TACO document.
//
#include "emu.h"
#include "hp_taco.h"

// Debugging
#include "logmacro.h"
#define LOG_DBG_MASK (LOG_GENERAL << 1)
#define LOG_DBG(...) LOGMASKED(LOG_DBG_MASK, __VA_ARGS__)
#define LOG_RW_MASK (LOG_DBG_MASK << 1)
#define LOG_RW(...) LOGMASKED(LOG_RW_MASK, __VA_ARGS__)
#define LOG_REG_MASK (LOG_RW_MASK << 1)
#define LOG_REG(...) LOGMASKED(LOG_REG_MASK, __VA_ARGS__)
#undef VERBOSE
//#define VERBOSE (LOG_GENERAL | LOG_DBG_MASK | LOG_RW_MASK | LOG_REG_MASK)
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

// Timers
enum {
	GAP_TMR_ID,
	EVD_TMR_ID,
	ERROR_TMR_ID
};

// **** Constants ****
constexpr unsigned TACH_FREQ_FAST = 87196;      // Tachometer pulse frequency for fast speed (90.08 ips)
constexpr unsigned TACH_FREQ_SLOW = 21276;      // Tachometer pulse frequency for slow speed (21.98 ips)
constexpr unsigned TACH_TICKS_PER_INCH = 968;   // Tachometer pulses per inch of tape movement
constexpr hti_format_t::tape_pos_t TACH_TICK_LEN = hti_format_t::ONE_INCH_POS / TACH_TICKS_PER_INCH;    // One tachometer tick every 1/968 of inch
constexpr double FAST_SPEED = double(TACH_FREQ_FAST) / double(TACH_TICKS_PER_INCH); // Fast speed: 90.08 ips
constexpr double SLOW_SPEED = double(TACH_FREQ_SLOW) / double(TACH_TICKS_PER_INCH); // Slow speed: 21.98 ips
constexpr double MOVING_THRESHOLD = 2.0;        // Tape is moving when speed > 2.0 ips
constexpr double ACCELERATION = 1200.0;         // Acceleration when speed set point is changed: 1200 ips^2
constexpr uint16_t PREAMBLE_WORD = 0x0001;      // Value of preamble word
constexpr unsigned ERROR_IRQ_PERIOD = 2048;     // Clocks between repetitions of IRQ during error condition
constexpr hti_format_t::tape_pos_t SHORT_GAP_LENGTH = 16 * TACH_TICK_LEN;   // Minimum length of short gaps: 0.017"
constexpr hti_format_t::tape_pos_t LONG_GAP_LENGTH = 1452 * TACH_TICK_LEN;  // Minimum length of long gaps: 1.5"
constexpr hti_format_t::tape_pos_t EVD_GAP_LENGTH = 5548 * TACH_TICK_LEN;   // End of valid data gap: 5.73"
constexpr hti_format_t::tape_pos_t GIR_GAP_LENGTH = 128 * TACH_TICK_LEN;    // Minimum length of gaps for Gap In Read error: 0.132" (made up)

// Bits in command/status register
enum cmd_status_bits : unsigned {
	CMD_ST_FWD = 15,    // Forward direction (1)
	CMD_ST_G5 = 14,     // Command bit G5
	CMD_ST_G4 = 13,     // Command bit G4
	CMD_ST_G3 = 12,     // Command bit G3
	CMD_ST_G2 = 11,     // Command bit G2
	CMD_ST_G1 = 10,     // Command bit G1
	CMD_ST_G0 = 9,      // Command bit G0
	CMD_ST_FGAP = 8,    // File gap (1)
	CMD_ST_FST = 7,     // Fast speed (1)
	CMD_ST_MOD = 6,     // MOD bit
	CMD_ST_TRB = 5,     // Track B (1)
	CMD_ST_GAP = 4,     // Gap detected (1)
	CMD_ST_WPR = 3,     // Write protection (1)
	CMD_ST_ESTS = 2,    // Servo failure (1)
	CMD_ST_CART_OUT = 1,// Cartridge out (1)
	CMD_ST_HOLE = 0     // Hole detected (1)
};

// Command register mask
constexpr uint16_t CMD_REG_MASK =
	BIT_MASK<uint16_t>(CMD_ST_FWD) |
	BIT_MASK<uint16_t>(CMD_ST_G5) |
	BIT_MASK<uint16_t>(CMD_ST_G4) |
	BIT_MASK<uint16_t>(CMD_ST_G3) |
	BIT_MASK<uint16_t>(CMD_ST_G2) |
	BIT_MASK<uint16_t>(CMD_ST_G1) |
	BIT_MASK<uint16_t>(CMD_ST_G0) |
	BIT_MASK<uint16_t>(CMD_ST_FGAP) |
	BIT_MASK<uint16_t>(CMD_ST_FST) |
	BIT_MASK<uint16_t>(CMD_ST_MOD);

// Status register mask
constexpr uint16_t STATUS_REG_MASK =
	BIT_MASK<uint16_t>(CMD_ST_TRB) |
	BIT_MASK<uint16_t>(CMD_ST_GAP) |
	BIT_MASK<uint16_t>(CMD_ST_WPR) |
	BIT_MASK<uint16_t>(CMD_ST_ESTS) |
	BIT_MASK<uint16_t>(CMD_ST_CART_OUT) |
	BIT_MASK<uint16_t>(CMD_ST_HOLE);

// Commands
enum cmd_t : uint8_t {
	//                      GGGGGG
	//                      543210     * = not emulated
	CMD_INT_ON_GAP      = 0b000000, // Interrupt on new gap
	CMD_ERASE           = 0b000100, // Erase tape
	CMD_WR_REVERSAL     = 0b000101, // Write flux reversal (*)
	CMD_WR              = 0b000110, // Write
	CMD_WR_9825         = 0b000111, // Write 9825
	CMD_STOP            = 0b001000, // Stop
	CMD_STOP_INT        = 0b001001, // Stop & interrupt
	CMD_SET_TRACK       = 0b001100, // Set track
	CMD_UNK_0D          = 0b001101, // Unknown (looks like a kind of NOP + IRQ)
	CMD_MOVE            = 0b010100, // Move tape
	CMD_OPP_DIR_N_TACH  = 0b011000, // Opposite direction on N tach
	CMD_RD_12UPD        = 0b011010, // Read 12% update (*)
	CMD_RD_9825_12UPD   = 0b011011, // Read 9825 12% update (*)
	CMD_CLEAR_ST        = 0b011100, // Clear status
	CMD_RD_CSUM_12UPD   = 0b011110, // Read checksum 12% update (*)
	CMD_RD_9825_CSUM12  = 0b011111, // Read checksum 9825 12% update (*)
	CMD_STOP_IN_GAP     = 0b100000, // Stop in gap
	CMD_STOP_IN_GAP1    = 0b100001, // Stop in gap (int when stopped)
	CMD_RD_NO_UPDATE    = 0b101010, // Read no update (*)
	CMD_RD_9815         = 0b101011, // Read 9815 (*)
	CMD_WR_GAP_N_TACH   = 0b101100, // Write gap of N tach
	CMD_INT_N_GAP       = 0b110000, // Interrupt on N gap
	CMD_WR_SYNC         = 0b110010, // Write synchronous
	CMD_WR_SYNC_9825    = 0b110011, // Write synchronous 9825
	CMD_INT_N_TACH_22   = 0b110100, // Interrupt on N tach after 22 ips
	CMD_WR_CSUM         = 0b110110, // Write checksum
	CMD_WR_CSUM_9825    = 0b110111, // Write checksum 9825
	CMD_INT_STOP_N_TACH = 0b111000, // Interrupt and stop on N tach
	CMD_INT_STOP_N_TACH1= 0b111001, // Interrupt and stop on N tach (int when stopped)
	CMD_RD_6UPD         = 0b111010, // Read 6% update
	CMD_RD_9825_6UPD    = 0b111011, // Read 9825 6% update
	CMD_INT_N_TACH      = 0b111100, // Interrupt on N tach
	CMD_RD_CSUM_6UPD    = 0b111110, // Read checksum 6% update
	CMD_RD_9825_CSUM6   = 0b111111  // Read checksum 9825 6% update
};

// Device type definition
DEFINE_DEVICE_TYPE(HP_TACO, hp_taco_device, "hp_taco", "HP TACO")

// Constructors
hp_taco_device::hp_taco_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_tape(*this , "drive")
	, m_irq_handler(*this)
	, m_flg_handler(*this)
	, m_sts_handler(*this)
{
}

hp_taco_device::hp_taco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp_taco_device(mconfig, HP_TACO, tag, owner, clock)
{
}

void hp_taco_device::set_name(const std::string& name)
{
	m_tape->set_name(name);
}

void hp_taco_device::reg_w(offs_t offset, uint16_t data)
{
	LOG_REG("wr R%u = %04x\n", 4 + offset , data);

	// Any I/O activity clears IRQ
	irq_w(false);

	switch (offset) {
	case 0:
		// Data register
		m_data_reg = data;
		break;

	case 1:
		// Command register
		start_cmd_exec(data & CMD_REG_MASK);
		break;

	case 2:
		// Tachometer register
		m_tach_reg = data;
		break;

	case 3:
		// Threshold register
		m_threshold_reg = data;
		break;
	}
}

uint16_t hp_taco_device::reg_r(offs_t offset)
{
	uint16_t res = 0;

	// Any I/O activity clears IRQ
	irq_w(false);

	switch (offset) {
	case 0:
		// Data register
		res = m_data_reg;
		break;

	case 1:
		// Command & status register
		if (m_tape->gap_reached(SHORT_GAP_LENGTH)) {
			BIT_SET(m_status_reg , CMD_ST_GAP);
		} else {
			BIT_CLR(m_status_reg , CMD_ST_GAP);
		}
		res = (m_cmd_reg & CMD_REG_MASK) | (m_status_reg & STATUS_REG_MASK);
		break;

	case 2:
		// Tachometer register
		res = m_tach_reg;
		break;

	case 3:
		// Checksum register: it clears when read
		res = m_checksum_reg;
		m_checksum_reg = 0;
		break;
	}

	LOG_REG("rd R%u = %04x\n", 4 + offset , res);

	return res;
}

READ_LINE_MEMBER(hp_taco_device::flg_r)
{
	return m_flg;
}

READ_LINE_MEMBER(hp_taco_device::sts_r)
{
	return m_sts;
}

WRITE_LINE_MEMBER(hp_taco_device::cart_out_w)
{
	LOG_DBG("cart_out_w %d\n" , state);
	set_tape_present(!state);
	if (state && m_cmd_state != CMD_IDLE) {
		set_error(true , false);
		m_cmd_state = CMD_IDLE;
	}
}

WRITE_LINE_MEMBER(hp_taco_device::hole_w)
{
	if (state) {
		LOG_DBG("hole_w\n");
		if (m_cmd_state != CMD_IDLE && m_cmd_state != CMD_STOPPING &&
			(!is_double_hole_cmd(m_cmd_reg) || BIT(m_status_reg , CMD_ST_HOLE))) {
			set_error(true , false);
		}
		BIT_SET(m_status_reg , CMD_ST_HOLE);
	}
}

WRITE_LINE_MEMBER(hp_taco_device::tacho_tick_w)
{
	if (state) {
		LOG_DBG("tacho_tick_w\n");
		if (m_cmd_state == CMD_STOPPING) {
			m_tach_reg++;
		} else if (m_cmd_state != CMD_IDLE) {
			switch (get_cmd(m_cmd_reg)) {
			case CMD_OPP_DIR_N_TACH:
				if (m_cmd_state == CMD_PH0) {
					m_tach_reg++;
					if (m_tach_reg == 0) {
						// Toggle FWD bit
						m_cmd_reg ^= BIT_MASK<uint16_t>(CMD_ST_FWD);
						send_go();
						m_cmd_state = CMD_PH1;
					}
				}
				break;

			case CMD_STOP_IN_GAP:
			case CMD_STOP_IN_GAP1:
			case CMD_INT_STOP_N_TACH:
			case CMD_INT_STOP_N_TACH1:
				if (m_cmd_state == CMD_PH2) {
					m_tach_reg++;
					if (m_tach_reg == 0) {
						if (!BIT(m_cmd_reg , CMD_ST_G0)) {
							irq_w(true);
						}
						send_stop();
					}
				}
				break;

			case CMD_INT_N_TACH_22:
				if (m_cmd_state != CMD_PH1) {
					break;
				}
				[[fallthrough]];

			case CMD_WR_GAP_N_TACH:
			case CMD_INT_N_TACH:
				m_tach_reg++;
				if (m_tach_reg == 0) {
					irq_and_end();
				}
				break;

			default:
				break;
			}
		}
	}
}

WRITE_LINE_MEMBER(hp_taco_device::motion_w)
{
	if (state) {
		cmd_fsm();
	}
}

WRITE_LINE_MEMBER(hp_taco_device::rd_bit_w)
{
	LOG_RW("RD bit %d (st=%d,w=%04x,i=%u)\n" , state , m_cmd_state , m_working_reg , m_bit_idx);
	if (m_cmd_state != CMD_IDLE) {
		switch (get_cmd(m_cmd_reg)) {
		case CMD_RD_6UPD:
		case CMD_RD_9825_6UPD:
		case CMD_RD_CSUM_6UPD:
		case CMD_RD_9825_CSUM6:
			if (m_cmd_state == CMD_PH1) {
				if (m_bit_idx < 8) {
					m_bit_idx++;
				} else if (state) {
					// Synchronized
					LOG_RW("RD synced!\n");
					m_cmd_state = CMD_PH2;
					m_bit_idx = BIT(m_cmd_reg , CMD_ST_G0) ? 17 : 15;
					m_working_reg = 0;
				}
			} else if (m_cmd_state == CMD_PH2) {
				if (m_bit_idx == 17) {
					// 9825 format: skip first 17th bit after preamble
					m_bit_idx = 15;
					break;
				} else if (m_bit_idx != 16 && state) {
					// Skip 17th bit when reading in 9825 format
					BIT_SET(m_working_reg , m_bit_idx);
				}
				if (adv_bit_idx()) {
					m_data_reg = m_working_reg;
					m_working_reg = 0;
					if (!BIT(m_cmd_reg , CMD_ST_G2)) {
						update_checksum(m_data_reg);
					}
					LOG_RW("RD word %04x csum=%04x\n" , m_data_reg , m_checksum_reg);
					irq_w(true);
				}
			}
			break;

		case CMD_WR_SYNC:
		case CMD_WR_SYNC_9825:
		case CMD_WR_CSUM:
		case CMD_WR_CSUM_9825:
			if (m_cmd_state == CMD_PH1) {
				LOG_RW("WR synced\n");
				m_cmd_state = CMD_PH2;
				m_bit_idx = 15;
				start_wr();
			}
			break;

		default:
			break;
		}
	}
}

READ_LINE_MEMBER(hp_taco_device::wr_bit_r)
{
	bool bit = false;
	if (is_cmd_wr(m_cmd_reg) && m_cmd_state == CMD_PH2) {
		if (m_bit_idx == 16) {
			// HP9825 format: 17th bit
			bit = true;
		} else {
			bit = BIT(m_working_reg , m_bit_idx);
		}
		if (adv_bit_idx()) {
			m_working_reg = m_data_reg;
			// Bit 0 selects HP9825 mode: it is to be ignored here
			if ((get_cmd(m_cmd_reg) & ~BIT_MASK<uint8_t>(0)) != CMD_WR_CSUM) {
				update_checksum(m_data_reg);
			}
			LOG_RW("WR word %04x csum=%04x\n" , m_working_reg , m_checksum_reg);
			m_data_reg = m_checksum_reg;
			irq_w(true);
		}
	}
	LOG_RW("WR bit %d (w=%04x,i=%u)\n" , bit , m_working_reg , m_bit_idx);
	return bit;
}

void hp_taco_device::device_add_mconfig(machine_config &config)
{
	HP_DC100_TAPE(config , m_tape , 0);
	m_tape->set_acceleration(ACCELERATION);
	m_tape->set_set_points(SLOW_SPEED , FAST_SPEED);
	m_tape->set_tick_size(TACH_TICK_LEN);
	m_tape->set_image_format(hti_format_t::HTI_DELTA_MOD_16_BITS);
	m_tape->set_go_threshold(MOVING_THRESHOLD);
	m_tape->cart_out().set(FUNC(hp_taco_device::cart_out_w));
	m_tape->hole().set(FUNC(hp_taco_device::hole_w));
	m_tape->tacho_tick().set(FUNC(hp_taco_device::tacho_tick_w));
	m_tape->motion_event().set(FUNC(hp_taco_device::motion_w));
	m_tape->rd_bit().set(FUNC(hp_taco_device::rd_bit_w));
	m_tape->wr_bit().set(FUNC(hp_taco_device::wr_bit_r));
}

void hp_taco_device::device_start()
{
	LOG("device_start\n");
	m_irq_handler.resolve_safe();
	m_flg_handler.resolve_safe();
	m_sts_handler.resolve_safe();

	save_item(NAME(m_data_reg));
	save_item(NAME(m_cmd_reg));
	save_item(NAME(m_status_reg));
	save_item(NAME(m_tach_reg));
	save_item(NAME(m_checksum_reg));
	save_item(NAME(m_threshold_reg));
	save_item(NAME(m_irq));
	save_item(NAME(m_flg));
	save_item(NAME(m_sts));
	save_item(NAME(m_error));
	save_item(NAME(m_gap_in_read));
	save_item(NAME(m_working_reg));
	save_item(NAME(m_bit_idx));

	m_gap_timer = timer_alloc(GAP_TMR_ID);
	m_evd_timer = timer_alloc(EVD_TMR_ID);
	m_error_timer = timer_alloc(ERROR_TMR_ID);
}

void hp_taco_device::device_reset()
{
	LOG("device_reset\n");
	clear_state();

	m_irq = false;
	m_flg = true;
	m_sts = true;

	m_irq_handler(false);
	m_flg_handler(true);
	m_sts_handler(true);
	set_error(false , false);

	m_gap_timer->reset();
	m_evd_timer->reset();
	m_error_timer->reset();
}

void hp_taco_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id) {
	case GAP_TMR_ID:
		m_tape->update_speed_pos();
		LOG_DBG("Gap tmr @%g pos=%d cmd %02x st %d\n" , machine().time().as_double() , m_tape->get_pos() , get_cmd(m_cmd_reg) , m_cmd_state);

		switch (get_cmd(m_cmd_reg)) {
		case CMD_INT_ON_GAP:
			if (m_cmd_state == CMD_PH1) {
				irq_and_end();
			}
			break;

		case CMD_STOP_IN_GAP:
		case CMD_STOP_IN_GAP1:
			if (m_cmd_state == CMD_PH1) {
				// Count 256 ticks
				m_tach_reg = 0xff00;
				m_cmd_state = CMD_PH2;
			}
			break;

		case CMD_INT_N_GAP:
			if (m_cmd_state == CMD_PH1) {
				m_tach_reg++;
				if (m_tach_reg == 0) {
					irq_and_end();
				} else {
					set_gap_timer();
				}
			}
			break;

		case CMD_RD_6UPD:
		case CMD_RD_9825_6UPD:
		case CMD_RD_CSUM_6UPD:
		case CMD_RD_9825_CSUM6:
			// Gap in read error
			set_error(false , true);
			break;

		default:
			break;
		}
		break;

	case EVD_TMR_ID:
		m_tape->update_speed_pos();
		LOG_DBG("EVD tmr @%g pos=%d cmd %02x st %d\n" , machine().time().as_double() , m_tape->get_pos() , get_cmd(m_cmd_reg) , m_cmd_state);
		set_error(true , false);
		break;

	case ERROR_TMR_ID:
		LOG_DBG("Error tmr\n");
		irq_w(true);
		break;

	default:
		break;
	}
}

void hp_taco_device::clear_state()
{
	m_data_reg = 0;
	m_cmd_reg = 0;
	m_status_reg = 0;
	m_tach_reg = 0;
	m_checksum_reg = 0;
	m_threshold_reg = 0;
	m_cmd_state = CMD_IDLE;
	m_working_reg = 0;
	m_bit_idx = 0;

	set_tape_present(false);
	set_tape_present(!m_tape->cart_out_r());
}

void hp_taco_device::irq_w(bool state)
{
	if (state != m_irq) {
		m_irq = state;
		m_irq_handler(state);
		LOG_DBG("IRQ = %d\n" , state);
	}
}

void hp_taco_device::sts_w(bool state)
{
	if (state != m_sts) {
		m_sts = state;
		m_sts_handler(state);
		LOG_DBG("STS = %d\n" , state);
	}
}

void hp_taco_device::set_error(bool error , bool gap_in_read)
{
	LOG_DBG("Error %d %d\n" , error , gap_in_read);

	m_error = error;
	m_gap_in_read = gap_in_read;

	if (m_error || m_gap_in_read) {
		end_cmd();
		m_tape->set_op(hp_dc100_tape_device::OP_IDLE);
		if (m_cmd_state != CMD_STOPPING) {
			m_cmd_state = CMD_IDLE;
		}
		sts_w(false);
		m_error_timer->adjust(attotime::zero , 0 , clocks_to_attotime(ERROR_IRQ_PERIOD));
		if (m_error) {
			send_stop();
		}
	} else {
		m_error_timer->reset();
	}
}

hti_format_t::tape_pos_t hp_taco_device::min_gap_size() const
{
	return BIT(m_cmd_reg , CMD_ST_FGAP) ? LONG_GAP_LENGTH : SHORT_GAP_LENGTH;
}

void hp_taco_device::set_gap_timer()
{
	m_tape->time_to_next_gap(min_gap_size() , true , m_gap_timer);
}

void hp_taco_device::set_evd_timer()
{
	m_tape->time_to_next_gap(EVD_GAP_LENGTH , false , m_evd_timer);
}

void hp_taco_device::set_tape_present(bool present)
{
	if (present) {
		if (m_tape->wpr_r()) {
			BIT_SET(m_status_reg, CMD_ST_WPR);
		} else {
			BIT_CLR(m_status_reg, CMD_ST_WPR);
		}
		// CMD_ST_CART_OUT is reset by CMD_CLEAR_ST
	} else {
		BIT_SET(m_status_reg, CMD_ST_CART_OUT);
		BIT_SET(m_status_reg, CMD_ST_WPR);
	}
}

void hp_taco_device::send_go()
{
	hp_dc100_tape_device::tape_speed_t speed;

	if (BIT(m_cmd_reg , CMD_ST_FST)) {
		speed = hp_dc100_tape_device::SP_FAST;
	} else {
		speed = hp_dc100_tape_device::SP_SLOW;
	}
	m_tape->set_speed_setpoint(speed , BIT(m_cmd_reg , CMD_ST_FWD));
}

void hp_taco_device::send_stop()
{
	m_tape->set_op(hp_dc100_tape_device::OP_IDLE);
	if (m_tape->set_speed_setpoint(hp_dc100_tape_device::SP_STOP , false)) {
		m_cmd_state = CMD_STOPPING;
	}
}

void hp_taco_device::end_cmd()
{
	m_gap_timer->reset();
	m_evd_timer->reset();
}

void hp_taco_device::irq_and_end()
{
	irq_w(true);
	end_cmd();
	m_cmd_state = CMD_IDLE;
}

bool hp_taco_device::is_at_slow_speed() const
{
	return !m_tape->is_accelerating() && fabs(m_tape->get_speed()) == SLOW_SPEED;
}

void hp_taco_device::start_rd()
{
	if (m_tape->get_op() != hp_dc100_tape_device::OP_READ) {
		m_tape->set_op(hp_dc100_tape_device::OP_READ);
		// TODO: check
		m_tape->time_to_next_gap(GIR_GAP_LENGTH , true , m_gap_timer);
		set_evd_timer();
	}
}

void hp_taco_device::start_wr()
{
	m_tape->set_op(hp_dc100_tape_device::OP_WRITE);
	m_gap_timer->reset();
	m_evd_timer->reset();
}

bool hp_taco_device::adv_bit_idx()
{
	if (m_bit_idx) {
		m_bit_idx--;
	} else {
		m_bit_idx = BIT(m_cmd_reg , CMD_ST_G0) ? 16 : 15;
	}
	return m_bit_idx == 15;
}

void hp_taco_device::update_checksum(uint16_t data)
{
	// Update checksum with data
	m_checksum_reg += data;
}

void hp_taco_device::cmd_fsm()
{
	auto prev_state = m_cmd_state;
	if (m_cmd_state == CMD_STOPPING) {
		if (!m_tape->is_moving()) {
			LOG_DBG("Tape stopped\n");
			m_cmd_state = CMD_IDLE;
			auto cmd = get_cmd(m_cmd_reg);
			if (cmd == CMD_STOP_INT ||
				cmd == CMD_STOP_IN_GAP1 ||
				cmd == CMD_INT_STOP_N_TACH1) {
				irq_w(true);
			}
		}
	} else if (m_cmd_state != CMD_IDLE) {
		switch (get_cmd(m_cmd_reg)) {
		case CMD_INT_ON_GAP:
			if (m_cmd_state == CMD_PH0 && is_at_slow_speed()) {
				m_cmd_state = CMD_PH1;
				set_gap_timer();
				set_evd_timer();
			}
			break;

		case CMD_ERASE:
		case CMD_MOVE:
			// Stay in PH0 until next command
			break;

		case CMD_WR:
		case CMD_WR_9825:
			if (m_cmd_state == CMD_PH0 && is_at_slow_speed()) {
				m_cmd_state = CMD_PH2;
				m_bit_idx = 15;
				start_wr();
			}
			break;

		case CMD_SET_TRACK:
			m_cmd_state = CMD_IDLE;
			break;

		case CMD_UNK_0D:
			irq_and_end();
			break;

		case CMD_OPP_DIR_N_TACH:
			if (m_cmd_state == CMD_PH1 && is_at_slow_speed()) {
				irq_and_end();
			}
			break;

		case CMD_STOP_IN_GAP:
		case CMD_STOP_IN_GAP1:
			if (m_cmd_state == CMD_PH0 && !m_tape->is_accelerating()) {
				m_cmd_state = CMD_PH1;
				set_gap_timer();
			}
			break;

		case CMD_WR_GAP_N_TACH:
			break;

		case CMD_INT_N_GAP:
			if (m_cmd_state == CMD_PH0 && !m_tape->is_accelerating()) {
				m_cmd_state = CMD_PH1;
				set_gap_timer();
				set_evd_timer();
			}
			break;

		case CMD_WR_SYNC:
		case CMD_WR_SYNC_9825:
		case CMD_WR_CSUM:
		case CMD_WR_CSUM_9825:
			if (m_cmd_state == CMD_PH0 && is_at_slow_speed()) {
				m_cmd_state = CMD_PH1;
				start_rd();
			}
			break;

		case CMD_INT_N_TACH_22:
			if (m_cmd_state == CMD_PH0 && is_at_slow_speed()) {
				m_cmd_state = CMD_PH1;
			}
			break;

		case CMD_INT_STOP_N_TACH:
		case CMD_INT_STOP_N_TACH1:
			if (m_cmd_state == CMD_PH0) {
				// PH2 to use common code with CMD_STOP_IN_GAP in tacho_tick_w
				m_cmd_state = CMD_PH2;
			}
			break;

		case CMD_RD_6UPD:
		case CMD_RD_9825_6UPD:
		case CMD_RD_CSUM_6UPD:
		case CMD_RD_9825_CSUM6:
			if (m_cmd_state == CMD_PH0 && is_at_slow_speed()) {
				m_cmd_state = CMD_PH1;
				start_rd();
				m_bit_idx = 0;
			}
			break;

		case CMD_INT_N_TACH:
			break;

		default:
			break;
		}
	}
	if (prev_state != m_cmd_state) {
		LOG_DBG("FSM st %d->%d\n" , prev_state , m_cmd_state);
	}
}

uint8_t hp_taco_device::get_cmd(uint16_t cmd_reg)
{
	return uint8_t((cmd_reg & (BIT_MASK<uint16_t>(CMD_ST_G5) | BIT_MASK<uint16_t>(CMD_ST_G4) |
							   BIT_MASK<uint16_t>(CMD_ST_G3) | BIT_MASK<uint16_t>(CMD_ST_G2) |
							   BIT_MASK<uint16_t>(CMD_ST_G1) | BIT_MASK<uint16_t>(CMD_ST_G0))) >> CMD_ST_G0);
}

bool hp_taco_device::is_cmd_rd_wr(uint16_t cmd_reg)
{
	return BIT(cmd_reg , CMD_ST_G1);
}

bool hp_taco_device::is_cmd_rd(uint16_t cmd_reg)
{
	return is_cmd_rd_wr(cmd_reg) && BIT(cmd_reg , CMD_ST_G3);
}

bool hp_taco_device::is_cmd_wr(uint16_t cmd_reg)
{
	return is_cmd_rd_wr(cmd_reg) && !BIT(cmd_reg , CMD_ST_G3);
}

bool hp_taco_device::is_double_hole_cmd(uint16_t cmd_reg)
{
	return is_cmd_rd_wr(cmd_reg) || get_cmd(cmd_reg) == CMD_ERASE || get_cmd(cmd_reg) == CMD_WR_GAP_N_TACH;
}

void hp_taco_device::start_cmd_exec(uint16_t new_cmd_reg)
{
	LOG_DBG("New cmd %02x D=%d S=%d @ %g cmd %02x st %d\n" , get_cmd(new_cmd_reg) , BIT(new_cmd_reg , CMD_ST_FWD) , BIT(new_cmd_reg , CMD_ST_FST) , machine().time().as_double() , get_cmd(m_cmd_reg) , m_cmd_state);

	m_tape->update_speed_pos();

	unsigned new_cmd_code = get_cmd(new_cmd_reg);

	if (new_cmd_code == CMD_CLEAR_ST) {
		// Clear status
		set_error(false , false);
		sts_w(true);
		if (!m_tape->cart_out_r()) {
			BIT_CLR(m_status_reg, CMD_ST_CART_OUT);
		}
		BIT_CLR(m_status_reg, CMD_ST_HOLE);
		if (m_cmd_state != CMD_STOPPING) {
			m_cmd_state = CMD_IDLE;
		}
	} else {
		bool start_tape = true;
		bool clear_timers = true;
		hp_dc100_tape_device::tape_op_t op = hp_dc100_tape_device::OP_IDLE;

		if (m_gap_in_read) {
			set_error(false , false);
		}

		m_cmd_state = CMD_PH0;
		uint16_t prev_cmd_reg = m_cmd_reg;
		m_cmd_reg = new_cmd_reg;

		switch (new_cmd_code) {
		case CMD_INT_ON_GAP:
			// 1. Wait for tape to reach 22 ips
			// 2. Wait to reach a new gap
			// 3. Int & end
			break;

		case CMD_ERASE:
			// 1. Start erase op
			op = hp_dc100_tape_device::OP_ERASE;
			break;

		case CMD_WR:
		case CMD_WR_9825:
		case CMD_WR_SYNC:
		case CMD_WR_SYNC_9825:
		case CMD_WR_CSUM:
		case CMD_WR_CSUM_9825:
			// 1. Wait for tape to reach 22 ips
			// 2. Wait for data to begin (not for CMD_WR)
			// 3. Write words on tape
			if (is_cmd_rd_wr(prev_cmd_reg)) {
				// Skip synchronization (steps 1 & 2)
				m_cmd_state = CMD_PH2;
				start_wr();
				op = m_tape->get_op();
			} else {
				m_working_reg = PREAMBLE_WORD;
			}
			break;

		case CMD_STOP:
		case CMD_STOP_INT:
			// 1. Send stop to tape
			// 2. Wait for tape to stop, keep counting tach ticks
			// 3. When tape has stopped, int. (CMD_STOP_INT only)
			send_stop();
			m_cmd_state = CMD_STOPPING;
			start_tape = false;
			break;

		case CMD_SET_TRACK:
			// 1. Set A/B track
			if (BIT(new_cmd_reg , CMD_ST_MOD)) {
				BIT_SET(m_status_reg , CMD_ST_TRB);
				m_tape->set_track_no(1);
			} else {
				BIT_CLR(m_status_reg , CMD_ST_TRB);
				m_tape->set_track_no(0);
			}
			start_tape = false;
			break;

		case CMD_UNK_0D:
			// 1. Int & end
			start_tape = false;
			break;

		case CMD_MOVE:
			// 1. Start tape
			break;

		case CMD_OPP_DIR_N_TACH:
			// 1. Count tach ticks until tacho reg rolls over to 0
			// 2. Toggle FWD bit
			// 3. Wait for tape to reach 22 ips
			// 4. Int & end
			break;

		case CMD_STOP_IN_GAP:
		case CMD_STOP_IN_GAP1:
			// 1. Wait for tape to reach set point
			// 2. Wait to reach a new gap
			// 3. Wait for 256 tacho ticks
			// 4. Stop tape
			break;

		case CMD_WR_GAP_N_TACH:
			// 1. Erase tape until tacho reg overflows
			// 2. Int & end
			op = hp_dc100_tape_device::OP_ERASE;
			break;

		case CMD_INT_N_GAP:
			// 1. Wait for tape to reach set point
			// 2. Wait for N gaps
			// 3. Int & end
			break;

		case CMD_INT_N_TACH_22:
			// 1. Wait for tape to reach 22 ips
			// 2. Count tach ticks until tacho reg rolls over to 0
			// 3. Int & end
			break;

		case CMD_INT_STOP_N_TACH:
		case CMD_INT_STOP_N_TACH1:
			// 1. Count tach ticks until tacho reg rolls over to 0
			// 2. Stop tape
			break;

		case CMD_RD_6UPD:
		case CMD_RD_9825_6UPD:
		case CMD_RD_CSUM_6UPD:
		case CMD_RD_9825_CSUM6:
			// 1. Wait for tape to reach 22 ips
			// 2. Wait for preamble
			// 3. Read words
			if (is_cmd_rd_wr(prev_cmd_reg)) {
				// Skip synchronization (steps 1 & 2)
				m_cmd_state = CMD_PH2;
				start_rd();
				clear_timers = false;
				op = m_tape->get_op();
			}
			break;

		case CMD_INT_N_TACH:
			// 1. Count tach ticks until tacho reg rolls over to 0
			// 2. Int & end
			break;

		default:
			LOG("Unrecognized command %x\n" , new_cmd_code);
			start_tape = false;
			break;
		}

		if (start_tape) {
			if (m_error ||
				(BIT(m_status_reg , CMD_ST_HOLE) && !is_double_hole_cmd(new_cmd_reg)) ||
				BIT(m_status_reg , CMD_ST_CART_OUT)) {
				set_error(true , false);
				return;
			} else {
				send_go();
			}
		}
		if (clear_timers) {
			m_gap_timer->reset();
			m_evd_timer->reset();
		}
		m_tape->set_op(op);
		cmd_fsm();
	}
}
