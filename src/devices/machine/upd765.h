// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_DEVICES_MACHINE_UPD765_H
#define MAME_DEVICES_MACHINE_UPD765_H

#pragma once

#include "fdc_pll.h"

class floppy_image_device;

/*
 * ready = true if the ready line is physically connected to the floppy drive
 * select = true if the fdc controls the floppy drive selection
 * mode = mode_t::AT, mode_t::PS2 or mode_t::M30 for the fdcs that have reset-time selection
 */

class upd765_family_device : public device_t {
public:
	enum class mode_t { AT, PS2, M30 };

	auto intrq_wr_callback() { return intrq_cb.bind(); }
	auto drq_wr_callback() { return drq_cb.bind(); }
	auto hdl_wr_callback() { return hdl_cb.bind(); }
	auto us_wr_callback() { return us_cb.bind(); }
	auto idx_wr_callback() { return idx_cb.bind(); }

	virtual void map(address_map &map) = 0;

	uint8_t dor_r();
	void dor_w(uint8_t data);
	uint8_t tdr_r();
	void tdr_w(uint8_t data);
	uint8_t msr_r();
	void dsr_w(uint8_t data);
	uint8_t fifo_r();
	void fifo_w(uint8_t data);
	uint8_t dir_r() { return do_dir_r(); }
	void ccr_w(uint8_t data);

	uint8_t do_dir_r();

	uint8_t dma_r();
	void dma_w(uint8_t data);

	bool get_irq() const;
	bool get_drq() const;
	void tc_w(bool val);
	void ready_w(bool val);

	DECLARE_WRITE_LINE_MEMBER(tc_line_w) { tc_w(state == ASSERT_LINE); }
	DECLARE_WRITE_LINE_MEMBER(reset_w);

	void set_rate(int rate); // rate in bps, to be used when the fdc is externally frequency-controlled

	void set_ready_line_connected(bool ready);
	void set_select_lines_connected(bool select);
	void set_floppy(floppy_image_device *image);
	virtual void soft_reset();

protected:
	upd765_family_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

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

		SPEC_ND  = 0x0001
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
		SCAN_DATA,

		// Sub-states
		COMMAND_DONE,

		SEEK_MOVE,
		SEEK_WAIT_STEP_SIGNAL_TIME,
		SEEK_WAIT_STEP_SIGNAL_TIME_DONE,
		SEEK_WAIT_STEP_TIME,
		SEEK_WAIT_STEP_TIME_DONE,
		SEEK_WAIT_DONE,
		SEEK_DONE,

