// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Sandro Ronco, Felipe Sanches
/***************************************************************************

    Atmel 8-bit AVR simulator

    Opcode implementations

***************************************************************************/

void avr8_base_device::populate_ops()
{
	for (uint32_t op = 0; op < 0x10000; op++)
	{
		m_op_cycles[op] = 1;

		switch (op & 0xf000)
		{
		case 0x0000:
			switch (op & 0x0f00)
			{
			case 0x0000:    // NOP
				m_op_funcs[op] = &avr8_base_device::op_nop;
				break;
			case 0x0100:    // MOVW Rd+1:Rd,Rr+1:Rd
				m_op_funcs[op] = &avr8_base_device::op_movw;
				break;
			case 0x0200:    // MULS Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_muls;
				m_op_cycles[op] = 2;
				break;
			case 0x0300:    // Multiplication
				switch (MULCONST2(op))
				{
				case 0x0000: // MULSU Rd,Rr
					m_op_funcs[op] = &avr8_base_device::op_mulsu;
					m_op_cycles[op] = 2;
					break;
				case 0x0001: // FMUL Rd,Rr
					m_op_funcs[op] = &avr8_base_device::op_fmul;
					m_op_cycles[op] = 2;
					break;
				case 0x0002: // FMULS Rd,Rr
					m_op_funcs[op] = &avr8_base_device::op_fmuls;
					m_op_cycles[op] = 2;
					break;
				case 0x0003: // FMULSU Rd,Rr
					m_op_funcs[op] = &avr8_base_device::op_fmulsu;
					m_op_cycles[op] = 2;
					break;
				}
				break;
			case 0x0400:
			case 0x0500:
			case 0x0600:
			case 0x0700:    // CPC Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_cpc;
				break;
			case 0x0800:
			case 0x0900:
			case 0x0a00:
			case 0x0b00:    // SBC Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_sbc;
				break;
			case 0x0c00:
			case 0x0d00:
			case 0x0e00:
			case 0x0f00:    // ADD Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_add;
				break;
			}
			break;
		case 0x1000:
			switch (op & 0x0c00)
			{
			case 0x0000:    // CPSE Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_cpse;
				break;
			case 0x0400:    // CP Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_cp;
				break;
			case 0x0800:    // SUB Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_sub;
				break;
			case 0x0c00:    // ADC Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_adc;
				break;
			}
			break;
		case 0x2000:
			switch (op & 0x0c00)
			{
			case 0x0000:    // AND Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_and;
				break;
			case 0x0400:    // EOR Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_eor;
				break;
			case 0x0800:    // OR Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_or;
				break;
			case 0x0c00:    // MOV Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_mov;
				break;
			}
			break;
		case 0x3000:    // CPI Rd,K
			m_op_funcs[op] = &avr8_base_device::op_cpi;
			break;
		case 0x4000:    // SBCI Rd,K
			m_op_funcs[op] = &avr8_base_device::op_sbci;
			break;
		case 0x5000:    // SUBI Rd,K
			m_op_funcs[op] = &avr8_base_device::op_subi;
			break;
		case 0x6000:    // ORI Rd,K
			m_op_funcs[op] = &avr8_base_device::op_ori;
			break;
		case 0x7000:    // ANDI Rd,K
			m_op_funcs[op] = &avr8_base_device::op_andi;
			break;
		case 0x8000:
		case 0xa000:
			switch (op & 0x0208)
			{
			case 0x0000:    // LDD Rd,Z+q
				m_op_funcs[op] = &avr8_base_device::op_lddz;
				m_op_cycles[op] = 2;
				break;
			case 0x0008:    // LDD Rd,Y+q
				m_op_funcs[op] = &avr8_base_device::op_lddy;
				m_op_cycles[op] = 2;
				break;
			case 0x0200:    // STD Z+q,Rr
				m_op_funcs[op] = &avr8_base_device::op_stdz;
				m_op_cycles[op] = 2;
				break;
			case 0x0208:    // STD Y+q,Rr
				m_op_funcs[op] = &avr8_base_device::op_stdy;
				m_op_cycles[op] = 2;
				break;
			}
			break;
		case 0x9000:
			switch (op & 0x0f00)
			{
			case 0x0000:
			case 0x0100:
				switch (op & 0x000f)
				{
				case 0x0000:    // LDS Rd,k
					m_op_funcs[op] = &avr8_base_device::op_lds;
					m_op_cycles[op] = 2;
					break;
				case 0x0001:    // LD Rd,Z+
					m_op_funcs[op] = &avr8_base_device::op_ldzi;
					m_op_cycles[op] = 2;
					break;
				case 0x0002:    // LD Rd,-Z
					m_op_funcs[op] = &avr8_base_device::op_ldzd;
					m_op_cycles[op] = 2;
					break;
				case 0x0004:    // LPM Rd,Z
					m_op_funcs[op] = &avr8_base_device::op_lpmz;
					m_op_cycles[op] = 3;
					break;
				case 0x0005:    // LPM Rd,Z+
					m_op_funcs[op] = &avr8_base_device::op_lpmzi;
					m_op_cycles[op] = 3;
					break;
				case 0x0006:    // ELPM Rd,Z
					m_op_funcs[op] = &avr8_base_device::op_elpmz;
					m_op_cycles[op] = 3;
					break;
				case 0x0007:    // ELPM Rd,Z+
					m_op_funcs[op] = &avr8_base_device::op_elpmzi;
					m_op_cycles[op] = 3;
					break;
				case 0x0009:    // LD Rd,Y+
					m_op_funcs[op] = &avr8_base_device::op_ldyi;
					m_op_cycles[op] = 2;
					break;
				case 0x000a:    // LD Rd,-Y
					m_op_funcs[op] = &avr8_base_device::op_ldyd;
					m_op_cycles[op] = 2;
					break;
				case 0x000c:    // LD Rd,X
					m_op_funcs[op] = &avr8_base_device::op_ldx;
					m_op_cycles[op] = 2;
					break;
				case 0x000d:    // LD Rd,X+
					m_op_funcs[op] = &avr8_base_device::op_ldxi;
					m_op_cycles[op] = 2;
					break;
				case 0x000e:    // LD Rd,-X
					m_op_funcs[op] = &avr8_base_device::op_ldxd;
					m_op_cycles[op] = 2;
					break;
				case 0x000f:    // POP Rd
					m_op_funcs[op] = &avr8_base_device::op_pop;
					m_op_cycles[op] = 2;
					break;
				default:
					m_op_funcs[op] = &avr8_base_device::op_unimpl;
					break;
				}
				break;
			case 0x0200:
			case 0x0300:
				switch (op & 0x000f)
				{
				case 0x0000:    // STS k,Rr
					m_op_funcs[op] = &avr8_base_device::op_sts;
					m_op_cycles[op] = 2;
					break;
				case 0x0001:    // ST Z+,Rd
					m_op_funcs[op] = &avr8_base_device::op_stzi;
					m_op_cycles[op] = 2;
					break;
				case 0x0002:    // ST -Z,Rd
					m_op_funcs[op] = &avr8_base_device::op_stzd;
					m_op_cycles[op] = 2;
					break;
				case 0x0009:    // ST Y+,Rd
					m_op_funcs[op] = &avr8_base_device::op_styi;
					m_op_cycles[op] = 2;
					break;
				case 0x000a:    // ST -Y,Rd
					m_op_funcs[op] = &avr8_base_device::op_styd;
					m_op_cycles[op] = 2;
					break;
				case 0x000c:    // ST X,Rd
					m_op_funcs[op] = &avr8_base_device::op_stx;
					break;
				case 0x000d:    // ST X+,Rd
					m_op_funcs[op] = &avr8_base_device::op_stxi;
					m_op_cycles[op] = 2;
					break;
				case 0x000e:    // ST -X,Rd
					m_op_funcs[op] = &avr8_base_device::op_stxd;
					m_op_cycles[op] = 2;
					break;
				case 0x000f:    // PUSH Rd
					m_op_funcs[op] = &avr8_base_device::op_push;
					m_op_cycles[op] = 2;
					break;
				default:
					m_op_funcs[op] = &avr8_base_device::op_unimpl;
					break;
				}
				break;
			case 0x0400:
				switch (op & 0x000f)
				{
				case 0x0000:    // COM Rd
					m_op_funcs[op] = &avr8_base_device::op_com;
					break;
				case 0x0001:    // NEG Rd
					m_op_funcs[op] = &avr8_base_device::op_neg;
					break;
				case 0x0002:    // SWAP Rd
					m_op_funcs[op] = &avr8_base_device::op_swap;
					break;
				case 0x0003:    // INC Rd
					m_op_funcs[op] = &avr8_base_device::op_inc;
					break;
				case 0x0005:    // ASR Rd
					m_op_funcs[op] = &avr8_base_device::op_asr;
					break;
				case 0x0006:    // LSR Rd
					m_op_funcs[op] = &avr8_base_device::op_lsr;
					break;
				case 0x0007:    // ROR Rd
					m_op_funcs[op] = &avr8_base_device::op_ror;
					break;
				case 0x0008:
					switch (op & 0x00f0)
					{
					case 0x0000:    // SEC
					case 0x0010:    // SEZ
					case 0x0020:    // SEN
					case 0x0030:    // SEV
					case 0x0040:    // SES
					case 0x0050:    // SEH
					case 0x0060:    // SET
					case 0x0070:    // SEI
						m_op_funcs[op] = &avr8_base_device::op_setf;
						break;
					case 0x0080:    // CLC
					case 0x0090:    // CLZ
					case 0x00a0:    // CLN
					case 0x00b0:    // CLV
					case 0x00c0:    // CLS
					case 0x00d0:    // CLH
					case 0x00e0:    // CLT
					case 0x00f0:    // CLI
						m_op_funcs[op] = &avr8_base_device::op_clrf;
						break;
					}
					break;
				case 0x0009:
					switch (op & 0x00f0)
					{
					case 0x0000:    // IJMP
						m_op_funcs[op] = &avr8_base_device::op_ijmp;
						m_op_cycles[op] = 2;
						break;
					case 0x0010:    // EIJMP
						m_op_funcs[op] = &avr8_base_device::op_eijmp;
						m_op_cycles[op] = 2;
						break;
					default:
						m_op_funcs[op] = &avr8_base_device::op_unimpl;
						break;
					}
					break;
				case 0x000a:    // DEC Rd
					m_op_funcs[op] = &avr8_base_device::op_dec;
					break;
				case 0x000c:
				case 0x000d:    // JMP k
					m_op_funcs[op] = &avr8_base_device::op_jmp;
					m_op_cycles[op] = 3;
					break;
				case 0x000e:    // CALL k
				case 0x000f:
					m_op_funcs[op] = &avr8_base_device::op_call;
					m_op_cycles[op] = 4;
					break;
				default:
					m_op_funcs[op] = &avr8_base_device::op_unimpl;
					break;
				}
				break;
			case 0x0500:
				switch (op & 0x000f)
				{
				case 0x0000:    // COM Rd
					m_op_funcs[op] = &avr8_base_device::op_com;
					break;
				case 0x0001:    // NEG Rd
					m_op_funcs[op] = &avr8_base_device::op_neg;
					break;
				case 0x0002:    // SWAP Rd
					m_op_funcs[op] = &avr8_base_device::op_swap;
					break;
				case 0x0003:    // INC Rd
					m_op_funcs[op] = &avr8_base_device::op_inc;
					break;
				case 0x0005:    // ASR Rd
					m_op_funcs[op] = &avr8_base_device::op_asr;
					break;
				case 0x0006:    // LSR Rd
					m_op_funcs[op] = &avr8_base_device::op_lsr;
					break;
				case 0x0007:    // ROR Rd
					m_op_funcs[op] = &avr8_base_device::op_ror;
					break;
				case 0x0008:
					switch (op & 0x00f0)
					{
					case 0x0000:    // RET
						m_op_funcs[op] = &avr8_base_device::op_ret;
						m_op_cycles[op] = 4;
						break;
					case 0x0010:    // RETI
						m_op_funcs[op] = &avr8_base_device::op_reti;
						m_op_cycles[op] = 4;
						break;
					case 0x0080:    // SLEEP
						m_op_funcs[op] = &avr8_base_device::op_sleep;
						m_op_cycles[op] = 1;
						break;
					case 0x0090:    // BREAK
						m_op_funcs[op] = &avr8_base_device::op_unimpl;
						break;
					case 0x00a0:    // WDR
						m_op_funcs[op] = &avr8_base_device::op_wdr;
						break;
					case 0x00c0:    // LPM
						m_op_funcs[op] = &avr8_base_device::op_lpm;
						m_op_cycles[op] = 3;
						break;
					case 0x00d0:    // ELPM
						m_op_funcs[op] = &avr8_base_device::op_elpm;
						break;
					case 0x00e0:    // SPM
						m_op_funcs[op] = &avr8_base_device::op_spm;
						break;
					case 0x00f0:    // SPM Z+
						m_op_funcs[op] = &avr8_base_device::op_spmzi;
						break;
					default:
						m_op_funcs[op] = &avr8_base_device::op_unimpl;
						break;
					}
					break;
				case 0x0009:
					switch (op & 0x00f0)
					{
					case 0x0000:    // ICALL
						m_op_funcs[op] = &avr8_base_device::op_icall;
						m_op_cycles[op] = 3;
						break;
					case 0x0010:    // EICALL
						m_op_funcs[op] = &avr8_base_device::op_eicall;
						break;
					default:
						m_op_funcs[op] = &avr8_base_device::op_unimpl;
						break;
					}
					break;
				case 0x000a:    // DEC Rd
					m_op_funcs[op] = &avr8_base_device::op_dec;
					break;
				case 0x000c:
				case 0x000d:    // JMP k
					m_op_funcs[op] = &avr8_base_device::op_jmp;
					m_op_cycles[op] = 3;
					break;
				case 0x000e:
				case 0x000f:    // CALL k
					m_op_funcs[op] = &avr8_base_device::op_call;
					m_op_cycles[op] = 4;
					break;
				}
				break;
			case 0x0600:    // ADIW Rd+1:Rd,K
				m_op_funcs[op] = &avr8_base_device::op_adiw;
				m_op_cycles[op] = 2;
				break;
			case 0x0700:    // SBIW Rd+1:Rd,K
				m_op_funcs[op] = &avr8_base_device::op_sbiw;
				m_op_cycles[op] = 2;
				break;
			case 0x0800:    // CBI A,b
				m_op_funcs[op] = &avr8_base_device::op_cbi;
				m_op_cycles[op] = 2;
				break;
			case 0x0900:    // SBIC A,b
				m_op_funcs[op] = &avr8_base_device::op_sbic;
				break;
			case 0x0a00:    // SBI A,b
				m_op_funcs[op] = &avr8_base_device::op_sbi;
				m_op_cycles[op] = 2;
				break;
			case 0x0b00:    // SBIS A,b
				m_op_funcs[op] = &avr8_base_device::op_sbis;
				break;
			case 0x0c00:
			case 0x0d00:
			case 0x0e00:
			case 0x0f00:    // MUL Rd,Rr
				m_op_funcs[op] = &avr8_base_device::op_mul;
				m_op_cycles[op] = 2;
				break;
			}
			break;
		case 0xb000:
			if (op & 0x0800) // OUT A,Rr
			{
				m_op_funcs[op] = &avr8_base_device::op_out;
			}
			else            // IN Rd,A
			{
				m_op_funcs[op] = &avr8_base_device::op_in;
			}
			break;
		case 0xc000:    // RJMP k
			m_op_funcs[op] = &avr8_base_device::op_rjmp;
			m_op_cycles[op] = 2;
			break;
		case 0xd000:    // RCALL k
			m_op_funcs[op] = &avr8_base_device::op_rcall;
			m_op_cycles[op] = 3;
			break;
		case 0xe000:    // LDI Rd,K
			m_op_funcs[op] = &avr8_base_device::op_ldi;
			break;
		case 0xf000:
			switch (op & 0x0c00)
			{
			case 0x0000: // BRLO through BRIE
				m_op_funcs[op] = &avr8_base_device::op_brset;
				break;
			case 0x0400: // BRSH through BRID
				m_op_funcs[op] = &avr8_base_device::op_brclr;
				break;
			case 0x0800:
				if (op & 0x0200) // BST Rd, b
				{
					m_op_funcs[op] = &avr8_base_device::op_bst;
				}
				else            // BLD Rd, b
				{
					m_op_funcs[op] = &avr8_base_device::op_bld;
				}
				break;
			case 0x0c00:
				if (op & 0x0200) // SBRS Rd, b
				{
					m_op_funcs[op] = &avr8_base_device::op_sbrs;
				}
				else             // SBRC Rd, b
				{
					m_op_funcs[op] = &avr8_base_device::op_sbrc;
				}
				break;
			}
			break;
		}
	}
}

