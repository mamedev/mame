// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tables.h"
#include "dsp56def.h"

namespace DSP56K
{
/******************/
/* Table decoding */
/******************/
bfShift decode_BBB_table(UINT16 BBB)
{
	switch(BBB)
	{
		case 0x4: return BBB_UPPER;
		case 0x2: return BBB_MIDDLE;
		case 0x1: return BBB_LOWER;
	}

	return BBB_INVALID;
}

void decode_cccc_table(const UINT16 cccc, op_mnem& mnemonic)
{
	switch (cccc)
	{
		case 0x0: mnemonic = oCC; break;
		case 0x1: mnemonic = oGE; break;
		case 0x2: mnemonic = oNE; break;
		case 0x3: mnemonic = oPL; break;
		case 0x4: mnemonic = oNN; break;
		case 0x5: mnemonic = oEC; break;
		case 0x6: mnemonic = oLC; break;
		case 0x7: mnemonic = oGT; break;
		case 0x8: mnemonic = oCS; break;
		case 0x9: mnemonic = oLT; break;
		case 0xa: mnemonic = oEQ; break;
		case 0xb: mnemonic = oMI; break;
		case 0xc: mnemonic = oNR; break;
		case 0xd: mnemonic = oES; break;
		case 0xe: mnemonic = oLS; break;
		case 0xf: mnemonic = oLE; break;
	}

// NEW //   switch (cccc)
// NEW //   {
// NEW //       case 0x0: sprintf(mnemonic, "cc(hs)"); break;
// NEW //       case 0x1: sprintf(mnemonic, "ge    "); break;
// NEW //       case 0x2: sprintf(mnemonic, "ne    "); break;
// NEW //       case 0x3: sprintf(mnemonic, "pl    "); break;
// NEW //       case 0x4: sprintf(mnemonic, "nn    "); break;
// NEW //       case 0x5: sprintf(mnemonic, "ec    "); break;
// NEW //       case 0x6: sprintf(mnemonic, "lc    "); break;
// NEW //       case 0x7: sprintf(mnemonic, "gt    "); break;
// NEW //       case 0x8: sprintf(mnemonic, "cs(lo)"); break;
// NEW //       case 0x9: sprintf(mnemonic, "lt    "); break;
// NEW //       case 0xa: sprintf(mnemonic, "eq    "); break;
// NEW //       case 0xb: sprintf(mnemonic, "mi    "); break;
// NEW //       case 0xc: sprintf(mnemonic, "nr    "); break;
// NEW //       case 0xd: sprintf(mnemonic, "es    "); break;
// NEW //       case 0xe: sprintf(mnemonic, "ls    "); break;
// NEW //       case 0xf: sprintf(mnemonic, "le    "); break;
// NEW //   }
}

void decode_DDDDD_table(const UINT16 DDDDD, reg_id& SD)
{
	switch(DDDDD)
	{
		case 0x00: SD = iX0;  break;
		case 0x01: SD = iY0;  break;
		case 0x02: SD = iX1;  break;
		case 0x03: SD = iY1;  break;
		case 0x04: SD = iA;   break;
		case 0x05: SD = iB;   break;
		case 0x06: SD = iA0;  break;
		case 0x07: SD = iB0;  break;
		case 0x08: SD = iLC;  break;
		case 0x09: SD = iSR;  break;
		case 0x0a: SD = iOMR; break;
		case 0x0b: SD = iSP;  break;
		case 0x0c: SD = iA1;  break;
		case 0x0d: SD = iB1;  break;
		case 0x0e: SD = iA2;  break;
		case 0x0f: SD = iB2;  break;

		case 0x10: SD = iR0;  break;
		case 0x11: SD = iR1;  break;
		case 0x12: SD = iR2;  break;
		case 0x13: SD = iR3;  break;
		case 0x14: SD = iM0;  break;
		case 0x15: SD = iM1;  break;
		case 0x16: SD = iM2;  break;
		case 0x17: SD = iM3;  break;
		case 0x18: SD = iSSH; break;
		case 0x19: SD = iSSL; break;
		case 0x1a: SD = iLA;  break;
		case 0x1b: SD = iINVALID;  break; /* no 0x1b */
		case 0x1c: SD = iN0;  break;
		case 0x1d: SD = iN1;  break;
		case 0x1e: SD = iN2;  break;
		case 0x1f: SD = iN3;  break;
	}
}

void decode_DD_table(const UINT16 DD, reg_id& SD)
{
	switch (DD)
	{
		case 0x0: SD = iX0; break;
		case 0x1: SD = iY0; break;
		case 0x2: SD = iX1; break;
		case 0x3: SD = iY1; break;
	}
}

void decode_DDF_table(const UINT16 DD, const UINT16 F, reg_id& S, reg_id& D)
{
	const UINT16 switchVal = (DD << 1) | F;

	switch (switchVal)
	{
		case 0x0: S = iX0; D = iA; break;
		case 0x1: S = iX0; D = iB; break;
		case 0x2: S = iY0; D = iA; break;
		case 0x3: S = iY0; D = iB; break;
		case 0x4: S = iX1; D = iA; break;
		case 0x5: S = iX1; D = iB; break;
		case 0x6: S = iY1; D = iA; break;
		case 0x7: S = iY1; D = iB; break;
	}
}

void decode_EE_table(const UINT16 EE, reg_id& D)
{
	switch(EE)
	{
		case 0x1: D = iMR;  break;
		case 0x3: D = iCCR; break;
		case 0x2: D = iOMR; break;
	}
}

void decode_F_table(const UINT16 F, reg_id& SD)
{
	switch(F)
	{
		case 0x0: SD = iA; break;
		case 0x1: SD = iB; break;
	}
}

void decode_h0hF_table(const UINT16 h0h, UINT16 F, reg_id& S, reg_id& D)
{
	const UINT16 switchVal = (h0h << 1) | F;

	switch (switchVal)
	{
		case 0x8: S = iX0; D = iA; break;
		case 0x9: S = iX0; D = iB; break;
		case 0xa: S = iY0; D = iA; break;
		case 0xb: S = iY0; D = iB; break;
		case 0x2: S = iA;  D = iA; break;
		case 0x1: S = iA;  D = iB; break;
		case 0x0: S = iB;  D = iA; break;
		case 0x3: S = iB;  D = iB; break;
	}
}

void decode_HH_table(const UINT16 HH, reg_id& SD)
{
	switch(HH)
	{
		case 0x0: SD = iX0; break;
		case 0x1: SD = iY0; break;
		case 0x2: SD = iA;  break;
		case 0x3: SD = iB;  break;
	}
}

void decode_HHH_table(const UINT16 HHH, reg_id& SD)
{
	switch(HHH)
	{
		case 0x0: SD = iX0; break;
		case 0x1: SD = iY0; break;
		case 0x2: SD = iX1; break;
		case 0x3: SD = iY1; break;
		case 0x4: SD = iA;  break;
		case 0x5: SD = iB;  break;
		case 0x6: SD = iA0; break;
		case 0x7: SD = iB0; break;
	}
}

void decode_IIIIx_table(const UINT16 IIII, const UINT16 x, reg_id& S, reg_id& D)
{
	S = D = iINVALID;
	switch(IIII)
	{
		case 0x0: S = iX0; D = iFHAT; break;
		case 0x1: S = iY0; D = iFHAT; break;
		case 0x2: S = iX1; D = iFHAT; break;
		case 0x3: S = iY1; D = iFHAT; break;
		case 0x4: S = iA;  D = iX0; break;
		case 0x5: S = iB;  D = iY0; break;
		case 0x6: S = iA0; D = iX0; break;
		case 0x7: S = iB0; D = iY0; break;
		case 0x8: if ( x) S = iF;  D = iFHAT; break;
		case 0x9: if (!x) S = iF;  D = iFHAT; break;
		case 0xa: S = iWEIRD;  D = iWEIRD; break;
		case 0xb: S = iWEIRD;  D = iWEIRD; break;
		case 0xc: S = iA;  D = iX1; break;
		case 0xd: S = iB;  D = iY1; break;
		case 0xe: S = iA0; D = iX1; break;
		case 0xf: S = iB0; D = iY1; break;
	}
}

void decode_JJJF_table(const UINT16 JJJ, const UINT16 F, reg_id& S, reg_id& D)
{
	const UINT16 switchVal = (JJJ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S = iB;  D = iA; break;
		case 0x1: S = iA;  D = iB; break;
		case 0x2: S = iINVALID; D = iINVALID; break;
		case 0x3: S = iINVALID; D = iINVALID; break;
		case 0x4: S = iX;  D = iA; break;
		case 0x5: S = iX;  D = iB; break;
		case 0x6: S = iY;  D = iA; break;
		case 0x7: S = iY;  D = iB; break;
		case 0x8: S = iX0; D = iA; break;
		case 0x9: S = iX0; D = iB; break;
		case 0xa: S = iY0; D = iA; break;
		case 0xb: S = iY0; D = iB; break;
		case 0xc: S = iX1; D = iA; break;
		case 0xd: S = iX1; D = iB; break;
		case 0xe: S = iY1; D = iA; break;
		case 0xf: S = iY1; D = iB; break;
	}
}

void decode_JJF_table(const UINT16 JJ, const UINT16 F, reg_id& S, reg_id& D)
{
	const UINT16 switchVal = (JJ << 1) | F;

	switch (switchVal)
	{
		case 0x0: S = iX0; D = iA; break;
		case 0x1: S = iX0; D = iB; break;
		case 0x2: S = iY0; D = iA; break;
		case 0x3: S = iY0; D = iB; break;
		case 0x4: S = iX1; D = iA; break;
		case 0x5: S = iX1; D = iB; break;
		case 0x6: S = iY1; D = iA; break;
		case 0x7: S = iY1; D = iB; break;
	}
}

void decode_JF_table(const UINT16 J, const UINT16 F, reg_id& S, reg_id& D)
{
	const UINT16 switchVal = (J << 1) | F;

	switch(switchVal)
	{
		case 0x0: S = iX; D = iA; break;
		case 0x1: S = iX; D = iB; break;
		case 0x2: S = iY; D = iA; break;
		case 0x3: S = iY; D = iB; break;
	}
}

// NEW // void decode_k_table(UINT16 k, char *Dnot)
// NEW // {
// NEW //   switch(k)
// NEW //   {
// NEW //       case 0x0: sprintf(Dnot, "B"); break;
// NEW //       case 0x1: sprintf(Dnot, "A"); break;
// NEW //   }
// NEW // }

void decode_kSign_table(const UINT16 k, std::string& plusMinus)
{
	switch(k)
	{
		case 0x0: plusMinus = "+"; break;
		case 0x1: plusMinus = "-"; break;
	}
}

void decode_KKK_table(const UINT16 KKK, reg_id& D1, reg_id& D2)
{
	switch(KKK)
	{
		case 0x0: D1 = iFHAT; D2 = iX0; break;
		case 0x1: D1 = iY0;   D2 = iX0; break;
		case 0x2: D1 = iX1;   D2 = iX0; break;
		case 0x3: D1 = iY1;   D2 = iX0; break;
		case 0x4: D1 = iX0;   D2 = iX1; break;
		case 0x5: D1 = iY0;   D2 = iX1; break;
		case 0x6: D1 = iFHAT; D2 = iY0; break;
		case 0x7: D1 = iY1;   D2 = iX1; break;
	}
}

void decode_NN_table(UINT16 NN, reg_id& ret)
{
	switch(NN)
	{
		case 0x0: ret = iN0; break;
		case 0x1: ret = iN1; break;
		case 0x2: ret = iN2; break;
		case 0x3: ret = iN3; break;
	}
}

void decode_TT_table(UINT16 TT, reg_id& ret)
{
	switch(TT)
	{
		case 0x0: ret = iR0; break;
		case 0x1: ret = iR1; break;
		case 0x2: ret = iR2; break;
		case 0x3: ret = iR3; break;
	}
}

void decode_QQF_table(const UINT16 QQ, const UINT16 F, reg_id& S1, reg_id& S2, reg_id& D)
{
	const UINT16 switchVal = (QQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S1 = iY0; S2 = iX0; D = iA; break;
		case 0x1: S1 = iY0; S2 = iX0; D = iB; break;
		case 0x2: S1 = iY1; S2 = iX0; D = iA; break;
		case 0x3: S1 = iY1; S2 = iX0; D = iB; break;
		case 0x4: S1 = iY0; S2 = iX1; D = iA; break;
		case 0x5: S1 = iY0; S2 = iX1; D = iB; break;
		case 0x6: S1 = iY1; S2 = iX1; D = iA; break;
		case 0x7: S1 = iY1; S2 = iX1; D = iB; break;
	}
}

void decode_QQF_special_table(const UINT16 QQ, const UINT16 F, reg_id& S1, reg_id& S2, reg_id& D)
{
	const UINT16 switchVal = (QQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S1 = iY0; S2 = iX0; D = iA; break;
		case 0x1: S1 = iY0; S2 = iX0; D = iB; break;
		case 0x2: S1 = iY1; S2 = iX0; D = iA; break;
		case 0x3: S1 = iY1; S2 = iX0; D = iB; break;
		case 0x4: S1 = iX1; S2 = iY0; D = iA; break;
		case 0x5: S1 = iX1; S2 = iY0; D = iB; break;
		case 0x6: S1 = iX1; S2 = iY1; D = iA; break;
		case 0x7: S1 = iX1; S2 = iY1; D = iB; break;
	}
}

void decode_QQQF_table(const UINT16 QQQ, const UINT16 F, reg_id& S1, reg_id& S2, reg_id& D)
{
	const UINT16 switchVal = (QQQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S1 = iX0; S2 = iX0; D = iA; break;
		case 0x1: S1 = iX0; S2 = iX0; D = iB; break;
		case 0x2: S1 = iX1; S2 = iX0; D = iA; break;
		case 0x3: S1 = iX1; S2 = iX0; D = iB; break;
		case 0x4: S1 = iA1; S2 = iY0; D = iA; break;
		case 0x5: S1 = iA1; S2 = iY0; D = iB; break;
		case 0x6: S1 = iB1; S2 = iX0; D = iA; break;
		case 0x7: S1 = iB1; S2 = iX0; D = iB; break;
		case 0x8: S1 = iY0; S2 = iX0; D = iA; break;
		case 0x9: S1 = iY0; S2 = iX0; D = iB; break;
		case 0xa: S1 = iY1; S2 = iX0; D = iA; break;
		case 0xb: S1 = iY1; S2 = iX0; D = iB; break;
		case 0xc: S1 = iY0; S2 = iX1; D = iA; break;
		case 0xd: S1 = iY0; S2 = iX1; D = iB; break;
		case 0xe: S1 = iY1; S2 = iX1; D = iA; break;
		case 0xf: S1 = iY1; S2 = iX1; D = iB; break;
	}
}

void decode_RR_table(UINT16 RR, reg_id& ret)
{
	switch(RR)
	{
		case 0x0: ret = iR0; break;
		case 0x1: ret = iR1; break;
		case 0x2: ret = iR2; break;
		case 0x3: ret = iR3; break;
	}
}

void decode_rr_table(UINT16 rr, reg_id& ret)
{
	switch(rr)
	{
		case 0x0: ret = iR0; break;
		case 0x1: ret = iR1; break;
		case 0x2: ret = iR2; break;
		case 0x3: ret = iR3; break;
	}
}

void decode_s_table(const UINT16 s, op_mnem& arithmetic)
{
	switch(s)
	{
		case 0x0: arithmetic = oSU; break;
		case 0x1: arithmetic = oUU; break;
	}
}

void decode_ss_table(const UINT16 ss, op_mnem& arithmetic)
{
	switch(ss)
	{
		case 0x0: arithmetic = oSS; break;
		case 0x1: arithmetic = oINVALID; break;
		// NEW // case 0x1: arithmetic = "ss"; break;
		case 0x2: arithmetic = oSU; break;
		case 0x3: arithmetic = oUU; break;
	}
}

void decode_uuuuF_table(const UINT16 uuuu, const UINT16 F, std::string& arg, reg_id& S, reg_id& D)
{
	const UINT16 switchVal = (uuuu << 1) | F;

	//D = "sub?";
	//S = "add";
	arg = "invalid";

	switch(switchVal)
	{
		case 0x00: arg = "add"; S = iX0; D = iA; break;
		case 0x01: arg = "add"; S = iX0; D = iB; break;
		case 0x02: arg = "add"; S = iY0; D = iA; break;
		case 0x03: arg = "add"; S = iY0; D = iB; break;
		case 0x04: arg = "add"; S = iX1; D = iA; break;
		case 0x05: arg = "add"; S = iX1; D = iB; break;
		case 0x06: arg = "add"; S = iY1; D = iA; break;
		case 0x07: arg = "add"; S = iY1; D = iB; break;

		case 0x08: arg = "sub"; S = iX0; D = iA; break;
		case 0x09: arg = "sub"; S = iX0; D = iB; break;
		case 0x0a: arg = "sub"; S = iY0; D = iA; break;
		case 0x0b: arg = "sub"; S = iY0; D = iB; break;
		case 0x0c: arg = "sub"; S = iX1; D = iA; break;
		case 0x0d: arg = "sub"; S = iX1; D = iB; break;
		case 0x0e: arg = "sub"; S = iY1; D = iA; break;
		case 0x0f: arg = "sub"; S = iY1; D = iB; break;

		case 0x18: arg = "add"; S = iB;  D = iA; break;
		case 0x19: arg = "add"; S = iA;  D = iB; break;

		case 0x1a: arg = "sub"; S = iB;  D = iA; break;
		case 0x1b: arg = "sub"; S = iA;  D = iB; break;

		case 0x1c: arg = "tfr"; S = iB;  D = iA; break;
		case 0x1d: arg = "tfr"; S = iA;  D = iB; break;

		case 0x1e: arg = "move"; S = iINVALID;  D = iINVALID; break;
		case 0x1f: arg = "move"; S = iINVALID;  D = iINVALID; break;
	}
}

void decode_Z_table(const UINT16 Z, std::string& ea)
{
	/* This is fixed as per the Family Manual errata addendum */
	switch(Z)
	{
		case 0x1: ea = "(A1)"; break;
		case 0x0: ea = "(B1)"; break;
	}
}

void assemble_ea_from_m_table(const UINT16 m, const int n, std::string& ea)
{
	char temp[32];
	switch(m)
	{
		case 0x0: sprintf(temp, "(R%d)+",n)       ; break;
		case 0x1: sprintf(temp, "(R%d)+N%d", n, n); break;
	}
	ea = temp;
}

void assemble_eas_from_mm_table(UINT16 mm, int n1, int n2, std::string& ea1, std::string& ea2)
{
	char temp1[32];
	char temp2[32];
	switch(mm)
	{
		case 0x0: sprintf(temp1, "(R%d)+",  n1) ;
					sprintf(temp2, "(R%d)+",    n2) ; break;
		case 0x1: sprintf(temp1, "(R%d)+",  n1) ;
					sprintf(temp2, "(R%d)+N%d", n2, n2); break;
		case 0x2: sprintf(temp1, "(R%d)+N%d", n1, n1);
					sprintf(temp2, "(R%d)+",    n2) ; break;
		case 0x3: sprintf(temp1, "(R%d)+N%d", n1, n1);
					sprintf(temp2, "(R%d)+N%d", n2, n2); break;
	}
	ea1 = temp1;
	ea2 = temp2;
}

void assemble_ea_from_MM_table(UINT16 MM, int n, std::string& ea)
{
	char temp[32];
	switch(MM)
	{
		case 0x0: sprintf(temp, "(R%d)",     n)   ; break;
		case 0x1: sprintf(temp, "(R%d)+",   n)   ; break;
		case 0x2: sprintf(temp, "(R%d)-",   n)   ; break;
		case 0x3: sprintf(temp, "(R%d)+N%d", n, n); break;
	}
	ea = temp;
}

void assemble_ea_from_q_table(UINT16 q, int n, std::string& ea)
{
	char temp[32];
	switch(q)
	{
		case 0x0: sprintf(temp, "(R%d+N%d)", n, n); break;
		case 0x1: sprintf(temp, "-(R%d)",   n)   ; break;
	}
	ea = temp;
}

void assemble_ea_from_t_table(UINT16 t, UINT16 val, std::string& ea)
{
	char temp[32];
	switch(t)
	{
		case 0x0: sprintf(temp, "X:>$%x", val); break;
		case 0x1: sprintf(temp, "#>$%x", val);  break;
		// NEW // case 0x0: sprintf(ea, "X:$%04x", val); break;
		// NEW // case 0x1: sprintf(ea, "#$%04x", val);  break;
	}
	ea = temp;
}

void assemble_ea_from_z_table(UINT16 z, int n, std::string& ea)
{
	char temp[32];
	switch(z)
	{
		case 0x0: sprintf(temp, "(R%d)-",   n)   ; break;
		case 0x1: sprintf(temp, "(R%d)+N%d", n, n); break;
	}
	ea = temp;
}

void assemble_D_from_P_table(UINT16 P, UINT16 ppppp, std::string& D)
{
	char temp[32];
	std::string fullAddy;    /* Convert Short Absolute Address to full 16-bit */

	switch(P)
	{
		case 0x0:
			sprintf(temp, "X:<$%x", ppppp);
			// NEW // sprintf(temp, "X:$%02x", ppppp);
			break;
		case 0x1:
			assemble_address_from_IO_short_address(ppppp, fullAddy);
			sprintf(temp, "X:<<$%s", fullAddy.c_str());
			// NEW // sprintf(temp, "X:$%s", fullAddy.c_str());
			break;
	}
	D = temp;
}

void assemble_arguments_from_W_table(UINT16 W, char ma, const reg_id& SD, const std::string& ea,
	std::string& source, std::string& destination)
{
	char temp[32];
	sprintf(temp, "%c:%s", ma, ea.c_str());
	switch(W)
	{
		case 0x0: source = regIdAsString(SD); destination = temp; break;
		case 0x1: source = temp; destination = regIdAsString(SD); break;
	}
}

void assemble_arguments_from_W_table(UINT16 W, char ma, const std::string& SD, const std::string& ea,
	std::string& source, std::string& destination)
{
	char temp[32];
	sprintf(temp, "%c:%s", ma, ea.c_str());
	switch(W)
	{
		case 0x0: source = SD;   destination = temp; break;
		case 0x1: source = temp; destination = SD;   break;
	}
}

void assemble_reg_from_W_table(UINT16 W, char ma, const reg_id& SD, const INT8 xx, std::string& S, std::string& D)
{
	UINT8 abs_xx;
	char temp[32];
	char operation[32];

	if(xx < 0)
		sprintf(operation,"-");
	else
		sprintf(operation,"+");

	abs_xx = abs(xx);

	sprintf(temp, "%c:(R2%s$%x)", ma, operation, abs_xx);
	// NEW // sprintf(temp, "%c:(R2%s$%02x)", ma, operation, abs_xx);
	switch(W)
	{
		case 0x0: S = regIdAsString(SD); D = temp; break;
		case 0x1: S = temp; D = regIdAsString(SD); break;
	}
}

void assemble_address_from_IO_short_address(UINT16 pp, std::string& ea)
{
	char temp[32];

	UINT16 fullAddy = 0xffe0;
	fullAddy |= pp;

	sprintf(temp, "%.04x", fullAddy);
	ea = temp;
}

INT8 get_6_bit_signed_value(UINT16 bits)
{
	UINT16 fullAddy = bits;
	if (fullAddy & 0x0020)
		fullAddy |= 0xffc0;

	return (INT8)fullAddy;
}


/********************/
/* HELPER FUNCTIONS */
/********************/

UINT16 dsp56k_op_maskn(UINT16 cur, UINT16 mask)
{
	int i;

	UINT16 retVal = (cur & mask);
	UINT16 temp = 0x0000;
	int offsetCount = 0;

	/* Shift everything right, eliminating 'whitespace'... */
	for (i = 0; i < 16; i++)
	{
		if (mask & (0x1<<i))        /* If mask bit is non-zero */
		{
			temp |= (((retVal >> i) & 0x1) << offsetCount);
			offsetCount++;
		}
	}

	return temp;
}

bool registerOverlap(const reg_id& r0, const size_t bmd, const reg_id& r1)
{
	if (bmd == BM_NONE)
		return false;

	if (r0 == r1)
		return true;

	if (r0 == iA && (bmd & BM_LOW)    && r1 == iA0) return true;
	if (r0 == iA && (bmd & BM_MIDDLE) && r1 == iA1) return true;
	if (r0 == iA && (bmd & BM_HIGH)   && r1 == iA2) return true;

	if (r0 == iB && (bmd & BM_LOW)    && r1 == iB0) return true;
	if (r0 == iB && (bmd & BM_MIDDLE) && r1 == iB1) return true;
	if (r0 == iB && (bmd & BM_HIGH)   && r1 == iB2) return true;

	return false;
}

UINT16 regValue16(dsp56k_core* cpustate, const reg_id& reg)
{
	if (reg == iX0) return X0;
	if (reg == iX1) return X1;
	if (reg == iY0) return Y0;
	if (reg == iY1) return Y1;

	if (reg == iA0) return A0;
	if (reg == iA1) return A1;
	if (reg == iB0) return B0;
	if (reg == iB1) return B1;

	if (reg == iR0) return R0;
	if (reg == iR1) return R1;
	if (reg == iR2) return R2;
	if (reg == iR3) return R3;

	if (reg == iN0) return N0;
	if (reg == iN1) return N1;
	if (reg == iN2) return N2;
	if (reg == iN3) return N3;

	if (reg == iM0) return M0;
	if (reg == iM1) return M1;
	if (reg == iM2) return M2;
	if (reg == iM3) return M3;

	osd_printf_debug("The dsp561xx core is requesting a 16 bit value from non-16 bit register!");
	return 0xdead;
}

void setReg16(dsp56k_core* cpustate, const UINT16& value, const reg_id& reg)
{
	if (reg == iX0) X0 = value;
	if (reg == iX1) X1 = value;
	if (reg == iY0) Y0 = value;
	if (reg == iY1) Y1 = value;

	if (reg == iA0) A0 = value;
	if (reg == iA1) A1 = value;
	if (reg == iB0) B0 = value;
	if (reg == iB1) B1 = value;

	if (reg == iR0) R0 = value;
	if (reg == iR1) R1 = value;
	if (reg == iR2) R2 = value;
	if (reg == iR3) R3 = value;

	if (reg == iN0) N0 = value;
	if (reg == iN1) N1 = value;
	if (reg == iN2) N2 = value;
	if (reg == iN3) N3 = value;

	if (reg == iM0) M0 = value;
	if (reg == iM1) M1 = value;
	if (reg == iM2) M2 = value;
	if (reg == iM3) M3 = value;
}

std::string regIdAsString(const reg_id& regId)
{
	switch(regId)
	{
		case iX:  return "X";
		case iX0: return "X0";
		case iX1: return "X1";
		case iY:  return "Y";
		case iY0: return "Y0";
		case iY1: return "Y1";
		case iA:  return "A";
		case iA0: return "A0";
		case iA1: return "A1";
		case iA2: return "A2";
		case iB:  return "B";
		case iB0: return "B0";
		case iB1: return "B1";
		case iB2: return "B2";
		case iR0: return "R0";
		case iR1: return "R1";
		case iR2: return "R2";
		case iR3: return "R3";
		case iN0: return "N0";
		case iN1: return "N1";
		case iN2: return "N2";
		case iN3: return "N3";
		case iM0: return "M0";
		case iM1: return "M1";
		case iM2: return "M2";
		case iM3: return "M3";
		case iLC: return "LC";
		case iSR: return "SR";
		case iOMR: return "OMR";
		case iSP:  return "SP";
		case iSSH: return "SSH";
		case iSSL: return "SSL";
		case iLA:  return "LA";
		case iMR:  return "MR";
		case iCCR: return "CCR";
		case iF:   return "F";
		case iFHAT: return "^F";
		case iINVALID: return "!!";
		case iWEIRD: return "?";
	}

	return "INVALID_REG_ID";
}

std::string opMnemonicAsString(const op_mnem& mnem)
{
	switch(mnem)
	{
		case oCC: return "cc";
		case oGE: return "ge";
		case oNE: return "ne";
		case oPL: return "pl";
		case oNN: return "nn";
		case oEC: return "ec";
		case oLC: return "lc";
		case oGT: return "gt";
		case oCS: return "cs";
		case oLT: return "lt";
		case oEQ: return "eq";
		case oMI: return "mi";
		case oNR: return "nr";
		case oES: return "es";
		case oLS: return "ls";
		case oLE: return "le";

		case oSS: return "ss";
		case oSU: return "su";
		case oUU: return "uu";
		case oINVALID: return "!!";
	}

	return "INVALID_OPCODE_MNEMONIC";
}

reg_id stringAsRegID(const std::string& str)
{
	if (str.compare("X")==0) return iX;
	if (str.compare("X0") == 0) return iX0;
	if (str.compare("X1") == 0) return iX1;
	if (str.compare("Y") == 0) return iY;
	if (str.compare("Y0") == 0) return iY0;
	if (str.compare("Y1") == 0) return iY1;
	if (str.compare("A") == 0) return iA;
	if (str.compare("A0") == 0) return iA0;
	if (str.compare("A1") == 0) return iA1;
	if (str.compare("A2") == 0) return iA2;
	if (str.compare("B") == 0) return iB;
	if (str.compare("B0") == 0) return iB0;
	if (str.compare("B1") == 0) return iB1;
	if (str.compare("B2") == 0) return iB2;
	if (str.compare("R0") == 0) return iR0;
	if (str.compare("R1") == 0) return iR1;
	if (str.compare("R2") == 0) return iR2;
	if (str.compare("R3") == 0) return iR3;
	if (str.compare("N0") == 0) return iN0;
	if (str.compare("N1") == 0) return iN1;
	if (str.compare("N2") == 0) return iN2;
	if (str.compare("N3") == 0) return iN3;
	if (str.compare("M0") == 0) return iM0;
	if (str.compare("M1") == 0) return iM1;
	if (str.compare("M2") == 0) return iM2;
	if (str.compare("M3") == 0) return iM3;
	if (str.compare("LC") == 0) return iLC;
	if (str.compare("SR") == 0) return iSR;
	if (str.compare("OMR") == 0) return iOMR;
	if (str.compare("SP") == 0) return iSP;
	if (str.compare("SSH") == 0) return iSSH;
	if (str.compare("SSL") == 0) return iSSL;
	if (str.compare("LA") == 0) return iLA;
	if (str.compare("MR") == 0) return iMR;
	if (str.compare("CCR") == 0) return iCCR;
	if (str.compare("F") == 0) return iF;
	if (str.compare("^F") == 0) return iFHAT;
	if (str.compare("!!") == 0) return iINVALID;
	if (str.compare("?") == 0)return iWEIRD;

	return iINVALID;
}

UINT8 regIDAsNum(const reg_id& regId)
{
	if (regId == iR0) return 0;
	if (regId == iR1) return 1;
	if (regId == iR2) return 2;
	if (regId == iR3) return 3;

	if (regId == iN0) return 0;
	if (regId == iN1) return 1;
	if (regId == iN2) return 2;
	if (regId == iN3) return 3;

	if (regId == iM0) return 0;
	if (regId == iM1) return 1;
	if (regId == iM2) return 2;
	if (regId == iM3) return 3;

	return 255;
}

}
