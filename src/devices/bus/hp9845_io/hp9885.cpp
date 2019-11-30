// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9885.cpp

    HP9885M 8" floppy drive
    =======================

    This driver implements HLE of HP9885 floppy drive.
    The HP9885M is a single-disk 8" floppy drive. It connects to
    main system through a HP98032 GPIO module. The controller in a
    HP9885M can drive 3 more external HP9885S drives. The "M" or "S"
    in the name stand for master and slave, respectively.
    The enclosure of HP9885M contains the floppy drive, the controller
    electronics and the power supply whereas the HP9885S only has the
    drive and the power supply. A master unit interfaces to slave units
    through a standard daisy-chained Shugart bus.
    The controller is based on a HP Nanoprocessor with a 2 kB FW ROM.
    Unfortunately no dumps are available of the ROM, AFAIK, so the HLE
    is needed.
    The HP9885 supports a single disk format having these characteristics:
    - Single side
    - HP MMFM modulation
    - 30 256-byte sectors per track
    - 77 tracks (only 67 are actually used)
    - 360 RPM
    - A total capacity of 514560 bytes per disk.

    Summary of the command words I identified
    =========================================

    *READ*

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----+-----+-----------------------------------+
    | 0  X|Unit#|       Sector count                |
    +-----+-----+-----------------------------------+

    Bit 14 selects tighter margin for data reading (not emulated here)

    *WRITE*

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----+-----+-----------------------------------+
    | 1  0|Unit#|       Sector count                |
    +-----+-----+-----------------------------------+

    *SEEK*

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----+-----+--------------------+--------------+
    | 1  1|Unit#|  Track # [0..76]   |Sector#[0..29]|
    +-----+-----+--------------------+--------------+

    *FORMAT TRACK* (not implemented yet)

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----+-----+--------------------+--------------+
    | 1  1|Unit#|  Track # [0..76]   |     0x1e     |
    +-----+-----+--------------------+--------------+

    *STEP IN*

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----+-----+--------------------+--------------+
    | 1  1|Unit#|         0x7c       |     0x1f     |
    +-----+-----+--------------------+--------------+

    *ERASE TRACK* (not implemented yet)

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----+-----+--------------------+--------------+
    | 1  1|Unit#|         0x7d       |     0x1f     |
    +-----+-----+--------------------+--------------+

    *READ STATUS*

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----+-----+--------------------+--------------+
    | 1  1|Unit#|         0x7f       |     0x1f     |
    +-----+-----+--------------------+--------------+

    This is the structure of the status word:

    +-----------------------------------------------+
    |15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0|
    +-----------------------+--+--+--+--+--+--+-----+
    |    Error code (0=OK)  |--|XC|SC|NR|WP|DC|Unit#|
    +-----------------------+--+--+--+--+--+--+-----+

    DC: Disk changed
    WP: Write protection
    NR: Not ready
    SC: Seek completed
    XC: Transfer completed

    Usage on HP9825
    ===============

    -slot0 98032_gpio -slot0:98032_gpio:gpio hp9885 -flop <floppy image>

    Usage on HP9845
    ===============

    -slot0 98032_gpio -slot0:98032_gpio:gpio hp9885 -rom1 massd -flop <floppy image>

    Note on floppy images
    =====================

    Images can be in two formats: MFI & HPI.
    A pre-formatted image must be used as formatting is not supported.
    It's not possible to format images for two reasons: the format
    command is not implemented yet and no dump of the disk system tape
    cartridge (HP part no. 09885-90035) is available.
    The latter is needed during format operation to create the so-called
    bootstraps on disk, which in turn are needed by 9825 systems.
    A pre-formatted image (9825_empty.hpi) is available here:
    http://www.hpmuseum.net/software/9825_discs.zip

    TODO
    ====

    + Implement missing commands
    + PRESET

    Acknowledgments
    ===============

    Thanks to Dyke Shaffer for publishing a lot of HP internal docs
    and source files regarding the HP9885.

    Fun fact: data I/O between disk and 98032 module is not buffered
    as there is no RAM in the controller. DMA must be used to keep
    data flow at disk speed and avoid underruns. The nominal disk
    data rate is one word every 32 µs.

*********************************************************************/

