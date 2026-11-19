// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dsp3210ops.hxx
    Instruction handlers for the DSP3210 core.  Included by dsp3210.cpp
    ahead of the generated dispatch table (dsp3210tbl.hxx), which takes
    the address of every handler named here.

    Handler naming follows the manual's format numbers:
      goto_c<C>        0b/1b  if (C) goto {N, rB, rB+N}; nop, return, ireturn
      dec_goto         3a     if (rM-- >= 0) goto
      do_imm / do_reg  3b/3c  do, dolock, doblock
      call             4a     call {N, rB, rB+N} (rM)
      shift_or         4b     rD = rS <<| N
      add_si<L>        5a/5b  rD = [(short)] rS3 + N
      alu_rr<F, L>     6a/6b  [if (C)] rD = [(short)] rS1 F rS2
      alu_ri<F, L>     6c/6d  rD = [(short)] rD F N
      move_direct      7a     rH <-> *L (on-chip window)
      move_ind         7b/7c  rH <-> ior (bit 10 set) / rH <-> *rP++rI
      move_ior_mem     7d     ior <-> *rP++rI
      goto24/load24/call24  8a/8b/8c
      da14<M, NegA, NegP, Tap>  DA formats 1 and 4 (the adder input is Y)
      da23<M, NegA, NegP, Tap>  DA formats 2 and 3 (the product is Y * X)
      da5<G>                    DA format 5

    The CA handlers perform all their reads before committing any
    post-modify or register write, so an aborted CA instruction changes
    nothing.  A DA handler post-modifies X's pointer before it reads Y and
    writes the accumulator and the flags before its Z store checks the
    alignment, so an address or bus error on Y or Z leaves those earlier
    updates in place; the manual (7.5.2) says only that the instruction
    is aborted.  DA memory stores go through the deferred ring.

    Semantics are from the manual's chapter 4 instruction pages and
    Table 4-1 (flag rules); where the manual is silent or wrong the
    behaviour established from Apple's shipped code is noted in place.

***************************************************************************/

//**************************************************************************
//  DIAGNOSTICS
//**************************************************************************

// the seven architected illegal opcode patterns (manual 7.5.3.2): the
// non-maskable Illegal Opcode error, vector 2
void dsp3210_device::op_illegal(uint32_t op)
{
	take_error(2);
}

// every other reserved encoding executes as a no-op: only the seven
// patterns above are listed as illegal
void dsp3210_device::op_reserved(uint32_t op)
{
	LOGMASKED(LOG_RESERVED, "%s: reserved encoding %08x\n", machine().describe_context(), op);
}


//**************************************************************************
//  REGISTER FILE
//**************************************************************************

// 5-bit register codes: 0 r0, 1-14 r1-r14, 15 pc, 16 reserved, 17-21
// r15-r19, 22 -n, 23 +n, 24-26 r20-r22, 27-29 reserved, 30 pcsh, 31
// reserved.  pc reads the address of the instruction after the delay
// slot (this instruction + 8, the value call links); the +-n
// pseudo-operands read as +-1 in ALU operand positions, the callers
// scale them where they post-modify pointers.
inline uint32_t dsp3210_device::reg_r(int code) const
{
	switch (code)
	{
		case RC_PC:    return m_ppc + 8;
		case RC_MINUS: return 0xffffffff;
		case RC_PLUS:  return 1;
		default:       return m_r[code];    // r0, code 16 and the reserved codes are never written
	}
}

// r0, the pseudo-operands and the reserved codes discard writes; pcsh is
// writable (the boot ROM's system boot depends on it); a write to the pc
// code is in no replacement table and acts as a branch
inline void dsp3210_device::reg_w(int code, uint32_t data)
{
	if (code == RC_PC)
	{
		m_npc = data;
	}
	else if (BIT(WRITEABLE_REGS, code))
	{
		m_r[code] = data;
	}
}


//**************************************************************************
//  CAU FLAGS AND ARITHMETIC (Table 4-1)
//**************************************************************************

inline void dsp3210_device::set_flags(bool n, bool z, bool v, bool c)
{
	m_nzcflags = (uint64_t(c) << 32) | (n ? 0x80000000 : (z ? 0 : 1));
	m_vflags = v ? 0x80000000 : 0;
}

// logic-class flags: n z, v = c = 0
inline void dsp3210_device::set_flags_nz(uint32_t res, bool w16)
{
	if (w16)
	{
		set_flags(BIT(res, 15), (res & 0xffff) == 0, false, false);
	}
	else
	{
		set_flags(BIT(res, 31), res == 0, false, false);
	}
}

// add with carry-in; c = carry out of bit 15/31, v = signed overflow;
// (short) results are sign-extended from bit 15
inline uint32_t dsp3210_device::alu_add(uint32_t a, uint32_t b, uint32_t cin, bool w16)
{
	if (w16)
	{
		const uint32_t ua = a & 0xffff;
		const uint32_t ub = b & 0xffff;
		const uint32_t sum = ua + ub + cin;
		set_flags(BIT(sum, 15), (sum & 0xffff) == 0, BIT(~(ua ^ ub) & (ua ^ sum), 15), BIT(sum, 16));
		return uint32_t(int32_t(int16_t(sum)));
	}
	const uint64_t sum = uint64_t(a) + b + cin;
	const uint32_t res = uint32_t(sum);
	set_flags(BIT(res, 31), res == 0, BIT(~(a ^ b) & (a ^ res), 31), BIT(sum, 32));
	return res;
}

