#ifndef __UPD765_F_H__
#define __UPD765_F_H__

#include "emu.h"
#include "imagedev/floppy.h"

/*
 * ready = true if the ready line is physically connected to the floppy drive
 * select = true if the fdc controls the floppy drive selection
 * mode = MODE_AT, MODE_PS2 or MODE_M30 for the fdcs that have reset-time selection
 */

#define MCFG_UPD765A_ADD(_tag, _ready, _select)	\
	MCFG_DEVICE_ADD(_tag, UPD765A, 0)			\
	downcast<upd765a_device *>(device)->set_ready_line_connected(_ready);	\
	downcast<upd765a_device *>(device)->set_select_lines_connected(_select);

#define MCFG_UPD765B_ADD(_tag, _ready, _select)	\
	MCFG_DEVICE_ADD(_tag, UPD765B, 0)			\
	downcast<upd765b_device *>(device)->set_ready_line_connected(_ready);	\
	downcast<upd765b_device *>(device)->set_select_lines_connected(_select);

#define MCFG_I8272A_ADD(_tag, _ready)	\
	MCFG_DEVICE_ADD(_tag, I8272A, 0)	\
	downcast<i8272a_device *>(device)->set_ready_line_connected(_ready);

#define MCFG_UPD72065_ADD(_tag, _ready, _select)	\
	MCFG_DEVICE_ADD(_tag, UPD72065, 0)				\
	downcast<upd72065_device *>(device)->set_ready_line_connected(_ready);	\
	downcast<upd72065_device *>(device)->set_select_lines_connected(_select);

#define MCFG_SMC37C78_ADD(_tag)	\
	MCFG_DEVICE_ADD(_tag, SMC37C78, 0)

#define MCFG_N82077AA_ADD(_tag, _mode)	\
	MCFG_DEVICE_ADD(_tag, N82077AA, 0)	\
	downcast<n82077aa_device *>(device)->set_mode(_mode);

#define MCFG_PC_FDC_SUPERIO_ADD(_tag)	\
	MCFG_DEVICE_ADD(_tag, PC_FDC_SUPERIO, 0)

/* Interface required for PC ISA wrapping */
class pc_fdc_interface : public device_t {
public:
	typedef delegate<void (bool state)> line_cb;
	typedef delegate<UINT8 ()> byte_read_cb;
	typedef delegate<void (UINT8)> byte_write_cb;

	pc_fdc_interface(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) : device_t(mconfig, type, name, tag, owner, clock) {}

	virtual void setup_intrq_cb(line_cb cb) = 0;
	virtual void setup_drq_cb(line_cb cb) = 0;

	/* Note that the address map must cover and handle the whole 0-7
     * range.  The upd765, while conforming to the rest of the
     * interface, is not eligible as a result.
     */

	virtual DECLARE_ADDRESS_MAP(map, 8) = 0;

	virtual UINT8 dma_r() = 0;
	virtual void dma_w(UINT8 data) = 0;

	virtual void tc_w(bool val) = 0;
	virtual UINT8 do_dir_r() = 0;
};

class upd765_family_device : public pc_fdc_interface {
public:
	enum { MODE_AT, MODE_PS2, MODE_M30 };

	upd765_family_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	void setup_intrq_cb(line_cb cb);
	void setup_drq_cb(line_cb cb);

	virtual DECLARE_ADDRESS_MAP(map, 8) = 0;

	DECLARE_READ8_MEMBER (sra_r);
	DECLARE_READ8_MEMBER (srb_r);
	DECLARE_READ8_MEMBER (dor_r);
	DECLARE_WRITE8_MEMBER(dor_w);
	DECLARE_READ8_MEMBER (tdr_r);
	DECLARE_WRITE8_MEMBER(tdr_w);
	DECLARE_READ8_MEMBER (msr_r);
	DECLARE_WRITE8_MEMBER(dsr_w);
	DECLARE_READ8_MEMBER (fifo_r);
	DECLARE_WRITE8_MEMBER(fifo_w);
	DECLARE_READ8_MEMBER (dir_r);
	DECLARE_WRITE8_MEMBER(ccr_w);

	virtual UINT8 do_dir_r();

	UINT8 dma_r();
	void dma_w(UINT8 data);

	// Same as the previous ones, but as memory-mappable members
	DECLARE_READ8_MEMBER(mdma_r);
	DECLARE_WRITE8_MEMBER(mdma_w);

	bool get_irq() const;
	bool get_drq() const;
	void tc_w(bool val);
	void ready_w(bool val);

