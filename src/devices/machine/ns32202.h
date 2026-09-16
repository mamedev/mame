// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_NS32202_H
#define MAME_MACHINE_NS32202_H

#pragma once

class ns32202_device : public device_t
{
public:
	// output lines
	auto out_int() { return m_out_int.bind(); }
	auto out_cout() { return m_out_cout.bind(); }
	template <unsigned Number> auto out_g() { return m_out_g[Number].bind(); }

	ns32202_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// input lines
	template <unsigned Number> void ir_w(int state);
	template <unsigned Number> void g_w(int state);

	template <unsigned ST1> void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_int(bool int_state);
	void set_cout(bool cout_state);

	void interrupt(s32 param);
	u8 interrupt_acknowledge(bool side_effects);
	u8 interrupt_return(bool side_effects);

	void interrupt_update();

	template <unsigned N> void counter(s32 param);

	// register read handlers
	template <unsigned ST1, bool SideEffects> u8 hvct_r();
	u8 eltgl_r() { return u8(m_eltg); }
	u8 eltgh_r() { return m_eltg >> 8; }
	u8 tpll_r() { return u8(m_tpl); }
	u8 tplh_r() { return m_tpl >> 8; }
	u8 ipndl_r() { return u8(m_ipnd); }
	u8 ipndh_r() { return m_ipnd >> 8; }
	u8 isrvl_r() { return u8(m_isrv); }
	u8 isrvh_r() { return m_isrv >> 8; }
	u8 imskl_r() { return u8(m_imsk); }
	u8 imskh_r() { return m_imsk >> 8; }
	u8 csrcl_r() { return u8(m_csrc); }
	u8 csrch_r() { return m_csrc >> 8; }
	u8 fprtl_r() { return u8(m_fprt); }
	u8 fprth_r() { return m_fprt >> 8; }
	u8 mctl_r() { return m_mctl; }
	u8 ocasn_r() { return m_ocasn; }
	u8 ciptr_r() { return m_ciptr; }
	u8 pdat_r();
	u8 ips_r() { return m_ips; }
	u8 pdir_r() { return m_pdir; }
	u8 cctl_r() { return m_cctl; }
	u8 cictl_r() { return m_cictl; }
	template <unsigned N> u8 csvl_r() { return u8(m_csv[N]); }
	template <unsigned N> u8 csvh_r() { return m_csv[N] >> 8; }
	template <unsigned N> u8 ccvl_r() { return u8(m_ccv[N]); }
	template <unsigned N> u8 ccvh_r() { return m_ccv[N] >> 8; }

	// register write handlers
	void svct_w(u8 data);
	void eltgl_w(u8 data);
	void eltgh_w(u8 data);
	void tpll_w(u8 data);
	void tplh_w(u8 data);
	void ipndl_w(u8 data);
	void ipndh_w(u8 data);
	void isrvl_w(u8 data);
	void isrvh_w(u8 data);
	void imskl_w(u8 data);
	void imskh_w(u8 data);
	void csrcl_w(u8 data);
	void csrch_w(u8 data);
	void fprtl_w(u8 data);
	void fprth_w(u8 data) {}
	void mctl_w(u8 data);
	void ocasn_w(u8 data);
	void ciptr_w(u8 data);
	void pdat_w(u8 data);
	void ips_w(u8 data);
	void pdir_w(u8 data);
	void cctl_w(u8 data);
	void cictl_w(u8 data);
	template <unsigned N> void csvl_w(u8 data);
	template <unsigned N> void csvh_w(u8 data);
	template <unsigned N> void ccvl_w(u8 data);
	template <unsigned N> void ccvh_w(u8 data);

	void update_ccv();
	void update_port();

private:
	devcb_write_line m_out_int;
	devcb_write_line m_out_cout;
	devcb_write_line::array<8> m_out_g;

	emu_timer *m_interrupt;
	emu_timer *m_counter[2];

	u8 m_hvct;    // hardware vector
	u16 m_eltg;   // edge/level triggering (1=level)
	u16 m_tpl;    // triggering polarity (1=active high)
	u16 m_ipnd;   // interrupts pending (1=pending)
	u16 m_isrv;   // interrupts in-service (1=in-service)
	u16 m_imsk;   // interrupt mask (1=masked)
	u16 m_csrc;   // cascaded source
	u16 m_fprt;   // first priority
	u8 m_mctl;    // mode control
	u8 m_ocasn;   // output clock assignment (1=clock output)
	u8 m_ciptr;   // counter interrupt pointer
	u8 m_pdat;    // port data
	u8 m_ips;     // interrupt/port select (1=interrupt)
	u8 m_pdir;    // port direction (1=input)
	u8 m_cctl;    // counter control
	u8 m_cictl;   // counter interrupt control
	u16 m_csv[2]; // counter starting value
	u16 m_ccv[2]; // counter current value

	u8 m_isrv_count[16];

	u16 m_line_state;
	bool m_out_int_state;
	bool m_out_cout_state;

	u8 m_pdat_in;
	u8 m_pdat_out;
};

DECLARE_DEVICE_TYPE(NS32202, ns32202_device)

#endif // MAME_MACHINE_NS32202_H
