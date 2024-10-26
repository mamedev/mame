// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Write enable pull

    On

****************************************************************************/

#include "emu.h"

#include "we_pullup.h"


namespace {

class h89bus_we_pullup_device : public device_t, public device_h89bus_right_card_interface
{
public:
	h89bus_we_pullup_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

h89bus_we_pullup_device::h89bus_we_pullup_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_WE_PULLUP, tag, owner, 0),
	device_h89bus_right_card_interface(mconfig, *this)
{
}

void h89bus_we_pullup_device::device_start()
{
}

void h89bus_we_pullup_device::device_reset()
{
	set_slot_fmwe(ASSERT_LINE);
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_WE_PULLUP, device_h89bus_right_card_interface, h89bus_we_pullup_device, "h89_we_pullup", "Pullup resistor needed when P506 slot is empty");
