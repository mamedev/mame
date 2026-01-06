// license:BSD-3-Clause
// copyright-holders: D. Donohoe

#include "emu.h"

#include "bus/isa/fdc.h"
#include "bus/isa/isa.h"

DECLARE_DEVICE_TYPE(ISA8_FDC_6300P, isa8_fdc_6300p_device)

class isa8_fdc_6300p_device : public isa8_upd765_fdc_device
{
public:
	isa8_fdc_6300p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;
	void rc_map(address_map &map) ATTR_COLD;

	uint8_t rc_r();
	void rc_w(uint8_t data);

private:
	int32_t m_rate;
};
