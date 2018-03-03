// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_RAM_H
#define MAME_BUS_MSX_SLOT_RAM_H

#include "slot.h"

#define MCFG_MSX_SLOT_RAM_ADD(_tag, _startpage, _numpages) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_RAM, _startpage, _numpages)

#define MCFG_MSX_SLOT_RAM_8KB \
	downcast<msx_slot_ram_device &>(*device).force_start_address(0xe000);


class msx_slot_ram_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void force_start_address(uint16_t start) { m_start_address = start; }

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_start() override;

private:
	std::vector<uint8_t> m_ram;
};


DECLARE_DEVICE_TYPE(MSX_SLOT_RAM, msx_slot_ram_device)


#endif // MAME_BUS_MSX_SLOT_RAM_H