// a - b; c is the borrow (the SUBTRACT page's negate special case: c = 0
// only for rS = 0, v = 1 only for rS = 0x80000000)
inline uint32_t dsp3210_device::alu_sub(uint32_t a, uint32_t b, bool w16)
{
	if (w16)
	{
		const uint32_t ua = a & 0xffff;
		const uint32_t ub = b & 0xffff;
		const uint32_t diff = ua - ub;
		set_flags(BIT(diff, 15), (diff & 0xffff) == 0, BIT((ua ^ ub) & (ua ^ diff), 15), ua < ub);
		return uint32_t(int32_t(int16_t(diff)));
	}
	const uint32_t res = a - b;
	set_flags(BIT(res, 31), res == 0, BIT((a ^ b) & (a ^ res), 31), a < b);
	return res;
}

// the shared ALU body of formats 6a-6d: result and flags; store is
// cleared for the compare and bit-test forms
template <int F, bool Long>
inline uint32_t dsp3210_device::alu_op(uint32_t a, uint32_t b, bool &store)
{
	constexpr bool w16 = !Long;
	constexpr uint32_t mask = w16 ? 0xffff : 0xffffffff;
	uint32_t res = 0;
	store = true;

	switch (F)
	{
		case F_ADD:
			return alu_add(a, b, 0, w16);

		case F_SUB:
			return alu_sub(a, b, w16);

		case F_RSUB:                            // N - rD (immediate form)
			return alu_sub(b, a, w16);

		case F_CRADD:
		{
			// carry-reverse add: the carry propagates from the MSB down
			// (bit-reversed addressing for FFTs); flags n z c, v = 0
			constexpr int nb = w16 ? 16 : 32;
			const uint64_t sum = uint64_t(bitrev(a & mask, nb)) + bitrev(b & mask, nb);
			res = bitrev(uint32_t(sum) & mask, nb);
			if (w16)
			{
				res = uint32_t(int32_t(int16_t(res)));
			}
			set_flags(BIT(res, 31), (res & mask) == 0, false, BIT(sum, nb));
			return res;
		}

		case F_ANDC: res = a & ~b; break;
		case F_XOR:  res = a ^ b;  break;
		case F_OR:   res = a | b;  break;
		case F_AND:  res = a & b;  break;

		case F_BTST:
			set_flags_nz(a & b, w16);
			store = false;
			return 0;

		case F_CMP:
			alu_sub(a, b, w16);
			store = false;
			return 0;

		case F_SHL:                             // logical; the (short) result is zero-extended
			res = (a << (b & 31)) & mask;
			set_flags_nz(res, w16);
			return res;

		case F_SHR:
			res = (a & mask) >> (b & 31);
			set_flags_nz(res, w16);
			return res;

		case F_ASR:
		{
			const int32_t sa = w16 ? int32_t(int16_t(a)) : int32_t(a);
			res = uint32_t(sa >> (b & 31));
			if (w16)
			{
				res = uint32_t(int32_t(int16_t(res)));
			}
			set_flags_nz(res, w16);
			return res;
		}

		case F_ROL:                             // rotate left through carry = a + a + c, add flags
			return alu_add(a, a, c_flag() ? 1 : 0, w16);

		case F_ROR:                             // rotate right through carry: n z c, v = 0
		{
			const bool newc = BIT(a, 0);
			res = ((a & mask) >> 1) | (uint32_t(c_flag()) << (w16 ? 15 : 31));
			if (w16)
			{
				res = uint32_t(int32_t(int16_t(res)));
			}
			set_flags(BIT(res, 31), (res & mask) == 0, false, newc);
			return res;
		}

		default:                                // F = 5 is reserved (never dispatched here)
			store = false;
			return 0;
	}

	// the logic ops: (short) results sign-extend; flags n z, v = c = 0
	if (w16)
	{
		res = uint32_t(int32_t(int16_t(res)));
	}
	set_flags_nz(res, w16);
	return res;
}

