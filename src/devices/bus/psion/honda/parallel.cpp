// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Parallel Printer Link cable

    TODO:
    - dump PIC internal ROM and hook up.

**********************************************************************/

#include "emu.h"
#include "parallel.h"

#include "bus/centronics/ctronics.h"
#include "cpu/pic16c62x/pic16c62x.h"


namespace {

class psion_parallel_device : public device_t, public device_psion_honda_interface
{
public:
	psion_parallel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, PSION_PARALLEL, tag, owner, clock)
		, device_psion_honda_interface(mconfig, *this)
	{
	}

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

protected:
	virtual void device_start() override ATTR_COLD { }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		PIC16C620(config, "mcu", 9.8304_MHz_XTAL).set_disable();

		CENTRONICS(config, "centronics", centronics_devices, "printer");
	}

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


//-------------------------------------------------
//  ROM( psion_parallel )
//-------------------------------------------------

ROM_START(psion_parallel)
	ROM_REGION(0x0400, "mcu", ROMREGION_ERASEFF)
	ROM_LOAD("pic16c620.bin", 0x0000, 0x0400, NO_DUMP)
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *psion_parallel_device::device_rom_region() const
{
	return ROM_NAME(psion_parallel);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PSION_PARALLEL, device_psion_honda_interface, psion_parallel_device, "psion_parallel", "Psion Parallel Printer Link cable")
