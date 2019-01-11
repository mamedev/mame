// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m8502.h

    6510 derivative, capable of running at 2MHz.

***************************************************************************/
#ifndef MAME_CPU_M6502_M8502_H
#define MAME_CPU_M6502_M8502_H

#pragma once

#include "m6510.h"

#define MCFG_M8502_PORT_CALLBACKS(_read, _write) \
	downcast<m8502_device *>(device)->set_callbacks(DEVCB_##_read, DEVCB_##_write);

#define MCFG_M8502_PORT_PULLS(_up, _down) \
	downcast<m8502_device *>(device)->set_pulls(_up, _down);

class m8502_device : public m6510_device {
public:
	m8502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	M8502_IRQ_LINE = m6502_device::IRQ_LINE,
	M8502_NMI_LINE = m6502_device::NMI_LINE
};

DECLARE_DEVICE_TYPE(M8502, m8502_device)

#endif // MAME_CPU_M6502_M8502_H
