// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __INTV_ECS_H
#define __INTV_ECS_H

#include "slot.h"
#include "rom.h"
#include "sound/ay8910.h"



// ======================> intv_ecs_device

class intv_ecs_device : public intv_rom_device
{
public:
	// construction/destruction
	intv_ecs_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const rom_entry *device_rom_region() const override;

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
	DECLARE_READ8_MEMBER(ay_porta_r);
	DECLARE_READ8_MEMBER(ay_portb_r);
	DECLARE_WRITE8_MEMBER(ay_porta_w);

	// passthru accesses
	virtual DECLARE_READ16_MEMBER(read_rom04) override { return m_subslot->read_rom04(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom40) override { return m_subslot->read_rom40(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom48) override { return m_subslot->read_rom48(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom50) override { return m_subslot->read_rom50(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom60) override { return m_subslot->read_rom60(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_rom80) override
	{
		if (m_ram88_enabled && offset >= 0x800)
			return m_subslot->read_ram(space, offset & 0x7ff, mem_mask);
		else
			return m_subslot->read_rom80(space, offset, mem_mask);
	}
	virtual DECLARE_READ16_MEMBER(read_rom90) override { return m_subslot->read_rom90(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_roma0) override { return m_subslot->read_roma0(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_romb0) override { return m_subslot->read_romb0(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_romc0) override { return m_subslot->read_romc0(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(read_romd0) override
	{
		if (m_ramd0_enabled && offset < 0x800)
			return m_subslot->read_ram(space, offset, mem_mask);
		else
			return m_subslot->read_romd0(space, offset, mem_mask);
	}

	// paged ROM banking
	virtual DECLARE_WRITE16_MEMBER(write_rom20) override
	{
		if (offset == 0xfff)
		{
			if (data == 0x2a50)
				m_bank_base[2] = 0;
			else if (data == 0x2a51)
				m_bank_base[2] = 1;
		}
	}
	virtual DECLARE_WRITE16_MEMBER(write_rom70) override
	{
		if (offset == 0xfff)
		{
			if (data == 0x7a50)
				m_bank_base[7] = 0;
			else if (data == 0x7a51)
				m_bank_base[7] = 1;
		}
	}
	virtual DECLARE_WRITE16_MEMBER(write_rome0) override
	{
		if (offset == 0xfff)
		{
			if (data == 0xea50)
				m_bank_base[14] = 0;
			else if (data == 0xea51)
				m_bank_base[14] = 1;
		}
	}
	virtual DECLARE_WRITE16_MEMBER(write_romf0) override
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
	virtual DECLARE_WRITE16_MEMBER(write_88) override { if (m_ram88_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(write_d0) override { if (m_ramd0_enabled) m_subslot->write_ram(space, offset, data, mem_mask); }
	// IntelliVoice passthru
	virtual DECLARE_READ16_MEMBER(read_speech) override { if (m_voice_enabled) return m_subslot->read_speech(space, offset, mem_mask); else return 0xffff; }
	virtual DECLARE_WRITE16_MEMBER(write_speech) override { if (m_voice_enabled) m_subslot->write_speech(space, offset, data, mem_mask); }

	virtual void late_subslot_setup() override;

	UINT8 intv_control_r(int hand);

private:

	required_device<ay8914_device> m_snd;
	required_device<intv_cart_slot_device> m_subslot;
	required_ioport_array<7> m_keybd;
	required_ioport_array<7> m_synth;
	required_ioport m_cntrlsel;
	required_ioport m_options;
	required_ioport_array<2> m_keypad;
	required_ioport_array<2> m_disc;
	required_ioport_array<2> m_discx;
	required_ioport_array<2> m_discy;

	int m_bank_base[0x10];
	UINT8 m_psg_porta;
	bool m_voice_enabled, m_ramd0_enabled, m_ram88_enabled;
};





// device type definition
extern const device_type INTV_ROM_ECS;

#endif
