// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx base emulation

#ifndef MAME_CPU_DSP563XX_DSP563XX_H
#define MAME_CPU_DSP563XX_DSP563XX_H

#pragma once

enum {
	DSP563XX_PC, DSP563XX_A, DSP563XX_B, DSP563XX_X0, DSP563XX_X1, DSP563XX_Y0, DSP563XX_Y1,
	DSP563XX_R0, DSP563XX_R1, DSP563XX_R2, DSP563XX_R3, DSP563XX_R4, DSP563XX_R5, DSP563XX_R6, DSP563XX_R7,
	DSP563XX_N0, DSP563XX_N1, DSP563XX_N2, DSP563XX_N3, DSP563XX_N4, DSP563XX_N5, DSP563XX_N6, DSP563XX_N7,
	DSP563XX_M0, DSP563XX_M1, DSP563XX_M2, DSP563XX_M3, DSP563XX_M4, DSP563XX_M5, DSP563XX_M6, DSP563XX_M7,
};

class dsp563xx_device : public cpu_device {
public:
	enum {
		AS_P = AS_PROGRAM,
		AS_X = AS_DATA,
		AS_Y = AS_IO,
	};

	void set_hard_omr(u8 mode);

protected:
	enum {
		F_C = 0x01,
		F_V = 0x02,
		F_Z = 0x04,
		F_N = 0x08,
		F_U = 0x10,
		F_E = 0x20,
		F_L = 0x40,
		F_S = 0x80,
	};

	static const u16 t_ipar[0x100];
	static const u16 t_move[0x10000];
	static const u16 t_npar[0x100000];

	static const u64 t_move_ex[39];
	static const u64 t_npar_ex[71];
		
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

	void execute_ipar(u16 kipar);
	void execute_pre_move(u16 kmove, u32 opcode, u32 exv);
	void execute_post_move(u16 kmove, u32 opcode, u32 exv);
	void execute_npar(u16 knpar, u32 opcode, u32 exv);

	void unhandled(const char *inst);

	address_space_config m_p_config, m_x_config, m_y_config;

	memory_access<24, 2, -2, ENDIANNESS_LITTLE>::cache m_p;
	memory_access<24, 2, -2, ENDIANNESS_LITTLE>::specific m_x, m_y;

	u64 m_a, m_b;
	std::array<u64, 16> m_stack;
	u32 m_pc, m_la, m_lc, m_vba;
	u32 m_x0, m_x1, m_y0, m_y1;
	u32 m_ep;
	u32 m_omr;
	u32 m_ssh, m_ssl, m_sp, m_sz;
	std::array<u32, 8> m_r, m_m, m_n;

	u32 m_npc;
	u8 m_sc, m_emr, m_mr, m_ccr, m_hard_omr;
	bool m_skip;
	int m_icount;
};

#endif
