// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop400op.inc

    National Semiconductor COP400 Emulator.

***************************************************************************/

/***************************************************************************
    ARITHMETIC INSTRUCTIONS
***************************************************************************/

/*

    Mnemonic:           ASC

    Hex Code:           30
    Binary:             0 0 1 1 0 0 0 0

    Data Flow:          A + C + RAM(B) -> A
                        Carry -> C

    Skip Conditions:    Carry

    Description:        Add with Carry, Skip on Carry

*/

INSTRUCTION( asc )
{
	A = A + C + RAM_R(B);

	if (A > 0xF)
	{
		C = 1;
		m_skip = 1;
		A &= 0xF;
	}
	else
	{
		C = 0;
	}
}

/*

    Mnemonic:           ADD

    Hex Code:           31
    Binary:             0 0 1 1 0 0 0 1

    Data Flow:          A + RAM(B) -> A

    Description:        Add RAM to A

*/

INSTRUCTION( add )
{
	A = (A + RAM_R(B)) & 0x0F;
}

/*

    Mnemonic:           AISC

    Operand:            y
    Hex Code:           5-
    Binary:             0 1 0 1 y3 y2 y1 y0

    Data Flow:          A + y -> A

    Skip Conditions:    Carry

    Description:        Add Immediate, Skip on Carry (y != 0)

*/

INSTRUCTION( aisc )
{
	UINT8 y = opcode & 0x0f;

	A = A + y;

	if (A > 0x0f)
	{
		m_skip = 1;
		A &= 0xF;
	}
}

/*

    Mnemonic:           CLRA

    Hex Code:           00
    Binary:             0 0 0 0 0 0 0 0

    Data Flow:          0 -> A

    Description:        Clear A

*/

INSTRUCTION( clra )
{
	A = 0;
}

/*

    Mnemonic:           COMP

    Hex Code:           40
    Binary:             0 1 0 0 0 0 0 0

    Data Flow:          ~A -> A

    Description:        Ones Complement of A to A

*/

INSTRUCTION( comp )
{
	A = A ^ 0xF;
}

/*

    Mnemonic:           NOP

    Hex Code:           44
    Binary:             0 1 0 0 0 1 0 0

    Description:        No Operation

*/

INSTRUCTION( nop )
{
	// do nothing
}

/*

    Mnemonic:           RC

    Hex Code:           32
    Binary:             0 0 1 1 0 0 1 0

    Data Flow:          "0" -> C

    Description:        Reset C

*/

INSTRUCTION( rc )
{
	C = 0;
}

/*

    Mnemonic:           SC

    Hex Code:           22
    Binary:             0 0 1 0 0 0 1 0

    Data Flow:          "1" -> C

    Description:        Set C

*/

INSTRUCTION( sc )
{
	C = 1;
}

/*

    Mnemonic:           XOR

    Hex Code:           02
    Binary:             0 0 0 0 0 0 1 0

    Data Flow:          A ^ RAM(B) -> A

    Description:        Exclusive-OR RAM with A

*/

INSTRUCTION( xor_ )
{
	A = A ^ RAM_R(B);
}

/*

    Mnemonic:           ADT

    Hex Code:           4A
    Binary:             0 1 0 0 1 0 1 0

    Data Flow:          A + 10 -> A

    Description:        Add Ten to A

*/

INSTRUCTION( adt )
{
	A = (A + 10) & 0x0F;
}

/*

    Mnemonic:           CASC

    Hex Code:           10
    Binary:             0 0 0 1 0 0 0 0

    Data Flow:          ~A + RAM(B) + C -> A
                        Carry -> C

    Skip Conditions:    Carry

    Description:        Complement and Add with Carry, Skip on Carry

*/

INSTRUCTION( casc )
{
	A = (A ^ 0xF) + RAM_R(B) + C;

	if (A > 0xF)
	{
		C = 1;
		m_skip = 1;
		A &= 0xF;
	}
	else
	{
		C = 0;
	}
}

/***************************************************************************
    TRANSFER-OF-CONTROL INSTRUCTIONS
***************************************************************************/

/*

    Mnemonic:           JID

    Hex Code:           FF
    Binary:             1 1 1 1 1 1 1 1

    Data Flow:          ROM(PC10:8,A,M) -> PC7:0

    Description:        Jump Indirect

*/

