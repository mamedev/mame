// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65sc02.h

    Rockwell-class 65c02 with internal static registers, making clock stoppable?

***************************************************************************/

#ifndef MAME_CPU_M6502_M65SC02_H
#define MAME_CPU_M6502_M65SC02_H

#include "r65c02.h"

class m65sc02_device : public r65c02_device {
public:
	m65sc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	M65SC02_IRQ_LINE = m6502_device::IRQ_LINE,
	M65SC02_NMI_LINE = m6502_device::NMI_LINE,
	M65SC02_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(M65SC02, m65sc02_device)

#endif // MAME_CPU_M6502_M65SC02_H
