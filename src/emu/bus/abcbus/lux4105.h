// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 4105 SASI hard disk controller emulation

*********************************************************************/

#pragma once

#ifndef __LUXOR_4105__
#define __LUXOR_4105__


#include "emu.h"
#include "abcbus.h"
#include "bus/scsi/scsi.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LUXOR_4105_TAG      "luxor_4105"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_4105_device

class luxor_4105_device :  public device_t,
							public device_abcbus_card_interface
{
public:
	// construction/destruction
	luxor_4105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( write_sasi_bsy );
	DECLARE_WRITE_LINE_MEMBER( write_sasi_req );
	DECLARE_WRITE_LINE_MEMBER( write_sasi_cd );
	DECLARE_WRITE_LINE_MEMBER( write_sasi_io );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data);
	virtual int abcbus_csb();
	virtual UINT8 abcbus_inp();
	virtual void abcbus_out(UINT8 data);
	virtual UINT8 abcbus_stat();
	virtual void abcbus_c1(UINT8 data);
	virtual void abcbus_c3(UINT8 data);
	virtual void abcbus_c4(UINT8 data);

private:
	inline void update_trrq_int();

	required_device<SCSI_PORT_DEVICE> m_sasibus;
	required_device<output_latch_device> m_sasi_data_out;
	required_device<input_buffer_device> m_sasi_data_in;
	required_ioport m_1e;
	required_ioport m_5e;

	bool m_cs;
	UINT8 m_data;
	UINT8 m_dma;

	int m_sasi_bsy;
	int m_sasi_req;
	int m_sasi_cd;
	int m_sasi_io;
};


// device type definition
extern const device_type LUXOR_4105;



#endif