void avr8_base_device::populate_add_flag_cache()
{
	for (uint16_t rd = 0; rd < 0x100; rd++)
	{
		for (uint16_t rr = 0; rr < 0x100; rr++)
		{
			const uint8_t res = rd + rr;
			uint8_t flags = 0;
			flags |= (((rd & 8) && (rr & 8)) || ((rr & 8) && !(res & 8)) || (!(res & 8) && (rd & 8))) ? SREG_MASK_H : 0;
			flags |= (((rd & 0x80) && (rr & 0x80) && !(res & 0x80)) || (!(rd & 0x80) && !(rr & 0x80) && (res & 0x80))) ? SREG_MASK_V : 0;
			flags |= (res & 0x80) ? SREG_MASK_N : 0;
			flags |= (bool(flags & SREG_MASK_N) != bool(flags & SREG_MASK_V)) ? SREG_MASK_S : 0;
			flags |= (res == 0) ? SREG_MASK_Z : 0;
			flags |= (((rd & 0x80) && (rr & 0x80)) || ((rr & 0x80) && !(res & 0x80)) || (!(res & 0x80) && (rd & 0x80))) ? SREG_MASK_C : 0;
			m_add_flag_cache[(rd << 8) | rr] = flags;
		}
	}
}

void avr8_base_device::populate_adc_flag_cache()
{
	for (uint16_t rd = 0; rd < 0x100; rd++)
	{
		for (uint16_t rr = 0; rr < 0x100; rr++)
		{
			for (uint8_t c = 0; c < 2; c++)
			{
				const uint8_t res = rd + rr + c;
				uint8_t flags = 0;
				flags |= (((rd & 8) && (rr & 8)) || ((rr & 8) && !(res & 8)) || (!(res & 8) && (rd & 8))) ? SREG_MASK_H : 0;
				flags |= (((rd & 0x80) && (rr & 0x80) && !(res & 0x80)) || (!(rd & 0x80) && !(rr & 0x80) && (res & 0x80))) ? SREG_MASK_V : 0;
				flags |= (res & 0x80) ? SREG_MASK_N : 0;
				flags |= (bool(flags & SREG_MASK_N) != bool(flags & SREG_MASK_V)) ? SREG_MASK_S : 0;
				flags |= (res == 0) ? SREG_MASK_Z : 0;
				flags |= (((rd & 0x80) && (rr & 0x80)) || ((rr & 0x80) && !(res & 0x80)) || (!(res & 0x80) && (rd & 0x80))) ? SREG_MASK_C : 0;
				m_adc_flag_cache[(c << 16) | (rd << 8) | rr] = flags;
			}
		}
	}
}

