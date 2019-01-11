// license:BSD-3-Clause
// copyright-holders:David Haywood, Jonathan Gevaryahu

#ifndef MAME_MACHINE_TAITOCCHIP_H
#define MAME_MACHINE_TAITOCCHIP_H

#pragma once

#include "cpu/upd7810/upd7811.h"
#include "machine/bankdev.h"

DECLARE_DEVICE_TYPE(TAITO_CCHIP, taito_cchip_device)

#define MCFG_TAITO_CCHIP_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, TAITO_CCHIP, _clock)

#define MCFG_CCHIP_IN_PORTA_CB(_devcb) \
	devcb = &downcast<taito_cchip_device &>(*device).set_in_pa_callback(DEVCB_##_devcb);

#define MCFG_CCHIP_IN_PORTB_CB(_devcb) \
	devcb = &downcast<taito_cchip_device &>(*device).set_in_pb_callback(DEVCB_##_devcb);

#define MCFG_CCHIP_IN_PORTC_CB(_devcb) \
	devcb = &downcast<taito_cchip_device &>(*device).set_in_pc_callback(DEVCB_##_devcb);

#define MCFG_CCHIP_IN_PORTAD_CB(_devcb) \
	devcb = &downcast<taito_cchip_device &>(*device).set_in_ad_callback(DEVCB_##_devcb);

#define MCFG_CCHIP_OUT_PORTA_CB(_devcb) \
	devcb = &downcast<taito_cchip_device &>(*device).set_out_pa_callback(DEVCB_##_devcb);

#define MCFG_CCHIP_OUT_PORTB_CB(_devcb) \
	devcb = &downcast<taito_cchip_device &>(*device).set_out_pb_callback(DEVCB_##_devcb);

#define MCFG_CCHIP_OUT_PORTC_CB(_devcb) \
	devcb = &downcast<taito_cchip_device &>(*device).set_out_pc_callback(DEVCB_##_devcb);


class taito_cchip_device :  public device_t
{
public:
	// construction/destruction
	taito_cchip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_in_pa_callback(Object &&cb)  { return m_in_pa_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_in_pb_callback(Object &&cb)  { return m_in_pb_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_in_pc_callback(Object &&cb)  { return m_in_pc_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_in_ad_callback(Object &&cb)  { return m_in_ad_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_pa_callback(Object &&cb) { return m_out_pa_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_pb_callback(Object &&cb) { return m_out_pb_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_pc_callback(Object &&cb) { return m_out_pc_cb.set_callback(std::forward<Object>(cb)); }


	// can be accessed externally
	DECLARE_READ8_MEMBER(asic_r);
	DECLARE_WRITE8_MEMBER(asic_w);
	DECLARE_WRITE8_MEMBER(asic68_w);

	DECLARE_READ8_MEMBER(mem_r);
	DECLARE_WRITE8_MEMBER(mem_w);

	DECLARE_READ8_MEMBER(mem68_r);
	DECLARE_WRITE8_MEMBER(mem68_w);


	void cchip_map(address_map &map);
	void cchip_ram_bank(address_map &map);
	void cchip_ram_bank68(address_map &map);

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_READ8_MEMBER(portc_r);

	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE8_MEMBER(portf_w);

	DECLARE_READ_LINE_MEMBER(an0_r);
	DECLARE_READ_LINE_MEMBER(an1_r);
	DECLARE_READ_LINE_MEMBER(an2_r);
	DECLARE_READ_LINE_MEMBER(an3_r);
	DECLARE_READ_LINE_MEMBER(an4_r);
	DECLARE_READ_LINE_MEMBER(an5_r);
	DECLARE_READ_LINE_MEMBER(an6_r);
	DECLARE_READ_LINE_MEMBER(an7_r);

	void ext_interrupt(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_reset() override;

private:
	uint8_t m_asic_ram[4];

	required_device<cpu_device> m_upd7811;
	required_device<address_map_bank_device> m_upd4464_bank;
	required_device<address_map_bank_device> m_upd4464_bank68;
	required_shared_ptr<uint8_t> m_upd4464;

	devcb_read8        m_in_pa_cb;
	devcb_read8        m_in_pb_cb;
	devcb_read8        m_in_pc_cb;
	devcb_read8        m_in_ad_cb;
	devcb_write8       m_out_pa_cb;
	devcb_write8       m_out_pb_cb;
	devcb_write8       m_out_pc_cb;
};

#endif // MAME_MACHINE_CCHIP_DEV_H
