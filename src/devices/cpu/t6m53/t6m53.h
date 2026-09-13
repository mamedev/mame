// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/***************************************************************************

        Toshiba T6M53 ASIC

***************************************************************************/

#ifndef MAME_CPU_T6M53_T6M53_H
#define MAME_CPU_T6M53_T6M53_H

#pragma once

class t6m53_device : public cpu_device
{
public:
	t6m53_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

    auto ring_out() { return m_ring_out.bind(); }
    auto tip_out() { return m_tip_out.bind(); }
    auto ring_in() { return m_ring_in.bind(); }
    auto tip_in() { return m_tip_in.bind(); }

    auto lcd_stb() { return m_stb.bind(); }
    auto btn_rows() { return m_btn_rows.bind(); }
    auto on_btn() { return m_on_btn.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 32; }
	virtual void execute_run() override;

	virtual space_config_vector memory_space_config() const override;

	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	enum : u32
	{
		T6M53_PC = 1,
		T6M53_A,
		T6M53_SP,
		T6M53_REP,
		T6M53_IL,
		T6M53_IH,
		T6M53_DPL,
		T6M53_DPH
	};

    const uint16_t m_divisors[8] = {5040, 2100, 1575, 1050, 350, 252, 180, 105};

	uint8_t m_regs[0x100];
	uint16_t m_stack[8];
	uint16_t m_pc = 0;
    uint8_t m_isel = 0;

	int m_icount = 0;
    int m_ftimer = 0;
    int m_vtimer = 0;

	uint8_t m_lastlow = 0;
	uint8_t m_lasthigh = 0;
	uint16_t m_previous_pc = 0;
	bool m_write_repeat = false;

	address_space_config m_program_config;
	address_space *m_program = nullptr;

    devcb_write_line m_ring_out;
    devcb_write_line m_tip_out;
    devcb_read_line  m_ring_in;
    devcb_read_line  m_tip_in;

    devcb_write_line m_stb;
    devcb_read8 m_btn_rows;
    devcb_read_line m_on_btn;

	uint8_t reg_r4_raw(uint16_t index, uint8_t &last);
	uint8_t reg_r4(uint16_t index) { return reg_r4_raw(index, m_lastlow); }
	void reg_w4_raw(uint16_t index, uint8_t data);
	void reg_w4(uint16_t index, uint8_t data) { m_lastlow = data & 0x0f; reg_w4_raw(index, m_lastlow); }
	uint8_t reg_r8(uint16_t index);
	void reg_w8(uint16_t index, uint8_t data);

	uint16_t get_i() const;
	void set_i(uint16_t value);
	uint16_t get_dp() const;
	void set_dp(uint16_t value);

	uint16_t add4(uint16_t x, uint8_t y) const;
	uint16_t bcd(uint8_t x);
	uint8_t adc(int bits, uint8_t x, uint8_t y, bool &carry, uint16_t op);

	uint16_t jyx(uint16_t op, uint16_t offset) const;
	uint16_t wyx(uint16_t op, uint16_t offset) const;
	uint16_t wyxs(uint16_t op, uint16_t offset) const;
	uint16_t wyxs_no_i(uint16_t op, uint16_t offset) const;
	uint16_t ef(uint16_t op, uint16_t offset) const;

	void jump_call(bool is_cond, bool condition);
    void add_cycles(int cycles);
	void execute_op(uint16_t op, int &repeat, uint16_t offset, bool is_repeat, bool &carry);

	uint8_t repeat_count() const { return m_regs[0x110 >> 1] & 0x0f; }
	uint8_t stack_pointer() const { return m_regs[0x118 >> 1] & 0x0f; }
};

DECLARE_DEVICE_TYPE(T6M53, t6m53_device)

#endif // MAME_CPU_T6M53_T6M53_H