		HEAD_LOAD,
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
		READ_HEADER_BLOCK_HEADER,
		READ_DATA_BLOCK_HEADER,
		READ_ID_BLOCK,
		SEARCH_ADDRESS_MARK_DATA,
		SEARCH_ADDRESS_MARK_DATA_FAILED,
		READ_SECTOR_DATA,
		READ_SECTOR_DATA_BYTE,
		SCAN_SECTOR_DATA_BYTE,

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
		uint8_t pcn, st0;
		bool st0_filled;
		bool live, index, ready;
	};

	struct live_info {
		enum { PT_NONE, PT_CRC_1, PT_CRC_2 };

		attotime tm;
		int state, next_state;
		floppy_info *fi;
		uint16_t shift_reg;
		uint16_t crc;
		int bit_counter, byte_counter, previous_type;
		bool data_separator_phase, data_bit_context;
		uint8_t data_reg;
		uint8_t idbuf[6];
		fdc_pll_t pll;
	};

	static constexpr int rates[4] = { 500000, 300000, 250000, 1000000 };

	bool ready_connected, ready_polled, select_connected, select_multiplexed, has_dor;

	bool external_ready;

	int recalibrate_steps;

	mode_t mode;
	int main_phase;

	live_info cur_live, checkpoint_live;
	devcb_write_line intrq_cb, drq_cb, hdl_cb, idx_cb;
	devcb_write8 us_cb;
	bool cur_irq, other_irq, data_irq, drq, internal_drq, tc, tc_done, locked, mfm, scan_done;
	floppy_info flopi[4];

	int fifo_pos, fifo_expected, command_pos, result_pos, sectors_read;
	bool fifo_write;
	uint8_t dor, dsr, msr, fifo[16], command[16], result[16];
	uint8_t st1, st2, st3;
	uint8_t fifocfg;
	uint8_t precomp;
	uint16_t spec;
	int sector_size;
	int cur_rate;
	int selected_drive;

	emu_timer *poll_timer;

	std::string results() const;
	std::string ttsn() const;

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
		C_SCAN_EQUAL,
		C_SCAN_LOW,
		C_SCAN_HIGH,
		C_MOTOR_ONOFF,
		C_VERSION,

		C_INVALID,
		C_INCOMPLETE
	};

	void end_reset();

	void delay_cycles(emu_timer *tm, int cycles);
	void check_irq();
	void fifo_expect(int size, bool write);
	void fifo_push(uint8_t data, bool internal);
	uint8_t fifo_pop(bool internal);
	void set_drq(bool state);
	bool get_ready(int fid);
	void set_ds(int fid);

	void enable_transfer();
	void disable_transfer();
	int calc_sector_size(uint8_t size);

	void run_drive_ready_polling();

	virtual int check_command();
	virtual void start_command(int cmd);
	virtual void execute_command(int cmd);
	virtual void command_end(floppy_info &fi, bool data_completion);
	virtual uint8_t get_st3(floppy_info &fi);

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

	void scan_start(floppy_info &fi);

	void general_continue(floppy_info &fi);
	virtual void index_callback(floppy_image_device *floppy, int state);
	bool sector_matches() const;

	void live_start(floppy_info &fi, int live_state);
	void live_abort();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_run(attotime limit = attotime::never);
	void live_write_raw(uint16_t raw);
	void live_write_fm(uint8_t fm);
	void live_write_mfm(uint8_t mfm);

	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);

	virtual u8 get_drive_busy() const { return 0; }
	virtual void clr_drive_busy() { }
};

class upd765a_device : public upd765_family_device {
public:
	upd765a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool ready, bool select)
		: upd765a_device(mconfig, tag, owner, clock)
	{
		set_ready_line_connected(ready);
		set_select_lines_connected(select);
	}
	upd765a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

class upd765b_device : public upd765_family_device {
public:
	upd765b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool ready, bool select)
		: upd765b_device(mconfig, tag, owner, clock)
	{
		set_ready_line_connected(ready);
		set_select_lines_connected(select);
	}
	upd765b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

class i8272a_device : public upd765_family_device {
public:
	i8272a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool ready)
		: i8272a_device(mconfig, tag, owner, clock)
	{
		set_ready_line_connected(ready);
	}
	i8272a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

class i82072_device : public upd765_family_device {
public:
	i82072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool ready)
		: i82072_device(mconfig, tag, owner, clock)
	{
		set_ready_line_connected(ready);
	}
	i82072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

protected:
	virtual void device_start() override;

	enum motorcfg_mask
	{
		MON  = 0x0f, // motor on delay
		MOFF = 0x70, // motor off delay
		HSDA = 0x80  // high speed disk adjust
	};

	virtual void soft_reset() override;
	virtual int check_command() override;
	virtual void start_command(int cmd) override;
	virtual void execute_command(int cmd) override;
	virtual void command_end(floppy_info &fi, bool data_completion) override;
	virtual void index_callback(floppy_image_device *floppy, int state) override;
	virtual u8 get_drive_busy() const override { return drive_busy; }
	virtual void clr_drive_busy() override { drive_busy = 0; }

	void motor_control(int fid, bool start_motor);

private:
	u8 motorcfg;
	u8 motor_off_counter;
	u8 motor_on_counter;
	u8 drive_busy;
	int delayed_command;
};

