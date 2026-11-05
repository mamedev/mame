// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis Floppy Disk Controller with SASI host emulation

**********************************************************************/

#ifndef MAME_BUS_ISBX_COMPIS_SASI_H
#define MAME_BUS_ISBX_COMPIS_SASI_H

#pragma once

#include "compis_fdc.h"

#include "machine/nscsi_cb.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_fdc_sasi_device

class compis_fdc_sasi_device : public compis_fdc_device
{
public:
	// construction/destruction
	compis_fdc_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_isbx_card_interface implementation
	virtual uint8_t mcs0_r(offs_t offset) override;
	virtual void mcs0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mcs1_r(offs_t offset) override;
	virtual void mcs1_w(offs_t offset, uint8_t data) override;

private:
	void req_w(int state);
	void io_w(int state);
	void bsy_w(int state);
	void drive_bus();

	required_device<nscsi_callback_device> m_sasi;
	required_ioport m_type;

	uint8_t m_data;
	uint8_t m_id;
};


// device type definition
DECLARE_DEVICE_TYPE(COMPIS_FDC_SASI, compis_fdc_sasi_device)


#endif // MAME_BUS_ISBX_COMPIS_SASI_H
