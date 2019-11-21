// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 4105 SASI hard disk controller emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_LUX4105_H
#define MAME_BUS_ABCBUS_LUX4105_H

#pragma once


#include "abcbus.h"
#include "bus/scsi/scsi.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_4105_device

class luxor_4105_device :  public device_t,
							public device_abcbus_card_interface
{
public:
	// construction/destruction
	luxor_4105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual int abcbus_csb() override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual void abcbus_c4(uint8_t data) override;

private:
	inline void update_trrq_int();

	DECLARE_WRITE_LINE_MEMBER( write_sasi_bsy );
	DECLARE_WRITE_LINE_MEMBER( write_sasi_req );
	DECLARE_WRITE_LINE_MEMBER( write_sasi_cd );
	DECLARE_WRITE_LINE_MEMBER( write_sasi_io );

	required_device<scsi_port_device> m_sasibus;
	required_device<output_latch_device> m_sasi_data_out;
	required_device<input_buffer_device> m_sasi_data_in;
	required_ioport m_1e;
	required_ioport m_5e;

	bool m_cs;
	uint8_t m_data;
	uint8_t m_dma;

	int m_sasi_bsy;
	bool m_sasi_req;
	bool m_sasi_cd;
	bool m_sasi_io;
};


// device type definition
DECLARE_DEVICE_TYPE(LUXOR_4105, luxor_4105_device)


#endif // MAME_BUS_ABCBUS_LUX4105_H
