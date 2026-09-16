// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_HPC_HPC_H
#define MAME_CPU_HPC_HPC_H

#pragma once


class hpc_device : public cpu_device
{
public:
	enum {
		HPC_PSW,
		HPC_SP,
		HPC_PC,
		HPC_A,
		HPC_K,
		HPC_B,
		HPC_X
	};

protected:
	// construction/destruction
	hpc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// internal register access
	u8 psw_r();
	void psw_w(u8 data);

private:
	// address space
	address_space_config m_program_config;
	address_space *m_program;

	// internal state
	required_shared_ptr<u16> m_core_regs;
	u8 m_psw;

	// execution sequencing
	s32 m_icount;
};

class hpc46003_device : public hpc_device
{
public:
	// construction/destruction
	hpc46003_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void internal_map(address_map &map) ATTR_COLD;
};

class hpc46104_device : public hpc_device
{
public:
	// construction/destruction
	hpc46104_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void internal_map(address_map &map) ATTR_COLD;
};


// device type declarations
DECLARE_DEVICE_TYPE(HPC46003, hpc46003_device)
DECLARE_DEVICE_TYPE(HPC46104, hpc46104_device)

#endif // MAME_CPU_HPC_HPC_H
