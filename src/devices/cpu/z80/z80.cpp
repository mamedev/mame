// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************

    ZiLOG Z80 emulator

TODO:
- Interrupt mode 0 should be able to execute arbitrary opcodes
- If LD A,I or LD A,R is interrupted, P/V flag gets reset, even if IFF2
  was set before this instruction (implemented, but not enabled: we need
  document Z80 types first, see below)
- Ideally, the tiny differences between Z80 types should be supported,
  currently known differences:
  - LD A,I/R P/V flag reset glitch is fixed on CMOS Z80
  - OUT (C),0 outputs 0 on NMOS Z80, $FF on CMOS Z80
  - SCF/CCF X/Y flags is ((flags | A) & 0x28) on SGS/SHARP/ZiLOG NMOS Z80,
    (flags & A & 0x28).
    However, recent findings say that SCF/CCF X/Y results depend on whether
    or not the previous instruction touched the flag register.
  This Z80 emulator assumes a ZiLOG NMOS model.

*****************************************************************************/

#include "emu.h"
#include "z80.h"
#include "z80dasm.h"

#include "z80.inc"

bool z80_device::tables_initialised = false;
u8 z80_device::SZ[] = {};       // zero and sign flags
u8 z80_device::SZ_BIT[] = {};   // zero, sign and parity/overflow (=zero) flags for BIT opcode
u8 z80_device::SZP[] = {};      // zero, sign and parity flags
u8 z80_device::SZHV_inc[] = {}; // zero, sign, half carry and overflow flags INC r8
u8 z80_device::SZHV_dec[] = {}; // zero, sign, half carry and overflow flags DEC r8
u8 z80_device::SZHVC_add[] = {};
u8 z80_device::SZHVC_sub[] = {};


/***************************************************************
 * Enter halt state; write 1 to callback on first execution
 ***************************************************************/
void z80_device::halt()
{
	if (!m_halt)
	{
		m_halt = 1;
		m_halt_cb(1);
	}
}

/***************************************************************
 * Leave halt state; write 0 to callback
 ***************************************************************/
void z80_device::leave_halt()
{
	if (m_halt)
	{
		m_halt = 0;
		m_halt_cb(0);
	}
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
u8 z80_device::data_read(u16 addr)
{
	return m_data.read_interruptible(translate_memory_address(addr));
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
void z80_device::data_write(u16 addr, u8 value)
{
	m_data.write_interruptible(translate_memory_address((u32)addr), value);
}

/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
u8 z80_device::opcode_read()
{
	return m_opcodes.read_byte(translate_memory_address(PCD));
}

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 * out: TDAT8
 ***************************************************************/
u8 z80_device::arg_read()
{
	return m_args.read_byte(translate_memory_address(PCD));
}

/***************************************************************
 * INC  r8
 ***************************************************************/
void z80_device::inc(u8 &r)
{
	++r;
	set_f((F & CF) | SZHV_inc[r]);
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
void z80_device::dec(u8 &r)
{
	--r;
	set_f((F & CF) | SZHV_dec[r]);
}

/***************************************************************
 * RLCA
 ***************************************************************/
void z80_device::rlca()
{
	A = (A << 1) | (A >> 7);
	set_f((F & (SF | ZF | PF)) | (A & (YF | XF | CF)));
}

/***************************************************************
 * RRCA
 ***************************************************************/
void z80_device::rrca()
{
	set_f((F & (SF | ZF | PF)) | (A & CF));
	A = (A >> 1) | (A << 7);
	F |= (A & (YF | XF));
}

/***************************************************************
 * RLA
 ***************************************************************/
void z80_device::rla()
{
	u8 res = (A << 1) | (F & CF);
	u8 c = (A & 0x80) ? CF : 0;
	set_f((F & (SF | ZF | PF)) | c | (res & (YF | XF)));
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
void z80_device::rra()
{
	u8 res = (A >> 1) | (F << 7);
	u8 c = (A & 0x01) ? CF : 0;
	set_f((F & (SF | ZF | PF)) | c | (res & (YF | XF)));
	A = res;
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
void z80_device::add_a(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) + value);
	set_f(SZHVC_add[ah | res]);
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
void z80_device::adc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) + value + c);
	set_f(SZHVC_add[(c << 16) | ah | res]);
	A = res;
}

/***************************************************************
 * SUB  n
 ***************************************************************/
void z80_device::sub(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - value);
	set_f(SZHVC_sub[ah | res]);
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
void z80_device::sbc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) - value - c);
	set_f(SZHVC_sub[(c << 16) | ah | res]);
	A = res;
}

