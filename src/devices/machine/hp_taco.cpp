// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp_taco.cpp

    HP TApe COntroller (5006-3012)

*********************************************************************/

// Documentation I used:
// [1]  HP, manual 64940-90905, may 80 rev. - Model 64940A tape control & drive service manual
// [2]  US patent 4,075,679 describing HP9825 system (this system had a discrete implementation of tape controller)

// Format of TACO command/status register (R5)
// Bit    R/W Content
// ===============
// 15     RW  Tape direction (1 = forward)
// 14..10 RW  Command
//  9     RW  ? Drive ON according to [1], doesn't match usage of firmware
//  8     RW  ? Size of gaps according to [1]
//  7     RW  Speed of tape (1 = 90 ips, 0 = 22 ips)
//  6     RW  Option bit for various commands
//  5     R   Current track (1 = B)
//  4     R   Gap detected (1)
//  3     R   Write protection (1)
//  2     R   Servo failure (1)
//  1     R   Cartridge out (1)
//  0     R   Hole detected (1)

// TODO: R6 è modificato durante il conteggio impulsi? Viene azzerato alla lettura?

#include "emu.h"
#include "hp_taco.h"

// Debugging
#define VERBOSE 1
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

// Macros to clear/set single bits
#define BIT_MASK(n) (1U << (n))
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Timers
enum {
        TAPE_TMR_ID
};

// Constants
#define CMD_REG_MASK    0xffc0  // Command register mask
#define STATUS_REG_MASK 0x003f  // Status register mask
#define STATUS_ERR_MASK 0x0002  // Mask of errors in status reg.
#define TACH_TICKS_PER_INCH     968     // Tachometer pulses per inch of tape movement
#define TACH_FREQ_SLOW  21276   // Tachometer pulse frequency for slow speed (21.98 ips)
#define TACH_FREQ_FAST  87196   // Tachometer pulse frequency for fast speed (90.08 ips)
#define TAPE_LENGTH     ((140 * 12 + 72 * 2) * TACH_TICKS_PER_INCH)        // Tape length (in tachometer pulses): 140 ft of usable tape + 72" of punched tape at either end
#define TAPE_INIT_POS   (80 * TACH_TICKS_PER_INCH)      // Initial tape position: 80" from beginning (just past the punched part)
#define QUICK_CMD_USEC  10      // usec for "quick" command execution

// Parts of command register
#define CMD_CODE(reg) \
        (((reg) >> 10) & 0x1f)
#define DIR_FWD(reg) \
        (BIT(reg , 15))
#define SPEED_FAST(reg) \
        (BIT(reg , 7))
#define CMD_OPT(reg) \
        (BIT(reg , 6))

// Commands
enum {
        CMD_ALIGN_0,            // 00: header alignment (?)
        CMD_UNK_01,             // 01: unknown
        CMD_FINAL_GAP,          // 02: write final gap
        CMD_INIT_WRITE,         // 03: write words for tape formatting
        CMD_STOP,               // 04: stop
        CMD_UNK_05,             // 05: unknown
        CMD_SET_TRACK,          // 06: set A/B track
        CMD_UNK_07,             // 07: unknown
        CMD_UNK_08,             // 08: unknown
        CMD_UNK_09,             // 09: unknown
        CMD_MOVE,               // 0a: move tape
        CMD_UNK_0b,             // 0b: unknown
        CMD_UNK_0c,             // 0c: unknown*
        CMD_UNK_0d,             // 0d: unknown
        CMD_CLEAR,              // 0e: clear errors/unlatch status bits
        CMD_UNK_0f,             // 0f: unknown
        CMD_ALIGN_PREAMBLE,     // 10: align to end of preamble (?)
        CMD_UNK_11,             // 11: unknown
        CMD_UNK_12,             // 12: unknown
        CMD_UNK_13,             // 13: unknown
        CMD_UNK_14,             // 14: unknown
        CMD_UNK_15,             // 15: unknown
        CMD_WRITE_IRG,          // 16: write inter-record gap
        CMD_UNK_17,             // 17: unknown
        CMD_SCAN_RECORDS,       // 18: scan records (count IRGs)
        CMD_RECORD_WRITE,       // 19: write record words
        CMD_UNK_MOVE,           // 1a: some kind of tape movement
        CMD_UNK_1b,             // 1b: unknown
        CMD_DELTA_MOVE_REC,     // 1c: move tape a given distance (optionally stop at 1st record) (?)
        CMD_START_READ,         // 1d: start record reading
        CMD_DELTA_MOVE_IRG,     // 1e: move tape a given distance (optionally stop at 1st IRG)
        CMD_END_READ            // 1f: stop reading
};

