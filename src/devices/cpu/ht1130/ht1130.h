// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_HT1130_HT1130_H
#define MAME_CPU_HT1130_HT1130_H

#pragma once

// when in halt state, inputs (configured by mask option) can wake up the CPU,
// driver is required to use set_input_line(HT1130_EXT_WAKEUP_LINE, state)
#define HT1130_EXT_WAKEUP_LINE 0


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

	// I/O ports
	auto pm_in_cb() { return m_port_in_pm.bind(); }
	auto ps_in_cb() { return m_port_in_ps.bind(); }
	auto pp_in_cb() { return m_port_in_pp.bind(); }

	auto pa_out_cb() { return m_port_out_pa.bind(); }

	// LCD output: COM in offset, SEG in data
	auto segment_out_cb() { return m_segment_out.bind(); }

protected:
	ht1130_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_run() override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == HT1130_EXT_WAKEUP_LINE; }
	virtual void execute_set_input(int inputnum, int state) override;
	virtual u32 execute_max_cycles() const noexcept override { return 2; }

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	void internal_map(address_map &map) ATTR_COLD;
	void internal_data_map(address_map &map) ATTR_COLD;

	void tempram_w(offs_t offset, u8 data);
	void displayram_w(offs_t offset, u8 data);

	required_shared_ptr<u8> m_tempram;
	required_shared_ptr<u8> m_displayram;

	address_space_config m_space_config;
	address_space_config m_data_config;

	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_space;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_data;

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

	void init_common();
	void init_lcd(u8 compins);
	TIMER_CALLBACK_MEMBER(update_lcd);
	virtual u64 get_segs(u8 com);

	void cycle();
	u8 fetch();
	void do_op();

	u32 m_pc;
	s32 m_icount;

	u8 m_regs[5];
	u8 m_acc;
	u8 m_carry;
	u8 m_irqen;
	u8 m_timer_en;
	u8 m_inhalt;
	u8 m_wakeline;
	u8 m_timerover;
	u16 m_timer;

	u8 m_comcount;
	emu_timer *m_lcd_timer;

	u16 m_stackaddr;
	u8 m_stackcarry;

	devcb_read8 m_port_in_pm;
	devcb_read8 m_port_in_ps;
	devcb_read8 m_port_in_pp;

	devcb_write8 m_port_out_pa;

	devcb_write64 m_segment_out;
};

class ht1190_device : public ht1130_device
{
public:
	ht1190_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual u64 get_segs(u8 coms) override;

private:
	void internal_data_map_ht1190(address_map &map) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(HT1130, ht1130_device)
DECLARE_DEVICE_TYPE(HT1190, ht1190_device)

#endif // MAME_CPU_HT1130_HT1130_H
