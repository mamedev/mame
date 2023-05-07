// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Zoran ZR36110 mpeg video decoder

#ifndef MAME_VIDEO_ZR36110
#define MAME_VIDEO_ZR36110

#pragma once

class zr36110_device : public device_t
{
public:
	zr36110_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	auto drq_handler() { return m_drq_handler.bind(); }

	void setup8_w(u8 data); // a = 0
	void mc18_w  (u8 data); // a = 1
	void cmd8_w  (u8 data); // a = 2
	void mc238_w (u8 data); // a = 3

	u8   stat0_r();
	u8   stat1_r();
	u8   stat2_r();
	u8   user_r (); // a = 3

protected:
	void device_start();
	void device_reset();

private:
	enum {
		S_INIT   = 0x0,
		S_IDLE   = 0x1,
		S_NORMAL = 0x2,
		S_PAUSE  = 0x3,
		S_STEP   = 0x4,
		S_DFIRST = 0x6,
		S_DNEXT  = 0x7,
		S_END    = 0x8
	};
	
	devcb_write_line m_drq_handler;

	u8 m_setup[0x80];
	u32 m_mc1_adr;
	u32 m_mc23_adr;
	u32 m_setup_adr;
	u16 m_cmd;
	u8 m_state, m_bus_control;
	bool m_cmd_phase;

	static double u6_10_to_f(u16 val);
	static double u5_19_to_f(u32 val);
	void setup_show() const;

	void go();
};

DECLARE_DEVICE_TYPE(ZR36110, zr36110_device)

#endif

