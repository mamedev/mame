// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1801VP1-128 gate array (MFM codec for floppy controllers)

**********************************************************************
                            _____   _____
                  _AD0   1 |*    \_/     | 42  +5V
                  _AD1   2 |             | 41  _DS0
                  _AD2   3 |             | 30  _DS1
                  _AD3   4 |             | 39  _DS2
                  _AD4   5 |             | 38  _DS3
                  _AD5   6 |             | 37  _MSW
                  _AD6   7 |             | 36  HS
                  _AD7   8 |             | 35  DIR
                  _AD8   9 |             | 34  _ST
                  _AD9  10 |             | 33  TR0
                 _AD10  11 | 1801VP1-128 | 32  RDY
                 _AD11  12 |             | 31  WPR
                 _AD12  13 |             | 30  _REZ
                 _AD13  14 |             | 29  DI
                 _AD14  15 |             | 28  _WRE
                 _AD15  16 |             | 27  _D01
                 _SYNC  17 |             | 26  _D02
                  _DIN  18 |             | 25  _D03
                 _DOUT  19 |             | 24  IND
                 _INIT  20 |             | 23  _RPLY
                   GND  21 |_____________| 22  CLC

**********************************************************************/

#ifndef MAME_MACHINE_1801VP128_H
#define MAME_MACHINE_1801VP128_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/pdp11.h"

#include "fdc_pll.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k1801vp128_device

class k1801vp128_device : public device_t
{
public:
	// construction/destruction
	k1801vp128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ds_in_callback() { return m_read_ds.bind(); }

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_floppy);

private:
	enum {
		CMD_READ,
		CMD_WRITE,
		CMD_SEEK
	};

	enum {
		IDLE,

		// Main states
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

		HEAD_LOAD,
		HEAD_LOAD_DONE,

		WAIT_INDEX,
		WAIT_INDEX_DONE,

		SCAN_ID,
		SCAN_ID_FAILED,

		TRACK_READ,
		TRACK_WRITTEN,

		// Live states
		SEARCH_ADDRESS_MARK_HEADER,
		READ_DATA_HIGH,
		READ_DATA_HIGH_BYTE,
		READ_DATA_LOW,
		READ_DATA_LOW_BYTE,

		WRITE_MFM_DATA_HIGH,
		WRITE_MFM_DATA_HIGH_BYTE,
		WRITE_MFM_DATA_LOW,
		WRITE_MFM_DATA_LOW_BYTE,
	};

	enum {
		CSR_R_TR0 = 0000001,
		CSR_R_RDY = 0000002,
		CSR_R_WPR = 0000004,
		CSR_R_TR  = CSR_DONE,
		CSR_R_CRC = 0040000,
		CSR_R_IND = 0100000,

		CSR_W_DS0 = 0000001,
		CSR_W_DS1 = 0000002,
		CSR_W_DS2 = 0000004,
		CSR_W_DS3 = 0000010,
		CSR_W_MSW = 0000020, // motor
		CSR_W_HS  = 0000040, // head select
		CSR_W_DIR = 0000100, // step direction
		CSR_W_ST  = 0000200, // step pulse
		CSR_W_GDR = 0000400,
		CSR_W_WM  = 0001000,
		CSR_W_REZ = 0002000,
		CSR_W_DS  = 0000017,
	};

	struct floppy_info
	{
		floppy_info();

		emu_timer *tm;
		floppy_image_device *dev;
		int id;
		int main_state, sub_state;
		int dir, counter;
		bool live, index;
	};

	struct live_info
	{
		live_info();

		attotime tm;
		int state, next_state;
		floppy_info *fi;
		uint16_t shift_reg, crc;
		int bit_counter;
		bool data_separator_phase, data_bit_context, crc_init;
		uint16_t data_reg;
		fdc_pll_t pll;
	};

	required_device_array<floppy_connector, 4> m_connectors;
	devcb_read16 m_read_ds;

	std::string ttsn() const;

	floppy_info flopi[4];
	int selected_drive;

	uint16_t m_cr;
	uint16_t m_sr;
	uint16_t m_rbuf;
	uint16_t m_wbuf;

	live_info cur_live, checkpoint_live;

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
	void live_write_mfm(uint8_t mfm, bool marker);

	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);
};


// device type definition
DECLARE_DEVICE_TYPE(K1801VP128, k1801vp128_device)

#endif // MAME_MACHINE_1801VP128_H