void avr8_base_device::populate_sub_flag_cache()
{
	for (uint16_t rd = 0; rd < 0x100; rd++)
	{
		for (uint16_t rr = 0; rr < 0x100; rr++)
		{
			const uint8_t res = rd - rr;
			uint8_t flags = 0;
			flags |= ((!(rd & 8) && (rr & 8)) || ((rr & 8) && (res & 8)) || ((res & 8) && !(rd & 8))) ? SREG_MASK_H : 0;
			flags |= (((rd & 0x80) && !(rr & 0x80) && !(res & 0x80)) || (!(rd & 0x80) && (rr & 0x80) && (res & 0x80))) ? SREG_MASK_V : 0;
			flags |= (res & 0x80) ? SREG_MASK_N : 0;
			flags |= (bool(flags & SREG_MASK_N) != bool(flags & SREG_MASK_V)) ? SREG_MASK_S : 0;
			flags |= (res == 0) ? SREG_MASK_Z : 0;
			flags |= ((!(rd & 0x80) && (rr & 0x80)) || ((rr & 0x80) && (res & 0x80)) || ((res & 0x80) && !(rd & 0x80))) ? SREG_MASK_C : 0;
			m_sub_flag_cache[(rd << 8) | rr] = flags;
		}
	}
}

void avr8_base_device::populate_sbc_flag_cache()
{
	for (uint16_t rd = 0; rd < 0x100; rd++)
	{
		for (uint16_t rr = 0; rr < 0x100; rr++)
		{
			for (uint8_t c = 0; c < 2; c++)
			{
				for (uint8_t z = 0; z < 2; z++)
				{
					const uint8_t res = rd - (rr + c);
					uint8_t flags = 0;
					flags |= ((!(rd & 8) && (rr & 8)) || ((rr & 8) && (res & 8)) || ((res & 8) && !(rd & 8))) ? SREG_MASK_H : 0;
					flags |= (((rd & 0x80) && !(rr & 0x80) && !(res & 0x80)) || (!(rd & 0x80) && (rr & 0x80) && (res & 0x80))) ? SREG_MASK_V : 0;
					flags |= (res & 0x80) ? SREG_MASK_N : 0;
					flags |= (bool(flags & SREG_MASK_N) != bool(flags & SREG_MASK_V)) ? SREG_MASK_S : 0;
					flags |= (res == 0) ? (z ? SREG_MASK_Z : 0) : 0;
					flags |= ((!(rd & 0x80) && (rr & 0x80)) || ((rr & 0x80) && (res & 0x80)) || ((res & 0x80) && !(rd & 0x80))) ? SREG_MASK_C : 0;
					m_sbc_flag_cache[(z << 17) | (c << 16) | (rd << 8) | rr] = flags;
				}
			}
		}
	}
}