INSTRUCTION( jid )
{
	UINT16 addr = (PC & 0x700) | (A << 4) | RAM_R(B);
	PC = (PC & 0x700) | ROM(addr);
}

/*

    Mnemonic:           JMP

    Operand:            a
    Hex Code:           6- --
    Binary:             0 1 1 0 0 a10 a9 a8 a7 a6 a5 a4 a3 a2 a1 a0

    Data Flow:          a -> PC

    Description:        Jump

*/

INSTRUCTION( jmp )
{
	UINT16 a = ((opcode & 0x07) << 8) | ROM(PC);

	PC = a;
}

/*

    Mnemonic:           JP

    Operand:            a
    Hex Code:           --
    Binary:             1 a6 a5 a4 a3 a2 a1 a0
                        (pages 2,3 only)

                        1 1 a5 a4 a3 a2 a1 a0
                        (all other pages)

    Data Flow:          a -> PC6:0

                        a -> PC5:0

    Description:        Jump within Page

*/

INSTRUCTION( jp )
{
	UINT8 page = PC >> 6;

	if (page == 2 || page == 3)
	{
		UINT8 a = opcode & 0x7f;
		PC = (PC & 0x780) | a;
	}
	else if ((opcode & 0xc0) == 0xc0)
	{
		UINT8 a = opcode & 0x3f;
		PC = (PC & 0x7c0) | a;
	}
	else
	{
		// JSRP
		UINT8 a = opcode & 0x3f;
		PUSH(PC);
		PC = 0x80 | a;
	}
}

/*

    Mnemonic:           JSR

    Operand:            a
    Hex Code:           6- --
    Binary:             0 1 1 0 1 a10 a9 a8 a7 a6 a5 a4 a3 a2 a1 a0

    Data Flow:          PC + 1 -> SA -> SB -> SC
                        a -> PC

    Description:        Jump to Subroutine

*/

INSTRUCTION( jsr )
{
	UINT16 a = ((opcode & 0x07) << 8) | ROM(PC);

	PUSH(PC + 1);
	PC = a;
}

/*

    Mnemonic:           RET

    Hex Code:           48
    Binary:             0 1 0 0 1 0 0 0

    Data Flow:          SC -> SB -> SA -> PC

    Description:        Return from Subroutine

*/

INSTRUCTION( ret )
{
	POP();
}

/*

    Processor:          COP420

    Mnemonic:           RET

    Hex Code:           48
    Binary:             0 1 0 0 1 0 0 0

    Data Flow:          SC -> SB -> SA -> PC

    Description:        Return from Subroutine, restore Skip logic

*/

INSTRUCTION( cop420_ret )
{
	POP();
	m_skip = m_last_skip;
}

/*

    Mnemonic:           RETSK

    Hex Code:           49
    Binary:             0 1 0 0 1 0 0 1

    Data Flow:          SC -> SB -> SA -> PC

    Skip Conditions:    Always Skip on Return

    Description:        Return from Subroutine then Skip

*/

INSTRUCTION( retsk )
{
	POP();
	m_skip = 1;
}

/*

    Processor:          COP410C/COP411C

    Mnemonic:           HALT

    Hex Code:           33 38
    Binary:             0 0 1 1 0 0 1 1 0 0 1 1 1 0 0 0

    Description:        Halt processor

*/

INSTRUCTION( halt )
{
	m_halt = 1;
}

/*

    Mnemonic:           IT

    Hex Code:           33 39
    Binary:             0 0 1 1 0 0 1 1 0 0 1 1 1 0 0 1

    Description:        IDLE till Timer Overflows then Continues

*/

INSTRUCTION( it )
{
	m_halt = 1;
	m_idle = 1;
}

/***************************************************************************
    MEMORY REFERENCE INSTRUCTIONS
***************************************************************************/

/*

    Mnemonic:           CAMQ

    Hex Code:           33 3C
    Binary:             0 0 1 1 0 0 1 1 0 0 1 1 1 1 0 0

    Data Flow:          A -> Q7:4
                        RAM(B) -> Q3:0

    Description:        Copy A, RAM to Q

*/

