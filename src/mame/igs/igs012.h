// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert

#ifndef MAME_IGS_IGS012_H
#define MAME_IGS_IGS012_H

#pragma once

class igs012_device : public device_t
{
public:
	igs012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// ROM overlay: the host supplies the ROM and the RAM pages, and maps overlay_r over the ROM while the overlay is active
	auto rom_read_callback() { return m_rom_read_cb.bind(); }
	auto ram_read_callback() { return m_ram_read_cb.bind(); }
	auto overlay_callback() { return m_overlay_cb.bind(); }

	void regs_w(offs_t offset, u8 data);
	u16 overlay_r(offs_t offset);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void prot_mode_w(offs_t offset, u8 data);
	void prot_inc_w(offs_t offset, u8 data);
	void prot_dec_inc_w(offs_t offset, u8 data);
	void prot_dec_copy_w(offs_t offset, u8 data);
	void prot_copy_w(offs_t offset, u8 data);
	void prot_swap_w(offs_t offset, u8 data);
	u16 prot_r();

	devcb_read16 m_rom_read_cb;
	devcb_read16 m_ram_read_cb;
	devcb_write_line m_overlay_cb;

	u8 m_prot = 0;
	u8 m_prot_swap = 0;
	u8 m_prot_mode = 0;

	u8 m_overlay_page[8];
	u8 m_overlay_enable;
};


DECLARE_DEVICE_TYPE(IGS012, igs012_device)

#endif // MAME_IGS_IGS012_H
