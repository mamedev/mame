/***************************************************************************

    410ops.c

    National Semiconductor COP410 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#define INSTRUCTION(mnemonic) static inline void (mnemonic)(UINT8 opcode)

#define ROM(addr)			cpu_readop(addr)
#define RAM_W(addr, value)	(data_write_byte_8(addr, value))
#define RAM_R(addr)			(data_read_byte_8(addr))

#define IN(addr)			io_read_byte_8(addr)
#define OUT(addr, value)	io_write_byte_8(addr, value)

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
#define IN_SI()			IN(COP400_PORT_SIO)
#define OUT_G(A)		OUT(COP400_PORT_G, (A) & R.G_mask)
#define OUT_L(A)		OUT(COP400_PORT_L, (A))
#define OUT_D(A)		OUT(COP400_PORT_D, (A) & R.D_mask)
#define OUT_SK(A)		OUT(COP400_PORT_SK, (A))
#define OUT_SO(A)		OUT(COP400_PORT_SIO, (A))

#ifndef PUSH
#define PUSH(addr) 		{ SB = SA; SA = addr; }
#define POP() 			{ PC = SA; SA = SB; }
#endif

/* Serial I/O */

static TIMER_CALLBACK(cop410_serial_tick)
{
    int cpunum = param;
	
	cpuintrf_push_context(cpunum);

	if (BIT(EN, 0))
	{
		/* SIO is a binary counter */

		// serial input

		int si = IN_SI();

		if (R.last_si && !si)
		{
			SIO--;

			if (SIO == 0)
			{
				SIO = 15;
			}
		}

		R.last_si = si;

		// serial output

		OUT_SO(BIT(EN, 3));

		// serial clock

		OUT_SK(SKL);
	}
	else
	{
		/* SIO is a	shift register */

		// serial output

		if (BIT(EN, 3))
		{
			OUT_SO(BIT(SIO, 3));
		}
		else
		{
			OUT_SO(0);
		}

		SIO <<= 1;

		// serial input

		SIO = (SIO & 0x0e) | BIT(IN_SI(), 0);

		// serial clock

		if (SKL)
		{
			OUT_SK(1);
		}
		else
		{
			OUT_SK(0);
		}
	}

	cpuintrf_pop_context();
}

INLINE void WRITE_Q(UINT8 data)
{
	Q = data;

	if (BIT(EN, 2))
	{
		OUT_L(Q);
	}
}

INLINE void WRITE_G(UINT8 data)
{
	G = data;
	OUT_G(G);
}

INSTRUCTION(illegal)
{
	logerror("COP400: PC = %04x, Illegal opcode = %02x\n", PC-1, ROM(PC-1));
}

/* Arithmetic Instructions */

/*

	Mnemonic:			ASC
	
	Hex Code:			30
	Binary:				0 0 1 1 0 0 0 0

	Data Flow:			A + C + RAM(B) -> A
						Carry -> C

	Skip Conditions:	Carry

	Description:		Add with Carry, Skip on Carry

*/

INSTRUCTION(asc)
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

/*

	Mnemonic:			ADD
	
	Hex Code:			31
	Binary:				0 0 1 1 0 0 0 1

	Data Flow:			A + RAM(B) -> A

	Skip Conditions:	None

	Description:		Add RAM to A

*/

INSTRUCTION(add)
{
	A = (A + RAM_R(B)) & 0x0F; 
}

/*

	Mnemonic:			AISC
	
	Operand:			y
	Hex Code:			5-
	Binary:				0 1 0 1 y3 y2 y1 y0

	Data Flow:			A + y -> A

	Skip Conditions:	Carry

	Description:		Add Immediate, Skip on Carry (y != 0)

*/

INSTRUCTION(aisc)
{
	UINT4 y = opcode & 0x0f;

	A = A + y;

	if (A > 0x0f)
	{
		skip = 1;
		A &= 0xF;
	}
}

/*

	Mnemonic:			CLRA
	
	Hex Code:			00
	Binary:				0 0 0 0 0 0 0 0

	Data Flow:			0 -> A

	Skip Conditions:	None

	Description:		Clear A

*/

INSTRUCTION(clra)
{
	A = 0;
}

