// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    HDC9234 Hard and Floppy Disk Controller
    For details see hdc9234.c
*/
#ifndef __HDC9234_H__
#define __HDC9234_H__

#include "emu.h"
#include "imagedev/floppy.h"
#include "fdc_pll.h"

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
#define MCFG_HDC9234_INTRQ_CALLBACK(_write) \
	devcb = &hdc9234_device::set_intrq_wr_callback(*device, DEVCB_##_write);

/* DMA request line. To be connected with the controller PCB. */
#define MCFG_HDC9234_DMARQ_CALLBACK(_write) \
	devcb = &hdc9234_device::set_dmarq_wr_callback(*device, DEVCB_##_write);

/* DMA in progress line. To be connected with the controller PCB. */
#define MCFG_HDC9234_DIP_CALLBACK(_write) \
	devcb = &hdc9234_device::set_dip_wr_callback(*device, DEVCB_##_write);

/* Auxiliary Bus. These 8 lines need to be connected to external latches
   and to a counter circuitry which works together with the external RAM.
   We use the S0/S1 lines as address lines. */
#define MCFG_HDC9234_AUXBUS_OUT_CALLBACK(_write) \
	devcb = &hdc9234_device::set_auxbus_wr_callback(*device, DEVCB_##_write);

/* Callback to read the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC9234_DMA_IN_CALLBACK(_read) \
	devcb = &hdc9234_device::set_dma_rd_callback(*device, DEVCB_##_read);

/* Callback to write the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC9234_DMA_OUT_CALLBACK(_write) \
	devcb = &hdc9234_device::set_dma_wr_callback(*device, DEVCB_##_write);

//===================================================================

class hdc9234_device : public device_t
{
public:
	hdc9234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// Accesors from the CPU side
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( reset );
	DECLARE_WRITE_LINE_MEMBER( dmaack );

	// Callbacks
	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_intrq.set_callback(object); }
	template<class _Object> static devcb_base &set_dmarq_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_dmarq.set_callback(object); }
	template<class _Object> static devcb_base &set_dip_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_dip.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_rd_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_in_dma.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_dma.set_callback(object); }

	// auxbus_in is intended to read events from the drives
	// In the real chip the status is polled; to avoid unnecessary load
	// we implement it as a push call
	void auxbus_in( UINT8 data );

	// Used to reconfigure the drive connections. Floppy drive selection is done
	// using the user-programmable outputs. Hence, the connection
	// is changed outside of the controller, and by this way we let it know.
	void connect_floppy_drive(floppy_image_device *floppy);

protected:
	void device_start();
	void device_reset();

private:
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

	// Command processing
	void  process_command(UINT8 opcode);

	// Command is done
	void set_command_done(int flags);
	void set_command_done();

	// Are we in FM mode?
	bool fm_mode();

	// Recent command.
	UINT8 m_command;

	// Interrupt management (outgoing INT pin)
	void set_interrupt(line_state intr);

	// Currently connected floppy
	floppy_image_device* m_floppy;

	// internal register OUTPUT1
	UINT8 m_output1;

	// internal register OUTPUT2
	UINT8 m_output2;

	// Direction for track seek; +1 = towards center, -1 = towards rim
	int m_step_direction;

	// Write the output registers to the latches
	void sync_latches_out();

	// Write the DMA address to the external latches
	void dma_address_out();

	// Utility routine to set or reset bits
	void set_bits(UINT8& byte, int mask, bool set);

	// Drive type that has been selected in drive_select
	int m_selected_drive_type;

	// Enable head load delays
	bool m_head_load_delay_enable;

	// Timers to delay execution/completion of commands */
	emu_timer *m_timer;
	emu_timer *m_cmd_timer;
	emu_timer *m_live_timer;

	// Timer callback
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// Phase-locked loops
	fdc_pll_t m_pll, m_checkpoint_pll;

	void ready_callback(int level);
	void index_callback(int level);
	void seek_complete_callback(int level);

	void wait_time(emu_timer *tm, int microsec, int next_substate);
	void wait_time(emu_timer *tm, attotime delay, int param);

	bool on_track00();
	void wait_line(int substate);
	// ===================================================
	//   Utility functions
	// ===================================================

	astring tts(const attotime &t);
	astring ttsn();

	// ===================================================
	//   Live state machine
	// ===================================================

	struct live_info
	{
		attotime time;
		UINT16 shift_reg;
		UINT16 crc;
		int bit_counter;
		int bit_count_total;    // used for timeout handling
		bool data_separator_phase;
		UINT8 data_reg;
		int state;
		int next_state;
	};

	void live_start(int state);
	void live_run();
	void live_run_until(attotime limit);
	void wait_for_realtime(int state);
	void live_sync();
	void rollback();

	void checkpoint();

	live_info m_live_state, m_checkpoint_state;

	// ===================================================
	//   PLL functions and interface to floppy
	// ===================================================

	void pll_reset(const attotime &when);
	bool read_one_bit(const attotime &limit);

	int get_sector_size();

	// ===================================================
	//   Commands
	// ===================================================

	void process_states();
	void general_continue();

	int m_main_state;
	int m_substate;
	int m_next_state;
	int m_last_live_state;
	int m_track_delta;
	int m_retry_save;
	bool m_multi_sector;
	bool m_wait_for_index;
	bool m_stop_after_index;
	bool m_initialized;

	// Intermediate storage
	UINT8 m_data;

	typedef void (hdc9234_device::*cmdfunc)(void);

	typedef struct
	{
		UINT8 baseval;
		UINT8 mask;
		int state;
		cmdfunc command;
	} cmddef;

	static const cmddef s_command[];

	int get_step_time();
	int pulse_width();

	int m_seek_count;

	void command_continue();
	void register_write_continue();

	// Commands
	void drive_select();
	void drive_deselect();
	void restore_drive();
	void step_drive();
	void step_drive_continue();
	void set_register_pointer();
	void read_sector_physical();
	void read_sector_logical();
	void read_sector_continue();
};

#endif
