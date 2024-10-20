// license:BSD-3-Clause
// copyright-holders:hap

// HMCS400 opcode handlers

#include "emu.h"
#include "hmcs400.h"


// internal helpers

inline u8 hmcs400_cpu_device::ram_r(u8 mem_mask)
{
	return m_data->read_byte(m_param & 0x3ff, mem_mask & 0xf) & 0xf;
}

inline void hmcs400_cpu_device::ram_w(u8 data, u8 mem_mask)
{
	m_data->write_byte(m_param & 0x3ff, data & 0xf, mem_mask & 0xf);
}

void hmcs400_cpu_device::pop_stack()
{
	u16 data = 0;
	for (int i = 0; i < 4; i++)
	{
		m_sp = ((m_sp + 1) | 0x3c0) & 0x3ff;
		data = data << 4 | (m_data->read_byte(m_sp) & 0xf);
	}

	if (m_op & 1)
	{
		// RTNI restores CA and ST
		m_ca = BIT(data, 7);
		m_st = BIT(data, 15);
	}
	m_pc = (~data & 0x7f00) >> 1 | (~data & 0x7f);
}

void hmcs400_cpu_device::push_stack()
{
	u16 data = (~m_pc << 1 & 0x7f00) | (~m_pc & 0x7f) | m_ca << 7 | m_st << 15;
	m_sp = (m_sp | 0x3c0) & 0x3ff;

	for (int i = 0; i < 4; i++)
	{
		m_data->write_byte(m_sp, data & 0xf);
		data >>= 4;
		m_sp = ((m_sp - 1) | 0x3c0) & 0x3ff;
	}
}


// instruction set

void hmcs400_cpu_device::op_illegal()
{
	logerror("unknown opcode $%03X @ $%04X\n", m_op, m_prev_pc);
}

void hmcs400_cpu_device::op_todo()
{
	logerror("unimplemented opcode $%03X @ $%04X\n", m_op, m_prev_pc);
}


// immediate instructions

void hmcs400_cpu_device::op_lai()
{
	// LAI i: Load A from Immediate
	m_a = m_i;
}

void hmcs400_cpu_device::op_lbi()
{
	// LBI i: Load B from Immediate
	m_b = m_i;
}

void hmcs400_cpu_device::op_lmi()
{
	// LMID i,d: Load Memory from Immediate
	ram_w(m_i);
}

void hmcs400_cpu_device::op_lmiiy()
{
	// LMIIY i: Load Memory from Immediate, Increment Y
	op_lmi();
	op_iy();
}


// register-to-register instructions

void hmcs400_cpu_device::op_lab()
{
	// LAB: Load A from B
	m_a = m_b;
}

void hmcs400_cpu_device::op_lba()
{
	// LBA: Load B from A
	m_b = m_a;
}

void hmcs400_cpu_device::op_law()
{
	// LAW: Load A from W
	m_a = m_w;
}

void hmcs400_cpu_device::op_lay()
{
	// LAY: Load A from Y
	m_a = m_y;
}

void hmcs400_cpu_device::op_laspx()
{
	// LASPX: Load A from SPX
	m_a = m_spx;
}

void hmcs400_cpu_device::op_laspy()
{
	// LASPY: Load A from SPY
	m_a = m_spy;
}

void hmcs400_cpu_device::op_lamr()
{
	// LAMR m: Load A from MR
	m_param = 0x20 | m_i;
	m_a = ram_r();
}

void hmcs400_cpu_device::op_xmra()
{
	// XMRA m: Exchange MR and A
	m_param = 0x20 | m_i;
	u8 old_a = m_a;
	m_a = ram_r();
	ram_w(old_a);
}


// RAM address instructions

void hmcs400_cpu_device::op_lwi()
{
	// LWI i: Load W from Immediate
	m_w = m_i;
}

void hmcs400_cpu_device::op_lxi()
{
	// LXI i: Load X from Immediate
	m_x = m_i;
}

void hmcs400_cpu_device::op_lyi()
{
	// LYI i: Load Y from Immediate
	m_y = m_i;
}

