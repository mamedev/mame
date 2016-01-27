// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    cpu/apexc/apexc.c: APE(X)C CPU emulation

    By Raphael Nabet

    APE(X)C (All Purpose Electronic X-ray Computer) was a computer built by Andrew D. Booth
    and others for the Birkbeck College, in London, which was used to compute cristal
    structure using X-ray diffraction.

    It was one of the APEC series of computer, which were simple electronic computers
    built in the early 1950s for various British Universities.  Known members of this series
    are:
    * APE(X)C: Birkbeck College, London (before 1953 (1951?))
    * APE(N)C: Board of Mathematical Machines, Oslo
    * APE(H)C: British Tabulating Machine Company
    * APE(R)C: British Rayon Research Association
    * UCC: University College, London (circa january 1956)
    * MAC (Magnetic Automatic Calculator): "built by Wharf Engineering Laboratories"
    (february 1955), which used some germanium diodes
    * The HEC (built by the British Tabulating Machine Company), a commercial machine sold
    in two models at least (HEC 2M and HEC 4) (before 1955)

    References:
    * Andrew D. Booth & Kathleen H. V. Booth: Automatic Digital Calculators, 2nd edition
    (Buttersworth Scientific Publications, 1956)  (referred to as 'Booth&Booth')
    * Kathleen H. V. Booth: Programming for an Automatic Digital Calculator
    (Buttersworth Scientific Publications, 1958)  (referred to as 'Booth')
    * Digital Engineering Newsletter vol 7 nb 1 p 60 and vol 8 nb 1 p 60-61 provided some
    dates
*/

/*
    Generals specs:
    * 32-bit data word size (10-bit addresses): uses fixed-point, 2's complement arithmetic
    * CPU has one accumulator (A) and one register (R), plus a Control Register (this is
      what we would call an "instruction register" nowadays).  No Program Counter, each
      instruction contains the address of the next instruction (!).
    * memory is composed of 256 (maximal value only found on the UCC - APE(X)C only has
      32 tracks) circular magnetic tracks of 32 words: only 32 tracks can
      be accessed at a time (the 16 first ones, plus 16 others chosen by the programmer),
      and the rotation rate is 3750rpm (62.5 rotations per second).
    * two I/O units: tape reader and tape puncher.  A teletyper was designed to read
      specially-encoded punched tapes and print decoded text.  (See /systems/apexc.c)
    * machine code has 15 instructions (!), including add, substract, shift, multiply (!),
      test and branch, input and punch.  A so-called vector mode allow to repeat the same
      operation 32 times with 32 successive memory locations.  Note the lack of bitwise
      and/or/xor (!) .
    * 1 kIPS, although memory access times make this figure fairly theorical (drum rotation
      time: 16ms, which would allow about 60IPS when no optimization is made)
    * there is no indirect addressing whatever, although dynamic modification of opcodes (!)
      allows to simulate it...
    * a control panel allows operation and debugging of the machine.  (See /systems/apexc.c)

    Conventions:
    Bits are numbered in big-endian order, starting with 1: bit #1 is the
    MSBit, and bit #32 is the LSBit.

    References:
    * Andrew D. Booth & Kathleen H. V. Booth: Automatic Digital Calculators, 2nd edition
    (Buttersworth Scientific Publications, 1956)
    * Kathleen H. V. Booth: Programming for an Automatic Digital Calculator
    (Buttersworth Scientific Publications, 1958)
*/

