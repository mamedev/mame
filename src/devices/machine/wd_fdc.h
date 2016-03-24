// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef WD_FDC_H
#define WD_FDC_H

#include "emu.h"
#include "imagedev/floppy.h"
#include "fdc_pll.h"

/*
 * The Western Digital floppy controller family
 *

 * Chip   Bus      SCtrl SCmp Len HLO Motor Rdy MFM clock  ENMF 58 pll

 * fd1771 inverted n     n    y   y   n     y   n   2MHz   n    n  analog

 * fd1781 inverted n     n    y   y   n     y   y   1/2MHz n    n  analog

 * fd1791 inverted n     y    n   y   n     y   y   1/2MHz n    n  analog
 * fd1792 inverted n     y    n   y   n     y   n   1/2MHz n    n  analog
 * fd1793 normal   n     y    n   y   n     y   y   1/2MHz n    n  analog
 * fd1794 normal   n     y    n   y   n     y   n   1/2MHz n    n  analog
 * fd1795 inverted y     n    y   y   n     y   y   1/2MHz n    n  analog
 * fd1797 normal   y     n    y   y   n     y   y   1/2MHz n    n  analog

 * mb8866 inverted n     n    n   y   n     y   y   1/2MHz n    n  analog   (fd1791 compatible)
 * mb8876 inverted n     y    n   y   n     y   y   1/2MHz n    n  analog   (fd1791-01/02 compatible)
 * mb8877 normal   n     y    n   y   n     y   y   1/2MHz n    n  analog   (fd1793 compatible)

 * fd1761 inverted n     y    n   y   n     y   y   1MHz   n    n  analog
 * fd1763 normal   n     y    n   y   n     y   y   1MHz   n    n  analog
 * fd1765 inverted y     n    y   y   n     y   y   1MHz   n    n  analog
 * fd1767 normal   y     n    y   y   n     y   y   1MHz   n    n  analog

 * wd2791 inverted n     y    n   y   n     y   y   1/2MHz y    y  analog
 * wd2793 normal   n     y    n   y   n     y   y   1/2MHz y    y  analog
 * wd2795 inverted y     n    y   y   n     y   y   1/2MHz n    y  analog
 * wd2797 normal   y     n    y   y   n     y   y   1/2MHz n    y  analog

 * wd1770 normal   n     n    n   n   y     n   y   8Mhz   n    n  digital
 * wd1772 normal   n     n    n   n   y     n   y   8MHz   n    n  digital
 * wd1773 normal   n     y    n   n   n     y   y   8MHz   n    n  digital

 */

#define MCFG_FD1771_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1771, _clock)

#define MCFG_FD1781_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1781, _clock)

#define MCFG_FD1791_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1791, _clock)

#define MCFG_FD1792_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1792, _clock)

#define MCFG_FD1793_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1793, _clock)

#define MCFG_KR1818VG93_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, KR1818VG93, _clock)

#define MCFG_FD1794_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1794, _clock)

#define MCFG_FD1795_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1795, _clock)

#define MCFG_FD1797_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1797, _clock)

#define MCFG_MB8866_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, MB8866, _clock)

#define MCFG_MB8876_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, MB8876, _clock)

#define MCFG_MB8877_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, MB8877, _clock)

#define MCFG_FD1761_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1761, _clock)

#define MCFG_FD1763_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1763, _clock)

#define MCFG_FD1765_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1765, _clock)

#define MCFG_FD1767_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1767, _clock)

#define MCFG_WD2791_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD2791, _clock)

#define MCFG_WD2793_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD2793, _clock)

#define MCFG_WD2795_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD2795, _clock)

#define MCFG_WD2797_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD2797, _clock)

#define MCFG_WD1770_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD1770, _clock)

#define MCFG_WD1772_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD1772, _clock)

#define MCFG_WD1773_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD1773, _clock)

#define MCFG_WD_FDC_FORCE_READY \
	downcast<wd_fdc_t *>(device)->set_force_ready(true);

