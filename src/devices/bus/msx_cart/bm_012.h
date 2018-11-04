// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_BM_012_H
#define MAME_BUS_MSX_CART_BM_012_H

#pragma once

#include "bus/msx_cart/cartridge.h"
#include "cpu/z80/tmpz84c015.h"
#include "bus/midi/midi.h"


DECLARE_DEVICE_TYPE(MSX_CART_BM_012, msx_cart_bm_012_device)


class msx_cart_bm_012_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_bm_012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(midi_in);

	void bm_012_memory_map(address_map &map);

	required_device<tmpz84c015_device> m_tmpz84c015af;
	required_device<z80pio_device> m_bm012_pio;
	required_device<midi_port_device> m_mdthru;
};


#endif // MAME_BUS_MSX_CART_BM_012_H
