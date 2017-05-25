// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_MACHINE_TELEPRINTER_H
#define MAME_MACHINE_TELEPRINTER_H

#pragma once

#include "machine/terminal.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define TELEPRINTER_TAG "teleprinter"
#define TELEPRINTER_SCREEN_TAG "tty_screen"

#define MCFG_GENERIC_TELEPRINTER_KEYBOARD_CB(cb) \
		generic_terminal_device::set_keyboard_callback(*device, KEYBOARDCB_##cb);

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class teleprinter_device : public generic_terminal_device
{
public:
	static constexpr unsigned WIDTH = 80;
	static constexpr unsigned HEIGHT = 50;

	teleprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint32_t tp_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void term_write(uint8_t data) override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	void scroll_line();
	void write_char(uint8_t data);
	void clear();
};

DECLARE_DEVICE_TYPE(TELEPRINTER, teleprinter_device)

#endif // MAME_MACHINE_TELEPRINTER_H
