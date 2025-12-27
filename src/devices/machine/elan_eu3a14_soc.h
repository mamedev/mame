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

class elan_eu3a14_soc_device : public m6502_device {
public:
	elan_eu3a14_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		AS_EXTERNAL = 5,
	};

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return m_vid->screen_update(screen, bitmap, cliprect); }

	template <typename T> void set_screen(T&& tag) { m_screen.set_tag(std::forward<T>(tag)); }

	void generate_custom_interrupt(int irq) { m_sys->generate_custom_interrupt(irq); }

	template <int Port> auto write_callback() { return m_write_callback[Port].bind(); }
	template <int Port> auto read_callback() { return m_read_callback[Port].bind(); }

	void set_is_pal() { m_is_pal = true; }

	// set per game kludges to pass to subdevices
	void set_default_spriteramaddr(int addr) { m_default_spriteramaddr = addr; }
	void set_tilerambase(int addr) { m_default_tileramaddr = addr; }
	void disable_timer_irq() { m_disable_timer = true; }

protected:
	elan_eu3a14_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	void int_map(address_map &map);
	address_space_config m_extbus_config;
	address_space *m_extbus_space;

	void porta_dir_w(uint8_t data) { m_portdir[0] = data; /* TODO: update state */ }
	void portb_dir_w(uint8_t data) { m_portdir[1] = data; /* TODO: update state */ }
	void portc_dir_w(uint8_t data) { m_portdir[2] = data; /* TODO: update state */ }

	uint8_t porta_dat_r() { /* TODO, use direction */ return m_read_callback[0](); }
	uint8_t portb_dat_r() { /* TODO, use direction */ return m_read_callback[1](); }
	uint8_t portc_dat_r() { /* TODO, use direction */ return m_read_callback[2](); }

	void porta_dat_w(uint8_t data) { /* TODO, use direction */ m_write_callback[0](data); }
	void portb_dat_w(uint8_t data) { /* TODO, use direction */ m_write_callback[1](data); }
	void portc_dat_w(uint8_t data) { /* TODO, use direction */ m_write_callback[2](data); }

	void sound_end0(int state) { m_sys->generate_custom_interrupt(2); }
	void sound_end1(int state) { m_sys->generate_custom_interrupt(3); }
	void sound_end2(int state) { m_sys->generate_custom_interrupt(4); }
	void sound_end3(int state) { m_sys->generate_custom_interrupt(5); }
	void sound_end4(int state) { m_sys->generate_custom_interrupt(6); }
	void sound_end5(int state) { m_sys->generate_custom_interrupt(7); }

	void bank_change(uint16_t bank) { m_current_bank = bank; }
	uint8_t read_full_space(offs_t offset) { address_space &extspace = space(AS_EXTERNAL); return extspace.read_byte(offset); }
	uint8_t bank_r(offs_t offset) { return space(AS_EXTERNAL).read_byte((m_current_bank * 0x8000) + offset); }
	void bank_w(offs_t offset, uint8_t data) { space(AS_EXTERNAL).write_byte((m_current_bank * 0x8000) + offset, data); }
	uint8_t fixed_r(offs_t offset) { /* always at 0 for this SoC ? */ return space(AS_EXTERNAL).read_byte(offset); }

	required_device<elan_eu3a14sys_device> m_sys;
	required_device<elan_eu3a05_sound_device> m_sound;
	required_device<elan_eu3a14vid_device> m_vid;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	uint16_t m_current_bank;
	uint8_t m_portdir[3];

private:
	devcb_read8::array<3> m_read_callback;
	devcb_write8::array<3> m_write_callback;

	// per game config kludges (until registers are figured out)
	uint16_t m_default_spriteramaddr;
	uint16_t m_default_tileramaddr;
	bool m_disable_timer;
	bool m_is_pal;
};

DECLARE_DEVICE_TYPE(ELAN_EU3A14_SOC,     elan_eu3a14_soc_device)

#endif // MAME_MACHINE_ELAN_EU3A14_SOC_H
