// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Power Software Joystick Interface

**********************************************************************/

#include "emu.h"
#include "pwrjoy.h"

#include "bus/vcs_ctrl/ctrl.h"


namespace {

class electron_pwrjoy_device
	: public device_t
	, public device_electron_expansion_interface
{
public:
	electron_pwrjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ELECTRON_PWRJOY, tag, owner, clock)
		, device_electron_expansion_interface(mconfig, *this)
		, m_exp_rom(*this, "exp_rom")
		, m_joy(*this, "joy")
		, m_romsel(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_exp_rom;
	required_device<vcs_control_port_device> m_joy;

	uint8_t m_romsel;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_pwrjoy_device::device_add_mconfig(machine_config &config)
{
	VCS_CONTROL_PORT(config, m_joy, vcs_control_port_devices, "joy");
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( pwrjoy )
	// Bank 12 Expansion module operating system
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("power_joystick.rom", 0x0000, 0x2000, CRC(44fb9360) SHA1(6d3aa85a436db952906e84839496e681ea115168))
ROM_END

const tiny_rom_entry *electron_pwrjoy_device::device_rom_region() const
{
	return ROM_NAME( pwrjoy );
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_pwrjoy_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x8000 && offset < 0xc000)
	{
		if (m_romsel == 15)
		{
			data = m_exp_rom->base()[offset & 0x1fff];
		}
	}
	else if (offset == 0xfcc0)
	{
		data = bitswap<8>(m_joy->read_joy(), 7, 6, 4, 5, 3, 2, 1, 0);
	}

	return data;
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_pwrjoy_device::expbus_w(offs_t offset, uint8_t data)
{
	if ((offset == 0xfe05) && !(data & 0xf0))
	{
		m_romsel = data & 0x0f;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ELECTRON_PWRJOY, device_electron_expansion_interface, electron_pwrjoy_device, "electron_pwrjoy", "Power Software Joystick Interface")