void hmcs400_cpu_device::op_lwa()
{
	// LWA: Load W from A
	m_w = m_a & 3;
}

void hmcs400_cpu_device::op_lxa()
{
	// LXA: Load X from A
	m_x = m_a;
}

void hmcs400_cpu_device::op_lya()
{
	// LYA: Load Y from A
	m_y = m_a;
}

void hmcs400_cpu_device::op_iy()
{
	// IY: Increment Y
	m_y = (m_y + 1) & 0xf;
	m_st = (m_y != 0);
}

void hmcs400_cpu_device::op_dy()
{
	// DY: Decrement Y
	m_y = (m_y - 1) & 0xf;
	m_st = (m_y != 0xf);
}

void hmcs400_cpu_device::op_ayy()
{
	// AYY: Add A to Y
	m_y += m_a;
	m_st = BIT(m_y, 4);
	m_y &= 0xf;
}

void hmcs400_cpu_device::op_syy()
{
	// SYY: Subtract A from Y
	m_y -= m_a;
	m_st = BIT(~m_y, 4);
	m_y &= 0xf;
}

void hmcs400_cpu_device::op_xsp()
{
	// XSP(XY): Exchange X and SPX, Y and SPY, or NOP if 0
	if (m_op & 1)
	{
		u8 old_x = m_x;
		m_x = m_spx;
		m_spx = old_x;
	}
	if (m_op & 2)
	{
		u8 old_y = m_y;
		m_y = m_spy;
		m_spy = old_y;
	}
}


// RAM register instructions

void hmcs400_cpu_device::op_lam()
{
	// LAM(XY) / LAMD d: Load A from Memory
	m_a = ram_r();
	op_xsp();
}

void hmcs400_cpu_device::op_lbm()
{
	// LBM(XY): Load B from Memory
	m_b = ram_r();
	op_xsp();
}

void hmcs400_cpu_device::op_lma()
{
	// LMA(XY) / LMAD d: Load Memory from A
	ram_w(m_a);
	op_xsp();
}

void hmcs400_cpu_device::op_lmaiy()
{
	// LMAIY(X): Load Memory from A, Increment Y
	op_lma();
	op_iy();
}

void hmcs400_cpu_device::op_lmady()
{
	// LMADY(X): Load Memory from A, Decrement Y
	op_lma();
	op_dy();
}

void hmcs400_cpu_device::op_xma()
{
	// XMA(XY) / XMAD d: Exchange Memory and A
	u8 old_a = m_a;
	m_a = ram_r();
	ram_w(old_a);
	op_xsp();
}

void hmcs400_cpu_device::op_xmb()
{
	// XMB(XY): Exchange Memory and B
	u8 old_b = m_b;
	m_b = ram_r();
	ram_w(old_b);
	op_xsp();
}


// arithmetic instructions

void hmcs400_cpu_device::op_ai()
{
	// AI i: Add Immediate to A
	m_a += m_i;
	m_st = BIT(m_a, 4);
	m_a &= 0xf;
}

void hmcs400_cpu_device::op_ib()
{
	// IB: Increment B
	m_b = (m_b + 1) & 0xf;
	m_st = (m_b != 0);
}

void hmcs400_cpu_device::op_db()
{
	// DB: Decrement B
	m_b = (m_b - 1) & 0xf;
	m_st = (m_b != 0xf);
}

void hmcs400_cpu_device::op_daa()
{
	// DAA: Decimal Adjust for Addition
	if (m_ca || m_a > 9)
	{
		m_a = (m_a + 6) & 0xf;
		m_ca = 1;
	}
}

void hmcs400_cpu_device::op_das()
{
	// DAS: Decimal Adjust for Subtraction
	if (!m_ca || m_a > 9)
	{
		m_a = (m_a + 10) & 0xf;
		m_ca = 0;
	}
}

void hmcs400_cpu_device::op_nega()
{
	// NEGA: Negate A
	m_a = (0 - m_a) & 0xf;
}

void hmcs400_cpu_device::op_comb()
{
	// COMB: Complement B
	m_b ^= 0xf;
}

