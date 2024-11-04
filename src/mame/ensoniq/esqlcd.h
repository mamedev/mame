// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
#ifndef MAME_ENSONIQ_ESQLCD_H
#define MAME_ENSONIQ_ESQLCD_H

#pragma once

#include "esqvfd.h"


// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------

class esq2x16_sq1_device : public esqvfd_device {
public:
	esq2x16_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(int data) override;
	virtual void update_display() override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_lcdpg[4][32]{};
	int m_lcdPage = 0;
	int m_lcdPos = 0, m_lcdSavedPos = 0;
	output_finder<4, 32*7> m_lcdPix;

	output_finder<16> m_leds;

private:
	void lcd_reset();
	void page_reset();

	static char rotate_lcd_char(uint8_t lcdChar, int charRow);

	int m_lcd_command = 0;
};

DECLARE_DEVICE_TYPE(ESQ2X16_SQ1, esq2x16_sq1_device)

#endif // MAME_ENSONIQ_ESQLCD_H
