#ifndef __N82077AA_H__
#define __N82077AA_H__

#include "emu.h"
#include "imagedev/floppy.h"

#define MCFG_N82077AA_ADD(_tag, _mode)	\
	MCFG_DEVICE_ADD(_tag, N82077AA, 0)	\
	downcast<n82077aa_device *>(device)->set_mode(_mode);

class n82077aa_device : public device_t {
public:
	typedef delegate<void (bool state)> line_cb;

	enum { MODE_AT, MODE_PS2, MODE_M30 };

	n82077aa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_mode(int mode);
	void setup_intrq_cb(line_cb cb);
	void setup_drq_cb(line_cb cb);

	DECLARE_ADDRESS_MAP(amap, 8);

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

	UINT8 dma_r();
	void dma_w(UINT8 data);
	bool get_drq() const;

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum {
		PHASE_CMD, PHASE_EXEC, PHASE_RESULT
	};

	enum {
		// General "doing nothing" state
		IDLE,

		// Main states
		RECALIBRATE,
		SEEK,
		READ_DATA,
		READ_ID,

		// Sub-states
		SEEK_MOVE,
		SEEK_WAIT_STEP_SIGNAL_TIME,
		SEEK_WAIT_STEP_SIGNAL_TIME_DONE,
		SEEK_WAIT_STEP_TIME,
		SEEK_WAIT_STEP_TIME_DONE,
		SEEK_DONE,

		HEAD_LOAD_DONE,

		SCAN_ID,
		SCAN_ID_FAILED,

		SECTOR_READ,

		// Live states
		SEARCH_ADDRESS_MARK_HEADER,
		READ_HEADER_BLOCK_HEADER,
		READ_DATA_BLOCK_HEADER,
		READ_ID_BLOCK_TO_LOCAL,
		SEARCH_ADDRESS_MARK_DATA,
		SEARCH_ADDRESS_MARK_DATA_FAILED,
		READ_SECTOR_DATA,
		READ_SECTOR_DATA_BYTE,

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
		emu_timer *tm;
		floppy_image_device *dev;
		int id;
		int main_state, sub_state;
		int dir, status;
		UINT8 pcn;
		bool irq, live, index;
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

	int mode;
	int main_phase;
	int counter;

	live_info cur_live, checkpoint_live;
	line_cb intrq_cb, drq_cb;
	bool cur_irq, data_irq, drq;
	floppy_info flopi[4];

	int fifo_pos, fifo_expected, command_pos, result_pos;
	bool fifo_write;
	UINT8 dor, dsr, msr, fifo[16], command[16], result[16];
	UINT8 fifocfg;
	UINT16 spec;
	int sector_size;

	static astring tts(attotime t);
	astring ttsn();

	enum {
		C_CONFIGURE,
		C_PERPENDICULAR,
		C_READ_DATA,
		C_READ_ID,
		C_RECALIBRATE,
		C_SEEK,
		C_SENSE_INTERRUPT_STATUS,
		C_SPECIFY,

		C_INVALID,
		C_INCOMPLETE,
	};

	void delay_cycles(emu_timer *tm, int cycles);
	void check_irq();
	void fifo_expect(int size, bool write);
	void fifo_push(UINT8 data);
	void set_drq(bool state);

	int check_command();
	void start_command(int cmd);
	void command_end(floppy_info &fi, bool data_completion, int status);

	void recalibrate_start(floppy_info &fi);
	void seek_start(floppy_info &fi);
	void seek_continue(floppy_info &fi);

	void read_data_start(floppy_info &fi);
	void read_data_continue(floppy_info &fi);

	void read_id_start(floppy_info &fi);
	void read_id_continue(floppy_info &fi);

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

	bool read_one_bit(attotime limit);
	bool write_one_bit(attotime limit);
};

extern const device_type N82077AA;

#endif