#include "emu.h"
#include "hp9885.h"
#include "formats/hpi_dsk.h"

// Debugging
#include "logmacro.h"
#define LOG_TIMER_MASK  (LOG_GENERAL << 1)
#define LOG_TIMER(...)  LOGMASKED(LOG_TIMER_MASK, __VA_ARGS__)
#define LOG_HS_MASK     (LOG_TIMER_MASK << 1)
#define LOG_HS(...)     LOGMASKED(LOG_HS_MASK, __VA_ARGS__)
#define LOG_HEAD_MASK   (LOG_HS_MASK << 1)
#define LOG_HEAD(...)   LOGMASKED(LOG_HEAD_MASK, __VA_ARGS__)
#define LOG_DISK_MASK   (LOG_HEAD_MASK << 1)
#define LOG_DISK(...)   LOGMASKED(LOG_DISK_MASK, __VA_ARGS__)

#undef VERBOSE
//#define VERBOSE (LOG_GENERAL | LOG_HS_MASK | LOG_HEAD_MASK)
#define VERBOSE 0

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

// device type definition
DEFINE_DEVICE_TYPE(HP9885, hp9885_device, "hp9885" , "HP9885 floppy drive")

// Timers
enum {
	FSM_TMR_ID,
	HEAD_TMR_ID,
	BIT_BYTE_TMR_ID
};

// Constants
constexpr unsigned MAX_TRACK    = 76;   // Maximum valid track
constexpr unsigned MAX_SECTOR   = 29;   // Maximum valid sector
constexpr unsigned UNKNOWN_TRACK= 0xff; // Current track unknown
constexpr unsigned STEP_MS      = 8;    // Step time (ms)
constexpr unsigned SETTLING_MS  = 8;    // Settling time (ms)
constexpr unsigned HEAD_TO_MS   = 415;  // Head unload timeout (ms)
constexpr unsigned HD_SETTLE_MS = 50;   // Head settling time (ms)
constexpr uint16_t PASSWORD     = 0xae87;   // "Password" to enable commands
constexpr unsigned HALF_CELL_US = 1;    // Half bit cell duration (µs)
constexpr unsigned STATUS_DELAY_US  = 100;  // Status delay (µs)
constexpr unsigned MISSED_ID_REVS   = 2;    // Disk rotations to stop ID search

// Bits in status word
constexpr unsigned STS_DISK_CHANGED = 2;    // Disk changed
constexpr unsigned STS_WRITE_PROTECT= 3;    // Write protection
constexpr unsigned STS_NOT_RDY      = 4;    // Drive not ready
constexpr unsigned STS_SEEK_COMPLETE= 5;    // Seek completed
constexpr unsigned STS_XFER_COMPLETE= 6;    // Data transfer completed

// Error codes
enum : unsigned {
	ERR_NONE = 0,
	ERR_NOT_POWERED = 1,
	ERR_DOOR_OPEN = 2,
	ERR_NO_DISK = 3,
	ERR_WR_DISABLED = 4,
	ERR_ID_ERROR = 5,
	ERR_TRACK_ERROR = 6,
	ERR_CRC_ERROR = 7,
	ERR_HW_FAILURE = 8
};

hp9885_device::hp9885_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP9885 , tag , owner , clock)
	, device_hp98032_gpio_interface(mconfig, *this)
	, m_drive_connector{*this , "floppy"}
{
}

hp9885_device::~hp9885_device()
{
}

uint16_t hp9885_device::get_jumpers() const
{
	return hp98032_gpio_slot_device::JUMPER_2 |
		hp98032_gpio_slot_device::JUMPER_7 |
		hp98032_gpio_slot_device::JUMPER_8 |
		hp98032_gpio_slot_device::JUMPER_B |
		hp98032_gpio_slot_device::JUMPER_E |
		hp98032_gpio_slot_device::JUMPER_F;
}

uint16_t hp9885_device::input_r() const
{
	uint16_t tmp = m_input;
	if (m_obf) {
		tmp |= m_output;
		LOG_HS("DATA OUT %04x\n" , tmp);
	}
	return tmp;
}

uint8_t hp9885_device::ext_status_r() const
{
	return 0;
}

void hp9885_device::output_w(uint16_t data)
{
	m_input = data;
}