#define MCFG_WD_FDC_INTRQ_CALLBACK(_write) \
	devcb = &wd_fdc_t::set_intrq_wr_callback(*device, DEVCB_##_write);

#define MCFG_WD_FDC_DRQ_CALLBACK(_write) \
	devcb = &wd_fdc_t::set_drq_wr_callback(*device, DEVCB_##_write);

#define MCFG_WD_FDC_HLD_CALLBACK(_write) \
	devcb = &wd_fdc_t::set_hld_wr_callback(*device, DEVCB_##_write);

#define MCFG_WD_FDC_ENP_CALLBACK(_write) \
	devcb = &wd_fdc_t::set_enp_wr_callback(*device, DEVCB_##_write);

#define MCFG_WD_FDC_ENMF_CALLBACK(_read) \
	devcb = &wd_fdc_t::set_enmf_rd_callback(*device, DEVCB_##_read);

class wd_fdc_t : public device_t {
public:
	wd_fdc_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<wd_fdc_t &>(device).intrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<wd_fdc_t &>(device).drq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_hld_wr_callback(device_t &device, _Object object) { return downcast<wd_fdc_t &>(device).hld_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_enp_wr_callback(device_t &device, _Object object) { return downcast<wd_fdc_t &>(device).enp_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_enmf_rd_callback(device_t &device, _Object object) { return downcast<wd_fdc_t &>(device).enmf_cb.set_callback(object); }

	void soft_reset();

	void dden_w(bool dden);
	void set_floppy(floppy_image_device *floppy);
	void set_force_ready(bool force_ready);

	void cmd_w(UINT8 val);
	UINT8 status_r();
	DECLARE_READ8_MEMBER( status_r ) { return status_r(); }
	DECLARE_WRITE8_MEMBER( cmd_w ) { cmd_w(data); }

	void track_w(UINT8 val);
	UINT8 track_r();
	DECLARE_READ8_MEMBER( track_r ) { return track_r(); }
	DECLARE_WRITE8_MEMBER( track_w ) { track_w(data); }

	void sector_w(UINT8 val);
	UINT8 sector_r();
	DECLARE_READ8_MEMBER( sector_r ) { return sector_r(); }
	DECLARE_WRITE8_MEMBER( sector_w ) { sector_w(data); }

	void data_w(UINT8 val);
	UINT8 data_r();
	DECLARE_READ8_MEMBER( data_r ) { return data_r(); }
	DECLARE_WRITE8_MEMBER( data_w ) { data_w(data); }

	void gen_w(int reg, UINT8 val);
	UINT8 gen_r(int reg);
	DECLARE_READ8_MEMBER( read ) { return gen_r(offset);}
	DECLARE_WRITE8_MEMBER( write ) { gen_w(offset,data); }

	bool intrq_r();
	bool drq_r();

	bool hld_r();
	void hlt_w(bool state);

	bool enp_r();

	void index_callback(floppy_image_device *floppy, int state);

protected:
	// Chip-specific configuration flags
	bool disable_mfm;
	bool enmf;
	bool has_enmf;
	bool inverted_bus;
	bool side_control;
	bool side_compare;
	bool head_control;
	bool motor_control;
	bool ready_hooked;
	bool nonsticky_immint;
	int clock_ratio;
	const int *step_times;
	int delay_register_commit;
	int delay_command_commit;

	static const int fd179x_step_times[4];
	static const int fd176x_step_times[4];

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual int calc_sector_size(UINT8 size, UINT8 command) const;
	virtual int settle_time() const;

	virtual void pll_reset(bool fm, bool enmf, const attotime &when) = 0;
	virtual void pll_start_writing(const attotime &tm) = 0;
	virtual void pll_commit(floppy_image_device *floppy, const attotime &tm) = 0;
	virtual void pll_stop_writing(floppy_image_device *floppy, const attotime &tm) = 0;
	virtual int pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit) = 0;
	virtual bool pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit) = 0;
	virtual void pll_save_checkpoint() = 0;
	virtual void pll_retrieve_checkpoint() = 0;

private:
	enum { TM_GEN, TM_CMD, TM_TRACK, TM_SECTOR };

	//  State machine general behaviour:
	//
	//  There are three levels of state.
	//
	//  Main state is associated to (groups of) commands.  They're set
	//  by a *_start() function below, and the associated _continue()
	//  function can then be called at pretty much any time.
	//
	//  Sub state is the state of execution within a command.  The
	//  principle is that the *_start() function selects the initial
	//  substate, then the *_continue() function decides what to do,
	//  possibly changing state.  Eventually it can:
	//  - decide to wait for an event (timer, index)
	//  - end the command with command_end()
	//  - start a live state (see below)
	//
	//  In the first case, it must first switch to a waiting
	//  sub-state, then return.  The waiting sub-state must just
	//  return immediatly when *_continue is called.  Eventually the
	//  event handler function will advance the state machine to
	//  another sub-state, and things will continue synchronously.
	//
	//  On command end it's also supposed to return immediatly.
	//
	//  The last option is to switch to the next sub-state, start a
	//  live state with live_start() then return.  The next sub-state
	//  will only be called once the live state is finished.
	//
	//  Live states change continually depending on the disk contents
	//  until the next externally discernable event is found.  They
	//  are checkpointing, run until an event is found, then they wait
	//  for it.  When an event eventually happens, the changes are
	//  either committed or replayed until the sync event time.
	//
	//  The transition to IDLE is only done on a synced event.  Some
	//  other transitions, such as activating drq, are also done after
	//  syncing without exiting live mode.  Syncing in live mode is
	//  done by calling live_delay() with the state to change to after
	//  syncing.

	enum {
		// General "doing nothing" state
		IDLE,

		// Main states - the commands
		RESTORE,
		SEEK,
		STEP,
		READ_SECTOR,
		READ_TRACK,
		READ_ID,
		WRITE_TRACK,
		WRITE_SECTOR,

		// Sub states, plus the reset-time restore request

		SPINUP,
		SPINUP_WAIT,
		SPINUP_DONE,

		SETTLE_WAIT,
		SETTLE_DONE,

		DATA_LOAD_WAIT,
		DATA_LOAD_WAIT_DONE,

		SEEK_MOVE,
		SEEK_WAIT_STEP_TIME,
		SEEK_WAIT_STEP_TIME_DONE,
		SEEK_WAIT_STABILIZATION_TIME,
		SEEK_WAIT_STABILIZATION_TIME_DONE,
		SEEK_DONE,

		WAIT_INDEX,
		WAIT_INDEX_DONE,

		SCAN_ID,
		SCAN_ID_FAILED,

		SECTOR_READ,
		SECTOR_WRITE,
		TRACK_DONE,

		INITIAL_RESTORE,

		// Live states

		SEARCH_ADDRESS_MARK_HEADER,
		READ_HEADER_BLOCK_HEADER,
		READ_DATA_BLOCK_HEADER,
		READ_ID_BLOCK_TO_LOCAL,
		READ_ID_BLOCK_TO_DMA,
		READ_ID_BLOCK_TO_DMA_BYTE,
		SEARCH_ADDRESS_MARK_DATA,
		SEARCH_ADDRESS_MARK_DATA_FAILED,
		READ_SECTOR_DATA,
		READ_SECTOR_DATA_BYTE,
		READ_TRACK_DATA,
		READ_TRACK_DATA_BYTE,
		WRITE_TRACK_DATA,
		WRITE_BYTE,
		WRITE_BYTE_DONE,
		WRITE_SECTOR_PRE,
		WRITE_SECTOR_PRE_BYTE
	};

	struct live_info {
		enum { PT_NONE, PT_CRC_1, PT_CRC_2 };

		attotime tm;
		int state, next_state;
		UINT16 shift_reg;
		UINT16 crc;
		int bit_counter, byte_counter, previous_type;
		bool data_separator_phase, data_bit_context;
		UINT8 data_reg;
		UINT8 idbuf[6];
	};

	enum {
		S_BUSY = 0x01,
		S_DRQ  = 0x02,
		S_IP   = 0x02,
		S_TR00 = 0x04,
		S_LOST = 0x04,
		S_CRC  = 0x08,
		S_RNF  = 0x10,
		S_HLD  = 0x20,
		S_SPIN = 0x20, // WD1770, WD1772
		S_DDM  = 0x20,
		S_WF   = 0x20, // WD1773
		S_WP   = 0x40,
		S_NRDY = 0x80,
		S_MON  = 0x80  // WD1770, WD1772
	};

	enum {
		I_RDY = 0x01,
		I_NRDY = 0x02,
		I_IDX = 0x04,
		I_IMM = 0x08
	};


	floppy_image_device *floppy;

	emu_timer *t_gen, *t_cmd, *t_track, *t_sector;

	bool dden, status_type_1, intrq, drq, hld, hlt, enp, force_ready;
	int main_state, sub_state;
	UINT8 command, track, sector, data, status, intrq_cond;
	int last_dir;

	int counter, motor_timeout, sector_size;

	int cmd_buffer, track_buffer, sector_buffer;

	live_info cur_live, checkpoint_live;

	devcb_write_line intrq_cb, drq_cb, hld_cb, enp_cb;
	devcb_read_line enmf_cb;

	UINT8 format_last_byte;
	int format_last_byte_count;
	std::string format_description_string;

	static std::string tts(const attotime &t);
	std::string ttsn();

	void delay_cycles(emu_timer *tm, int cycles);

	// Device timer subfunctions
	void do_cmd_w();
	void do_track_w();
	void do_sector_w();
	void do_generic();


	// Main-state handling functions
	void seek_start(int state);
	void seek_continue();

	void read_sector_start();
	void read_sector_continue();

	void read_track_start();
	void read_track_continue();

	void read_id_start();
	void read_id_continue();

	void write_track_start();
	void write_track_continue();

	void write_sector_start();
	void write_sector_continue();

	void interrupt_start();

	void general_continue();
	void command_end();

	void spinup();
	void ready_callback(floppy_image_device *floppy, int state);
	bool sector_matches() const;
	bool is_ready();

	void live_start(int live_state);
	void live_abort();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_run(attotime limit = attotime::never);
	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);

	void live_write_raw(UINT16 raw);
	void live_write_mfm(UINT8 mfm);
	void live_write_fm(UINT8 fm);

	void drop_drq();
	void set_drq();
};

class wd_fdc_analog_t : public wd_fdc_t {
public:
	wd_fdc_analog_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	virtual void pll_reset(bool fm, bool enmf, const attotime &when) override;
	virtual void pll_start_writing(const attotime &tm) override;
	virtual void pll_commit(floppy_image_device *floppy, const attotime &tm) override;
	virtual void pll_stop_writing(floppy_image_device *floppy, const attotime &tm) override;
	virtual int pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit) override;
	virtual bool pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit) override;
	virtual void pll_save_checkpoint() override;
	virtual void pll_retrieve_checkpoint() override;

private:
	fdc_pll_t cur_pll, checkpoint_pll;
};

class wd_fdc_digital_t : public wd_fdc_t {
public:
	wd_fdc_digital_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	static const int wd_digital_step_times[4];

	virtual void pll_reset(bool fm, bool enmf, const attotime &when) override;
	virtual void pll_start_writing(const attotime &tm) override;
	virtual void pll_commit(floppy_image_device *floppy, const attotime &tm) override;
	virtual void pll_stop_writing(floppy_image_device *floppy, const attotime &tm) override;
	virtual int pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit) override;
	virtual bool pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit) override;
	virtual void pll_save_checkpoint() override;
	virtual void pll_retrieve_checkpoint() override;

private:
	struct digital_pll_t {
		UINT16 counter;
		UINT16 increment;
		UINT16 transition_time;
		UINT8 history;
		UINT8 slot;
		UINT8 phase_add, phase_sub, freq_add, freq_sub;
		attotime ctime;

		attotime delays[42];

		attotime write_start_time;
		attotime write_buffer[32];
		int write_position;

		void set_clock(const attotime &period);
		void reset(const attotime &when);
		int get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit);
		bool write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit);
		void start_writing(const attotime &tm);
		void commit(floppy_image_device *floppy, const attotime &tm);
		void stop_writing(floppy_image_device *floppy, const attotime &tm);
	};

	digital_pll_t cur_pll, checkpoint_pll;
};

