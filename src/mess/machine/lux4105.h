/**********************************************************************

    Luxor 4105 SASI hard disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __LUXOR_4105__
#define __LUXOR_4105__


#include "emu.h"
#include "abc1600_bus.h"
#include "machine/scsicb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LUXOR_4105_TAG		"luxor_4105"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_4105_device

class luxor_4105_device :  public device_t,
						   public device_abc1600bus_card_interface
{
public:
    // construction/destruction
    luxor_4105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( sasi_bsy_w );
	DECLARE_WRITE_LINE_MEMBER( sasi_io_w );
	DECLARE_WRITE_LINE_MEMBER( sasi_req_w );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "luxor_4105"; }

	// device_abc1600bus_interface overrides
	virtual void abc1600bus_cs(UINT8 data);
	virtual int abc1600bus_csb();
	virtual void abc1600bus_brst();
	virtual UINT8 abc1600bus_inp();
	virtual void abc1600bus_out(UINT8 data);
	virtual UINT8 abc1600bus_stat();
	virtual void abc1600bus_c1(UINT8 data);
	virtual void abc1600bus_c3(UINT8 data);
	virtual void abc1600bus_c4(UINT8 data);

private:
	inline void update_trrq_int();

	abc1600bus_slot_device *m_slot;

	required_device<scsicb_device> m_sasibus;

	int m_cs;
	UINT8 m_data;
	UINT8 m_dma;
};


// device type definition
extern const device_type LUXOR_4105;



#endif
