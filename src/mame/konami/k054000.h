// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
#ifndef MAME_KONAMI_K054000_H
#define MAME_KONAMI_K054000_H

#pragma once


class k054000_device : public device_t
{
public:
	k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~k054000_device() {}

	void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	void acx_w(offs_t offset, u8 data);
	void acy_w(offs_t offset, u8 data);
	void bcx_w(offs_t offset, u8 data);
	void bcy_w(offs_t offset, u8 data);
	s32 convert_raw_to_result_delta(u8 *buf);
	u8 axis_check(u32 ac, u32 bc, u32 aa, u32 ba);
	u8 status_r();

	u8 m_raw_acx[4]{}, m_raw_acy[4]{}, m_raw_bcx[4]{}, m_raw_bcy[4]{};
	s32 m_acx = 0, m_acy = 0, m_bcx = 0, m_bcy = 0;
	s32 m_aax = 0, m_aay = 0, m_bax = 0, m_bay = 0;

	std::string print_hitbox_state(bool result);
};

DECLARE_DEVICE_TYPE(K054000, k054000_device)

#endif // MAME_KONAMI_K054000_H
