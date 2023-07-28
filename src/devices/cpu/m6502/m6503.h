// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6503.h

    MOS Technology 6502, NMOS variant with reduced address bus

***************************************************************************/

#ifndef MAME_CPU_M6502_M6503_H
#define MAME_CPU_M6502_M6503_H

#include "m6502.h"

class m6503_device : public m6502_device {
public:
	m6503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


enum {
	M6503_IRQ_LINE = m6502_device::IRQ_LINE
};

DECLARE_DEVICE_TYPE(M6503, m6503_device);

#endif // MAME_CPU_M6502_M6503_H
