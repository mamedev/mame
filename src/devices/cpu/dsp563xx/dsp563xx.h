// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx base emulation

#ifndef MAME_CPU_DSP563XX_DSP563XX_H
#define MAME_CPU_DSP563XX_DSP563XX_H

#pragma once

class dsp563xx_device : public cpu_device {
public:
	enum {
		AS_P = AS_PROGRAM,
		AS_X = AS_DATA,
		AS_Y = AS_IO,
	};

	void set_hard_omr(u8 mode);

protected:
	dsp563xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
					address_map_constructor map_p, address_map_constructor map_x, address_map_constructor map_y);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 5; }
	virtual void execute_run() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 get_reset_vector() const = 0;

	address_space_config m_p_config, m_x_config, m_y_config;

	memory_access<24, 2, -2, ENDIANNESS_LITTLE>::cache m_p;
	memory_access<24, 2, -2, ENDIANNESS_LITTLE>::specific m_x, m_y;

	u32 m_pc;
	u8 m_omr, m_hard_omr;
	int m_icount;
};

#endif