/*

	Mnemonic:			COMP
	
	Hex Code:			40
	Binary:				0 1 0 0 0 0 0 0

	Data Flow:			~A -> A

	Skip Conditions:	None

	Description:		Ones Complement of A to A

*/

INSTRUCTION(comp)
{
	A = A ^ 0xF;
}

/*

	Mnemonic:			NOP
	
	Hex Code:			44
	Binary:				0 1 0 0 0 1 0 0

	Data Flow:			None

	Skip Conditions:	None

	Description:		No Operation

*/

INSTRUCTION(nop)
{
	// do nothing
}

/*

	Mnemonic:			RC
	
	Hex Code:			32
	Binary:				0 0 1 1 0 0 1 0

	Data Flow:			"0" -> C

	Skip Conditions:	None

	Description:		Reset C

*/

INSTRUCTION(rc)
{
	C = 0;
}

/*

	Mnemonic:			SC
	
	Hex Code:			22
	Binary:				0 0 1 0 0 0 1 0

	Data Flow:			"1" -> C

	Skip Conditions:	None

	Description:		Set C

*/

INSTRUCTION(sc)
{
	C = 1;
}

/*

	Mnemonic:			XOR
	
	Hex Code:			02
	Binary:				0 0 0 0 0 0 1 0

	Data Flow:			A ^ RAM(B) -> A

	Skip Conditions:	None

	Description:		

*/

INSTRUCTION(xor)
{
	A = RAM_R(B) ^ A;
}

/* Transfer-of-Control Instructions */

/*

	Mnemonic:			JID
	
	Hex Code:			FF
	Binary:				1 1 1 1 1 1 1 1

	Data Flow:			ROM(PC9:8,A,M) -> PC7:0

	Skip Conditions:	None

	Description:		Jump Indirect

*/

INSTRUCTION(jid)
{
	UINT16 addr = (PC & 0x300) | (A << 4) | READ_M;
	PC = (PC & 0x300) | ROM(addr);
}

/*

	Mnemonic:			JMP
	
	Operand:			a
	Hex Code:			6- --
	Binary:				0 1 1 0 0 0 a9 a8 a7 a6 a5 a4 a3 a2 a1 a0

	Data Flow:			a -> PC

	Skip Conditions:	None

	Description:		Jump

*/

INSTRUCTION(jmp)
{
	PC = ((opcode & 0x03) << 8) | ROM(PC);
}

/*

	Mnemonic:			JP
	
	Operand:			a
	Hex Code:			--
	Binary:				1 a6 a5 a4 a3 a2 a1 a0
						(pages 2,3 only)

						1 1 a5 a4 a3 a2 a1 a0
						(all other pages)

	Data Flow:			a -> PC6:0

						a -> PC5:0

	Skip Conditions:	None

	Description:		Jump within Page

*/

INSTRUCTION(jp)
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

/*

	Mnemonic:			JSR
	
	Operand:			a
	Hex Code:			6- --
	Binary:				0 1 1 0 1 0 a9 a8 a7 a6 a5 a4 a3 a2 a1 a0

	Data Flow:			PC + 1 -> SA -> SB -> SC
						a -> PC

	Skip Conditions:	None

	Description:		Jump to Subroutine

*/

INLINE void JSR(UINT8 a8)
{
	PUSH(PC + 1);
	PC = (a8 << 8) | ROM(PC);
}

INSTRUCTION(jsr0) { JSR(0); }
INSTRUCTION(jsr1) { JSR(1); }
INSTRUCTION(jsr2) { JSR(2); }
INSTRUCTION(jsr3) { JSR(3); }

/*

	Mnemonic:			RET
	
	Hex Code:			48
	Binary:				0 1 0 0 1 0 0 0

	Data Flow:			SC -> SB -> SA -> PC

	Description:		Return from Subroutine

*/

INSTRUCTION(ret)
{
	POP();
}

/*

	Mnemonic:			RETSK
	
	Hex Code:			49
	Binary:				0 1 0 0 1 0 0 1

	Data Flow:			SC -> SB -> SA -> PC

	Skip Conditions:	Always Skip on Return

	Description:		Return from Subroutine then Skip

*/

