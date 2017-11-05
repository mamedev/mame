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
	virtual DECLARE_READ16_MEMBER(read_rom20) override;
	virtual DECLARE_READ16_MEMBER(read_rom70) override;
	virtual DECLARE_READ16_MEMBER(read_rome0) override;
	virtual DECLARE_READ16_MEMBER(read_romf0) override;
	// RAM
	virtual DECLARE_READ16_MEMBER(read_ram) override { return (int)m_ram[offset & (m_ram.size() - 1)]; }
	virtual DECLARE_WRITE16_MEMBER(write_ram) override { m_ram[offset & (m_ram.size() - 1)] = data & 0xff; }
	// AY8914
	virtual DECLARE_READ16_MEMBER(read_ay) override;
	virtual DECLARE_WRITE16_MEMBER(write_ay) override;

	// passthru accesses
	virtual DECLARE_READ16_MEMBER(read_rom04) override { return m_subslot->read_rom04(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom40) override { return m_subslot->read_rom40(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom48) override { return m_subslot->read_rom48(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom50) override { return m_subslot->read_rom50(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom60) override { return m_subslot->read_rom60(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom80) override;
	virtual DECLARE_READ16_MEMBER(read_rom90) override { return m_subslot->read_rom90(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_roma0) override { return m_subslot->read_roma0(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_romb0) override { return m_subslot->read_romb0(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_romc0) override { return m_subslot->read_romc0(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_romd0) override;

	// paged ROM banking
	virtual DECLARE_WRITE16_MEMBER(write_rom20) override;
	virtual DECLARE_WRITE16_MEMBER(write_rom70) override;
	virtual DECLARE_WRITE16_MEMBER(write_rome0) override;
	virtual DECLARE_WRITE16_MEMBER(write_romf0) override;
	// RAM passthru write
	virtual DECLARE_WRITE16_MEMBER(write_88) override { if (m_ram88_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(write_d0) override { if (m_ramd0_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	// IntelliVoice passthru
	virtual DECLARE_READ16_MEMBER(read_speech) override { if (m_voice_enabled) return m_subslot->read_speech(space, offset, mem_mask); else return 0xffff; }
	virtual DECLARE_WRITE16_MEMBER(write_speech) override { if (m_voice_enabled) m_subslot->write_speech(space, offset, data, mem_mask); }

	virtual void late_subslot_setup() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:

	required_device<ay8914_device> m_snd;
	required_device<intv_cart_slot_device> m_subslot;

	int m_bank_base[0x10];
	bool m_voice_enabled, m_ramd0_enabled, m_ram88_enabled;
};


// device type definition
DECLARE_DEVICE_TYPE(INTV_ROM_ECS, intv_ecs_device)

#endif // MAME_BUS_INTV_ECS_H
