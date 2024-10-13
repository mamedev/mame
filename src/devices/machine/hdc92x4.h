// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    HDC9224 / HDC9234 Hard and Floppy Disk Controller
    For details see hdc92x4.c
*/
#ifndef MAME_MACHINE_HDC92X4_H
#define MAME_MACHINE_HDC92X4_H

#include "imagedev/floppy.h"
#include "imagedev/mfmhd.h"
#include "fdc_pll.h"

DECLARE_DEVICE_TYPE(HDC9224, hdc9224_device)
DECLARE_DEVICE_TYPE(HDC9234, hdc9234_device)

class hdc92x4_device : public device_t
{
public:
	/*
	    Enumeration of the latches outside of the controller
	*/
	enum
	{
		INPUT_STATUS    = 0x00,
		OUTPUT_DMA_ADDR = 0x01,
		OUTPUT_1        = 0x02,
		OUTPUT_2        = 0x03
	};


	/*
	    Definition of bits in the Disk-Status register
	*/
	enum
	{
		DS_ECCERR  = 0x80,        // ECC error
		DS_INDEX   = 0x40,        // index hole
		DS_SKCOM   = 0x20,        // seek complete
		DS_TRK00   = 0x10,        // track 0
		DS_UDEF    = 0x08,        // user-defined
		DS_WRPROT  = 0x04,        // write-protected
		DS_READY   = 0x02,        // drive ready bit
		DS_WRFAULT = 0x01         // write fault
	};

	// Accessors from the CPU side
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void reset(int state);
	void dmaack(int state);

	// Callbacks
	auto intrq_cb() { return m_out_intrq.bind(); }
	auto dmarq_cb() { return m_out_dmarq.bind(); }
	auto dip_cb() { return m_out_dip.bind(); }
	auto auxbus_cb() { return m_out_auxbus.bind(); }
	auto dmain_cb() { return m_in_dma.bind(); }
	auto dmaout_cb() { return m_out_dma.bind(); }

	// auxbus_in is intended to read events from the drives
	// In the real chip the status is polled; to avoid unnecessary load
	// we implement it as a push call
	void auxbus_in( uint8_t data );

	// We pretend that the data separator is part of this controller. It is
	// in fact a separate circuit. The clock divider must be properly set
	// for MFM (CD0=1, CD1=0) or FM (CD0=0, CD1=1).
	// This is not set by the controller itself!
	void set_clock_divider(int pin, int value);

	// Used to reconfigure the drive connections. Floppy drive selection is done
	// using the user-programmable outputs. Hence, the connection
	// is changed outside of the controller, and by this way we let it know.
	void connect_floppy_drive(floppy_image_device *floppy);

	// Used to reconfigure the drive connections. See connect_floppy_drive.
	void connect_hard_drive(mfm_harddisk_device *harddisk);

protected:
	hdc92x4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_hdc9234);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(gen_timer_expired);
	TIMER_CALLBACK_MEMBER(com_timer_expired);

private:
	const bool m_is_hdc9234;

	devcb_write_line   m_out_intrq;    // INT line
	devcb_write_line   m_out_dmarq;    // DMA request line
	devcb_write_line   m_out_dip;      // DMA in progress line
	devcb_write8       m_out_auxbus;   // AB0-7 lines (using S0,S1 as address)
	devcb_read8        m_in_dma;       // DMA read access to the cache buffer
	devcb_write8       m_out_dma;      // DMA write access to the cache buffer

	// Internal register pointer used for sequential register loading
	int m_register_pointer;

protected:
	// Read and write registers
	uint8_t m_register_w[12];
	uint8_t m_register_r[15];

private:
	// Interrupt management (outgoing INT pin)
	void set_interrupt(line_state intr);

	// Currently connected floppy
	floppy_image_device* m_floppy;

	// Currently connected harddisk
	mfm_harddisk_device* m_harddisk;

	// internal register OUTPUT1
	uint8_t m_output1, m_output1_old;

	// internal register OUTPUT2
	uint8_t m_output2, m_output2_old;

	// Write the output registers to the latches
	void auxbus_out();

	// Write the DMA address to the external latches
	void dma_address_out(uint8_t addrub, uint8_t addrhb, uint8_t addrlb);

	// Intermediate storage for register
	uint8_t m_regvalue;

