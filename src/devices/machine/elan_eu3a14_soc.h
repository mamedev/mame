// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_ELAN_EU3A14_SOC_H
#define MAME_MACHINE_ELAN_EU3A14_SOC_H

#include "elan_eu3a05_a.h"
#include "elan_eu3a14sys.h"
#include "elan_eu3a14vid.h"

#include "cpu/m6502/m6502.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class elan_eu3a14_cpu_device : public m6502_device {
public:
	elan_eu3a14_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void bank_change(uint16_t bank)	{ m_current_bank = bank; }
	uint16_t m_current_bank;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		return m_vid->screen_update(screen, bitmap, cliprect);
	}

	void generate_custom_interrupt(int irq) { m_sys->generate_custom_interrupt(irq); }

	// set per game kludges to pass to subdevices
	void set_default_spriteramaddr(int addr) { m_default_spriteramaddr = addr; }
	void set_tilerambase(int addr) { m_default_tileramaddr = addr; }
	void disable_timer_irq() { m_disable_timer = true; }

protected:
	elan_eu3a14_cpu_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	void int_map(address_map &map);
	address_space_config m_extbus_config;
	address_space *m_extbus_space;


	// sound callback
	uint8_t read_full_space(offs_t offset)
	{
		address_space& fullbankspace = space(5);
		return fullbankspace.read_byte(offset);
	}

	void porta_dir_w(uint8_t data)
	{
		m_portdir[0] = data;
		// update state
	}

	void portb_dir_w(uint8_t data)
	{
		m_portdir[1] = data;
		// update state
	}

	void portc_dir_w(uint8_t data)
	{
		m_portdir[2] = data;
		// update state
	}

	void porta_dat_w(uint8_t data)
	{
	}

	void portb_dat_w(uint8_t data)
	{
	}

	void portc_dat_w(uint8_t data)
	{
	}


	required_device<elan_eu3a14sys_device> m_sys;
	required_device<elan_eu3a05_sound_device> m_sound;
	required_device<elan_eu3a14vid_device> m_vid;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	uint8_t m_portdir[3];

	void sound_end0(int state) { m_sys->generate_custom_interrupt(2); }
	void sound_end1(int state) { m_sys->generate_custom_interrupt(3); }
	void sound_end2(int state) { m_sys->generate_custom_interrupt(4); }
	void sound_end3(int state) { m_sys->generate_custom_interrupt(5); }
	void sound_end4(int state) { m_sys->generate_custom_interrupt(6); }
	void sound_end5(int state) { m_sys->generate_custom_interrupt(7); }


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

private:
	// per game config kludges (until registers are figured out)
	uint16_t m_default_spriteramaddr;
	uint16_t m_default_tileramaddr;
	bool m_disable_timer;
};

DECLARE_DEVICE_TYPE(ELAN_EU3A14_SOC,     elan_eu3a14_cpu_device)

#endif // MAME_MACHINE_ELAN_EU3A14_SOC_H