class ps2_fdc_device : public upd765_family_device {
public:
	void set_mode(mode_t mode);

	uint8_t sra_r();
	uint8_t srb_r();

protected:
	ps2_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void soft_reset() override;
	virtual int check_command() override;
	virtual void execute_command(int cmd) override;

	uint8_t perpmode;
};

class smc37c78_device : public ps2_fdc_device {
public:
	smc37c78_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

class upd72065_device : public upd765_family_device {
public:
	upd72065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool ready, bool select)
		: upd72065_device(mconfig, tag, owner, clock)
	{
		set_ready_line_connected(ready);
		set_select_lines_connected(select);
	}

	upd72065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
	virtual void auxcmd_w(uint8_t data);

protected:
	upd72065_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class upd72067_device : public upd72065_device {
public:
	upd72067_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void auxcmd_w(uint8_t data) override;
};

class upd72069_device : public upd72065_device {
public:
	upd72069_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class n82077aa_device : public ps2_fdc_device {
public:
	n82077aa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, mode_t mode)
		: n82077aa_device(mconfig, tag, owner, clock)
	{
		set_mode(mode);
	}
	n82077aa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

class pc_fdc_superio_device : public upd765_family_device {
public:
	pc_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

class dp8473_device : public upd765_family_device {
public:
	dp8473_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

protected:
	virtual void soft_reset() override;
};

class pc8477a_device : public ps2_fdc_device {
public:
	pc8477a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, mode_t mode)
		: pc8477a_device(mconfig, tag, owner, clock)
	{
		set_mode(mode);
	}
	pc8477a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

class wd37c65c_device : public upd765_family_device {
public:
	wd37c65c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename X>
	wd37c65c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, X &&clock2)
		: wd37c65c_device(mconfig, tag, owner, clock)
	{
		set_clock2(std::forward<X>(clock2));
	}

	void set_clock2(uint32_t clock) { m_clock2 = clock; }
	void set_clock2(const XTAL &xtal) { set_clock2(xtal.value()); }

	virtual void map(address_map &map) override;
	virtual uint8_t get_st3(floppy_info &fi) override;

private:
	uint32_t m_clock2;
};

class mcs3201_device : public upd765_family_device {
public:
	mcs3201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto input_handler() { return m_input_handler.bind(); }

	virtual void map(address_map &map) override;
	uint8_t input_r();

protected:
	virtual void device_start() override;

private:
	devcb_read8 m_input_handler;
};

class tc8566af_device : public upd765_family_device {
public:
	tc8566af_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	void cr1_w(uint8_t data);

protected:
	virtual void device_start() override;

private:
	uint8_t m_cr1;
};

DECLARE_DEVICE_TYPE(UPD765A,        upd765a_device)
DECLARE_DEVICE_TYPE(UPD765B,        upd765b_device)
DECLARE_DEVICE_TYPE(I8272A,         i8272a_device)
DECLARE_DEVICE_TYPE(UPD72065,       upd72065_device)
DECLARE_DEVICE_TYPE(UPD72067,       upd72067_device)
DECLARE_DEVICE_TYPE(UPD72069,       upd72069_device)
DECLARE_DEVICE_TYPE(I82072,         i82072_device)
DECLARE_DEVICE_TYPE(SMC37C78,       smc37c78_device)
DECLARE_DEVICE_TYPE(N82077AA,       n82077aa_device)
DECLARE_DEVICE_TYPE(PC_FDC_SUPERIO, pc_fdc_superio_device)
DECLARE_DEVICE_TYPE(DP8473,         dp8473_device)
DECLARE_DEVICE_TYPE(PC8477A,        pc8477a_device)
DECLARE_DEVICE_TYPE(WD37C65C,       wd37c65c_device)
DECLARE_DEVICE_TYPE(MCS3201,        mcs3201_device)
DECLARE_DEVICE_TYPE(TC8566AF,       tc8566af_device)

#endif // MAME_DEVICES_MACHINE_UPD765_H
