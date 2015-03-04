// HMCS40 opcode handlers

// internal helpers

void hmcs40_cpu_device::op_illegal()
{
	logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_pc);
}



// instruction set

// Register-to-Register Instruction

void hmcs40_cpu_device::op_lab()
{
	// LAB: Load A from B
	op_illegal();
}

void hmcs40_cpu_device::op_lba()
{
	// LBA: Load B from A
	op_illegal();
}

void hmcs40_cpu_device::op_lay()
{
	// LAY: Load A from Y
	op_illegal();
}

void hmcs40_cpu_device::op_laspx()
{
	// LASPX: Load A from SPX
	op_illegal();
}

void hmcs40_cpu_device::op_laspy()
{
	// LASPY: Load A from SPY
	op_illegal();
}

void hmcs40_cpu_device::op_xamr()
{
	// XAMR m: Exchange A and MR(m)
	op_illegal();
}


// RAM Address Instruction

void hmcs40_cpu_device::op_lxa()
{
	// LXA: Load X from A
	op_illegal();
}

void hmcs40_cpu_device::op_lya()
{
	// LYA: Load Y from A
	op_illegal();
}

void hmcs40_cpu_device::op_lxi()
{
	// LXI i: Load X from Immediate
	op_illegal();
}

void hmcs40_cpu_device::op_lyi()
{
	// LYI i: Load Y from Immediate
	op_illegal();
}

void hmcs40_cpu_device::op_iy()
{
	// IY: Increment Y
	op_illegal();
}

void hmcs40_cpu_device::op_dy()
{
	// DY: Decrement Y
	op_illegal();
}

void hmcs40_cpu_device::op_ayy()
{
	// AYY: Add A to Y
	op_illegal();
}

void hmcs40_cpu_device::op_syy()
{
	// SYY: Subtract A from Y
	op_illegal();
}

void hmcs40_cpu_device::op_xspx()
{
	// XSPX: Exchange X and SPX
	op_illegal();
}

void hmcs40_cpu_device::op_sxpy()
{
	// SXPY: Exchange Y and SPY
	op_illegal();
}

void hmcs40_cpu_device::op_xspxy()
{
	// XSPXY: Exchange X and SPX, Y and SPY
	op_illegal();
}


// Ram Register Instruction

void hmcs40_cpu_device::op_lam()
{
	// LAM (XY): Load A from Memory
	op_illegal();
}

void hmcs40_cpu_device::op_lbm()
{
	// LBM (XY): Load B from Memory
	op_illegal();
}

void hmcs40_cpu_device::op_xma()
{
	// XMA (XY): Exchange Memory and A
	op_illegal();
}

void hmcs40_cpu_device::op_xmb()
{
	// XMB (XY): Exchange Memory and B
	op_illegal();
}

void hmcs40_cpu_device::op_lmaiy()
{
	// LMAIY (X): Load Memory from A, Increment Y
	op_illegal();
}

void hmcs40_cpu_device::op_lmady()
{
	// LMADY (X): Load Memory from A, Decrement Y
	op_illegal();
}


// Immediate Instruction

void hmcs40_cpu_device::op_lmiiy()
{
	// LMIIY i: Load Memory from Immediate, Increment Y
	op_illegal();
}

void hmcs40_cpu_device::op_lai()
{
	// LAI i: Load A from Immediate
	op_illegal();
}

void hmcs40_cpu_device::op_lbi()
{
	// LBI i: Load B from Immediate
	op_illegal();
}


// Arithmetic Instruction

void hmcs40_cpu_device::op_ai()
{
	// AI i: Add Immediate to A
	op_illegal();
}

void hmcs40_cpu_device::op_ib()
{
	// IB: Increment B
	op_illegal();
}

void hmcs40_cpu_device::op_db()
{
	// DB: Decrement B
	op_illegal();
}

void hmcs40_cpu_device::op_amc()
{
	// AMC: Add A to Memory with Carry
	op_illegal();
}

void hmcs40_cpu_device::op_smc()
{
	// SMC: Subtract A from Memory with Carry
	op_illegal();
}

void hmcs40_cpu_device::op_am()
{
	// AM: Add A to Memory
	op_illegal();
}

void hmcs40_cpu_device::op_daa()
{
	// DAA: Decimal Adjust for Addition
	op_illegal();
}

