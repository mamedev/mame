// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    First Byte Switched Joystick Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/FirstByte_JoystickIF.html

**********************************************************************/

#include "emu.h"
#include "fbjoy.h"

#include "bus/vcs_ctrl/ctrl.h"


namespace {

class electron_fbjoy_device
	: public device_t
	, public device_electron_expansion_interface
{
public:
	electron_fbjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ELECTRON_FBJOY, tag, owner, clock)
		, device_electron_expansion_interface(mconfig, *this)
		, m_joy(*this, "joy")
	{
	}

	virtual uint8_t expbus_r(offs_t offset) override;

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<vcs_control_port_device> m_joy;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_fbjoy_device::device_add_mconfig(machine_config &config)
{
	VCS_CONTROL_PORT(config, m_joy, vcs_control_port_devices, "joy");
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_fbjoy_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset == 0xfcc0)
	{
		data = bitswap<8>(m_joy->read_joy(), 7, 6, 4, 5, 3, 2, 1, 0);
	}

	return data;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ELECTRON_FBJOY, device_electron_expansion_interface, electron_fbjoy_device, "electron_fbjoy", "First Byte Joystick Interface")