INSTRUCTION( camq )
{
	/*

	    Excerpt from the COP410L data sheet:

	    False states may be generated on L0-L7 during the execution of the CAMQ instruction.
	    The L-ports should not be used as clocks for edge sensitive devices such as flip-flops,
	    counters, shift registers, etc. the following short program that illustrates this situation.

	    START:
	        CLRA        ;ENABLE THE Q
	        LEI 4       ;REGISTER TO L LINES
	        LBI TEST
	        STII 3
	        AISC 12
	    LOOP:
	        LBI TEST    ;LOAD Q WITH X'C3
	        CAMQ
	        JP LOOP

	    In this program the internal Q register is enabled onto the L lines and a steady bit
	    pattern of logic highs is output on L0, L1, L6, L7, and logic lows on L2-L5 via the
	    two-byte CAMQ instruction. Timing constraints on the device are such that the Q
	    register may be temporarily loaded with the second byte of the CAMQ opcode (3C) prior
	    to receiving the valid data pattern. If this occurs, the opcode will ripple onto the L
	    lines and cause negative-going glitches on L0, L1, L6, L7, and positive glitches on
	    L2-L5. Glitch durations are under 2 ms, although the exact value may vary due to data
	    patterns, processing parameters, and L line loading. These false states are peculiar
	    only to the CAMQ instruction and the L lines.

	*/

	UINT8 data = (A << 4) | RAM_R(B);

	WRITE_Q(data);

#ifdef CAMQ_BUG
	WRITE_Q(0x3c);
	WRITE_Q(data);
#endif
}

/*

    Mnemonic:           LD

    Operand:            r
    Hex Code:           -5
    Binary:             0 0 r1 r0 0 1 0 1

    Data Flow:          RAM(B) -> A
                        Br ^ r -> Br

    Description:        Load RAM into A, Exclusive-OR Br with r

*/

INSTRUCTION( ld )
{
	UINT8 r = opcode & 0x30;

	A = RAM_R(B);
	B = B ^ r;
}

/*

    Mnemonic:           LQID

    Hex Code:           BF
    Binary:             1 0 1 1 1 1 1 1

    Data Flow:          ROM(PC10:8,A,M) -> Q
                        SB -> SC

    Description:        Load Q Indirect

*/

INSTRUCTION( lqid )
{
	PUSH(PC);
	PC = (PC & 0x700) | (A << 4) | RAM_R(B);
	WRITE_Q(ROM(PC));
	POP();
}

/*

    Mnemonic:           RMB

    Operand:            0
                        1
                        2
                        3

    Hex Code:           4C
                        45
                        42
                        43

    Binary:             0 1 0 0 1 1 0 0
                        0 1 0 0 0 1 0 1
                        0 1 0 0 0 0 1 0
                        0 1 0 0 0 0 1 1

    Data Flow:          0 -> RAM(B)0
                        0 -> RAM(B)1
                        0 -> RAM(B)2
                        0 -> RAM(B)3

    Description:        Reset RAM Bit

*/

INSTRUCTION( rmb0 ) { RAM_W(B, RAM_R(B) & 0xE); }
INSTRUCTION( rmb1 ) { RAM_W(B, RAM_R(B) & 0xD); }
INSTRUCTION( rmb2 ) { RAM_W(B, RAM_R(B) & 0xB); }
INSTRUCTION( rmb3 ) { RAM_W(B, RAM_R(B) & 0x7); }

/*

    Mnemonic:           SMB

    Operand:            0
                        1
                        2
                        3

    Hex Code:           4D
                        47
                        46
                        4B

    Binary:             0 1 0 0 1 1 0 1
                        0 1 0 0 0 1 1 1
                        0 1 0 0 0 1 1 0
                        0 1 0 0 1 0 1 1

    Data Flow:          1 -> RAM(B)0
                        1 -> RAM(B)1
                        1 -> RAM(B)2
                        1 -> RAM(B)3

    Description:        Set RAM Bit

*/

INSTRUCTION( smb0 ) { RAM_W(B, RAM_R(B) | 0x1); }
INSTRUCTION( smb1 ) { RAM_W(B, RAM_R(B) | 0x2); }
INSTRUCTION( smb2 ) { RAM_W(B, RAM_R(B) | 0x4); }
INSTRUCTION( smb3 ) { RAM_W(B, RAM_R(B) | 0x8); }

/*

    Mnemonic:           STII

    Operand:            y
    Hex Code:           7-
    Binary:             0 1 1 1 y3 y2 y1 y0

    Data Flow:          y -> RAM(B)
                        Bd + 1 -> Bd

    Description:        Store Memory Immediate and Increment Bd

*/

