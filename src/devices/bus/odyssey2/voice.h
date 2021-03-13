// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli
/******************************************************************************

    Magnavox The Voice emulation

******************************************************************************/

#ifndef MAME_BUS_ODYSSEY2_VOICE_H
#define MAME_BUS_ODYSSEY2_VOICE_H

#pragma once

#include "slot.h"
#include "sound/sp0256.h"


// ======================> o2_voice_device

class o2_voice_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void cart_init() override;

	// reading and writing
	virtual u8 read_rom04(offs_t offset) override { return (m_subslot->exists()) ? m_subslot->read_rom04(offset) : 0xff; }
	virtual u8 read_rom0c(offs_t offset) override { return (m_subslot->exists()) ? m_subslot->read_rom0c(offset) : 0xff; }

	virtual void write_p1(u8 data) override { m_control = data; if (m_subslot->exists()) m_subslot->write_p1(data & 3); }
	virtual void write_p2(u8 data) override { if (m_subslot->exists()) m_subslot->write_p2(data & ~4); }
	virtual void bus_write(u8 data) override { if (m_subslot->exists()) m_subslot->bus_write(data); }
	virtual u8 bus_read() override { return (m_subslot->exists()) ? m_subslot->bus_read() : 0xff; }

	virtual void io_write(offs_t offset, u8 data) override;
	virtual DECLARE_READ_LINE_MEMBER(t0_read) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<sp0256_device> m_speech;
	required_device<o2_cart_slot_device> m_subslot;

	u8 m_control = 0;
};


// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_VOICE, o2_voice_device)

#endif // MAME_BUS_ODYSSEY2_VOICE_H