void hp9885_device::ext_control_w(uint8_t data)
{
	LOG_HS("EXT CTRL %u\n" , data);
	if (BIT(data , 0) &&
		!BIT(m_status , STS_XFER_COMPLETE) &&
		(m_op == OP_READ || m_op == OP_WRITE)) {
		// CTL0 terminates current data transfer
		LOG("xfer terminated\n");
		BIT_SET(m_status , STS_XFER_COMPLETE);
		// Prepare to output status
		set_output();
	}
}

WRITE_LINE_MEMBER(hp9885_device::pctl_w)
{
	m_pctl = state;
	if (m_pctl) {
		if (!m_outputting) {
			set_ibf(true);
		}
	} else {
		LOG_HS("DATA IN %04x\n" , m_input);
		new_word();
	}
	update_busy();
}

WRITE_LINE_MEMBER(hp9885_device::io_w)
{
	LOG_HS("I/O = %d\n" , state);
}

WRITE_LINE_MEMBER(hp9885_device::preset_w)
{
	LOG("PRESET = %d\n" , state);
}

static const floppy_format_type hp9885_floppy_formats[] = {
	FLOPPY_MFI_FORMAT,
	FLOPPY_HPI_FORMAT,
	nullptr
};

void hp9885_device::device_add_mconfig(machine_config &config)
{
	FLOPPY_CONNECTOR(config , "floppy" , "8ssdd" , FLOPPY_8_SSDD , true , hp9885_floppy_formats).set_fixed(true);
}

void hp9885_device::device_start()
{
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_status));
	save_item(NAME(m_fsm_state));
	save_item(NAME(m_head_state));
	save_item(NAME(m_op));
	save_item(NAME(m_pctl));
	save_item(NAME(m_ibf));
	save_item(NAME(m_obf));
	save_item(NAME(m_outputting));
	save_item(NAME(m_had_transition));
	save_item(NAME(m_dskchg));
	save_item(NAME(m_track));
	save_item(NAME(m_seek_track));
	save_item(NAME(m_seek_sector));
	save_item(NAME(m_sector_cnt));
	save_item(NAME(m_word_cnt));
	save_item(NAME(m_rev_cnt));
	save_item(NAME(m_am_detector));
	save_item(NAME(m_crc));

	m_fsm_timer = timer_alloc(FSM_TMR_ID);
	m_head_timer = timer_alloc(HEAD_TMR_ID);
	m_bit_byte_timer = timer_alloc(BIT_BYTE_TMR_ID);

	m_drive = m_drive_connector->get_device();

	m_drive->setup_ready_cb(floppy_image_device::ready_cb(&hp9885_device::floppy_ready_cb , this));
	m_drive->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp9885_device::floppy_index_cb , this));
}

void hp9885_device::device_reset()
{
	eir_w(0);
	psts_w(1);
	m_dskchg = true;
	m_obf = false;
	recalibrate();
	m_seek_track = 0;
	m_seek_sector = 0;
	m_fsm_state = FSM_RECALIBRATING;
	set_state(FSM_IDLE);
}