// condition codes (Table 4-7); a DAU condition sees the flags established
// four instructions back (Latency 4, 4.4.2.4), which the pipe holds
bool dsp3210_device::condition(int c) const
{
	const uint8_t d = m_fpipe[m_slot & 3];
	switch (c & 63)
	{
		case 0:  return false;                              // false
		case 1:  return true;                               // true
		case 2:  return !n_flag();                          // pl
		case 3:  return n_flag();                           // mi
		case 4:  return !z_flag();                          // ne
		case 5:  return z_flag();                           // eq
		case 6:  return !v_flag();                          // vc
		case 7:  return v_flag();                           // vs
		case 8:  return !c_flag();                          // cc
		case 9:  return c_flag();                           // cs
		case 10: return !(n_flag() ^ v_flag());             // ge
		case 11: return n_flag() ^ v_flag();                // lt
		case 12: return !(z_flag() || (n_flag() ^ v_flag())); // gt
		case 13: return z_flag() || (n_flag() ^ v_flag());  // le
		case 14: return !(c_flag() || z_flag());            // hi
		case 15: return c_flag() || z_flag();               // ls
		case 16: return !(d & DAU_U);                       // auc
		case 17: return (d & DAU_U) != 0;                   // aus
		case 18: return !(d & DAU_N);                       // age
		case 19: return (d & DAU_N) != 0;                   // alt
		case 20: return !(d & DAU_Z);                       // ane
		case 21: return (d & DAU_Z) != 0;                   // aeq
		case 22: return !(d & DAU_V);                       // avc
		case 23: return (d & DAU_V) != 0;                   // avs
		case 24: return !(d & (DAU_N | DAU_Z));             // agt
		case 25: return (d & (DAU_N | DAU_Z)) != 0;         // ale
		case 32: return true;                               // ibe: no SIO, the input buffer is never full
		case 33: return false;                              // ibf
		case 34: return false;                              // obf: the output buffer is always empty
		case 35: return true;                               // obe
		case 40: return true;                               // syc
		case 41: return false;                              // sys
		case 42: return true;                               // fbc
		case 43: return false;                              // fbs
		case 44: return !BIT(m_ps_pins, 0);                 // ir0c: IR0N low
		case 45: return BIT(m_ps_pins, 0) != 0;             // ir0s
		case 46: return !BIT(m_ps_pins, 1);                 // ir1c
		case 47: return BIT(m_ps_pins, 1) != 0;             // ir1s
		default: return false;                              // reserved
	}
}


//**************************************************************************
//  MOVE HELPERS
//**************************************************************************

// register value loaded by a move of size W (LOAD page)
uint32_t dsp3210_device::w_extend(int w, uint32_t raw)
{
	switch (w & 7)
	{
		case 0:  return raw & 0xff;                         // (byte)
		case 1:  return uint32_t(int32_t(int8_t(raw)));     // (char)
		case 2:  return raw & 0xffff;                       // (ushort)
		case 3:  return uint32_t(int32_t(int16_t(raw)));    // (short)
		case 4:  return (raw & 0xff) << 8;                  // (hbyte)
		default: return raw;                                // (long)
	}
}

// register lane stored by a move of size W (STORE page)
uint32_t dsp3210_device::w_select(int w, uint32_t reg)
{
	switch (w & 7)
	{
		case 0: case 1: return reg & 0xff;                  // (byte)/(char): bits 7-0
		case 2: case 3: return reg & 0xffff;                // bits 15-0
		case 4:         return BIT(reg, 8, 8);              // (hbyte): bits 15-8
		default:        return reg;
	}
}

inline uint32_t dsp3210_device::mem_read(offs_t addr, int size)
{
	switch (size)
	{
		case 1:  return read_byte(addr);
		case 2:  return read_word(addr);
		default: return read_dword(addr);
	}
}

inline void dsp3210_device::mem_write(offs_t addr, int size, uint32_t data)
{
	switch (size)
	{
		case 1:  write_byte(addr, data); break;
		case 2:  write_word(addr, data); break;
		default: write_dword(addr, data); break;
	}
}

// post-modification of rP by the rI code: r0 = none, +n/-n = +-size,
// anything else adds the register value unscaled
inline void dsp3210_device::ca_postmod(int rp, int ri, int size)
{
	uint32_t delta;
	switch (ri)
	{
		case RC_R0:    return;
		case RC_PLUS:  delta = size; break;
		case RC_MINUS: delta = uint32_t(-size); break;
		default:       delta = reg_r(ri); break;
	}
	reg_w(rp, reg_r(rp) + delta);
}


//**************************************************************************
//  CAU: BRANCHES
//**************************************************************************

// 0b/1b: if (C) goto {N, rB, rB+N}; C = 0 is nop, C = 1 with rB = pcsh
// and N = 0 is ireturn.  The delay slot is executed by the main loop.
template <int C>
void dsp3210_device::goto_c(uint32_t op)
{
	if constexpr (C == 0)
	{
		return;
	}
	else if constexpr (C == 1)
	{
		if ((op & 0x001fffff) == 0x001e0000)
		{
			do_ireturn();
			return;
		}
	}
	else
	{
		if (!condition(C))
		{
			return;
		}
	}

	m_npc = reg_r(BIT(op, 16, 5)) + uint32_t(int16_t(op));
}

// 3a: if (rM-- >= 0) goto {N, rB, rB+N}: the old value is tested, the
// register is decremented either way
void dsp3210_device::dec_goto(uint32_t op)
{
	const int rm = BIT(op, 21, 5);
	const uint32_t val = reg_r(rm);
	if (int32_t(val) >= 0)
	{
		m_npc = reg_r(BIT(op, 16, 5)) + uint32_t(int16_t(op));
	}
	reg_w(rm, val - 1);
}

