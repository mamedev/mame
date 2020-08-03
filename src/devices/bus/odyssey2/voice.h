// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ODYSSEY2_VOICE_H
#define MAME_BUS_ODYSSEY2_VOICE_H

#pragma once

#include "slot.h"
#include "rom.h"
#include "sound/sp0256.h"


// ======================> o2_voice_device

class o2_voice_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom04(offs_t offset) override { return (m_subslot->exists()) ? m_subslot->read_rom04(offset) : 0xff; }
	virtual uint8_t read_rom0c(offs_t offset) override { return (m_subslot->exists()) ? m_subslot->read_rom0c(offset) : 0xff; }

	virtual void write_p1(uint8_t data) override { m_control = data; if (m_subslot->exists()) m_subslot->write_p1(data); }
	virtual void write_p2(uint8_t data) override { if (m_subslot->exists()) m_subslot->write_p2(data); }

	virtual void io_write(offs_t offset, uint8_t data) override;
	virtual uint8_t io_read(offs_t offset) override { return (m_subslot->exists()) ? m_subslot->io_read(offset) : 0xff; }
	virtual DECLARE_READ_LINE_MEMBER(t0_read) override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(lrq_callback);

	required_device<sp0256_device> m_speech;
	required_device<o2_cart_slot_device> m_subslot;

	int m_lrq_state;
	uint8_t m_control;
};


// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_VOICE, o2_voice_device)

#endif // MAME_BUS_ODYSSEY2_VOICE_H
