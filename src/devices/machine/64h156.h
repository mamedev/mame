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

#pragma once

#ifndef __C64H156__
#define __C64H156__

#include "emu.h"
#include "imagedev/floppy.h"
#include "formats/d64_dsk.h"
#include "formats/g64_dsk.h"
#include "formats/d71_dsk.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_64H156_ATN_CALLBACK(_write) \
	devcb = &c64h156_device::set_atn_wr_callback(*device, DEVCB_##_write);

#define MCFG_64H156_SYNC_CALLBACK(_write) \
	devcb = &c64h156_device::set_sync_wr_callback(*device, DEVCB_##_write);

#define MCFG_64H156_BYTE_CALLBACK(_write) \
	devcb = &c64h156_device::set_byte_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64h156_device

class c64h156_device :  public device_t
{
public:
	// construction/destruction
	c64h156_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_atn_wr_callback(device_t &device, _Object object) { return downcast<c64h156_device &>(device).m_write_atn.set_callback(object); }
	template<class _Object> static devcb_base &set_sync_wr_callback(device_t &device, _Object object) { return downcast<c64h156_device &>(device).m_write_sync.set_callback(object); }
	template<class _Object> static devcb_base &set_byte_wr_callback(device_t &device, _Object object) { return downcast<c64h156_device &>(device).m_write_byte.set_callback(object); }

	DECLARE_READ8_MEMBER( yb_r );
	DECLARE_WRITE8_MEMBER( yb_w );

	DECLARE_WRITE_LINE_MEMBER( test_w );
	DECLARE_WRITE_LINE_MEMBER( accl_w );
	DECLARE_WRITE_LINE_MEMBER( ted_w );
	DECLARE_WRITE_LINE_MEMBER( mtr_w );
	DECLARE_WRITE_LINE_MEMBER( oe_w );
	DECLARE_WRITE_LINE_MEMBER( soe_w );
	DECLARE_WRITE_LINE_MEMBER( atni_w );
	DECLARE_WRITE_LINE_MEMBER( atna_w );

	DECLARE_READ_LINE_MEMBER( sync_r ) { return checkpoint_live.sync; }
	DECLARE_READ_LINE_MEMBER( byte_r ) { return checkpoint_live.byte; }
	DECLARE_READ_LINE_MEMBER( atn_r ) { return m_atni ^ m_atna; }

	void stp_w(int stp);
	void ds_w(int ds);

	void set_floppy(floppy_image_device *floppy);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum {
		IDLE,
		RUNNING,
		RUNNING_SYNCPOINT
	};

	struct live_info {
		attotime tm;
		int state, next_state;
		int sync;
		int byte;
		int ds;
		int oe;
		int soe;
		int accl;
		UINT8 accl_yb;

		attotime edge;
		UINT16 shift_reg;
		int cycle_counter;
		int cell_counter;
		int bit_counter;
		int zero_counter;
		int cycles_until_random_flux;

		UINT8 yb;
		UINT8 shift_reg_write;
		attotime write_start_time;
		attotime write_buffer[32];
		int write_position;
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
	UINT8 m_yb;
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
extern const device_type C64H156;



#endif
