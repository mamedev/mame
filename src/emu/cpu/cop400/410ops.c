/**************************************************************************
 *               National Semiconductor COP410 Emulator                   *
 *                                                                        *
 *                   Copyright (C) 2006 MAME Team                         *
 **************************************************************************/

#define ROM(A)			cpu_readop(A)
#define RAM_W(A,V)		(data_write_byte_8(A,V))
#define RAM_R(A)		(data_read_byte_8(A))

#define IN(A)			io_read_byte_8(A)
#define OUT(A,V)		io_write_byte_8(A,V)

#define A				R.A
#define B				R.B
#define C				R.C
#define G				R.G
#define Q				R.Q
#define EN				R.EN
#define SA				R.SA
#define SB				R.SB
#define SIO				R.SIO
#define SKL				R.SKL
#define PC				R.PC
#define prevPC			R.PREVPC
#define skip			R.skip
#define skipLBI			R.skipLBI

#define READ_M			RAM_R(B)
#define WRITE_M(VAL)	RAM_W(B,VAL)

#define IN_G()			IN(COP400_PORT_G)
#define IN_L()			IN(COP400_PORT_L)
#define OUT_G(A)		OUT(COP400_PORT_G, (A) & R.G_mask)
#define OUT_L(A)		OUT(COP400_PORT_L, (A))
#define OUT_D(A)		OUT(COP400_PORT_D, (A) & R.D_mask)
#define OUT_SK(A)		OUT(COP400_PORT_SK,A)

#ifndef PUSH
#define PUSH(addr) 		{ SB = SA; SA = addr; }
#define POP() 			{ PC = SA; SA = SB; }
#endif

INLINE void illegal(void)
{
	logerror("ICOP400:  PC = %04x,  Illegal opcode = %02x\n", PC-1, ROM(PC-1));
}

INLINE void WRITE_SK(UINT8 data)
{
	SKL = data;

	if (EN & 0x01)
	{
		OUT_SK(SKL);
	}
	else
	{
		// NOT IMPLEMENTED
		OUT_SK(SKL);
	}
}

INLINE void WRITE_Q(UINT8 data)
{
	Q = data;

	if (EN & 0x04)
	{
		OUT_L(Q);
	}
}

INLINE void WRITE_G(UINT8 data)
{
	G = data;
	OUT_G(G);
}

INLINE void add(void) { A = (A + RAM_R(B)) & 0x0F; }

INLINE void asc(void)
{
	A = A + C + RAM_R(B);

	if (A > 0xF)
	{
		C = 1;
		skip = 1;
		A &= 0xF;
	}
	else
	{
		C = 0;
	}
}

INLINE void AISC(int y)
{
	A = A + y;

	if (A > 0xF)
	{
		skip = 1;
		A &= 0xF;
	}
}

INLINE void aisc1(void) { AISC(0x1); }
INLINE void aisc2(void) { AISC(0x2); }
INLINE void aisc3(void) { AISC(0x3); }
INLINE void aisc4(void) { AISC(0x4); }
INLINE void aisc5(void) { AISC(0x5); }
INLINE void aisc6(void) { AISC(0x6); }
INLINE void aisc7(void) { AISC(0x7); }
INLINE void aisc8(void) { AISC(0x8); }
INLINE void aisc9(void) { AISC(0x9); }
INLINE void aisc10(void) { AISC(0xA); }
INLINE void aisc11(void) { AISC(0xB); }
INLINE void aisc12(void) { AISC(0xC); }
INLINE void aisc13(void) { AISC(0xD); }
INLINE void aisc14(void) { AISC(0xE); }
INLINE void aisc15(void) { AISC(0xF); }

INLINE void cab(void) { B = (B & 0x30) | A; }

INLINE void camq(void) { WRITE_Q((A << 4) | READ_M); }

INLINE void cba(void) { A = B & 0xF; }

INLINE void clra(void){ A = 0; }

INLINE void comp(void) { A = A ^ 0xF; }

INLINE void ing(void) { A = IN_G(); }

