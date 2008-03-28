/***************************************************************************

    420ops.c

    National Semiconductor COP420 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#define SC				R.SC
#define IL				R.IL

#define PUSH(addr) 		{ SC = SB; SB = SA; SA = addr; }
#define POP() 			{ PC = SA; SA = SB; SB = SC; }

#include "410ops.c"

#define IN_IN()			IN(COP400_PORT_IN)

/* Arithmetic Instructions */

/*

	Mnemonic:			ADT
	
	Hex Code:			4A
	Binary:				0 1 0 0 1 0 1 0

	Data Flow:			A + 10 -> A

	Skip Conditions:	None

	Description:		Add Ten to A

*/

INSTRUCTION(adt)
{
	A = (A + 10) & 0x0F;
}

/*

	Mnemonic:			CASC
	
	Hex Code:			10
	Binary:				0 0 0 1 0 0 0 0

	Data Flow:			~A + RAM(B) + C -> A
						Carry -> C

	Skip Conditions:	Carry

	Description:		Complement and Add with Carry, Skip on Carry

*/

INSTRUCTION(casc)
{
	A = (A ^ 0xF) + RAM_R(B) + C;

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

/* Transfer-of-Control Instructions */

/*

	Mnemonic:			RET
	
	Hex Code:			48
	Binary:				0 1 0 0 1 0 0 0

	Data Flow:			SC -> SB -> SA -> PC

	Description:		Return from Subroutine, restore Skip logic

*/

INSTRUCTION(cop420_ret)
{
	POP();
	skip = R.last_skip;
}

/* Memory Reference Instructions */

/*

	Mnemonic:			CQMA
	
	Hex Code:			33 2C
	Binary:				0 0 1 1 0 0 1 1 0 0 1 0 1 1 0 0

	Data Flow:			Q7:4 -> RAM(B)
						Q3:0 -> A

	Skip Conditions:	None

	Description:		Copy Q to RAM, A

*/

INSTRUCTION(cqma)
{
	WRITE_M(Q >> 4);
	A = Q & 0xF;
}

/*

	Mnemonic:			LDD
	
	Operand:			r, d
	Hex Code:			23 --
	Binary:				0 0 1 0 0 0 1 1 0 0 r1 r0 d3 d2 d1 d0

	Data Flow:			RAM(r,d) -> A

	Description:		Load A with RAM pointed to directly by r,d

*/

INSTRUCTION(ldd)
{
	UINT8 rd = opcode & 0x3f;
	
	A = RAM_R(rd);
}

/* Register Reference Instructions */

/*

	Mnemonic:			XABR
	
	Hex Code:			12
	Binary:				0 0 0 1 0 0 1 0

	Data Flow:			A <-> Br(0,0 -> A3,A2)

	Description:		Exchange A with Br

*/

INSTRUCTION(xabr)
{
	UINT8 Br = A & 0x03;
	UINT8 Bd = B & 0x0f;

	A = (B & 0x30) >> 4;
	B = (Br << 4) + Bd;
}

/* Test Instructions */

/*

	Mnemonic:			SKT
	
	Hex Code:			41
	Binary:				0 1 0 0 0 0 0 1

	Skip Conditions:	A time-base counter carry has occurred since last test

	Description:		Skip on Timer

*/

INSTRUCTION(skt)
{
	if (R.timerlatch == 1)
	{
		R.timerlatch = 0;
		skip = 1;
	}
}

/* Input/Output Instructions */

/*

	Mnemonic:			ININ
	
	Hex Code:			33 28
	Binary:				

	Data Flow:			IN -> A

	Description:		Input IN Inputs to A

*/

INSTRUCTION(inin) { A = IN_IN(); }

/*

	Processor:			COP402M

	Mnemonic:			ININ
	
	Hex Code:			33 28
	Binary:				

	Data Flow:			IN -> A, A1 = "1"

	Description:		Input IN Inputs to A

*/

INSTRUCTION(cop402m_inin)
{
	A = IN_IN() | 0x02;
}


/*

	Mnemonic:			INIL
	
	Hex Code:			33 29
	Binary:				

	Data Flow:			IL3,"1","0",IL0 -> A

	Description:		Input IL Latches to A

*/

INSTRUCTION(inil)
{
	// NOT PROPERLY IMPLEMENTED
	
	A = (IN_IN() & 0x09) | 0x04;
}

/*

	Processor:			COP421

	Mnemonic:			INIL
	
	Hex Code:			33 29
	Binary:				

	Data Flow:			"0",CKO,"0","0" -> A

	Description:		Input CKO to A

*/

INSTRUCTION(cop421_inil)
{
	// NOT IMPLEMENTED
}

/*

	Mnemonic:			OGI
	
	Operand:			y
	Hex Code:			33 5-
	Binary:				0 0 1 1 0 0 1 1 0 1 0 1 y3 y2 y1 y0

	Data Flow:			y -> G

	Description:		Output to G Ports Immediate

*/

INSTRUCTION(ogi)
{
	UINT4 y = opcode & 0x0f;

	WRITE_G(y);
}