// Bits of status register
#define STATUS_HOLE_BIT         0       // Hole detected
#define STATUS_CART_OUT_BIT     1       // Cartridge out
#define STATUS_SFAIL_BIT        2       // Servo failure
#define STATUS_WPR_BIT          3       // Write protection
#define STATUS_GAP_BIT          4       // Gap detected
#define STATUS_TRACKB_BIT       5       // Track B selected

// *** Position of tape holes ***
// At beginning of tape:
// *START*
// |<-----24"----->|<---12"--->|<---12"--->|<-----24"----->|
//               O   O       O   O       O   O             O
//               |<->|       |<->|       |<->|
//               0.218"      0.218"      0.218"
// At end of tape:
//                                                     *END*
// |<-----24"----->|<---12"--->|<---12"--->|<-----24"----->|
// O               O           O           O
//
static const hp_taco_device::tape_pos_t tape_holes[] = {
        (hp_taco_device::tape_pos_t)(23.891 * TACH_TICKS_PER_INCH),     // 24 - 0.218 / 2
        (hp_taco_device::tape_pos_t)(24.109 * TACH_TICKS_PER_INCH),     // 24 + 0.218 / 2
        (hp_taco_device::tape_pos_t)(35.891 * TACH_TICKS_PER_INCH),     // 36 - 0.218 / 2
        (hp_taco_device::tape_pos_t)(36.109 * TACH_TICKS_PER_INCH),     // 36 + 0.218 / 2
        (hp_taco_device::tape_pos_t)(47.891 * TACH_TICKS_PER_INCH),     // 48 - 0.218 / 2
        (hp_taco_device::tape_pos_t)(48.109 * TACH_TICKS_PER_INCH),     // 48 + 0.218 / 2
        72 * TACH_TICKS_PER_INCH,       // 72
        1752 * TACH_TICKS_PER_INCH,     // 1752
        1776 * TACH_TICKS_PER_INCH,     // 1776
        1788 * TACH_TICKS_PER_INCH,     // 1788
        1800 * TACH_TICKS_PER_INCH      // 1800
};

// Device type definition
const device_type HP_TACO = &device_creator<hp_taco_device>;

// Constructors
hp_taco_device::hp_taco_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname)
        : device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
          m_irq_handler(*this),
          m_flg_handler(*this),
          m_sts_handler(*this),
          m_data_reg(0),
          m_cmd_reg(0),
          m_status_reg(0),
          m_tach_reg(0),
          m_checksum_reg(0),
          m_timing_reg(0),
          m_tape_pos(TAPE_INIT_POS)
{
}

hp_taco_device::hp_taco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
        : device_t(mconfig, HP_TACO, "HP TACO", tag, owner, clock, "TACO", __FILE__),
          m_irq_handler(*this),
          m_flg_handler(*this),
          m_sts_handler(*this),
          m_data_reg(0),
          m_cmd_reg(0),
          m_status_reg(0),
          m_tach_reg(0),
          m_checksum_reg(0),
          m_timing_reg(0),
          m_tape_pos(TAPE_INIT_POS)
{
}

WRITE16_MEMBER(hp_taco_device::reg_w)
{
        LOG(("wr R%u = %04x\n", 4 + offset , data));

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
                // Timing register
                m_timing_reg = data;
                break;
        }
}

