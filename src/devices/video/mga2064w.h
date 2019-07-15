// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_VIDEO_MGA2064W_H
#define MAME_VIDEO_MGA2064W_H

#pragma once

#include "machine/pci.h"

class mga2064w_device : public pci_device {
public:
	mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(MGA2064W, mga2064w_device);

#endif // MAME_VIDEO_MGA2064W_H
