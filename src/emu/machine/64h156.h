// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64H156 Gate Array emulation

    Used in 1541B/1541C/1541-II/1551/1571

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
	devcb = &c64h156_device::set_atn_wr_callback(*device, DEVCB2_##_write);

#define MCFG_64H156_SYNC_CALLBACK(_write) \
	devcb = &c64h156_device::set_sync_wr_callback(*device, DEVCB2_##_write);

#define MCFG_64H156_BYTE_CALLBACK(_write) \
	devcb = &c64h156_device::set_byte_wr_callback(*device, DEVCB2_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64h156_device

class c64h156_device :  public device_t,
						public device_execute_interface
{
public:
	// construction/destruction
	c64h156_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_atn_wr_callback(device_t &device, _Object object) { return downcast<c64h156_device &>(device).m_write_atn.set_callback(object); }
	template<class _Object> static devcb2_base &set_sync_wr_callback(device_t &device, _Object object) { return downcast<c64h156_device &>(device).m_write_sync.set_callback(object); }
	template<class _Object> static devcb2_base &set_byte_wr_callback(device_t &device, _Object object) { return downcast<c64h156_device &>(device).m_write_byte.set_callback(object); }

	DECLARE_READ8_MEMBER( yb_r );
	DECLARE_WRITE8_MEMBER( yb_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );
	DECLARE_WRITE_LINE_MEMBER( accl_w );
	DECLARE_READ_LINE_MEMBER( sync_r );
	DECLARE_READ_LINE_MEMBER( byte_r );
	DECLARE_WRITE_LINE_MEMBER( ted_w );
	DECLARE_WRITE_LINE_MEMBER( mtr_w );
	DECLARE_WRITE_LINE_MEMBER( oe_w );
	DECLARE_WRITE_LINE_MEMBER( soe_w );
	DECLARE_READ_LINE_MEMBER( atn_r );
	DECLARE_WRITE_LINE_MEMBER( atni_w );
	DECLARE_WRITE_LINE_MEMBER( atna_w );

	void set_floppy(floppy_image_device *floppy);

	void stp_w(int stp);
	void ds_w(int ds);

protected:
	// device-level overrides
	virtual void device_start();

	// device_execute_interface overrides
	virtual void execute_run();

	int m_icount;

	inline void set_atn_line();
	inline void receive_bit();
	inline void decode_bit();
	inline void get_next_edge(attotime when);

private:
	devcb2_write_line m_write_atn;
	devcb2_write_line m_write_sync;
	devcb2_write_line m_write_byte;

	floppy_image_device *m_floppy;

	// track
	attotime m_period;
	attotime m_edge;
	UINT16 m_shift;

	// motors
	int m_mtr;                              // spindle motor on

	// signals
	int m_accl;                             // 1/2 MHz select
	int m_stp;
	int m_ds;                               // density select
	int m_soe;                              // serial output enable
	int m_oe;                               // output enable (0 = write, 1 = read)

	// IEC
	int m_atni;                             // attention input
	int m_atna;                             // attention acknowledge

	// read logic
	int m_last_bit_sync;
	int m_bit_sync;
	int m_byte_sync;
	int m_accl_byte_sync;
	int m_block_sync;
	int m_ue7;
	int m_ue7_tc;
	int m_uf4;
	int m_uf4_qb;
	UINT8 m_ud2;
	UINT8 m_accl_yb;
	int m_u4a;
	int m_u4b;
	int m_ue3;
	int m_uc1b;
	int m_zero_count;
	int m_cycles_until_random_flux;

	// write logic
	UINT8 m_via_pa;
	UINT8 m_ud3;
};



// device type definition
extern const device_type C64H156;



#endif