/*
    Machine code (reference: Booth):

    Format of a machine instruction:
bits:       1-5         6-10        11-15       16-20       21-25       26-31       32
field:      X address   X address   Y address   Y address   Function    C6          Vector
            (track)     (location)  (track)     (location)

    Meaning of fields:
    X: address of an operand, or immediate, or meaningless, depending on Function
        (When X is meaningless, it should be a duplicate of Y.  Maybe this is because
        X is unintentionnally loaded into the memory address register, and if track # is
        different, we add unneeded track switch delays (this theory is either wrong or
        incomplete, since it cannot be true for B or X))
    Y: address of the next instruction
    Function: code for the actual instruction executed
    C6: immediate value used by shift, multiply and store operations
    Vector: repeat operation 32 times (on all 32 consecutive locations of a track,
        starting with the location given by the X field)

    Function code:
    #   Mnemonic    C6      Description

    0   Stop

    2   I(y)                Input.  A 5-bit word is read from tape and loaded
                            into the 5 MSBits of R.  (These bits of R must be
                            cleared initially.)

    4   P(y)                Punch.  The 5 MSBits of R are punched onto the
                            output tape.

    6   B<(x)>=(y)          Branch.  If A<0, next instruction is fetched from @x, whereas
                            if A>=0, next instruction is fetched from @y

    8   l (y)       n       Shift left: the 64 bits of A and R are rotated left n times.
         n

    10  r (y)       64-n    Shift right: the 64 bits of A and R are shifted right n times.
         n                  The sign bit of A is duplicated.

    14  X (x)(y)    33-n    Multiply the contents of *track* x by the last n digits of the
         n                  number in R, sending the 32 MSBs to A and 31 LSBs to R

    16  +c(x)(y)            A <- (x)

    18  -c(x)(y)            A <- -(x)

    20  +(x)(y)             A <- A+(x)

    22  -(x)(y)             A <- A-(x)

    24  T(x)(y)             R <- (x)

    26  R   (x)(y)  32+n    Store first or last bits of R into (x).  The remaining bits of (x)
         1-n                are unaffected.  "The contents of R are filled with 0s or 1s
                            according as the original contents were positive or negative".
        R    (x)(y) n-1
         n-32

    28  A   (x)(y)  32+n    Same as 26, except that source is A, and the contents of A are
         1-n                not modified.

        A    (x)(y) n-1
         n-32

    30  S(x)(y)             Block Head switch.  This enables the block of heads specified
                            in x to be loaded into the working store.

    Note: Mnemonics use subscripts (!), which I tried to render the best I could.  Also,
      ">=" is actually one single character.  Last, "1-n" and "n-32" in store mnemonics
      are the actual sequences "1 *DASH* <number n>" and "<number n> *DASH* 32"
      (these are NOT formulas with substract signs).

    Note2: Short-hand notations: X stands for X  , A for A    , and R for R    .
                                               32         1-32             1-32

    Note3: Vectors instruction are notated with a subscript 'v' following the basic
      mnemonic.  For instance:

        A (x)(y), + (x)(y)
         v         v

      are the vector counterparts of A(x)(y) and +(x)(y).




    Note that the code has been presented so far as it was in 1957.  It appears that
    it was somewhat different in 1953 (Booth&Booth):

    Format of a machine instruction:
    Format for r, l, A:
bits:       1-9         10-15       16-17   18-21       22-30       31-32
field:      X address   C6          spare   Function    Y address   spare
    Format for other instructions:
bits:       1-9         10-17       18-21       22-30       31-32
field:      X address   D           Function    Y address   D (part 2)

    Meaning of fields:
    D (i.e. drum #): MSBs for the address of the X operand.  I don't know whether this feature
        was actually implemented, since it is said in Booth&Booth that the APE(X)C does
        not use this feature (it had only one drum of 16 tracks at the time, hence the 9
        address bits).

    Function code:
    #   Mnemonic    C6      Description

    1   A   (x)(y)  32+n(?) record first bits of A in (x).  The remaining bits of x
         1-n                are unaffected.

    2   +c(x)(y)            A <- (x)

    3   -c(x)(y)            A <- -(x)

    4   +(x)(y)             A <- A+(x)

    5   -(x)(y)             A <- A-(x)

    6   T(x)(y)             R <- (x)

    7   X (x)(y)            Multiply the contents of (x) by the number in R,
                            sending the 32 MSBs to A and 31 LSBs to R

    8   r (y)       64-n(?) Shift right: the 64 bits of A and R are shifted right n times.
         n                  The sign bit of A is duplicated.

    9   l (y)       n(?)    Shift left: the 64 bits of A and R are rotated left n times.
         n

    10  R   (x)(y)  32+n    record R into (x).
         1-n                "the contents of R are filled with 0s or 1s
                            according as the original contents were positive or negative".

    11  B<(x)>=(y)          Branch.  If A<0, next instruction is read from @x, whereas
                            if A>=0, next instruction is read from @y

    12  Print(y)            Punch.  Contents of A are printed.

    13  C(d+x)              branch ("switch Control") to instruction located in position
                            (D:X)

    14  Stop

    You will notice the absence of input instruction.  It seems that program and data were
    meant to be entered with a teletyper or a card reader located on the control panel.

    I don't know whether this computer really was in operation with this code.  Handle
    these info with caution.
*/