// 4a: call {N, rB, rB+N} (rM): rM links the address after the delay slot
void dsp3210_device::call(uint32_t op)
{
	const uint32_t target = reg_r(BIT(op, 16, 5)) + uint32_t(int16_t(op));
	reg_w(BIT(op, 21, 5), m_ppc + 8);
	m_npc = target;
}

// 8a: goto {M, rB+M}
void dsp3210_device::goto24(uint32_t op)
{
	m_npc = reg_r(BIT(op, 16, 5)) + ((BIT(op, 21, 8) << 16) | (op & 0xffff));
}

// 8c: call M (rM)
void dsp3210_device::call24(uint32_t op)
{
	reg_w(BIT(op, 16, 5), m_ppc + 8);
	m_npc = (BIT(op, 21, 8) << 16) | (op & 0xffff);
}

// 3b/3c: do/dolock/doblock K,{L,rM}: the next K+1 instructions run
// count+1 times; the back edge is taken by the main loop
void dsp3210_device::do_loop(uint32_t op, uint32_t count)
{
	const bool b = BIT(op, 24);
	const bool m = BIT(op, 23);
	if (b && m)
	{
		return;                                 // reserved combination
	}
	m_do_active = true;
	m_do_lock = b;
	m_do_start = m_pc;                          // the next instruction
	m_do_end = m_pc + (m ? 0 : 4 * BIT(op, 11, 7));
	m_do_count = count + 1;
}

void dsp3210_device::do_imm(uint32_t op)
{
	do_loop(op, op & 0x7ff);
}

void dsp3210_device::do_reg(uint32_t op)
{
	do_loop(op, reg_r(op & 31) & 0x7ff);
}

// ireturn: restore the shadow set, then replay the instruction that was
// prefetched when the interrupt hit in place of a delay slot and resume
// at pcsh (which the handler may have rewritten)
void dsp3210_device::do_ireturn()
{
	if (m_level == 1)
	{
		m_nzcflags = m_sh_nzcflags;
		m_vflags = m_sh_vflags;
		m_dauflags = m_sh_dauflags;
		m_dauc = m_sh_dauc;
		m_ctr = m_sh_ctr;
		std::copy(std::begin(m_sh_acc), std::end(m_sh_acc), std::begin(m_acc));
		m_apipe_m = m_sh_apipe_m;
		m_apipe_e = m_sh_apipe_e;
		std::copy(std::begin(m_sh_fpipe), std::end(m_sh_fpipe), std::begin(m_fpipe));
		m_slot = m_sh_slot;
		m_do_active = m_sh_do_active;
		m_do_lock = m_sh_do_lock;
		m_do_start = m_sh_do_start;
		m_do_end = m_sh_do_end;
		m_do_count = m_sh_do_count;
		m_level = 0;
		m_servicing = 0;
		update_iack();
	}
	m_pc = m_r[RC_PCSH];
	m_npc = m_pc + 4;
	m_irsh_pending = true;
}


//**************************************************************************
//  CAU: ARITHMETIC
//**************************************************************************

// 5a/5b: rD = [(short)] rS3 + N (rS3 = r0 is the load-immediate SET)
template <bool Long>
void dsp3210_device::add_si(uint32_t op)
{
	reg_w(BIT(op, 21, 5), alu_add(reg_r(BIT(op, 16, 5)), uint32_t(int16_t(op)), 0, !Long));
}

// 6a/6b: [if (C)] rD = [(short)] rS1 F rS2
template <int F, bool Long>
void dsp3210_device::alu_rr(uint32_t op)
{
	const int c = BIT(op, 5, 6);
	if ((c != 1) && !condition(c))
	{
		return;                                 // COND false: no store, no flags
	}

	const int rd = BIT(op, 16, 5);
	const int rs1 = BIT(op, 11, 5);
	const int rs2 = op & 31;
	const uint32_t a = reg_r(rs1);
	uint32_t b;
	if ((F == F_ADD) && (rd == RC_SP) && (rs1 == RC_SP) && ((rs2 == RC_PLUS) || (rs2 == RC_MINUS)))
	{
		b = (rs2 == RC_PLUS) ? 4 : uint32_t(-4);    // sp = sp++ / sp = sp-- move by 4
	}
	else
	{
		b = reg_r(rs2);                             // +n/-n read as +-1
	}

	bool store;
	const uint32_t res = alu_op<F, Long>(a, b, store);
	if (store)
	{
		reg_w(rd, res);
	}
}

// 6c/6d: rD = [(short)] rD F N (the rotates have no immediate operand)
template <int F, bool Long>
void dsp3210_device::alu_ri(uint32_t op)
{
	const int rd = BIT(op, 16, 5);
	const uint32_t a = reg_r(rd);
	const uint32_t n = ((F == F_ROL) || (F == F_ROR)) ? a : uint32_t(int16_t(op));

	bool store;
	const uint32_t res = alu_op<F, Long>(a, n, store);
	if (store)
	{
		reg_w(rd, res);
	}
}

