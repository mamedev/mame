// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    IGS003 I/O and protection chip

***************************************************************************/

#ifndef MAME_IGS_IGS003_H
#define MAME_IGS_IGS003_H

#pragma once


class igs003_device : public device_t
{
public:
	// hold protection: table of taps used for bit 0, one row per mode (selected by the game)
	enum class hold_taps : u8 { A, B };

	igs003_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// board side: three 8-bit ports, direction set per bit by the CPU (indices 0x03-0x05, 1 = input)
	auto in_pa_callback() { return m_in_port_cb[0].bind(); }
	auto in_pb_callback() { return m_in_port_cb[1].bind(); }
	auto in_pc_callback() { return m_in_port_cb[2].bind(); }
	auto out_pa_callback() { return m_out_port_cb[0].bind(); }
	auto out_pb_callback() { return m_out_port_cb[1].bind(); }
	auto out_pc_callback() { return m_out_port_cb[2].bind(); }

	// fixed in each chip variant, orlegend changes them at reset to stand in for each region's chip
	void set_hold_taps(hold_taps taps) { m_hold_taps = taps; }
	void set_hold_xpos(u8 p0, u8 p1, u8 p2, u8 p3) { m_hold_xpos[0] = p0; m_hold_xpos[1] = p1; m_hold_xpos[2] = p2; m_hold_xpos[3] = p3; }

	void index_w(u8 data);
	u8 data_r();
	void data_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	hold_taps m_hold_taps;
	u8 m_hold_xpos[4];

	u8 m_index;
	u8 m_latch[3];
	u8 m_dir[3];

	u16 m_hold;
	u16 m_word;
	u8 m_x;
	u8 m_mode;

	u8 port_r(int port);
	void port_w(int port, u8 data);
	void dir_w(int port, u8 data);
	void hold_step(int data_bit);
	u8 hold_r() const;

	devcb_read8::array<3> m_in_port_cb;
	devcb_write8::array<3> m_out_port_cb;
};

DECLARE_DEVICE_TYPE(IGS003, igs003_device)

#endif // MAME_IGS_IGS003_H
