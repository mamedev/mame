// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Alex W. Jackson
#define GetRB   \
	ModRM = FETCH();    \
	if (ModRM >= 0xc0)  \
		tmp = Wreg(Mod_RM.RM.w[ModRM]) & 0x7;   \
	else {                          \
		logerror("%06x: Invalid MODRM for register banking instruction\n",PC());   \
		tmp = 0;    \
	}

#define RETRBI  \
	tmp = (Wreg(PSW_SAVE) & 0x7000) >> 12;  \
	m_ip = Wreg(PC_SAVE);  \
	ExpandFlags(Wreg(PSW_SAVE));    \
	SetRB(tmp); \
	CHANGE_PC

#define TSKSW   \
	Wreg(PSW_SAVE) = CompressFlags();   \
	Wreg(PC_SAVE) = m_ip;  \
	SetRB(tmp); \
	m_ip = Wreg(PC_SAVE);  \
	ExpandFlags(Wreg(PSW_SAVE));    \
	CHANGE_PC

#define MOVSPA  \
	tmp = (Wreg(PSW_SAVE) & 0x7000) >> 8;   \
	Sreg(SS) = m_ram.w[tmp+SS];    \
	Wreg(SP) = m_ram.w[tmp+SP]

#define MOVSPB  \
	tmp <<= 4;  \
	m_ram.w[tmp+SS] = Sreg(SS);    \
	m_ram.w[tmp+SP] = Wreg(SP)

#define FINT    \
	for(tmp = 1; tmp < 0x100; tmp <<= 1) {  \
		if(m_ISPR & tmp) {     \
			m_ISPR &= ~tmp;    \
			break;  \
		}   \
	}

OP( 0x0f, i_pre_v25  ) { UINT32 ModRM, tmp, tmp2;
	switch (FETCH()) {
		case 0x10 : BITOP_BYTE; CLKS(3,3,4); tmp2 = Breg(CL) & 0x7; m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0; m_CarryVal=m_OverVal=0; break; /* Test */
		case 0x11 : BITOP_WORD; CLKS(3,3,4); tmp2 = Breg(CL) & 0xf; m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0; m_CarryVal=m_OverVal=0; break; /* Test */
		case 0x12 : BITOP_BYTE; CLKS(5,5,4); tmp2 = Breg(CL) & 0x7; tmp &= ~(1<<tmp2);  PutbackRMByte(ModRM,tmp);   break; /* Clr */
		case 0x13 : BITOP_WORD; CLKS(5,5,4); tmp2 = Breg(CL) & 0xf; tmp &= ~(1<<tmp2);  PutbackRMWord(ModRM,tmp);   break; /* Clr */
		case 0x14 : BITOP_BYTE; CLKS(4,4,4); tmp2 = Breg(CL) & 0x7; tmp |= (1<<tmp2);   PutbackRMByte(ModRM,tmp);   break; /* Set */
		case 0x15 : BITOP_WORD; CLKS(4,4,4); tmp2 = Breg(CL) & 0xf; tmp |= (1<<tmp2);   PutbackRMWord(ModRM,tmp);   break; /* Set */
		case 0x16 : BITOP_BYTE; CLKS(4,4,4); tmp2 = Breg(CL) & 0x7; BIT_NOT;            PutbackRMByte(ModRM,tmp);   break; /* Not */
		case 0x17 : BITOP_WORD; CLKS(4,4,4); tmp2 = Breg(CL) & 0xf; BIT_NOT;            PutbackRMWord(ModRM,tmp);   break; /* Not */

		case 0x18 : BITOP_BYTE; CLKS(4,4,4); tmp2 = (FETCH()) & 0x7;    m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0; m_CarryVal=m_OverVal=0; break; /* Test */
		case 0x19 : BITOP_WORD; CLKS(4,4,4); tmp2 = (FETCH()) & 0xf;    m_ZeroVal = (tmp & (1<<tmp2)) ? 1 : 0; m_CarryVal=m_OverVal=0; break; /* Test */
		case 0x1a : BITOP_BYTE; CLKS(6,6,4); tmp2 = (FETCH()) & 0x7;    tmp &= ~(1<<tmp2);      PutbackRMByte(ModRM,tmp);   break; /* Clr */
		case 0x1b : BITOP_WORD; CLKS(6,6,4); tmp2 = (FETCH()) & 0xf;    tmp &= ~(1<<tmp2);      PutbackRMWord(ModRM,tmp);   break; /* Clr */
		case 0x1c : BITOP_BYTE; CLKS(5,5,4); tmp2 = (FETCH()) & 0x7;    tmp |= (1<<tmp2);       PutbackRMByte(ModRM,tmp);   break; /* Set */
		case 0x1d : BITOP_WORD; CLKS(5,5,4); tmp2 = (FETCH()) & 0xf;    tmp |= (1<<tmp2);       PutbackRMWord(ModRM,tmp);   break; /* Set */
		case 0x1e : BITOP_BYTE; CLKS(5,5,4); tmp2 = (FETCH()) & 0x7;    BIT_NOT;                PutbackRMByte(ModRM,tmp);   break; /* Not */
		case 0x1f : BITOP_WORD; CLKS(5,5,4); tmp2 = (FETCH()) & 0xf;    BIT_NOT;                PutbackRMWord(ModRM,tmp);   break; /* Not */

		case 0x20 : ADD4S; CLKS(7,7,2); break;
		case 0x22 : SUB4S; CLKS(7,7,2); break;
		case 0x25 : MOVSPA; CLK(16); break;
		case 0x26 : CMP4S; CLKS(7,7,2); break;
		case 0x28 : ModRM = FETCH(); tmp = GetRMByte(ModRM); tmp <<= 4; tmp |= Breg(AL) & 0xf; Breg(AL) = (Breg(AL) & 0xf0) | ((tmp>>8)&0xf); tmp &= 0xff; PutbackRMByte(ModRM,tmp); CLKM(13,13,9,28,28,15); break;
		case 0x2a : ModRM = FETCH(); tmp = GetRMByte(ModRM); tmp2 = (Breg(AL) & 0xf)<<4; Breg(AL) = (Breg(AL) & 0xf0) | (tmp&0xf); tmp = tmp2 | (tmp>>4);   PutbackRMByte(ModRM,tmp); CLKM(17,17,13,32,32,19); break;
		case 0x2d : GetRB; nec_bankswitch(tmp); CLK(15); break;
		case 0x31 : ModRM = FETCH(); ModRM=0; logerror("%06x: Unimplemented bitfield INS\n",PC()); break;
		case 0x33 : ModRM = FETCH(); ModRM=0; logerror("%06x: Unimplemented bitfield EXT\n",PC()); break;
		case 0x91 : RETRBI; CLK(12); break;
		case 0x92 : FINT; CLK(2); m_no_interrupt = 1; break;
		case 0x94 : GetRB; TSKSW; CLK(20); break;
		case 0x95 : GetRB; MOVSPB; CLK(11); break;
		case 0x9e : logerror("%06x: STOP\n",PC()); m_icount=0; break;
		default:    logerror("%06x: Unknown V25 instruction\n",PC()); break;
	}
}

OP( 0x63, i_brkn   ) { nec_interrupt(FETCH(), BRKN); CLKS(50,50,24); }
OP( 0xF1, i_brks   ) { nec_interrupt(FETCH(), BRKS); CLKS(50,50,24); }
