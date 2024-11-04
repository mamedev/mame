// license:BSD-3-Clause
#ifndef MAME_SINCLAIR_ATM_H
#define MAME_SINCLAIR_ATM_H

#pragma once

#include "spec128.h"

#include "beta_m.h"
#include "bus/ata/ataintf.h"
#include "bus/centronics/ctronics.h"

class atm_state : public spectrum_128_state
{
public:
	atm_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bank_view0(*this, "bank_view0")
		, m_bank_view1(*this, "bank_view1")
		, m_bank_view2(*this, "bank_view2")
		, m_bank_view3(*this, "bank_view3")
		, m_io_view(*this, "io_view")
		, m_bank_rom(*this, "bank_rom%u", 0U)
		, m_char_rom(*this, "charrom")
		, m_beta(*this, BETA_DISK_TAG)
		, m_ata(*this, "ata")
		, m_centronics(*this, "centronics")
		, m_palette(*this, "palette")
	{ }

	void atm(machine_config &config);
	void atmtb2(machine_config &config);
	void atmtb2plus(machine_config &config);

protected:
	static constexpr u16 PEN_WRDISBL_MASK = 1 << 13;
	static constexpr u16 PEN_RAMNROM_MASK = 1 << 14; // 1-RAM, 0-ROM
	static constexpr u16 PEN_DOS7FFD_MASK = 1 << 15;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void atm_io(address_map &map) ATTR_COLD;
	void atm_mem(address_map &map) ATTR_COLD;
	void atm_switch(address_map &map) ATTR_COLD;
	template <u8 Bank> void atm_ram_w(offs_t offset, u8 data);

	u8 beta_neutral_r(offs_t offset);
	u8 beta_enable_r(offs_t offset);
	u8 beta_disable_r(offs_t offset);
	u8 ata_r(offs_t offset);
	void ata_w(offs_t offset, u8 data);

	void atm_ula_w(offs_t offset, u8 data);
	virtual void atm_port_ff_w(offs_t offset, u8 data);
	void atm_port_77_w(offs_t offset, u8 data);
	void atm_port_f7_w(offs_t offset, u8 data);
	void atm_port_7ffd_w(offs_t offset, u8 data);

	virtual void atm_update_cpu();
	virtual void atm_update_io();
	u16 &pen_page(u8 bank) { return m_pages_map[BIT(m_port_7ffd_data, 4)][bank]; }
	void atm_update_memory();
	virtual u8 merge_ram_with_7ffd(u8 ram_page) { return (ram_page & ~0x07) | (m_port_7ffd_data & 0x07); }
	virtual bool is_port_7ffd_locked() { return BIT(m_port_7ffd_data, 5); }
	bool is_dos_active() { return !m_cpm_n || m_beta->is_active(); }

	virtual void spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void atm_update_video_mode();
	void atm_update_screen_lo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atm_update_screen_hi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atm_update_screen_tx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual u16 atm_update_memory_get_page(u8 bank);
	virtual u8 get_border_color(u16 hpos, u16 vpos) override;
	virtual rectangle get_screen_area() override;
	INTERRUPT_GEN_MEMBER(atm_interrupt);

	memory_view m_bank_view0;
	memory_view m_bank_view1;
	memory_view m_bank_view2;
	memory_view m_bank_view3;
	memory_view m_io_view;
	required_memory_bank_array<4> m_bank_rom;
	optional_region_ptr<u8> m_char_rom; // required for ATM2, absent in ATM1
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

	required_device<beta_disk_device> m_beta;
	required_device<ata_interface_device> m_ata;
	required_device<centronics_device> m_centronics;
	required_device<device_palette_interface> m_palette;

	u8 *m_char_location;
	u8 rom_pages_mask;
	u8 ram_pages_mask;

	u8 m_port_77_data;
	bool m_pen;           // PEN - extended memory manager
	bool m_cpm_n;
	u16 m_pages_map[2][4]; // map: 0,1

	bool m_pen2;          // palette selector
	u8 m_rg = 0b011;      // 0:320x200lo, 2:640:200hi, 3:256x192zx, 6:80x25txt
	u8 m_br3;
	std::vector<u8> m_palette_data;
	u8 m_ata_data_latch;
	u8 m_beta_drive_selected;
};

#endif // MAME_SINCLAIR_ATM_H
