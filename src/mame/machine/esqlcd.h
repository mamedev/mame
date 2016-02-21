// license:BSD-3-Clause
// copyright-holders:R. Belmont, Parduz
#ifndef ESQLCD_H
#define ESQLCD_H

#include "emu.h"
#include "esqvfd.h"


// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------

#define MCFG_ESQ2x16_SQ1_ADD(_tag)  \
	MCFG_DEVICE_ADD(_tag, ESQ2x16_SQ1, 60)

#define MCFG_ESQ2x16_SQ1_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

class esq2x16_sq1_t : public esqvfd_t {
public:
	esq2x16_sq1_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void write_char(int data) override;
	virtual void update_display() override;
	virtual void device_reset() override;


	void lcd_reset();
	void page_reset();
	char RotateLcdChar(UINT8 lcdChar, int charRow);
protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	UINT8 m_lcdpg[4][32];
	int m_lcdPage;
	int m_lcdPos,m_lcdSavedPos;

	UINT8 m_leds[16];
	UINT8 m_ledsDirty[16];

private:
	int m_LcdCommand;
};

extern const device_type ESQ2x16_SQ1;

#endif
