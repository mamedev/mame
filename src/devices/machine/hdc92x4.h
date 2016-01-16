// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    HDC9224 / HDC9234 Hard and Floppy Disk Controller
    For details see hdc92x4.c
*/
#ifndef __HDC92X4_H__
#define __HDC92X4_H__

#include "emu.h"
#include "imagedev/floppy.h"
#include "imagedev/mfmhd.h"
#include "fdc_pll.h"

extern const device_type HDC9224;
extern const device_type HDC9234;

/*
    Enumeration of the latches outside of the controller
*/
enum
{
	HDC_INPUT_STATUS    = 0x00,
	HDC_OUTPUT_DMA_ADDR = 0x01,
	HDC_OUTPUT_1        = 0x02,
	HDC_OUTPUT_2        = 0x03
};


/*
    Definition of bits in the Disk-Status register
*/
enum
{
	HDC_DS_ECCERR  = 0x80,        // ECC error
	HDC_DS_INDEX   = 0x40,        // index hole
	HDC_DS_SKCOM   = 0x20,        // seek complete
	HDC_DS_TRK00   = 0x10,        // track 0
	HDC_DS_UDEF    = 0x08,        // user-defined
	HDC_DS_WRPROT  = 0x04,        // write-protected
	HDC_DS_READY   = 0x02,        // drive ready bit
	HDC_DS_WRFAULT = 0x01         // write fault
};

//===================================================================