/***************************************************************
 * NEG
 ***************************************************************/
void z80_device::neg()
{
	u8 value = A;
	A = 0;
	sub(value);
}

/***************************************************************
 * DAA
 ***************************************************************/
void z80_device::daa()
{
	u8 a = A;
	if (F & NF)
	{
		if ((F&HF) | ((A&0xf)>9)) a-=6;
		if ((F&CF) | (A>0x99)) a-=0x60;
	}
	else
	{
		if ((F&HF) | ((A&0xf)>9)) a+=6;
		if ((F&CF) | (A>0x99)) a+=0x60;
	}

	set_f((F&(CF|NF)) | (A>0x99) | ((A^a)&HF) | SZP[a]);
	A = a;
}

/***************************************************************
 * AND  n
 ***************************************************************/
void z80_device::and_a(u8 value)
{
	A &= value;
	set_f(SZP[A] | HF);
}

/***************************************************************
 * OR   n
 ***************************************************************/
void z80_device::or_a(u8 value)
{
	A |= value;
	set_f(SZP[A]);
}

/***************************************************************
 * XOR  n
 ***************************************************************/
void z80_device::xor_a(u8 value)
{
	A ^= value;
	set_f(SZP[A]);
}

/***************************************************************
 * CP   n
 ***************************************************************/
void z80_device::cp(u8 value)
{
	unsigned val = value;
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - val);
	set_f((SZHVC_sub[ah | res] & ~(YF | XF)) | (val & (YF | XF)));
}

/***************************************************************
 * EXX
 ***************************************************************/
