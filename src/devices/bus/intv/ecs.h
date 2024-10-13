// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_INTV_ECS_H
#define MAME_BUS_INTV_ECS_H

#pragma once

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

	// reading and writing

	// actual ECS accesses
	// paged ROMs
	virtual uint16_t read_rom20(offs_t offset) override;
	virtual uint16_t read_rom70(offs_t offset) override;
	virtual uint16_t read_rome0(offs_t offset) override;
	virtual uint16_t read_romf0(offs_t offset) override;
	// RAM
	virtual uint16_t read_ram(offs_t offset) override { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual void write_ram(offs_t offset, uint16_t data) override { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
	// AY8914
	virtual uint16_t read_ay(offs_t offset) override;
	virtual void write_ay(offs_t offset, uint16_t data) override;

	// passthru accesses
	virtual uint16_t read_rom04(offs_t offset) override { return m_subslot->read_rom04(offset); }
	virtual uint16_t read_rom40(offs_t offset) override { return m_subslot->read_rom40(offset); }
	virtual uint16_t read_rom48(offs_t offset) override { return m_subslot->read_rom48(offset); }
	virtual uint16_t read_rom50(offs_t offset) override { return m_subslot->read_rom50(offset); }
	virtual uint16_t read_rom60(offs_t offset) override { return m_subslot->read_rom60(offset); }
	virtual uint16_t read_rom80(offs_t offset) override;
	virtual uint16_t read_rom90(offs_t offset) override { return m_subslot->read_rom90(offset); }
	virtual uint16_t read_roma0(offs_t offset) override { return m_subslot->read_roma0(offset); }
	virtual uint16_t read_romb0(offs_t offset) override { return m_subslot->read_romb0(offset); }
	virtual uint16_t read_romc0(offs_t offset) override { return m_subslot->read_romc0(offset); }
	virtual uint16_t read_romd0(offs_t offset) override;

	// paged ROM banking
	virtual void write_rom20(offs_t offset, uint16_t data) override;
	virtual void write_rom70(offs_t offset, uint16_t data) override;
	virtual void write_rome0(offs_t offset, uint16_t data) override;
	virtual void write_romf0(offs_t offset, uint16_t data) override;
	// RAM passthru write
	virtual void write_88(offs_t offset, uint16_t data) override { if (m_ram88_enabled) m_subslot->write_ram(offset, data); }
	virtual void write_d0(offs_t offset, uint16_t data) override { if (m_ramd0_enabled) m_subslot->write_ram(offset, data); }
	// IntelliVoice passthru
	virtual uint16_t read_speech(offs_t offset) override { if (m_voice_enabled) return m_subslot->read_speech(offset); else return 0xffff; }
	virtual void write_speech(offs_t offset, uint16_t data) override { if (m_voice_enabled) m_subslot->write_speech(offset, data); }

	virtual void late_subslot_setup() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:

	required_device<ay8914_device> m_snd;
	required_device<intv_cart_slot_device> m_subslot;

	int m_bank_base[0x10];
	bool m_voice_enabled, m_ramd0_enabled, m_ram88_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(INTV_ROM_ECS, intv_ecs_device)

#endif // MAME_BUS_INTV_ECS_H
