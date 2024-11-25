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

	auto drq_w()     { return m_drq_w.bind(); }
	auto sp1_frm_w() { return m_sp_frm_w[0].bind();   }
	auto sp1_dat_w() { return m_sp_dat_w[0].bind();   }
	auto sp1_clk_w() { return m_sp_clk_w[0].bind();   }
	auto sp2_frm_w() { return m_sp_frm_w[1].bind();   }
	auto sp2_dat_w() { return m_sp_dat_w[1].bind();   }
	auto sp2_clk_w() { return m_sp_clk_w[1].bind();   }

	void setup8_w(u8 data); // a = 0 (also mpeg data in pio mode)
	void mc18_w  (u8 data); // a = 1
	void cmd8_w  (u8 data); // a = 2
	void mc238_w (u8 data); // a = 3

	void setup_w(u16 data); // a = 0 (also mpeg data in pio mode)
	void mc1_w  (u16 data); // a = 1
	void cmd_w  (u16 data); // a = 2
	void mc23_w (u16 data); // a = 3

	// For when d0-d7 and d8-d15 are deliberately inverted
	void setupx_w(u16 data); // a = 0 (also mpeg data in pio mode)
	void mc1x_w  (u16 data); // a = 1
	void cmdx_w  (u16 data); // a = 2
	void mc23x_w (u16 data); // a = 3

	void dma8_w  (u8  data); // mpeg data write with dma
	void dma_w   (u16 data);
	void dmax_w  (u16 data);

	u8   stat08_r(); // a = 0
	u8   stat18_r(); // a = 1
	u8   stat28_r(); // a = 2
	u8   user8_r();  // a = 3

	u16  stat0_r();  // a = 0
	u16  stat1_r();  // a = 1
	u16  stat2_r();  // a = 2
	u16  user_r();   // a = 3

	u16  stat0x_r(); // a = 0
	u16  stat1x_r(); // a = 1
	u16  stat2x_r(); // a = 2
	u16  userx_r();  // a = 3

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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

	devcb_write_line m_drq_w;
	devcb_write_line m_sp_frm_w[2];
	devcb_write_line m_sp_dat_w[2];
	devcb_write_line m_sp_clk_w[2];

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
	void end_decoding(u8 mode);
};

DECLARE_DEVICE_TYPE(ZR36110, zr36110_device)

#endif

