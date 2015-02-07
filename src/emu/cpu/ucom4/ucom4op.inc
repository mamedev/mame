// uCOM-4 opcode handlers

// internal helpers

void ucom4_cpu_device::op_illegal()
{
	logerror("%s unknown opcode $%02X at $%03X\n", tag(), m_op, m_pc);
}



// basic instruction set

// Load

void ucom4_cpu_device::op_li()
{
	// LI X: Load ACC with X
	// note: only execute the first one in a sequence of LAI
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
		m_acc = m_op & 0x0f;
}

void ucom4_cpu_device::op_lm()
{
	// LM X: Load ACC with RAM, xor DPh with X
	op_illegal();
}

void ucom4_cpu_device::op_ldi()
{
	// LDI X: Load DP with X
	op_illegal();
}

void ucom4_cpu_device::op_ldz()
{
	// LDZ X: Load DPh with 0, Load DPl with X
	op_illegal();
}


// Store

void ucom4_cpu_device::op_s()
{
	// S: Store ACC into RAM
	op_illegal();
}


// Transfer

void ucom4_cpu_device::op_tal()
{
	// TAL: Transfer ACC to DPl
	op_illegal();
}

void ucom4_cpu_device::op_tla()
{
	// TLA: Transfer 
	op_illegal();
}


// Exchange

void ucom4_cpu_device::op_xm()
{
	// XM X: Exchange ACC with RAM, xor DPh with X
	op_illegal();
}

