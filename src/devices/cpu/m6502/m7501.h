// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m7501.h

    6510 derivative, essentially identical.  Also known as the 8501.

***************************************************************************/

#ifndef MAME_CPU_M6502_M7501_H
#define MAME_CPU_M6502_M7501_H

#include "m6510.h"

class m7501_device : public m6510_device {
public:
	m7501_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	M7501_IRQ_LINE = m6502_device::IRQ_LINE,
	M7501_NMI_LINE = m6502_device::NMI_LINE
};

DECLARE_DEVICE_TYPE(M7501, m7501_device)

#endif // MAME_CPU_M6502_M7501_H