READ16_MEMBER(hp_taco_device::reg_r)
{
        UINT16 res = 0;

        // Any I/O activity clears IRQ
        irq_w(false);

        switch (offset) {
        case 0:
                // Data register
                res = m_data_reg;
                break;

        case 1:
                // Command & status register
                res = (m_cmd_reg & CMD_REG_MASK) | (m_status_reg & STATUS_REG_MASK);
                break;

        case 2:
                // Tachometer register
                res = m_tach_reg;
                break;

        case 3:
                // Checksum register
                res = m_checksum_reg;
                m_checksum_reg = 0;
                break;
        }

        LOG(("rd R%u = %04x\n", 4 + offset , res));

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

// device_start
void hp_taco_device::device_start()
{
        m_irq_handler.resolve_safe();
        m_flg_handler.resolve_safe();
        m_sts_handler.resolve_safe();

        save_item(NAME(m_data_reg));
        save_item(NAME(m_cmd_reg));
        save_item(NAME(m_status_reg));
        save_item(NAME(m_tach_reg));
        save_item(NAME(m_checksum_reg));
        save_item(NAME(m_timing_reg));
        save_item(NAME(m_irq));
        save_item(NAME(m_flg));
        save_item(NAME(m_sts));
        save_item(NAME(m_tape_pos));
        save_item(NAME(m_start_time));

        m_tape_timer = timer_alloc(TAPE_TMR_ID);
}

// device_reset
void hp_taco_device::device_reset()
{
        m_data_reg = 0;
        m_cmd_reg = 0;
        m_status_reg = 0;
        m_tach_reg = 0;
        m_checksum_reg = 0;
        m_timing_reg = 0;
        m_start_time = attotime::never;

        m_irq = false;
        m_flg = true;

        m_irq_handler(false);
        m_flg_handler(true);
        set_error(false);
}

void hp_taco_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
        switch (id) {
        case TAPE_TMR_ID:
                LOG(("Tape tmr @%g\n" , machine().time().as_double()));
                switch (CMD_CODE(m_cmd_reg)) {
                case CMD_MOVE:
                        // Generate an interrupt each time a hole is crossed (tape doesn't stop)
                        update_tape_pos();
                        m_tape_timer->adjust(time_to_target(next_hole(DIR_FWD(m_cmd_reg)) , SPEED_FAST(m_cmd_reg)));
                        BIT_SET(m_status_reg, STATUS_HOLE_BIT);
                        break;

                case CMD_DELTA_MOVE_REC:
                case CMD_DELTA_MOVE_IRG:
                        // Interrupt & stop at end of movement
                        stop_tape();
                        break;

                default:
                        // Other commands: just raise irq
                        break;
                }
                irq_w(true);
                break;

        default:
                break;
        }
}

void hp_taco_device::irq_w(bool state)
{
        if (state != m_irq) {
                m_irq = state;
                m_irq_handler(state);
                LOG(("IRQ = %d\n" , state));
        }
}

void hp_taco_device::set_error(bool state)
{
        m_sts = !state;
        m_sts_handler(m_sts);
        LOG(("error = %d\n" , state));
}

bool hp_taco_device::check_for_errors(void)
{
        // Is it an error when "status" flag is already reporting an error? Dunno...
        if ((m_status_reg & STATUS_ERR_MASK) != 0) {
                set_error(true);
                return true;
        } else {
                return false;
        }
}

unsigned hp_taco_device::speed_to_tick_freq(bool fast)
{
        return fast ? TACH_FREQ_FAST : TACH_FREQ_SLOW;
}

void hp_taco_device::update_tape_pos(void)
{
        attotime delta_time(machine().time() - m_start_time);
        m_start_time = machine().time();
        LOG(("delta_time = %g\n" , delta_time.as_double()));
        // How many tachometer ticks has the tape moved?
        unsigned delta_tach = (unsigned)(delta_time.as_ticks(speed_to_tick_freq(SPEED_FAST(m_cmd_reg))));
        LOG(("delta_tach = %u\n" , delta_tach));

        if (DIR_FWD(m_cmd_reg)) {
                // Forward
                m_tape_pos += delta_tach;

                // In real life tape would unspool..
                if (m_tape_pos > TAPE_LENGTH) {
                        m_tape_pos = TAPE_LENGTH;
                        LOG(("Tape unspooled at the end!\n"));
                }
        } else {
                // Reverse
                if (delta_tach >= m_tape_pos) {
                        m_tape_pos = 0;
                        LOG(("Tape unspooled at the start!\n"));
                } else {
                        m_tape_pos -= delta_tach;
                }
        }
        LOG(("Tape pos = %u\n" , m_tape_pos));
}

