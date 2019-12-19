// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VT50_VT50_H
#define MAME_CPU_VT50_VT50_H

#pragma once

class vt5x_cpu_device : public cpu_device
{
public:
	enum {
		VT5X_PC, VT5X_PAGE,
		VT5X_MODE, VT5X_DONE,
		VT5X_AC, VT5X_B, VT5X_X, VT5X_Y, VT5X_X8,
		VT5X_CFF, VT5X_VID
	};

	// callback configuration
	auto uart_rd_callback() { return m_uart_rd_callback.bind(); }
	auto uart_xd_callback() { return m_uart_xd_callback.bind(); }
	auto ur_flag_callback() { return m_ur_flag_callback.bind(); }
	auto ut_flag_callback() { return m_ut_flag_callback.bind(); }
	auto ruf_callback() { return m_ruf_callback.bind(); }
	auto key_up_callback() { return m_key_up_callback.bind(); }
	auto bell_callback() { return m_bell_callback.bind(); }

protected:
	// construction/destruction
	vt5x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int bbits, int ybits);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address translation
	offs_t translate_xy() const;

	// execution helpers
	void execute_te(u8 inst);
	void execute_tf(u8 inst);
	virtual void execute_tg(u8 inst) = 0;
	void execute_tw(u8 inst);
	void execute_th(u8 inst);
	void execute_tj(u8 dest);

	// address spaces
	address_space_config m_rom_config;
	address_space_config m_ram_config;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_rom_cache;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_ram_cache;

	// device callbacks
	devcb_read8 m_uart_rd_callback;
	devcb_write8 m_uart_xd_callback;
	devcb_read_line m_ur_flag_callback;
	devcb_read_line m_ut_flag_callback;
	devcb_write_line m_ruf_callback;
	devcb_read8 m_key_up_callback;
	devcb_write_line m_bell_callback;

	// register dimensions
	const u8 m_bbits;
	const u8 m_ybits;

	// internal state
	u16 m_pc;
	u8 m_rom_page;
	bool m_mode_ff;
	bool m_done_ff;
	u8 m_ac;
	u8 m_buffer;
	u8 m_x;
	u8 m_y;
	bool m_x8;
	bool m_cursor_ff;
	bool m_video_process;
	u8 m_ram_do;

	// execution phases
	u8 m_t;
	bool m_write_ff;
	bool m_flag_test_ff;
	bool m_m2u_ff;
	bool m_bell_ff;
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

	virtual void execute_tg(u8 inst) override;
};

class vt52_cpu_device : public vt5x_cpu_device
{
public:
	// device type constructor
	vt52_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void execute_tg(u8 inst) override;
};

DECLARE_DEVICE_TYPE(VT50_CPU, vt50_cpu_device)
DECLARE_DEVICE_TYPE(VT52_CPU, vt52_cpu_device)

#endif // MAME_CPU_VT50_VT50_H
