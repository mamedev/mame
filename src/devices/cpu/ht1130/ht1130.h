// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_HT1130_HT1130_H
#define MAME_CPU_HT1130_HT1130_H

#pragma once


class ht1130_device : public cpu_device
{
public:
	ht1130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		HT1130_PC,
		HT1130_R0,
		HT1130_R1,
		HT1130_R2,
		HT1130_R3,
		HT1130_R4,
		HT1130_ACC,
		HT1130_TIMER_EN,
		HT1130_TIMER,
	};

	auto pm_in_cb() { return m_port_in_pm.bind(); }
	auto ps_in_cb() { return m_port_in_ps.bind(); }
	auto pp_in_cb() { return m_port_in_pp.bind(); }

	auto pa_out_cb() { return m_port_out_pa.bind(); }

	auto display_offset_out_cb() { return m_display_offset_out.bind(); }
	auto display_data_out_cb() { return m_display_data_out.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// address space
	address_space_config m_space_config;
	address_space_config m_extregs_config;

	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_space;

	void internal_map(address_map &map);
	void internal_data_map(address_map &map);

	void tempram_w(offs_t offset, u8 data);
	void displayram_w(offs_t offset, u8 data);

	void setreg(u8 which, u8 data);
	u8 getreg(u8 which);
	void setacc(u8 data);
	u8 getacc();
	void setcarry();
	void clearcarry();
	u8 getcarry();
	u8 getr1r0();
	u8 getr1r0_data();
	void setr1r0_data(u8 data);
	u8 getr3r2();
	u8 getr3r2_data();
	void setr3r2_data(u8 data);
	void settimer(u8 data);
	void settimer_upper(u8 data);
	void settimer_lower(u8 data);
	u8 gettimer_upper();
	u8 gettimer_lower();

	inline u8 fetch();
	void do_op();

	u32 m_pc;
	s32 m_icount;

	u8 m_regs[5];
	u8 m_acc;
	u8 m_carry;
	u8 m_irqen;
	u8 m_timer_en;
	u8 m_inhalt;
	u8 m_timerover;
	u16 m_timer;

	u16 m_stackaddr;
	u8 m_stackcarry;

	required_shared_ptr<u8> m_tempram;
	required_shared_ptr<u8> m_displayram;

	devcb_read8 m_port_in_pm;
	devcb_read8 m_port_in_ps;
	devcb_read8 m_port_in_pp;

	devcb_write8 m_port_out_pa;

	devcb_write8 m_display_offset_out;
	devcb_write8 m_display_data_out;

};

DECLARE_DEVICE_TYPE(HT1130, ht1130_device)

#endif // MAME_CPU_HT1130_HT1130_H