void z80_device::exx()
{
	using std::swap;
	swap(m_bc, m_bc2);
	swap(m_de, m_de2);
	swap(m_hl, m_hl2);
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
u8 z80_device::rlc(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * RRC  r8
 ***************************************************************/
u8 z80_device::rrc(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res << 7)) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * RL   r8
 ***************************************************************/
u8 z80_device::rl(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (F & CF)) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * RR   r8
 ***************************************************************/
u8 z80_device::rr(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (F << 7)) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * SLA  r8
 ***************************************************************/
u8 z80_device::sla(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * SRA  r8
 ***************************************************************/
u8 z80_device::sra(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res & 0x80)) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * SLL  r8
 ***************************************************************/
u8 z80_device::sll(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | 0x01) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * SRL  r8
 ***************************************************************/
u8 z80_device::srl(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = (res >> 1) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
void z80_device::bit(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1 << bit)] & ~(YF | XF)) | (value & (YF | XF)));
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
void z80_device::bit_hl(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1 << bit)] & ~(YF | XF)) | (WZ_H & (YF | XF)));
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
void z80_device::bit_xy(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1 << bit)] & ~(YF | XF)) | ((m_ea >> 8) & (YF | XF)));
}

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
u8 z80_device::res(int bit, u8 value)
{
	return value & ~(1 << bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
u8 z80_device::set(int bit, u8 value)
{
	return value | (1 << bit);
}

void z80_device::block_io_interrupted_flags()
{
	F &= ~(YF | XF);
	F |= (PC >> 8) & (YF | XF);
	if (F & CF)
	{
		F &= ~HF;
		if (TDAT8 & 0x80)
		{
			F ^= (SZP[(B - 1) & 0x07] ^ PF) & PF;
			if ((B & 0x0f) == 0x00) F |= HF;
		}
		else
		{
			F ^= (SZP[(B + 1) & 0x07] ^ PF) & PF;
			if ((B & 0x0f) == 0x0f) F |= HF;
		}
	}
	else
	{
		F ^=(SZP[B & 0x07] ^ PF) & PF;
	}
}

/***************************************************************
 * EI
 ***************************************************************/
void z80_device::ei()
{
	m_iff1 = m_iff2 = 1;
	m_after_ei = true;
}

void z80_device::set_f(u8 f)
{
	QT = 0;
	F = f;
}

void z80_device::illegal_1()
{
	LOGUNDOC("ill. opcode $%02x $%02x ($%04x)\n",
			 m_opcodes.read_byte(translate_memory_address((PCD - 1) & 0xffff)), m_opcodes.read_byte(translate_memory_address(PCD)), PCD - 1);
}

void z80_device::illegal_2()
{
	LOGUNDOC("ill. opcode $ed $%02x\n",
			 m_opcodes.read_byte(translate_memory_address((PCD - 1) & 0xffff)));
}

/****************************************************************************
 * Processor initialization
 ****************************************************************************/
void z80_device::device_validity_check(validity_checker &valid) const
{
	cpu_device::device_validity_check(valid);

	if (4 > m_m1_cycles)
		osd_printf_error("M1 cycles %u is less than minimum 4\n", m_m1_cycles);
	if (3 > m_memrq_cycles)
		osd_printf_error("MEMRQ cycles %u is less than minimum 3\n", m_memrq_cycles);
	if (4 > m_iorq_cycles)
		osd_printf_error("IORQ cycles %u is less than minimum 4\n", m_iorq_cycles);
}

void z80_device::device_start()
{
	if (!tables_initialised)
	{
		u8 *padd = &SZHVC_add[  0*256];
		u8 *padc = &SZHVC_add[256*256];
		u8 *psub = &SZHVC_sub[  0*256];
		u8 *psbc = &SZHVC_sub[256*256];
		for (int oldval = 0; oldval < 256; oldval++)
		{
			for (int newval = 0; newval < 256; newval++)
			{
				// add or adc w/o carry set
				int val = newval - oldval;
				*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padd |= (newval & (YF | XF));  // undocumented flag bits 5+3
				if ((newval & 0x0f) < (oldval & 0x0f)) *padd |= HF;
				if (newval < oldval) *padd |= CF;
				if ((val^oldval^0x80) & (val^newval) & 0x80) *padd |= VF;
				padd++;

				// adc with carry set
				val = newval - oldval - 1;
				*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padc |= (newval & (YF | XF));  // undocumented flag bits 5+3
				if ((newval & 0x0f) <= (oldval & 0x0f)) *padc |= HF;
				if (newval <= oldval) *padc |= CF;
				if ((val^oldval^0x80) & (val^newval) & 0x80) *padc |= VF;
				padc++;

				// cp, sub or sbc w/o carry set
				val = oldval - newval;
				*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psub |= (newval & (YF | XF));  // undocumented flag bits 5+3
				if ((newval & 0x0f) > (oldval & 0x0f)) *psub |= HF;
				if (newval > oldval) *psub |= CF;
				if ((val^oldval) & (oldval^newval) & 0x80) *psub |= VF;
				psub++;

				// sbc with carry set
				val = oldval - newval - 1;
				*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psbc |= (newval & (YF | XF));  // undocumented flag bits 5+3
				if ((newval & 0x0f) >= (oldval & 0x0f)) *psbc |= HF;
				if (newval >= oldval) *psbc |= CF;
				if ((val^oldval) & (oldval^newval) & 0x80) *psbc |= VF;
				psbc++;
			}
		}

		for (int i = 0; i < 256; i++)
		{
			int p = 0;
			for (int b = 0; b < 8; b++)
				p += BIT(i, b);
			SZ[i] = i ? i & SF : ZF;
			SZ[i] |= (i & (YF | XF));         // undocumented flag bits 5+3
			SZ_BIT[i] = i ? i & SF : ZF | PF;
			SZ_BIT[i] |= (i & (YF | XF));     // undocumented flag bits 5+3
			SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
			SZHV_inc[i] = SZ[i];
			if (i == 0x80) SZHV_inc[i] |= VF;
			if ((i & 0x0f) == 0x00) SZHV_inc[i] |= HF;
			SZHV_dec[i] = SZ[i] | NF;
			if (i == 0x7f) SZHV_dec[i] |= VF;
			if ((i & 0x0f) == 0x0f) SZHV_dec[i] |= HF;
		}

		tables_initialised = true;
	}

	save_item(NAME(m_prvpc.w.l));
	save_item(NAME(PC));
	save_item(NAME(SP));
	save_item(NAME(AF));
	save_item(NAME(BC));
	save_item(NAME(DE));
	save_item(NAME(HL));
	save_item(NAME(IX));
	save_item(NAME(IY));
	save_item(NAME(WZ));
	save_item(NAME(m_af2.w.l));
	save_item(NAME(m_bc2.w.l));
	save_item(NAME(m_de2.w.l));
	save_item(NAME(m_hl2.w.l));
	save_item(NAME(m_r));
	save_item(NAME(m_r2));
	save_item(NAME(m_q));
	save_item(NAME(m_qtemp));
	save_item(NAME(m_iff1));
	save_item(NAME(m_iff2));
	save_item(NAME(m_halt));
	save_item(NAME(m_im));
	save_item(NAME(m_i));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_wait_state));
	save_item(NAME(m_busrq_state));
	save_item(NAME(m_busack_state));
	save_item(NAME(m_after_ei));
	save_item(NAME(m_after_ldair));
	save_item(NAME(m_ref));
	save_item(NAME(m_tmp_irq_vector));
	save_item(NAME(m_shared_addr.w));
	save_item(NAME(m_shared_data.w));
	save_item(NAME(m_shared_data2.w));

	// Reset registers to their initial values
	PRVPC = 0;
	PCD = 0;
	SPD = 0;
	AFD = 0;
	BCD = 0;
	DED = 0;
	HLD = 0;
	IXD = 0;
	IYD = 0;
	WZ = 0;
	m_af2.d = 0;
	m_bc2.d = 0;
	m_de2.d = 0;
	m_hl2.d = 0;
	m_r = 0;
	m_r2 = 0;
	m_iff1 = 0;
	m_iff2 = 0;
	m_halt = 0;
	m_im = 0;
	m_i = 0;
	m_nmi_state = 0;
	m_nmi_pending = false;
	m_irq_state = 0;
	m_wait_state = 0;
	m_busrq_state = 0;
	m_busack_state = 0;
	m_after_ei = false;
	m_after_ldair = false;
	m_ea = 0;

	space(AS_PROGRAM).cache(m_args);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_opcodes);
	space(AS_PROGRAM).specific(m_data);
	space(AS_IO).specific(m_io);

	IX = IY = 0xffff; // IX and IY are FFFF after a reset!
	set_f(ZF);        // Zero flag is set

	// set up the state table
	state_add(STATE_GENPC,     "PC",        m_pc.w.l).callimport();
	state_add(STATE_GENPCBASE, "CURPC",     m_prvpc.w.l).callimport().noshow();
	state_add(Z80_SP,          "SP",        SP);
	state_add(STATE_GENFLAGS,  "GENFLAGS",  F).noshow().formatstr("%8s");
	state_add(Z80_A,           "A",         A).noshow();
	state_add(Z80_B,           "B",         B).noshow();
	state_add(Z80_C,           "C",         C).noshow();
	state_add(Z80_D,           "D",         D).noshow();
	state_add(Z80_E,           "E",         E).noshow();
	state_add(Z80_H,           "H",         H).noshow();
	state_add(Z80_L,           "L",         L).noshow();
	state_add(Z80_AF,          "AF",        AF);
	state_add(Z80_BC,          "BC",        BC);
	state_add(Z80_DE,          "DE",        DE);
	state_add(Z80_HL,          "HL",        HL);
	state_add(Z80_IX,          "IX",        IX);
	state_add(Z80_IY,          "IY",        IY);
	state_add(Z80_AF2,         "AF2",       m_af2.w.l);
	state_add(Z80_BC2,         "BC2",       m_bc2.w.l);
	state_add(Z80_DE2,         "DE2",       m_de2.w.l);
	state_add(Z80_HL2,         "HL2",       m_hl2.w.l);
	state_add(Z80_WZ,          "WZ",        WZ);
	state_add(Z80_R,           "R",         m_rtemp).callimport().callexport();
	state_add(Z80_I,           "I",         m_i);
	state_add(Z80_IM,          "IM",        m_im).mask(0x3);
	state_add(Z80_IFF1,        "IFF1",      m_iff1).mask(0x1);
	state_add(Z80_IFF2,        "IFF2",      m_iff2).mask(0x1);
	state_add(Z80_HALT,        "HALT",      m_halt).mask(0x1);

	// set our instruction counter
	set_icountptr(m_icount);
}

