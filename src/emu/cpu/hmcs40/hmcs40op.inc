// HMCS40 opcode handlers

// internal helpers

inline UINT8 hmcs40_cpu_device::ram_r()
{
	UINT8 address = (m_x << 4 | m_y) & m_datamask;
	return m_data->read_byte(address) & 0xf;
}

inline void hmcs40_cpu_device::ram_w(UINT8 data)
{
	UINT8 address = (m_x << 4 | m_y) & m_datamask;
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


// i/o

UINT8 hmcs40_cpu_device::read_r(int index)
{
	index &= 7;
	UINT8 inp = 0xf;
	
	switch (index)
	{
		case 0: inp = m_read_r0(index, 0xff); break;
		case 1: inp = m_read_r1(index, 0xff); break;
		case 2: inp = m_read_r2(index, 0xff); break;
		case 3: inp = m_read_r3(index, 0xff); break;
		case 4: inp = m_read_r4(index, 0xff); break;
		case 5: inp = m_read_r5(index, 0xff); break;
		case 6: inp = m_read_r6(index, 0xff); break;
		case 7: inp = m_read_r7(index, 0xff); break;
	}
	
	if (m_is_cmos)
		return inp & m_r[index];
	else
		return inp | m_r[index];
}

void hmcs40_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;
	m_r[index] = data & 0xf;
	
	switch (index)
	{
		case 0: m_write_r0(index, m_r[index], 0xff); break;
		case 1: m_write_r1(index, m_r[index], 0xff); break;
		case 2: m_write_r2(index, m_r[index], 0xff); break;
		case 3: m_write_r3(index, m_r[index], 0xff); break;
		case 4: m_write_r4(index, m_r[index], 0xff); break;
		case 5: m_write_r5(index, m_r[index], 0xff); break;
		case 6: m_write_r6(index, m_r[index], 0xff); break;
		case 7: m_write_r7(index, m_r[index], 0xff); break;
	}
}

int hmcs40_cpu_device::read_d(int index)
{
	index &= 15;
	
	if (m_is_cmos)
		return (m_read_d(index, 0xffff) & m_d) >> index & 1;
	else
		return (m_read_d(index, 0xffff) | m_d) >> index & 1;
}

void hmcs40_cpu_device::write_d(int index, int state)
{
	index &= 15;
	
	m_d = (m_d & ~(1 << index)) | (state << index);
	m_write_d(index, m_d, 0xffff);
}

// HMCS43:
// R0 is input-only, R1 is i/o, R2,R3 are output-only, no R4-R7
// D0-D3 are i/o, D4-D15 are output-only

UINT8 hmcs43_cpu_device::read_r(int index)
{
	if ((index & 7) >= 4)
		logerror("%s read from unknown port R%d at $%04X\n", tag(), index & 7, m_prev_pc << 1);
	
	return hmcs40_cpu_device::read_r(index);
}

void hmcs43_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;

	if (index != 0 && index < 4)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("%s ineffective write to port R%d = $%X at $%04X\n", tag(), index, data & 0xf, m_prev_pc << 1);
}

int hmcs43_cpu_device::read_d(int index)
{
	if ((index & 15) >= 4)
		return m_d >> index & 1;
	else
		return hmcs40_cpu_device::read_d(index);
}

// HMCS44:
// R0-R3 are i/o, R4,R5 are extra registers, no R6,R7
// D0-D15 are i/o

UINT8 hmcs44_cpu_device::read_r(int index)
{
	if ((index & 7) >= 6)
		logerror("%s read from unknown port R%d at $%04X\n", tag(), index & 7, m_prev_pc << 1);
	
	return hmcs40_cpu_device::read_r(index);
}

void hmcs44_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;

	if (index < 6)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("%s ineffective write to port R%d = $%X at $%04X\n", tag(), index, data & 0xf, m_prev_pc << 1);
}

// HMCS45:
// R0-R5 are i/o, R6 is output-only, no R7
// D0-D15 are i/o

UINT8 hmcs45_cpu_device::read_r(int index)
{
	if ((index & 7) == 7)
		logerror("%s read from unknown port R%d at $%04X\n", tag(), index & 7, m_prev_pc << 1);
	
	return hmcs40_cpu_device::read_r(index);
}

