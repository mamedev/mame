// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_DIVMMC_H
#define MAME_SINCLAIR_SPECNEXT_DIVMMC_H

#pragma once

class specnext_divmmc_device : public device_t
{
public:
	specnext_divmmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void cpu_a_15_13_w(u8 data) { m_cpu_a_15_13 = data & 0x07; };
	void cpu_mreq_n_w(bool data) { m_cpu_mreq_n = data; }
	void cpu_m1_n_w(bool data) { m_cpu_m1_n = data; }
	void automap_active_w(bool data) { m_automap_active = data; }
	void automap_rom3_active_w(bool data) { m_automap_rom3_active = data; }

	void en_w(bool data) { m_en = data; }
	void automap_reset_w(bool data) { m_automap_reset = data; }
	void retn_seen_w(bool data) { m_retn_seen = data; }

	void divmmc_button_w(bool data) { m_divmmc_button = data; }
	void divmmc_reg_w(u8 data) { m_divmmc_reg = data; }

	void automap_instant_on_w(bool data) { m_automap_instant_on = data; }
	void automap_delayed_on_w(bool data) { m_automap_delayed_on = data; }
	void automap_delayed_off_w(bool data) { m_automap_delayed_off = data; }
	void automap_rom3_instant_on_w(bool data) { m_automap_rom3_instant_on = data; }
	void automap_rom3_delayed_on_w(bool data) { m_automap_rom3_delayed_on = data; }
	void automap_nmi_instant_on_w(bool data) { m_automap_nmi_instant_on = data; }
	void automap_nmi_delayed_on_w(bool data) { m_automap_nmi_delayed_on = data; }

	bool divmmc_rom_en_r() const { return rom_en() && m_en; }
	bool divmmc_ram_en_r() const { return ram_en() && m_en; }
	bool divmmc_rdonly_r() const { return page0() || (mapram() && ram_bank() == 3); }
	u8 divmmc_ram_bank_r() const { return ram_bank() & 0x0f; }

	void clock_w() noexcept; // called on active clock edge

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// in
	u8 m_cpu_a_15_13;
	bool m_cpu_mreq_n;
	bool m_cpu_m1_n;

	bool m_en;
	bool m_automap_reset;
	bool m_automap_active;
	bool m_automap_rom3_active;
	bool m_retn_seen;

	bool m_divmmc_button;
	u8 m_divmmc_reg;

	bool m_automap_instant_on;
	bool m_automap_delayed_on;
	bool m_automap_delayed_off;
	bool m_automap_rom3_instant_on;
	bool m_automap_rom3_delayed_on;
	bool m_automap_nmi_instant_on;
	bool m_automap_nmi_delayed_on;

	// internal
	bool m_button_nmi;
	bool m_automap_hold;
	bool m_automap_held;

	// signal
	bool conmem() const noexcept;
	bool mapram() const noexcept;
	bool page0() const noexcept;
	bool page1() const noexcept;
	bool rom_en() const noexcept;
	bool ram_en() const noexcept;
	u8 ram_bank() const noexcept;
	bool automap_nmi_instant_on() const noexcept;
	bool automap_nmi_delayed_on() const noexcept;
	bool automap() const noexcept;
};

DECLARE_DEVICE_TYPE(SPECNEXT_DIVMMC, specnext_divmmc_device)

#endif // MAME_SINCLAIR_SPECNEXT_DIVMMC_H