void avr8_base_device::populate_bool_flag_cache()
{
	for (uint16_t res = 0; res < 0x100; res++)
	{
		uint8_t flags = 0;
		flags |= (res & 0x80) ? SREG_MASK_N : 0;
		flags |= (bool(flags & SREG_MASK_N) != bool(flags & SREG_MASK_V)) ? SREG_MASK_S : 0;
		flags |= (res == 0) ? SREG_MASK_Z : 0;
		m_bool_flag_cache[res] = flags;
	}
}

void avr8_base_device::populate_shift_flag_cache()
{
	for (uint16_t rd = 0; rd < 0x100; rd++)
	{
		for (uint16_t res = 0; res < 0x100; res++)
		{
			uint8_t flags = 0;
			flags |= (rd & 1) ? SREG_MASK_C : 0;
			flags |= (res == 0) ? SREG_MASK_Z : 0;
			flags |= (rd & 0x80) ? SREG_MASK_N : 0;
			flags |= (bool(flags & SREG_MASK_N) != bool(flags & SREG_MASK_C)) ? SREG_MASK_V : 0;
			flags |= (bool(flags & SREG_MASK_N) != bool(flags & SREG_MASK_V)) ? SREG_MASK_S : 0;
			m_shift_flag_cache[(rd << 8) | res] = flags;
		}
	}
}

void avr8_base_device::op_nop(uint16_t op)
{
}

void avr8_base_device::op_movw(uint16_t op)
{
	m_r[(RD4(op) << 1) + 1] = m_r[(RR4(op) << 1) + 1];
	m_r[RD4(op) << 1] = m_r[RR4(op) << 1];
}

void avr8_base_device::op_muls(uint16_t op)
{
	const int16_t sd = (int8_t)m_r[16 + RD4(op)] * (int8_t)m_r[16 + RR4(op)];
	m_r[1] = (sd >> 8) & 0x00ff;
	m_r[0] = sd & 0x00ff;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z);
	if (BIT(sd, 15))
		m_r[SREG] |= SREG_MASK_C;
	else if (sd == 0)
		m_r[SREG] |= SREG_MASK_Z;
}

void avr8_base_device::op_mulsu(uint16_t op)
{
	const int16_t sd = (int8_t)m_r[16 + RD3(op)] * (uint8_t)m_r[16 + RR3(op)];
	m_r[1] = (sd >> 8) & 0x00ff;
	m_r[0] = sd & 0x00ff;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z);
	if (BIT(sd, 15))
		m_r[SREG] |= SREG_MASK_C;
	else if (sd == 0)
		m_r[SREG] |= SREG_MASK_Z;
}

