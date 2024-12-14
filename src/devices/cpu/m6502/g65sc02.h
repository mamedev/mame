// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    g65sc02.h

    Rockwell-class 65c02 with internal static registers, making clock stoppable?

***************************************************************************/

#ifndef MAME_CPU_M6502_G65SC02_H
#define MAME_CPU_M6502_G65SC02_H

#include "r65c02.h"

class g65sc02_device : public r65c02_device {
public:
	g65sc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class g65sc12_device : public r65c02_device {
public:
	g65sc12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class g65sc102_device : public r65c02_device {
public:
	g65sc102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class g65sc112_device : public r65c02_device {
public:
	g65sc112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	G65SC02_IRQ_LINE = m6502_device::IRQ_LINE,
	G65SC02_NMI_LINE = m6502_device::NMI_LINE,
	G65SC02_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(G65SC02, g65sc02_device)
DECLARE_DEVICE_TYPE(G65SC12, g65sc12_device)
DECLARE_DEVICE_TYPE(G65SC102, g65sc102_device)
DECLARE_DEVICE_TYPE(G65SC112, g65sc112_device)

#endif // MAME_CPU_M6502_G65SC02_H