// 4b: rD = rS <<| N: rS | (N << 16), always 32-bit, flags n z
void dsp3210_device::shift_or(uint32_t op)
{
	const uint32_t res = reg_r(BIT(op, 16, 5)) | ((op & 0xffff) << 16);
	set_flags_nz(res, false);
	reg_w(BIT(op, 21, 5), res);
}

// 8b: rD = (ushort24) M, no flags
void dsp3210_device::load24(uint32_t op)
{
	reg_w(BIT(op, 16, 5), (BIT(op, 21, 8) << 16) | (op & 0xffff));
}


//**************************************************************************
//  CAU: MOVES
//**************************************************************************

// Register loads set n/z from the loaded (extended) value with v = c = 0:
// the LOAD page's header says "none", but its own restriction speaks of
// "the flags set as a result of the load" and Apple's code branches on
// `rD = *rD ; nop ; if (gt)`.

// 7a: rH = (w) *L / *L = (w) rH: L addresses the on-chip window
template <bool Store, int W>
void dsp3210_device::move_direct(uint32_t op)
{
	const offs_t addr = m_onchip_base | (op & 0xffff);
	const int rh = BIT(op, 16, 5);
	constexpr int size = w_size(W);

	if constexpr (!Store)
	{
		const uint32_t v = w_extend(W, mem_read(addr, size));
		reg_w(rh, v);
		set_flags_nz(v, false);
	}
	else
	{
		mem_write(addr, size, w_select(W, reg_r(rh)));
	}
}

// 7b (bit 10 set): rH = (w) ior / ior = (w) rH, including the spc
// pseudo-instructions; 7c: rH = (w) *rP++rI / *rP++rI = (w) rH
template <bool Store, int W>
void dsp3210_device::move_ind(uint32_t op)
{
	const int rh = BIT(op, 16, 5);
	constexpr int size = w_size(W);

	if (BIT(op, 10))
	{
		const int ior = op & 31;
		if constexpr (!Store)
		{
			const uint32_t v = w_extend(W, ior_read(ior));
			reg_w(rh, v);
			set_flags_nz(v, false);
		}
		else
		{
			ior_write(ior, size, w_select(W, reg_r(rh)));
		}
		return;
	}

	const int rp = BIT(op, 11, 5);
	const int ri = op & 31;
	const offs_t addr = reg_r(rp);
	if constexpr (!Store)
	{
		const uint32_t v = w_extend(W, mem_read(addr, size));
		ca_postmod(rp, ri, size);
		reg_w(rh, v);
		set_flags_nz(v, false);
	}
	else
	{
		mem_write(addr, size, w_select(W, reg_r(rh)));
		ca_postmod(rp, ri, size);
	}
}

// 7d: ior = (w) *rP++rI / *rP++rI = (w) ior
template <bool Store, int W>
void dsp3210_device::move_ior_mem(uint32_t op)
{
	const int ior = BIT(op, 16, 5);
	const int rp = BIT(op, 11, 5);
	const int ri = op & 31;
	constexpr int size = w_size(W);
	const offs_t addr = reg_r(rp);

	if constexpr (!Store)
	{
		const uint32_t v = mem_read(addr, size);
		ca_postmod(rp, ri, size);
		ior_write(ior, size, v);                // a byte/short load affects only that lane
	}
	else
	{
		mem_write(addr, size, w_select(W, ior_read(ior)));
		ca_postmod(rp, ri, size);
	}
}


//**************************************************************************
//  DAU: OPERANDS, STORES AND FLAGS
//**************************************************************************

// the accumulator file as the multiplier sees it: as of three slots back
// (Latency 2, 4.4.2.2)
inline dsp3210dau::acc_t dsp3210_device::da_mult_acc(int n) const
{
	const int k = (m_slot + 1) & 3;
	return { m_apipe_m[k][n], m_apipe_e[k][n] };
}

// post-modification of a DA pointer register r1-r14 by a field's I bits:
// 0 none, 1-5 add r15-r19 unscaled, 6/7 minus/plus the operand size
inline void dsp3210_device::da_postmod(int p, int i, int size)
{
	switch (i & 7)
	{
		case 0:  break;
		case 6:  m_r[p] -= size; break;
		case 7:  m_r[p] += size; break;
		default: m_r[p] += m_r[16 + (i & 7)]; break;
	}
}

// an X or Y field: register-direct (an accumulator - the live value for
// the adder, the guard-truncated, three-slots-old value for the
// multiplier) or *rP with post-modification.  Table 10-3 reserves p =
// 1111 in X/Y; it reads as an accumulator here.
inline void dsp3210_device::da_read(int field, int size, da_operand &o)
{
	const int p = BIT(field, 3, 4);
	const int i = field & 7;
	if ((p == 0) || (p == 15))
	{
		o.from_acc = true;
		o.is_mem = false;
		o.acc = i & 3;
		o.preg = 0;
		o.ea = 0;
		o.val = m_acc[o.acc];
		o.raw = dsp3210dau::pack(da_mult_acc(o.acc));
		return;
	}
	o.from_acc = false;
	o.is_mem = true;
	o.acc = 0;
	o.preg = p;
	o.ea = m_r[p];
	o.raw = mem_read(o.ea, size);
	da_postmod(p, i, size);
}

