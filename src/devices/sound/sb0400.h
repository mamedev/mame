// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Creative labs SB0400 Audigy2 Value

#ifndef MAME_SOUND_SB0400_H
#define MAME_SOUND_SB0400_H

#include "machine/pci.h"

class sb0400_device : public pci_device {
public:
	sb0400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id)
		: sb0400_device(mconfig, tag, owner, clock)
	{
		set_ids(0x11020008, 0x00, 0x040100, subdevice_id);
	}
	sb0400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SB0400, sb0400_device)

#endif // MAME_SOUND_SB0400_H
