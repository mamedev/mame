// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gew7.h

    Yamaha GEW7, GEW7I, GEW7S (65c02-based)

***************************************************************************/
#ifndef MAME_CPU_M6502_GEW7_H
#define MAME_CPU_M6502_GEW7_H

#include "m65c02.h"
#include "sound/gew7.h"

class gew7_device : public m65c02_device, public device_mixer_interface
{
public:

	template<offs_t Num> auto port_in_cb() { return m_in_cb[Num].bind(); }
	template<offs_t Num> auto port_out_cb() { return m_out_cb[Num].bind(); }

protected:
	gew7_device(const machine_config &mconfig, device_type type, bool ignore_in_ddr, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;

	virtual void device_start() override;
	virtual void device_reset() override;

	u8 timer_stat_r();
	void timer_stat_w(u8 data);
	void timer_en_w(u8 data);

	u8 timer_count_r(offs_t offset);
	void timer_count_w(offs_t offset, u8 data);

	void bank_w(offs_t offset, u8 data);

	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);
	u8 port_ddr_r(offs_t offset);
	void port_ddr_w(offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(timer_tick);

	void internal_map(address_map &map);

	emu_timer* m_timer[2];
	u8 m_timer_stat;
	u16 m_timer_count[2];

	devcb_read8::array<6> m_in_cb;
	devcb_write8::array<6> m_out_cb;
	u8 m_port_data[6];
	u8 m_port_ddr[6];
	// GEW7 and GEW7S seem to have different behavior when reading input bits
	// (see gew7_device::port_r)
	const bool m_ignore_in_ddr;

	required_memory_region        m_rom;
	required_memory_bank_array<2> m_bank;
	u8 m_bank_mask;

	required_device<gew7_pcm_device> m_pcm;
};

class ymw270f_device : public gew7_device
{
public:
	ymw270f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ymw276f_device : public gew7_device
{
public:
	ymw276f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ymw282f_device : public gew7_device
{
public:
	ymw282f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(YMW270F, ymw270f_device)
DECLARE_DEVICE_TYPE(YMW276F, ymw276f_device)
DECLARE_DEVICE_TYPE(YMW282F, ymw282f_device)

#endif // MAME_CPU_M6502_GEW7_H
