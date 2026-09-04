// license:BSD-3-Clause
// copyright-holders:

/******************************************************************************

    Skeleton device for Analog Devices ADSP-21xx EZ-KIT Lite.
    https://www.analog.com/en/products/21xx-ezlite.html

    Hardware specifications for ADSP-2181 EZ-KIT Lite:
    - Analog Devices ADSP-2181 near a 16.670 MHz xtal.
    - Analog Devices AD1847JP SoundPort near two xtals, 24.576 and 16.9344 MHz.
    - Jacks connectors for audio in and audio out.
    - Connectors for line in and microphone.
	- RS-232 (female DB9).
    - Connector for EZ-ICE.
    - Buttons for reset and interrupt.

    TODO:
    - Everything.

    PC software and PCB images can be downloaded from:
    - https://github.com/ArcadeHacker/Dumps/tree/main/Accessories/Emulators-Debuggers/Analog%20Devices%20ADSP-2181%20EZ-KIT%20Lite

******************************************************************************/

#include "emu.h"
#include "adsp2181ekl.h"

#include "cpu/adsp2100/adsp2100.h"

namespace {

class adsp2181ekl_device : public device_t, public device_rs232_port_interface
{
public:
	adsp2181ekl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ADSP2181EKL, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_dsp(*this, "maincpu")
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<adsp2181_device> m_dsp;

	[[maybe_unused]] void adsp2181ekl_map(address_map &map) ATTR_COLD;
};

void adsp2181ekl_device::device_start()
{
}

void adsp2181ekl_device::device_reset()
{
}

ROM_START( adsp2181ekl )
	ROM_REGION(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "2181ekl_r1", "21xx EZ Kit Lite Rev. 1.0")
	ROMX_LOAD("21xx_ez-kit_lite_rev_1.0_at27c010.u2", 0x00000, 0x20000, CRC(7b910bbf) SHA1(20bf850e1b28447548bb11e4a312521e842e59d5), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *adsp2181ekl_device::device_rom_region() const
{
	return ROM_NAME( adsp2181ekl );
}

void adsp2181ekl_device::adsp2181ekl_map(address_map &map)
{
}

void adsp2181ekl_device::device_add_mconfig(machine_config &config)
{
	ADSP2181(config, m_dsp, 16.670_MHz_XTAL);

	// Audio hardware
	// AD1847JP SoundPort...
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ADSP2181EKL, device_rs232_port_interface, adsp2181ekl_device, "adsp2181ekl", "Analog Devices ADSP-21xx EZ-KIT")