	void set_rate(int rate); // rate in bps, to be used when the fdc is externally frequency-controlled

	void set_mode(int mode);
	void set_ready_line_connected(bool ready);
	void set_select_lines_connected(bool select);
	void set_floppy(floppy_image_device *image);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	enum {
		TIMER_DRIVE_READY_POLLING = 4
	};

	enum {
		PHASE_CMD, PHASE_EXEC, PHASE_RESULT
	};

	enum {
		MSR_DB   = 0x0f,
		MSR_CB   = 0x10,
		MSR_EXM  = 0x20,
		MSR_DIO  = 0x40,
		MSR_RQM  = 0x80,

		ST0_UNIT = 0x07,
		ST0_NR   = 0x08,
		ST0_EC   = 0x10,
		ST0_SE   = 0x20,
		ST0_FAIL = 0x40,
		ST0_UNK  = 0x80,
		ST0_ABRT = 0xc0,

		ST1_MA   = 0x01,
		ST1_NW   = 0x02,
		ST1_ND   = 0x04,
		ST1_OR   = 0x10,
		ST1_DE   = 0x20,
		ST1_EN   = 0x80,

		ST2_MD   = 0x01,
		ST2_BC   = 0x02,
		ST2_SN   = 0x04,
		ST2_SH   = 0x08,
		ST2_WC   = 0x10,
		ST2_DD   = 0x20,
		ST2_CM   = 0x40,

		ST3_UNIT = 0x07,
		ST3_TS   = 0x08,
		ST3_T0   = 0x10,
		ST3_RY   = 0x20,
		ST3_WP   = 0x40,
		ST3_FT   = 0x80,

		FIF_THR  = 0x0f,
		FIF_POLL = 0x10,
		FIF_DIS  = 0x20,
		FIF_EIS  = 0x40,

		SPEC_ND  = 0x0001,
	};


	enum {
		// General "doing nothing" state
		IDLE,

		// Main states
		RECALIBRATE,
		SEEK,
		READ_DATA,
		WRITE_DATA,
		READ_TRACK,
		FORMAT_TRACK,
		READ_ID,

		// Sub-states
		COMMAND_DONE,

		SEEK_MOVE,
		SEEK_WAIT_STEP_SIGNAL_TIME,
		SEEK_WAIT_STEP_SIGNAL_TIME_DONE,
		SEEK_WAIT_STEP_TIME,
		SEEK_WAIT_STEP_TIME_DONE,
		SEEK_DONE,

		HEAD_LOAD_DONE,

		WAIT_INDEX,
		WAIT_INDEX_DONE,

		SCAN_ID,
		SCAN_ID_FAILED,

		SECTOR_READ,
		SECTOR_WRITTEN,
		TC_DONE,

		TRACK_DONE,

		// Live states
		SEARCH_ADDRESS_MARK_HEADER,
		SEARCH_ADDRESS_MARK_HEADER_FM,
		READ_HEADER_BLOCK_HEADER,
		READ_DATA_BLOCK_HEADER,
		READ_ID_BLOCK,
		SEARCH_ADDRESS_MARK_DATA,
		SEARCH_ADDRESS_MARK_DATA_FM,
		SEARCH_ADDRESS_MARK_DATA_FAILED,
		READ_SECTOR_DATA,
		READ_SECTOR_DATA_BYTE,

		WRITE_SECTOR_SKIP_GAP2,
		WRITE_SECTOR_SKIP_GAP2_BYTE,
		WRITE_SECTOR_DATA,
		WRITE_SECTOR_DATA_BYTE,

		WRITE_TRACK_PRE_SECTORS,
		WRITE_TRACK_PRE_SECTORS_BYTE,

		WRITE_TRACK_SECTOR,
		WRITE_TRACK_SECTOR_BYTE,

		WRITE_TRACK_POST_SECTORS,
		WRITE_TRACK_POST_SECTORS_BYTE,
	};

	struct pll_t {
		attotime ctime, period, min_period, max_period, period_adjust_base, phase_adjust;

		attotime write_start_time;
		attotime write_buffer[32];
		int write_position;
		int freq_hist;