INLINE void inl(void)
{
	UINT8 L = IN_L();
	RAM_W(B, L >> 4);
	A = L & 0xF;
}

INLINE void jid(void)
{
	UINT16 addr = (PC & 0x300) | (A << 4) | READ_M;
	PC = (PC & 0x300) | ROM(addr);
}

INLINE void JMP(UINT8 a8)
{
	PC = (a8 << 8) | ROM(PC);
}

INLINE void jmp0(void) { JMP(0); }
INLINE void jmp1(void) { JMP(1); }
INLINE void jmp2(void) { JMP(2); }
INLINE void jmp3(void) { JMP(3); }

INLINE void jp(void)
{
	UINT8 op = ROM(prevPC);

	if (((PC & 0x3E0) >= 0x80) && ((PC & 0x3E0) < 0x100)) //JP pages 2,3
	{
		PC = (UINT16)((PC & 0x380) | (op & 0x7F));
	}
	else
	{
		if ((op & 0xC0) == 0xC0) //JP other pages
		{
			PC = (UINT16)((PC & 0x3C0) | (op & 0x3F));
		}
		else					//JSRP
		{
			PUSH((UINT16)(PC));
			PC = (UINT16)(0x80 | (op & 0x3F));
		}
	}
}

INLINE void JSR(UINT8 a8)
{
	PUSH(PC + 1);
	PC = (a8 << 8) | ROM(PC);
}

INLINE void jsr0(void) { JSR(0); }
INLINE void jsr1(void) { JSR(1); }
INLINE void jsr2(void) { JSR(2); }
INLINE void jsr3(void) { JSR(3); }

INLINE void LD(UINT8 r)
{
	A = RAM_R(B);
	B = B ^ (r << 4);
}

INLINE void ld0(void) { LD(0); }
INLINE void ld1(void) { LD(1); }
INLINE void ld2(void) { LD(2); }
INLINE void ld3(void) { LD(3); }

INLINE void LBI(UINT8 r, UINT8 d)
{
	B = (r << 4) | d;
	skipLBI = 1;
}

INLINE void lbi0_0(void) { LBI(0,0); }
INLINE void lbi0_1(void) { LBI(0,1); }
INLINE void lbi0_2(void) { LBI(0,2); }
INLINE void lbi0_3(void) { LBI(0,3); }
INLINE void lbi0_4(void) { LBI(0,4); }
INLINE void lbi0_5(void) { LBI(0,5); }
INLINE void lbi0_6(void) { LBI(0,6); }
INLINE void lbi0_7(void) { LBI(0,7); }
INLINE void lbi0_8(void) { LBI(0,8); }
INLINE void lbi0_9(void) { LBI(0,9); }
INLINE void lbi0_10(void) { LBI(0,10); }
INLINE void lbi0_11(void) { LBI(0,11); }
INLINE void lbi0_12(void) { LBI(0,12); }
INLINE void lbi0_13(void) { LBI(0,13); }
INLINE void lbi0_14(void) { LBI(0,14); }
INLINE void lbi0_15(void) { LBI(0,15); }

INLINE void lbi1_0(void) { LBI(1,0); }
INLINE void lbi1_1(void) { LBI(1,1); }
INLINE void lbi1_2(void) { LBI(1,2); }
INLINE void lbi1_3(void) { LBI(1,3); }
INLINE void lbi1_4(void) { LBI(1,4); }
INLINE void lbi1_5(void) { LBI(1,5); }
INLINE void lbi1_6(void) { LBI(1,6); }
INLINE void lbi1_7(void) { LBI(1,7); }
INLINE void lbi1_8(void) { LBI(1,8); }
INLINE void lbi1_9(void) { LBI(1,9); }
INLINE void lbi1_10(void) { LBI(1,10); }
INLINE void lbi1_11(void) { LBI(1,11); }
INLINE void lbi1_12(void) { LBI(1,12); }
INLINE void lbi1_13(void) { LBI(1,13); }
INLINE void lbi1_14(void) { LBI(1,14); }
INLINE void lbi1_15(void) { LBI(1,15); }

