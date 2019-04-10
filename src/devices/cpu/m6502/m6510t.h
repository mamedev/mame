// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510t.h

    6510 with the full 8 i/o pins at the expense of the NMI and RDY lines.

***************************************************************************/

#ifndef MAME_CPU_M6502_M6510T_H
#define MAME_CPU_M6502_M6510T_H

#include "m6510.h"

class m6510t_device : public m6510_device {
public:
	m6510t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	M6510T_IRQ_LINE = m6502_device::IRQ_LINE,
	M6510T_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(M6510T, m6510t_device)

#endif // MAME_CPU_M6502_M6510T_H
