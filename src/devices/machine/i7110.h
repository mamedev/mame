// license:BSD-3-Clause
// copyright-holders:F. Ulivi

/*********************************************************************

    Intel 1 Mbit magnetic bubble memory subsystem

    A MBM subsystem is composed of one i7220-1 BMC (Bubble Memory
    Controller) and 1 to 8 MBMs (Magnetic Bubble Module). Each MBM is
    composed of the bubble memory itself (i7110) and 5 support chips:
    - i7242 FSA (Formatter and Sense Amplifier),
    - i7230 CPG (Current Pulse Generator),
    - i7250 CPD (Coil Pre-Driver),
    - 2x i7254 Quad VMOS Drive Transistors

    The BMC interfaces to FSA(s) through a common I/O serial bus. A
    daisy-chained select signal is propagated from BMC through the
    FSAs. Each FSA contains two identical channels (A & B), one per
    half of i7110 memory.
    Here i7220-1 is modeled by i7220_1_device class, each MBM by
    an instance of ibubble_device class. Each instance contains the 2
    FSA channels (fsa_channel_device).

                   ____    ____
     /PWR.FAIL  1 |*   \__/    | 40 VCC
    /RESET.OUT  2 |            | 39 /X+
           CLK  3 |            | 38 /X-
        /RESET  4 |            | 37 /Y+
           /RD  5 |            | 36 /Y-
           /WR  6 |            | 35 /TM.A
         /DACK  7 |            | 34 /TM.B
           DRQ  8 |            | 33 /REP.EN
           INT  9 |            | 32 /BOOT.EN
            A0 10 |            | 31 /SWAP.EN
            D0 11 |  i7220-1   | 30 /BOOT.SW.EN
            D1 12 |    BMC     | 29 C/D
            D2 13 |            | 28 /DET.ON
            D3 14 |            | 27 /ERR.FLG
            D4 15 |            | 26 /WAIT
            D5 16 |            | 25 /BUS.RD
            D6 17 |            | 24 /SHIFT.CLK
            D7 18 |            | 23 /SYNC
            D8 19 |            | 22 DIO
           GND 20 |____________| 21 /CS

                   _________________
     PULSE.COM  1 |*                | 20 DET.SUPPLY
   REPLICATE.B  2 |                 | 19 DET.OUT.A-
   REPLICATE.A  3 |                 | 18 DET.OUT.A+
      BOOT.REP  4 |                 | 17 DET.OUT.B-
     BOOT.SWAP  5 |      i7110      | 16 DET.OUT.B+
           N/C  6 |                 | 15 DET.COM
    GENERATE.A  7 |                 | 14 SWAP.B
    GENERATE.B  8 |                 | 13 SWAP.A
    X-.COIL.IN  9 |                 | 12 Y+.COIL.IN
    X+.COIL.IN 10 |_________________| 11 Y-.COIL.IN

                   __   __
           /CS  1 |* \_/  | 20 VCC
   /SELECT.OUT  2 |       | 19 /SELECT.IN
           C/D  3 |       | 18 CLK
     /ERR.FLAG  4 |       | 17 DIO
           VDD  5 | i7242 | 16 /RESET
        DET.A+  6 |  FSA  | 15 /SHIFT.CLK
        DET.A-  7 |       | 14 /ENABLE.B
        DET.B+  8 |       | 13 /ENABLE.A
        DET.B-  9 |       | 12 /DATA.OUT.B
           GND 10 |_______| 11 /DATA.OUT.A

                   __   __
           VDD  1 |* \_/  | 22 VCC
         /TM.A  2 |       | 21 /PWR.FAIL
         /TM.B  3 |       | 20 REFR
     /GEN.EN.B  4 |       | 19 GEN.B
     /GEN.EN.A  5 |       | 18 GEN.A
      /SWAP.EN  6 | i7230 | 17 SWAP
           /CS  7 |  CPG  | 16 REP.B
       /REP.EN  8 |       | 15 REP.A
   /BOOT.SW.EN  9 |       | 14 BOOT.SWAP
      /BOOT.EN 10 |       | 13 BOOT.REP
           GND 11 |_______| 12 GND

                   __   __
            /CS 1 |* \_/  | 16 VDD
         /RESET 2 |       | 15 /X+.OUT
         /X+.IN 3 |       | 14 X+.OUT
         /X-.IN 4 | i7250 | 13 /X-.OUT
         /Y+.IN 5 |  CPD  | 12 X-.OUT
         /Y-.IN 6 |       | 11 /Y+.OUT
         Y-.OUT 7 |       | 10 Y+.OUT
            GND 8 |_______| 9  /Y-.OUT

                   __   __
             D1 1 |* \_/  | 14 D4
             S1 2 |       | 13 S4
             G1 3 |       | 12 G4
             NC 4 | i7254 | 11 NC
             G2 5 |       | 10 G3
             S2 6 |       | 9  S3
             D2 7 |_______| 8  D3

    Reference docs:
    - Intel Magnetics, BPK 72, Bubble memory prototype kit UM
    - i7220-1 data sheet
    - i7242 data sheet
    - Intel, AP-157, Software design and implementation details for
      bubble memory systems

    This driver is dedicated to the memory of my father.

*********************************************************************/

