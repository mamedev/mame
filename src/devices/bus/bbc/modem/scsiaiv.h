// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AIV SCSI Host Adaptor

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_SCSIAIV_H
#define MAME_BUS_BBC_1MHZBUS_SCSIAIV_H

#include "modem.h"
#include "machine/nscsi_cb.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_scsiaiv_device:
	public device_t,
	public device_bbc_modem_interface
{
public:
	// construction/destruction
	bbc_scsiaiv_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(bsy_w);
	DECLARE_WRITE_LINE_MEMBER(req_w);

protected:
	bbc_scsiaiv_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config& config) override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	required_device<nscsi_callback_device> m_scsi;

	int m_irq_enable;
	int m_irq_state;
};


// ======================> bbc_vp415_device

//class bbc_vp415_device : public bbc_scsiaiv_device
//{
//public:
//	// construction/destruction
//	bbc_vp415_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
//
//protected:
//	// optional information overrides
//	virtual void device_add_mconfig(machine_config& config) override;
//};


// device type definition
DECLARE_DEVICE_TYPE(BBC_SCSIAIV, bbc_scsiaiv_device);
//DECLARE_DEVICE_TYPE(BBC_VP415, bbc_vp415_device);


#endif /* MAME_BUS_BBC_1MHZBUS_SCSIAIV_H */
