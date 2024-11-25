// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Carne

// KS0164 core

#ifndef MAME_CPU_KS0164_KS0164_H
#define MAME_CPU_KS0164_KS0164_H

#pragma once

class ks0164_cpu_device : public cpu_device
{
public:
	ks0164_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	enum {
		R_SP = 4,
		R_PSW = 5,
		R_PC = 6,
		R_ZERO = 7
	};

	enum {
		F_MASK = 0xf0,

		F_Z = 0x10,
		F_C = 0x20,
		F_N = 0x40,
		F_V = 0x80,

		F_I = 0x8000
	};

	static const u16 imask[16];

	int m_icount;
	u16 m_r[8];
	u32 m_irq;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	const address_space_config m_program_config;
	memory_access<16, 1, 0, ENDIANNESS_BIG>::cache m_program_cache;
	memory_access<16, 1, 0, ENDIANNESS_BIG>::specific m_program;

	void handle_irq();
	u16 snz(u16 r);
	void do_alu(u16 opcode, u16 v);
	void unk(u16 opcode);
};

enum {
	KS0164_R0,
	KS0164_R1,
	KS0164_R2,
	KS0164_R3,
	KS0164_SP,
	KS0164_PSW,
	KS0164_PC,
};

DECLARE_DEVICE_TYPE(KS0164CPU, ks0164_cpu_device)

#endif
