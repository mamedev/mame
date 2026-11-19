// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    r6511.h

    Rockwell R6511Q one-chip microprocessor (R6500/11 family)

***************************************************************************/

#ifndef MAME_CPU_M6502_R6511_H
#define MAME_CPU_M6502_R6511_H

#pragma once

#include "m6502mcu.h"

class r6511_device : public m6502_mcu_device_base<m6502_device>
{
public:
	r6511_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto pa_out_cb() { return m_port_out_cb[0].bind(); }
	auto pb_out_cb() { return m_port_out_cb[1].bind(); }
	auto pc_out_cb() { return m_port_out_cb[2].bind(); }
	auto pd_out_cb() { return m_port_out_cb[3].bind(); }
	auto pa_in_cb() { return m_port_in_cb[0].bind(); }
	auto pb_in_cb() { return m_port_in_cb[1].bind(); }
	auto pc_in_cb() { return m_port_in_cb[2].bind(); }
	auto pd_in_cb() { return m_port_in_cb[3].bind(); }

	template <unsigned Bit> void pa_w(int state)
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(r6511_device::set_pa_in), this), (Bit << 1) | (state ? 1 : 0));
	}

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	enum
	{
		R6511_MCR = M6502_IR + 1,
		R6511_IER,
		R6511_IFR,
		R6511_SCCR,
		R6511_SCSR,
		R6511_CA,
		R6511_LA,
		R6511_CB,
		R6511_LB,
		R6511_LC
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override;
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override;

	virtual void internal_update(u64 current_time) override;
	using m6502_mcu_device_base<m6502_device>::internal_update;

private:
#define O(o) void o ## _full(); void o ## _partial()

	O(bbr_zpb);
	O(bbs_zpb);
	O(rmb_bzp);
	O(smb_bzp);

#undef O

	void internal_map(address_map &map) ATTR_COLD;

	bool rmw_read() const;
	u8 pa_output() const;
	u8 pa_pins() const;
	void pa_update(u8 prev_pins);
	void update_irq();
	TIMER_CALLBACK_MEMBER(set_pa_in);

	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);
	void ifr_clear_w(u8 data);
	u8 ifr_r();
	u8 ier_r();
	void ier_w(u8 data);
	u8 mcr_r();
	void mcr_w(u8 data);
	u8 sccr_r();
	void sccr_w(u8 data);
	u8 scsr_r();
	void scsr_w(u8 data);
	u8 serial_data_r();
	void serial_data_w(u8 data);
	void update_serial_irq();

	u8 lca_r();
	u8 uca_r();
	u8 lca_noclear_r();
	void lla_w(u8 data);
	void ula_w(u8 data);
	void ula_start_w(u8 data);
	u8 lcb_r();
	u8 ucb_r();
	u8 lcb_noclear_r();
	void llb_w(u8 data);
	void ulb_latch_c_w(u8 data);
	void ulb_start_w(u8 data);

	unsigned counter_a_mode() const { return m_mcr & 0x03; }
	unsigned counter_b_mode() const { return (m_mcr >> 2) & 0x03; }
	u64 update_counter_a(u64 current_time);
	u64 update_counter_b(u64 current_time);

	devcb_read8::array<4> m_port_in_cb;
	devcb_write8::array<4> m_port_out_cb;

	u8 m_port_latch[4];
	u8 m_pa_in;
	u8 m_pa_out;
	u8 m_pb_latch;

	u8 m_mcr;
	u8 m_ier;
	u8 m_ifr;
	u8 m_sccr;
	u8 m_scsr;
	u8 m_rdr;

	u64 m_counter_a_base;
	u16 m_counter_a;
	u16 m_latch_a;
	u8 m_cnta_out;

	u64 m_counter_b_base;
	u16 m_counter_b;
	u16 m_latch_b;
	u16 m_latch_c;
	u8 m_cntb_out;
};

DECLARE_DEVICE_TYPE(R6511, r6511_device)

#endif // MAME_CPU_M6502_R6511_H