void hmcs45_cpu_device::write_r(int index, UINT8 data)
{
	index &= 7;

	if (index != 7)
		hmcs40_cpu_device::write_r(index, data);
	else
		logerror("%s ineffective write to port R%d = $%X at $%04X\n", tag(), index, data & 0xf, m_prev_pc << 1);
}





// instruction set

// Register-to-Register Instruction

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
	UINT8 y = m_op & 0xf;
	UINT8 x = (y > 3) ? 0xf : (y + 12);
	UINT8 address = (x << 4 | y) & m_datamask;
	
	UINT8 old_a = m_a;
	m_a = m_data->read_byte(address) & 0xf;
	m_data->write_byte(address, old_a & 0xf);
}


// RAM Address Instruction

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
	m_x = m_op & 0xf;
}

void hmcs40_cpu_device::op_lyi()
{
	// LYI i: Load Y from Immediate
	m_y = m_op & 0xf;
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
	m_s = m_y >> 4 & 1;
	m_y &= 0xf;
}

void hmcs40_cpu_device::op_syy()
{
	// SYY: Subtract A from Y
	m_y -= m_a;
	m_s = ~m_y >> 4 & 1;
	m_y &= 0xf;
}

void hmcs40_cpu_device::op_xspx()
{
	// XSPX: Exchange X and SPX
	UINT8 old_x = m_x;
	m_x = m_spx;
	m_spx = old_x;
}

void hmcs40_cpu_device::op_xspy()
{
	// XSPY: Exchange Y and SPY
	UINT8 old_y = m_y;
	m_y = m_spy;
	m_spy = old_y;
}

void hmcs40_cpu_device::op_xspxy()
{
	// XSPXY: Exchange X and SPX, Y and SPY
	op_xspx();
	op_xspy();
}


// Ram Register Instruction

void hmcs40_cpu_device::op_lam()
{
	// LAM (XY): Load A from Memory
	m_a = ram_r();
	op_xspxy();
}

void hmcs40_cpu_device::op_lbm()
{
	// LBM (XY): Load B from Memory
	m_b = ram_r();
	op_xspxy();
}

void hmcs40_cpu_device::op_xma()
{
	// XMA (XY): Exchange Memory and A
	UINT8 old_a = m_a;
	m_a = ram_r();
	ram_w(old_a);
	op_xspxy();
}

void hmcs40_cpu_device::op_xmb()
{
	// XMB (XY): Exchange Memory and B
	UINT8 old_b = m_b;
	m_b = ram_r();
	ram_w(old_b);
	op_xspxy();
}

void hmcs40_cpu_device::op_lmaiy()
{
	// LMAIY (X): Load Memory from A, Increment Y
	ram_w(m_a);
	op_iy();
	op_xspx();
}

void hmcs40_cpu_device::op_lmady()
{
	// LMADY (X): Load Memory from A, Decrement Y
	ram_w(m_a);
	op_dy();
	op_xspx();
}


// Immediate Instruction

void hmcs40_cpu_device::op_lmiiy()
{
	// LMIIY i: Load Memory from Immediate, Increment Y
	ram_w(m_op & 0xf);
	op_iy();
}

void hmcs40_cpu_device::op_lai()
{
	// LAI i: Load A from Immediate
	m_a = m_op & 0xf;
}

void hmcs40_cpu_device::op_lbi()
{
	// LBI i: Load B from Immediate
	m_b = m_op & 0xf;
}


// Arithmetic Instruction

