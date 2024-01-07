// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_UPD777_UPD777_H
#define MAME_CPU_UPD777_UPD777_H

#pragma once


class upd777_device : public cpu_device
{
public:
	upd777_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		UPD777_PC = 0
	};

protected:
	upd777_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	void internal_map(address_map &map);
	void internal_data_map(address_map &map);

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

	u32 m_ppc;
	u32 m_pc;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(UPD777, upd777_device)

#endif // MAME_CPU_UPD777_UPD777_H
