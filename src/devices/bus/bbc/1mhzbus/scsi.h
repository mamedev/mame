// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn SCSI Host Adaptor

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_SCSI_H
#define MAME_BUS_BBC_1MHZBUS_SCSI_H

#include "1mhzbus.h"
#include "machine/nscsi_cb.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_scsi_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_scsi_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	void bsy_w(int state);
	void req_w(int state);

protected:
	bbc_scsi_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config& config) override;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;

private:
	required_device<nscsi_callback_device> m_scsi;

	int m_irq_enable;
	int m_irq_state;
};


// ======================> bbc_awhd_device

class bbc_awhd_device : public bbc_scsi_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	// construction/destruction
	bbc_awhd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_SCSI, bbc_scsi_device);
DECLARE_DEVICE_TYPE(BBC_AWHD, bbc_awhd_device);


#endif /* MAME_BUS_BBC_1MHZBUS_SCSI_H */
