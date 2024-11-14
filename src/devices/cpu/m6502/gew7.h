// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gew7.h

    Yamaha GEW7, GEW7I, GEW7S (65c02-based)

***************************************************************************/
#ifndef MAME_CPU_M6502_GEW7_H
#define MAME_CPU_M6502_GEW7_H

#include "m6502mcu.h"
#include "m65c02.h"
#include "sound/gew7.h"

class gew7_device : public m6502_mcu_device_base<m65c02_device>, public device_mixer_interface
{
public:
	gew7_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	template<offs_t Num> auto port_in_cb() { return m_in_cb[Num].bind(); }
	template<offs_t Num> auto port_out_cb() { return m_out_cb[Num].bind(); }

	void port_force_bits(offs_t num, u8 bits, u8 mask);

protected:

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void internal_update(u64 current_time) override;
	using m6502_mcu_device_base<m65c02_device>::internal_update;

	u8 timer_stat_r();
	void timer_stat_w(u8 data);
	void timer_en_w(u8 data);

	u8 timer_count_r(offs_t offset);
	void timer_count_w(offs_t offset, u8 data);
	u64 timer_update(int num, u64 current_time);

	void bank_w(offs_t offset, u8 data);

	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);
	u8 port_ddr_r(offs_t offset);
	void port_ddr_w(offs_t offset, u8 data);

	void internal_map(address_map &map) ATTR_COLD;

	u8 m_timer_stat, m_timer_en;
	u16 m_timer_count[2];
	u64 m_timer_base[2];

	devcb_read8::array<6> m_in_cb;
	devcb_write8::array<6> m_out_cb;
	u8 m_port_data[6];
	u8 m_port_ddr[6];
	u8 m_port_force_bits[6];
	u8 m_port_force_mask[6];

	required_memory_region        m_rom;
	required_memory_bank_array<2> m_bank;
	u8 m_bank_mask;

	required_device<gew7_pcm_device> m_pcm;
};

DECLARE_DEVICE_TYPE(GEW7, gew7_device)

#endif // MAME_CPU_M6502_GEW7_H
