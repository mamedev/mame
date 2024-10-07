// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

Sony Magic Key emulation

It's a dongle that enables a cheat menu on some games published by Sony
when inserted into port B. It was a lottery prize from a Japanese MSX
magazine, not sold separately.

The dongle ties pin 1/2 (up/down) to pin 8 (strobe).

Note that this cheat menu can also be accessed with an FM Towns pad.

**********************************************************************/

#include "emu.h"
#include "magickey.h"


namespace {

class msx_magickey_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	msx_magickey_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override { return m_pin8_state ? 0xff : 0xfc; }
	virtual void pin_8_w(int state) override { m_pin8_state = state; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_pin8_state = 0;
};


msx_magickey_device::msx_magickey_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_MAGICKEY, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
{
}

void msx_magickey_device::device_start()
{
	save_item(NAME(m_pin8_state));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_MAGICKEY, device_msx_general_purpose_port_interface, msx_magickey_device, "msx_magickey", "Sony Magic Key Dongle")