#ifndef MAME_MACHINE_I7110_H
#define MAME_MACHINE_I7110_H

#pragma once

#include "machine/timer.h"

#include <bitset>

// +--------------------+
// | fsa_channel_device |
// +--------------------+
class fsa_channel_device : public device_t
{
public:
	fsa_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// FSA commands
	enum fsa_cmd : uint8_t {
		FSA_CMD_NOP        = 0b0000,    // 0000 NOP
		FSA_CMD_SWRESET    = 0b0010,    // 0010 Software reset
		FSA_CMD_INIT       = 0b0011,    // 0011 Initialize
		FSA_CMD_WRITE_DATA = 0b0100,    // 0100 Write MBM data
		FSA_CMD_READ_DATA  = 0b0101,    // 0101 Read MBM data
		FSA_CMD_ICD        = 0b0110,    // 0110 Internally correct data
		FSA_CMD_RCD        = 0b0111,    // 0111 Read corrected data
		FSA_CMD_WRITE_BLR  = 0b1000,    // 1000 Write bootloop register
		FSA_CMD_READ_BLR   = 0b1001,    // 1001 Read bootloop register
		FSA_CMD_SET_ENABLE = 0b1100,    // 1100 Set enable bit
		FSA_CMD_READ_ERRFLG= 0b1101,    // 1101 Read ERRFLG status
		FSA_CMD_SET_EC     = 0b1110,    // 1110 Set error correction enable bit
		FSA_CMD_READ_STATUS= 0b1111     // 1111 Read status register
	};

	// Bits in status register
	enum status_bits {
		STAT_UNCORRERR  = 0,    // Uncorrectable error
		STAT_CORRERR    = 1,    // Correctable error
		STAT_TIMERR     = 2,    // Timing error
		STAT_ECF        = 3,    // Error correction enabled
		STAT_FIFOFL     = 4,    // FIFO full
		STAT_FIFOMT     = 5,    // FIFO empty
	};

	// Read enable
	bool enable_r() const { return m_selected && m_enable; }
	// Do 1 rotation of magnetic field
	void field_rotate();
	// Select channel & execute a FSA command
	void cmd_w(bool select, fsa_channel_device::fsa_cmd cmd);
	// Write one data bit
	void dio_w(int data);
	// Read one data bit from channel
	int dio_r();
	// Read status register
	uint8_t status_r();
	// One pulse of shift clock
	void shiftclk();
	// Replicate bubbles in output track
	void bubble_replicate();
	// Replicate bootloop bubble in output track
	void bootloop_replicate();
	// Exchange bubbles between input track and data loops
	void bubble_swap();
	// Exchange bubbles between input track and bootloop
	void bootloop_swap();
	// Read errflg signal
	bool errflg_r() const;

