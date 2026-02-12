// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 keyboard emulation

*********************************************************************/

#ifndef MAME_NOKIA_MM2KB_H
#define MAME_NOKIA_MM2KB_H

#pragma once

DECLARE_DEVICE_TYPE(NOKIA_MM2_KBD, mm2_keyboard_device)

class mm2_keyboard_device :  public device_t
{
public:
	// construction/destruction
	mm2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto txd_handler() { return m_write_txd.bind(); }

	void rxd_w(int state) { }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write_line m_write_txd;
};

#endif // MAME_NOKIA_MM2KB_H
