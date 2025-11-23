// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx base emulation

#ifndef MAME_CPU_DSP563XX_DSP563XX_H
#define MAME_CPU_DSP563XX_DSP563XX_H

#pragma once

#include "hi08.h"
#include "shi.h"

enum {
	DSP563XX_PC, DSP563XX_LA, DSP563XX_LC,
	DSP563XX_SR, DSP563XX_EMR, DSP563XX_MR, DSP563XX_CCR, DSP563XX_OMR,
	DSP563XX_SSH, DSP563XX_SSL,
	DSP563XX_A, DSP563XX_B, DSP563XX_X0, DSP563XX_X1, DSP563XX_Y0, DSP563XX_Y1,
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

	void hi08_w(offs_t offset, u8 data);
	u8 hi08_r(offs_t offset);

protected:
	enum {
		CCR_C = 0x01,
		CCR_V = 0x02,
		CCR_Z = 0x04,
		CCR_N = 0x08,
		CCR_U = 0x10,
		CCR_E = 0x20,
		CCR_L = 0x40,
		CCR_S = 0x80,

		MR_S0 = 0x08,
		MR_S1 = 0x10,
		MR_SC = 0x20,
		MR_DM = 0x40,
		MR_LF = 0x80,

		EMR_FV = 0x01,
		EMR_SA = 0x02,
		EMR_CE = 0x08,
		EMR_SM = 0x10,
		EMR_RM = 0x20,
	};

	static const u16 t_ipar[0x100];
	static const u16 t_move[0x10000];
	static const u16 t_npar[0x100000];

	static const u64 t_move_ex[40];
	static const u64 t_npar_ex[72];

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

	optional_device<hi08_device> m_hi08;
	optional_device<dsp5636x_shi_device> m_shi;

	address_space_config m_p_config, m_x_config, m_y_config;

	memory_access<24, 2, -2, ENDIANNESS_LITTLE>::cache m_p;
	memory_access<24, 2, -2, ENDIANNESS_LITTLE>::specific m_x, m_y;

	u64 m_a, m_b;
	u32 m_tmp1, m_tmp2;
	std::array<u32, 16> m_stackh, m_stackl;
	u32 m_pc, m_la, m_lc, m_temp_lc, m_vba;
	u32 m_x0, m_x1, m_y0, m_y1;
	u32 m_ep;
	u32 m_omr;
	u32 m_sp, m_sz;
	std::array<u32, 8> m_r, m_m, m_n;

	u32 m_npc;
	u8 m_sc, m_emr, m_mr, m_ccr, m_hard_omr;
	bool m_skip;
	bool m_rep;
	int m_icount;