INSTRUCTION( stii )
{
	UINT8 y = opcode & 0x0f;
	UINT16 Bd;

	RAM_W(B, y);

	Bd = ((B & 0x0f) + 1) & 0x0f;
	B = (B & 0x70) + Bd;
}

/*

    Mnemonic:           X

    Operand:            r
    Hex Code:           -6
    Binary:             0 0 r1 r0 0 1 1 0

    Data Flow:          RAM(B) <-> A
                        Br ^ r -> Br

    Description:        Exchange RAM with A, Exclusive-OR Br with r

*/

INSTRUCTION( x )
{
	UINT8 r = opcode & 0x30;
	UINT8 t = RAM_R(B);

	RAM_W(B, A);

	A = t;
	B = B ^ r;
}

/*

    Mnemonic:           XAD

    Operand:            r,d
    Hex Code:           23 --
    Binary:             0 0 1 0 0 0 1 1 1 r2 r1 r0 d3 d2 d1 d0

    Data Flow:          RAM(r,d) <-> A

    Description:        Exchange A with RAM pointed to directly by r,d

*/

INSTRUCTION( xad )
{
	UINT8 rd = opcode & 0x7f;
	UINT8 t = A;

	A = RAM_R(rd);

	RAM_W(rd, t);
}

/*

    Mnemonic:           XDS

    Operand:            r
    Hex Code:           -7
    Binary:             0 0 r1 r0 0 1 1 1

    Data Flow:          RAM(B) <-> A
                        Bd - 1 -> Bd
                        Br ^ r -> Br

    Skip Conditions:    Bd decrements past 0

    Description:        Exchange RAM with A and Decrement Bd, Exclusive-OR Br with r

*/

INSTRUCTION( xds )
{
	UINT8 t, Bd;
	UINT8 r = opcode & 0x30;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Bd = ((B & 0x0f) - 1) & 0x0f;
	B = (B & 0x70) | Bd;

	B = B ^ r;

	if (Bd == 0x0f) m_skip = 1;
}

/*

    Mnemonic:           XIS

    Operand:            r
    Hex Code:           -4
    Binary:             0 0 r1 r0 0 1 0 0

    Data Flow:          RAM(B) <-> A
                        Bd + 1 -> Bd
                        Br ^ r -> Br

    Skip Conditions:    Bd increments past 15

    Description:        Exchange RAM with A and Increment Bd, Exclusive-OR Br with r

*/

INSTRUCTION( xis )
{
	UINT8 t, Bd;
	UINT8 r = opcode & 0x30;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Bd = ((B & 0x0f) + 1) & 0x0f;
	B = (B & 0x70) | Bd;

	B = B ^ r;

	if (Bd == 0x00) m_skip = 1;
}

/*

    Mnemonic:           CQMA

    Hex Code:           33 2C
    Binary:             0 0 1 1 0 0 1 1 0 0 1 0 1 1 0 0

    Data Flow:          Q7:4 -> RAM(B)
                        Q3:0 -> A

    Description:        Copy Q to RAM, A

*/

INSTRUCTION( cqma )
{
	RAM_W(B, Q >> 4);
	A = Q & 0xF;
}

/*

    Mnemonic:           LDD

    Operand:            r, d
    Hex Code:           23 --
    Binary:             0 0 1 0 0 0 1 1 0 r2 r1 r0 d3 d2 d1 d0

    Data Flow:          RAM(r,d) -> A

    Description:        Load A with RAM pointed to directly by r,d

*/

INSTRUCTION( ldd )
{
	UINT8 rd = opcode & 0x7f;

	A = RAM_R(rd);
}

/*

    Mnemonic:           CAMT

    Hex Code:           33 3F
    Binary:

    Data Flow:          A -> T7:4
                        RAM(B) -> T3:0

    Description:        Copy A, RAM to T

*/

INSTRUCTION( camt )
{
	T = (A << 4) | RAM_R(B);
}
/*

    Mnemonic:           CTMA

    Hex Code:           33 2F
    Binary:

    Data Flow:          T7:4 -> RAM(B)
                        T3:0 -> A

    Description:        Copy T to RAM, A

*/

