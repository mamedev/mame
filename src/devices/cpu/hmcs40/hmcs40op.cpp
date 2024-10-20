// license:BSD-3-Clause
// copyright-holders:hap

// HMCS40 opcode handlers

#include "emu.h"
#include "hmcs40.h"


// internal helpers

inline u8 hmcs40_cpu_device::ram_r()
{
	u8 address = (m_x << 4 | m_y) & m_datamask;
	return m_data->read_byte(address) & 0xf;
}

inline void hmcs40_cpu_device::ram_w(u8 data)
{
	u8 address = (m_x << 4 | m_y) & m_datamask;
	m_data->write_byte(address, data & 0xf);
}

void hmcs40_cpu_device::pop_stack()
{
	m_pc = m_stack[0] & m_pcmask;
	for (int i = 0; i < m_stack_levels-1; i++)
		m_stack[i] = m_stack[i+1];
}

void hmcs40_cpu_device::push_stack()
{
	for (int i = m_stack_levels-1; i >= 1; i--)
		m_stack[i] = m_stack[i-1];
	m_stack[0] = m_pc;
}


// instruction set

void hmcs40_cpu_device::op_illegal()
{
	logerror("unknown opcode $%03X @ $%04X\n", m_op, m_prev_pc);
}


// register-to-register instructions

void hmcs40_cpu_device::op_lab()
{
	// LAB: Load A from B
	m_a = m_b;
}

void hmcs40_cpu_device::op_lba()
{
	// LBA: Load B from A
	m_b = m_a;
}

void hmcs40_cpu_device::op_lay()
{
	// LAY: Load A from Y
	m_a = m_y;
}

void hmcs40_cpu_device::op_laspx()
{
	// LASPX: Load A from SPX
	m_a = m_spx;
}

void hmcs40_cpu_device::op_laspy()
{
	// LASPY: Load A from SPY
	m_a = m_spy;
}

void hmcs40_cpu_device::op_xamr()
{
	// XAMR m: Exchange A and MR(m)

	// determine MR(Memory Register) location
	u8 address = m_op & 0xf;

	// HMCS42: MR0 on file 0, MR4-MR15 on file 4 (there is no file 1-3)
	// HMCS43: MR0-MR3 on file 0-3, MR4-MR15 on file 4
	if (m_family == HMCS42_FAMILY || m_family == HMCS43_FAMILY)
		address |= (address < 4) ? (address << 4) : 0x40;

	// HMCS44/45/46/47: all on last file
	else
		address |= 0xf0;

	address &= m_datamask;
	u8 old_a = m_a;
	m_a = m_data->read_byte(address) & 0xf;
	m_data->write_byte(address, old_a & 0xf);
}


// RAM address instructions

void hmcs40_cpu_device::op_lxa()
{
	// LXA: Load X from A
	m_x = m_a;
}

void hmcs40_cpu_device::op_lya()
{
	// LYA: Load Y from A
	m_y = m_a;
}

void hmcs40_cpu_device::op_lxi()
{
	// LXI i: Load X from Immediate
	m_x = m_i;
}

void hmcs40_cpu_device::op_lyi()
{
	// LYI i: Load Y from Immediate
	m_y = m_i;
}

void hmcs40_cpu_device::op_iy()
{
	// IY: Increment Y
	m_y = (m_y + 1) & 0xf;
	m_s = (m_y != 0);
}

void hmcs40_cpu_device::op_dy()
{
	// DY: Decrement Y
	m_y = (m_y - 1) & 0xf;
	m_s = (m_y != 0xf);
}

void hmcs40_cpu_device::op_ayy()
{
	// AYY: Add A to Y
	m_y += m_a;
	m_s = BIT(m_y, 4);
	m_y &= 0xf;
}

void hmcs40_cpu_device::op_syy()
{
	// SYY: Subtract A from Y
	m_y -= m_a;
	m_s = BIT(~m_y, 4);
	m_y &= 0xf;
}