	// Bits in each loop
	static inline constexpr unsigned BITS_PER_LOOP = 4096;
	// Loops in each quad (4 quads in a i7110, 2 in a FSA channel)
	static inline constexpr unsigned LOOPS_PER_QUAD = 80;
	enum quad_id {
		EVEN_QUAD,
		ODD_QUAD,
		QUADS_PER_CH    // Quads in each FSA channel
	};
	// Size of image
	// 655360 bits (160 data loops) + 4096 bits (bootloop)
	static inline constexpr std::size_t IMAGE_SIZE = (BITS_PER_LOOP * LOOPS_PER_QUAD * QUADS_PER_CH + BITS_PER_LOOP) / 8;
	// Load image (img must be at least IMAGE_SIZE bytes long)
	void load_image(const std::vector<uint8_t>& img);
	// Load an empty image
	void create_image();
	// Save image
	std::vector<uint8_t> save_image() const;

	// Format of bootloop is only partially documented, so I had to guess to fill the gaps.
	// Pre-sync 3158 bits = 1
	// Sync     242 bits  = 0
	// Pattern  14 bits   = 10101010101010
	// Bootloop 320 bits  = alternating bits of channel A & B bootloops
	// Padding  362 bits  = 0
	static inline constexpr unsigned BL_PRE_SYNC_BITS = 3158;
	static inline constexpr unsigned BL_SYNC_BITS = 242;
	static inline constexpr unsigned BL_PATTERN_BITS = 14;
	static inline constexpr unsigned BL_BOOTLOOP_BITS = LOOPS_PER_QUAD * QUADS_PER_CH * 2;
	static inline constexpr unsigned BL_PAD_BITS = 362;

	// Payload bits (error correction enabled)
	static inline constexpr unsigned PAYLOAD_BITS = 256;
	// Fire code bits (error correction enabled)
	static inline constexpr unsigned FIRE_CODE_BITS = 14;
	// Total bits when error correction is enabled
	static inline constexpr unsigned EC_BITS = PAYLOAD_BITS + FIRE_CODE_BITS;
	// Total bits when error correction is disabled
	static inline constexpr unsigned NO_EC_BITS = PAYLOAD_BITS + 16;
	// Bits in bootloop register
	static inline constexpr unsigned BLR_BITS = QUADS_PER_CH * LOOPS_PER_QUAD;
	// Raw data bits
	static inline constexpr unsigned RAW_BITS = QUADS_PER_CH * LOOPS_PER_QUAD * 2;

protected:
	// Fire code polynomial
	//  14   11   9   5   2
	// X  + X  + X + X + X + 1
	// Code is a truncated version of (279, 265) Fire code: first 9 bits don't actually exist
	// It can correct a single error burst up to 5 bits in length
	static inline constexpr uint16_t FIRE_POLY = 0b101000100101;
	// Mask of Fire code bits
	static inline constexpr uint16_t FIRE_MASK = (1U << FIRE_CODE_BITS) - 1;
	// Mask of error trap
	static inline constexpr uint16_t ERR_TRAP_MASK = 0x1ff;
	// Bits chopped off (279, 265) code to make it a (270, 256) code
	static inline constexpr unsigned FIRE_CHOPPED_BITS = 9;
	// Length of output track of odd quad
	// Loop-80 o o o Loop-79 o o o ... Loop-1 (39x o) Det
	static inline constexpr unsigned ODD_OUT_LEN = 357;
	// Length of output track of even quad
	// Bootloop o o o o Loop-80 o o o Loop-79 o o o ... Loop-1 (40x o) Det
	static inline constexpr unsigned EVEN_OUT_LEN = 363;
	// Position of bootloop bit in even output track
	static inline constexpr unsigned EVEN_OUT_BL_POS = EVEN_OUT_LEN - 1;
	// Position of loop-80 bit in even output track
	static inline constexpr unsigned EVEN_OUT_LOOP80_POS = EVEN_OUT_LEN - 6;
	// Position of loop-80 bit in odd output track
	static inline constexpr unsigned ODD_OUT_LOOP80_POS = ODD_OUT_LEN - 1;
	// Spacing of data bits in output tracks
	static inline constexpr unsigned DATA_BIT_OUT_DIST = 4;
	// Position of detector in even output track
	static inline constexpr unsigned EVEN_OUT_DET_POS = 0;
	// Position of detector in odd output track
	static inline constexpr unsigned ODD_OUT_DET_POS = 0;
	// Length of input track of odd quad
	// Gen (23x o) Loop-80 o o o Loop-79 o o o ... Loop-1
	static inline constexpr unsigned ODD_IN_LEN = 341;
	// Length of input track of even quad
	// Gen (17x o) Bootloop (4x o) Loop-80 o o o Loop-79 o o o ... Loop-1
	static inline constexpr unsigned EVEN_IN_LEN = 340;
	// Position of bootloop bit in even input track
	static inline constexpr unsigned EVEN_IN_BL_POS = EVEN_IN_LEN - 19;
	// Position of loop-80 bit in even input track
	static inline constexpr unsigned EVEN_IN_LOOP80_POS = EVEN_IN_LEN - 24;
	// Position of loop-80 bit in odd input track
	static inline constexpr unsigned ODD_IN_LOOP80_POS = ODD_IN_LEN - 25;
	// Spacing of data bits in input tracks
	static inline constexpr unsigned DATA_BIT_IN_DIST = 4;
	// Position of generator in even input track
	static inline constexpr unsigned EVEN_IN_GEN_POS = EVEN_IN_LEN - 1;
	// Position of generator in odd input track
	static inline constexpr unsigned ODD_IN_GEN_POS = ODD_IN_LEN - 1;
	// Read (replicate) position of loops
	static inline constexpr unsigned LOOP_RD_POS = 0;
	// Write (swap) position of loops
	static inline constexpr unsigned LOOP_WR_POS = 2048;
	// Duration of ICD command (clocks)
	static inline constexpr unsigned ICD_CLOCKS = 1400;

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_to);

