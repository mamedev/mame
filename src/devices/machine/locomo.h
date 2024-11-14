// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sharp LoCoMo peripheral chip emulation skeleton

***************************************************************************/

#ifndef MAME_MACHINE_LOCOMO
#define MAME_MACHINE_LOCOMO

#pragma once

class locomo_device : public device_t
{
public:
	locomo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0U);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0U);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint16_t m_kbd_cmd;
	uint16_t m_kbd_row;
	uint16_t m_kbd_col;
	uint16_t m_kbd_level;
};

DECLARE_DEVICE_TYPE(LOCOMO, locomo_device)

#endif // MAME_MACHINE_LOCOMO