void avr8_base_device::op_fmul(uint16_t op)
{
	const int16_t sd = ((uint8_t)m_r[16 + RD3(op)] * (uint8_t)m_r[16 + RR3(op)]) << 1;
	m_r[1] = (sd >> 8) & 0x00ff;
	m_r[0] = sd & 0x00ff;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z);
	if (BIT(sd, 15))
		m_r[SREG] |= SREG_MASK_C;
	else if (sd == 0)
		m_r[SREG] |= SREG_MASK_Z;
}

void avr8_base_device::op_fmuls(uint16_t op)
{
	const int16_t sd = ((int8_t)m_r[16 + RD3(op)] * (int8_t)m_r[16 + RR3(op)]) << 1;
	m_r[1] = (sd >> 8) & 0x00ff;
	m_r[0] = sd & 0x00ff;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z);
	if (BIT(sd, 15))
		m_r[SREG] |= SREG_MASK_C;
	else if (sd == 0)
		m_r[SREG] |= SREG_MASK_Z;
}

void avr8_base_device::op_fmulsu(uint16_t op)
{
	const int16_t sd = ((int8_t)m_r[16 + RD3(op)] * (uint8_t)m_r[16 + RR3(op)]) << 1;
	m_r[1] = (sd >> 8) & 0x00ff;
	m_r[0] = sd & 0x00ff;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z);
	if (BIT(sd, 15))
		m_r[SREG] |= SREG_MASK_C;
	else if (sd == 0)
		m_r[SREG] |= SREG_MASK_Z;
}

void avr8_base_device::op_cpc(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t rr = m_r[RR5(op)];
	const uint8_t c = m_r[SREG] & SREG_MASK_C;
	const uint32_t z = (m_r[SREG] & SREG_MASK_Z) ? (1 << 17) : 0;
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_sbc_flag_cache[z | (c << 16) | (rd << 8) | rr];
}

void avr8_base_device::op_sbc(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t rr = m_r[RR5(op)];
	const uint8_t c = m_r[SREG] & SREG_MASK_C;
	const uint8_t res = rd - (rr + c);
	m_r[RD5(op)] = res;
	const uint32_t z = (m_r[SREG] & SREG_MASK_Z) ? (1 << 17) : 0;
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_sbc_flag_cache[z | (c << 16) | (rd << 8) | rr];
}