void hmcs400_cpu_device::op_rotr()
{
	// ROTR: Rotate Right with Carry
	u8 ca = m_a & 1;
	m_a = m_a >> 1 | m_ca << 3;
	m_ca = ca;
}

void hmcs400_cpu_device::op_rotl()
{
	// ROTL: Rotate Left with Carry
	m_a = m_a << 1 | m_ca;
	m_ca = BIT(m_a, 4);
	m_a &= 0xf;
}

void hmcs400_cpu_device::op_sec()
{
	// SEC: Set Carry
	m_ca = 1;
}

void hmcs400_cpu_device::op_rec()
{
	// REC: Reset Carry
	m_ca = 0;
}

void hmcs400_cpu_device::op_tc()
{
	// TC: Test Carry
	m_st = m_ca;
}

void hmcs400_cpu_device::op_am()
{
	// AM / AMD d: Add A to Memory
	m_a += ram_r();
	m_st = BIT(m_a, 4);
	m_a &= 0xf;
}

void hmcs400_cpu_device::op_amc()
{
	// AMC / AMCD d: Add A to Memory with Carry
	m_a += ram_r() + m_ca;
	m_ca = BIT(m_a, 4);
	m_st = m_ca;
	m_a &= 0xf;
}

void hmcs400_cpu_device::op_smc()
{
	// SMC / SMCD d: Subtract A from Memory with Carry
	m_a = ram_r() - m_a - (m_ca ^ 1);
	m_ca = BIT(~m_a, 4);
	m_st = m_ca;
	m_a &= 0xf;
}

void hmcs400_cpu_device::op_or()
{
	// OR: Or A with B
	m_a |= m_b;
}

void hmcs400_cpu_device::op_anm()
{
	// ANM / ANMD d: And Memory with A
	m_a &= ram_r();
	m_st = (m_a != 0);
}

void hmcs400_cpu_device::op_orm()
{
	// ORM / ORMD d: Or Memory with A
	m_a |= ram_r();
	m_st = (m_a != 0);
}

void hmcs400_cpu_device::op_eorm()
{
	// EORM / EORMD d: Exclusive Or Memory with A
	m_a ^= ram_r();
	m_st = (m_a != 0);
}


// compare instructions

void hmcs400_cpu_device::op_inem()
{
	// INEM i / INEMD i,d: Immediate Not Equal to Memory
	m_st = (m_i != ram_r());
}

void hmcs400_cpu_device::op_anem()
{
	// ANEM / ANEMD d: A Not Equal to Memory
	m_st = (m_a != ram_r());
}

void hmcs400_cpu_device::op_bnem()
{
	// BNEM: B Not Equal to Memory
	m_st = (m_b != ram_r());
}

void hmcs400_cpu_device::op_ynei()
{
	// YNEI i: Y Not Equal to Immediate
	m_st = (m_y != m_i);
}

void hmcs400_cpu_device::op_ilem()
{
	// ILEM i / ILEMD i,d: Immediate Less or Equal to Memory
	m_st = (m_i <= ram_r());
}

void hmcs400_cpu_device::op_alem()
{
	// ALEM / ALEMD d: A Less or Equal to Memory
	m_st = (m_a <= ram_r());
}

void hmcs400_cpu_device::op_blem()
{
	// BLEM: B Less or Equal to Memory
	m_st = (m_b <= ram_r());
}

void hmcs400_cpu_device::op_alei()
{
	// ALEI i: A Less or Equal to Immediate
	m_st = (m_a <= m_i);
}


// RAM bit manipulation instructions

void hmcs400_cpu_device::op_sem()
{
	// SEM n / SEMD n,d: Set Memory Bit
	u8 mask = 1 << (m_op & 3);
	ram_w(ram_r(~mask) | mask, mask);
}

void hmcs400_cpu_device::op_rem()
{
	// REM n / REMD n,d: Reset Memory Bit
	u8 mask = 1 << (m_op & 3);
	ram_w(ram_r(~mask) & ~mask, mask);
}

