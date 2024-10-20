// license:BSD-3-Clause
// copyright-holders: R. Belmont

#ifndef MAME_VIDEO_ATIRAGE_H
#define MAME_VIDEO_ATIRAGE_H

#pragma once

#include "machine/pci.h"
#include "video/ati_mach32.h"

class atirage_device : public pci_device
{
public:
	atirage_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void reg_map(address_map &map) ATTR_COLD;

	auto gpio_get_cb() { return read_gpio.bind(); }
	auto gpio_set_cb() { return write_gpio.bind(); }

	void set_gpio_pullups(u16 pullups) { m_gpio_pullups = pullups; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void config_map(address_map &map) override ATTR_COLD;

	required_device<mach64_device> m_mach64;
	required_device<screen_device> m_screen;

	u8 m_regs0[0x400];
	u8 m_regs1[0x400];

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	devcb_read16 read_gpio;
	devcb_write16 write_gpio;

	u32 m_user_cfg;
	u32 m_hres, m_vres, m_htotal, m_vtotal, m_format, m_pixel_clock;
	u8 m_dac_windex, m_dac_rindex, m_dac_state, m_dac_mask;
	u32 m_dac_colors[256];
	u8 m_pll_regs[16];
	u16 m_gpio_pullups;

	u8 regs_0_read(offs_t offset);
	void regs_0_write(offs_t offset, u8 data);
	u8 regs_1_read(offs_t offset);
	void regs_1_write(offs_t offset, u8 data);

	u32 user_cfg_r();
	void user_cfg_w(u32 data);

	void update_mode();
};

class atirageii_device : public atirage_device
{
public:
	atirageii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class atirageiic_device : public atirage_device
{
public:
	atirageiic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class atirageiidvd_device : public atirage_device
{
public:
	atirageiidvd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	void legacy_io_map(address_map &map) ATTR_COLD;

private:
	required_memory_region m_vga_rom;
};

class atiragepro_device : public atirage_device
{
public:
	atiragepro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(ATI_RAGEII, atirageii_device)
DECLARE_DEVICE_TYPE(ATI_RAGEIIC, atirageiic_device)
DECLARE_DEVICE_TYPE(ATI_RAGEIIDVD, atirageiidvd_device)
DECLARE_DEVICE_TYPE(ATI_RAGEPRO, atiragepro_device)

#endif