// The Z field: p = 0000 never stores (0000111 is the manual's "no
// write"), p = 0001-1110 stores through its own pointer register, and
// p = 1111 - "not allowed" in Table 10-3 but what Apple's assembler
// emits for every in-place operation (*r2++ = a0 = *r2 * a1, the whole
// body of the standard-sound Midput module) - stores through the Y
// operand's own effective address, the Z I bits post-modifying Y's
// pointer; with an accumulator Y there is nothing to store through.
inline bool dsp3210_device::da_z_writes(int z, const da_operand &y)
{
	const int p = BIT(z, 3, 4);
	if (p == 0)
	{
		return false;
	}
	if (p != 15)
	{
		return true;
	}
	return y.is_mem;
}

// a DA store reaches memory four slots later (8.2.6: the result is
// driven in the fourth pipeline stage), through the deferred ring; the
// pointer post-modification happens now
inline void dsp3210_device::da_store_z(int z, const da_operand &y, int size, uint32_t data)
{
	const int p = BIT(z, 3, 4);
	const int i = z & 7;
	if (p == 0)
	{
		return;
	}
	// the alignment is judged now (an unmasked address error aborts the
	// instruction before the post-modify), the write lands later
	mbuf_entry &e = m_mbuf[m_slot & 3];
	if (p == 15)
	{
		if (!y.is_mem)
		{
			return;
		}
		e = { uint32_t(check_align(y.ea, size)), data, uint8_t(size) };
		da_postmod(y.preg, i, size);
	}
	else
	{
		e = { uint32_t(check_align(m_r[p], size)), data, uint8_t(size) };
		da_postmod(p, i, size);
	}
}

// the Z post-modification without the store (a suppressed conditional
// store with dauc[6] set: "the address postmodifications are always
// performed")
inline void dsp3210_device::da_postmod_only(int z, const da_operand &y, int size)
{
	const int p = BIT(z, 3, 4);
	if (p == 0)
	{
		return;
	}
	if (p == 15)
	{
		if (y.is_mem)
		{
			da_postmod(y.preg, z & 7, size);
		}
		return;
	}
	da_postmod(p, z & 7, size);
}

// the adder input: an accumulator keeps its guard bits (a zero exponent
// is a dirty zero), a memory word has none
inline dsp3210dau::acc_t dsp3210_device::da_adder(const da_operand &o)
{
	if (o.from_acc)
	{
		return (o.val.e == 0) ? dsp3210dau::zero() : o.val;
	}
	return dsp3210dau::unpack(o.raw);
}

// flags from a result: N and Z always, V and U from the range check when
// the instruction affects them; ctr shifts in N (Table 8-4).  V or U
// request the DAU overflow/underflow error (vector 5), raised after the
// result has been computed: it is written (saturated or flushed) before
// the dispatch to the vector.
dsp3210dau::acc_t dsp3210_device::da_set_flags(dsp3210dau::acc_t res, bool affect_vu)
{
	bool v = false;
	bool u = false;
	if (affect_vu)
	{
		res = dsp3210dau::check_range(res, v, u);
	}
	const bool n = (res.e != 0) && (res.m < 0);
	const bool z = (res.e == 0);
	m_dauflags = (n ? DAU_N : 0) | (z ? DAU_Z : 0) | (u ? DAU_U : 0) | (v ? DAU_V : 0);
	m_ctr = ((m_ctr << 1) | (n ? 1 : 0)) & 0x3f;
	if (v || u)
	{
		take_error(5);
	}
	return res;
}

void dsp3210_device::da_set_flags_raw(uint8_t flags)
{
	m_dauflags = flags;
	m_ctr = ((m_ctr << 1) | (flags & DAU_N)) & 0x3f;
}

// the multiply/accumulate common to formats 1-4: aN = [-]S {+,-} P, with
// the tap forms passing the Y operand rather than the result to Z
void dsp3210_device::da_mac(dsp3210dau::acc_t adder, int64_t pm, int pe, bool nega, bool negp, bool tap, const da_operand &y, int n, int zf)
{
	if (nega)
	{
		adder = dsp3210dau::negate(adder);
	}
	if (negp)
	{
		pm = -pm;                                   // exact at 46 fraction bits
	}
	const dsp3210dau::acc_t res = da_set_flags(dsp3210dau::accumulate(adder, pm, pe), true);
	m_acc[n] = res;
	if (da_z_writes(zf, y))
	{
		// a tap stores the Y operand (an accumulator Y: its live value);
		// otherwise the result with its guard bits truncated
		const uint32_t data = tap ? (y.from_acc ? dsp3210dau::pack(m_acc[y.acc]) : y.raw) : dsp3210dau::pack(res);
		da_store_z(zf, y, 4, data);
	}
}