void hp9885_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG_TIMER("Tmr %.06f ID %d FSM %d HD %d\n" , machine().time().as_double() , id , m_fsm_state , m_head_state);

	switch (id) {
	case FSM_TMR_ID:
		do_FSM();
		break;

	case HEAD_TMR_ID:
		if (m_head_state == HEAD_SETTLING) {
			LOG_HEAD("%.06f Head loaded\n" , machine().time().as_double());
			m_head_state = HEAD_LOADED;
			// Trigger actions to be done on head loading
			do_FSM();
			m_head_timer->adjust(attotime::from_msec(HEAD_TO_MS - HD_SETTLE_MS));
		} else {
			LOG_HEAD("%.06f Head unloaded\n" , machine().time().as_double());
			m_head_state = HEAD_UNLOADED;
		}
		break;

	case BIT_BYTE_TMR_ID:
		{
			switch (m_fsm_state) {
			case FSM_WAIT_ID_AM:
			case FSM_WAIT_DATA_AM:
				{
					attotime edge;
					attotime tm;
					edge = m_drive->get_next_transition(m_pll.ctime);
					bool half_bit = m_pll.feed_read_data(tm , edge , attotime::never);
					m_am_detector <<= 1;
					m_am_detector |= half_bit;
					if (m_am_detector == 0x55552a54) {
						// ID AM
						// CDCDCDCDCDCDCDCD
						//  0 0 0 0 1 1 1 0
						// 0 1 1 1 0 0 0 0
						LOG_DISK("Got ID AM\n");
						preset_crc();
						m_word_cnt = 2;
						set_state(FSM_RD_ID);
					} else if (m_am_detector == 0x55552a44) {
						// DATA AM
						// CDCDCDCDCDCDCDCD
						//  0 0 0 0 1 0 1 0
						// 0 1 1 1 0 0 0 0
						LOG_DISK("Got Data AM\n");
						if (m_fsm_state == FSM_WAIT_DATA_AM) {
							m_rev_cnt = 0;
							if (BIT(m_status , STS_XFER_COMPLETE)) {
								output_status();
								return;
							} else {
								preset_crc();
								if (m_op == OP_READ) {
									m_word_cnt = 129;
									set_state(FSM_RD_DATA);
								} else {
									m_word_cnt = 130;
									set_state(FSM_WR_DATA);
									m_pll.start_writing(m_pll.ctime);
									m_had_transition = false;
									wr_word(m_input);
									set_ibf(false);
								}
							}
						}
					}
				}
				break;

			case FSM_RD_ID:
				{
					// This is needed when state is switched to one of the AM waiting states
					m_am_detector = 0;
					auto word = rd_word();
					m_word_cnt--;
					LOG_DISK("W %04x C %u\n" , word , m_word_cnt);
					if (m_word_cnt && word != ((m_seek_sector << 8) | m_track)) {
						set_state(FSM_WAIT_ID_AM);
					} else if (m_word_cnt == 0) {
						if (m_crc) {
							LOG_DISK("Wrong CRC in ID\n");
							set_state(FSM_WAIT_ID_AM);
						} else {
							LOG_DISK("Sector found\n");
							set_state(FSM_WAIT_DATA_AM);
						}
					}
				}
				break;

			case FSM_RD_DATA:
				{
					auto word = rd_word();
					m_word_cnt--;
					LOG_DISK("W %04x C %u\n" , word , m_word_cnt);
					if (m_word_cnt >= 1) {
						if (!BIT(m_status , STS_XFER_COMPLETE)) {
							m_output = word;
							m_obf = true;
							update_busy();
						}
					} else if (m_word_cnt == 0) {
						if (m_crc) {
							LOG_DISK("Wrong CRC in data\n");
						}
						// Move to next sector
						adv_sector();
						if (BIT(m_status , STS_XFER_COMPLETE) || m_sector_cnt == 0) {
							BIT_SET(m_status , STS_XFER_COMPLETE);
							output_status();
						} else {
							set_state(FSM_POSITIONING);
							do_FSM();
						}
						return;
					}
				}
				break;

			case FSM_WR_DATA:
				{
					m_word_cnt--;
					if (m_word_cnt > 2) {
						if (BIT(m_status , STS_XFER_COMPLETE)) {
							wr_word(0);
						} else {
							wr_word(m_input);
							if (m_word_cnt > 3) {
								set_ibf(false);
							}
						}
					} else if (m_word_cnt == 2) {
						wr_word(m_crc);
					} else if (m_word_cnt == 1) {
						// Post-amble
						wr_word(0);
					} else {
						m_pll.stop_writing(m_drive , m_pll.ctime);
						// Move to next sector
						adv_sector();
						if (BIT(m_status , STS_XFER_COMPLETE) || m_sector_cnt == 0) {
							BIT_SET(m_status , STS_XFER_COMPLETE);
							output_status();
						} else {
							set_ibf(false);
							set_state(FSM_POSITIONING);
							do_FSM();
						}
						return;
					}
				}
				break;

			default:
				LOG("Invalid FSM state %d\n" , m_fsm_state);
				set_state(FSM_IDLE);
				return;
			}
			timer.adjust(m_pll.ctime - machine().time());
		}
		break;
	}
}

