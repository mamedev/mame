// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
#ifndef MAME_KONAMI_K054000_H
#define MAME_KONAMI_K054000_H

#pragma once


class k054000_device : public device_t
{
public:
	k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
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
	int convert_raw_to_result_delta(u8 *buf);
	int convert_raw_to_result(u8 *buf);
	u8 axis_check(u32 m_Ac, u32 m_Bc, u32 m_Aa, u32 m_Ba);
	u8 status_r();

	u8 m_raw_Acx[4]{}, m_raw_Acy[4]{}, m_raw_Bcx[4]{}, m_raw_Bcy[4]{};
	int m_Acx = 0, m_Acy = 0, m_Bcx = 0, m_Bcy = 0;
	int m_Aax = 0, m_Aay = 0, m_Bax = 0, m_Bay = 0;

	std::string print_hitbox_state(bool result);
};

DECLARE_DEVICE_TYPE(K054000, k054000_device)

#endif // MAME_KONAMI_K054000_H
