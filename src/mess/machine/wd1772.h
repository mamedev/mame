#ifndef WD1772_H
#define WD1772_H

#include "emu.h"
#include "imagedev/floppy.h"

#define MCFG_WD1770x_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD1770x, _clock)

#define MCFG_WD1772x_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD1772x, _clock)

#define MCFG_WD1773x_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD1773x, _clock)

#define MCFG_WD2793x_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD2793x, _clock)

#define MCFG_WD2797x_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, WD2797x, _clock)

#define MCFG_FD1793x_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, FD1793x, _clock)

class wd177x_t : public device_t {
public:
	typedef delegate<void (bool state)> line_cb;

	wd177x_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	void dden_w(bool dden);
	void set_floppy(floppy_image_device *floppy);
	void setup_intrq_cb(line_cb cb);
	void setup_drq_cb(line_cb cb);
	void setup_hld_cb(line_cb cb);
	void setup_enp_cb(line_cb cb);

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

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual bool has_ready() const;
	virtual bool has_motor() const = 0;
	virtual bool has_head_load() const;
	virtual bool has_side_check() const;
	virtual bool has_side_select() const;
	virtual bool has_sector_length_select() const;
	virtual bool has_precompensation() const;
	virtual int step_time(int mode) const;
	virtual int settle_time() const;

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
	//  for it.  When an event eventually happen the the changes are
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

		// Sub states

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
		WRITE_SECTOR_PRE_BYTE,
	};

	struct pll_t {
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

		void set_clock(attotime period);
		void reset(attotime when);
		int get_next_bit(attotime &tm, floppy_image_device *floppy, attotime limit);
		bool write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, attotime limit);
		void start_writing(attotime tm);
		void commit(floppy_image_device *floppy, attotime tm);
		void stop_writing(floppy_image_device *floppy, attotime tm);
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
		pll_t pll;
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

	bool dden, status_type_1, intrq, drq, hld, hlt, enp;
	int main_state, sub_state;
	UINT8 command, track, sector, data, status, intrq_cond;
	int last_dir;

	int counter, motor_timeout, sector_size;

	int cmd_buffer, track_buffer, sector_buffer;

	live_info cur_live, checkpoint_live;
	line_cb intrq_cb, drq_cb, hld_cb, enp_cb;

	static astring tts(attotime t);
	astring ttsn();

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
	void index_callback(floppy_image_device *floppy, int state);
	bool sector_matches() const;
	bool is_ready();

	void live_start(int live_state);
	void live_abort();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_run(attotime limit = attotime::never);
	bool read_one_bit(attotime limit);
	bool write_one_bit(attotime limit);

	void live_write_raw(UINT16 raw);
	void live_write_mfm(UINT8 mfm);

	void drop_drq();
	void set_drq();
};

class wd1770_t : public wd177x_t {
public:
	wd1770_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual bool has_motor() const { return true; }
	virtual bool has_precompensation() const { return true; }
};

class wd1772_t : public wd177x_t {
public:
	wd1772_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual bool has_motor() const { return true; }
	virtual bool has_precompensation() const { return true; }
	virtual int step_time(int mode) const;
	virtual int settle_time() const;
};

class wd1773_t : public wd177x_t {
public:
	wd1773_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual bool has_motor() const { return false; }
	virtual bool has_head_load() const { return true; }
	virtual bool has_side_check() const { return true; }
};

class wd2793_t : public wd177x_t {
public:
	wd2793_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual bool has_ready() const { return true; }
	virtual bool has_motor() const { return false; }
	virtual bool has_head_load() const { return true; }
	virtual bool has_side_check() const { return true; }
};

class wd2797_t : public wd177x_t {
public:
	wd2797_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual bool has_ready() const { return true; }
	virtual bool has_motor() const { return false; }
	virtual bool has_head_load() const { return true; }
	virtual bool has_side_select() const { return true; }
	virtual bool has_sector_length_select() const { return true; }
};

class fd1793_t : public wd177x_t {
public:
	fd1793_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual bool has_ready() const { return true; }
	virtual bool has_motor() const { return false; }
	virtual bool has_head_load() const { return true; }
	virtual bool has_side_check() const { return true; }
};

extern const device_type WD1770x;
extern const device_type WD1772x;
extern const device_type WD1773x;
extern const device_type WD2793x;
extern const device_type WD2797x;
extern const device_type FD1793x;

#endif
