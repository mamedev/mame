/***************************************************************************

    410ops.c

    National Semiconductor COP410 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#define UINT1	UINT8
#define UINT4	UINT8
#define UINT6	UINT8
#define UINT10	UINT16

typedef struct _cop400_state cop400_state;
struct _cop400_state
{
	const cop400_interface *intf;

    const address_space *program;
    const address_space *data;
    const address_space *io;

	/* registers */
	UINT10 	pc;	   			/* 11-bit ROM address program counter */
	UINT10	prevpc;			/* previous value of program counter */
	UINT4	a;				/* 4-bit accumulator */
	UINT8	b;				/* 5/6/7-bit RAM address register */
	UINT1	c;				/* 1-bit carry register */
	UINT4	en;				/* 4-bit enable register */
	UINT4	g;				/* 4-bit general purpose I/O port */
	UINT8	q;				/* 8-bit latch for L port */
	UINT10	sa, sb, sc;		/* subroutine save registers */
	UINT4	sio;			/* 4-bit shift register and counter */
	UINT1	skl;			/* 1-bit latch for SK output */
	UINT8	si;				/* serial input */
	UINT4	h;				/* 4-bit general purpose I/O port (COP440 only) */
	UINT8	r;				/* 8-bit general purpose I/O port (COP440 only) */

	/* counter */
	UINT8	t;				/* 8-bit timer */
	int		skt_latch;		/* timer overflow latch */

	/* input/output ports */
	UINT8	g_mask;			/* G port mask */
	UINT8	d_mask;			/* D port mask */
	UINT8	in_mask;		/* IN port mask */
	UINT4	il;				/* IN latch */
	UINT4	in[4];			/* IN port shift register */

	/* skipping logic */
	int skip;				/* skip next instruction */
	int skip_lbi;			/* skip until next non-LBI instruction */
	int last_skip;			/* last value of skip */
	int halt;				/* halt mode */
	int idle;				/* idle mode */

	/* microbus */
	int microbus_int;		/* microbus interrupt */

	/* execution logic */
	int InstLen[256];		/* instruction length in bytes */
	int LBIops[256];
	int LBIops33[256];
	int icount;				/* instruction counter */

	const device_config *device;

	/* timers */
	emu_timer *serial_timer;
	emu_timer *counter_timer;
	emu_timer *inil_timer;
	emu_timer *microbus_timer;
};

/* The opcode table now is a combination of cycle counts and function pointers */
typedef struct {
	unsigned cycles;
	void (*function) (cop400_state *cop400, UINT8 opcode);
}	s_opcode;

#define INSTRUCTION(mnemonic) INLINE void (mnemonic)(cop400_state *cop400, UINT8 opcode)

#define ROM(a)			memory_decrypted_read_byte(cop400->program, a)
#define RAM_R(a)		memory_read_byte_8le(cop400->data, a)
#define RAM_W(a, v)		memory_write_byte_8le(cop400->data, a, v)
#define IN(a)			memory_read_byte_8le(cop400->io, a)
#define OUT(a, v)		memory_write_byte_8le(cop400->io, a, v)

#define A				cop400->a
#define B				cop400->b
#define C				cop400->c
#define G				cop400->g
#define Q				cop400->q
#define EN				cop400->en
#define SA				cop400->sa
#define SB				cop400->sb
#define SIO				cop400->sio
#define SKL				cop400->skl
#define PC				cop400->pc
#define prevPC			cop400->prevpc

#define IN_G()			IN(COP400_PORT_G)
#define IN_L()			IN(COP400_PORT_L)
#define IN_SI()			BIT(IN(COP400_PORT_SIO), 0)
#define IN_CKO()		BIT(IN(COP400_PORT_CKO), 0)
#define OUT_G(A)		OUT(COP400_PORT_G, (A) & cop400->g_mask)
#define OUT_L(A)		OUT(COP400_PORT_L, (A))
#define OUT_D(A)		OUT(COP400_PORT_D, (A) & cop400->d_mask)
#define OUT_SK(A)		OUT(COP400_PORT_SK, (A))
#define OUT_SO(A)		OUT(COP400_PORT_SIO, (A))

#ifndef PUSH
#define PUSH(addr) 		{ SB = SA; SA = addr; }
#define POP() 			{ PC = SA; SA = SB; }
#endif

/* Serial I/O */

