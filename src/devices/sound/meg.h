// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha MEG - Multiple effects generator
//
// Audio dsp dedicated to effects generation, part of the SWP20 lineup

#ifndef DEVICES_SOUND_MEG_H
#define DEVICES_SOUND_MEG_H

#pragma once

class meg_device : public device_t
{
public:
	meg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 44100*256);
	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_r4[256];
	u8 m_r5[256];
	u8 m_r8[256];
	u8 m_r9[256];
	u8 m_re[256];
	u8 m_rf[256];
	u8 m_r12[256];
	u8 m_r13[256];
	u8 m_r14[256];
	u8 m_r16[256];
	u8 m_r17[256];
	u8 m_r18[256];
	u8 m_reg;

	std::array<s16, 0x100> m_const;
	std::array<s16, 0x100> m_offset;

	u8 s2_r();
	u8 s10_r();
	u8 s11_r();
	void select_w(u8 reg);
	void s1_w(u8 data);
	void s2_w(u8 data);
	void s3_w(u8 data);
	void s4_w(u8 data);
	void s5_w(u8 data);
	void s7_w(u8 data);
	void s8_w(u8 data);
	void s9_w(u8 data);
	void sa_w(u8 data);
	void consth_w(u8 data);
	void constl_w(u8 data);
	void se_w(u8 data);
	void sf_w(u8 data);
	void s10_w(u8 data);
	void s11_w(u8 data);
	void offseth_w(u8 data);
	void offsetl_w(u8 data);
	void s14_w(u8 data);
	void s15_w(u8 data);
	void s16_w(u8 data);
	void s17_w(u8 data);
	void s18_w(u8 data);
};

DECLARE_DEVICE_TYPE(MEG, meg_device)

#endif