protected:
	// Drive type that has been selected in drive_select
	int m_selected_drive_type;

private:
	// Drive numbere that has been selected in drive_select
	int m_selected_drive_number;

	// Indicates whether the device has completed initialization
	bool m_initialized;

	// Timers to delay execution/completion of commands */
	emu_timer *m_timer;
	emu_timer *m_cmd_timer;
	// emu_timer *m_live_timer;

	// Handlers for incoming signals
	void ready_handler();
	void index_handler();
	void seek_complete_handler();

	// Wait for this line?
	bool waiting_for_line(int line, int level);

	// Wait for some other line?
	bool waiting_for_other_line(int line);

	// Wait for some time to pass or for a line to change level
	void wait_time(emu_timer *tm, int microsec, int next_substate);
	void wait_time(emu_timer *tm, const attotime &delay, int param);
	void wait_line(int line, line_state level, int substate, bool stopwrite);

	// Current time
	std::string ttsn() const;

	// Utility routine to set or reset bits
	void set_bits(uint8_t& byte, int mask, bool set);

	// Event handling
	line_state m_line_level;
	int m_event_line;
	int m_state_after_line;
	bool m_timed_wait;

	// ==============================================
	//   Live state machine
	// ==============================================

	struct live_info
	{
		attotime time;
		uint16_t shift_reg;
		uint16_t shift_reg_save;
		uint16_t crc;
		int bit_counter;
		int bit_count_total;    // used for timeout handling
		int byte_counter;
		bool data_separator_phase;
		bool last_data_bit;
		uint8_t clock_reg;
		uint8_t data_reg;
		int state;
		int next_state;
		int repeat; // for formatting
		int return_state; // for formatting
	};

	live_info m_live_state, m_checkpoint_state;
	int m_last_live_state;

	// Presets CRC.
	void preset_crc(live_info& live, int value);

	// Starts the live run
	void live_start(int state);

	// Analyses the track until the given time
	void live_run_until(attotime limit);

	// Same for hard disks
	void live_run_hd_until(attotime limit);

	// Live run until next index pulse
	void live_run();

	// Control functions for syncing the track analyser with the machine time
	void wait_for_realtime(int state);
	void live_sync();
	void live_abort();
	void rollback();
	void checkpoint();

	// Found a mark
	bool found_mark(int state);

	// Delivers the data bits from the given encoding
	uint8_t get_data_from_encoding(uint16_t raw);

	// ==============================================
	//    PLL functions and interface to floppy and harddisk
	// ==============================================

	// Phase-locked loops
	fdc_pll_t m_pll, m_checkpoint_pll;

	// Clock divider value
	uint8_t m_clock_divider;

	// MFM HD encoding type
	mfmhd_enc_t m_hd_encoding;

	// Resets the PLL to the given time
	void pll_reset(const attotime &when, bool write);

	// Puts the word into the shift register directly. Changes the m_live_state members
	// shift_reg, and last_data_bit
	void encode_raw(uint16_t word);

	// Encodes a byte in FM or MFM. Called by encode_byte.
	uint16_t encode(uint8_t byte);

	// Encodes a byte in FM or MFM. Called by encode_byte.
	uint16_t encode_hd(uint8_t byte);
	uint16_t encode_a1_hd();

	// Encode the latest byte again
	void encode_again();

	// Reads from the current position on the track
	bool read_one_bit(const attotime &limit);

	// Writes to the current position on the track
	bool write_one_bit(const attotime &limit);

	// Writes to the current position on the track
	void write_on_track(uint16_t raw, int count, int next_state);

	// Skips bytes on the track
	void skip_on_track(int count, int next_state);

	// Read from the MFM HD
	bool read_from_mfmhd(const attotime &limit);

	// Write to the MFM HD
	bool write_to_mfmhd(const attotime &limit);

	// ==============================================
	//   Command state machine
	// ==============================================

	int m_substate;

	typedef void (hdc92x4_device::*cmdfunc)(void);

	typedef struct
	{
		uint8_t baseval;
		uint8_t mask;
		cmdfunc command;
	} cmddef;

	static const cmddef s_command[];

	// Indicates whether a command is currently being executed
	bool m_executing;

	// Keeps the pointer to the function for later continuation
	cmdfunc m_command;

	// Invoked after the commit period for command initiation or register write access
	void process_command();

	// Re-enters the state machine after a delay
	void reenter_command_processing();

	// Command is done
	void set_command_done(int flags);
	void set_command_done();

	// Difference between current cylinder and desired cylinder
	int m_track_delta;

	// Used to restore the retry count for multi-sector operations
	int m_retry_save;

	// ==============================================
	//   Operation properties
	// ==============================================

	// Precompensation value
	int m_precompensation;

	// Do we have a multi-sector operation?
	bool m_multi_sector;

	// Shall we wait for the index hole?
	bool m_wait_for_index;

	// Shall we stop after the next index hole?
	bool m_stop_after_index;

	// Is data transfer enabled for read operations?
	bool m_transfer_enabled;

	// Is it a read or a write operation?
	bool m_write;

	// Have we found a deleted sector?
	bool m_deleted;

	// Do we apply a reduced write current?
	bool m_reduced_write_current;

	// Used in RESTORE to find out when to give up
	int m_seek_count;

	// Read/write logical or physical?
	bool m_logical;

	// Shall bad sectors be bypassed or shall the command be terminated in error?
	bool m_bypass;

	// Signals to abort writing
	bool m_stopwrite;

	// Flag to remember whether we found the first sector during a physical access
	bool m_first_sector_found;

	// Used for formatting
	int m_sector_count;
	int m_sector_size;
	int m_gap0_size;
	int m_gap1_size;
	int m_gap2_size;
	int m_gap3_size;
	int m_sync_size;

