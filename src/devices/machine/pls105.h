// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_PLS105_H
#define MAME_MACHINE_PLS105_H

#pragma once

class pls105_device : public device_t
{
public:
	pls105_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	void pr_w(int state);
	template <unsigned N> void i_w(int state);
	template <unsigned N> auto f() { return m_f_cb[N].bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void decode();
	attotime next_cycle() const;
	void update(s32 param);
	void output(u8 mask = 0xff);

private:
	required_region_ptr<u8> m_fuse;
	devcb_write_line::array<8> m_f_cb;

	emu_timer *m_update;

	// device state
	bool m_pr; // preset/output enable
	u16 m_i;   // logic inputs
	u8 m_p;    // state flip-flops (6)
	u8 m_f;    // output flip-flops (8)

	// decoded fuses
	u64 m_fuse_i[2][16]; // input
	u64 m_fuse_p[2][6];  // present state
	u64 m_fuse_c[2];     // complement
	u64 m_fuse_n[2][6];  // next state
	u64 m_fuse_f[2][8];  // output function
	bool m_option;       // option fuse (0=PR, 1=OE)
};

DECLARE_DEVICE_TYPE(PLS105, pls105_device)

#endif // MAME_MACHINE_PLS105_H
