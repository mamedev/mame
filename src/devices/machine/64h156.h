// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64H156 Gate Array emulation

    Used in 1541B/1541C/1541-II/1551/1571

**********************************************************************
                            _____   _____
                  TEST   1 |*    \_/     | 40  _BYTE
                   YB0   2 |             | 39  SOE
                   YB1   3 |             | 38  B
                   YB2   4 |             | 37  CK
                   YB3   5 |             | 36  _QX
                   YB4   6 |             | 35  Q
                   YB5   7 |             | 34  R/_W
                   YB6   8 |             | 33  LOCK
                   YB7   9 |             | 32  PLL
                   Vss  10 |  64H156-01  | 31  CLR
                  STP1  11 |  251828-01  | 30  Vcc
                  STP0  12 |             | 29  _XRW
                   MTR  13 |             | 28  Y3
                    _A  14 |             | 27  Y2
                   DS0  15 |             | 26  Y1
                   DS1  16 |             | 25  Y0
                 _SYNC  17 |             | 24  ATN
                   TED  18 |             | 23  ATNI
                    OE  19 |             | 22  ATNA
                 _ACCL  20 |_____________| 21  OSC

                            _____   _____
                  TEST   1 |*    \_/     | 42  _BYTE
                   YB0   2 |             | 41  SOE
                   YB1   3 |             | 40  B
                   YB2   4 |             | 39  CK
                   YB3   5 |             | 38  _QX
                   YB4   6 |             | 37  Q
                   YB5   7 |             | 36  R/_W
                   YB6   8 |             | 35  LOCK
                   YB7   9 |             | 34  PLL
                   Vss  10 |  64H156-02  | 33  CLR
                  STP1  11 |  251828-02  | 32  Vcc
                  STP0  12 |             | 31  _XRW
                   MTR  13 |             | 30  Y3
                    _A  14 |             | 29  Y2
                   DS0  15 |             | 28  Y1
                   DS1  16 |             | 27  Y0
                 _SYNC  17 |             | 26  ATN
                   TED  18 |             | 25  ATNI
                    OE  19 |             | 24  ATNA
                 _ACCL  20 |             | 23  OSC
                   Vcc  21 |_____________| 22  Vss

**********************************************************************/

#ifndef MAME_MACHINE_64H156_H
#define MAME_MACHINE_64H156_H

#pragma once

#include "imagedev/floppy.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64h156_device

class c64h156_device :  public device_t
{
public:
	// construction/destruction
	c64h156_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto atn_callback() { return m_write_atn.bind(); }
	auto sync_callback() { return m_write_sync.bind(); }
	auto byte_callback() { return m_write_byte.bind(); }

	uint8_t yb_r();
	void yb_w(uint8_t data);

	void test_w(int state);
	void accl_w(int state);
	void ted_w(int state);
	void mtr_w(int state);
	void oe_w(int state);
	void soe_w(int state);
	void atni_w(int state);
	void atna_w(int state);

	int sync_r() { return checkpoint_live.sync; }
	int byte_r() { return checkpoint_live.byte; }
	int atn_r() { return m_atni ^ m_atna; }

	void stp_w(int stp);
	void ds_w(int ds);

	void set_floppy(floppy_image_device *floppy);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_tick);

private:
	enum {
		IDLE,
		RUNNING,
		RUNNING_SYNCPOINT
	};

	struct live_info {
		attotime tm;
		int state = 0, next_state = 0;
		int sync = 0;
		int byte = 0;
		int ds = 0;
		int oe = 0;
		int soe = 0;
		int accl = 0;
		uint8_t accl_yb = 0;

		attotime edge;
		uint16_t shift_reg = 0;
		int cycle_counter = 0;
		int cell_counter = 0;
		int bit_counter = 0;
		int zero_counter = 0;
		int cycles_until_random_flux = 0;

		uint8_t yb = 0;
		uint8_t shift_reg_write = 0;
		attotime write_start_time;
		attotime write_buffer[32];
		int write_position = 0;
	};

	devcb_write_line m_write_atn;
	devcb_write_line m_write_sync;
	devcb_write_line m_write_byte;

	floppy_image_device *m_floppy;

	int m_mtr;
	int m_accl;
	int m_stp;
	int m_ds;
	int m_soe;
	int m_oe;
	int m_ted;
	uint8_t m_yb;
	int m_atni;
	int m_atna;

	attotime m_period;

	live_info cur_live, checkpoint_live;
	emu_timer *t_gen;

	void live_start();
	void checkpoint();
	void rollback();
	bool write_next_bit(bool bit, const attotime &limit);
	void start_writing(const attotime &tm);
	void commit(const attotime &tm);
	void stop_writing(const attotime &tm);
	void live_delay(int state);
	void live_sync();
	void live_abort();
	void live_run(const attotime &limit = attotime::never);
	void get_next_edge(const attotime &when);
	int get_next_bit(attotime &tm, const attotime &limit);
};


// device type definition
DECLARE_DEVICE_TYPE(C64H156, c64h156_device)

#endif // MAME_MACHINE_64H156_H