private:
	required_device<timer_device> m_timer;

	std::bitset<BITS_PER_LOOP> m_bootloop;  // 4096 bits
	std::bitset<BITS_PER_LOOP> m_data_loops[ QUADS_PER_CH ][ LOOPS_PER_QUAD ];  // 655360 bits
	std::bitset<EVEN_OUT_LEN> m_even_out;   // 363 bits
	std::bitset<ODD_OUT_LEN> m_odd_out;     // 357 bits
	std::bitset<EVEN_IN_LEN> m_even_in;     // 340 bits
	std::bitset<ODD_IN_LEN> m_odd_in;       // 341 bits

	bool m_selected;
	bool m_enable;
	bool m_data_out;
	uint8_t m_status;
	std::bitset<BLR_BITS> m_blr;        // 160 bits
	std::bitset<NO_EC_BITS> m_fifo;     // 272 bits
	unsigned m_blr_idx;
	unsigned m_fifo_in_idx;
	unsigned m_fifo_out_idx;
	uint16_t m_code_accum;
	unsigned m_code_cnt;
	fsa_cmd m_curr_cmd;

	// Error correction state
	enum class ec_state {
		EC_NO_ERROR,            // No correction needed
		EC_WAIT_TRAP,           // Waiting for error to be trapped
		EC_WAIT_TO_START,       // Waiting to apply correction
		EC_CORRECTING,          // Correction in progress
		EC_DONE                 // Correction done
	};

	ec_state m_ec_state;

	void reset_pointers();
	unsigned get_blr_idx() const;
	void clear_errors();
	void reset_error_corr();
	void set_enable(bool en);
	template<std::size_t N> static void rotate(std::bitset<N>& bits, bool loop);
	bool detector_r() const;
	void generate(bool bit);
	unsigned fifo_size() const;
	void fifo_enqueue(bool bit);
	bool fifo_dequeue();
	void fire_code_enc(bool bit);
	void fire_code_syn(bool bit);
	void pre_error_trapping();
	bool correct_one_bit();
	static void encode_loop(const std::bitset<BITS_PER_LOOP>& loop, std::vector<uint8_t>& out);
	static void decode_loop(std::vector<uint8_t>::const_iterator& in, std::bitset<BITS_PER_LOOP>& loop);
};

