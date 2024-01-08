// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_UPD777_UPD777_H
#define MAME_CPU_UPD777_UPD777_H

#pragma once


class upd777_cpu_device : public cpu_device
{
public:

	enum {
		UPD777_PC = 0,
		UPD777_A1,
		UPD777_A2,
		UPD777_A3,
		UPD777_A4,
		UPD777_SKIP,
		UPD777_H,
		UPD777_L,
		UPD777_ADDR_STACK0,
		UPD777_ADDR_STACK1,
		UPD777_ADDR_STACK2,
		UPD777_ADDR_STACK_POS,
	};

protected:
	upd777_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	upd777_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	void internal_map(address_map &map);
	void internal_data_map(address_map &map);

	required_shared_ptr<u8> m_datamem;

private:
	address_space_config m_space_config;
	address_space_config m_data_config;

	memory_access<11, 1, -1, ENDIANNESS_LITTLE>::specific m_space;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_data;


	std::string get_300optype_name(int optype);
	std::string get_200optype_name(int optype);
	std::string get_reg_name(int reg);

	void increment_pc();
	u16 fetch();
	void do_op();


	u8 read_data_mem(u8 addr);
	void write_data_mem(u8 addr, u8 data);
	void push_to_stack(u16 addr);
	u16 pull_from_stack();

	u8 get_l();
	void set_l(int l);
	u8 get_h_shifted();
	u8 get_h();
	void set_h(int h);
	void set_a11(int a11);
	void set_new_pc(int newpc);
	void set_a1(u8 data);
	void set_a2(u8 data);
	void set_a3(u8 data);
	void set_a4(u8 data);

	void set_frs(u8 frs);
	void set_fls(u8 fls);
	void set_mode(u8 mode);

	u8 get_m_data();
	void set_m_data(u8 data);

	void set_disp(u8 data);
	void set_gpe(u8 data);
	void set_kie(u8 data);
	void set_sme(u8 data);

	u8 get_kie();
	u8 get_sme();

	u8 get_a1();
	u8 get_a2();
	u8 get_a3();
	u8 get_a4();

	u32 m_ppc;
	u32 m_pc;
	s32 m_icount;
	u8 m_skip;
	u8 m_a[4]; // A1 to A4
	u8 m_h;
	u8 m_l;
	u16 m_stack[3]; // 3 11-bit registers
	u8 m_stackpos;

	u8 m_frs;
	u8 m_fls;

	u8 m_mode;

	// single bit enable registers?
	u8 m_disp;
	u8 m_gpe;
	u8 m_kie;
	u8 m_sme;
};

#endif // MAME_CPU_UPD777_UPD777_H