void ucom4_cpu_device::op_xmi()
{
	// XMI X: Exchange ACC with RAM, xor DPh with X, Increment DPl, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_xmd()
{
	// XMD X: Exchange ACC with RAM, xor DPh with X, Decrement DPl, skip next on carry
	op_illegal();
}


// Arithmetic

void ucom4_cpu_device::op_ad()
{
	// AD: Add RAM to Acc, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_adc()
{
	// ADC: Add RAM and carry to Acc, store Carry F/F
	op_illegal();
}

void ucom4_cpu_device::op_ads()
{
	// ADS: Add RAM and carry to Acc, store Carry F/F, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_daa()
{
	// DAA: Add 6 to ACC to adjust decimal for BCD Addition
	op_illegal();
}

void ucom4_cpu_device::op_das()
{
	// DAS: Add 10 to ACC to adjust decimal for BCD Subtraction
	op_illegal();
}


// Logical

void ucom4_cpu_device::op_exl()
{
	// EXL: Xor ACC with RAM
	op_illegal();
}


// Accumulator

void ucom4_cpu_device::op_cma()
{
	// CMA: Complement ACC
	op_illegal();
}

void ucom4_cpu_device::op_cia()
{
	// CIA: Complement ACC, Increment ACC
	op_illegal();
}


// Carry Flag

void ucom4_cpu_device::op_clc()
{
	// CLC: Reset Carry F/F
	op_illegal();
}

void ucom4_cpu_device::op_stc()
{
	// STC: Set Carry F/F
	op_illegal();
}

void ucom4_cpu_device::op_tc()
{
	// TC: skip next on Carry F/F
	op_illegal();
}


// Increment and Decrement

void ucom4_cpu_device::op_inc()
{
	// INC: Increment ACC, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_dec()
{
	// DEC: Decrement ACC, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_ind()
{
	// IND: Increment DPl, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_ded()
{
	// DED: Decrement DPl, skip next on carry
	op_illegal();
}


// Bit Manipulation

void ucom4_cpu_device::op_rmb()
{
	// RMB B: Reset a single bit of RAM
	op_illegal();
}

void ucom4_cpu_device::op_smb()
{
	// SMB B: Set a single bit of RAM
	op_illegal();
}

void ucom4_cpu_device::op_reb()
{
	// REB B: Reset a single bit of output port E
	op_illegal();
}

void ucom4_cpu_device::op_seb()
{
	// SEB B: Set a single bit of output port E
	op_illegal();
}

void ucom4_cpu_device::op_rpb()
{
	// RPB B: Reset a single bit of output port (DPl)
	op_illegal();
}

void ucom4_cpu_device::op_spb()
{
	// SPB B: Set a single bit of output port (DPl)
	op_illegal();
}


// Jump, Call and Return

void ucom4_cpu_device::op_jmpcal()
{
	// JMP A: Jump to Address / CAL A: Call Address
	op_illegal();
}

void ucom4_cpu_device::op_jcp()
{
	// JCP A: Jump to Address in current page
	op_illegal();
}

void ucom4_cpu_device::op_jpa()
{
	// JPA: Jump to (ACC)
	op_illegal();
}

void ucom4_cpu_device::op_czp()
{
	// CZP A: Call (Address)
	op_illegal();
}

void ucom4_cpu_device::op_rt()
{
	// RT: Return from subroutine
	op_illegal();
}

void ucom4_cpu_device::op_rts()
{
	// RTS: Return from subroutine, skip next
	op_rt();
	m_skip = true;
}


// Skip

void ucom4_cpu_device::op_ci()
{
	// CI X: skip next on ACC equals X
	op_illegal();
}

void ucom4_cpu_device::op_cm()
{
	// CM: skip next on ACC equals RAM
	op_illegal();
}

void ucom4_cpu_device::op_cmb()
{
	// CMB B: skip next on bit(ACC) equals bit(RAM)
	op_illegal();
}

void ucom4_cpu_device::op_tab()
{
	// TAB B: skip next on bit(ACC)
	op_illegal();
}

void ucom4_cpu_device::op_cli()
{
	// CLI X: skip next on DPl equals X
	op_illegal();
}

void ucom4_cpu_device::op_tmb()
{
	// TMB B: skip next on bit(RAM)
	op_illegal();
}

void ucom4_cpu_device::op_tpa()
{
	// TPA B: skip next on bit(input port A)
	op_illegal();
}

void ucom4_cpu_device::op_tpb()
{
	// TPB B: skip next on bit(input port (DPl))
	op_illegal();
}


// Interrupt

void ucom4_cpu_device::op_tit()
{
	// TIT: skip next on Interrupt F/F, reset Interrupt F/F
	op_illegal();
}


// Parallel I/O

void ucom4_cpu_device::op_ia()
{
	// IA: x
	op_illegal();
}

void ucom4_cpu_device::op_ip()
{
	// IP: x
	op_illegal();
}

void ucom4_cpu_device::op_oe()
{
	// OE: x
	op_illegal();
}

void ucom4_cpu_device::op_op()
{
	// OP: x
	op_illegal();
}

void ucom4_cpu_device::op_ocd()
{
	// OCD X: x
	op_illegal();
}


// CPU Control

void ucom4_cpu_device::op_nop()
{
	// NOP: No Operation
}



// uCOM-43 extended instructions

inline bool ucom4_cpu_device::check_op_43()
{
	// these opcodes are officially only supported on uCOM-43
	if (m_family != NEC_UCOM43)
		logerror("%s using uCOM-43 opcode $%02X at $%03X\n", tag(), m_op, m_pc);

	return (m_family == NEC_UCOM43);
}

// Transfer

void ucom4_cpu_device::op_taw()
{
	if (!check_op_43()) return;

	// TAW: Transfer ACC to W
	op_illegal();
}

void ucom4_cpu_device::op_taz()
{
	if (!check_op_43()) return;
	
	// TAZ: Transfer ACC to Z
	op_illegal();
}

void ucom4_cpu_device::op_thx()
{
	if (!check_op_43()) return;
	
	// THX: Transfer DPh to X
	op_illegal();
}

void ucom4_cpu_device::op_tly()
{
	if (!check_op_43()) return;
	
	// TLY: Transfer DPl to Y
	op_illegal();
}


// Exchange

void ucom4_cpu_device::op_xaw()
{
	if (!check_op_43()) return;
	
	// XAW: Exchange ACC with W
	op_illegal();
}

void ucom4_cpu_device::op_xaz()
{
	if (!check_op_43()) return;
	
	// XAZ: Exchange ACC with Z
	op_illegal();
}

void ucom4_cpu_device::op_xhr()
{
	if (!check_op_43()) return;
	
	// XHR: Exchange DPh with R
	op_illegal();
}

void ucom4_cpu_device::op_xhx()
{
	if (!check_op_43()) return;
	
	// XHX: Exchange DPh with X
	op_illegal();
}

void ucom4_cpu_device::op_xls()
{
	if (!check_op_43()) return;
	
	// XLS: Exchange DPl with S
	op_illegal();
}

void ucom4_cpu_device::op_xly()
{
	if (!check_op_43()) return;
	
	// XLY: Exchange DPl with Y
	op_illegal();
}

void ucom4_cpu_device::op_xc()
{
	if (!check_op_43()) return;
	
	// XC: Exchange Carry F/F with Carry Save F/F
	op_illegal();
}


// Flag

void ucom4_cpu_device::op_sfb()
{
	if (!check_op_43()) return;
	
	// SFB B: Set a single bit of FLAG
	op_illegal();
}

void ucom4_cpu_device::op_rfb()
{
	if (!check_op_43()) return;
	
	// RFB B: Reset a single bit of FLAG
	op_illegal();
}

void ucom4_cpu_device::op_fbt()
{
	if (!check_op_43()) return;
	
	// FBT B: skip next on bit(FLAG)
	op_illegal();
}

void ucom4_cpu_device::op_fbf()
{
	if (!check_op_43()) return;
	
	// FBF B: skip next on not bit(FLAG)
	op_illegal();
}


// Accumulator

void ucom4_cpu_device::op_rar()
{
	if (!check_op_43()) return;
	
	// RAR: Rotate ACC Right through Carry F/F
	op_illegal();
}


// Increment and Decrement

void ucom4_cpu_device::op_inm()
{
	if (!check_op_43()) return;
	
	// INM: Increment RAM, skip next on carry
	op_illegal();
}

void ucom4_cpu_device::op_dem()
{
	if (!check_op_43()) return;
	
	// DEM: Decrement RAM, skip next on carry
	op_illegal();
}


// Timer

void ucom4_cpu_device::op_stm()
{
	if (!check_op_43()) return;
	
	// STM X: Reset Timer F/F, Start Timer with X
	op_illegal();
}

void ucom4_cpu_device::op_ttm()
{
	if (!check_op_43()) return;
	
	// TTM: skip next on Timer F/F
	op_illegal();
}


// Interrupt

void ucom4_cpu_device::op_ei()
{
	if (!check_op_43()) return;
	
	// EI: Set Interrupt Enable F/F
	op_illegal();
}

void ucom4_cpu_device::op_di()
{
	if (!check_op_43()) return;
	
	// DI: Reset Interrupt Enable F/F
	op_illegal();
}
