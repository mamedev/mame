// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_ELAN_EP3A19A_SOC_H
#define MAME_MACHINE_ELAN_EP3A19A_SOC_H

#include "cpu/m6502/m6502.h"

#include "elan_eu3a05_a.h"
#include "elan_eu3a05gpio.h"
#include "elan_ep3a19asys.h"
#include "elan_eu3a05vid.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class elan_ep3a19a_cpu_device : public m6502_device {
public:
	elan_ep3a19a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
	{
		return m_vid->screen_update(screen, bitmap, cliprect);
	}


protected:
	elan_ep3a19a_cpu_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	void int_map(address_map &map);
	address_space_config m_extbus_config;
	address_space *m_extbus_space;

	void bank_change(uint16_t bank)	{ m_current_bank = bank; }
	uint16_t m_current_bank;

	required_device<elan_ep3a19asys_device> m_sys;
	required_device<elan_eu3a05gpio_device> m_gpio;

	required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_ram;
	required_device<elan_eu3a05_sound_device> m_sound;
	required_device<elan_eu3a05vid_device> m_vid;
	required_device<palette_device> m_palette;

	//void sound_end0(int state) { m_sys->generate_custom_interrupt(2); }
	//void sound_end1(int state) { m_sys->generate_custom_interrupt(3); }
	//void sound_end2(int state) { m_sys->generate_custom_interrupt(4); }
	//void sound_end3(int state) { m_sys->generate_custom_interrupt(5); }
	//void sound_end4(int state) { m_sys->generate_custom_interrupt(6); }
	//void sound_end5(int state) { m_sys->generate_custom_interrupt(7); }

	uint8_t nmi_vector_r(offs_t offset)
	{
		return 0xffd4 >> (offset * 8);
	}

	uint8_t bank_r(offs_t offset)
	{
		return space(5).read_byte((m_current_bank * 0x8000) + offset);
	}

	void bank_w(offs_t offset, uint8_t data)
	{
		space(5).write_byte((m_current_bank * 0x8000) + offset, data);
	}

	uint8_t fixed_r(offs_t offset)
	{
		// always at 0 for this SoC?
		return space(5).read_byte(offset);
	}	

	// sound callback
	uint8_t read_full_space(offs_t offset)
	{
		address_space& fullbankspace = space(5);
		return fullbankspace.read_byte(offset);
	}


private:

};

DECLARE_DEVICE_TYPE(ELAN_EP3A19A_SOC,     elan_ep3a19a_cpu_device)

#endif // MAME_MACHINE_ELAN_EP3A19A_SOC_H
