// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Torch SCSI Host Adaptor

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_SASI_H
#define MAME_BUS_BBC_1MHZBUS_SASI_H

#include "1mhzbus.h"
#include "machine/nscsi_cb.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_sasi_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void req_w(int state);
	void sel_w(int state);

protected:
	bbc_sasi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config& config) override;

	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<nscsi_callback_device> m_sasi;

	int m_sel_state;
};


// ======================> bbc_torchhd_device

class bbc_torchhd_device : public bbc_sasi_device
{
public:
	// construction/destruction
	bbc_torchhd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_SASI, bbc_sasi_device);
DECLARE_DEVICE_TYPE(BBC_TORCHHD, bbc_torchhd_device);


#endif /* MAME_BUS_BBC_1MHZBUS_SASI_H */