	inline void set_a(u64 v) { m_a = v & 0xffffffffffffff; }
	inline void set_ah(u32 v) { set_a(u64(util::sext(v, 24)) << 24); }
	inline void set_af(u8 v) { m_a = u64(v) << 48; }
	inline void set_a2(u8 v) { m_a = (m_a & 0x00ffffffffffff) | (u64(v) << 48); }
	inline void set_a1(u32 v) { m_a = (m_a & 0xff000000ffffff) | (u64(v & 0xffffff) << 24); }
	inline void set_a0(u32 v) { m_a = (m_a & 0xffffffff000000) | u64(v & 0xffffff); }
	inline void set_a10(u64 v) { set_a(util::sext(v, 48)); }
	inline void set_al(u64 v) { set_a(util::sext(v, 48)); }
	inline void set_b(u64 v) { m_b = v & 0xffffffffffffff; }
	inline void set_bh(u32 v) { set_b(u64(util::sext(v, 24)) << 24); }
	inline void set_bf(u64 v) { m_b = u64(v) << 48; }
	inline void set_b2(u8 v) { m_b = (m_b & 0x00ffffffffffff) | (u64(v) << 48); }
	inline void set_b1(u32 v) { m_b = (m_b & 0xff000000ffffff) | (u64(v & 0xffffff) << 24); }
	inline void set_b0(u32 v) { m_b = (m_b & 0xffffffff000000) | u64(v & 0xffffff); }
	inline void set_b10(u64 v) { set_b(util::sext(v, 48)); }
	inline void set_bl(u64 v) { set_b(util::sext(v, 48)); }
	inline void set_ab(u64 v) { set_ah(v >> 24); set_bh(v); }
	inline void set_ba(u64 v) { set_bh(v >> 24); set_ah(v); }
	inline void set_x0(u32 v) { m_x0 = v & 0xffffff; }
	inline void set_x0f(u8 v) { m_x0 = u32(v) << 16; }
	inline void set_x1(u32 v) { m_x1 = v & 0xffffff; }
	inline void set_x1f(u8 v) { m_x1 = u32(v) << 16; }
	inline void set_y0(u32 v) { m_y0 = v & 0xffffff; }
	inline void set_y0f(u8 v) { m_y0 = u32(v) << 16; }
	inline void set_y1(u32 v) { m_y1 = v & 0xffffff; }
	inline void set_y1f(u8 v) { m_y1 = u32(v) << 16; }
	inline void set_x(u64 v) { m_x1 = v >> 24; m_x0 = v & 0xffffff; }
	inline void set_y(u64 v) { m_y1 = v >> 24; m_y0 = v & 0xffffff; }
	inline void set_r(int index, u32 v) { m_r[index] = v & 0xffffff; }
	inline void set_n(int index, u32 v) { m_n[index] = v & 0xffffff; }
	inline void set_m(int index, u32 v) { m_m[index] = v & 0xffffff; }
	inline void set_ep(u32 v) { m_ep = v; }
	inline void set_la(u32 v) { m_la = v; }
	inline void set_lc(u32 v) { m_lc = v & 0xffffff; }
	inline void set_omr(u32 v) { m_omr = v; }
	inline void set_sc(u32 v) { m_sc = v; }
	inline void set_sp(u32 v) { m_sp = v; }
	inline void set_sr(u32 v) { m_emr = v >> 16; m_mr = v >> 8; m_ccr = v; }
	inline void set_ssh(u32 v) { m_stackh[m_sp] = v; }
	inline void set_ssl(u32 v) { m_stackl[m_sp] = v; }
	inline void set_sz(u32 v) { m_sz = v; }
	inline void set_vba(u32 v) { m_vba = v; }
	inline void set_mr(u8 v) { m_mr = v; }
	inline void set_ccr(u8 v) { m_ccr = v; }
	inline void set_com(u8 v) { m_omr = (m_omr & 0xffff00) | v; }
	inline void set_eom(u8 v) { m_omr = (m_omr & 0xff00ff) | (v << 8); }

	inline u64 get_a() const { return m_a; }
	inline u32 get_ah() const { return std::clamp(s32(m_a >> 24), -0x00800000, 0x007fffff) & 0xffffff; }
	inline u32 get_a2() const { return m_a >> 48; }
	inline u32 get_a1() const { return (m_a >> 24) & 0xffffff; }
	inline u32 get_a0() const { return m_a & 0xffffff; }
	inline u64 get_a10() const { return m_a & 0xffffffffffff; }
	inline u64 get_al() const { return m_a & 0xffffffffffff; }
	inline u64 get_b() const { return m_b; }
	inline u32 get_bh() const { return std::clamp(s32(m_b >> 24), -0x00800000, 0x007fffff) & 0xffffff; }
	inline u32 get_b2() const { return m_b >> 48; }
	inline u32 get_b1() const { return (m_b >> 24) & 0xffffff; }
	inline u32 get_b0() const { return m_b & 0xffffff; }
	inline u64 get_b10() const { return m_b & 0xffffffffffff; }
	inline u64 get_bl() const { return m_b & 0xffffffffffff; }
	inline u64 get_ab() const { return u32(get_ah()) << 24 | get_bh(); }
	inline u64 get_ba() const { return u32(get_bh()) << 24 | get_ah(); }
	inline u32 get_x0() const { return m_x0; }
	inline u32 get_x1() const { return m_x1; }
	inline u64 get_x() const { return u64(m_x1) << 24 | m_x0; }
	inline u32 get_y0() const { return m_y0; }
	inline u32 get_y1() const { return m_y1; }
	inline u64 get_y() const { return u64(m_y1) << 24 | m_y0; }
	inline u32 get_r(int index) const { return m_r[index]; }
	inline u32 get_n(int index) const { return m_n[index]; }
	inline u32 get_m(int index) const { return m_m[index]; }
	inline u32 get_ep() const { return m_ep; }
	inline u32 get_la() const { return m_la; }
	inline u32 get_lc() const { return m_lc; }
	inline u32 get_omr() const { return m_omr; }
	inline u32 get_sc() const { return m_sc; }
	inline u32 get_sp() const { return m_sp; }
	inline u32 get_sr() const { return (m_emr << 16) | (m_mr << 8) | m_ccr; }
	inline u32 get_ssh() const { return m_stackh[m_sp]; }
	inline u32 get_ssl() const { return m_stackl[m_sp]; }
	inline u32 get_sz() const { return m_sz; }
	inline u32 get_vba() const { return m_vba; }
	inline u8 get_mr() const { return m_mr; }
	inline u8 get_ccr() const { return m_mr; }
	inline u8 get_com() const { return m_omr; }
	inline u8 get_eom() const { return m_omr >> 8; }

