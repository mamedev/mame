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
		F_V = 8
	};
		
	static const u8 bpo[8];

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual uint32_t execute_input_lines() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	const address_space_config m_program_config;
	address_space *m_program;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_program_cache;

	int m_icount;
	u32 m_pc;
	u32 m_r[8];
	u8 m_f;

	static inline int r1(u32 opcode) { return (opcode >> 22) & 7; }
	static inline int r2(u32 opcode) { return (opcode >> 19) & 7; }
	static inline int r3(u32 opcode) { return (opcode >> 16) & 7; }

	static inline u32 val24u(u32 opcode) { return opcode & 0x00ffffff; }
	static inline u32 val22s(u32 opcode) { return opcode & 0x200000 ? opcode | 0xffc00000 : opcode & 0x3fffff; }
	static inline u32 val19u(u32 opcode) { return opcode & 0x0007ffff; }
	static inline u32 val16u(u32 opcode) { return static_cast<u16>(opcode); }
	static inline u32 val16s(u32 opcode) { return static_cast<s16>(opcode); }
	static inline u32 val14h(u32 opcode) { return (opcode << 10) & 0xfffc0000; }
	static inline u32 val14u(u32 opcode) { return (opcode >>  8) & 0x00003fff; }
	static inline u32 val14s(u32 opcode) { return opcode & 0x200000 ? (opcode >> 8) | 0xffffc000 : (opcode >> 8) & 0x3fff; }
	static inline u32 val11s(u32 opcode) { return opcode & 0x40000 ? (opcode >> 8) | 0xfffff800 : (opcode >> 8) & 0x7ff; }
	static inline u32 val11u(u32 opcode) { return (opcode >>  8) & 0x000007ff; }
	static inline u32 val8s(u32 opcode)  { return static_cast<s8>(opcode >> 16); }
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
		m_f = f;
		return r;
	}

	inline u32 do_sub(u32 v1, u32 v2) {
		u32 r = v1 - v2;
		u32 f = 0;
		if(!r)
			f |= F_Z;
		if(r & 0x80000000)
			f |= F_N;
		if(((v1 & r) | ((~v2) & (v1 | r))) & 0x80000000)
			f |= F_C;
		if(((v1 ^ v2) & (v2 ^ r)) & 0x80000000)
			f |= F_V;
		m_f = f;
		return r;
	}

	inline u32 do_or(u32 v1, u32 v2) {
		u32 r = v1 | v2;
		u32 f = 0;
		if(!r)
			f |= F_Z;
		if(r & 0x80000000)
			f |= F_N;
		m_f = f;
		return r;		
	}

	inline u32 do_and(u32 v1, u32 v2) {
		u32 r = v1 & v2;
		u32 f = 0;
		if(!r)
			f |= F_Z;
		if(r & 0x80000000)
			f |= F_N;
		m_f = f;
		return r;		
	}

	inline u32 do_lsl(u32 v1, u32 shift) {
		if(!shift) {
			m_f = v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z;
			return v1;
		} else if(shift < 32) {
			u32 r = v1 << shift;
			u32 f = v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z;
			if(v1 & (1 << (32-shift)))
				f |= F_C;
			m_f = f;
			return r;
		} else if(shift == 32) {
			m_f = v1 & 1 ? F_C|F_Z : F_Z;
			return 0;
		} else {
			m_f = F_Z;
			return 0;
		}
	}

	inline u32 do_lsr(u32 v1, u32 shift) {
		if(!shift) {
			m_f = v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z;
			return v1;
		} else if(shift < 32) {
			u32 r = v1 >> shift;
			u32 f = v1 ? 0 : F_Z;
			if(v1 & (1 << (shift - 1)))
				f |= F_C;
			m_f = f;
			return r;
		} else if(shift == 32) {
			m_f = v1 & 0x80000000 ? F_C|F_Z : F_Z;
			return 0;
		} else {
			m_f = F_Z;
			return 0;
		}
	}

	inline u32 do_asr(u32 v1, u32 shift) {
		if(!shift) {
			m_f = v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z;
			return v1;
		} else if(shift < 32) {
			u32 r = static_cast<s32>(v1) >> shift;
			u32 f = v1 ? v1 & 0x80000000 ? F_N : 0 : F_Z;
			if(v1 & (1 << (shift - 1)))
				f |= F_C;
			m_f = f;
			return r;
		} else {
			if(v1 & 0x80000000) {
				m_f = F_C;
				return 0xffffffff;
			} else {
				m_f = F_Z;
				return 0;
			}
		}
	}
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
	XAVIX2_LR
};

DECLARE_DEVICE_TYPE(XAVIX2, xavix2_device)

#endif /* MAME_CPU_XAVIX2_XAVIX2_H */