void nsc800_device::device_start()
{
	z80_device::device_start();

	save_item(NAME(m_nsc800_irq_state));
}

/****************************************************************************
 * Do a reset
 ****************************************************************************/
void z80_device::device_reset()
{
	leave_halt();

	m_ref = 0xffff00;
	PC = 0x0000;
	m_i = 0;
	m_r = 0;
	m_r2 = 0;
	m_nmi_pending = false;
	m_after_ei = false;
	m_after_ldair = false;
	m_iff1 = 0;
	m_iff2 = 0;

	WZ = PCD;
}

void nsc800_device::device_reset()
{
	z80_device::device_reset();
	memset(m_nsc800_irq_state, 0, sizeof(m_nsc800_irq_state));
}

void z80_device::do_op()
{
	#include "cpu/z80/z80.hxx"
}

void nsc800_device::do_op()
{
	#include "cpu/z80/ncs800.hxx"
}

/****************************************************************************
 * Execute 'cycles' T-states.
 ****************************************************************************/
void z80_device::execute_run()
{
	if (m_wait_state)
	{
		m_icount = 0; // stalled
		return;
	}

	while (m_icount > 0)
	{
		do_op();
	}
}

void z80_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case Z80_INPUT_LINE_BUSRQ:
		m_busrq_state = state;
		break;

	case INPUT_LINE_NMI:
		// mark an NMI pending on the rising edge
		if (m_nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			m_nmi_pending = true;
		m_nmi_state = state;
		break;

	case INPUT_LINE_IRQ0:
		// update the IRQ state via the daisy chain
		m_irq_state = state;
		if (daisy_chain_present())
			m_irq_state = (daisy_update_irq_state() == ASSERT_LINE) ? ASSERT_LINE : m_irq_state;

		// the main execute loop will take the interrupt
		break;

	case Z80_INPUT_LINE_WAIT:
		m_wait_state = state;
		break;

	default:
		break;
	}
}

