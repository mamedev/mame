// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DVK "MX" floppy controller (decimal ID 3.057.122)

***************************************************************************/

#ifndef MAME_BUS_QBUS_DVK_MX_H
#define MAME_BUS_QBUS_DVK_MX_H

#pragma once

#include "qbus.h"

#include "imagedev/floppy.h"
#include "machine/fdc_pll.h"
#include "machine/pdp11.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dvk_mx_device

class dvk_mx_device : public device_t, public device_qbus_card_interface
{
public:
	dvk_mx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_floppy);
	TIMER_CALLBACK_MEMBER(twokhz_tick);

private:
	enum {
		IDLE,

		// Main states
		SEEK,
		READ_DATA,
		WRITE_DATA,

		// Sub-states
		COMMAND_DONE,

		SEEK_MOVE,
		SEEK_WAIT_STEP_SIGNAL_TIME,
		SEEK_WAIT_STEP_SIGNAL_TIME_DONE,
		SEEK_WAIT_STEP_TIME,
		SEEK_WAIT_STEP_TIME_DONE,
		SEEK_WAIT_DONE,
		SEEK_DONE,

		WAIT_INDEX,
		WAIT_INDEX_DONE,

		SCAN_ID,
		SCAN_ID_FAILED,

		TRACK_READ,
		TRACK_WRITTEN,

		// Live states
		SEARCH_ID,
		READ_TRACK_DATA,
		READ_TRACK_DATA_BYTE,

		WRITE_TRACK_DATA,
		WRITE_TRACK_DATA_BYTE,
	};

	enum {
		MXCSR_DRVSE_L = 0000002,
		MXCSR_STEP    = 0000020,
		MXCSR_DIR     = 0000040,
		MXCSR_MON     = 0000100,
		MXCSR_TIMER   = 0000200,
		MXCSR_ERR     = 0000400,
		MXCSR_INDEX   = 0001000,
		MXCSR_WP      = 0002000,
		MXCSR_TRK0    = 0004000,
		MXCSR_TOPHEAD = 0010000,
		MXCSR_WRITE   = 0020000,
		MXCSR_GO      = 0040000,
		MXCSR_TR      = 0100000,
		MXCSR_V_DRIVE = 2,
		MXCSR_M_DRIVE = 3,
		MXCSR_DRIVE   = (MXCSR_M_DRIVE << MXCSR_V_DRIVE),
		MXCSR_RD      = (MXCSR_TR|MXCSR_GO|MXCSR_WRITE|MXCSR_TOPHEAD|MXCSR_TRK0|MXCSR_WP|MXCSR_INDEX|MXCSR_ERR|MXCSR_TIMER|MXCSR_MON|MXCSR_DIR|MXCSR_STEP|MXCSR_DRIVE|MXCSR_DRVSE_L),
		MXCSR_WR      = (MXCSR_WRITE|MXCSR_TOPHEAD|MXCSR_TIMER|MXCSR_MON|MXCSR_DIR|MXCSR_STEP|MXCSR_DRIVE|MXCSR_DRVSE_L)
	};

	struct floppy_info {
		floppy_info();

		emu_timer *tm;
		floppy_image_device *dev;
		int id;
		int main_state, sub_state;
		int dir, counter;
		bool live, index;
	};

	struct live_info {
		live_info();

		attotime tm;
		int state, next_state;
		floppy_info *fi;
		uint32_t shift_reg;
		int bit_counter, byte_counter;
		bool data_separator_phase;
		uint16_t data_reg, cksum;
		fdc_pll_t pll;
	};

	required_device_array<floppy_connector, 4> m_connectors;

	uint16_t m_mxcs;
	uint16_t m_rbuf;
	uint16_t m_wbuf;

	int selected_drive;

	emu_timer *m_timer_2khz;

	floppy_info flopi[4];
	live_info cur_live, checkpoint_live;

	static void floppy_formats(format_registration &fr);

	std::string ttsn() const;

	void execute_command(int command);
	void set_ds(int fid);

	void seek_start(floppy_info &fi);
	void seek_continue(floppy_info &fi);

	void read_data_start(floppy_info &fi);
	void read_data_continue(floppy_info &fi);

	void write_data_start(floppy_info &fi);
	void write_data_continue(floppy_info &fi);

	void general_continue(floppy_info &fi);
	void index_callback(floppy_image_device *floppy, int state);

	void live_start(floppy_info &fi, int live_state);
	void live_abort();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_run(attotime limit = attotime::never);
	void live_write_fm(uint16_t fm);

	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);
};


// device type declaration
DECLARE_DEVICE_TYPE(DVK_MX, dvk_mx_device)

#endif // MAME_BUS_QBUS_DVK_MX_H