INSTRUCTION( ctma )
{
	RAM_W(B, T >> 4);
	A = T & 0x0f;
}

/***************************************************************************
    REGISTER REFERENCE INSTRUCTIONS
***************************************************************************/

/*

    Mnemonic:           CAB

    Hex Code:           50
    Binary:             0 1 0 1 0 0 0 0 0

    Data Flow:          A -> Bd

    Description:        Copy A to Bd

*/

INSTRUCTION( cab )
{
	B = (B & 0x70) | A;
}

/*

    Mnemonic:           CBA

    Hex Code:           4E
    Binary:             0 1 0 0 1 1 1 0

    Data Flow:          Bd -> A

    Description:        Copy Bd to A

*/

INSTRUCTION( cba )
{
	A = B & 0xF;
}

/*

    Mnemonic:           LBI

    Operand:            r,d
    Hex Code:           --
                        33 --

    Binary:             0 0 r1 r0 d3 d2 d1 d0 (d-1)
                        0 0 1 1 0 0 1 1 1 r2 r1 r0 d3 d2 d1 d0

    Data Flow:          r,d -> B

    Skip Conditions:    Skip until not a LBI

    Description:        Load B Immediate with r,d

*/

INSTRUCTION( lbi )
{
	m_skip_lbi++;
	if (m_skip_lbi > 1) return;
	m_skip_lbi++;

	if (opcode & 0x80)
	{
		B = opcode & 0x7f;
	}
	else
	{
		B = (opcode & 0x30) | (((opcode & 0x0f) + 1) & 0x0f);
	}
}

/*

    Mnemonic:           LEI

    Operand:            y
    Hex Code:           33 6-
    Binary:             0 0 1 1 0 0 1 1 0 1 1 0 y3 y2 y1 y0

    Data Flow:          y -> EN

    Description:        Load EN Immediate

*/

INSTRUCTION( lei )
{
	UINT8 y = opcode & 0x0f;

	EN = y;

	if (BIT(EN, 2))
	{
		OUT_L(Q);
	}
}

/*

    Mnemonic:           XABR

    Hex Code:           12
    Binary:             0 0 0 1 0 0 1 0

    Data Flow:          A <-> Br(0,0 -> A3,A2)

    Description:        Exchange A with Br

*/

INSTRUCTION( xabr )
{
	UINT8 Br = A & 0x03;
	UINT8 Bd = B & 0x0f;

	A = B >> 4;
	B = (Br << 4) + Bd;
}

/*

    Processor:          COP444

    Mnemonic:           XABR

    Hex Code:           12
    Binary:             0 0 0 1 0 0 1 0

    Data Flow:          A <-> Br(0 -> A3)

    Description:        Exchange A with Br

*/

INSTRUCTION( cop444_xabr )
{
	UINT8 Br = A & 0x07;
	UINT8 Bd = B & 0x0f;

	A = B >> 4;
	B = (Br << 4) + Bd;
}

/***************************************************************************
    TEST INSTRUCTIONS
***************************************************************************/

/*

    Mnemonic:           SKC

    Hex Code:           20
    Binary:             0 0 1 0 0 0 0 0

    Skip Conditions:    C = "1"

    Description:        Skip if C is True

*/

INSTRUCTION( skc )
{
	if (C == 1) m_skip = 1;
}

/*

    Mnemonic:           SKE

    Hex Code:           21
    Binary:             0 0 1 0 0 0 0 1

    Skip Conditions:    A = RAM(B)

    Description:        Skip if A Equals RAM

*/

INSTRUCTION( ske )
{
	if (A == RAM_R(B)) m_skip = 1;
}

/*

    Mnemonic:           SKGZ

    Hex Code:           33 21
    Binary:             00 0 1 1 0 0 1 1 0 0 1 0 0 0 0 1

    Skip Conditions:    G3:0 = 0

    Description:        Skip if G is Zero (all 4 bits)

*/

INSTRUCTION( skgz )
{
	if (IN_G() == 0) m_skip = 1;
}

/*

    Mnemonic:           SKGBZ

    Hex Code:           33 01
                        33 11
                        33 03
                        33 13

    Binary:

    Skip Conditions:    G0 = 0
                        G1 = 0
                        G2 = 0
                        G3 = 0

    Description:        Skip if G Bit is Zero

*/

void cop400_cpu_device::skgbz(int bit)
{
	if (!BIT(IN_G(), bit)) m_skip = 1;
}

