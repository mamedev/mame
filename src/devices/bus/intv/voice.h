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

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// reading and writing
	// actual IntelliVoice access
	virtual uint16_t read_speech(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual void write_speech(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;

	// passthru access
	virtual uint16_t read_rom04(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom04(space, offset, mem_mask); }
	virtual uint16_t read_rom20(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom20(space, offset, mem_mask); }
	virtual uint16_t read_rom40(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom40(space, offset, mem_mask); }
	virtual uint16_t read_rom48(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom48(space, offset, mem_mask); }
	virtual uint16_t read_rom50(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom50(space, offset, mem_mask); }
	virtual uint16_t read_rom60(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom60(space, offset, mem_mask); }
	virtual uint16_t read_rom70(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom70(space, offset, mem_mask); }
	virtual uint16_t read_rom80(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override
	{
		if (m_ram88_enabled && offset >= 0x800)
			return m_subslot->read_ram(space, offset & 0x7ff, mem_mask);
		else
			return m_subslot->read_rom80(space, offset, mem_mask);
	}
	virtual uint16_t read_rom90(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom90(space, offset, mem_mask); }
	virtual uint16_t read_roma0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_roma0(space, offset, mem_mask); }
	virtual uint16_t read_romb0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_romb0(space, offset, mem_mask); }
	virtual uint16_t read_romc0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_romc0(space, offset, mem_mask); }
	virtual uint16_t read_romd0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override
	{
		if (m_ramd0_enabled && offset < 0x800)
			return m_subslot->read_ram(space, offset, mem_mask);
		else
			return m_subslot->read_romd0(space, offset, mem_mask);
	}
	virtual uint16_t read_rome0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rome0(space, offset, mem_mask); }
	virtual uint16_t read_romf0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_romf0(space, offset, mem_mask); }

	// RAM passthru write
	virtual void write_88(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { if (m_ram88_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	virtual void write_d0(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { if (m_ramd0_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	virtual uint16_t read_ram(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_ram(space, offset, mem_mask); }
	virtual void write_ram(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { m_subslot->write_ram(space, offset, data, mem_mask); }

	virtual void late_subslot_setup() override;

private:
	required_device<sp0256_device> m_speech;
	required_device<intv_cart_slot_device> m_subslot;
	bool m_ramd0_enabled, m_ram88_enabled;
};




// device type definition
extern const device_type INTV_ROM_VOICE;

#endif
