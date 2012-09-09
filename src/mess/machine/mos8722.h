/**********************************************************************

    MOS Technology 8722 Memory Management Unit emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   Vdd   1 |*    \_/     | 48  SENSE40
                _RESET   2 |             | 47  (MS3) 128/64
                  TA15   3 |             | 46  _EXROM
                  TA14   4 |             | 45  _GAME
                  TA13   5 |             | 44  FSDIR
                  TA12   6 |             | 43  _Z80EN
                  TA11   7 |             | 42  D7
                  TA10   8 |             | 41  D6
                   TA9   9 |             | 40  D5
                   TA8  10 |             | 39  D4
                 _CAS1  11 |             | 38  D3
                 _CAS0  12 |   MOS8722   | 37  D2
         I/O SEL (MS2)  13 |             | 36  D1
        ROMBANK1 (MS1)  14 |             | 35  D0
        ROMBANK0 (MS0)  15 |             | 34  Vss
                   AEC  16 |             | 33  phi0
                   MUX  17 |             | 32  R/_W
                    A0  18 |             | 31  A15
                    A1  19 |             | 30  A14
                    A2  20 |             | 29  A13
                    A3  21 |             | 28  A12
                 A4/A5  22 |             | 27  A11
                 A6/A7  23 |             | 26  A10
                    A8  24 |_____________| 25  A9

**********************************************************************/

#pragma once

#ifndef __MOS8722__
#define __MOS8722__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS8722_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, MOS8722, 0)	\
	MCFG_DEVICE_CONFIG(_config)


#define MOS8722_INTERFACE(name) \
	const mos8722_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos8722_interface

struct mos8722_interface
{
	devcb_write_line	m_out_z80en_cb;
	devcb_write_line	m_out_fsdir_cb;
	devcb_read_line		m_in_game_cb;
	devcb_read_line		m_in_exrom_cb;
	devcb_read_line		m_in_sense40_cb;
};


// ======================> mos8722_device

class mos8722_device :	public device_t,
                        public mos8722_interface
{
public:
    // construction/destruction
    mos8722_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

    DECLARE_READ_LINE_MEMBER( fsdir_r );
    DECLARE_READ_LINE_MEMBER( ms0_r );
    DECLARE_READ_LINE_MEMBER( ms1_r );
    DECLARE_READ_LINE_MEMBER( ms2_r );
    DECLARE_READ_LINE_MEMBER( ms3_r );

    offs_t ta_r(offs_t offset, int aec);

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();

private:
	devcb_resolved_write_line	m_out_z80en_func;
	devcb_resolved_write_line	m_out_fsdir_func;
	devcb_resolved_read_line	m_in_game_func;
	devcb_resolved_read_line	m_in_exrom_func;
	devcb_resolved_read_line	m_in_sense40_func;

	UINT8 m_reg[10];
};


// device type definition
extern const device_type MOS8722;



#endif
