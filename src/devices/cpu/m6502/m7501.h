// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m7501.h

    6510 derivative, essentially identical.  Also known as the 8501.

***************************************************************************/

#ifndef __M7501_H__
#define __M7501_H__

#include "m6510.h"

#define MCFG_M7501_PORT_CALLBACKS(_read, _write) \
	downcast<m7501_device *>(device)->set_callbacks(DEVCB_##_read, DEVCB_##_write);

#define MCFG_M7501_PORT_PULLS(_up, _down) \
	downcast<m7501_device *>(device)->set_pulls(_up, _down);

class m7501_device : public m6510_device {
public:
	m7501_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

enum {
	M7501_IRQ_LINE = m6502_device::IRQ_LINE,
	M7501_NMI_LINE = m6502_device::NMI_LINE
};

extern const device_type M7501;

#endif