INSTRUCTION(retsk)
{
	POP();
	skip = 1;
}

/* Memory Reference Instructions */

/*

	Mnemonic:			CAMQ
	
	Hex Code:			33 3C
	Binary:				0 0 1 1 0 0 1 1	0 0 1 1 1 1 0 0

	Data Flow:			A -> Q7:4
						RAM(B) -> Q3:0

	Description:		Copy A, RAM to Q

*/

INSTRUCTION(camq)
{
	WRITE_Q((A << 4) | READ_M);
}

/*

	Mnemonic:			LD
	
	Operand:			r
	Hex Code:			-5
	Binary:				0 0 r1 r0 0 1 0 1

	Data Flow:			RAM(B) -> A
						Br ^ r -> Br

	Description:		Load RAM into A, Exclusive-OR Br with r

*/

INLINE void LD(UINT8 r)
{
	A = RAM_R(B);
	B = B ^ (r << 4);
}

INSTRUCTION(ld0) { LD(0); }
INSTRUCTION(ld1) { LD(1); }
INSTRUCTION(ld2) { LD(2); }
INSTRUCTION(ld3) { LD(3); }

/*

	Mnemonic:			LQID
	
	Hex Code:			BF
	Binary:				1 0 1 1 1 1 1 1

	Data Flow:			ROM(PC9:8,A,M) -> Q
						SB -> SC

	Description:		Load Q Indirect

*/

INSTRUCTION(lqid)
{
	PUSH(PC + 1);
	PC = (UINT16)((PC & 0x300) | (A << 4) | READ_M);
	WRITE_Q(ROM(PC));
	POP();
}

/*

	Mnemonic:			RMB
	
	Operand:			0
						1
						2
						3

	Hex Code:			4C
						45
						42
						43

	Binary:				0 1 0 0 1 1 0 0
						0 1 0 0 0 1 0 1
						0 1 0 0 0 0 1 0
						0 1 0 0 0 0 1 1

	Data Flow:			0 -> RAM(B)0
						0 -> RAM(B)1
						0 -> RAM(B)2
						0 -> RAM(B)3

	Description:		Reset RAM Bit

*/

INSTRUCTION(rmb0) { RAM_W(B, RAM_R(B) & 0xE); }
INSTRUCTION(rmb1) { RAM_W(B, RAM_R(B) & 0xD); }
INSTRUCTION(rmb2) { RAM_W(B, RAM_R(B) & 0xB); }
INSTRUCTION(rmb3) { RAM_W(B, RAM_R(B) & 0x7); }

/*

	Mnemonic:			SMB
	
	Operand:			0
						1
						2
						3

	Hex Code:			4D
						47
						46
						4B

	Binary:				0 1 0 0 1 1 0 1
						0 1 0 0 0 1 1 1
						0 1 0 0 0 1 1 0
						0 1 0 0 1 0 1 1

	Data Flow:			1 -> RAM(B)0
						1 -> RAM(B)1
						1 -> RAM(B)2
						1 -> RAM(B)3

	Description:		Set RAM Bit

*/

INSTRUCTION(smb0) { RAM_W(B, RAM_R(B) | 0x1); }
INSTRUCTION(smb1) { RAM_W(B, RAM_R(B) | 0x2); }
INSTRUCTION(smb2) { RAM_W(B, RAM_R(B) | 0x4); }
INSTRUCTION(smb3) { RAM_W(B, RAM_R(B) | 0x8); }

/*

	Mnemonic:			STII
	
	Operand:			y
	Hex Code:			7-
	Binary:				0 1 1 1 y3 y2 y1 y0

	Data Flow:			y -> RAM(B)
						Bd + 1 -> Bd

	Description:		Store Memory Immediate and Increment Bd

*/

INSTRUCTION(stii)
{
	UINT4 y = opcode & 0x0f;
	UINT16 Bd;

	RAM_W(B, y);
	Bd = (B & 0x0f) + 1;
	if (Bd > 15) Bd = 0;
	B = (B & 0x30) + Bd;
}

/*

	Mnemonic:			X
	
	Operand:			r
	Hex Code:			-6
	Binary:				0 0 r1 r0 0 1 1 0

	Data Flow:			RAM(B) <-> A
						Br ^ r -> Br

	Description:		Exchange RAM with A, Exclusive-OR Br with r

*/