static TIMER_CALLBACK( cop400_serial_tick )
{
	cop400_state *cop400 = ptr;

	if (BIT(EN, 0))
	{
		/*

            SIO is an asynchronous binary counter decrementing its value by one upon each low-going pulse ("1" to "0") occurring on the SI input.
            Each pulse must remain at each logic level at least two instruction cycles. SK outputs the value of the C upon the execution of an XAS
            and remains latched until the execution of another XAS instruction. The SO output is equal to the value of EN3.

        */

		// serial output

		OUT_SO(BIT(EN, 3));

		// serial clock

		OUT_SK(SKL);

		// serial input

		cop400->si <<= 1;
		cop400->si = (cop400->si & 0x0e) | IN_SI();

		if ((cop400->si & 0x0f) == 0x0c) // 1100
		{
			SIO--;
			SIO &= 0x0f;
		}
	}
	else
	{
		/*

            SIO is a serial shift register, shifting continuously left each instruction cycle time. The data present at SI goes into the least
            significant bit of SIO: SO can be enabled to output the most significant bit of SIO each cycle time. SK output becomes a logic-
            controlled clock, providing a SYNC signal each instruction time. It will start outputting a SYNC pulse upon the execution of an XAS
            instruction with C = "1," stopping upon the execution of a subsequent XAS with C = "0".

            If EN0 is changed from "1" to "0" ("0" to "1") the SK output will change from "1" to SYNC (SYNC to "1") without the execution of
            an XAS instruction.

        */

		// serial output

		if (BIT(EN, 3))
		{
			OUT_SO(BIT(SIO, 3));
		}
		else
		{
			OUT_SO(0);
		}

		// serial clock

		if (SKL)
		{
			OUT_SK(1); // SYNC
		}
		else
		{
			OUT_SK(0);
		}

		// serial input

		SIO = ((SIO << 1) | IN_SI()) & 0x0f;
	}
}

INLINE void WRITE_Q(cop400_state *cop400, UINT8 data)
{
	Q = data;

	if (BIT(EN, 2))
	{
		OUT_L(Q);
	}
}

INLINE void WRITE_G(cop400_state *cop400, UINT8 data)
{
	if (cop400->intf->microbus == COP400_MICROBUS_ENABLED)
	{
		data = (data & 0x0e) | cop400->microbus_int;
	}

	G = data;

	OUT_G(G);
}

INSTRUCTION(illegal)
{
	logerror("COP400: PC = %04x, Illegal opcode = %02x\n", PC-1, ROM(PC-1));
}

/* Arithmetic Instructions */

/*

    Mnemonic:           ASC

    Hex Code:           30
    Binary:             0 0 1 1 0 0 0 0

    Data Flow:          A + C + RAM(B) -> A
                        Carry -> C

    Skip Conditions:    Carry

    Description:        Add with Carry, Skip on Carry

*/