void hmcs40_cpu_device::op_das()
{
	// DAS: Decimal Adjust for Subtraction
	op_illegal();
}

void hmcs40_cpu_device::op_nega()
{
	// NEGA: Negate A
	op_illegal();
}

void hmcs40_cpu_device::op_comb()
{
	// COMB: Complement B
	op_illegal();
}

void hmcs40_cpu_device::op_sec()
{
	// SEC: Set Carry
	op_illegal();
}

void hmcs40_cpu_device::op_rec()
{
	// REC: Reset Carry
	op_illegal();
}

void hmcs40_cpu_device::op_tc()
{
	// TC: Test Carry
	op_illegal();
}

void hmcs40_cpu_device::op_rotl()
{
	// ROTL: Rotate Left A with Carry
	op_illegal();
}

void hmcs40_cpu_device::op_rotr()
{
	// ROTR: Rotate Right A with Carry
	op_illegal();
}

void hmcs40_cpu_device::op_or()
{
	// OR: OR A and B
	op_illegal();
}


// Compare Instruction

void hmcs40_cpu_device::op_mnei()
{
	// MNEI i: Memory Not Equal to Immediate
	op_illegal();
}

void hmcs40_cpu_device::op_ynei()
{
	// YNEI i: Y Not Equal to Immediate
	op_illegal();
}

void hmcs40_cpu_device::op_anem()
{
	// ANEM: A Not Equal to Memory
	op_illegal();
}

void hmcs40_cpu_device::op_bnem()
{
	// BNEM: B Not Equal to Memory
	op_illegal();
}

void hmcs40_cpu_device::op_alei()
{
	// ALEI i: A Less or Equal to Immediate
	op_illegal();
}

void hmcs40_cpu_device::op_alem()
{
	// ALEM: A Less or Equal to Memory
	op_illegal();
}

void hmcs40_cpu_device::op_blem()
{
	// BLEM: B Less or Equal to Memory
	op_illegal();
}


// RAM Bit Manipulation Instruction

void hmcs40_cpu_device::op_sem()
{
	// SEM n: Set Memory Bit
	op_illegal();
}

void hmcs40_cpu_device::op_rem()
{
	// REM n: Reset Memory Bit
	op_illegal();
}

void hmcs40_cpu_device::op_tm()
{
	// TM n: Test Memory Bit
	op_illegal();
}


// ROM Address Instruction

void hmcs40_cpu_device::op_br()
{
	// BR a: Branch on Status 1
	op_illegal();
}

void hmcs40_cpu_device::op_cal()
{
	// CAL a: Subroutine Jump on Status 1
	op_illegal();
}

void hmcs40_cpu_device::op_lpu()
{
	// LPU u: Load Program Counter Upper on Status 1
	op_illegal();
}

void hmcs40_cpu_device::op_tbr()
{
	// TBR p: Table Branch
	op_illegal();
}

void hmcs40_cpu_device::op_rtn()
{
	// RTN: Return from Subroutine
	op_illegal();
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
	op_illegal();
}

void hmcs40_cpu_device::op_red()
{
	// RED: Reset Discrete I/O Latch
	op_illegal();
}

void hmcs40_cpu_device::op_td()
{
	// TD: Test Discrete I/O Latch
	op_illegal();
}

void hmcs40_cpu_device::op_sedd()
{
	// SEDD n: Set Discrete I/O Latch Direct
	op_illegal();
}

void hmcs40_cpu_device::op_redd()
{
	// REDD n: Reset Discrete I/O Latch Direct
	op_illegal();
}

void hmcs40_cpu_device::op_lar()
{
	// LAR p: Load A from R-Port Register
	op_illegal();
}

void hmcs40_cpu_device::op_lbr()
{
	// LBR p: Load B from R-Port Register
	op_illegal();
}

void hmcs40_cpu_device::op_lra()
{
	// LRA p: Load R-Port Register from A
	op_illegal();
}

void hmcs40_cpu_device::op_lrb()
{
	// LRB p: Load R-Port Register from B
	op_illegal();
}

void hmcs40_cpu_device::op_p()
{
	// P p: Pattern Generation
	op_illegal();
}


// Control Instruction

void hmcs40_cpu_device::op_nop()
{
	// NOP: No Operation
}