INLINE void lbi2_0(void) { LBI(2,0); }
INLINE void lbi2_1(void) { LBI(2,1); }
INLINE void lbi2_2(void) { LBI(2,2); }
INLINE void lbi2_3(void) { LBI(2,3); }
INLINE void lbi2_4(void) { LBI(2,4); }
INLINE void lbi2_5(void) { LBI(2,5); }
INLINE void lbi2_6(void) { LBI(2,6); }
INLINE void lbi2_7(void) { LBI(2,7); }
INLINE void lbi2_8(void) { LBI(2,8); }
INLINE void lbi2_9(void) { LBI(2,9); }
INLINE void lbi2_10(void) { LBI(2,10); }
INLINE void lbi2_11(void) { LBI(2,11); }
INLINE void lbi2_12(void) { LBI(2,12); }
INLINE void lbi2_13(void) { LBI(2,13); }
INLINE void lbi2_14(void) { LBI(2,14); }
INLINE void lbi2_15(void) { LBI(2,15); }

INLINE void lbi3_0(void) { LBI(3,0); }
INLINE void lbi3_1(void) { LBI(3,1); }
INLINE void lbi3_2(void) { LBI(3,2); }
INLINE void lbi3_3(void) { LBI(3,3); }
INLINE void lbi3_4(void) { LBI(3,4); }
INLINE void lbi3_5(void) { LBI(3,5); }
INLINE void lbi3_6(void) { LBI(3,6); }
INLINE void lbi3_7(void) { LBI(3,7); }
INLINE void lbi3_8(void) { LBI(3,8); }
INLINE void lbi3_9(void) { LBI(3,9); }
INLINE void lbi3_10(void) { LBI(3,10); }
INLINE void lbi3_11(void) { LBI(3,11); }
INLINE void lbi3_12(void) { LBI(3,12); }
INLINE void lbi3_13(void) { LBI(3,13); }
INLINE void lbi3_14(void) { LBI(3,14); }
INLINE void lbi3_15(void) { LBI(3,15); }

INLINE void LEI(UINT8 y)
{
	EN = y & 0x0f;

	WRITE_Q(Q);
}

INLINE void lei0(void) { LEI(0); }
INLINE void lei1(void) { LEI(1); }
INLINE void lei2(void) { LEI(2); }
INLINE void lei3(void) { LEI(3); }
INLINE void lei4(void) { LEI(4); }
INLINE void lei5(void) { LEI(5); }
INLINE void lei6(void) { LEI(6); }
INLINE void lei7(void) { LEI(7); }
INLINE void lei8(void) { LEI(8); }
INLINE void lei9(void) { LEI(9); }
INLINE void lei10(void) { LEI(10); }
INLINE void lei11(void) { LEI(11); }
INLINE void lei12(void) { LEI(12); }
INLINE void lei13(void) { LEI(13); }
INLINE void lei14(void) { LEI(14); }
INLINE void lei15(void) { LEI(15); }

INLINE void lqid(void)
{
	PUSH(PC + 1);
	PC = (UINT16)((PC & 0x300) | (A << 4) | READ_M);
	WRITE_Q(ROM(PC));
	POP();
}

INLINE void nop(void) { }

INLINE void obd(void) { OUT_D(B); }

INLINE void omg(void) { WRITE_G(RAM_R(B)); }

INLINE void rc(void) { C = 0; }

INLINE void ret(void) { POP(); }

INLINE void retsk(void) { POP(); skip = 1; }

INLINE void rmb0(void) { RAM_W(B, RAM_R(B) & 0xE); }
INLINE void rmb1(void) { RAM_W(B, RAM_R(B) & 0xD); }
INLINE void rmb2(void) { RAM_W(B, RAM_R(B) & 0xB); }
INLINE void rmb3(void) { RAM_W(B, RAM_R(B) & 0x7); }

INLINE void sc(void) { C = 1; }

INLINE void skc(void) { if (C == 1) skip = 1; }

INLINE void ske(void) { if (A == RAM_R(B)) skip = 1; }

