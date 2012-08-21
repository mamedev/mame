/**********************************************************************

    COMX-35 Expansion Slot emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                     NC       2      B       DS
                    +5V       3      C       V+
                     D0       4      D       D1
                     D2       5      E       D3
                     D4       6      F       D5
                     D6       7      H       D7
                    _DP       8      J       Q
                 _CLEAR       9      K       _MRD
                    TPA      10      L       N0
                     N1      11      M       N2
                   _RAS      12      N       _INT
                  _WAIT      13      P       CLOCK
                    SC1      14      R       SC0
                   _EF4      15      S       _CASE
                    TPB      16      T       _A15
                   _MWR      17      U       A14
                    MA7      18      V       _A14
                    MA5      19      W       MA6
                    MA4      20      X       MA3
                    MA2      21      Y       _EXTROM
                    MA1      22      Z       MA0

**********************************************************************/

#pragma once

#ifndef __COMX35_EXPANSION_SLOT__
#define __COMX35_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define COMX_EXPANSION_BUS_TAG		"comxexp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define COMX_EXPANSION_INTERFACE(_name) \
	const comx_expansion_slot_interface (_name) =


#define MCFG_COMX_EXPANSION_SLOT_ADD(_tag, _config, _slot_intf, _def_slot, _def_inp) \
    MCFG_DEVICE_ADD(_tag, COMX_EXPANSION_SLOT, 0) \
    MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_expansion_slot_interface

struct comx_expansion_slot_interface
{
    devcb_write_line	m_out_int_cb;
    devcb_write_line	m_out_ef4_cb;
    devcb_write_line	m_out_wait_cb;
    devcb_write_line	m_out_clear_cb;
};


// ======================> comx_expansion_slot_device

class device_comx_expansion_card_interface;

class comx_expansion_slot_device : public device_t,
								   public comx_expansion_slot_interface,
								   public device_slot_interface
{
public:
	// construction/destruction
	comx_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~comx_expansion_slot_device();

	UINT8 mrd_r(offs_t offset, int *extrom);
	void mwr_w(offs_t offset, UINT8 data);

	UINT8 io_r(offs_t offset);
	void io_w(offs_t offset, UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( ds_w );
	DECLARE_WRITE_LINE_MEMBER( q_w );

	DECLARE_WRITE_LINE_MEMBER( int_w );
	DECLARE_WRITE_LINE_MEMBER( ef4_w );
	DECLARE_WRITE_LINE_MEMBER( wait_w );
	DECLARE_WRITE_LINE_MEMBER( clear_w );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	devcb_resolved_write_line	m_out_int_func;
	devcb_resolved_write_line	m_out_ef4_func;
	devcb_resolved_write_line	m_out_wait_func;
	devcb_resolved_write_line	m_out_clear_func;

	device_comx_expansion_card_interface *m_card;
};


// ======================> device_comx_expansion_card_interface

// class representing interface-specific live comx_expansion card
class device_comx_expansion_card_interface : public device_slot_card_interface
{
	friend class comx_expansion_slot_device;

public:
	// construction/destruction
	device_comx_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_comx_expansion_card_interface();

protected:
	// signals
	virtual void comx_ds_w(int state) { };
	virtual void comx_q_w(int state) { };

	// memory access
	virtual UINT8 comx_mrd_r(offs_t offset, int *extrom) { return 0; };
	virtual void comx_mwr_w(offs_t offset, UINT8 data) { };

	// I/O access
	virtual UINT8 comx_io_r(offs_t offset) { return 0; };
	virtual void comx_io_w(offs_t offset, UINT8 data) { };

	// video
	virtual UINT32 comx_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return false; }

	comx_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type COMX_EXPANSION_SLOT;



#endif
