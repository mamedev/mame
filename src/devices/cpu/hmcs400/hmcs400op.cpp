// license:BSD-3-Clause
// copyright-holders:hap

// HMCS400 opcode handlers

#include "emu.h"
#include "hmcs400.h"


// internal helpers



// instruction set

void hmcs400_cpu_device::op_illegal()
{
	logerror("unknown opcode $%03X at $%04X\n", m_op, m_prev_pc);
}


// immediate instructions

void hmcs400_cpu_device::op_lai()
{
	// LAI i: Load A from Immediate
}

void hmcs400_cpu_device::op_lbi()
{
	// LBI i: Load B from Immediate
}

void hmcs400_cpu_device::op_lmid()
{
	// LMID i,d: Load Memory from Immediate
}

void hmcs400_cpu_device::op_lmiiy()
{
	// LMIIY i: Load Memory from Immediate, Increment Y
}


// register-to-register instructions

void hmcs400_cpu_device::op_lab()
{
	// LAB: Load A from B
}

void hmcs400_cpu_device::op_lba()
{
	// LBA: Load B from A
}

void hmcs400_cpu_device::op_law()
{
	// LAW: Load A from W
}

void hmcs400_cpu_device::op_lay()
{
	// LAY: Load A from Y
}

void hmcs400_cpu_device::op_laspx()
{
	// LASPX: Load A from SPX
}

void hmcs400_cpu_device::op_laspy()
{
	// LASPY: Load A from SPY
}

void hmcs400_cpu_device::op_lamr()
{
	// LAMR m: Load A from MR
}

void hmcs400_cpu_device::op_xmra()
{
	// XMRA m: Exchange MR and A
}


// RAM address instructions

void hmcs400_cpu_device::op_lwi()
{
	// LWI i: Load W from Immediate
}

void hmcs400_cpu_device::op_lxi()
{
	// LXI i: Load X from Immediate
}

void hmcs400_cpu_device::op_lyi()
{
	// LYI i: Load Y from Immediate
}

void hmcs400_cpu_device::op_lwa()
{
	// LWA: Load W from A
}

void hmcs400_cpu_device::op_lxa()
{
	// LXA: Load X from A
}

void hmcs400_cpu_device::op_lya()
{
	// LYA: Load Y from A
}

void hmcs400_cpu_device::op_iy()
{
	// IY: Increment Y
}

void hmcs400_cpu_device::op_dy()
{
	// DY: Decrement Y
}

void hmcs400_cpu_device::op_ayy()
{
	// AYY: Add A to Y
}

void hmcs400_cpu_device::op_syy()
{
	// SYY: Subtract A from Y
}

void hmcs400_cpu_device::op_xsp()
{
	// XSP(XY): Exchange X and SPX, Y and SPY, or NOP if 0
}


// RAM register instructions

void hmcs400_cpu_device::op_lam()
{
	// LAM(XY) / LAMD d: Load A from Memory
}

void hmcs400_cpu_device::op_lbm()
{
	// LBM(XY): Load B from Memory
}

void hmcs400_cpu_device::op_lma()
{
	// LMA(XY) / LMAD d: Load Memory from A
}

void hmcs400_cpu_device::op_lmaiy()
{
	// LMAIY(X): Load Memory from A, Increment Y
}

void hmcs400_cpu_device::op_lmady()
{
	// LMADY(X): Load Memory from A, Decrement Y
}

void hmcs400_cpu_device::op_xma()
{
	// XMA(XY) / XMAD d: Exchange Memory and A
}

void hmcs400_cpu_device::op_xmb()
{
	// XMB(XY): Exchange Memory and B
}


// arithmetic instructions

void hmcs400_cpu_device::op_ai()
{
	// AI i: Add Immediate to A
}

void hmcs400_cpu_device::op_ib()
{
	// IB: Increment B
}

void hmcs400_cpu_device::op_db()
{
	// DB: Decrement B
}

void hmcs400_cpu_device::op_daa()
{
	// DAA: Decimal Adjust for Addition
}

void hmcs400_cpu_device::op_das()
{
	// DAS: Decimal Adjust for Subtraction
}

void hmcs400_cpu_device::op_nega()
{
	// NEGA: Negate A
}

void hmcs400_cpu_device::op_comb()
{
	// COMB: Complement B
}

void hmcs400_cpu_device::op_rotr()
{
	// ROTR: Rotate Right with Carry
}

void hmcs400_cpu_device::op_rotl()
{
	// ROTL: Rotate Left with Carry
}

void hmcs400_cpu_device::op_sec()
{
	// SEC: Set Carry
}

