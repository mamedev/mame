// license:BSD-3-Clause
// copyright-holders:Carl,Olivier Galibert
#ifndef I8271N_H_
#define I8271N_H_

#include "emu.h"
#include "imagedev/floppy.h"
#include "fdc_pll.h"

#define MCFG_I8271_IRQ_CALLBACK(_write) \
	devcb = &i8271_device::set_intrq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8271_DRQ_CALLBACK(_write) \
	devcb = &i8271_device::set_drq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8271_HDL_CALLBACK(_write) \
	devcb = &i8271_device::set_hdl_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8271_OPT_CALLBACK(_write) \
	devcb = &i8271_device::set_opt_wr_callback(*device, DEVCB_##_write);

/***************************************************************************
    MACROS
***************************************************************************/

class i8271_device : public device_t
{
public:
	i8271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~i8271_device() {}

	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<i8271_device &>(device).intrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<i8271_device &>(device).drq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_hdl_wr_callback(device_t &device, _Object object) { return downcast<i8271_device &>(device).hdl_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_opt_wr_callback(device_t &device, _Object object) { return downcast<i8271_device &>(device).opt_cb.set_callback(object); }

	DECLARE_READ8_MEMBER (sr_r);
	DECLARE_READ8_MEMBER (rr_r);
	DECLARE_WRITE8_MEMBER(reset_w) { if(data == 1) soft_reset(); }
	DECLARE_WRITE8_MEMBER(cmd_w);
	DECLARE_WRITE8_MEMBER(param_w);
	DECLARE_READ8_MEMBER (data_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_ADDRESS_MAP(map, 8);

	void ready_w(bool val);

	void set_rate(int rate); // rate in bps, to be used when the fdc is externally frequency-controlled

	void set_ready_line_connected(bool ready);
	void set_select_lines_connected(bool select);
	void set_floppy(floppy_image_device *image);
	void soft_reset();

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum {
		PHASE_IDLE, PHASE_CMD, PHASE_EXEC, PHASE_RESULT
	};

	enum {
		RR_DEL = 0x20,
		RR_CT  = 0x18,
		RR_CC  = 0x06,

		SR_BSY = 0x80,
		SR_CF  = 0x40,
		SR_PF  = 0x20,
		SR_RF  = 0x10,
		SR_IRQ = 0x08,
		SR_DRQ = 0x04
	};

	enum {
		ERR_NONE = 0x00,
		ERR_SMEQ = 0x02,
		ERR_SMNE = 0x04,
		ERR_CLK  = 0x08,
		ERR_DMA  = 0x0a,
		ERR_ICRC = 0x0c,
		ERR_DCRC = 0x0e,
		ERR_NR   = 0x10,
		ERR_WP   = 0x12,
		ERR_T0NF = 0x14,
		ERR_WF   = 0x16,
		ERR_NF   = 0x18
	};

	enum {
		// General "doing nothing" state
		IDLE,

		// Main states
		RECALIBRATE,
		SEEK,
		READ_DATA,
		WRITE_DATA,
		FORMAT_TRACK,
		READ_ID,
		SCAN_DATA,
		VERIFY_DATA,

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
		READ_ID_BLOCK,
		SEARCH_ADDRESS_MARK_DATA,
		SEARCH_ADDRESS_MARK_DATA_FAILED,
		READ_SECTOR_DATA,
		READ_SECTOR_DATA_BYTE,
		SCAN_SECTOR_DATA_BYTE,
		VERIFY_SECTOR_DATA_BYTE,

		WRITE_SECTOR_SKIP_GAP2,
		WRITE_SECTOR_SKIP_GAP2_BYTE,
		WRITE_SECTOR_DATA,
		WRITE_SECTOR_DATA_BYTE,

		WRITE_TRACK_PRE_SECTORS,
		WRITE_TRACK_PRE_SECTORS_BYTE,

		WRITE_TRACK_SECTOR,
		WRITE_TRACK_SECTOR_BYTE,

		WRITE_TRACK_POST_SECTORS,
		WRITE_TRACK_POST_SECTORS_BYTE
	};

	struct pll_t {
		attotime ctime, period, min_period, max_period, period_adjust_base, phase_adjust;

		attotime write_start_time;
		attotime write_buffer[32];
		int write_position;
		int freq_hist;

		void set_clock(const attotime &period);
		void reset(const attotime &when);
		int get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit);
		bool write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit);
		void start_writing(const attotime &tm);
		void commit(floppy_image_device *floppy, const attotime &tm);
		void stop_writing(floppy_image_device *floppy, const attotime &tm);
	};

	struct floppy_info {
		enum { IRQ_NONE, IRQ_POLLED, IRQ_SEEK, IRQ_DONE };
		emu_timer *tm;
		floppy_image_device *dev;
		int id;
		int main_state, sub_state;
		int dir, counter;
		UINT8 pcn, badtrack[2];
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
		fdc_pll_t pll;
	};

	bool ready_connected, select_connected;

	bool external_ready;

	int mode;
	int main_phase;

	live_info cur_live, checkpoint_live;
	devcb_write_line intrq_cb, drq_cb, hdl_cb, opt_cb;
	bool irq, drq, scan_done, scan_match;
	floppy_info flopi[2];

	int command_pos, sectors_read, scan_len;
	UINT8 command[6], dma_data;
	UINT8 rr, scan_sec, moder;
	UINT8 precomp, perpmode, scan_cnt[2];
	UINT8 srate, hset, icnt, hload;
	int sector_size;
	int cur_rate;

	static std::string tts(attotime t);
	std::string ttsn();

	enum {
		C_FORMAT_TRACK,
		C_READ_DATA_SINGLE,
		C_READ_DATA_MULTI,
		C_VERIFY_DATA_SINGLE,
		C_VERIFY_DATA_MULTI,
		C_WRITE_DATA_SINGLE,
		C_WRITE_DATA_MULTI,
		C_READ_ID,
		C_SEEK,
		C_READ_DRIVE_STATUS,
		C_SPECIFY,
		C_SCAN,
		C_WRITE_SPECIAL_REGISTER,
		C_READ_SPECIAL_REGISTER,

		C_INVALID,
		C_INCOMPLETE
	};

	void delay_cycles(emu_timer *tm, int cycles);
	void set_drq(bool state);
	void set_irq(bool state);
	bool get_ready(int fid);

	void enable_transfer();
	void disable_transfer();
	int calc_sector_size(UINT8 size);

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

	void format_track_start(floppy_info &fi);
	void format_track_continue(floppy_info &fi);

	void read_id_start(floppy_info &fi);
	void read_id_continue(floppy_info &fi);

	void scan_start(floppy_info &fi);
	void verify_data_start(floppy_info &fi);

	void general_continue(floppy_info &fi);
	void index_callback(floppy_image_device *floppy, int state);
	bool sector_matches() const;

	void live_start(floppy_info &fi, int live_state);
	void live_abort();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_run(attotime limit = attotime::never);
	void live_write_raw(UINT16 raw);
	void live_write_fm(UINT8 fm);

	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);
	bool set_output(UINT8 data);
	bool get_input(UINT8 *data);
};

extern const device_type I8271;

#endif
