// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Creative labs SB0400 Audigy2 Value

#ifndef MAME_SOUND_SB0400_H
#define MAME_SOUND_SB0400_H

#include "machine/pci.h"

#define MCFG_SB0400_ADD(_tag, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, SB0400, 0x11020008, 0x00, 0x040100, _subdevice_id)

class sb0400_device : public pci_device {
public:
	sb0400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);
};

DECLARE_DEVICE_TYPE(SB0400, sb0400_device)

#endif // MAME_SOUND_SB0400_H
