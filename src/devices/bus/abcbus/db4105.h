// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    DataBoard 4105 SASI hard disk controller emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_DB4105_H
#define MAME_BUS_ABCBUS_DB4105_H

#pragma once


#include "abcbus.h"
#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> databoard_4105_device

class databoard_4105_device :  public device_t,
						   	   public nscsi_device_interface,
						       public device_abcbus_card_interface
{
public:
	// construction/destruction
	databoard_4105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual int abcbus_csb() override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual void abcbus_c4(uint8_t data) override;
	virtual uint8_t abcbus_tren() override;
	virtual void abcbus_tren(uint8_t data) override;
	virtual void abcbus_prac(int state) override;

	// nscsi_device_interface overrides
	virtual void scsi_ctrl_changed() override;

private:
	void internal_reset();
	void update_ack();
	void update_dma();
	void write_dma_register(uint8_t data);
	void write_sasi_data(uint8_t data);

	required_ioport m_1e;
	required_ioport m_5e;

	bool m_cs;
	uint8_t m_data_out;
	uint8_t m_dma;
	bool m_req;
	bool m_drq;
	bool m_pren;
	bool m_prac;
	bool m_trrq;
};


// device type definition
DECLARE_DEVICE_TYPE(DATABOARD_4105, databoard_4105_device)


#endif // MAME_BUS_ABCBUS_DB4105_H
