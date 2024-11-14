// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VT50_VT50_H
#define MAME_CPU_VT50_VT50_H

#pragma once

class vt5x_cpu_device : public cpu_device, public device_video_interface
{
public:
	enum {
		VT5X_PC, VT5X_PAGE,
		VT5X_MODE, VT5X_DONE,
		VT5X_AC, VT5X_B, VT5X_X, VT5X_Y, VT5X_X8, VT5X_XYAD,
		VT5X_CFF, VT5X_VID
	};

	// callback configuration
	auto baud_9600_callback() { return m_baud_9600_callback.bind(); }
	auto vert_count_callback() { return m_vert_count_callback.bind(); }
	auto uart_rd_callback() { return m_uart_rd_callback.bind(); }
	auto uart_xd_callback() { return m_uart_xd_callback.bind(); }
	auto ur_flag_callback() { return m_ur_flag_callback.bind(); }
	auto ut_flag_callback() { return m_ut_flag_callback.bind(); }
	auto ruf_callback() { return m_ruf_callback.bind(); }
	auto key_up_callback() { return m_key_up_callback.bind(); }
	auto kclk_callback() { return m_kclk_callback.bind(); }
	auto frq_callback() { return m_frq_callback.bind(); }
	auto bell_callback() { return m_bell_callback.bind(); }
	auto cen_callback() { return m_cen_callback.bind(); }
	auto ccf_callback() { return m_ccf_callback.bind(); }
	auto csf_callback() { return m_csf_callback.bind(); }
	auto char_data_callback() { return m_char_data_callback.bind(); }

	// screen update method
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// construction/destruction
	vt5x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int bbits, int ybits);

	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// video helpers
	void draw_char_line();

	// address translation
	offs_t translate_xy() const;

	// execution helpers
	void execute_te(u8 inst);
	void execute_tf(u8 inst);
	virtual void execute_tw(u8 inst);
	virtual void execute_tg(u8 inst) = 0;
	virtual void execute_th(u8 inst);
	void execute_tj(u8 dest);
	void clock_video_counters();

	// address spaces
	address_space_config m_rom_config;
	address_space_config m_ram_config;
	memory_access<10, 0, 0, ENDIANNESS_LITTLE>::cache m_rom_cache;
	memory_access<11, 0, 0, ENDIANNESS_LITTLE>::cache m_ram_cache;

	// device callbacks
	devcb_write_line m_baud_9600_callback;
	devcb_write8 m_vert_count_callback;
	devcb_read8 m_uart_rd_callback;
	devcb_write8 m_uart_xd_callback;
	devcb_read_line m_ur_flag_callback;
	devcb_read_line m_ut_flag_callback;
	devcb_write_line m_ruf_callback;
	devcb_read8 m_key_up_callback;
	devcb_read_line m_kclk_callback;
	devcb_read_line m_frq_callback;
	devcb_write_line m_bell_callback;
	devcb_write_line m_cen_callback;
	devcb_read_line m_csf_callback;
	devcb_read_line m_ccf_callback;
	devcb_read8 m_char_data_callback;

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
	bool m_cursor_active;
	bool m_video_process;
	u8 m_ram_do;

	// execution phases
	u8 m_t;
	bool m_write_ff;
	bool m_flag_test_ff;
	bool m_m2u_ff;
	bool m_bell_ff;
	bool m_load_pc;
	s32 m_icount;

	// video timing
	u8 m_horiz_count;
	u16 m_vert_count;
	bool m_top_of_screen;
	u16 m_current_line;
	u16 m_first_line;

	// display bitmap
	bitmap_rgb32 m_bitmap;
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

	auto graphic_callback() { return m_graphic_callback.bind(); }

protected:
	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void execute_tw(u8 inst) override;
	virtual void execute_tg(u8 inst) override;
	virtual void execute_th(u8 inst) override;

private:
	devcb_write8 m_graphic_callback;
};

DECLARE_DEVICE_TYPE(VT50_CPU, vt50_cpu_device)
DECLARE_DEVICE_TYPE(VT52_CPU, vt52_cpu_device)

#endif // MAME_CPU_VT50_VT50_H