class fd1771_t : public wd_fdc_analog_t {
public:
	fd1771_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class fd1781_t : public wd_fdc_analog_t {
public:
	fd1781_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class fd1791_t : public wd_fdc_analog_t {
public:
	fd1791_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fd1792_t : public wd_fdc_analog_t {
public:
	fd1792_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fd1793_t : public wd_fdc_analog_t {
public:
	fd1793_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class kr1818vg93_t : public wd_fdc_analog_t {
public:
	kr1818vg93_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fd1794_t : public wd_fdc_analog_t {
public:
	fd1794_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fd1795_t : public wd_fdc_analog_t {
public:
	fd1795_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class fd1797_t : public wd_fdc_analog_t {
public:
	fd1797_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class mb8866_t : public wd_fdc_analog_t {
public:
	mb8866_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mb8876_t : public wd_fdc_analog_t {
public:
	mb8876_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mb8877_t : public wd_fdc_analog_t {
public:
	mb8877_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fd1761_t : public wd_fdc_analog_t {
public:
	fd1761_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fd1763_t : public wd_fdc_analog_t {
public:
	fd1763_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fd1765_t : public wd_fdc_analog_t {
public:
	fd1765_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class fd1767_t : public wd_fdc_analog_t {
public:
	fd1767_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class wd2791_t : public wd_fdc_analog_t {
public:
	wd2791_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_WRITE_LINE_MEMBER(enmf_w) { enmf = state ? false : true; }
};

class wd2793_t : public wd_fdc_analog_t {
public:
	wd2793_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_WRITE_LINE_MEMBER(enmf_w) { enmf = state ? false : true; }
};

class wd2795_t : public wd_fdc_analog_t {
public:
	wd2795_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class wd2797_t : public wd_fdc_analog_t {
public:
	wd2797_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int calc_sector_size(UINT8 size, UINT8 command) const override;
};

class wd1770_t : public wd_fdc_digital_t {
public:
	wd1770_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class wd1772_t : public wd_fdc_digital_t {
public:
	wd1772_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int settle_time() const override;
};

class wd1773_t : public wd_fdc_digital_t {
public:
	wd1773_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type FD1771;

extern const device_type FD1781;

extern const device_type FD1791;
extern const device_type FD1792;
extern const device_type FD1793;
extern const device_type FD1794;
extern const device_type KR1818VG93;
extern const device_type FD1795;
extern const device_type FD1797;

extern const device_type MB8866;
extern const device_type MB8876;
extern const device_type MB8877;

extern const device_type FD1761;
extern const device_type FD1763;
extern const device_type FD1765;
extern const device_type FD1767;

extern const device_type WD2791;
extern const device_type WD2793;
extern const device_type WD2795;
extern const device_type WD2797;

extern const device_type WD1770;
extern const device_type WD1772;
extern const device_type WD1773;

#endif
