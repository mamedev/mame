// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M6502_M37640_H
#define MAME_CPU_M6502_M37640_H

#pragma once

#include "m740.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m37640_device

class m37640_device :  public m740_device
{
public:
	enum {
		INT1_LINE = INPUT_LINE_IRQ0,
		INT2_LINE,
	};

	m37640_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 mode = 2);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u32 m_mode;

	void map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(M37640, m37640_device)

#endif // MAME_CPU_M6502_M37640_H