void hmcs400_cpu_device::op_tm()
{
	// TM n / TMD n,d: Test Memory Bit
	u8 mask = 1 << (m_op & 3);
	m_st = (ram_r(mask) & mask) ? 1 : 0;
}


// ROM address instructions

void hmcs400_cpu_device::op_br()
{
	// BR b: Branch on Status 1
	if (m_st)
		m_pc = (m_pc & ~0xff) | (m_op & 0xff);
	else
		m_st = 1;
}

void hmcs400_cpu_device::op_brl()
{
	// BRL u: Long Branch on Status 1
	if (m_st)
		op_jmpl();
	else
		m_st = 1;
}

void hmcs400_cpu_device::op_jmpl()
{
	// JMPL u: Long Jump Unconditionally
	m_pc = m_i << 10 | m_param;
}

void hmcs400_cpu_device::op_cal()
{
	// CAL a: Subroutine Jump on Status 1
	if (m_st)
	{
		cycle();
		push_stack();
		m_pc = m_op & 0x3f;
	}
	else
		m_st = 1;
}

void hmcs400_cpu_device::op_call()
{
	// CALL u: Long Subroutine Jump on Status 1
	if (m_st)
	{
		push_stack();
		op_jmpl();
	}
	else
		m_st = 1;
}

void hmcs400_cpu_device::op_tbr()
{
	// TBR p: Table Branch
	m_pc = m_i << 8 | m_b << 4 | m_a;
}

void hmcs400_cpu_device::op_rtn()
{
	// RTN: Return from Subroutine
	cycle();
	cycle();
	pop_stack();
}

void hmcs400_cpu_device::op_rtni()
{
	// RTNI: Return from Interrupt
	op_rtn();
	m_irq_flags |= 1;
}


// input/output instructions

void hmcs400_cpu_device::op_sed()
{
	// SED: Set Discrete I/O Latch
	write_d(m_y, 1);
}

void hmcs400_cpu_device::op_sedd()
{
	// SEDD m: Set Discrete I/O Latch Direct
	write_d(m_i, 1);
}

void hmcs400_cpu_device::op_red()
{
	// RED: Reset Discrete I/O Latch
	write_d(m_y, 0);
}

void hmcs400_cpu_device::op_redd()
{
	// REDD m: Reset Discrete I/O Latch Direct
	write_d(m_i, 0);
}

void hmcs400_cpu_device::op_td()
{
	// TD: Test Discrete I/O Latch
	m_st = read_d(m_y);
}

void hmcs400_cpu_device::op_tdd()
{
	// TDD m: Test Discrete I/O Latch Direct
	m_st = read_d(m_i);
}

void hmcs400_cpu_device::op_lar()
{
	// LAR m: Load A from R Port Register
	m_a = read_r(m_i);
}

void hmcs400_cpu_device::op_lbr()
{
	// LBR m: Load B from R Port Register
	m_b = read_r(m_i);
}

void hmcs400_cpu_device::op_lra()
{
	// LRA m: Load R Port Register from A
	write_r(m_i, m_a);
}

void hmcs400_cpu_device::op_lrb()
{
	// LRB m: Load R Port Register from B
	write_r(m_i, m_b);
}

void hmcs400_cpu_device::op_p()
{
	// P p: Pattern Generation
	cycle();
	u16 data = m_program->read_word(m_i << 8 | m_b << 4 | m_a);

	// destination is determined by the 2 highest bits
	if (data & 0x100)
	{
		// to A/B registers
		m_a = data & 0xf;
		m_b = data >> 4 & 0xf;
	}
	if (data & 0x200)
	{
		// to R1/R2 ports
		write_r(1, data & 0xf);
		write_r(2, data >> 4 & 0xf);
	}
}


// control instructions

void hmcs400_cpu_device::op_sts()
{
	// STS: Start Serial
	op_todo();
}

void hmcs400_cpu_device::op_sby()
{
	// SBY: Standby Mode
	m_standby = true;
}

void hmcs400_cpu_device::op_stop()
{
	// STOP: Stop Mode
	m_stop = true;

	if (m_icount > 0)
		m_icount = 0;

	// all I/O pins go high-impedance
	m_stop_cb(1);
}
