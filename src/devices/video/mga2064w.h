// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_VIDEO_MGA2064W_H
#define MAME_VIDEO_MGA2064W_H

#pragma once

#include "machine/pci.h"

#define MCFG_MGA2064W_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, MGA2064W, 0x102b0519, 0x01, 0x030000, 0x00000000)

class mga2064w_device : public pci_device {
public:
	mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(MGA2064W, mga2064w_device);

#endif // MAME_VIDEO_MGA2064W_H