INSTRUCTION(x)
{
	UINT4 r = (opcode >> 4) & 0x03;
	UINT8 t = RAM_R(B);

	RAM_W(B, A);

	A = t;
	B = B ^ (r << 4);
}

/*

	Mnemonic:			XAD
	
	Operand:			r,d
	Hex Code:			23 --
	Binary:				0 0 1 0 0 0 1 1 1 0 r1 r0 d3 d2 d1 d0

	Data Flow:			RAM(r,d) -> A

	Description:		Exchange A with RAM pointed to directly by r,d

*/

INSTRUCTION(xad)
{
	UINT8 addr = ROM(PC++) & 0x3f;
	UINT8 t = A;
	A = RAM_R(addr);
	RAM_W(addr, t);
}

/*

	Mnemonic:			XDS
	
	Operand:			r
	Hex Code:			-7
	Binary:				0 0 r1 r0 0 1 1 1

	Data Flow:			RAM(B) <-> A
						Bd - 1 -> Bd
						Br ^ r -> Br

	Skip Conditions:	Bd decrements past 0

	Description:		Exchange RAM with A and Decrement Bd, Exclusive-OR Br with r

*/

INSTRUCTION(xds)
{
	UINT8 t, Bd, Br;
	UINT4 r = (opcode >> 4) & 0x03;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Br = (UINT8)((B & 0x30) ^ (r << 4));
	Bd = (UINT8)((B & 0x0F) - 1);
	B = (UINT8)(Br | (Bd & 0x0F));

	if (Bd == 0xFF) skip = 1;
}

/*

	Mnemonic:			XIS
	
	Operand:			r
	Hex Code:			-4
	Binary:				0 0 r1 r0 0 1 0 0

	Data Flow:			RAM(B) <-> A
						Bd + 1 -> Bd
						Br ^ r -> Br

	Skip Conditions:	Bd increments past 15

	Description:		Exchange RAM with A and Increment Bd, Exclusive-OR Br with r

*/

INSTRUCTION(xis)
{
	UINT8 t, Bd, Br;
	UINT4 r = (opcode >> 4) & 0x03;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Br = (UINT8)((B & 0x30) ^ (r << 4));
	Bd = (UINT8)((B & 0x0F) + 1);
	B = (UINT8)(Br | (Bd & 0x0F));

	if (Bd == 0x10) skip = 1;
}

/* Register Reference Instructions */

/*

	Mnemonic:			CAB
	
	Hex Code:			50
	Binary:				0 1 0 1 0 0 0 0 0

	Data Flow:			A -> Bd

	Description:		Copy A to Bd

*/

INSTRUCTION(cab)
{
	B = (B & 0x30) | A;
}

/*

	Mnemonic:			CBA
	
	Hex Code:			4E
	Binary:				0 1 0 0 1 1 1 0

	Data Flow:			Bd -> A

	Description:		Copy Bd to A

*/

INSTRUCTION(cba)
{
	A = B & 0xF;
}

/*

	Mnemonic:			LBI
	
	Operand:			r,d
	Hex Code:			--
						33 --

	Binary:				0 0 r1 r0 d3 d2 d1 d0 (d-1)
						0 0 1 1 0 0 1 1 1 0 r1 r0 d3 d2 d1 d0

	Data Flow:			r,d -> B

	Skip Conditions:	Skip until not a LBI

	Description:		Load B Immediate with r,d

*/

INLINE void LBI(UINT8 r, UINT8 d)
{
	B = (r << 4) | d;
	skipLBI = 1;
}

