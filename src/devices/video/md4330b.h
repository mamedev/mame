// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitel MD4330B / MD4332B LCD Driver

*/

#ifndef MAME_VIDEO_MD4330B_H
#define MAME_VIDEO_MD4330B_H

#pragma once

// pinout reference

/*
             ____   ____                           ____   ____
    _T/C  1 |*   \_/    | 40 VDD          _T/C  1 |*   \_/    | 40 VDD
      DI  2 |           | 39 CLK            DI  2 |           | 39 CLK
      NC  3 |           | 38 RST            NC  3 |           | 38 RST
      NC  4 |           | 37 DO             Q1  4 |           | 37 DO
      Q1  5 |           | 36 Q30            Q2  5 |           | 36 Q32
      Q2  6 |           | 35 Q29            Q3  6 |           | 35 Q31
      Q3  7 |           | 34 Q28            Q4  7 |           | 34 Q30
      Q4  8 |           | 33 Q27            Q5  8 |           | 33 Q29
      Q5  9 |           | 32 Q26            Q6  9 |           | 32 Q28
      Q6 10 |  MD4330BC | 31 Q25            Q7 10 |  MD4332BC | 31 Q27
      Q7 11 |  MD4330BE | 30 Q24            Q8 11 |  MD4332BE | 30 Q26
      Q8 12 |           | 29 Q23            Q9 12 |           | 29 Q25
      Q9 13 |           | 28 Q22           Q10 13 |           | 28 Q24
     Q10 14 |           | 27 Q21           Q11 14 |           | 27 Q23
     Q11 15 |           | 26 Q20           Q12 15 |           | 26 Q22
     Q12 16 |           | 25 Q19           Q13 16 |           | 25 Q21
     Q13 17 |           | 24 Q18           Q14 17 |           | 24 Q20
     Q14 18 |           | 23 Q17           Q15 18 |           | 23 Q19
     Q15 19 |           | 22 Q16           Q16 19 |           | 22 Q18
     VSS 20 |___________| 21 NC            VSS 20 |___________| 21 Q17

*/


class md4330b_device : public device_t
{
public:
	md4330b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	auto write_q() { return m_write_q.bind(); }
	auto write_do() { return m_write_do.bind(); }

	void clk_w(int state);
	void tc_w(int state) { m_tc = (state) ? 1 : 0; update_output(); }
	void di_w(int state) { m_di = (state) ? 1 : 0; }
	void rst_w(int state) { m_rst = (state) ? 1 : 0; }
	int do_r() { return m_do; }

protected:
	md4330b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 qmax);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void update_output();

	const u8 m_qmax; // number of Q pins
	u32 m_shift;

	// pin state
	int m_clk;
	int m_di;
	int m_do;
	int m_rst;
	int m_tc;

	// callbacks
	devcb_write32 m_write_q;
	devcb_write_line m_write_do;
};


class md4332b_device : public md4330b_device
{
public:
	md4332b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};


DECLARE_DEVICE_TYPE(MD4330B, md4330b_device)
DECLARE_DEVICE_TYPE(MD4332B, md4332b_device)

#endif // MAME_VIDEO_MD4330B_H
