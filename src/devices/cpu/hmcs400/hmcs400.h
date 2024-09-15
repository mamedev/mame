// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS400 MCU family cores

*/

#ifndef MAME_CPU_HMCS400_HMCS400_H
#define MAME_CPU_HMCS400_HMCS400_H

#pragma once


class hmcs400_cpu_device : public cpu_device
{
public:
	virtual ~hmcs400_cpu_device();

	// configuration helpers

	// system clock divider mask option (only for HMCS408, HMCS414, HMCS424)
	// valid options: 4, 8, 16, default to 8
	auto &set_divider(u8 div) { assert(m_has_div); m_divider = div; return *this; }

protected:
	// construction
	hmcs400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + m_divider - 1) / m_divider; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * m_divider); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2+2; } // max 2 + interrupt
	virtual u32 execute_input_lines() const noexcept override { return 2; }
	//virtual void execute_set_input(int line, int state) override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// memory maps
	void program_map(address_map &map);
	void data_map(address_map &map);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	const u32 m_rom_size; // ROM size in 16-bit words
	const u32 m_ram_size; // RAM size minus the 64-byte stack
	bool m_has_div;       // MCU supports divider mask option
	bool m_has_law;       // MCU supports LAW/LWA opcodes
	u8 m_divider;         // system clock divider

	u16 m_pc;
	u16 m_prev_pc;
	u16 m_op;

	int m_icount;

	// opcode handlers
	void op_illegal();
};


class hmcs402_cpu_device : public hmcs400_cpu_device
{
protected:
	hmcs402_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

class hd614022_device : public hmcs402_cpu_device
{
public:
	hd614022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614025_device : public hmcs402_cpu_device
{
public:
	hd614025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614028_device : public hmcs402_cpu_device
{
public:
	hd614028_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs404_cpu_device : public hmcs400_cpu_device
{
protected:
	hmcs404_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

class hd614042_device : public hmcs404_cpu_device
{
public:
	hd614042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614045_device : public hmcs404_cpu_device
{
public:
	hd614045_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614048_device : public hmcs404_cpu_device
{
public:
	hd614048_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs408_cpu_device : public hmcs400_cpu_device
{
protected:
	hmcs408_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

class hd614080_device : public hmcs408_cpu_device
{
public:
	hd614080_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614085_device : public hmcs408_cpu_device
{
public:
	hd614085_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614088_device : public hmcs408_cpu_device
{
public:
	hd614088_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(HD614022, hd614022_device)
DECLARE_DEVICE_TYPE(HD614025, hd614025_device)
DECLARE_DEVICE_TYPE(HD614028, hd614028_device)

DECLARE_DEVICE_TYPE(HD614042, hd614042_device)
DECLARE_DEVICE_TYPE(HD614045, hd614045_device)
DECLARE_DEVICE_TYPE(HD614048, hd614048_device)

DECLARE_DEVICE_TYPE(HD614080, hd614080_device)
DECLARE_DEVICE_TYPE(HD614085, hd614085_device)
DECLARE_DEVICE_TYPE(HD614088, hd614088_device)

#endif // MAME_CPU_HMCS400_HMCS400_H