void hmcs40_cpu_device::op_ai()
{
	// AI i: Add Immediate to A
	m_a += (m_op & 0xf);
	m_s = m_a >> 4 & 1;
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
	m_c = m_a >> 4 & 1;
	m_s = m_c;
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_smc()
{
	// SMC: Subtract A from Memory with Carry
	m_a = ram_r() - m_a - (m_c ^ 1);
	m_c = ~m_a >> 4 & 1;
	m_s = m_c;
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_am()
{
	// AM: Add A to Memory
	m_a += ram_r();
	m_s = m_a >> 4 & 1;
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
	m_c = m_a >> 4 & 1;
	m_a &= 0xf;
}

void hmcs40_cpu_device::op_rotr()
{
	// ROTR: Rotate Right A with Carry
	UINT8 c = m_a & 1;
	m_a = m_a >> 1 | m_c << 3;
	m_c = c;
}

void hmcs40_cpu_device::op_or()
{
	// OR: OR A and B
	m_a |= m_b;
}


// Compare Instruction

void hmcs40_cpu_device::op_mnei()
{
	// MNEI i: Memory Not Equal to Immediate
	m_s = (ram_r() != (m_op & 0xf));
}

void hmcs40_cpu_device::op_ynei()
{
	// YNEI i: Y Not Equal to Immediate
	m_s = (m_y != (m_op & 0xf));
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
	m_s = (m_a <= (m_op & 0xf));
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


// RAM Bit Manipulation Instruction

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
	m_s = ((ram_r() & (1 << (m_op & 3))) != 0);
}


// ROM Address Instruction

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
		m_page = m_op & 0x1f;
	else
		m_op = 0;
}

void hmcs40_cpu_device::op_tbr()
{
	// TBR p: Table Branch
	m_pc = (m_a | m_b << 4 | m_c << 8 | (m_op & 7) << 9) & m_pcmask;
}

void hmcs40_cpu_device::op_rtn()
{
	// RTN: Return from Subroutine
	pop_stack();
}


// Interrupt Instruction

void hmcs40_cpu_device::op_seie()
{
	// SEIE: Set I/E
	op_illegal();
}

void hmcs40_cpu_device::op_seif0()
{
	// SEIF0: Set IF0
	op_illegal();
}

void hmcs40_cpu_device::op_seif1()
{
	// SEIF1: Set IF1
	op_illegal();
}

void hmcs40_cpu_device::op_setf()
{
	// SETF: Set TF
	op_illegal();
}

void hmcs40_cpu_device::op_secf()
{
	// SECF: Set CF
	op_illegal();
}

void hmcs40_cpu_device::op_reie()
{
	// REIE: Reset I/E
	op_illegal();
}

void hmcs40_cpu_device::op_reif0()
{
	// REIF0: Reset IF0
	op_illegal();
}

void hmcs40_cpu_device::op_reif1()
{
	// REIF1: Reset IF1
	op_illegal();
}

void hmcs40_cpu_device::op_retf()
{
	// RETF: Reset TF
	op_illegal();
}

void hmcs40_cpu_device::op_recf()
{
	// RECF: Reset CF
	op_illegal();
}

void hmcs40_cpu_device::op_ti0()
{
	// TI0: Test INT0
	op_illegal();
}

void hmcs40_cpu_device::op_ti1()
{
	// TI1: Test INT1
	op_illegal();
}

void hmcs40_cpu_device::op_tif0()
{
	// TIF0: Test IF0
	op_illegal();
}

void hmcs40_cpu_device::op_tif1()
{
	// TIF1: Test IF1
	op_illegal();
}

void hmcs40_cpu_device::op_ttf()
{
	// TTF: Test TF
	op_illegal();
}

void hmcs40_cpu_device::op_lti()
{
	// LTI i: Load Timer/Counter from Immediate
	op_illegal();
}

void hmcs40_cpu_device::op_lta()
{
	// LTA: Load Timer/Counter from A
	op_illegal();
}

void hmcs40_cpu_device::op_lat()
{
	// LAT: Load A from Timer/Counter
	op_illegal();
}

void hmcs40_cpu_device::op_rtni()
{
	// RTNI: Return from Interrupt
	op_illegal();
}


// Input/Output Instruction

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
	write_d(m_op & 0xf, 1);
}

void hmcs40_cpu_device::op_redd()
{
	// REDD n: Reset Discrete I/O Latch Direct
	write_d(m_op & 0xf, 0);
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
	m_icount--;
	UINT16 address = (m_a | m_b << 4 | m_c << 8 | (m_op & 7) << 9) | (m_pc & ~0x3f);
	UINT16 o = m_program->read_word((address & m_prgmask) << 1);
	
	// destination is determined by the 2 highest bits
	if (o & 0x100)
	{
		m_a = o & 0xf;
		m_b = o >> 4 & 0xf;
	}
	if (o & 0x200)
	{
		write_r(2, o >> 4 & 0xf);
		write_r(3, o & 0xf);
	}
}


// Control Instruction

void hmcs40_cpu_device::op_nop()
{
	// NOP: No Operation
}


void hmcs40_cpu_device::op_illegal()
{
	logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_prev_pc << 1);
}