void hmcs40_cpu_device::op_xsp()
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

void hmcs40_cpu_device::op_lam()
{
	// LAM(XY): Load A from Memory
	m_a = ram_r();
	op_xsp();
}

void hmcs40_cpu_device::op_lbm()
{
	// LBM(XY): Load B from Memory
	m_b = ram_r();
	op_xsp();
}

void hmcs40_cpu_device::op_xma()
{
	// XMA(XY): Exchange Memory and A
	u8 old_a = m_a;
	m_a = ram_r();
	ram_w(old_a);
	op_xsp();
}

void hmcs40_cpu_device::op_xmb()
{
	// XMB(XY): Exchange Memory and B
	u8 old_b = m_b;
	m_b = ram_r();
	ram_w(old_b);
	op_xsp();
}

void hmcs40_cpu_device::op_lmaiy()
{
	// LMAIY(X): Load Memory from A, Increment Y
	ram_w(m_a);
	op_iy();
	op_xsp();
}

void hmcs40_cpu_device::op_lmady()
{
	// LMADY(X): Load Memory from A, Decrement Y
	ram_w(m_a);
	op_dy();
	op_xsp();
}


// immediate instructions

void hmcs40_cpu_device::op_lmiiy()
{
	// LMIIY i: Load Memory from Immediate, Increment Y
	ram_w(m_i);
	op_iy();
}

void hmcs40_cpu_device::op_lai()
{
	// LAI i: Load A from Immediate
	m_a = m_i;
}

void hmcs40_cpu_device::op_lbi()
{
	// LBI i: Load B from Immediate
	m_b = m_i;
}


// arithmetic instructions

