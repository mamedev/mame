// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __INTV_VOICE_H
#define __INTV_VOICE_H

#include "slot.h"
#include "rom.h"
#include "sound/sp0256.h"


// ======================> intv_voice_device

class intv_voice_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	// actual IntelliVoice access
	virtual uint16_t read_speech(offs_t offset) override;
	virtual void write_speech(offs_t offset, uint16_t data) override;

	// passthru access
	virtual uint16_t read_rom04(offs_t offset) override { return m_subslot->read_rom04(offset); }
	virtual uint16_t read_rom20(offs_t offset) override { return m_subslot->read_rom20(offset); }
	virtual uint16_t read_rom40(offs_t offset) override { return m_subslot->read_rom40(offset); }
	virtual uint16_t read_rom48(offs_t offset) override { return m_subslot->read_rom48(offset); }
	virtual uint16_t read_rom50(offs_t offset) override { return m_subslot->read_rom50(offset); }
	virtual uint16_t read_rom60(offs_t offset) override { return m_subslot->read_rom60(offset); }
	virtual uint16_t read_rom70(offs_t offset) override { return m_subslot->read_rom70(offset); }
	virtual uint16_t read_rom80(offs_t offset) override;
	virtual uint16_t read_rom90(offs_t offset) override { return m_subslot->read_rom90(offset); }
	virtual uint16_t read_roma0(offs_t offset) override { return m_subslot->read_roma0(offset); }
	virtual uint16_t read_romb0(offs_t offset) override { return m_subslot->read_romb0(offset); }
	virtual uint16_t read_romc0(offs_t offset) override { return m_subslot->read_romc0(offset); }
	virtual uint16_t read_romd0(offs_t offset) override;
	virtual uint16_t read_rome0(offs_t offset) override { return m_subslot->read_rome0(offset); }
	virtual uint16_t read_romf0(offs_t offset) override { return m_subslot->read_romf0(offset); }

	// RAM passthru write
	virtual void write_88(offs_t offset, uint16_t data) override { if (m_ram88_enabled) m_subslot->write_ram(offset, data); }
	virtual void write_d0(offs_t offset, uint16_t data) override { if (m_ramd0_enabled) m_subslot->write_ram(offset, data); }
	virtual uint16_t read_ram(offs_t offset) override { return m_subslot->read_ram(offset); }
	virtual void write_ram(offs_t offset, uint16_t data) override { m_subslot->write_ram(offset, data); }

	virtual void late_subslot_setup() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<sp0256_device> m_speech;
	required_device<intv_cart_slot_device> m_subslot;
	bool m_ramd0_enabled, m_ram88_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(INTV_ROM_VOICE, intv_voice_device)

#endif // MAME_BUS_INTV_SLOT_H
