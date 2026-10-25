// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_I750_I750_H
#define MAME_CPU_I750_I750_H

#pragma once

class i82750pb_device : public cpu_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::TIMING | feature::GRAPHICS; }

	i82750pb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_callback() { return m_irq_cb.bind(); }

	// host interface
	u16 host_r(offs_t offset);
	void host_w(offs_t offset, u16 data);

	// 82750DB interface
	u16 pointer_r(unsigned index) const;
	void vblank_w(int state);
	void db_shutdown_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override;
	virtual u32 execute_max_cycles() const noexcept override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	enum reg_id : unsigned
	{
		R0, R1, R2, R3, R4, R5, R6, R7,
		R8, R9, R10, R11, R12, R13, R14, R15,
		DRAM1, DRAM2, DRAM3, DRAM4, RAM1, RAM1_INC, RAM1_DEC, RAM2,
		RAM2_INC, RAM2_DEC, RAM3, RAM3_INC, RAM3_DEC, RAM4, RAM4_INC, RAM4_DEC,
		IMM0, IMM1, IMM2, IMM3, IMM4, IMM5, IMM6, IMM7,
		IN1, IN2, OUT1, OUT2, STAT, STAT_STOP, RESERVED, ALU,
		CC, CNT, CNT2, HWID, IN1_C, IN1_HI, IN1_LO, IN2_C,
		IN2_HI, IN2_LO, LCNT, MADDR, MCODE1, MCODE2, MCODE3, NUL,
		OUT1_INC, OUT1_C, OUT1_HI, OUT1_LO, OUT2_INC, OUT2_C, OUT2_HI, OUT2_LO,
		PC, PIXINT, PIXINT_C, PROF, SHIFT, SHIFT_L, SHIFT_R, SHIFT_RL,
		STAT_C, STAT_HI, STAT_LO, STAT_RAM, VPITCH, VPTR_HI, VPTR_LO, VU_HI,
		VU_LO, VUPITCH, YEVEN_HI, YEVEN_LO, YODD_HI, YODD_LO, YPITCH,
		REG_COUNT
	};

	static constexpr u16 SIGNATURE = 0xfffc;

	void program_map(address_map &map) ATTR_COLD;
	void ram_map(address_map &map) ATTR_COLD;

	static reg_id a_source(unsigned code);
	static reg_id a_destination(unsigned code);
	static reg_id b_source(unsigned code);
	static reg_id b_destination(unsigned code);
	static reg_id pointer_register(unsigned index);

	u16 source(reg_id reg, bool effects = true);
	void destination(reg_id reg, u16 value);
	unsigned ram_address(reg_id reg, bool effects);
	void alu(unsigned op, unsigned select, u16 a, u16 b);
	u16 stat_decode(bool effects = true);
	u16 pixel_read(bool effects);
	void pixel_write(u16 data);
	void pixel_push(u8 data);
	u8 pixel_pop();
	void interrupt(u16 flags);
	void update_irq();
	void snapshot_pointers();
	void pointer_w(unsigned index, u16 data);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_ram_config;
	required_shared_ptr<u64> m_code;
	required_shared_ptr<u16> m_ram;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_data;
	devcb_write_line m_irq_cb;

	u16 m_reg[REG_COUNT];
	u16 m_old_ptr[4];
	u16 m_pointer[32];
	u16 m_pc;
	u16 m_a;
	u16 m_b;
	u16 m_alu;
	u16 m_cc;
	u16 m_bus_b;
	s16 m_pending_pc;
	u16 m_control;
	u16 m_interrupt;
	u8 m_lc;
	bool m_prof;
	bool m_irq;
	u32 m_input[2];
	u32 m_output[2];
	u8 m_stat_table[8];
	u8 m_stat_index;
	u64 m_stat_bit;
	u16 m_stat_buffer;
	bool m_stat_valid;
	u8 m_pixels[32];
	u8 m_pixel_head;
	u8 m_pixel_count;
	u8 m_pixel_inputs;
	u16 m_pixel_top;
	u16 m_pixel_prev_top;
	u16 m_pixel_prev_bottom;
	bool m_pixel_previous;
	int m_icount;
};

DECLARE_DEVICE_TYPE(I82750PB, i82750pb_device)

#endif // MAME_CPU_I750_I750_H
