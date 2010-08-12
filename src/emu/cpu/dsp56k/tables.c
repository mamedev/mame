#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "tables.h"

namespace DSP56K
{

/******************/
/* Table decoding */
/******************/
int decode_BBB_table(UINT16 BBB)
{
	switch(BBB)
	{
		case 0x4: return BBB_UPPER;
		case 0x2: return BBB_MIDDLE;
		case 0x1: return BBB_LOWER;
	}

	return BBB_INVALID;
}

void decode_cccc_table(const UINT16 cccc, std::string& mnemonic)
{
	switch (cccc)
	{
		case 0x0: mnemonic = "cc"; break;
		case 0x1: mnemonic = "ge"; break;
		case 0x2: mnemonic = "ne"; break;
		case 0x3: mnemonic = "pl"; break;
		case 0x4: mnemonic = "nn"; break;
		case 0x5: mnemonic = "ec"; break;
		case 0x6: mnemonic = "lc"; break;
		case 0x7: mnemonic = "gt"; break;
		case 0x8: mnemonic = "cs"; break;
		case 0x9: mnemonic = "lt"; break;
		case 0xa: mnemonic = "eq"; break;
		case 0xb: mnemonic = "mi"; break;
		case 0xc: mnemonic = "nr"; break;
		case 0xd: mnemonic = "es"; break;
		case 0xe: mnemonic = "ls"; break;
		case 0xf: mnemonic = "le"; break;
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

void decode_DDDDD_table(const UINT16 DDDDD, std::string& SD)
{
	switch(DDDDD)
	{
		case 0x00: SD = "X0";  break;
		case 0x01: SD = "Y0";  break;
		case 0x02: SD = "X1";  break;
		case 0x03: SD = "Y1";  break;
		case 0x04: SD = "A";   break;
		case 0x05: SD = "B";   break;
		case 0x06: SD = "A0";  break;
		case 0x07: SD = "B0";  break;
		case 0x08: SD = "LC";  break;
		case 0x09: SD = "SR";  break;
		case 0x0a: SD = "OMR"; break;
		case 0x0b: SD = "SP";  break;
		case 0x0c: SD = "A1";  break;
		case 0x0d: SD = "B1";  break;
		case 0x0e: SD = "A2";  break;
		case 0x0f: SD = "B2";  break;

		case 0x10: SD = "R0";  break;
		case 0x11: SD = "R1";  break;
		case 0x12: SD = "R2";  break;
		case 0x13: SD = "R3";  break;
		case 0x14: SD = "M0";  break;
		case 0x15: SD = "M1";  break;
		case 0x16: SD = "M2";  break;
		case 0x17: SD = "M3";  break;
		case 0x18: SD = "SSH"; break;
		case 0x19: SD = "SSL"; break;
		case 0x1a: SD = "LA";  break;
		case 0x1b: SD = "!!";  break; /* no 0x1b */
		case 0x1c: SD = "N0";  break;
		case 0x1d: SD = "N1";  break;
		case 0x1e: SD = "N2";  break;
		case 0x1f: SD = "N3";  break;
	}
}

void decode_DD_table(const UINT16 DD, std::string& SD)
{
	switch (DD)
	{
		case 0x0: SD = "X0"; break;
		case 0x1: SD = "Y0"; break;
		case 0x2: SD = "X1"; break;
		case 0x3: SD = "Y1"; break;
	}
}

void decode_DDF_table(const UINT16 DD, const UINT16 F, std::string& S, std::string& D)
{
	const UINT16 switchVal = (DD << 1) | F;

	switch (switchVal)
	{
		case 0x0: S = "X0"; D = "A"; break;
		case 0x1: S = "X0"; D = "B"; break;
		case 0x2: S = "Y0"; D = "A"; break;
		case 0x3: S = "Y0"; D = "B"; break;
		case 0x4: S = "X1"; D = "A"; break;
		case 0x5: S = "X1"; D = "B"; break;
		case 0x6: S = "Y1"; D = "A"; break;
		case 0x7: S = "Y1"; D = "B"; break;
	}
}

void decode_EE_table(const UINT16 EE, std::string& D)
{
	switch(EE)
	{
		case 0x1: D = "MR";  break;
		case 0x3: D = "CCR"; break;
		case 0x2: D = "OMR"; break;
	}
}

void decode_F_table(const UINT16 F, std::string& SD)
{
	switch(F)
	{
		case 0x0: SD = "A"; break;
		case 0x1: SD = "B"; break;
	}
}

void decode_h0hF_table(const UINT16 h0h, UINT16 F, std::string& S, std::string& D)
{
	const UINT16 switchVal = (h0h << 1) | F;

	switch (switchVal)
	{
		case 0x8: S = "X0"; D = "A"; break;
		case 0x9: S = "X0"; D = "B"; break;
		case 0xa: S = "Y0"; D = "A"; break;
		case 0xb: S = "Y0"; D = "B"; break;
		case 0x2: S = "A";  D = "A"; break;
		case 0x1: S = "A";  D = "B"; break;
		case 0x0: S = "B";  D = "A"; break;
		case 0x3: S = "B";  D = "B"; break;
	}
}

void decode_HH_table(const UINT16 HH, std::string& SD)
{
	switch(HH)
	{
		case 0x0: SD = "X0"; break;
		case 0x1: SD = "Y0"; break;
		case 0x2: SD = "A";  break;
		case 0x3: SD = "B";  break;
	}
}

void decode_HHH_table(const UINT16 HHH, std::string& SD)
{
	switch(HHH)
	{
		case 0x0: SD = "X0"; break;
		case 0x1: SD = "Y0"; break;
		case 0x2: SD = "X1"; break;
		case 0x3: SD = "Y1"; break;
		case 0x4: SD = "A";  break;
		case 0x5: SD = "B";  break;
		case 0x6: SD = "A0"; break;
		case 0x7: SD = "B0"; break;
	}
}

void decode_IIIIx_table(const UINT16 IIII, const UINT16 x, std::string& S, std::string& D)
{
	S = D = "!";
	switch(IIII)
	{
		case 0x0: S = "X0"; D = "^F"; break;
		case 0x1: S = "Y0"; D = "^F"; break;
		case 0x2: S = "X1"; D = "^F"; break;
		case 0x3: S = "Y1"; D = "^F"; break;
		case 0x4: S = "A";  D = "X0"; break;
		case 0x5: S = "B";  D = "Y0"; break;
		case 0x6: S = "A0"; D = "X0"; break;
		case 0x7: S = "B0"; D = "Y0"; break;
		case 0x8: if ( x) S = "F";  D = "^F"; break;
		case 0x9: if (!x) S = "F";  D = "^F"; break;
		case 0xa: S = "?";  D = "?";  break;
		case 0xb: S = "?";  D = "?";  break;
		case 0xc: S = "A";  D = "X1"; break;
		case 0xd: S = "B";  D = "Y1"; break;
		case 0xe: S = "A0"; D = "X1"; break;
		case 0xf: S = "B0"; D = "Y1"; break;
	}
}

void decode_JJJF_table(const UINT16 JJJ, const UINT16 F, std::string& S, std::string& D)
{
	const UINT16 switchVal = (JJJ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S = "B";  D = "A"; break;
		case 0x1: S = "A";  D = "B"; break;
		case 0x2: S = "!";  D = "!"; break;
		case 0x3: S = "!";  D = "!"; break;
		case 0x4: S = "X";  D = "A"; break;
		case 0x5: S = "X";  D = "B"; break;
		case 0x6: S = "Y";  D = "A"; break;
		case 0x7: S = "Y";  D = "B"; break;
		case 0x8: S = "X0"; D = "A"; break;
		case 0x9: S = "X0"; D = "B"; break;
		case 0xa: S = "Y0"; D = "A"; break;
		case 0xb: S = "Y0"; D = "B"; break;
		case 0xc: S = "X1"; D = "A"; break;
		case 0xd: S = "X1"; D = "B"; break;
		case 0xe: S = "Y1"; D = "A"; break;
		case 0xf: S = "Y1"; D = "B"; break;
	}
}

void decode_JJF_table(const UINT16 JJ, const UINT16 F, std::string& S, std::string& D)
{
	const UINT16 switchVal = (JJ << 1) | F;

	switch (switchVal)
	{
		case 0x0: S = "X0"; D = "A"; break;
		case 0x1: S = "X0"; D = "B"; break;
		case 0x2: S = "Y0"; D = "A"; break;
		case 0x3: S = "Y0"; D = "B"; break;
		case 0x4: S = "X1"; D = "A"; break;
		case 0x5: S = "X1"; D = "B"; break;
		case 0x6: S = "Y1"; D = "A"; break;
		case 0x7: S = "Y1"; D = "B"; break;
	}
}

void decode_JF_table(const UINT16 J, const UINT16 F, std::string& S, std::string& D)
{
	const UINT16 switchVal = (J << 1) | F;

	switch(switchVal)
	{
		case 0x0: S = "X"; D = "A"; break;
		case 0x1: S = "X"; D = "B"; break;
		case 0x2: S = "Y"; D = "A"; break;
		case 0x3: S = "Y"; D = "B"; break;
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

void decode_KKK_table(const UINT16 KKK, std::string& D1, std::string& D2)
{
	switch(KKK)
	{
		case 0x0: D1 = "^F"; D2 = "X0"; break;
		case 0x1: D1 = "Y0"; D2 = "X0"; break;
		case 0x2: D1 = "X1"; D2 = "X0"; break;
		case 0x3: D1 = "Y1"; D2 = "X0"; break;
		case 0x4: D1 = "X0"; D2 = "X1"; break;
		case 0x5: D1 = "Y0"; D2 = "X1"; break;
		case 0x6: D1 = "^F"; D2 = "Y0"; break;
		case 0x7: D1 = "Y1"; D2 = "X1"; break;
	}
}

void decode_NN_table(UINT16 NN, INT8& ret)
{
	ret = NN;
}

void decode_TT_table(UINT16 TT, INT8& ret)
{
	ret = TT;
}

void decode_QQF_table(const UINT16 QQ, const UINT16 F, std::string& S1, std::string& S2, std::string& D)
{
	const UINT16 switchVal = (QQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S1 = "Y0"; S2 = "X0"; D = "A"; break;
		case 0x1: S1 = "Y0"; S2 = "X0"; D = "B"; break;
		case 0x2: S1 = "Y1"; S2 = "X0"; D = "A"; break;
		case 0x3: S1 = "Y1"; S2 = "X0"; D = "B"; break;
		case 0x4: S1 = "Y0"; S2 = "X1"; D = "A"; break;
		case 0x5: S1 = "Y0"; S2 = "X1"; D = "B"; break;
		case 0x6: S1 = "Y1"; S2 = "X1"; D = "A"; break;
		case 0x7: S1 = "Y1"; S2 = "X1"; D = "B"; break;
	}
}

void decode_QQF_special_table(const UINT16 QQ, const UINT16 F, std::string& S1, std::string& S2, std::string& D)
{
	const UINT16 switchVal = (QQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S1 = "Y0"; S2 = "X0"; D = "A"; break;
		case 0x1: S1 = "Y0"; S2 = "X0"; D = "B"; break;
		case 0x2: S1 = "Y1"; S2 = "X0"; D = "A"; break;
		case 0x3: S1 = "Y1"; S2 = "X0"; D = "B"; break;
		case 0x4: S1 = "X1"; S2 = "Y0"; D = "A"; break;
		case 0x5: S1 = "X1"; S2 = "Y0"; D = "B"; break;
		case 0x6: S1 = "X1"; S2 = "Y1"; D = "A"; break;
		case 0x7: S1 = "X1"; S2 = "Y1"; D = "B"; break;
	}
}

void decode_QQQF_table(const UINT16 QQQ, const UINT16 F, std::string& S1, std::string& S2, std::string& D)
{
	const UINT16 switchVal = (QQQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: S1 = "X0"; S2 = "X0"; D = "A"; break;
		case 0x1: S1 = "X0"; S2 = "X0"; D = "B"; break;
		case 0x2: S1 = "X1"; S2 = "X0"; D = "A"; break;
		case 0x3: S1 = "X1"; S2 = "X0"; D = "B"; break;
		case 0x4: S1 = "A1"; S2 = "Y0"; D = "A"; break;
		case 0x5: S1 = "A1"; S2 = "Y0"; D = "B"; break;
		case 0x6: S1 = "B1"; S2 = "X0"; D = "A"; break;
		case 0x7: S1 = "B1"; S2 = "X0"; D = "B"; break;
		case 0x8: S1 = "Y0"; S2 = "X0"; D = "A"; break;
		case 0x9: S1 = "Y0"; S2 = "X0"; D = "B"; break;
		case 0xa: S1 = "Y1"; S2 = "X0"; D = "A"; break;
		case 0xb: S1 = "Y1"; S2 = "X0"; D = "B"; break;
		case 0xc: S1 = "Y0"; S2 = "X1"; D = "A"; break;
		case 0xd: S1 = "Y0"; S2 = "X1"; D = "B"; break;
		case 0xe: S1 = "Y1"; S2 = "X1"; D = "A"; break;
		case 0xf: S1 = "Y1"; S2 = "X1"; D = "B"; break;
	}
}

void decode_RR_table(UINT16 RR, INT8& ret)
{
	ret = RR;
}

void decode_rr_table(UINT16 rr, INT8& ret)
{
	ret = rr;
}

void decode_s_table(const UINT16 s, std::string& arithmetic)
{
	switch(s)
	{
		case 0x0: arithmetic = "su"; break;
		case 0x1: arithmetic = "uu"; break;
	}
}

void decode_ss_table(const UINT16 ss, std::string& arithmetic)
{
	switch(ss)
	{
		case 0x0: arithmetic = "ss"; break;
		case 0x1: arithmetic = "!!"; break;
	  //case 0x1: arithmetic = "ss"; break;
		case 0x2: arithmetic = "su"; break;
		case 0x3: arithmetic = "uu"; break;
	}
}

void decode_uuuuF_table(const UINT16 uuuu, const UINT16 F, std::string& arg, std::string& S, std::string& D)
{
	const UINT16 switchVal = (uuuu << 1) | F;

	D = "sub?";
	S = "add";
	arg = "invalid";

	switch(switchVal)
	{
		case 0x00: arg = "add"; S = "X0"; D = "A"; break;
		case 0x01: arg = "add"; S = "X0"; D = "B"; break;
		case 0x02: arg = "add"; S = "Y0"; D = "A"; break;
		case 0x03: arg = "add"; S = "Y0"; D = "B"; break;
		case 0x04: arg = "add"; S = "X1"; D = "A"; break;
		case 0x05: arg = "add"; S = "X1"; D = "B"; break;
		case 0x06: arg = "add"; S = "Y1"; D = "A"; break;
		case 0x07: arg = "add"; S = "Y1"; D = "B"; break;

		case 0x08: arg = "sub"; S = "X0"; D = "A"; break;
		case 0x09: arg = "sub"; S = "X0"; D = "B"; break;
		case 0x0a: arg = "sub"; S = "Y0"; D = "A"; break;
		case 0x0b: arg = "sub"; S = "Y0"; D = "B"; break;
		case 0x0c: arg = "sub"; S = "X1"; D = "A"; break;
		case 0x0d: arg = "sub"; S = "X1"; D = "B"; break;
		case 0x0e: arg = "sub"; S = "Y1"; D = "A"; break;
		case 0x0f: arg = "sub"; S = "Y1"; D = "B"; break;

		case 0x18: arg = "add"; S = "B";  D = "A"; break;
		case 0x19: arg = "add"; S = "A";  D = "B"; break;

		case 0x1a: arg = "sub"; S = "B";  D = "A"; break;
		case 0x1b: arg = "sub"; S = "A";  D = "B"; break;

		case 0x1c: arg = "tfr"; S = "B";  D = "A"; break;
		case 0x1d: arg = "tfr"; S = "A";  D = "B"; break;

		case 0x1e: arg = "move"; S = "";  D = ""; break;
		case 0x1f: arg = "move"; S = "";  D = ""; break;
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
		case 0x0: sprintf(temp, "(R%d)+",n)	   ; break;
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
		case 0x0: sprintf(temp1, "(R%d)+",	n1)	;
				  sprintf(temp2, "(R%d)+",	n2)	; break;
		case 0x1: sprintf(temp1, "(R%d)+",	n1)	;
				  sprintf(temp2, "(R%d)+N%d", n2, n2); break;
		case 0x2: sprintf(temp1, "(R%d)+N%d", n1, n1);
				  sprintf(temp2, "(R%d)+",	n2)	; break;
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
		case 0x0: sprintf(temp, "(R%d)",	 n)   ; break;
		case 0x1: sprintf(temp, "(R%d)+",	n)   ; break;
		case 0x2: sprintf(temp, "(R%d)-",	n)   ; break;
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
		case 0x1: sprintf(temp, "-(R%d)",	n)   ; break;
	}
	ea = temp;
}

void assemble_ea_from_t_table(UINT16 t,  UINT16 val, std::string& ea)
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
		case 0x0: sprintf(temp, "(R%d)-",	n)   ; break;
		case 0x1: sprintf(temp, "(R%d)+N%d", n, n); break;
	}
	ea = temp;
}

void assemble_D_from_P_table(UINT16 P, UINT16 ppppp, std::string& D)
{
	char temp[32];
	std::string fullAddy;	 /* Convert Short Absolute Address to full 16-bit */

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

void assemble_reg_from_W_table(UINT16 W, char ma, const std::string& SD, const INT8 xx, std::string& S, std::string& D)
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
		case 0x0: S = SD; D = temp; break;
		case 0x1: S = temp; D = SD; break;
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


/*******************/
/* HELPER FUNCTION */
/*******************/

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

bool registerOverlap(const std::string& r0, const size_t bmd, const std::string& r1)
{
	if (bmd == BM_NONE)
		return false;

	if (r0 == r1)
		return true;

	if (r0 == "A" && (bmd & BM_LOW)	&& r1 == "A0") return true;
	if (r0 == "A" && (bmd & BM_MIDDLE) && r1 == "A1") return true;
	if (r0 == "A" && (bmd & BM_HIGH)   && r1 == "A2") return true;

	if (r0 == "B" && (bmd & BM_LOW)	&& r1 == "B0") return true;
	if (r0 == "B" && (bmd & BM_MIDDLE) && r1 == "B1") return true;
	if (r0 == "B" && (bmd & BM_HIGH)   && r1 == "B2") return true;

	return false;
}

}