// let every pending DA store land, oldest first: the pipeline completes
// when the core stops on bkpt
void dsp3210_device::flush_mbuf()
{
	for (int k = 1; k <= 4; k++)
	{
		mbuf_entry &e = m_mbuf[(m_slot + k) & 3];
		if (e.width)
		{
			mem_write(e.addr, e.width, e.data);
			e.width = 0;
		}
	}
}

// the end of a slot: record the accumulator file and the flags for the
// latency pipes
inline void dsp3210_device::advance_pipes()
{
	const int k = m_slot & 3;
	for (int i = 0; i < 4; i++)
	{
		m_apipe_m[k][i] = m_acc[i].m;
		m_apipe_e[k][i] = m_acc[i].e;
	}
	m_fpipe[k] = m_dauflags;
	m_slot++;
}


//**************************************************************************
//  DAU: MULTIPLY/ACCUMULATE (FORMATS 1-4)
//**************************************************************************

// format 1: [Z =] aN = [-]Y {+,-} {aM, 0.0, 1.0} * X
// format 4 (format 1, M = 110, with Tap): aN = [-](Z = Y) {+,-} X
template <int M, bool NegA, bool NegP, bool Tap>
void dsp3210_device::da14(uint32_t op)
{
	da_operand x, y;
	da_read(BIT(op, 14, 7), 4, x);
	da_read(BIT(op, 7, 7), 4, y);
	int64_t pm = 0;
	int pe = 0;
	if constexpr (M == 5)
	{
		dsp3210dau::multiply(dsp3210dau::WORD_ONE, x.raw, pm, pe);
	}
	else if constexpr (M < 4)
	{
		dsp3210dau::multiply(dsp3210dau::pack(da_mult_acc(M)), x.raw, pm, pe);
	}
	da_mac(da_adder(y), pm, pe, NegA, NegP, Tap, y, BIT(op, 21, 2), op & 0x7f);
}

// format 2 (Tap): aN = [-]{aM, 0.0, 1.0} {+,-} (Z = Y) * X
// format 3:       [Z =] aN = [-]{aM, 0.0, 1.0} {+,-} Y * X
template <int M, bool NegA, bool NegP, bool Tap>
void dsp3210_device::da23(uint32_t op)
{
	da_operand x, y;
	da_read(BIT(op, 14, 7), 4, x);
	da_read(BIT(op, 7, 7), 4, y);
	dsp3210dau::acc_t adder;
	if constexpr (M == 4)
	{
		adder = dsp3210dau::zero();
	}
	else if constexpr (M == 5)
	{
		adder = dsp3210dau::unpack(dsp3210dau::WORD_ONE);
	}
	else
	{
		adder = (m_acc[M].e == 0) ? dsp3210dau::zero() : m_acc[M];
	}
	int64_t pm;
	int pe;
	dsp3210dau::multiply(y.raw, x.raw, pm, pe);
	da_mac(adder, pm, pe, NegA, NegP, Tap, y, BIT(op, 21, 2), op & 0x7f);
}


//**************************************************************************
//  DAU: SPECIAL FUNCTIONS (FORMAT 5)
//**************************************************************************

