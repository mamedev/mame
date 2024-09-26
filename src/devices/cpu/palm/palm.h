// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_PALM_PALM_H
#define MAME_CPU_PALM_PALM_H

#pragma once

class palm_device : public cpu_device
{
public:
	// interrupt lines are numbered 1..3
	static unsigned constexpr IRPT_REQ1 = INPUT_LINE_IRQ0;
	static unsigned constexpr IRPT_REQ2 = INPUT_LINE_IRQ1;
	static unsigned constexpr IRPT_REQ3 = INPUT_LINE_IRQ2;

	// four address spaces
	static unsigned constexpr AS_PGM = AS_PROGRAM;
	static unsigned constexpr AS_RWS = AS_DATA;
	static unsigned constexpr AS_IOC = AS_IO;
	static unsigned constexpr AS_IOD = 4;

	auto getb_bus() { return m_getb_bus.bind(); }
	auto program_level() { return m_program_level.bind(); }
	auto select_ros() { return m_select_ros.bind(); }

	palm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 17; }
	virtual u32 execute_max_cycles() const noexcept override { return 54; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	bool condition(unsigned const modifier, u16 const data, u8 const mask) const;
	s16 modifier(unsigned const modifier) const;

	void control(u8 data);

private:
	// address spaces
	address_space_config const m_pgm_config;
	address_space_config const m_rws_config;
	address_space_config const m_ioc_config;
	address_space_config const m_iod_config;

	memory_access<16, 1, 0, ENDIANNESS_BIG>::specific m_pgm;
	memory_access<16, 1, 0, ENDIANNESS_BIG>::specific m_rws;
	memory_access<4, 0, 0, ENDIANNESS_BIG>::specific m_ioc;
	memory_access<4, 0, 0, ENDIANNESS_BIG>::specific m_iod;

	devcb_write8 m_getb_bus;
	devcb_write_line m_program_level;
	devcb_write_line m_select_ros;

	// mame state
	int m_icount;
	u16 m_pc;

	u16 m_r[4][16];  // registers
	u8 m_il;         // interrupt level
	u8 m_ff;         // controller flip-flops
};

DECLARE_DEVICE_TYPE(PALM, palm_device)

#endif // MAME_CPU_PALM_PALM_H