void hmcs400_cpu_device::op_rec()
{
	// REC: Reset Carry
}

void hmcs400_cpu_device::op_tc()
{
	// TC: Test Carry
}

void hmcs400_cpu_device::op_am()
{
	// AM / AMD d: Add A to Memory
}

void hmcs400_cpu_device::op_amc()
{
	// AMC / AMCD d: Add A to Memory with Carry
}

void hmcs400_cpu_device::op_smc()
{
	// SMC / SMCD d: Subtract A from Memory with Carry
}

void hmcs400_cpu_device::op_or()
{
	// OR: Or A with B
}

void hmcs400_cpu_device::op_anm()
{
	// ANM / ANMD d: And Memory with A
}

void hmcs400_cpu_device::op_orm()
{
	// ORM / ORMD d: Or Memory with A
}

void hmcs400_cpu_device::op_eorm()
{
	// EORM / EORMD d: Exclusive Or Memory with A
}


// RAM bit manipulation instructions

void hmcs400_cpu_device::op_inem()
{
	// INEM i / INEMD i,d: Immediate Not Equal to Memory
}

void hmcs400_cpu_device::op_anem()
{
	// ANEM / ANEMD d: A Not Equal to Memory
}

void hmcs400_cpu_device::op_bnem()
{
	// BNEM: B Not Equal to Memory
}

void hmcs400_cpu_device::op_ynei()
{
	// YNEI i: Y Not Equal to Immediate
}

void hmcs400_cpu_device::op_ilem()
{
	// ILEM i / ILEMD i,d: Immediate Less or Equal to Memory
}

void hmcs400_cpu_device::op_alem()
{
	// ALEM / ALEMD d: A Less or Equal to Memory
}

void hmcs400_cpu_device::op_blem()
{
	// BLEM: B Less or Equal to Memory
}

void hmcs400_cpu_device::op_alei()
{
	// ALEI i: A Less or Equal to Immediate
}


// compare instructions

void hmcs400_cpu_device::op_sem()
{
	// SEM n / SEMD n,d: Set Memory Bit
}

void hmcs400_cpu_device::op_rem()
{
	// REM n / REMD n,d: Reset Memory Bit
}

void hmcs400_cpu_device::op_tm()
{
	// TM n / TMD n,d: Test Memory Bit
}


// ROM address instructions

void hmcs400_cpu_device::op_br()
{
	// BR b: Branch on Status 1
}

void hmcs400_cpu_device::op_brl()
{
	// BRL u: Long Branch on Status 1
}

void hmcs400_cpu_device::op_jmpl()
{
	// JMPL u: Long Jump Unconditionally
}

void hmcs400_cpu_device::op_cal()
{
	// CAL a: Subroutine Jump on Status 1
}

void hmcs400_cpu_device::op_call()
{
	// CALL u: Long Subroutine Jump on Status 1
}

void hmcs400_cpu_device::op_tbr()
{
	// TBR p: Table Branch
}

void hmcs400_cpu_device::op_rtn()
{
	// RTN: Return from Subroutine
}

void hmcs400_cpu_device::op_rtni()
{
	// RTNI: Return from Interrupt
}


// input/output instructions

void hmcs400_cpu_device::op_sed()
{
	// SED: Set Discrete I/O Latch
}

void hmcs400_cpu_device::op_sedd()
{
	// SEDD m: Set Discrete I/O Latch Direct
}

void hmcs400_cpu_device::op_red()
{
	// RED: Reset Discrete I/O Latch
}

void hmcs400_cpu_device::op_redd()
{
	// REDD m: Reset Discrete I/O Latch Direct
}

void hmcs400_cpu_device::op_td()
{
	// TD: Test Discrete I/O Latch
}

void hmcs400_cpu_device::op_tdd()
{
	// TDD m: Test Discrete I/O Latch Direct
}

void hmcs400_cpu_device::op_lar()
{
	// LAR m: Load A from R Port Register
}

void hmcs400_cpu_device::op_lbr()
{
	// LBR m: Load B from R Port Register
}

void hmcs400_cpu_device::op_lra()
{
	// LRA m: Load R Port Register from A
}

void hmcs400_cpu_device::op_lrb()
{
	// LRB m: Load R Port Register from B
}

void hmcs400_cpu_device::op_p()
{
	// P p: Pattern Generation
}


// control instructions

void hmcs400_cpu_device::op_sts()
{
	// STS: Start Serial
}

void hmcs400_cpu_device::op_sby()
{
	// SBY: Standby Mode
}

void hmcs400_cpu_device::op_stop()
{
	// STOP: Stop Mode
}
