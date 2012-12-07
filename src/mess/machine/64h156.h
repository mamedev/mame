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
#include "imagedev/flopdrv.h"
#include "formats/d64_dsk.h"
#include "formats/g64_dsk.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_64H156_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, C64H156, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define C64H156_INTERFACE(_name) \
	const c64h156_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64h156_interface

struct c64h156_interface
{
	devcb_write_line	m_out_atn_cb;
	devcb_write_line	m_out_sync_cb;
	devcb_write_line	m_out_byte_cb;
};

// ======================> c64h156_device

class c64h156_device :  public device_t,
						public device_execute_interface,
						public c64h156_interface
{
public:
    // construction/destruction
    c64h156_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( yb_r );
	DECLARE_WRITE8_MEMBER( yb_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );
	DECLARE_WRITE_LINE_MEMBER( accl_w );
	DECLARE_READ_LINE_MEMBER( sync_r );
	DECLARE_READ_LINE_MEMBER( byte_r );
	DECLARE_WRITE_LINE_MEMBER( mtr_w );
	DECLARE_WRITE_LINE_MEMBER( oe_w );
	DECLARE_WRITE_LINE_MEMBER( soe_w );
	DECLARE_READ_LINE_MEMBER( atn_r );
	DECLARE_WRITE_LINE_MEMBER( atni_w );
	DECLARE_WRITE_LINE_MEMBER( atna_w );

	void stp_w(int data);
	void ds_w(int data);
	void set_side(int side);

	void on_disk_changed(int wp);

protected:
    // device-level overrides
    virtual void device_config_complete();
    virtual void device_start();

    // device_execute_interface overrides
	virtual void execute_run();

    int m_icount;

	inline void set_atn_line();
	inline void read_current_track();
	inline void update_cycles_until_next_bit();
	inline void receive_bit();
	inline void decode_bit();

private:
	devcb_resolved_write_line	m_out_atn_func;
	devcb_resolved_write_line	m_out_sync_func;
	devcb_resolved_write_line	m_out_byte_func;

	required_device<legacy_floppy_image_device> m_image;
	optional_shared_ptr<UINT8> m_track_buffer;					// track data buffer
	optional_shared_ptr<UINT8> m_speed_buffer;					// speed block buffer

	// track
	UINT16 m_shift;
	int m_side;								// disk side
	int m_track_len;						// track length
	offs_t m_buffer_pos;					// current byte position within track buffer
	int m_bit_pos;							// current bit position within track buffer byte
	int m_bit_count;						// current data byte bit counter
	int m_cycles_until_next_bit;
	int m_zero_count;
	int m_cycles_until_random_flux;

	// motors
	int m_mtr;								// spindle motor on

	// signals
	int m_accl;								// 1/2 MHz select
	int m_ds;								// density select
	int m_soe;								// s? output enable
	int m_oe;								// output enable (0 = write, 1 = read)

	// IEC
	int m_atni;								// attention input
	int m_atna;								// attention acknowledge

	// read logic
	int m_last_bit_sync;
	int m_bit_sync;
	int m_byte_sync;
	int m_block_sync;
	int m_ue7;
	int m_ue7_tc;
	int m_uf4;
	int m_uf4_qb;
	UINT8 m_ud2;
	int m_u4a;
	int m_u4b;
	int m_ue3;
	int m_uc1b;

	// write logic
	UINT8 m_via_pa;
	UINT8 m_ud3;
	int m_wp;
};



// device type definition
extern const device_type C64H156;



#endif
