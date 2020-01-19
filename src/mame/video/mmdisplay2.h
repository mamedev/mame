// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Mephisto Display Module (2nd version)

*********************************************************************/

#ifndef MAME_VIDEO_MMDISPLAY2_H
#define MAME_VIDEO_MMDISPLAY2_H

#pragma once

#include "video/hd44780.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// ======================> mephisto_display_module2_device

class mephisto_display_module2_device : public device_t
{
public:
	// construction/destruction
	mephisto_display_module2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	hd44780_device *get() { return m_lcdc; }

	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_WRITE8_MEMBER(io_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void lcd_palette(palette_device &palette) const;

private:
	optional_device<hd44780_device> m_lcdc;
	required_device<dac_byte_interface> m_dac;
	uint8_t m_latch;
	uint8_t m_ctrl;
};


// device type definition
DECLARE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE2, mephisto_display_module2_device)

#endif // MAME_VIDEO_MMDISPLAY2_H
