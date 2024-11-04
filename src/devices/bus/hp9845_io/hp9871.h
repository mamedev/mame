// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9871.h

    HP9871 daisy-wheel printer

*********************************************************************/

#ifndef MAME_BUS_HP9845_IO_HP9871_H
#define MAME_BUS_HP9845_IO_HP9871_H

#pragma once

#include "98032.h"
#include "imagedev/printer.h"

class hp9871_device : public device_t, public device_hp98032_gpio_interface
{
public:
	hp9871_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9871_device();

	// hp98032_gpio_card_device overrides
	virtual uint16_t get_jumpers() const override;
	virtual uint16_t input_r() const override;
	virtual uint8_t ext_status_r() const override;
	virtual void output_w(uint16_t data) override;
	virtual void ext_control_w(uint8_t data) override;
	virtual void pctl_w(int state) override;
	virtual void io_w(int state) override;
	virtual void preset_w(int state) override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<printer_image_device> m_printer;

	uint8_t m_data;
	bool m_ibf;

	void printer_online(int state);
	void update_busy();
	void output(bool printer_ready);
};

DECLARE_DEVICE_TYPE(HP9871 , hp9871_device)

#endif /* MAME_BUS_HP9845_IO_HP9871_H */