void hp9885_device::floppy_ready_cb(floppy_image_device *floppy , int state)
{
	LOG("ready %d\n" , state);
	if (state) {
		// drive not ready
		m_dskchg = true;
	}
}

void hp9885_device::floppy_index_cb(floppy_image_device *floppy , int state)
{
	if (state && m_rev_cnt && --m_rev_cnt == 0) {
		// Sector not found
		LOG("Sector not found\n");
		stop_rdwr();
		set_error(ERR_ID_ERROR);
		output_status(true);
	}
}

void hp9885_device::set_state(int new_state)
{
	if (m_fsm_state != new_state) {
		LOG("%.06f FSM %d->%d\n" , machine().time().as_double() , m_fsm_state , new_state);
		m_fsm_state = new_state;
		if (m_fsm_state == FSM_IDLE) {
			m_op = OP_NONE;
			m_outputting = false;
			set_ibf(false);
			m_fsm_timer->reset();
			stop_rdwr();
			m_rev_cnt = 0;
		}
	}
}

void hp9885_device::init_status(unsigned unit_no)
{
	m_status = unit_no & 3;
	if (unit_no == 0) {
		if (m_drive->ready_r()) {
			BIT_SET(m_status , STS_NOT_RDY);
		}
		if (m_dskchg) {
			BIT_SET(m_status , STS_DISK_CHANGED);
		}
		if (m_drive->wpt_r()) {
			BIT_SET(m_status , STS_WRITE_PROTECT);
		}
	} else {
		// Units 1,2,3 are not present
		BIT_SET(m_status , STS_NOT_RDY);
	}
}

void hp9885_device::encode_error(bool writing)
{
	if (m_status & 3) {
		set_error(ERR_NOT_POWERED);
	} else if (writing && BIT(m_status , STS_WRITE_PROTECT)) {
		set_error(ERR_WR_DISABLED);
	} else if (BIT(m_status , STS_NOT_RDY)) {
		set_error(ERR_NO_DISK);
	}
}

void hp9885_device::set_error(unsigned error_code)
{
	m_status = (m_status & 0xff) | (error_code << 8);
	if (error_code != ERR_NONE) {
		LOG_HS("EIR 1\n");
		eir_w(1);
		psts_w(0);
	} else {
		LOG_HS("EIR 0\n");
		eir_w(0);
		psts_w(1);
	}
}