/*
    memory interface:

    Data is exchanged on a 1-bit (!) data bus, 10-bit address bus.

    While the bus is 1-bit wide, read/write operation can only be take place on word
    (i.e. 32 bit) boundaries.  However, it is possible to store only the n first bits or
    n last bits of a word, leaving other bits in memory unaffected.

    The LSBits are transferred first, since this enables to perform bit-per-bit add and
    substract.  Otherwise, the CPU would need an additionnal register to store the second
    operand, and it would be probably slower, since the operation could only
    take place after all the data has been transfered.

    Memory operations are synchronous with 2 clocks found on the memory controller:
    * word clock: a pulse on each word boundary (3750rpm*32 -> 2kHz)
    * bit clock: a pulse when a bit is present on the bus (word clock * 32 -> 64kHz)

    CPU operation is synchronous with these clocks, too.  For instance, the AU does bit-per-bit
    addition and substraction with a memory operand, synchronously with bit clock,
    starting and stopping on word clock boundaries.  Similar thing with a Fetch operation.

    There is a 10-bit memory location (i.e. address) register on the memory controller.
    It is loaded with the contents of X after when instruction fetch is complete, and
    with the contents of Y when instruction execution is complete, so that the next fetch
    can be executed correctly.
*/

/*
    Instruction timings:


    References: Booth p. 14 for the table below


    Mnemonic                    delay in word clock cycles

    I                           32

    P                           32

    B                           0

    l                           1 if n>=32 (i.e. C6>=32) (see 4.)
     n                          2 if n<32  (i.e. C6<32)

    r                           1 if n<=32 (i.e. C6>=32) (see 4.)
     n                          2 if n>32  (i.e. C6<32)

    X                           32

    +c, -c, +, -, T             0

    R    , R    , A   , A       1 (see 1. & 4.)
      1-n   n-32   1-n   n-32

    track switch                6 (see 2.)

    vector                      12 (see 3.)


    (S and stop are missing in the table)


    Note that you must add the fetch delay (at least 1 cycle), and, when applicable, the
    operand read/write delay (at least 1 cycle).


    Notes:

    1.  The delay is applied after the store is done (from the analysis of the example
     in Booth p.52)

    2.  I guess that the memory controller needs 6 cycles to stabilize whenever track
      switching occurs, i.e. when X does not refer to the current track, and then when Y
      does not refer to the same track as X.  This matches various examples in Booth,
      although it appears that this delay is not applied when X is not read (cf cross-track
      B in Booth p. 49).
        However, and here comes the wacky part, analysis of Booth p. 55 shows that
      no additionnal delay is caused by an X instruction having its X operand
      on another track.  Maybe, just maybe, this is related to the fact that X does not
      need to take the word count into account, any word in track is as good as any (yet,
      this leaves the question of why this optimization could not be applied to vector
      operations unanswered).

    3.  This is an ambiguous statement.  Analysis of Booth p. 55 shows that
      an instance of an Av instruction with its destination on another track takes no more
      than 45 cycles, as follow:
        * 1 cycle for fetch
        * 6-cycle delay (at most) before write starts (-> track switch)
        * 32 memory cycles
        * 6-cycle delay (at most) after write completion (-> track switch)
      It appears that the delay associated with the vector mode is not distinguishable from
      the delay caused by track switch and even the delay associated to the Av instruction.
        Is there really a specific delay associated with the vector mode? To know this, we
      would need to see a vector instruction on the same track as its operands, which is
      unlikely to be seen (the only reasonnable application I can see is running a '+_v'
      to compute the checksum of the current track).

    4.  Example in Booth p. 76 ("20/4 A (27/27) (21/2)") seems to imply that
      when doing a store with a destination on a track other than the track where next
      instruction is located, the 1-cycle post-store delay is merged with the 6-cycle track
      switch delay.  (I assume this because there is lots of room on track 21, and if
      the delays were not merged, it should be easy to move the instruction forward
      to speed up loop execution time.
        Similarly, example in Booth p. 49-50 ("4/24 l 32 (5/31)") seems to show that
      a similar delay merge occurs when doing a shift with the next instruction located on
      another track.
*/

#include "emu.h"
#include "debugger.h"
#include "apexc.h"


const device_type APEXC = &device_creator<apexc_cpu_device>;


/* decrement ICount by n */
#define DELAY(n)    {m_icount -= (n); m_current_word = (m_current_word + (n)) & 0x1f;}


apexc_cpu_device::apexc_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, APEXC, "APEXC", tag, owner, clock, "apexc_cpu", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 15, 0)
	, m_io_config("io", ENDIANNESS_BIG, 8, 1, 0)
	, m_a(0)
	, m_r(0)
	, m_cr(0)
	, m_ml(0)
	, m_working_store(1)
	, m_running(0)
	, m_pc(0)
	, m_ml_full(0)
{
}


