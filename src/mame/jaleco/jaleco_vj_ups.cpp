// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
 *  Densei UPS for VJ
 */

#include "emu.h"
#include "jaleco_vj_ups.h"


namespace {

class jaleco_vj_ups_device : public device_t, public device_rs232_port_interface
{
public:
	jaleco_vj_ups_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


jaleco_vj_ups_device::jaleco_vj_ups_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JALECO_VJ_UPS, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
{
}

void jaleco_vj_ups_device::device_start()
{
}

void jaleco_vj_ups_device::device_reset()
{
	// TODO: provide a way for the user to change the status?
	output_cts(1); // line power down
	output_dsr(1); // line shutdown
	output_dcd(0); // line low battery
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(JALECO_VJ_UPS, device_rs232_port_interface, jaleco_vj_ups_device, "rs232_densei_ups", "RS232 Densei UPS")
