// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Write Enable pull up resistor

    The H89 has a 1k floppy RAM which can be write protected. With the original
    equipment, the hard-sector controller card (H-88-1) could control the memory.
    In later systems, Heath/Zenith wanted to provide a system with only the soft-
    sectored controller (Z-89-37), but needed to allow writing to the floppy RAM.
    Heath provided a pullup resistor with the new controller, which allowed the
    memory to always be write enabled, this was installed on slot P506. Without
    this, HDOS is not bootable on Z-89-37 soft-sectored controller.

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

h89bus_we_pullup_device::h89bus_we_pullup_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
  : device_t(mconfig, H89BUS_WE_PULLUP, tag, owner, 0)
  , device_h89bus_right_card_interface(mconfig, *this)
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
