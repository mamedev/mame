// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion 3-Fax Modem

**********************************************************************/

#ifndef MAME_BUS_PSION_SIBO_3FAX_H
#define MAME_BUS_PSION_SIBO_3FAX_H

#include "slot.h"
#include "machine/psion_asic4.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_3fax_modem_device

class psion_3fax_modem_device :
	public device_t,
	public device_psion_sibo_interface
{
public:
	// construction/destruction
	psion_3fax_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t data_r() override { return m_asic4->data_r(); }
	virtual void data_w(uint16_t data) override { m_asic4->data_w(data); }

private:
	required_device<psion_asic4_device> m_asic4;

	void asic4_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_3FAX_MODEM, psion_3fax_modem_device)


#endif // MAME_BUS_PSION_SIBO_3FAX_H