/*
    word accessor functions

    take a 10-bit word address
        5 bits (MSBs): track address within working store
        5 bits (LSBs): word position within track

    'special' flag: if true, read first word found in track (used by X instruction only)

    'mask': one bit is set for each bit to write (used by store instructions)

    memory latency delays are taken into account, but not track switching delays
*/

/* compute complete word address (i.e. translate a logical track address (expressed
in current working store) to an absolute track address) */
UINT32 apexc_cpu_device::effective_address(UINT32 address)
{
	if (address & 0x200)
	{
		address = (address & 0x1FF) | (m_working_store) << 9;
	}

	return address;
}

/* read word */
UINT32 apexc_cpu_device::word_read(UINT32 address, UINT32 special)
{
	UINT32 result;

	/* compute absolute track address */
	address = effective_address(address);

	if (special)
	{
		/* ignore word position in x - use current position instead */
		address = (address & ~ 0x1f) | m_current_word;
	}
	else
	{
		/* wait for requested word to appear under the heads */
		DELAY(((address /*& 0x1f*/) - m_current_word) & 0x1f);
	}

	/* read 32 bits */
	result = apexc_readmem(address);

	/* read takes one memory cycle */
	DELAY(1);

	return result;
}

/* write word (or part of a word, according to mask) */
void apexc_cpu_device::word_write(UINT32 address, UINT32 data, UINT32 mask)
{
	/* compute absolute track address */
	address = effective_address(address);

	/* wait for requested word to appear under the heads */
	DELAY(((address /*& 0x1f*/) - m_current_word) & 0x1f);

	/* write 32 bits according to mask */
	apexc_writemem_masked(address, data, mask);

	/* write takes one memory cycle (2, actually, but the 2nd cycle is taken into
	account in execute) */
	DELAY(1);
}

/*
    I/O accessors

    no address is used, these functions just punch or read 5 bits
*/

UINT8 apexc_cpu_device::papertape_read()
{
	return m_io->read_byte(0) & 0x1f;
}

void apexc_cpu_device::papertape_punch(UINT8 data)
{
	m_io->write_byte(0, data);
}

/*
    now for emulation code
*/

/*
    set the memory location (i.e. address) register, and compute the associated delay
*/
UINT32 apexc_cpu_device::load_ml(UINT32 address, UINT32 vector)
{
	int delay;

	/* additionnal delay appears if we switch tracks */
	if (((m_ml & 0x3E0) != (address & 0x3E0)) /*|| vector*/)
		delay = 6;  /* if tracks are different, delay to allow for track switching */
	else
		delay = 0;  /* else, no problem */

	m_ml = address; /* save ml */

	return delay;
}