void hp9885_device::new_word()
{
	unsigned unit_no = (m_input >> 12) & 3;

	switch (m_fsm_state) {
	case FSM_IDLE:
		if (m_input == PASSWORD) {
			LOG("Got PW\n");
			set_state(FSM_GOT_PW);
		} else {
			LOG("Wrong sequence\n");
			// TODO:
			// It probably does nothing IRL
		}
		set_ibf(false);
		break;

	case FSM_GOT_PW:
		// Decode new command
		switch (m_input & 0xc000) {
		case 0x0000:
		case 0x4000:
			// Read
			init_status(unit_no);
			if (!BIT(m_status , STS_NOT_RDY)) {
				m_sector_cnt = m_input & 0x0fff;
				LOG("Read %u sectors @%u:%u\n" , m_sector_cnt , m_seek_track , m_seek_sector);
				m_op = OP_READ;
				set_state(FSM_POSITIONING);
				if (load_head()) {
					m_fsm_timer->adjust(attotime::zero);
				}
				set_output();
			} else {
				encode_error(false);
				output_status();
			}
			break;

			case 0x8000:
				// Write
				init_status(unit_no);
				if (!BIT(m_status , STS_NOT_RDY) && !BIT(m_status , STS_WRITE_PROTECT)) {
					m_sector_cnt = m_input & 0x0fff;
					LOG("Write %u sectors @%u:%u\n" , m_sector_cnt , m_seek_track , m_seek_sector);
					m_op = OP_WRITE;
					set_state(FSM_POSITIONING);
					if (load_head()) {
						m_fsm_timer->adjust(attotime::zero);
					}
					set_ibf(false);
				} else {
					encode_error(true);
					output_status(true);
				}
				break;

			case 0xc000:
				{
					// Seek & other commands
					uint8_t track_no = (m_input >> 5) & 0x7f;
					uint8_t sect_no = m_input & 0x1f;
					if (sect_no == 0x1e) {
						// Format
						LOG("Format\n");
						// TODO:
					} else if (sect_no == 0x1f) {
						switch (track_no) {
						case 0x7c:
							// Step in
							init_status(unit_no);
							if (!BIT(m_status , STS_NOT_RDY)) {
								LOG("Step in\n");
								m_seek_track = m_track + 1;
								m_op = OP_STEP_IN;
								set_state(FSM_SEEKING);
								if (load_head()) {
									m_fsm_timer->adjust(attotime::zero);
								}
							} else {
								encode_error(false);
								output_status();
							}
							break;

						case 0x7d:
							// Write all track
							LOG("Write all track\n");
							// TODO:
							break;

						case 0x7f:
							// Read status
							init_status(unit_no);
							if (!BIT(m_status , STS_NOT_RDY) && BIT(m_status , STS_DISK_CHANGED)) {
								LOG("Get status DSKCHG\n");
								m_op = OP_GET_STATUS;
								set_state(FSM_POSITIONING);
								if (load_head()) {
									m_fsm_timer->adjust(attotime::zero);
								}
							} else {
								LOG("Get status !DSKCHG\n");
								encode_error(false);
								output_status();
							}
							break;

						default:
							LOG("Unknown command %02x\n" , track_no);
						}
					} else {
						// Plain seek
						LOG("Seek to %u:%u\n" , track_no , sect_no);
						m_seek_track = track_no;
						m_seek_sector = sect_no;
						set_state(FSM_IDLE);
					}
				}
				break;
			}
		break;

	case FSM_RD_STATUS1:
		set_error(ERR_NONE);
		set_state(FSM_RD_STATUS2);
		m_outputting = false;
		m_obf = false;
		break;

	case FSM_RD_STATUS2:
		set_state(FSM_IDLE);
		break;

	case FSM_RD_DATA:
		m_obf = false;
		break;

	default:
		if (m_op != OP_WRITE) {
			LOG("Got data in state %d!\n" , m_fsm_state);
		}
	}
}

void hp9885_device::do_FSM()
{
	switch (m_fsm_state) {
	case FSM_RECALIBRATING:
		// Keep head loaded
		load_head();
		if (m_drive->trk00_r()) {
			one_step(true);
		} else {
			set_state(FSM_POSITIONING);
			m_fsm_timer->adjust(attotime::from_msec(SETTLING_MS));
			m_track = 0;
		}
		break;

	case FSM_SETTLING:
		if (m_op == OP_READ || m_op == OP_WRITE) {
			// Set seek complete
			BIT_SET(m_status , STS_SEEK_COMPLETE);
			if (m_sector_cnt--) {
				m_rev_cnt = MISSED_ID_REVS;
				set_state(FSM_WAIT_ID_AM);
				start_rd();
			} else {
				output_status();
			}
		} else if (m_op == OP_STEP_IN) {
			// Step IN
			// Set seek complete
			BIT_SET(m_status , STS_SEEK_COMPLETE);
			output_status();
		} else {
			// Get status
			output_status();
		}
		break;

	case FSM_POSITIONING:
	case FSM_SEEKING:
		// Keep head loaded
		load_head();
		// Need recalibration?
		if (m_track == UNKNOWN_TRACK) {
			set_state(FSM_RECALIBRATING);
			m_fsm_timer->adjust(attotime::zero);
		} else if (m_seek_track != m_track) {
			set_state(FSM_SEEKING);
			one_step(m_seek_track < m_track);
		} else {
			if (m_fsm_state == FSM_SEEKING) {
				m_fsm_timer->adjust(attotime::from_msec(SETTLING_MS));
			} else {
				m_fsm_timer->adjust(attotime::zero);
			}
			set_state(FSM_SETTLING);
		}
		break;

	case FSM_STATUS_DELAY:
		output_status();
		break;

	default:
		LOG("Invalid state=%d\n" , m_fsm_state);
		set_state(FSM_IDLE);
	}
	update_busy();
}