		void set_clock(attotime period);
		void reset(attotime when);
		int get_next_bit(attotime &tm, floppy_image_device *floppy, attotime limit);
		bool write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, attotime limit);
		void start_writing(attotime tm);
		void commit(floppy_image_device *floppy, attotime tm);
		void stop_writing(floppy_image_device *floppy, attotime tm);
	};

	struct floppy_info {
		enum { IRQ_NONE, IRQ_SEEK, IRQ_POLLED };
		emu_timer *tm;
		floppy_image_device *dev;
		int id;
		int main_state, sub_state;
		int dir, counter;
		UINT8 pcn;
		int irq;
		bool live, index, ready;
	};

	struct live_info {
		enum { PT_NONE, PT_CRC_1, PT_CRC_2 };

		attotime tm;
		int state, next_state;
		floppy_info *fi;
		UINT16 shift_reg;
		UINT16 crc;
		int bit_counter, byte_counter, previous_type;
		bool data_separator_phase, data_bit_context;
		UINT8 data_reg;
		UINT8 idbuf[6];
		pll_t pll;
	};

	static int rates[4];

	bool ready_connected, ready_polled, select_connected;

	bool external_ready;

	int mode;
	int main_phase;

	live_info cur_live, checkpoint_live;
	line_cb intrq_cb, drq_cb;
	bool cur_irq, polled_irq, data_irq, drq, internal_drq, tc, tc_done, locked;
	floppy_info flopi[4];

	int fifo_pos, fifo_expected, command_pos, result_pos;
	bool fifo_write;
	UINT8 dor, dsr, msr, fifo[16], command[16], result[16];
	UINT8 st0, st1, st2, st3;
	UINT8 fifocfg, dor_reset;
	UINT8 precomp, perpmode;
	UINT16 spec;
	int sector_size;
	int cur_rate;

	emu_timer *poll_timer;

	static astring tts(attotime t);
	astring ttsn();

	enum {
		C_CONFIGURE,
		C_DUMP_REG,
		C_FORMAT_TRACK,
		C_LOCK,
		C_PERPENDICULAR,
		C_READ_DATA,
		C_READ_ID,
		C_READ_TRACK,
		C_RECALIBRATE,
		C_SEEK,
		C_SENSE_DRIVE_STATUS,
		C_SENSE_INTERRUPT_STATUS,
		C_SPECIFY,
		C_WRITE_DATA,

		C_INVALID,
		C_INCOMPLETE,
	};

	void delay_cycles(emu_timer *tm, int cycles);
	void check_irq();
	void soft_reset();
	void fifo_expect(int size, bool write);
	void fifo_push(UINT8 data, bool internal);
	UINT8 fifo_pop(bool internal);
	void set_drq(bool state);
	bool get_ready(int fid);

	void enable_transfer();
	void disable_transfer();
	int calc_sector_size(UINT8 size);

	void run_drive_ready_polling();

	int check_command();
	void start_command(int cmd);
	void command_end(floppy_info &fi, bool data_completion);

	void recalibrate_start(floppy_info &fi);
	void seek_start(floppy_info &fi);
	void seek_continue(floppy_info &fi);

	void read_data_start(floppy_info &fi);
	void read_data_continue(floppy_info &fi);

	void write_data_start(floppy_info &fi);
	void write_data_continue(floppy_info &fi);

	void read_track_start(floppy_info &fi);
	void read_track_continue(floppy_info &fi);

	void format_track_start(floppy_info &fi);
	void format_track_continue(floppy_info &fi);

	void read_id_start(floppy_info &fi);
	void read_id_continue(floppy_info &fi);

	void general_continue(floppy_info &fi);
	void index_callback(floppy_image_device *floppy, int state);
	bool sector_matches() const;

	void live_start(floppy_info &fi, int live_state, bool mfm);
	void live_abort();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_run(attotime limit = attotime::never);
	void live_write_raw(UINT16 raw);
	void live_write_fm(UINT8 fm);
	void live_write_mfm(UINT8 mfm);

	bool read_one_bit(attotime limit);
	bool write_one_bit(attotime limit);
};

class upd765a_device : public upd765_family_device {
public:
	upd765a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

class upd765b_device : public upd765_family_device {
public:
	upd765b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

class i8272a_device : public upd765_family_device {
public:
	i8272a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

class smc37c78_device : public upd765_family_device {
public:
	smc37c78_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

class upd72065_device : public upd765_family_device {
public:
	upd72065_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

class n82077aa_device : public upd765_family_device {
public:
	n82077aa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

class pc_fdc_superio_device : public upd765_family_device {
public:
	pc_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);
};

extern const device_type UPD765A;
extern const device_type UPD765B;
extern const device_type I8272A;
extern const device_type UPD72065;
extern const device_type SMC37C78;
extern const device_type N82077AA;
extern const device_type PC_FDC_SUPERIO;

#endif