protected:
	// Are we in FM mode?
	bool fm_mode();

	// Do we have timed steps?
	bool timed_steps();

	// Seek completed?
	bool seek_complete();

	// Are we on track 0?
	bool on_track00();

	// Are we at the index hole?
	bool index_hole();

	// Is the attached drive ready?
	bool drive_ready();

	// Are we reading a track?
	bool reading_track();

	// Delivers the desired head
	int desired_head();

	// Delivers the desired sector
	int desired_sector();

	// Delivers the desired cylinder. The value is spread over two registers.
	int desired_cylinder();

	// Delivers the current head as read from the track
	int current_head();

	// Delivers the current sector as read from the track
	int current_sector();

	// Delivers the current cylinder as read from the track
	int current_cylinder();

	// Delivers the current command
	uint8_t current_command();

	// Step time (minus pulse width)
	virtual int step_time() =0;

	// Step pulse width
	int pulse_width();

	// Sector size as read from the track or given by register A (PC-AT mode)
	int sector_size();

	// Returns the sector header length
	virtual int header_length() =0;

	// Returns the index of the register for the header field
	int register_number(int slot);

	// Is the currently selected drive a floppy drive?
	bool using_floppy();

	// Was the Bad Sector flag set for the recently read sector header?
	bool bad_sector();

private:
	// Common subprograms READ ID, VERIFY, and DATA TRANSFER
	void read_id(int& cont, bool implied_seek, bool wait_seek_complete);
	void verify(int& cont);
	void data_transfer(int& cont);

	// ===================================================
	//   Commands
	// ===================================================

	void reset_controller();
	void drive_deselect();
	void restore_drive();
	void step_drive();
	void tape_backup();
	void poll_drives();
	void drive_select();
	void set_register_pointer();
	void seek_read_id();
	void read_sectors();
	void read_track();
	void format_track();
	void write_sectors();
};

// =====================================================
//   Subclasses: the two variants
// =====================================================

class hdc9224_device : public hdc92x4_device
{
public:
	hdc9224_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int step_time() override;
	virtual int header_length() override;
};

class hdc9234_device : public hdc92x4_device
{
public:
	hdc9234_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual int step_time() override;
	virtual int header_length() override;
};

#endif // MAME_MACHINE_HDC92X4_H
