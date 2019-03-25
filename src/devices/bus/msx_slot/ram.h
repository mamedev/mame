// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_RAM_H
#define MAME_BUS_MSX_SLOT_RAM_H

#include "slot.h"

class msx_slot_ram_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Set to 0xe000 for 8KB RAM
	void force_start_address(uint16_t start) { m_start_address = start; }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;

private:
	std::vector<uint8_t> m_ram;
};


DECLARE_DEVICE_TYPE(MSX_SLOT_RAM, msx_slot_ram_device)


#endif // MAME_BUS_MSX_SLOT_RAM_H