// +----------------+
// | ibubble_device |
// +----------------+
class ibubble_device : public device_t,
					   public device_image_interface
{
public:
	ibubble_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Do 1 rotation of magnetic field
	void field_rotate(int state);
	// Select channels in FSA (upstream I/F). Bit 0 selects channel A, bit 1 channel B, remaining bits are passed to downstream CB
	void select_w(uint16_t data);
	// Select channels in FSA (downstream I/F)
	auto select_w_callback() { return select_w_cb.bind(); }
	// Execute a FSA command in selected channels
	void cmd_w(uint8_t data);
	// Write data to channels (upstream I/F). Bit 0 goes to channel A, bit 1 to channel B, remaining bits are passed to downstream CB
	void dio_w(uint16_t data);
	// Write data to channels (downstream I/F)
	auto dio_w_callback() { return dio_w_cb.bind(); }
	// Read data from channels (upstream I/F). Bit 0 comes from channel A, bit 1 from channel B, remaining bits from downstream CB
	uint16_t dio_r();
	// Read data from channels (downstream I/F)
	auto dio_r_callback() { return dio_r_cb.bind(); }
	// Read status register of a channel (upstream I/F)
	uint8_t status_r();
	// Read status register of a channel (downstream I/F)
	auto status_r_callback() { return status_r_cb.bind(); }
	// One pulse of shift clock
	void shiftclk(int state);
	// Replicate bubbles in output track
	void bubble_replicate(int state);
	// Replicate bootloop bubble in output track
	void bootloop_replicate(int state);
	// Exchange bubbles between input track and data loops
	void bubble_swap(int state);
	// Exchange bubbles between input track and bootloop
	void bootloop_swap(int state);
	// Read errflg signal (upstream I/F)
	int errflg_r();
	// Read errflg signal (downstream I/F)
	auto errflg_r_callback() { return errflg_r_cb.bind(); }

	// Set chip select (active low)
	void cs_w(int state);

	// device_image_interface overrides
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "imbm"; }
	virtual const char *image_type_name() const noexcept override { return "bubble"; }
	virtual const char *image_brief_type_name() const noexcept override { return "mbm"; }

	// Channels in a MBM
	static inline constexpr unsigned CHS_PER_MBM = 2;

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<fsa_channel_device> m_chA;
	required_device<fsa_channel_device> m_chB;

	devcb_write16 select_w_cb;
	devcb_write16 dio_w_cb;
	devcb_read16 dio_r_cb;
	devcb_read8 status_r_cb;
	devcb_read_line errflg_r_cb;

	int m_cs;
	bool m_select_a;
	bool m_select_b;
	bool m_dirty;
};