void nsc800_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case NSC800_RSTA:
		m_nsc800_irq_state[NSC800_RSTA] = state;
		break;

	case NSC800_RSTB:
		m_nsc800_irq_state[NSC800_RSTB] = state;
		break;

	case NSC800_RSTC:
		m_nsc800_irq_state[NSC800_RSTC] = state;
		break;

	default:
		z80_device::execute_set_input(inputnum, state);
		break;
	}
}

/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/
void z80_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case STATE_GENPCBASE:
		m_pc = m_prvpc;
		[[fallthrough]];
	case STATE_GENPC:
		m_prvpc = m_pc;
		m_ref = 0xffff00;
		m_after_ei = false;
		m_after_ldair = false;
		break;

	case Z80_R:
		m_r = m_rtemp & 0x7f;
		m_r2 = m_rtemp & 0x80;
		break;

	default:
		fatalerror("CPU_IMPORT_STATE() called for unexpected value\n");
	}
}

void z80_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case Z80_R:
		m_rtemp = (m_r & 0x7f) | (m_r2 & 0x80);
		break;

	default:
		fatalerror("CPU_EXPORT_STATE() called for unexpected value\n");
	}
}

void z80_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				F & 0x80 ? 'S':'.',
				F & 0x40 ? 'Z':'.',
				F & 0x20 ? 'Y':'.',
				F & 0x10 ? 'H':'.',
				F & 0x08 ? 'X':'.',
				F & 0x04 ? 'P':'.',
				F & 0x02 ? 'N':'.',
				F & 0x01 ? 'C':'.');
			break;
	}
}

/**************************************************************************
 * disassemble - call the disassembly helper function
 **************************************************************************/
std::unique_ptr<util::disasm_interface> z80_device::create_disassembler()
{
	return std::make_unique<z80_disassembler>();
}

z80_device::z80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, Z80, tag, owner, clock)
{
}

z80_device::z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	cpu_device(mconfig, type, tag, owner, clock),
	z80_daisy_chain_interface(mconfig, *this),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0),
	m_opcodes_config("opcodes", ENDIANNESS_LITTLE, 8, 16, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0),
	m_irqack_cb(*this),
	m_refresh_cb(*this),
	m_nomreq_cb(*this),
	m_halt_cb(*this),
	m_busack_cb(*this),
	m_branch_cb(*this),
	m_irqfetch_cb(*this),
	m_reti_cb(*this),
	m_m1_cycles(4),
	m_memrq_cycles(3),
	m_iorq_cycles(4)
{
}

device_memory_interface::space_config_vector z80_device::memory_space_config() const
{
	if (has_configured_map(AS_OPCODES))
	{
		return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config),
				std::make_pair(AS_OPCODES, &m_opcodes_config),
				std::make_pair(AS_IO,      &m_io_config) };
	}
	else
	{
		return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config),
				std::make_pair(AS_IO,      &m_io_config) };
	}
}

DEFINE_DEVICE_TYPE(Z80, z80_device, "z80", "Zilog Z80")

nsc800_device::nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, NSC800, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(NSC800, nsc800_device, "nsc800", "National Semiconductor NSC800")
