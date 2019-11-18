// license:BSD-3-Clause
// copyright-holders:David Haywood, Jonathan Gevaryahu
#ifndef MAME_MACHINE_TAITOCCHIP_H
#define MAME_MACHINE_TAITOCCHIP_H

#pragma once

#include "machine/bankdev.h"

DECLARE_DEVICE_TYPE(TAITO_CCHIP, taito_cchip_device)


class taito_cchip_device :  public device_t
{
public:
	// construction/destruction
	taito_cchip_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto in_pa_callback()  { return m_in_pa_cb.bind(); }
	auto in_pb_callback()  { return m_in_pb_cb.bind(); }
	auto in_pc_callback()  { return m_in_pc_cb.bind(); }
	auto in_ad_callback()  { return m_in_ad_cb.bind(); }
	auto out_pa_callback() { return m_out_pa_cb.bind(); }
	auto out_pb_callback() { return m_out_pb_cb.bind(); }
	auto out_pc_callback() { return m_out_pc_cb.bind(); }

	// can be accessed externally
	u8 asic_r(offs_t offset);
	void asic_w(offs_t offset, u8 data);
	void asic68_w(offs_t offset, u8 data);

	u8 mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);

	u8 mem68_r(offs_t offset);
	void mem68_w(offs_t offset, u8 data);

	void ext_interrupt(int state);

protected:
	void cchip_map(address_map &map);
	void cchip_ram_bank(address_map &map);
	void cchip_ram_bank68(address_map &map);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_reset() override;

private:
	u8 m_asic_ram[4];

	required_device<cpu_device> m_upd7811;
	required_device<address_map_bank_device> m_upd4464_bank;
	required_device<address_map_bank_device> m_upd4464_bank68;
	required_shared_ptr<u8> m_upd4464;

	devcb_read8        m_in_pa_cb;
	devcb_read8        m_in_pb_cb;
	devcb_read8        m_in_pc_cb;
	devcb_read8        m_in_ad_cb;
	devcb_write8       m_out_pa_cb;
	devcb_write8       m_out_pb_cb;
	devcb_write8       m_out_pc_cb;
};

#endif // MAME_MACHINE_TAITOCCHIP_H