// +----------------+
// | i7220_1_device |
// +----------------+
class i7220_1_device : public device_t
{
public:
	// construction/destruction
	i7220_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return intrq_cb.bind(); }
	auto drq_callback() { return drq_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// Do 1 magnetic field rotation
	// Width: 1 bit (value is not important)
	// Distribution: in parallel to all ibubble_device
	auto field_rotate_callback() { return field_rotate_cb.bind(); }
	// Set command selection
	// Width: 16 bits (LSB selects channel A of 1st MBM)
	// Distribution: daisy-chained (BMC->MBM #0->MBM #1->...)
	auto select_w_callback() { return select_w_cb.bind(); }
	// Execute a command in selected FSAs
	// Width: 8 bits
	// Distribution: parallel
	auto cmd_w_callback() { return cmd_w_cb.bind(); }
	// Send data to selected FSAs
	// Width: 16 bits
	// Distribution: daisy-chained
	auto dio_w_callback() { return dio_w_cb.bind(); }
	// Read data from selected FSAs
	// Width: 16 bits
	// Distribution: daisy-chained
	auto dio_r_callback() { return dio_r_cb.bind(); }
	// Read status register from selected FSA
	// Width: 8 bits
	// Distribution: daisy-chained
	auto status_r_callback() { return status_r_cb.bind(); }
	// Send one pulse of shift clock
	// Width: 1 bit (value is not important)
	// Distribution: parallel
	auto shiftclk_callback() { return shiftclk_cb.bind(); }
	// Replicate bubbles in output track
	// Width: 1 bit (value is not important)
	// Distribution: parallel
	auto bubble_replicate_callback() { return bubble_replicate_cb.bind(); }
	// Replicate bootloop bubble in output track
	// Width: 1 bit (value is not important)
	// Distribution: parallel
	auto bootloop_replicate_callback() { return bootloop_replicate_cb.bind(); }
	// Exchange bubbles between input track and data loops
	// Width: 1 bit (value is not important)
	// Distribution: parallel
	auto bubble_swap_callback() { return bubble_swap_cb.bind(); }
	// Exchange bubbles between input track and bootloop
	// Width: 1 bit (value is not important)
	// Distribution: parallel
	auto bootloop_swap_callback() { return bootloop_swap_cb.bind(); }
	// Read errflg signal
	// Width: 1 bit
	// Distribution: daisy-chained
	auto errflg_r_callback() { return errflg_r_cb.bind(); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_to);

private:
	// Register indexes
	enum {
		REG_IDX_FIFO = 0,       // FIFO
		REG_IDX_UR = 10,        // Utility register
		REG_IDX_BLR_LSB = 11,   // LSB of block length register
		REG_IDX_BLR_MSB = 12,   // MSB of block length register
		REG_IDX_ER = 13,        // Enable register
		REG_IDX_AR_LSB = 14,    // LSB of address register
		REG_IDX_AR_MSB = 15,    // MSB of address register
	};

	// Commands
	enum bmc_cmds : uint8_t {
		CMD_WR_BLR_MASKED = 0b0000, // Write bootloop register masked
		CMD_INITIALIZE    = 0b0001, // Initialize
		CMD_RD_DATA       = 0b0010, // Read bubble data
		CMD_WR_DATA       = 0b0011, // Write bubble data
		CMD_RD_SEEK       = 0b0100, // Read seek
		CMD_RD_BLR        = 0b0101, // Read bootloop register
		CMD_WR_BLR        = 0b0110, // Write bootloop register
		CMD_WR_BL         = 0b0111, // Write bootloop
		CMD_RD_FSA_STAT   = 0b1000, // Read FSA status
		CMD_ABORT         = 0b1001, // Abort
		CMD_WR_SEEK       = 0b1010, // Write seek
		CMD_RD_BL         = 0b1011, // Read bootloop
		CMD_RCD           = 0b1100, // Read corrected data
		CMD_RESET_FIFO    = 0b1101, // Reset FIFO
		CMD_MBM_PURGE     = 0b1110, // MBM purge
		CMD_SWRESET       = 0b1111  // Software reset
	};

	// Bits in status register
	enum status_bits {
		STAT_FIFO_RDY   = 0,    // FIFO ready
		STAT_PARERR     = 1,    // Parity error
		STAT_UNCORRERR  = 2,    // Uncorrectable error
		STAT_CORRERR    = 3,    // Correctable error
		STAT_TIMERR     = 4,    // Timing error
		STAT_OPFAIL     = 5,    // Operation failed
		STAT_OPCOMPLETE = 6,    // Operation completed
		STAT_BUSY       = 7,    // Busy
	};

	// Bits in enable register
	enum enable_bits {
		EN_INT_NORM     = 0,    // Enable interrupt (normal)
		EN_INT_ERR      = 1,    // Enable interrupt (error)
		EN_DMA          = 2,    // Enable DMA
		EN_MFBTR        = 3,    // Enable maximum FSA to BMC transfer rate
		EN_WR_BL        = 4,    // Enable write bootloop
		EN_RCD          = 5,    // Enable RCD
		EN_ICD          = 6,    // Enable ICD
		EN_INT_PARERR   = 7,    // Enable parity interrupt
	};

	// States of command/sub-command FSM
	enum fsm_state {
		IDLE,
		S0,
		S1,
		S2,
		S3,
		S4,
		S5,
		S6,
		S7,
		S8,
		S9,
		S10,
		S11,
		S12,
		S13,
		S14,
		S15,
		S16,
		S17,
		S18,
		S19,
		S20,
		S21
	};

	// Commands & sub-commands
	enum cmds {
		CMDS_WR_BLR_MASKED = CMD_WR_BLR_MASKED,
		CMDS_INITIALIZE    = CMD_INITIALIZE,
		CMDS_RD_DATA       = CMD_RD_DATA,
		CMDS_WR_DATA       = CMD_WR_DATA,
		CMDS_RD_SEEK       = CMD_RD_SEEK,
		CMDS_RD_BLR        = CMD_RD_BLR,
		CMDS_WR_BLR        = CMD_WR_BLR,
		CMDS_WR_BL         = CMD_WR_BL,
		CMDS_RD_FSA_STAT   = CMD_RD_FSA_STAT,
		CMDS_ABORT         = CMD_ABORT,
		CMDS_WR_SEEK       = CMD_WR_SEEK,
		CMDS_RD_BL         = CMD_RD_BL,
		CMDS_RCD           = CMD_RCD,
		CMDS_RESET_FIFO    = CMD_RESET_FIFO,
		CMDS_MBM_PURGE     = CMD_MBM_PURGE,
		CMDS_SWRESET       = CMD_SWRESET,
		SUBCMD_READ_SEEK,
		SUBCMD_READ_CH_STAT,
		SUBCMD_FSA_BMC_XFER,
		SUBCMD_WRITE_SEEK,
		SUBCMD_ROTATE
	};

	// Maximum number of MBMs
	static inline constexpr unsigned MAX_MBM = 8;
	// Maximum number of channels
	static inline constexpr unsigned MAX_CH = MAX_MBM * ibubble_device::CHS_PER_MBM;
	// Mask of RAC register
	static inline constexpr uint8_t RAC_MASK = 0x0f;
	// Mask of CMD register
	static inline constexpr uint8_t CMD_MASK = 0x0f;
	// Distance in logical addresses between consecutive physical addresses
	static inline constexpr uint16_t LA_DIST = 0x589;
	// Write offset
	static inline constexpr uint16_t WR_OFF = 0x706;
	// Mask of logical addresses
	static inline constexpr uint16_t LA_MASK = make_bitmask<uint16_t>(11);
	// Mask of address register (LA + MBM select)
	static inline constexpr uint16_t AR_MASK = make_bitmask<uint16_t>(15);
	// Mask of no. of pages
	static inline constexpr uint16_t N_PAGE_MASK = make_bitmask<uint16_t>(11);
	// Position of NFC field in block length register
	static inline constexpr unsigned NFC_SHIFT = 12;
	// Mask of NFC field
	static inline constexpr uint8_t NFC_MASK = make_bitmask<uint8_t>(4);
	// Position of MBM select field in address register
	static inline constexpr unsigned MBM_SHIFT = 11;
	// Mask of MBM select field
	static inline constexpr uint8_t MBM_MASK = make_bitmask<uint8_t>(4);
	// Clocks for 1 field rotation
	static inline constexpr unsigned CLKS_ROTATE = 80;
	// Clocks to output selection & command to FSAs
	static inline constexpr unsigned CLKS_CMD = 40;
	// Clocks for one fast word I/O
	static inline constexpr unsigned CLKS_FAST_IO = 20;
	// Clocks for one slow word I/O
	static inline constexpr unsigned CLKS_SLOW_IO = 80;

	devcb_write_line intrq_cb;
	devcb_write_line drq_cb;
	devcb_write_line field_rotate_cb;
	devcb_write16 select_w_cb;
	devcb_write8 cmd_w_cb;
	devcb_write16 dio_w_cb;
	devcb_read16 dio_r_cb;
	devcb_read8 status_r_cb;
	devcb_write_line shiftclk_cb;
	devcb_write_line bubble_replicate_cb;
	devcb_write_line bootloop_replicate_cb;
	devcb_write_line bubble_swap_cb;
	devcb_write_line bootloop_swap_cb;
	devcb_read_line errflg_r_cb;

	required_device<timer_device> m_timer;

	bool m_initialized;
	bool m_drq;
	bool m_irq;
	bool m_mbm_select_changed;
	bool m_aborted;
	bool m_read_cmd;
	uint8_t m_rac;
	uint8_t m_cmdr;
	uint8_t m_str;
	uint8_t m_en;
	uint8_t m_ur;
	uint8_t m_fifo_accum;
	uint8_t m_fifo_mask;
	uint16_t m_blr;
	uint16_t m_cnt1;
	uint16_t m_cnt2;
	uint16_t m_cnt3;
	uint16_t m_cnt4;
	uint16_t m_ar;
	uint16_t m_selected_chs;
	uint16_t m_la[ MAX_MBM ];
	unsigned m_skip[ MAX_MBM ];
	unsigned m_mbm_count;
	fsm_state m_fsm_state;
	fsm_state m_ret_state;
	cmds m_sub_cmd;
	util::fifo<uint8_t, 42> m_fifo;

	void field_rotate(fsm_state new_state);
	void cmd_w(uint16_t mask, fsa_channel_device::fsa_cmd cmd);
	void dio_w(uint16_t data);
	uint16_t dio_r();
	uint8_t status_r();
	void shiftclk();
	void bubble_replicate();
	void bootloop_replicate();
	void bubble_swap();
	void bootloop_swap();
	bool errflg_r();
	bool ec_enabled() const;
	unsigned bits_per_xfer() const;
	void clr_irq();
	void set_irq();
	void update_drq();
	void clr_status();
	bool get_busy_state() const;
	bool is_executing() const;
	void cmd_start(uint8_t new_cmd);
	void call_subcmd(cmds sub_cmd, fsm_state ret_state);
	void ret_subcmd();
	bool in_a_subcmd() const;
	void leave();
	void set_op_complete();
	void set_op_complete_n_leave();
	void set_op_fail();
	void set_op_fail_n_leave();
	bool cond_assert(bool c);
	void run_fsm();
	void one_cmd_step(cmds cmd);
	void delay(unsigned clks, fsm_state new_state);
	void clr_fifo_enqueue();
	bool fifo_enqueue_bits(unsigned n_bits, uint16_t data);
	bool fifo_enqueue_byte(uint8_t data);
	void clr_fifo_dequeue();
	bool fifo_dequeue_bits(unsigned n_bits, uint16_t &data);
	void do_wr_blr_masked();
	void do_initialize();
	void do_rd_data();
	void do_wr_data();
	void do_rd_seek();
	void do_rd_blr();
	void do_wr_blr();
	void do_wr_bl();
	void do_rd_fsa_stat();
	void do_abort();
	void do_wr_seek();
	void do_rd_bl();
	void do_rcd();
	void do_reset_fifo();
	void do_mbm_purge();
	void do_swreset();
	void do_seek(uint16_t offset);
	void do_read_seek();
	void do_read_ch_stat();
	void do_fsa_bmc_xfer();
	void do_write_seek();
	void do_rotate();
	uint8_t get_nfc() const;
	void set_nfc(uint8_t n);
	uint8_t get_mbm_select() const;
	void set_mbm_select(uint8_t m);
	uint16_t get_n_pages() const;
	void dec_n_pages();
	uint16_t get_la() const;
	bool inc_ar();
	void select_chs();
	unsigned fsa_n_channels() const;
	unsigned fsa_1st_ch() const;
	unsigned fsa_1st_mbm() const;
	uint16_t fsa_ch_mask() const;
	static uint16_t fsa_1ch_mask(uint8_t mbm_select);
	static uint16_t fsa_ch_mask(unsigned n_ch, uint8_t mbm_select);
};

// device type declaration
DECLARE_DEVICE_TYPE(IBUBBLE, ibubble_device)
DECLARE_DEVICE_TYPE(I7220_1, i7220_1_device)

#endif /* MAME_MACHINE_I7110_H */
