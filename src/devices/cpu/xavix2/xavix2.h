// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Nathan Gilbert

#ifndef MAME_CPU_XAVIX2_XAVIX2_H
#define MAME_CPU_XAVIX2_XAVIX2_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class xavix2_device : public cpu_device
{
public:
	xavix2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	enum {
		F_Z = 1,
		F_N = 2,
		F_C = 4,
		F_V = 8,
		F_MASK = 15,
		F_I = 16
	};

	static const u8 bpo[8];

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
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_program_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;

	int m_icount, m_ei_count;
	u32 m_pc;
	u32 m_r[8], m_hr[64];
	u32 m_ilr1;
	u8 m_if1;

	bool m_int_line, m_wait;

	static inline int r1(u32 opcode) { return (opcode >> 22) & 7; }
	static inline int r2(u32 opcode) { return (opcode >> 19) & 7; }
	static inline int r3(u32 opcode) { return (opcode >> 16) & 7; }

	static inline u32 val24u(u32 opcode) { return opcode & 0x00ffffff; }
	static inline u32 val22h(u32 opcode) { return opcode << 10; }
	static inline u32 val22s(u32 opcode) { return opcode & 0x200000 ? opcode | 0xffc00000 : opcode & 0x3fffff; }
	static inline u32 val19s(u32 opcode) { return opcode & 0x40000 ? opcode | 0xfff80000 : opcode & 0x7ffff; }
	static inline u32 val19u(u32 opcode) { return opcode & 0x0007ffff; }
	static inline u32 val16s(u32 opcode) { return static_cast<s16>(opcode >> 8); }
	static inline u32 val14h(u32 opcode) { return (opcode << 10) & 0xfffc0000; }
	static inline u32 val14u(u32 opcode) { return (opcode >>  8) & 0x00003fff; }
	static inline u32 val14s(u32 opcode) { return opcode & 0x200000 ? (opcode >> 8) | 0xffffc000 : (opcode >> 8) & 0x3fff; }
	static inline u32 val11s(u32 opcode) { return opcode & 0x40000 ? (opcode >> 8) | 0xfffff800 : (opcode >> 8) & 0x7ff; }
	static inline u32 val11u(u32 opcode) { return (opcode >>  8) & 0x000007ff; }
	static inline u32 val8s(u32 opcode)  { return static_cast<s8>(opcode >> 16); }
	static inline u32 val6u(u32 opcode)  { return (opcode >> 16) & 0x0000003f; }
	static inline u32 val6s(u32 opcode)  { return opcode & 0x200000 ? (opcode >> 16) | 0xffffffc0 : (opcode >> 16) & 0x3f; }
	static inline u32 val3u(u32 opcode)  { return (opcode >> 16) & 0x00000007; }
	static inline u32 val3s(u32 opcode)  { return opcode & 0x40000 ? (opcode >> 16) | 0xfffffff8 : (opcode >> 16) & 0x7; }

	inline u32 do_add(u32 v1, u32 v2) {
		u32 r = v1 + v2;
		u32 f = 0;
		if(!r)
			f |= F_Z;
		if(r & 0x80000000)
			f |= F_N;
		if(((v1 & v2) | ((~r) & (v1 | v2))) & 0x80000000)
			f |= F_C;
		if(((v1 ^ r) & (v2 ^ r)) & 0x80000000)
			f |= F_V;
		m_hr[4] = (m_hr[4] & ~F_MASK) | f;
		return r;
	}

	inline u32 do_sub(u32 v1, u32 v2) {
		u32 r = v1 - v2;
		u32 f = 0;
		if(!r)
			f |= F_Z;
		if(r & 0x80000000)
			f |= F_N;
		if(((v2 & r) | ((~v1) & (v2 | r))) & 0x80000000)
			f |= F_C;
		if(((v1 ^ v2) & (v1 ^ r)) & 0x80000000)
			f |= F_V;
		m_hr[4] = (m_hr[4] & ~F_MASK) | f;
		return r;
	}

	inline u32 snz(u32 r) {
		u32 f = 0;
		if(!r)
			f |= F_Z;
		if(r & 0x80000000)
			f |= F_N;
		m_hr[4] = (m_hr[4] & ~F_MASK) | f;
		return r;
	}

	inline u32 do_lsl(u32 v1, u32 shift) {
		if(!shift) {
			m_hr[4] = (m_hr[4] & ~F_MASK) | (v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z);
			return v1;
		} else if(shift < 32) {
			u32 r = v1 << shift;
			u32 f = r ? r & 0x80000000 ? F_N : 0 : F_Z;
			if(v1 & (1 << (32-shift)))
				f |= F_C;
			m_hr[4] = (m_hr[4] & ~F_MASK) | f;
			return r;
		} else if(shift == 32) {
			m_hr[4] = (m_hr[4] & ~F_MASK) | (v1 & 1 ? F_C|F_Z : F_Z);
			return 0;
		} else {
			m_hr[4] = (m_hr[4] & ~F_MASK) | F_Z;
			return 0;
		}
	}

	inline u32 do_lsr(u32 v1, u32 shift) {
		if(!shift) {
			m_hr[4] = (m_hr[4] & ~F_MASK) | (v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z);
			return v1;
		} else if(shift < 32) {
			u32 r = v1 >> shift;
			u32 f = r ? 0 : F_Z;
			if(v1 & (1 << (shift - 1)))
				f |= F_C;
			m_hr[4] = (m_hr[4] & ~F_MASK) | f;
			return r;
		} else if(shift == 32) {
			m_hr[4] = (m_hr[4] & ~F_MASK) | (v1 & 0x80000000 ? F_C|F_Z : F_Z);
			return 0;
		} else {
			m_hr[4] = (m_hr[4] & ~F_MASK) | F_Z;
			return 0;
		}
	}

	inline u32 do_asr(u32 v1, u32 shift) {
		if(!shift) {
			m_hr[4] = (m_hr[4] & ~F_MASK) | (v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z);
			return v1;
		} else if(shift < 32) {
			u32 r = static_cast<s32>(v1) >> shift;
			u32 f = r ? r & 0x80000000 ? F_N : 0 : F_Z;
			if(v1 & (1 << (shift - 1)))
				f |= F_C;
			m_hr[4] = (m_hr[4] & ~F_MASK) | f;
			return r;
		} else {
			if(v1 & 0x80000000) {
				m_hr[4] = (m_hr[4] & ~F_MASK) | F_C;
				return 0xffffffff;
			} else {
				m_hr[4] = (m_hr[4] & ~F_MASK) | F_Z;
				return 0;
			}
		}
	}

	u32 check_interrupt(u32 npc);
};

