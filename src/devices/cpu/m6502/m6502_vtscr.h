// license:BSD-3-Clause
// copyright-holders:David Shah
/***************************************************************************

    m6502_vtscr.h

    6502 with VRT VTxx instruction scrambling
    
***************************************************************************/

#ifndef MAME_CPU_M6502_VTSCR_H
#define MAME_CPU_M6502_VTSCR_H

#pragma once

#include "m6502.h"

class m6502_vtscr : public m6502_device {
public:
	m6502_vtscr(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_next_scramble(bool scr);
	void set_scramble(bool scr);

protected:
	m6502_vtscr(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void prefetch() override;
	virtual void prefetch_noirq() override;
	virtual void device_reset() override;
private:
	virtual uint8_t descramble(uint8_t op);
	bool toggle_scramble(uint8_t op);
	bool m_scramble_en = false;
	bool m_next_scramble = false;
	
};

DECLARE_DEVICE_TYPE(M6502_VTSCR, m6502_vtscr)

#endif // MAME_CPU_M6502_VTSCR_H