INLINE void skmbz0(void) { if ((RAM_R(B) & 0x01) == 0 ) skip = 1; }
INLINE void skmbz1(void) { if ((RAM_R(B) & 0x02) == 0 ) skip = 1; }
INLINE void skmbz2(void) { if ((RAM_R(B) & 0x04) == 0 ) skip = 1; }
INLINE void skmbz3(void) { if ((RAM_R(B) & 0x08) == 0 ) skip = 1; }

INLINE void skgbz0(void) { if ((IN_G() & 0x01) == 0) skip = 1; }
INLINE void skgbz1(void) { if ((IN_G() & 0x02) == 0) skip = 1; }
INLINE void skgbz2(void) { if ((IN_G() & 0x04) == 0) skip = 1; }
INLINE void skgbz3(void) { if ((IN_G() & 0x08) == 0) skip = 1; }

INLINE void skgz(void) { if (IN_G() == 0) skip = 1; }

INLINE void smb0(void) { RAM_W(B, RAM_R(B) | 0x1); }
INLINE void smb1(void) { RAM_W(B, RAM_R(B) | 0x2); }
INLINE void smb2(void) { RAM_W(B, RAM_R(B) | 0x4); }
INLINE void smb3(void) { RAM_W(B, RAM_R(B) | 0x8); }

INLINE void STII(UINT8 y)
{
	UINT16 Bd;

	RAM_W(B, y);
	Bd = (B & 0x0f) + 1;
	if (Bd > 15) Bd = 0;
	B = (B & 0x30) + Bd;
}

INLINE void stii0(void) { STII(0x0); }
INLINE void stii1(void) { STII(0x1); }
INLINE void stii2(void) { STII(0x2); }
INLINE void stii3(void) { STII(0x3); }

INLINE void stii4(void) { STII(0x4); }
INLINE void stii5(void) { STII(0x5); }
INLINE void stii6(void) { STII(0x6); }
INLINE void stii7(void) { STII(0x7); }

INLINE void stii8(void) { STII(0x8); }
INLINE void stii9(void) { STII(0x9); }
INLINE void stii10(void) { STII(0xA); }
INLINE void stii11(void) { STII(0xB); }

INLINE void stii12(void) { STII(0xC); }
INLINE void stii13(void) { STII(0xD); }
INLINE void stii14(void) { STII(0xE); }
INLINE void stii15(void) { STII(0xF); }

INLINE void X(UINT8 r)
{
	UINT8 t = RAM_R(B);
	RAM_W(B, A);
	A = t;
	B = B ^ (r << 4);
}

INLINE void x0(void) { X(0); }
INLINE void x1(void) { X(1); }
INLINE void x2(void) { X(2); }
INLINE void x3(void) { X(3); }

INLINE void xad(void)
{
	UINT8 addr = ROM(PC++) & 0x3f;
	UINT8 t = A;
	A = RAM_R(addr);
	RAM_W(addr, t);
}

INLINE void xas(void)
{
	UINT8 t = SIO;
	SIO = A;
	A = t;

	WRITE_SK(C);
}

INLINE void XDS(UINT8 r)
{
	UINT8 t, Bd, Br;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Br = (UINT8)((B & 0x30) ^ (r << 4));
	Bd = (UINT8)((B & 0x0F) - 1);
	B = (UINT8)(Br | (Bd & 0x0F));

	if (Bd == 0xFF) skip = 1;
}

INLINE void XIS(UINT8 r)
{
	UINT8 t, Bd, Br;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Br = (UINT8)((B & 0x30) ^ (r << 4));
	Bd = (UINT8)((B & 0x0F) + 1);
	B = (UINT8)(Br | (Bd & 0x0F));

	if (Bd == 0x10) skip = 1;
}

INLINE void xis0(void) { XIS(0); }
INLINE void xis1(void) { XIS(1); }
INLINE void xis2(void) { XIS(2); }
INLINE void xis3(void) { XIS(3); }

INLINE void xds0(void) { XDS(0); }
INLINE void xds1(void) { XDS(1); }
INLINE void xds2(void) { XDS(2); }
INLINE void xds3(void) { XDS(3); }

INLINE void xor(void) { A = RAM_R(B) ^ A; }
