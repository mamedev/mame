// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Siena SSD Drive

**********************************************************************/

#ifndef MAME_BUS_PSION_HONDA_SSD_H
#define MAME_BUS_PSION_HONDA_SSD_H

#include "slot.h"
#include "machine/psion_ssd.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_siena_ssd_device

class psion_siena_ssd_device :
	public device_t,
	public device_psion_honda_interface
{
public:
	// construction/destruction
	psion_siena_ssd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t data_r() override { return m_ssd->data_r(); }
	virtual void data_w(uint16_t data) override { m_ssd->data_w(data); }

private:
	required_device<psion_ssd_device> m_ssd;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_SIENA_SSD, psion_siena_ssd_device)


#endif // MAME_BUS_PSION_HONDA_SSD_H