/*
    execute one instruction

    TODO:
    * test!!!

    NOTE:
    * I do not know whether we should fetch instructions at the beginning or the end of the
    instruction cycle.  Either solution is roughly equivalent to the other, but changes
    the control panel operation (and I know virtually nothing on the control panel).
    Currently, I fetch each instruction right after executing the previous instruction, so that
    the user may enter an instruction into the control register with the control panel, then
    execute it.
    This solution makes timing simulation much simpler, too.
*/
void apexc_cpu_device::execute()
{
	int x, y, function, c6, vector; /* instruction fields */
	int i = 0;          /* misc counter */
	int has_operand;    /* true if instruction is an AU operation with an X operand */
	static const char has_operand_table[32] =   /* table for has_operand - one entry for each function code */
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0
	};
	int delay1; /* pre-operand-access delay */
	int delay2; /* post-operation delay */
	int delay3; /* pre-operand-fetch delay */

	/* first isolate the instruction fields */
	x = (m_cr >> 22) & 0x3FF;
	y = (m_cr >> 12) & 0x3FF;
	function = (m_cr >> 7) & 0x1F;
	c6 = (m_cr >> 1) & 0x3F;
	vector = m_cr & 1;
	m_pc = y<<2;

	function &= 0x1E;   /* this is a mere guess - the LSBit is reserved for future additions */

	/* determinates if we need to read an operand*/
	has_operand = has_operand_table[function];

	if (has_operand)
	{
		/* load ml with X */
		delay1 = load_ml(x, vector);
		/* burn pre-operand-access delay if needed */
		if (delay1)
		{
			DELAY(delay1);
		}
	}

	delay2 = 0; /* default */

	do
	{
		switch (function)
		{
		case 0:
			/* stop */

			m_running = FALSE;

			/* BTW, I don't know whether stop loads y into ml or not, and whether
			subsequent fetch is done */
			break;

		case 2:
			/* I */
			/* I do not know whether the CPU does an OR or whatever, but since docs say that
			the 5 bits must be cleared initially, an OR kind of makes sense */
			m_r |= papertape_read() << 27;
			delay2 = 32;    /* no idea whether this should be counted as an absolute delay
                            or as a value in delay2 */
			break;

		case 4:
			/* P */
			papertape_punch((m_r >> 27) & 0x1f);
			delay2 = 32;    /* no idea whether this should be counted as an absolute delay
                            or as a value in delay2 */
			break;

		case 6:
			/* B<(x)>=(y) */
			/* I have no idea what we should do if the vector bit is set */
			if (m_a & 0x80000000UL)
			{
				/* load ml with X */
				delay1 = load_ml(x, vector);
				m_pc = x<<2;
				/* burn pre-fetch delay if needed */
				if (delay1)
				{
					DELAY(delay1);
				}
				/* and do fetch at X */
				goto special_fetch;
			}
			/* else, the instruction ends with a normal fetch */
			break;

		case 8:
			/* l_n */
			delay2 = (c6 & 0x20) ? 1 : 2;   /* if more than 32 shifts, it takes more time */

			/* Yes, this code is inefficient, but this must be the way the APEXC does it ;-) */
			while (c6 != 0)
			{
				int shifted_bit = 0;

				/* shift and increment c6 */
				shifted_bit = m_r & 1;
				m_r >>= 1;
				if (m_a & 1)
					m_r |= 0x80000000UL;
				m_a >>= 1;
				if (shifted_bit)
					m_a |= 0x80000000UL;

				c6 = (c6+1) & 0x3f;
			}

			break;

		case 10:
			/* r_n */
			delay2 = (c6 & 0x20) ? 1 : 2;   /* if more than 32 shifts, it takes more time */

			/* Yes, this code is inefficient, but this must be the way the APEXC does it ;-) */
			while (c6 != 0)
			{
				/* shift and increment c6 */
				m_r >>= 1;
				if (m_a & 1)
					m_r |= 0x80000000UL;
				m_a = ((INT32) m_a) >> 1;

				c6 = (c6+1) & 0x3f;
			}

			break;

		case 12:
			/* unused function code.  I assume this results into a NOP, for lack of any
			specific info... */

			break;

		case 14:
			/* X_n(x) */

			/* Yes, this code is inefficient, but this must be the way the APEXC does it ;-) */
			/* algorithm found in Booth&Booth, p. 45-48 */
			{
				int shifted_bit;

				m_a = 0;
				shifted_bit = 0;
				while (1)
				{
					/* note we read word at current word position */
					if (shifted_bit && ! (m_r & 1))
						m_a += word_read(x, 1);
					else if ((! shifted_bit) && (m_r & 1))
						m_a -= word_read(x, 1);
					else
						/* Even if we do not read anything, the loop still takes 1 cycle of
						the memory word clock. */
						/* Anyway, maybe we still read the data even if we do not use it. */
						DELAY(1);

					/* exit if c6 reached 32 ("c6 & 0x20" is simpler to implement and
					essentially equivalent, so this is most likely the actual implementation) */
					if (c6 & 0x20)
						break;

					/* else increment c6 and  shift */
					c6 = (c6+1) & 0x3f;

					/* shift */
					shifted_bit = m_r & 1;
					m_r >>= 1;
					if (m_a & 1)
						m_r |= 0x80000000UL;
					m_a = ((INT32) m_a) >> 1;
				}
			}

			//DELAY(32);    /* mmmh... we have already counted 32 wait states */
			/* actually, if (n < 32) (which is an untypical case), we do not have 32 wait
			states.  Question is: do we really have 32 wait states if (n < 32), or is
			the timing table incomplete? */
			break;

		case 16:
			/* +c(x) */
			m_a = + word_read(m_ml, 0);
			break;

		case 18:
			/* -c(x) */
			m_a = - word_read(m_ml, 0);
			break;

		case 20:
			/* +(x) */
			m_a += word_read(m_ml, 0);
			break;

		case 22:
			/* -(x) */
			m_a -= word_read(m_ml, 0);
			break;

		case 24:
			/* T(x) */
			m_r = word_read(m_ml, 0);
			break;

		case 26:
			/* R_(1-n)(x) & R_(n-32)(x) */

			{
				UINT32 mask;

				if (c6 & 0x20)
					mask = 0xFFFFFFFFUL << (64 - c6);
				else
					mask = 0xFFFFFFFFUL >> c6;

				word_write(m_ml, m_r, mask);
			}

			m_r = (m_r & 0x80000000UL) ? 0xFFFFFFFFUL : 0;

			delay2 = 1;
			break;

		case 28:
			/* A_(1-n)(x) & A_(n-32)(x) */

			{
				UINT32 mask;

				if (c6 & 0x20)
					mask = 0xFFFFFFFFUL << (64 - c6);
				else
					mask = 0xFFFFFFFFUL >> c6;

				word_write(m_ml, m_a, mask);
			}

			delay2 = 1;
			break;

		case 30:
			/* S(x) */
			m_working_store = (x >> 5) & 0xf;   /* or is it (x >> 6)? */
			DELAY(32);  /* no idea what the value is...  All I know is that it takes much
                        more time than track switching (which takes 6 cycles) */
			break;
		}
		if (vector)
			/* increment word position in vector operations */
			m_ml = (m_ml & 0x3E0) | ((m_ml + 1) & 0x1F);
	} while (vector && has_operand && (++i < 32));  /* iterate 32 times if vector bit is set */
													/* the has_operand is a mere guess */

	/* load ml with Y */
	delay3 = load_ml(y, 0);

	/* compute max(delay2, delay3) */
	if (delay2 > delay3)
		delay3 = delay2;

	/* burn pre-fetch delay if needed */
	if (delay3)
	{
		DELAY(delay3);
	}

	/* entry point after a successful Branch (which alters the normal instruction sequence,
	in order not to load ml with Y) */