// [Z =] aN = g(Y).  The integer and companded conversions deposit their
// result raw in the accumulator's lane (INT32: "the 32 MSBs of an
// accumulator, the mantissa and guard bits"; INT16: the upper 16; OC:
// the most significant byte) with the exponent unpredictable; ic,
// float16 and float32 read the same lane back, which is how Apple's
// sound-input converter splits a phase accumulator.
template <int G>
void dsp3210_device::da5(uint32_t op)
{
	constexpr int ysize = (G == 0) ? 1 : (G == 2) ? 2 : 4;
	constexpr int zsize = (G == 1) ? 1 : (G == 3) ? 2 : 4;
	const int n = BIT(op, 21, 2);
	const int zf = op & 0x7f;
	const int mode = BIT(m_dauc, 4, 2);
	da_operand y;
	da_read(BIT(op, 7, 7), ysize, y);
	bool zw = da_z_writes(zf, y);
	dsp3210dau::acc_t res = m_acc[n];
	uint32_t zbits = 0;

	if constexpr (G == 0)
	{
		// ic: companded or unsigned byte -> float, N Z, V = U = 0; from an
		// accumulator the byte is the top of the lane
		const uint8_t code = y.from_acc ? uint8_t(dsp3210dau::lane(y.val) >> 24) : uint8_t(y.raw);
		if (BIT(m_dauc, 2))
		{
			res = dsp3210dau::from_int(code);
		}
		else
		{
			// the decoded value is a half-integer: build twice it and halve exactly
			res = dsp3210dau::from_int(BIT(m_dauc, 0) ? dsp3210dau::alaw_decode2(code) : dsp3210dau::mulaw_decode2(code));
			if (res.e)
			{
				res.e--;
			}
		}
		res = da_set_flags(res, false);
		zbits = dsp3210dau::pack(res);
	}
	else if constexpr (G == 1)
	{
		// oc: float -> companded or unsigned byte, no flags (dauc[1] = 1
		// selects A-law out per Table 8-3; the OC page has it backwards)
		const dsp3210dau::acc_t yv = da_adder(y);
		const uint8_t code = BIT(m_dauc, 3) ? uint8_t(dsp3210dau::to_int(yv, mode, 8)) : dsp3210dau::companded_encode(yv, BIT(m_dauc, 1));
		res = dsp3210dau::from_lane(int32_t(uint32_t(code) << 24), yv.e);
		zbits = code;
	}
	else if constexpr (G == 2)
	{
		// float16: int16 -> float, N Z, V = U = 0
		const int16_t iv = y.from_acc ? int16_t(dsp3210dau::lane(y.val) >> 16) : int16_t(y.raw);
		res = da_set_flags(dsp3210dau::from_int(iv), false);
		zbits = dsp3210dau::pack(res);
	}
	else if constexpr (G == 8)
	{
		// float32: int32 -> float, N Z, V = U = 0
		const int32_t iv = y.from_acc ? int32_t(dsp3210dau::lane(y.val)) : int32_t(y.raw);
		res = da_set_flags(dsp3210dau::from_int(iv), false);
		zbits = dsp3210dau::pack(res);
	}
	else if constexpr (G == 3)
	{
		// int16: float -> int16 per dauc[5:4], saturating, no flags
		const dsp3210dau::acc_t yv = da_adder(y);
		const int64_t iv = dsp3210dau::to_int(yv, mode, 16);
		res = dsp3210dau::from_lane(int32_t(uint32_t(iv) << 16), yv.e);
		zbits = uint32_t(iv) & 0xffff;
	}
	else if constexpr (G == 9)
	{
		// int32
		const dsp3210dau::acc_t yv = da_adder(y);
		const int64_t iv = dsp3210dau::to_int(yv, mode, 32);
		res = dsp3210dau::from_lane(int32_t(uint32_t(iv)), yv.e);
		zbits = uint32_t(iv);
	}
	else if constexpr (G == 4)
	{
		// round: 40 -> 32 bits, N Z V U
		res = da_set_flags(dsp3210dau::round(da_adder(y)), true);
		zbits = dsp3210dau::pack(res);
	}
	else if constexpr ((G == 5) || (G == 6) || (G == 7))
	{
		// ifalt/ifaeq/ifagt: load Y on the LIVE flags of the last DA
		// result (the documented zero-latency exception), no flags; the
		// Z write stores aN's resulting value and, with dauc[6] set, is
		// itself conditional
		const bool an = (m_dauflags & DAU_N) != 0;
		const bool az = (m_dauflags & DAU_Z) != 0;
		const bool cond = (G == 5) ? an : (G == 6) ? az : !(an || az);
		if (cond)
		{
			res = da_adder(y);
		}
		zbits = dsp3210dau::pack(res);
		if (zw && BIT(m_dauc, 6) && !cond)
		{
			da_postmod_only(zf, y, zsize);
			zw = false;
		}
	}
	else if constexpr (G == 12)
	{
		// ieee: DSP32 -> IEEE single, no flags; the accumulator keeps the value
		const dsp3210dau::acc_t yv = da_adder(y);
		zbits = dsp3210dau::to_ieee(yv);
		res = yv;
	}
	else if constexpr (G == 13)
	{
		// dsp: IEEE single -> DSP32, N Z V U; +-inf saturate, a NaN also
		// requests the IEEE NaN error (vector 6), zero and denormals give
		// +0 (8.2.4.1)
		const uint32_t iw = y.from_acc ? dsp3210dau::pack(y.val) : y.raw;
		const bool sgn = BIT(iw, 31);
		const int exp = BIT(iw, 23, 8);
		const uint32_t mant = iw & 0x7fffff;
		if (exp == 255)
		{
			zbits = sgn ? 0x800000ff : 0x7fffffff;
			res = dsp3210dau::unpack(zbits);
			da_set_flags_raw((sgn ? DAU_N : 0) | ((!sgn || mant) ? DAU_V : 0));
			if (mant)
			{
				take_error(6);
			}
		}
		else if (exp == 0)
		{
			res = dsp3210dau::zero();
			da_set_flags_raw(DAU_Z | (mant ? DAU_U : 0));
			zbits = 0;
		}
		else
		{
			res = da_set_flags(dsp3210dau::from_ieee(iw), true);
			zbits = dsp3210dau::pack(res);
		}
	}
	else if constexpr (G == 14)
	{
		// seed: invert every bit but the sign, N Z U, V = 0 (an exponent
		// of 0xff inverts to a zero exponent: underflow)
		const uint32_t yb = y.from_acc ? dsp3210dau::pack(y.val) : y.raw;
		const uint32_t sb = yb ^ 0x7fffffff;
		res = da_set_flags(dsp3210dau::unpack(sb), false);
		if ((yb & 0xff) == 0xff)
		{
			m_dauflags |= DAU_U;
		}
		zbits = sb;
	}

	m_acc[n] = res;
	if (zw)
	{
		da_store_z(zf, y, zsize, zbits);
	}
}
