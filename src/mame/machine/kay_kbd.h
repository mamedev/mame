// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_MACHINE_KAY_KBD_H
#define MAME_MACHINE_KAY_KBD_H

#pragma once

#include "sound/spkrdev.h"


#define MCFG_KAYPRO10KBD_RXD_CB(cb) \
		devcb = &downcast<kaypro_10_keyboard_device &>(*device).set_rxd_cb(DEVCB_##cb);


class kaypro_10_keyboard_device : public device_t
{
public:
	kaypro_10_keyboard_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			std::uint32_t clock);

	template <class Object> devcb_base &set_rxd_cb(Object &&cb) { return m_rxd_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(txd_w) { m_txd = state ? 1U : 0U; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ_LINE_MEMBER(t1_r);
	DECLARE_READ8_MEMBER(bus_r);
	DECLARE_WRITE8_MEMBER(bus_w);

private:
	required_device<cpu_device>             m_mcu;
	required_device<speaker_sound_device>   m_bell;
	required_ioport_array<16>               m_matrix;
	required_ioport                         m_modifiers;
	devcb_write_line                        m_rxd_cb;

	std::uint8_t        m_txd;
	std::uint8_t        m_bus;
};

DECLARE_DEVICE_TYPE(KAYPRO_10_KEYBOARD, kaypro_10_keyboard_device)

#endif // MAME_MACHINE_KAY_KBD_H