// Is there any hole in a given section of tape?
bool hp_taco_device::any_hole(tape_pos_t tape_pos_a , tape_pos_t tape_pos_b)
{
        if (tape_pos_a > tape_pos_b) {
                // Ensure A always comes before B
                tape_pos_t tmp;
                tmp = tape_pos_a;
                tape_pos_a = tape_pos_b;
                tape_pos_b = tmp;
        }

        for (tape_pos_t hole : tape_holes) {
                if (tape_pos_a < hole && tape_pos_b >= hole) {
                        return true;
                }
        }

        return false;
}

// Position of next hole tape will reach in a given direction
hp_taco_device::tape_pos_t hp_taco_device::next_hole(bool fwd) const
{
        if (fwd) {
                for (tape_pos_t hole : tape_holes) {
                        if (hole > m_tape_pos) {
                                LOG(("next hole fwd @%u = %u\n" , m_tape_pos , hole));
                                return hole;
                        }
                }
                // No more holes: will hit end of tape
                return TAPE_LENGTH;
        } else {
                for (int i = (sizeof(tape_holes) / sizeof(tape_holes[ 0 ])) - 1; i >= 0; i--) {
                        if (tape_holes[ i ] < m_tape_pos) {
                                LOG(("next hole rev @%u = %u\n" , m_tape_pos , tape_holes[ i ]));
                                return tape_holes[ i ];
                        }
                }
                // No more holes: will hit start of tape
                return 0;
        }
}

attotime hp_taco_device::time_to_distance(tape_pos_t distance, bool fast)
{
        // +1 for rounding
        return attotime::from_ticks(distance + 1 , speed_to_tick_freq(fast));
}

attotime hp_taco_device::time_to_target(tape_pos_t target, bool fast) const
{
        return time_to_distance(abs(target - m_tape_pos), fast);
}

void hp_taco_device::start_tape(void)
{
        m_start_time = machine().time();
        BIT_CLR(m_status_reg, STATUS_HOLE_BIT);
}

void hp_taco_device::stop_tape(void)
{
        if (!m_start_time.is_never()) {
                tape_pos_t tape_start_pos = m_tape_pos;
                update_tape_pos();
                if (any_hole(tape_start_pos , m_tape_pos)) {
                        // Crossed one or more holes
                        BIT_SET(m_status_reg , STATUS_HOLE_BIT);
                }
                m_start_time = attotime::never;
        }
        m_tape_timer->reset();
}

void hp_taco_device::start_cmd_exec(UINT16 new_cmd_reg)
{
        LOG(("Cmd = %02x\n" , CMD_CODE(new_cmd_reg)));

        attotime cmd_duration = attotime::never;

        // Should irq be raised anyway when already in error condition? Here we do nothing.

        switch (CMD_CODE(new_cmd_reg)) {
        case CMD_CLEAR:
                set_error(false);
                BIT_CLR(m_status_reg, STATUS_HOLE_BIT);
                // This is a special command: it doesn't raise IRQ at completion and it
                // doesn't replace the current command, if any.
                return;

        case CMD_STOP:
                stop_tape();
                cmd_duration = attotime::from_usec(QUICK_CMD_USEC);
                break;

        case CMD_SET_TRACK:
                if (!check_for_errors()) {
                        if (CMD_OPT(new_cmd_reg)) {
                                BIT_SET(m_status_reg, STATUS_TRACKB_BIT);
                        } else {
                                BIT_CLR(m_status_reg, STATUS_TRACKB_BIT);
                        }
                        cmd_duration = attotime::from_usec(QUICK_CMD_USEC);
                }
                break;

        case CMD_MOVE:
                stop_tape();
                if (!check_for_errors()) {
                        start_tape();
                        cmd_duration = time_to_target(next_hole(DIR_FWD(new_cmd_reg)) , SPEED_FAST(new_cmd_reg));
                }
                break;

        case CMD_DELTA_MOVE_REC:
        case CMD_DELTA_MOVE_IRG:
                // TODO: record/irg detection
                stop_tape();
                if (!check_for_errors()) {
                        start_tape();
                        cmd_duration = time_to_distance(0x10000U - m_tach_reg , SPEED_FAST(new_cmd_reg));
                }
                break;

        default:
                LOG(("Unrecognized command\n"));
                return;
        }

        m_tape_timer->adjust(cmd_duration);
        m_cmd_reg = new_cmd_reg;
}