void hmcs40_cpu_device::op_ai()
{
	// AI i: Add Immediate to A
	m_a += m_i;
	m_s = BIT(m_a, 4);
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_ib()
{
	// IB: Increment B
	m_b = (m_b + 1) & 0xf;
	m_s = (m_b != 0);
}

void hmcs40_cpu_device::op_db()
{
	// DB: Decrement B
	m_b = (m_b - 1) & 0xf;
	m_s = (m_b != 0xf);
}

void hmcs40_cpu_device::op_amc()
{
	// AMC: Add A to Memory with Carry
	m_a += ram_r() + m_c;
	m_c = BIT(m_a, 4);
	m_s = m_c;
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_smc()
{
	// SMC: Subtract A from Memory with Carry
	m_a = ram_r() - m_a - (m_c ^ 1);
	m_c = BIT(~m_a, 4);
	m_s = m_c;
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_am()
{
	// AM: Add A to Memory
	m_a += ram_r();
	m_s = BIT(m_a, 4);
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_daa()
{
	// DAA: Decimal Adjust for Addition
	if (m_c || m_a > 9)
	{
		m_a = (m_a + 6) & 0xf;
		m_c = 1;
	}
}

void hmcs40_cpu_device::op_das()
{
	// DAS: Decimal Adjust for Subtraction
	if (!m_c || m_a > 9)
	{
		m_a = (m_a + 10) & 0xf;
		m_c = 0;
	}
}

void hmcs40_cpu_device::op_nega()
{
	// NEGA: Negate A
	m_a = (0 - m_a) & 0xf;
}

void hmcs40_cpu_device::op_comb()
{
	// COMB: Complement B
	m_b ^= 0xf;
}

void hmcs40_cpu_device::op_sec()
{
	// SEC: Set Carry
	m_c = 1;
}

void hmcs40_cpu_device::op_rec()
{
	// REC: Reset Carry
	m_c = 0;
}

void hmcs40_cpu_device::op_tc()
{
	// TC: Test Carry
	m_s = m_c;
}

void hmcs40_cpu_device::op_rotl()
{
	// ROTL: Rotate Left A with Carry
	m_a = m_a << 1 | m_c;
	m_c = BIT(m_a, 4);
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_rotr()
{
	// ROTR: Rotate Right A with Carry
	u8 c = m_a & 1;
	m_a = m_a >> 1 | m_c << 3;
	m_c = c;
}

void hmcs40_cpu_device::op_or()
{
	// OR: Or A with B
	m_a |= m_b;
}


// compare instructions

void hmcs40_cpu_device::op_mnei()
{
	// MNEI i: Memory Not Equal to Immediate
	m_s = (m_i != ram_r());
}

void hmcs40_cpu_device::op_ynei()
{
	// YNEI i: Y Not Equal to Immediate
	m_s = (m_y != m_i);
}

void hmcs40_cpu_device::op_anem()
{
	// ANEM: A Not Equal to Memory
	m_s = (m_a != ram_r());
}

void hmcs40_cpu_device::op_bnem()
{
	// BNEM: B Not Equal to Memory
	m_s = (m_b != ram_r());
}

void hmcs40_cpu_device::op_alei()
{
	// ALEI i: A Less or Equal to Immediate
	m_s = (m_a <= m_i);
}

void hmcs40_cpu_device::op_alem()
{
	// ALEM: A Less or Equal to Memory
	m_s = (m_a <= ram_r());
}

void hmcs40_cpu_device::op_blem()
{
	// BLEM: B Less or Equal to Memory
	m_s = (m_b <= ram_r());
}


// RAM bit manipulation instructions

void hmcs40_cpu_device::op_sem()
{
	// SEM n: Set Memory Bit
	ram_w(ram_r() | (1 << (m_op & 3)));
}

void hmcs40_cpu_device::op_rem()
{
	// REM n: Reset Memory Bit
	ram_w(ram_r() & ~(1 << (m_op & 3)));
}

void hmcs40_cpu_device::op_tm()
{
	// TM n: Test Memory Bit
	m_s = BIT(ram_r(), m_op & 3);
}


// ROM address instructions

void hmcs40_cpu_device::op_br()
{
	// BR a: Branch on Status 1
	if (m_s)
		m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
	else
		m_s = 1;
}

void hmcs40_cpu_device::op_cal()
{
	// CAL a: Subroutine Jump on Status 1
	if (m_s)
	{
		m_block_int = true;
		push_stack();
		m_pc = m_op & 0x3f; // short calls default to page 0
	}
	else
		m_s = 1;
}

void hmcs40_cpu_device::op_lpu()
{
	// LPU u: Load Program Counter Upper on Status 1
	if (m_s)
	{
		m_block_int = true;
		m_pc_upper = m_op & 0x1f;

		// on HMCS46/47, also latches bank from R70
		if (m_family == HMCS46_FAMILY || m_family == HMCS47_FAMILY)
			m_pc_upper |= ~m_r[7] << 5 & 0x20;
	}
	else
		m_op |= 0x400; // indicate unhandled LPU
}

void hmcs40_cpu_device::op_tbr()
{
	// TBR p: Table Branch
	u16 address = m_a | m_b << 4 | m_c << 8 | (m_op & 7) << 9 | (m_pc & ~0x3f);
	m_pc = address & m_pcmask;
}

void hmcs40_cpu_device::op_rtn()
{
	// RTN: Return from Subroutine
	pop_stack();
}


// interrupt instructions

void hmcs40_cpu_device::op_seie()
{
	// SEIE: Set I/E
	m_ie = 1;
}

void hmcs40_cpu_device::op_seif0()
{
	// SEIF0: Set IF0
	m_if[0] = 1;
}

void hmcs40_cpu_device::op_seif1()
{
	// SEIF1: Set IF1
	m_if[1] = 1;
}

void hmcs40_cpu_device::op_setf()
{
	// SETF: Set TF
	m_tf = 1;
}

void hmcs40_cpu_device::op_secf()
{
	// SECF: Set CF
	m_cf = 1;
}

void hmcs40_cpu_device::op_reie()
{
	// REIE: Reset I/E
	m_ie = 0;
}

void hmcs40_cpu_device::op_reif0()
{
	// REIF0: Reset IF0
	m_if[0] = 0;
}

void hmcs40_cpu_device::op_reif1()
{
	// REIF1: Reset IF1
	m_if[1] = 0;
}

void hmcs40_cpu_device::op_retf()
{
	// RETF: Reset TF
	m_tf = 0;
}

void hmcs40_cpu_device::op_recf()
{
	// RECF: Reset CF
	m_cf = 0;
}

void hmcs40_cpu_device::op_ti0()
{
	// TI0: Test INT0
	m_s = m_int[0];
}

void hmcs40_cpu_device::op_ti1()
{
	// TI1: Test INT1
	m_s = m_int[1];
}

void hmcs40_cpu_device::op_tif0()
{
	// TIF0: Test IF0
	m_s = m_if[0];
}

void hmcs40_cpu_device::op_tif1()
{
	// TIF1: Test IF1
	m_s = m_if[1];
}

void hmcs40_cpu_device::op_ttf()
{
	// TTF: Test TF
	m_s = m_tf;
}

void hmcs40_cpu_device::op_lti()
{
	// LTI i: Load Timer/Counter from Immediate
	m_tc = m_i;
	m_prescaler = 0;
}

void hmcs40_cpu_device::op_lta()
{
	// LTA: Load Timer/Counter from A
	m_tc = m_a;
	m_prescaler = 0;
}

void hmcs40_cpu_device::op_lat()
{
	// LAT: Load A from Timer/Counter
	m_a = m_tc;
}

void hmcs40_cpu_device::op_rtni()
{
	// RTNI: Return from Interrupt
	op_seie();
	op_rtn();
}


// input/output instructions

void hmcs40_cpu_device::op_sed()
{
	// SED: Set Discrete I/O Latch
	write_d(m_y, 1);
}

void hmcs40_cpu_device::op_red()
{
	// RED: Reset Discrete I/O Latch
	write_d(m_y, 0);
}

void hmcs40_cpu_device::op_td()
{
	// TD: Test Discrete I/O Latch
	m_s = read_d(m_y);
}

void hmcs40_cpu_device::op_sedd()
{
	// SEDD n: Set Discrete I/O Latch Direct
	write_d(m_op & 3, 1);
}

void hmcs40_cpu_device::op_redd()
{
	// REDD n: Reset Discrete I/O Latch Direct
	write_d(m_op & 3, 0);
}

void hmcs40_cpu_device::op_lar()
{
	// LAR p: Load A from R-Port Register
	m_a = read_r(m_op & 7);
}

void hmcs40_cpu_device::op_lbr()
{
	// LBR p: Load B from R-Port Register
	m_b = read_r(m_op & 7);
}

void hmcs40_cpu_device::op_lra()
{
	// LRA p: Load R-Port Register from A
	write_r(m_op & 7, m_a);
}

void hmcs40_cpu_device::op_lrb()
{
	// LRB p: Load R-Port Register from B
	write_r(m_op & 7, m_b);
}

void hmcs40_cpu_device::op_p()
{
	// P p: Pattern Generation
	cycle();

	u16 address = m_a | m_b << 4 | m_c << 8 | (m_op & 7) << 9 | (m_pc & ~0x3f);
	u16 data = m_program->read_word(address & m_prgmask);

	// destination is determined by the 2 highest bits
	if (data & 0x100)
	{
		// B3 B2 B1 B0 A0 A1 A2 A3
		m_a = bitswap<4>(data,0,1,2,3);
		m_b = data >> 4 & 0xf;
	}
	if (data & 0x200)
	{
		// R20 R21 R22 R23 R30 R31 R32 R33
		data = bitswap<8>(data,0,1,2,3,4,5,6,7);
		write_r(2, data & 0xf);
		write_r(3, data >> 4 & 0xf);
	}
}