/* Interrupt line. To be connected with the controller PCB. */
#define MCFG_HDC92X4_INTRQ_CALLBACK(_write) \
	devcb = &hdc92x4_device::set_intrq_wr_callback(*device, DEVCB_##_write);

/* DMA request line. To be connected with the controller PCB. */
#define MCFG_HDC92X4_DMARQ_CALLBACK(_write) \
	devcb = &hdc92x4_device::set_dmarq_wr_callback(*device, DEVCB_##_write);

/* DMA in progress line. To be connected with the controller PCB. */
#define MCFG_HDC92X4_DIP_CALLBACK(_write) \
	devcb = &hdc92x4_device::set_dip_wr_callback(*device, DEVCB_##_write);

/* Auxiliary Bus. These 8 lines need to be connected to external latches
   and to a counter circuitry which works together with the external RAM.
   We use the S0/S1 lines as address lines. */
#define MCFG_HDC92X4_AUXBUS_OUT_CALLBACK(_write) \
	devcb = &hdc92x4_device::set_auxbus_wr_callback(*device, DEVCB_##_write);

/* Callback to read the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC92X4_DMA_IN_CALLBACK(_read) \
	devcb = &hdc92x4_device::set_dma_rd_callback(*device, DEVCB_##_read);

/* Callback to write the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC92X4_DMA_OUT_CALLBACK(_write) \
	devcb = &hdc92x4_device::set_dma_wr_callback(*device, DEVCB_##_write);

//===================================================================

class hdc92x4_device : public device_t
{
public:
	hdc92x4_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// Accesors from the CPU side
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( reset );
	DECLARE_WRITE_LINE_MEMBER( dmaack );

	// Callbacks
	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<hdc92x4_device &>(device).m_out_intrq.set_callback(object); }
	template<class _Object> static devcb_base &set_dmarq_wr_callback(device_t &device, _Object object) { return downcast<hdc92x4_device &>(device).m_out_dmarq.set_callback(object); }
	template<class _Object> static devcb_base &set_dip_wr_callback(device_t &device, _Object object) { return downcast<hdc92x4_device &>(device).m_out_dip.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_wr_callback(device_t &device, _Object object) { return downcast<hdc92x4_device &>(device).m_out_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_rd_callback(device_t &device, _Object object) { return downcast<hdc92x4_device &>(device).m_in_dma.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_wr_callback(device_t &device, _Object object) { return downcast<hdc92x4_device &>(device).m_out_dma.set_callback(object); }

	// auxbus_in is intended to read events from the drives
	// In the real chip the status is polled; to avoid unnecessary load
	// we implement it as a push call
	void auxbus_in( UINT8 data );

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
	void device_start() override;
	void device_reset() override;

	bool m_is_hdc9234;

	devcb_write_line   m_out_intrq;    // INT line
	devcb_write_line   m_out_dmarq;    // DMA request line
	devcb_write_line   m_out_dip;      // DMA in progress line
	devcb_write8       m_out_auxbus;   // AB0-7 lines (using S0,S1 as address)
	devcb_read8        m_in_dma;       // DMA read access to the cache buffer
	devcb_write8       m_out_dma;      // DMA write access to the cache buffer

	// Internal register pointer used for sequential register loading
	int m_register_pointer;

	// Read and write registers
	UINT8 m_register_w[12];
	UINT8 m_register_r[15];

	// Interrupt management (outgoing INT pin)
	void set_interrupt(line_state intr);

	// Currently connected floppy
	floppy_image_device* m_floppy;

	// Currently connected harddisk
	mfm_harddisk_device* m_harddisk;

	// internal register OUTPUT1
	UINT8 m_output1, m_output1_old;

	// internal register OUTPUT2
	UINT8 m_output2, m_output2_old;

	// Write the output registers to the latches
	void auxbus_out();

	// Write the DMA address to the external latches
	void dma_address_out(UINT8 addrub, UINT8 addrhb, UINT8 addrlb);

	// Intermediate storage for register
	UINT8 m_regvalue;

	// Drive type that has been selected in drive_select
	int m_selected_drive_type;

	// Drive numbere that has been selected in drive_select
	int m_selected_drive_number;

	// Indicates whether the device has completed initialization
	bool m_initialized;

	// Timers to delay execution/completion of commands */
	emu_timer *m_timer;
	emu_timer *m_cmd_timer;
	// emu_timer *m_live_timer;

	// Timer callback
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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

	// Converts attotime to a string
	std::string tts(const attotime &t);

	// Current time
	std::string ttsn();

	// Utility routine to set or reset bits
	void set_bits(UINT8& byte, int mask, bool set);

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
		UINT16 shift_reg;
		UINT16 shift_reg_save;
		UINT16 crc;
		int bit_counter;
		int bit_count_total;    // used for timeout handling
		int byte_counter;
		bool data_separator_phase;
		bool last_data_bit;
		UINT8 clock_reg;
		UINT8 data_reg;
		int state;
		int next_state;
		int repeat; // for formatting
		int return_state; // for formatting
	};

	live_info m_live_state, m_checkpoint_state;
	int m_last_live_state;

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
	UINT8 get_data_from_encoding(UINT16 raw);

	// ==============================================
	//    PLL functions and interface to floppy and harddisk
	// ==============================================

	// Phase-locked loops
	fdc_pll_t m_pll, m_checkpoint_pll;

	// Clock divider value
	UINT8 m_clock_divider;

	// MFM HD encoding type
	mfmhd_enc_t m_hd_encoding;

	// Resets the PLL to the given time
	void pll_reset(const attotime &when, bool write);

	// Puts the word into the shift register directly. Changes the m_live_state members
	// shift_reg, and last_data_bit
	void encode_raw(UINT16 word);

	// Encodes a byte in FM or MFM. Called by encode_byte.
	UINT16 encode(UINT8 byte);

	// Encodes a byte in FM or MFM. Called by encode_byte.
	UINT16 encode_hd(UINT8 byte);
	UINT16 encode_a1_hd();

	// Encode the latest byte again
	void encode_again();

	// Reads from the current position on the track
	bool read_one_bit(const attotime &limit);

	// Writes to the current position on the track
	bool write_one_bit(const attotime &limit);

	// Writes to the current position on the track
	void write_on_track(UINT16 raw, int count, int next_state);

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
		UINT8 baseval;
		UINT8 mask;
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

	// Are we in FM mode?
	bool fm_mode();

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
	UINT8 current_command();

	// Step time (minus pulse width)
	int step_time();

	// Step pulse width
	int pulse_width();

	// Sector size as read from the track
	int calc_sector_size();

	// Is the currently selected drive a floppy drive?
	bool using_floppy();

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
	hdc9224_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class hdc9234_device : public hdc92x4_device
{
public:
	hdc9234_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

#endif