INSTRUCTION(lbi0_0) { LBI(0,0); }
INSTRUCTION(lbi0_1) { LBI(0,1); }
INSTRUCTION(lbi0_2) { LBI(0,2); }
INSTRUCTION(lbi0_3) { LBI(0,3); }
INSTRUCTION(lbi0_4) { LBI(0,4); }
INSTRUCTION(lbi0_5) { LBI(0,5); }
INSTRUCTION(lbi0_6) { LBI(0,6); }
INSTRUCTION(lbi0_7) { LBI(0,7); }
INSTRUCTION(lbi0_8) { LBI(0,8); }
INSTRUCTION(lbi0_9) { LBI(0,9); }
INSTRUCTION(lbi0_10) { LBI(0,10); }
INSTRUCTION(lbi0_11) { LBI(0,11); }
INSTRUCTION(lbi0_12) { LBI(0,12); }
INSTRUCTION(lbi0_13) { LBI(0,13); }
INSTRUCTION(lbi0_14) { LBI(0,14); }
INSTRUCTION(lbi0_15) { LBI(0,15); }

INSTRUCTION(lbi1_0) { LBI(1,0); }
INSTRUCTION(lbi1_1) { LBI(1,1); }
INSTRUCTION(lbi1_2) { LBI(1,2); }
INSTRUCTION(lbi1_3) { LBI(1,3); }
INSTRUCTION(lbi1_4) { LBI(1,4); }
INSTRUCTION(lbi1_5) { LBI(1,5); }
INSTRUCTION(lbi1_6) { LBI(1,6); }
INSTRUCTION(lbi1_7) { LBI(1,7); }
INSTRUCTION(lbi1_8) { LBI(1,8); }
INSTRUCTION(lbi1_9) { LBI(1,9); }
INSTRUCTION(lbi1_10) { LBI(1,10); }
INSTRUCTION(lbi1_11) { LBI(1,11); }
INSTRUCTION(lbi1_12) { LBI(1,12); }
INSTRUCTION(lbi1_13) { LBI(1,13); }
INSTRUCTION(lbi1_14) { LBI(1,14); }
INSTRUCTION(lbi1_15) { LBI(1,15); }

INSTRUCTION(lbi2_0) { LBI(2,0); }
INSTRUCTION(lbi2_1) { LBI(2,1); }
INSTRUCTION(lbi2_2) { LBI(2,2); }
INSTRUCTION(lbi2_3) { LBI(2,3); }
INSTRUCTION(lbi2_4) { LBI(2,4); }
INSTRUCTION(lbi2_5) { LBI(2,5); }
INSTRUCTION(lbi2_6) { LBI(2,6); }
INSTRUCTION(lbi2_7) { LBI(2,7); }
INSTRUCTION(lbi2_8) { LBI(2,8); }
INSTRUCTION(lbi2_9) { LBI(2,9); }
INSTRUCTION(lbi2_10) { LBI(2,10); }
INSTRUCTION(lbi2_11) { LBI(2,11); }
INSTRUCTION(lbi2_12) { LBI(2,12); }
INSTRUCTION(lbi2_13) { LBI(2,13); }
INSTRUCTION(lbi2_14) { LBI(2,14); }
INSTRUCTION(lbi2_15) { LBI(2,15); }

INSTRUCTION(lbi3_0) { LBI(3,0); }
INSTRUCTION(lbi3_1) { LBI(3,1); }
INSTRUCTION(lbi3_2) { LBI(3,2); }
INSTRUCTION(lbi3_3) { LBI(3,3); }
INSTRUCTION(lbi3_4) { LBI(3,4); }
INSTRUCTION(lbi3_5) { LBI(3,5); }
INSTRUCTION(lbi3_6) { LBI(3,6); }
INSTRUCTION(lbi3_7) { LBI(3,7); }
INSTRUCTION(lbi3_8) { LBI(3,8); }
INSTRUCTION(lbi3_9) { LBI(3,9); }
INSTRUCTION(lbi3_10) { LBI(3,10); }
INSTRUCTION(lbi3_11) { LBI(3,11); }
INSTRUCTION(lbi3_12) { LBI(3,12); }
INSTRUCTION(lbi3_13) { LBI(3,13); }
INSTRUCTION(lbi3_14) { LBI(3,14); }
INSTRUCTION(lbi3_15) { LBI(3,15); }

/*

	Mnemonic:			LEI
	
	Operand:			y
	Hex Code:			33 6-
	Binary:				0 0 1 1 0 0 1 1 0 1 1 0 y3 y2 y1 y0

	Data Flow:			y -> EN

	Description:		Load EN Immediate

*/