INSTRUCTION( skgbz0 ) { skgbz(0); }
INSTRUCTION( skgbz1 ) { skgbz(1); }
INSTRUCTION( skgbz2 ) { skgbz(2); }
INSTRUCTION( skgbz3 ) { skgbz(3); }

/*

    Mnemonic:           SKMBZ

    Hex Code:           01
                        11
                        03
                        13

    Binary:

    Skip Conditions:    RAM(B)0 = 0
                        RAM(B)0 = 1
                        RAM(B)0 = 2
                        RAM(B)0 = 3

    Description:        Skip if RAM Bit is Zero

*/

void cop400_cpu_device::skmbz(int bit)
{
	if (!BIT(RAM_R(B), bit)) m_skip = 1;
}

INSTRUCTION( skmbz0 ) { skmbz(0); }
INSTRUCTION( skmbz1 ) { skmbz(1); }
INSTRUCTION( skmbz2 ) { skmbz(2); }
INSTRUCTION( skmbz3 ) { skmbz(3); }

/*

    Mnemonic:           SKT

    Hex Code:           41
    Binary:             0 1 0 0 0 0 0 1

    Skip Conditions:    A time-base counter carry has occurred since last test

    Description:        Skip on Timer

*/

INSTRUCTION( skt )
{
	if (m_skt_latch)
	{
		m_skt_latch = 0;
		m_skip = 1;
	}
}

/***************************************************************************
    INPUT/OUTPUT INSTRUCTIONS
***************************************************************************/

/*

    Mnemonic:           ING

    Hex Code:           33 2A
    Binary:

    Data Flow:          G -> A

    Description:        Input G Ports to A

*/

INSTRUCTION( ing )
{
	A = IN_G();
}

/*

    Mnemonic:           INL

    Hex Code:           33 2E
    Binary:

    Data Flow:          L7:4 -> RAM(B)
                        L3:0 -> A

    Description:        Input L Ports to RAM,A

*/

INSTRUCTION( inl )
{
	UINT8 L = IN_L();

	RAM_W(B, L >> 4);
	A = L & 0xF;
}

/*

    Mnemonic:           OBD

    Hex Code:           33 3E
    Binary:

    Data Flow:          Bd -> D

    Description:        Output Bd to D Outputs

*/

INSTRUCTION( obd )
{
	OUT_D(B & 0x0f);
}

/*

    Mnemonic:           OMG

    Hex Code:           33 3A
    Binary:

    Data Flow:          RAM(B) -> G

    Description:        Output RAM to G Ports

*/

INSTRUCTION( omg )
{
	WRITE_G(RAM_R(B));
}

/*

    Mnemonic:           XAS

    Hex Code:           4F
    Binary:             0 1 0 0 1 1 1 1

    Data Flow:          A <-> SIO
                        C -> SK

    Description:        Exchange A with SIO

*/

INSTRUCTION( xas )
{
	UINT8 t = SIO;
	SIO = A;
	A = t;

	SKL = C;
}

/*

    Mnemonic:           ININ

    Hex Code:           33 28
    Binary:

    Data Flow:          IN -> A

    Description:        Input IN Inputs to A

*/

INSTRUCTION( inin )
{
	A = IN_IN();
}

/*

    Processor:          COP402M

    Mnemonic:           ININ

    Hex Code:           33 28
    Binary:

    Data Flow:          IN -> A, A1 = "1"

    Description:        Input IN Inputs to A

*/

INSTRUCTION( cop402m_inin )
{
	A = IN_IN() | 0x02;
}

/*

    Mnemonic:           INIL

    Hex Code:           33 29
    Binary:

    Data Flow:          IL3,CKO,"0",IL0 -> A

    Description:        Input IL Latches to A

*/

INSTRUCTION( inil )
{
	A = (IL & 0x09) | IN_CKO() << 2;

	IL = 0;
}

/*

    Mnemonic:           OGI

    Operand:            y
    Hex Code:           33 5-
    Binary:             0 0 1 1 0 0 1 1 0 1 0 1 y3 y2 y1 y0

    Data Flow:          y -> G

    Description:        Output to G Ports Immediate

*/

INSTRUCTION( ogi )
{
	UINT8 y = opcode & 0x0f;

	WRITE_G(y);
}
