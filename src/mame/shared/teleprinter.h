// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_SHARED_TELEPRINTER_H
#define MAME_SHARED_TELEPRINTER_H

#pragma once

#include "machine/terminal.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define TELEPRINTER_SCREEN_TAG "tty_screen"

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class teleprinter_device : public generic_terminal_device
{
public:
	teleprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void term_write(uint8_t data) override;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static inline constexpr unsigned WIDTH = 80;
	static inline constexpr unsigned HEIGHT = 50;

	uint32_t tp_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void scroll_line();
	void write_char(uint8_t data);
	void clear();
};

DECLARE_DEVICE_TYPE(TELEPRINTER, teleprinter_device)

#endif // MAME_SHARED_TELEPRINTER_H