bool hp9885_device::load_head()
{
	m_dskchg = false;

	switch (m_head_state) {
	case HEAD_UNLOADED:
	case HEAD_SETTLING:
		LOG_HEAD("%.06f Loading head..\n" , machine().time().as_double());
		m_head_state = HEAD_SETTLING;
		m_head_timer->adjust(attotime::from_msec(HD_SETTLE_MS));
		return false;

	case HEAD_LOADED:
		LOG_HEAD("%.06f Keep head loaded\n" , machine().time().as_double());
		m_head_timer->adjust(attotime::from_msec(HEAD_TO_MS));
		return true;

	default:
		LOG("Invalid head state %d\n" , m_head_state);
		m_head_state = HEAD_UNLOADED;
		return false;
	}
}

void hp9885_device::recalibrate()
{
	m_track = UNKNOWN_TRACK;
}

void hp9885_device::one_step(bool outward)
{
	if (outward) {
		if (m_track > 0) {
			m_track--;
		}
	} else {
		if (m_track < MAX_TRACK) {
			m_track++;
		}
	}
	LOG_HEAD("%.06f Step to trk %u\n" , machine().time().as_double() , m_track);
	m_drive->dir_w(outward);
	m_drive->stp_w(0);
	m_drive->stp_w(1);
	m_fsm_timer->adjust(attotime::from_msec(STEP_MS));
}

void hp9885_device::adv_sector()
{
	if (++m_seek_sector > MAX_SECTOR) {
		m_seek_sector = 0;
		if (m_seek_track < MAX_TRACK) {
			m_seek_track++;
		}
	}
}

void hp9885_device::start_rd()
{
	m_pll.set_clock(attotime::from_usec(HALF_CELL_US));
	m_pll.read_reset(machine().time());
	m_bit_byte_timer->adjust(attotime::zero);
	m_am_detector = 0;
}

void hp9885_device::stop_rdwr()
{
	m_bit_byte_timer->reset();
}

uint16_t hp9885_device::rd_word()
{
	uint16_t word = 0;
	for (unsigned i = 0; i < 16; ++i) {
		attotime edge;
		attotime tm;
		edge = m_drive->get_next_transition(m_pll.ctime);
		// Read & discard clock bit
		m_pll.feed_read_data(tm , edge , attotime::never);
		edge = m_drive->get_next_transition(m_pll.ctime);
		bool data_bit = m_pll.feed_read_data(tm , edge , attotime::never);
		word >>= 1;
		if (data_bit) {
			BIT_SET(word , 15);
		}
		update_crc(data_bit);
	}
	return word;
}

void hp9885_device::wr_word(uint16_t word)
{
	for (unsigned i = 0; i < 16; ++i) {
		bool data_bit = BIT(word , i);
		bool clock_bit = !data_bit && !m_had_transition;
		m_had_transition = data_bit || clock_bit;
		attotime dummy;

		m_pll.write_next_bit(clock_bit , dummy , nullptr , attotime::never);
		m_pll.write_next_bit(data_bit , dummy , nullptr , attotime::never);
		update_crc(data_bit);
	}
	m_pll.commit(m_drive , m_pll.ctime);
}

void hp9885_device::preset_crc()
{
	m_crc = ~0;
}

void hp9885_device::update_crc(bool bit)
{
	bool crc_x15 = BIT(m_crc , 0);
	m_crc >>= 1;
	if (bit ^ crc_x15) {
		m_crc ^= 0x8408;
	}
}

void hp9885_device::set_ibf(bool state)
{
	m_ibf = state;
	update_busy();
}

void hp9885_device::set_output()
{
	m_outputting = true;
	m_obf = false;
	set_ibf(false);
}

void hp9885_device::output_status(bool delayed)
{
	stop_rdwr();
	if (delayed) {
		set_output();
		set_state(FSM_STATUS_DELAY);
		m_fsm_timer->adjust(attotime::from_usec(STATUS_DELAY_US));
	} else {
		set_state(FSM_RD_STATUS1);
		m_outputting = true;
		m_obf = true;
		// Set status in output buffer
		m_output = m_status;
		set_ibf(false);
	}
}

void hp9885_device::update_busy()
{
	bool busy = (!m_outputting && m_ibf) || (m_outputting && m_pctl && m_obf);
	LOG_HS("PCTL %d BUSY %d OUT %d IBF %d OBF %d\n" , m_pctl , busy , m_outputting , m_ibf , m_obf);
	pflg_w(!busy);
}