INSTRUCTION(asc)
{
	A = A + C + RAM_R(B);

	if (A > 0xF)
	{
		C = 1;
		cop400->skip = 1;
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
	UINT4 y = opcode & 0x0f;

	A = A + y;

	if (A > 0x0f)
	{
		cop400->skip = 1;
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

INSTRUCTION( xor )
{
	A = A ^ RAM_R(B);
}

/* Transfer-of-Control Instructions */

/*

    Mnemonic:           JID

    Hex Code:           FF
    Binary:             1 1 1 1 1 1 1 1

    Data Flow:          ROM(PC9:8,A,M) -> PC7:0

    Description:        Jump Indirect

*/

INSTRUCTION( jid )
{
	UINT16 addr = (PC & 0x300) | (A << 4) | RAM_R(B);
	PC = (PC & 0x300) | ROM(addr);
}

/*

    Mnemonic:           JMP

    Operand:            a
    Hex Code:           6- --
    Binary:             0 1 1 0 0 0 a9 a8 a7 a6 a5 a4 a3 a2 a1 a0

    Data Flow:          a -> PC

    Description:        Jump

*/

INSTRUCTION( jmp )
{
	UINT16 a = ((opcode & 0x03) << 8) | ROM(PC);

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
	UINT4 page = PC >> 6;

	if (page == 2 || page == 3)
	{
		UINT8 a = opcode & 0x7f;
		PC = (PC & 0x380) | a;
	}
	else if ((opcode & 0xc0) == 0xc0)
	{
		UINT8 a = opcode & 0x3f;
		PC = (PC & 0x3c0) | a;
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
    Binary:             0 1 1 0 1 0 a9 a8 a7 a6 a5 a4 a3 a2 a1 a0

    Data Flow:          PC + 1 -> SA -> SB -> SC
                        a -> PC

    Description:        Jump to Subroutine

*/

INSTRUCTION( jsr )
{
	UINT16 a = ((opcode & 0x03) << 8) | ROM(PC);

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
	cop400->skip = 1;
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
	cop400->halt = 1;
}

/* Memory Reference Instructions */

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

	WRITE_Q(cop400, data);

#ifdef CAMQ_BUG
	WRITE_Q(cop400, 0x3c);
	WRITE_Q(cop400, data);
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

    Data Flow:          ROM(PC9:8,A,M) -> Q
                        SB -> SC

    Description:        Load Q Indirect

*/

INSTRUCTION( lqid )
{
	PUSH(PC);
	PC = (PC & 0x300) | (A << 4) | RAM_R(B);
	WRITE_Q(cop400, ROM(PC));
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
	UINT4 y = opcode & 0x0f;
	UINT16 Bd;

	RAM_W(B, y);

	Bd = ((B & 0x0f) + 1) & 0x0f;
	B = (B & 0x30) + Bd;
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
	UINT4 r = opcode & 0x30;
	UINT8 t = RAM_R(B);

	RAM_W(B, A);

	A = t;
	B = B ^ r;
}

/*

    Mnemonic:           XAD

    Operand:            r,d
    Hex Code:           23 --
    Binary:             0 0 1 0 0 0 1 1 1 0 r1 r0 d3 d2 d1 d0

    Data Flow:          RAM(r,d) <-> A

    Description:        Exchange A with RAM pointed to directly by r,d

*/

INSTRUCTION( xad )
{
	UINT8 rd = opcode & 0x3f;
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
	UINT4 r = opcode & 0x30;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Bd = ((B & 0x0f) - 1) & 0x0f;
	B = (B & 0x30) | Bd;

	B = B ^ r;

	if (Bd == 0x0f) cop400->skip = 1;
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
	UINT4 r = opcode & 0x30;

	t = RAM_R(B);
	RAM_W(B, A);
	A = t;

	Bd = ((B & 0x0f) + 1) & 0x0f;
	B = (B & 0x30) | Bd;

	B = B ^ r;

	if (Bd == 0x00) cop400->skip = 1;
}

/* Register Reference Instructions */

/*

    Mnemonic:           CAB

    Hex Code:           50
    Binary:             0 1 0 1 0 0 0 0 0

    Data Flow:          A -> Bd

    Description:        Copy A to Bd

*/

INSTRUCTION( cab )
{
	B = (B & 0x30) | A;
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
                        0 0 1 1 0 0 1 1 1 0 r1 r0 d3 d2 d1 d0

    Data Flow:          r,d -> B

    Skip Conditions:    Skip until not a LBI

    Description:        Load B Immediate with r,d

*/

INSTRUCTION( lbi )
{
	if (opcode & 0x80)
	{
		B = opcode & 0x3f;
	}
	else
	{
		B = (opcode & 0x30) | (((opcode & 0x0f) + 1) & 0x0f);
	}

	cop400->skip_lbi = 1;
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
	UINT4 y = opcode & 0x0f;

	EN = y;

	if (BIT(EN, 2))
	{
		OUT_L(Q);
	}
}

/* Test Instructions */

/*

    Mnemonic:           SKC

    Hex Code:           20
    Binary:             0 0 1 0 0 0 0 0

    Skip Conditions:    C = "1"

    Description:        Skip if C is True

*/

INSTRUCTION( skc )
{
	if (C == 1) cop400->skip = 1;
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
	if (A == RAM_R(B)) cop400->skip = 1;
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
	if (IN_G() == 0) cop400->skip = 1;
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

INLINE void skgbz(cop400_state *cop400, int bit)
{
	if (!BIT(IN_G(), bit)) cop400->skip = 1;
}

INSTRUCTION( skgbz0 ) { skgbz(cop400, 0); }
INSTRUCTION( skgbz1 ) { skgbz(cop400, 1); }
INSTRUCTION( skgbz2 ) { skgbz(cop400, 2); }
INSTRUCTION( skgbz3 ) { skgbz(cop400, 3); }

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

INLINE void skmbz(cop400_state *cop400, int bit)
{
	if (!BIT(RAM_R(B), bit)) cop400->skip = 1;
}

INSTRUCTION( skmbz0 ) { skmbz(cop400, 0); }
INSTRUCTION( skmbz1 ) { skmbz(cop400, 1); }
INSTRUCTION( skmbz2 ) { skmbz(cop400, 2); }
INSTRUCTION( skmbz3 ) { skmbz(cop400, 3); }

/* Input/Output Instructions */

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
	WRITE_G(cop400, RAM_R(B));
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