INSTRUCTION(lei)
{
	UINT4 y = opcode & 0x0f;

	EN = y;

	if (!BIT(EN, 2))
	{
		OUT_L(0);
	}
}

/* Test Instructions */

/*

	Mnemonic:			SKC
	
	Hex Code:			20
	Binary:				0 0 1 0 0 0 0 0

	Skip Conditions:	C = "1"

	Description:		Skip if C is True

*/

INSTRUCTION(skc)
{
	if (C == 1) skip = 1; 
}

/*

	Mnemonic:			SKE
	
	Hex Code:			21
	Binary:				0 0 1 0 0 0 0 1

	Skip Conditions:	A = RAM(B)

	Description:		Skip if A Equals RAM

*/

INSTRUCTION(ske)
{
	if (A == RAM_R(B)) skip = 1;
}

/*

	Mnemonic:			SKGZ
	
	Hex Code:			33 21
	Binary:				00 0 1 1 0 0 1 1 0 0 1 0 0 0 0 1

	Skip Conditions:	G3:0 = 0

	Description:		Skip if G is Zero (all 4 bits)

*/

INSTRUCTION(skgz)
{
	if (IN_G() == 0) skip = 1;
}

/*

	Mnemonic:			SKGBZ
	
	Hex Code:			33 01
						33 11
						33 03
						33 13
	
	Binary:				

	Skip Conditions:	G0 = 0
						G1 = 0
						G2 = 0
						G3 = 0

	Description:		Skip if G Bit is Zero

*/

INSTRUCTION(skgbz0) { if (!BIT(IN_G(), 0)) skip = 1; }
INSTRUCTION(skgbz1) { if (!BIT(IN_G(), 1)) skip = 1; }
INSTRUCTION(skgbz2) { if (!BIT(IN_G(), 2)) skip = 1; }
INSTRUCTION(skgbz3) { if (!BIT(IN_G(), 3)) skip = 1; }

/*

	Mnemonic:			SKMBZ
	
	Hex Code:			01
						11
						03
						13

	Binary:				

	Skip Conditions:	RAM(B)0 = 0
						RAM(B)0 = 1
						RAM(B)0 = 2
						RAM(B)0 = 3

	Description:		Skip if RAM Bit is Zero

*/

INSTRUCTION(skmbz0) { if (!BIT(RAM_R(B), 0)) skip = 1; }
INSTRUCTION(skmbz1) { if (!BIT(RAM_R(B), 1)) skip = 1; }
INSTRUCTION(skmbz2) { if (!BIT(RAM_R(B), 2)) skip = 1; }
INSTRUCTION(skmbz3) { if (!BIT(RAM_R(B), 3)) skip = 1; }

/* Input/Output Instructions */

/*

	Mnemonic:			ING
	
	Hex Code:			33 2A
	Binary:				

	Data Flow:			G -> A

	Description:		Input G Ports to A

*/

INSTRUCTION(ing)
{
	A = IN_G();
}

/*

	Mnemonic:			INL
	
	Hex Code:			33 2E
	Binary:				

	Data Flow:			L7:4 -> RAM(B)
						L3:0 -> A

	Description:		Input L Ports to RAM,A

*/

INSTRUCTION(inl)
{
	UINT8 L = IN_L();
	RAM_W(B, L >> 4);
	A = L & 0xF;
}

/*

	Mnemonic:			OBD
	
	Hex Code:			33 3E
	Binary:				

	Data Flow:			Bd -> D

	Description:		Output Bd to D Outputs

*/

INSTRUCTION(obd)
{
	OUT_D(B & 0x0f);
}

/*

	Mnemonic:			OMG
	
	Hex Code:			33 3A
	Binary:				

	Data Flow:			RAM(B) -> G

	Description:		Output RAM to G Ports

*/

INSTRUCTION(omg)
{
	WRITE_G(RAM_R(B));
}

/*

	Mnemonic:			XAS
	
	Hex Code:			4F
	Binary:				0 1 0 0 1 1 1 1

	Data Flow:			A <-> SIO
						C -> SK

	Description:		Exchange A with SIO

*/

INSTRUCTION(xas)
{
	UINT8 t = SIO;
	SIO = A;
	A = t;

	SKL = C;
}