void avr8_base_device::op_add(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t rr = m_r[RR5(op)];
	const uint8_t res = rd + rr;
	m_r[RD5(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_add_flag_cache[(rd << 8) | rr];
}

void avr8_base_device::op_cpse(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t rr = m_r[RR5(op)];
	if (rd == rr)
	{
		const uint16_t data = (uint32_t)m_program->read_word(m_pc + 2);
		m_opcycles += is_long_opcode(data) ? 2 : 1;
		m_pc += is_long_opcode(data) ? 4 : 2;
	}
}

void avr8_base_device::op_cp(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t rr = m_r[RR5(op)];
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_sub_flag_cache[(rd << 8) | rr];
}

void avr8_base_device::op_sub(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t rr = m_r[RR5(op)];
	const uint8_t res = rd - rr;
	m_r[RD5(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_sub_flag_cache[(rd << 8) | rr];
}

void avr8_base_device::op_adc(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t rr = m_r[RR5(op)];
	const uint8_t c = m_r[SREG] & SREG_MASK_C;
	const uint8_t res = rd + rr + c;
	m_r[RD5(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_adc_flag_cache[(c << 16) | (rd << 8) | rr];
}

void avr8_base_device::op_and(uint16_t op)
{
	const uint8_t res = m_r[RD5(op)] & m_r[RR5(op)];
	m_r[RD5(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z);
	m_r[SREG] |= m_bool_flag_cache[res];
}

void avr8_base_device::op_eor(uint16_t op)
{
	const uint8_t res = m_r[RD5(op)] ^ m_r[RR5(op)];
	m_r[RD5(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z);
	m_r[SREG] |= m_bool_flag_cache[res];
}

void avr8_base_device::op_or(uint16_t op)
{
	const uint8_t res = m_r[RD5(op)] | m_r[RR5(op)];
	m_r[RD5(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z);
	m_r[SREG] |= m_bool_flag_cache[res];
}

void avr8_base_device::op_mov(uint16_t op)
{
	m_r[RD5(op)] = m_r[RR5(op)];
}

void avr8_base_device::op_cpi(uint16_t op)
{
	const uint8_t rd = m_r[16 + RD4(op)];
	const uint8_t rr = KCONST8(op);
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_sub_flag_cache[(rd << 8) | rr];
}

void avr8_base_device::op_sbci(uint16_t op)
{
	const uint8_t rd = m_r[16 + RD4(op)];
	const uint8_t rr = KCONST8(op);
	const uint8_t c = m_r[SREG] & SREG_MASK_C;
	const uint8_t res = rd - (rr + c);
	m_r[16 + RD4(op)] = res;
	const uint32_t z = (m_r[SREG] & SREG_MASK_Z) ? (1 << 17) : 0;
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_sbc_flag_cache[z | (c << 16) | (rd << 8) | rr];
}

void avr8_base_device::op_subi(uint16_t op)
{
	const uint8_t rd = m_r[16 + RD4(op)];
	const uint8_t rr = KCONST8(op);
	const uint8_t res = rd - rr;
	m_r[16 + RD4(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_H | SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	m_r[SREG] |= m_sub_flag_cache[(rd << 8) | rr];
}

void avr8_base_device::op_ori(uint16_t op)
{
	const uint8_t res = m_r[16 + RD4(op)] | KCONST8(op);
	m_r[16 + RD4(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z);
	m_r[SREG] |= m_bool_flag_cache[res];
}

void avr8_base_device::op_andi(uint16_t op)
{
	const uint8_t res = m_r[16 + RD4(op)] & KCONST8(op);
	m_r[16 + RD4(op)] = res;
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z);
	m_r[SREG] |= m_bool_flag_cache[res];
}

void avr8_base_device::op_lddz(uint16_t op)
{
	m_r[RD5(op)] = m_data->read_byte(ZREG + QCONST6(op));
}

void avr8_base_device::op_lddy(uint16_t op)
{
	m_r[RD5(op)] = m_data->read_byte(YREG + QCONST6(op));
}

void avr8_base_device::op_stdz(uint16_t op)
{
	m_data->write_byte(ZREG + QCONST6(op), m_r[RD5(op)]);
}

void avr8_base_device::op_stdy(uint16_t op)
{
	m_data->write_byte(YREG + QCONST6(op), m_r[RD5(op)]);
}

void avr8_base_device::op_lds(uint16_t op)
{
	m_pc += 2;
	const uint16_t addr = m_program->read_word(m_pc);
	m_r[RD5(op)] = m_data->read_byte(addr);
}

void avr8_base_device::op_ldzi(uint16_t op)
{
	uint16_t pd = ZREG;
	m_r[RD5(op)] = m_data->read_byte(pd);
	pd++;
	m_r[31] = (pd >> 8) & 0x00ff;
	m_r[30] = pd & 0x00ff;
}

void avr8_base_device::op_ldzd(uint16_t op)
{
	const uint16_t pd = ZREG - 1;
	m_r[RD5(op)] = m_data->read_byte(pd);
	m_r[31] = (pd >> 8) & 0x00ff;
	m_r[30] = pd & 0x00ff;
}

void avr8_base_device::op_lpmz(uint16_t op)
{
	m_r[RD5(op)] = m_program->read_byte(ZREG);
}

void avr8_base_device::op_lpmzi(uint16_t op)
{
	uint16_t pd = ZREG;
	m_r[RD5(op)] = m_program->read_byte(pd);
	pd++;
	m_r[31] = (pd >> 8) & 0x00ff;
	m_r[30] = pd & 0x00ff;
}

void avr8_base_device::op_elpmz(uint16_t op)
{
	m_r[RD5(op)] = m_program->read_byte((m_r[RAMPZ] << 16) | ZREG);
}

void avr8_base_device::op_elpmzi(uint16_t op)
{
	uint32_t pd32 = (m_r[RAMPZ] << 16) | ZREG;
	m_r[RD5(op)] = m_program->read_byte(pd32);
	pd32++;
	m_r[RAMPZ] = (pd32 >> 16) & 0x00ff;
	m_r[31] = (pd32 >> 8) & 0x00ff;
	m_r[30] = pd32 & 0x00ff;
}

void avr8_base_device::op_ldyi(uint16_t op)
{
	uint16_t pd = YREG;
	m_r[RD5(op)] = m_data->read_byte(pd);
	pd++;
	m_r[29] = (pd >> 8) & 0x00ff;
	m_r[28] = pd & 0x00ff;
}

void avr8_base_device::op_ldyd(uint16_t op)
{
	const uint16_t pd = YREG - 1;
	m_r[RD5(op)] = m_data->read_byte(pd);
	m_r[29] = (pd >> 8) & 0x00ff;
	m_r[28] = pd & 0x00ff;
}

void avr8_base_device::op_ldx(uint16_t op)
{
	m_r[RD5(op)] = m_data->read_byte(XREG);
}

void avr8_base_device::op_ldxi(uint16_t op)
{
	uint16_t pd = XREG;
	m_r[RD5(op)] = m_data->read_byte(pd);
	pd++;
	m_r[27] = (pd >> 8) & 0x00ff;
	m_r[26] = pd & 0x00ff;
}

void avr8_base_device::op_ldxd(uint16_t op)
{
	const uint16_t pd = XREG - 1;
	m_r[RD5(op)] = m_data->read_byte(pd);
	m_r[27] = (pd >> 8) & 0x00ff;
	m_r[26] = pd & 0x00ff;
}

void avr8_base_device::op_pop(uint16_t op)
{
	m_r[RD5(op)] = pop();
}

void avr8_base_device::op_sts(uint16_t op)
{
	m_pc += 2;
	const uint16_t addr = m_program->read_word(m_pc);
	m_data->write_byte(addr, m_r[RD5(op)]);
}

void avr8_base_device::op_stzi(uint16_t op)
{
	uint16_t pd = ZREG;
	m_data->write_byte(pd, m_r[RD5(op)]);
	pd++;
	m_r[31] = (pd >> 8) & 0x00ff;
	m_r[30] = pd & 0x00ff;
}

void avr8_base_device::op_stzd(uint16_t op)
{
	const uint16_t pd = ZREG - 1;
	m_data->write_byte(pd, m_r[RD5(op)]);
	m_r[31] = (pd >> 8) & 0x00ff;
	m_r[30] = pd & 0x00ff;
}

void avr8_base_device::op_styi(uint16_t op)
{
	uint16_t pd = YREG;
	m_data->write_byte(pd, m_r[RD5(op)]);
	pd++;
	m_r[29] = (pd >> 8) & 0x00ff;
	m_r[28] = pd & 0x00ff;
}

void avr8_base_device::op_styd(uint16_t op)
{
	const uint16_t pd = YREG - 1;
	m_data->write_byte(pd, m_r[RD5(op)]);
	m_r[29] = (pd >> 8) & 0x00ff;
	m_r[28] = pd & 0x00ff;
}

void avr8_base_device::op_stx(uint16_t op)
{
	m_data->write_byte(XREG, m_r[RD5(op)]);
}

void avr8_base_device::op_stxi(uint16_t op)
{
	uint16_t pd = XREG;
	m_data->write_byte(pd, m_r[RD5(op)]);
	pd++;
	m_r[27] = (pd >> 8) & 0x00ff;
	m_r[26] = pd & 0x00ff;
}

void avr8_base_device::op_stxd(uint16_t op)
{
	const uint16_t pd = XREG - 1;
	m_data->write_byte(pd, m_r[RD5(op)]);
	m_r[27] = (pd >> 8) & 0x00ff;
	m_r[26] = pd & 0x00ff;
}

void avr8_base_device::op_push(uint16_t op)
{
	push(m_r[RD5(op)]);
}

void avr8_base_device::op_com(uint16_t op)
{
	const uint8_t res = ~m_r[RD5(op)];
	m_r[SREG] &= ~(SREG_MASK_Z | SREG_MASK_N | SREG_MASK_V | SREG_MASK_S);
	m_r[SREG] |= SREG_MASK_C;
	if (BIT(res, 7))
		m_r[SREG] |= SREG_MASK_N | SREG_MASK_S;
	else if (res == 0)
		m_r[SREG] |= SREG_MASK_Z;
	m_r[RD5(op)] = res;
}

void avr8_base_device::op_neg(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t res = 0 - rd;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z | SREG_MASK_N | SREG_MASK_V | SREG_MASK_S | SREG_MASK_H);
	if (res == 0)
		m_r[SREG] |= SREG_MASK_Z;
	else
	{
		m_r[SREG] |= SREG_MASK_C;
		if (res == 0x80)
			m_r[SREG] |= SREG_MASK_N | SREG_MASK_V;
		else if (BIT(res, 7))
			m_r[SREG] |= SREG_MASK_N | SREG_MASK_S;
	}
	if (BIT(rd, 3) || BIT(res, 3))
		m_r[SREG] |= SREG_MASK_H;
	m_r[RD5(op)] = res;
}

void avr8_base_device::op_swap(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	m_r[RD5(op)] = (rd >> 4) | (rd << 4);
}

void avr8_base_device::op_inc(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t res = rd + 1;
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z);
	if (res == 0)
		m_r[SREG] |= SREG_MASK_Z;
	else if (res == 0x80)
		m_r[SREG] |= SREG_MASK_N | SREG_MASK_V;
	else if (BIT(res, 7))
		m_r[SREG] |= SREG_MASK_N | SREG_MASK_S;
	m_r[RD5(op)] = res;
}

void avr8_base_device::op_asr(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t res = (rd & 0x80) | (rd >> 1);
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z | SREG_MASK_N | SREG_MASK_V | SREG_MASK_S);
	m_r[SREG] |= m_shift_flag_cache[(rd << 8) | res];
	m_r[RD5(op)] = res;
}

void avr8_base_device::op_lsr(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t res = rd >> 1;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z | SREG_MASK_N | SREG_MASK_V | SREG_MASK_S);
	m_r[SREG] |= m_shift_flag_cache[(rd << 8) | res];
	m_r[RD5(op)] = res;
}

void avr8_base_device::op_ror(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t res = (rd >> 1) | (m_r[SREG] << 7);
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z | SREG_MASK_N | SREG_MASK_V | SREG_MASK_S);
	m_r[SREG] |= m_shift_flag_cache[(rd << 8) | res];
	m_r[RD5(op)] = res;
}

void avr8_base_device::op_setf(uint16_t op)
{
	m_r[SREG] |= 1 << ((op >> 4) & 0x07);
}

void avr8_base_device::op_clrf(uint16_t op)
{
	m_r[SREG] &= ~(1 << ((op >> 4) & 0x07));
}

void avr8_base_device::op_ijmp(uint16_t op)
{
	m_pc = (ZREG << 1) - 2;
}

void avr8_base_device::op_eijmp(uint16_t op)
{
	m_pc = ((m_r[EIND] << 16 | ZREG) << 1) - 2;
}

void avr8_base_device::op_dec(uint16_t op)
{
	const uint8_t rd = m_r[RD5(op)];
	const uint8_t res = rd - 1;
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z);
	if (res == 0)
		m_r[SREG] |= SREG_MASK_Z;
	else if (res == 0x7f)
		m_r[SREG] |= SREG_MASK_V | SREG_MASK_S;
	else if (BIT(res, 7))
		m_r[SREG] |= SREG_MASK_N | SREG_MASK_S;
	m_r[RD5(op)] = res;
}

void avr8_base_device::op_jmp(uint16_t op)
{
	uint32_t offs = KCONST22(op) << 16;
	m_pc += 2;
	offs |= m_program->read_word(m_pc);
	m_pc = offs << 1;
	m_pc -= 2;
}

void avr8_base_device::op_call(uint16_t op)
{
	push(((m_pc >> 1) + 2) & 0x00ff);
	push((((m_pc >> 1) + 2) >> 8) & 0x00ff);
	uint32_t offs = KCONST22(op) << 16;
	m_pc += 2;
	offs |= m_program->read_word(m_pc);
	m_pc = offs << 1;
	m_pc -= 2;
}

void avr8_base_device::op_ret(uint16_t op)
{
	m_pc = pop() << 8;
	m_pc |= pop();
	m_pc = (m_pc << 1) - 2;
}

void avr8_base_device::op_reti(uint16_t op)
{
	m_pc = pop() << 8;
	m_pc |= pop();
	m_pc = (m_pc << 1) - 2;
	m_r[SREG] |= SREG_MASK_I;
}

void avr8_base_device::op_sleep(uint16_t op)
{
	m_pc -= 2;
}

void avr8_base_device::op_break(uint16_t op)
{
	op_unimpl(op);
}

void avr8_base_device::op_wdr(uint16_t op)
{
	LOGMASKED(LOG_WDOG, "%s: Watchdog reset opcode\n", machine().describe_context());
	//op_unimpl(op);
}

void avr8_base_device::op_lpm(uint16_t op)
{
	m_r[0] = m_program->read_byte(ZREG);
}

void avr8_base_device::op_elpm(uint16_t op)
{
	op_unimpl(op);
}

void avr8_base_device::op_spm(uint16_t op)
{
	op_unimpl(op);
}

void avr8_base_device::op_spmzi(uint16_t op)
{
	op_unimpl(op);
}

void avr8_base_device::op_icall(uint16_t op)
{
	push(((m_pc >> 1) + 1) & 0x00ff);
	push((((m_pc >> 1) + 1) >> 8) & 0x00ff);
	m_pc = (ZREG << 1) - 2;
}

void avr8_base_device::op_eicall(uint16_t op)
{
	op_unimpl(op);
}

void avr8_base_device::op_adiw(uint16_t op)
{
	const uint8_t rd = m_r[24 + (DCONST(op) << 1)];
	const uint8_t rr = m_r[25 + (DCONST(op) << 1)];
	const uint16_t pd = ((rr << 8) | rd) + KCONST6(op);
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	if (BIT(pd, 15))
	{
		m_r[SREG] |= SREG_MASK_N;
		if (BIT(rr, 7))
			m_r[SREG] |= SREG_MASK_S;
		else
			m_r[SREG] |= SREG_MASK_V;
	}
	else
	{
		if (pd == 0)
			m_r[SREG] |= SREG_MASK_Z;
		if (BIT(rr, 7))
			m_r[SREG] |= SREG_MASK_C;
	}
	m_r[24 + (DCONST(op) << 1)] = pd & 0x00ff;
	m_r[25 + (DCONST(op) << 1)] = (pd >> 8) & 0x00ff;
}

void avr8_base_device::op_sbiw(uint16_t op)
{
	const uint8_t rd = m_r[24 + (DCONST(op) << 1)];
	const uint8_t rr = m_r[25 + (DCONST(op) << 1)];
	const uint16_t pd = ((rr << 8) | rd) - KCONST6(op);
	m_r[SREG] &= ~(SREG_MASK_V | SREG_MASK_N | SREG_MASK_S | SREG_MASK_Z | SREG_MASK_C);
	if (BIT(pd, 15))
	{
		m_r[SREG] |= SREG_MASK_N | SREG_MASK_S;
		if (!BIT(rr, 7))
			m_r[SREG] |= SREG_MASK_C;
	}
	else
	{
		if (pd == 0)
			m_r[SREG] |= SREG_MASK_Z;
		if (BIT(rr, 7))
			m_r[SREG] |= SREG_MASK_V | SREG_MASK_S;
	}
	m_r[24 + (DCONST(op) << 1)] = pd & 0x00ff;
	m_r[25 + (DCONST(op) << 1)] = (pd >> 8) & 0x00ff;
}

void avr8_base_device::op_cbi(uint16_t op)
{
	m_data->write_byte(32 + ACONST5(op), m_data->read_byte(32 + ACONST5(op)) &~ (1 << RR3(op)));
}

void avr8_base_device::op_sbic(uint16_t op)
{
	if (!BIT(m_data->read_byte(32 + ACONST5(op)), RR3(op)))
	{
		const uint16_t data = (uint32_t)m_program->read_word(m_pc + 2);
		m_opcycles += is_long_opcode(data) ? 2 : 1;
		m_pc += is_long_opcode(data) ? 4 : 2;
	}
}

void avr8_base_device::op_sbi(uint16_t op)
{
	m_data->write_byte(32 + ACONST5(op), m_data->read_byte(32 + ACONST5(op)) | (1 << RR3(op)));
}

void avr8_base_device::op_sbis(uint16_t op)
{
	if (BIT(m_data->read_byte(32 + ACONST5(op)), RR3(op)))
	{
		const uint16_t data = (uint32_t)m_program->read_word(m_pc + 2);
		m_opcycles += is_long_opcode(data) ? 2 : 1;
		m_pc += is_long_opcode(data) ? 4 : 2;
	}
}

void avr8_base_device::op_mul(uint16_t op)
{
	const int16_t sd = (uint8_t)m_r[RD5(op)] * (uint8_t)m_r[RR5(op)];
	m_r[1] = (sd >> 8) & 0x00ff;
	m_r[0] = sd & 0x00ff;
	m_r[SREG] &= ~(SREG_MASK_C | SREG_MASK_Z);
	if (sd == 0)
		m_r[SREG] |= SREG_MASK_Z;
	else if (BIT(sd, 15))
		m_r[SREG] |= SREG_MASK_C;
}

void avr8_base_device::op_out(uint16_t op)
{
	m_data->write_byte(32 + ACONST6(op), m_r[RD5(op)]);
}

void avr8_base_device::op_in(uint16_t op)
{
	m_r[RD5(op)] = m_data->read_byte(0x20 + ACONST6(op));
}

void avr8_base_device::op_rjmp(uint16_t op)
{
	m_pc += (int32_t)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff)) << 1;
}

void avr8_base_device::op_rcall(uint16_t op)
{
	const int32_t offs = (int32_t)((op & 0x0800) ? ((op & 0x0fff) | 0xfffff000) : (op & 0x0fff)) << 1;
	push(((m_pc >> 1) + 1) & 0x00ff);
	push((((m_pc >> 1) + 1) >> 8) & 0x00ff);
	m_pc += offs;
}

void avr8_base_device::op_ldi(uint16_t op)
{
	m_r[16 + RD4(op)] = KCONST8(op);
}

void avr8_base_device::op_brset(uint16_t op)
{
	if (BIT(m_r[SREG], op & 0x0007))
	{
		m_pc += util::sext(KCONST7(op), 7) << 1;
		m_opcycles++;
	}
}

void avr8_base_device::op_brclr(uint16_t op)
{
	if (!BIT(m_r[SREG], op & 0x0007))
	{
		m_pc += util::sext(KCONST7(op), 7) << 1;
		m_opcycles++;
	}
}

void avr8_base_device::op_bst(uint16_t op)
{
	if (BIT(m_r[RD5(op)], RR3(op)))
		m_r[SREG] |= SREG_MASK_T;
	else
		m_r[SREG] &= ~SREG_MASK_T;
}

void avr8_base_device::op_bld(uint16_t op)
{
	if (m_r[SREG] & SREG_MASK_T)
		m_r[RD5(op)] |= (1 << RR3(op));
	else
		m_r[RD5(op)] &= ~(1 << RR3(op));
}

void avr8_base_device::op_sbrs(uint16_t op)
{
	if (BIT(m_r[RD5(op)], RR3(op)))
	{
		const uint16_t data = (uint32_t)m_program->read_word(m_pc + 2);
		m_opcycles += is_long_opcode(data) ? 2 : 1;
		m_pc += is_long_opcode(data) ? 4 : 2;
	}
}

void avr8_base_device::op_sbrc(uint16_t op)
{
	if (!BIT(m_r[RD5(op)], RR3(op)))
	{
		const uint16_t data = (uint32_t)m_program->read_word(m_pc + 2);
		m_opcycles += is_long_opcode(data) ? 2 : 1;
		m_pc += is_long_opcode(data) ? 4 : 2;
	}
}

void avr8_base_device::op_unimpl(uint16_t op)
{
	unimplemented_opcode(op);
}
