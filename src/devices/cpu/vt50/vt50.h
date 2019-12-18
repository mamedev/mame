// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VT50_VT50_H
#define MAME_CPU_VT50_VT50_H

#pragma once

class vt5x_cpu_device : public cpu_device
{
public:
	enum {
		VT5X_PC, VT5X_BANK,
		VT5X_MODE, VT5X_DONE,
		VT5X_AC, VT5X_B, VT5X_X, VT5X_Y, VT5X_X8,
		VT5X_CFF, VT5X_VID
	};

protected:
	// construction/destruction
	vt5x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int bbits, int ybits);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	// execution helpers
	void execute_te(u8 inst);
	void execute_tf(u8 inst);
	void execute_tw(u8 inst);
	void execute_tg(u8 inst);
	void execute_th(u8 inst);
	void execute_tj(u8 dest);

	// address spaces
	address_space_config m_rom_config;
	address_space_config m_ram_config;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_rom_cache;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_ram_cache;

	// register dimensions
	const u8 m_bbits;
	const u8 m_ybits;

	// internal state
	u16 m_pc;
	u8 m_rom_bank;
	bool m_mode_ff;
	bool m_done_ff;
	u8 m_ac;
	u8 m_buffer;
	u8 m_x;
	u8 m_y;
	bool m_x8;
	bool m_cursor_ff;
	bool m_video_process;

	// execution phases
	u8 m_t;
	bool m_write_ff;
	bool m_flag_test_ff;
	bool m_load_pc;
	bool m_qa_e23;
	s32 m_icount;
};

class vt50_cpu_device : public vt5x_cpu_device
{
public:
	// device type constructor
	vt50_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class vt52_cpu_device : public vt5x_cpu_device
{
public:
	// device type constructor
	vt52_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

DECLARE_DEVICE_TYPE(VT50_CPU, vt50_cpu_device)
DECLARE_DEVICE_TYPE(VT52_CPU, vt52_cpu_device)

#endif // MAME_CPU_VT50_VT50_H
