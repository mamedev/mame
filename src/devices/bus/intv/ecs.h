// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __INTV_ECS_H
#define __INTV_ECS_H

#include "slot.h"
#include "rom.h"
#include "sound/ay8910.h"
#include "bus/intv_ctrl/ecs_ctrl.h"


// ======================> intv_ecs_device

class intv_ecs_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_ecs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// reading and writing

	// actual ECS accesses
	// paged ROMs
	virtual uint16_t read_rom20(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual uint16_t read_rom70(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual uint16_t read_rome0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual uint16_t read_romf0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	// RAM
	virtual uint16_t read_ram(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual void write_ram(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
	// AY8914
	virtual uint16_t read_ay(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual void write_ay(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;

	// passthru accesses
	virtual uint16_t read_rom04(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom04(space, offset, mem_mask); }
	virtual uint16_t read_rom40(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom40(space, offset, mem_mask); }
	virtual uint16_t read_rom48(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom48(space, offset, mem_mask); }
	virtual uint16_t read_rom50(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom50(space, offset, mem_mask); }
	virtual uint16_t read_rom60(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { return m_subslot->read_rom60(space, offset, mem_mask); }
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

	// paged ROM banking
	virtual void write_rom20(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override
	{
		if (offset == 0xfff)
		{
			if (data == 0x2a50)
				m_bank_base[2] = 0;
			else if (data == 0x2a51)
				m_bank_base[2] = 1;
		}
	}
	virtual void write_rom70(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override
	{
		if (offset == 0xfff)
		{
			if (data == 0x7a50)
				m_bank_base[7] = 0;
			else if (data == 0x7a51)
				m_bank_base[7] = 1;
		}
	}
	virtual void write_rome0(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override
	{
		if (offset == 0xfff)
		{
			if (data == 0xea50)
				m_bank_base[14] = 0;
			else if (data == 0xea51)
				m_bank_base[14] = 1;
		}
	}
	virtual void write_romf0(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override
	{
		if (offset == 0xfff)
		{
			if (data == 0xfa50)
				m_bank_base[15] = 0;
			else if (data == 0xfa51)
				m_bank_base[15] = 1;
		}
	}
	// RAM passthru write
	virtual void write_88(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { if (m_ram88_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	virtual void write_d0(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { if (m_ramd0_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	// IntelliVoice passthru
	virtual uint16_t read_speech(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override { if (m_voice_enabled) return m_subslot->read_speech(space, offset, mem_mask); else return 0xffff; }
	virtual void write_speech(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override { if (m_voice_enabled) m_subslot->write_speech(space, offset, data, mem_mask); }

	virtual void late_subslot_setup() override;

private:

	required_device<ay8914_device> m_snd;
	required_device<intv_cart_slot_device> m_subslot;

	int m_bank_base[0x10];
	bool m_voice_enabled, m_ramd0_enabled, m_ram88_enabled;
};


// device type definition
extern const device_type INTV_ROM_ECS;

#endif