special_fetch:

	/* fetch current instruction into control register */
	m_cr = word_read(m_ml, 0);
}


void apexc_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	save_item(NAME(m_a));
	save_item(NAME(m_r));
	save_item(NAME(m_cr));
	save_item(NAME(m_ml));
	save_item(NAME(m_working_store));
	save_item(NAME(m_current_word));
	save_item(NAME(m_running));
	save_item(NAME(m_pc));

	state_add( APEXC_CR, "CR", m_cr ).formatstr("%08X");
	state_add( APEXC_A, "A", m_a ).formatstr("%08X");
	state_add( APEXC_R, "R", m_r ).formatstr("%08X");
	state_add( APEXC_ML, "ML", m_ml ).mask(0xfff).formatstr("%03X");
	state_add( APEXC_WS, "WS", m_working_store ).mask(0x01);
	state_add( APEXC_STATE, "CPU state", m_running ).mask(0x01);
	state_add( APEXC_PC, "PC", m_pc ).callimport().callexport().formatstr("%03X");
	state_add( APEXC_ML_FULL, "ML_FULL", m_ml_full ).callimport().callexport().noshow();

	m_icountptr = &m_icount;
}


void apexc_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case APEXC_PC:
			/* keep address 9 LSBits - 10th bit depends on whether we are accessing the permanent
			track group or a switchable one */
			m_ml = m_pc & 0x1ff;
			if (m_pc & 0x1e00)
			{   /* we are accessing a switchable track group */
				m_ml |= 0x200;  /* set 10th bit */

				if (((m_pc >> 9) & 0xf) != m_working_store)
				{   /* we need to do a store switch */
					m_working_store = ((m_pc >> 9) & 0xf);
				}
			}
			break;
	}
}


void apexc_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case APEXC_ML_FULL:
			m_ml_full = effective_address(m_ml);
			break;
	}
}


void apexc_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c", m_running ? 'R' : 'S');
			break;
	}
}


void apexc_cpu_device::device_reset()
{
	/* mmmh...  I don't know what happens on reset with an actual APEXC. */

	m_working_store = 1;    /* mere guess */
	m_current_word = 0;     /* well, we do have to start somewhere... */

	/* next two lines are just the product of my bold fantasy */
	m_cr = 0;               /* first instruction executed will be a stop */
	m_running = TRUE;       /* this causes the CPU to load the instruction at 0/0,
                               which enables easy booting (just press run on the panel) */
	m_a = 0;
	m_r = 0;
	m_pc = 0;
	m_ml = 0;
}


void apexc_cpu_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, m_pc);

		if (m_running)
			execute();
		else
		{
			DELAY(m_icount);    /* burn cycles once for all */
		}
	} while (m_icount > 0);
}


offs_t apexc_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( apexc );
	return CPU_DISASSEMBLE_NAME(apexc)(this, buffer, pc, oprom, opram, options);
}