enum {
	XAVIX2_PC,
	XAVIX2_FLAGS,
	XAVIX2_R0,
	XAVIX2_R1,
	XAVIX2_R2,
	XAVIX2_R3,
	XAVIX2_R4,
	XAVIX2_R5,
	XAVIX2_SP,
	XAVIX2_LR,
	XAVIX2_ILR1,
	XAVIX2_IF1,

	XAVIX2_HR00,
	XAVIX2_HR01,
	XAVIX2_HR02,
	XAVIX2_HR03,
	XAVIX2_HR04,
	XAVIX2_HR05,
	XAVIX2_HR06,
	XAVIX2_HR07,
	XAVIX2_HR08,
	XAVIX2_HR09,
	XAVIX2_HR0A,
	XAVIX2_HR0B,
	XAVIX2_HR0C,
	XAVIX2_HR0D,
	XAVIX2_HR0E,
	XAVIX2_HR0F,

	XAVIX2_HR10,
	XAVIX2_HR11,
	XAVIX2_HR12,
	XAVIX2_HR13,
	XAVIX2_HR14,
	XAVIX2_HR15,
	XAVIX2_HR16,
	XAVIX2_HR17,
	XAVIX2_HR18,
	XAVIX2_HR19,
	XAVIX2_HR1A,
	XAVIX2_HR1B,
	XAVIX2_HR1C,
	XAVIX2_HR1D,
	XAVIX2_HR1E,
	XAVIX2_HR1F,

	XAVIX2_HR20,
	XAVIX2_HR21,
	XAVIX2_HR22,
	XAVIX2_HR23,
	XAVIX2_HR24,
	XAVIX2_HR25,
	XAVIX2_HR26,
	XAVIX2_HR27,
	XAVIX2_HR28,
	XAVIX2_HR29,
	XAVIX2_HR2A,
	XAVIX2_HR2B,
	XAVIX2_HR2C,
	XAVIX2_HR2D,
	XAVIX2_HR2E,
	XAVIX2_HR2F,

	XAVIX2_HR30,
	XAVIX2_HR31,
	XAVIX2_HR32,
	XAVIX2_HR33,
	XAVIX2_HR34,
	XAVIX2_HR35,
	XAVIX2_HR36,
	XAVIX2_HR37,
	XAVIX2_HR38,
	XAVIX2_HR39,
	XAVIX2_HR3A,
	XAVIX2_HR3B,
	XAVIX2_HR3C,
	XAVIX2_HR3D,
	XAVIX2_HR3E,
	XAVIX2_HR3F
};

DECLARE_DEVICE_TYPE(XAVIX2, xavix2_device)

#endif /* MAME_CPU_XAVIX2_XAVIX2_H */