	inline bool test_cc() const { return (m_ccr & CCR_C) == 0; }
	inline bool test_ge() const { return (m_ccr & (CCR_N|CCR_V)) == 0 || (m_ccr & (CCR_N|CCR_V)) == (CCR_N|CCR_V); }
	inline bool test_ne() const { return (m_ccr & CCR_Z) == 0; }
	inline bool test_pl() const { return (m_ccr & CCR_N) == 0; }
	inline bool test_nn() const { return (m_ccr & CCR_Z) == 0 && (m_ccr & (CCR_U|CCR_E)) != 0; }
	inline bool test_ec() const { return (m_ccr & CCR_E) == 0; }
	inline bool test_lc() const { return (m_ccr & CCR_L) == 0; }
	inline bool test_gt() const { return (m_ccr & CCR_Z) == 0 && ((m_ccr & (CCR_N|CCR_V)) == 0 || (m_ccr & (CCR_N|CCR_V)) == (CCR_N|CCR_V)); }
	inline bool test_cs() const { return (m_ccr & CCR_C) == CCR_C; }
	inline bool test_lt() const { return (m_ccr & (CCR_N|CCR_V)) == CCR_V || (m_ccr & (CCR_N|CCR_V)) == CCR_N; }
	inline bool test_eq() const { return (m_ccr & CCR_Z) == CCR_Z; }
	inline bool test_mi() const { return (m_ccr & CCR_N) == CCR_N; }
	inline bool test_nr() const { return (m_ccr & CCR_Z) == CCR_Z || (m_ccr & (CCR_U|CCR_E)) == 0; }
	inline bool test_es() const { return (m_ccr & CCR_E) == CCR_E; }
	inline bool test_ls() const { return (m_ccr & CCR_L) == CCR_L; }
	inline bool test_le() const { return (m_ccr & CCR_Z) == CCR_Z || (m_ccr & (CCR_N|CCR_V)) == CCR_V || (m_ccr & (CCR_N|CCR_V)) == CCR_N; }

	inline void inc_sp() { m_sp++; }
	inline void dec_sp() { m_sp--; }

	inline u32 calc_add_r(int reg, s32 delta) {
		u16 mode = m_m[reg];
		if(mode == 0xffff)
			return (m_r[reg] + delta) & 0xffffff;
		logerror("Unhandled mode %04x\n", mode);
		return 0;
	}

	inline void add_r(int reg, s32 delta) { m_r[reg] = calc_add_r(reg, delta); }

	u64 do_add56(u64 v1, u64 v2);
	u64 do_sub56(u64 v1, u64 v2);
	u64 do_asl56(u8 n, u64 v);
	u64 do_asr56(u8 n, u64 v);
	void do_tst56(u64 v);
};

#endif
