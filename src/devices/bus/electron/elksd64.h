// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ElkSD64 Electron SD Interface

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_ELKSD64_H
#define MAME_BUS_ELECTRON_ELKSD64_H

#include "exp.h"
#include "machine/spi_sdcard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_elksd64_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_elksd64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_flash;
	required_device<spi_sdcard_device> m_sdcard;

	uint8_t m_romsel;
	uint8_t m_status;

	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_ELKSD64, electron_elksd64_device)


#endif /* MAME_BUS_ELECTRON_ELKSD64_H */
