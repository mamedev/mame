// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller

/*****************************************************************************
 *
 *   pps4.c
 *
 *   Rockwell PPS-4 CPU
 *   Introduced in 1972, it ran at 256kHz. An improved version was released
 *   in 1975, but could only manage 200kHz. The chipset continued to be
 *   produced through the 1980s, but never found much acceptance. Chip
 *   numbers are 10660 (original), 11660, 12660.
 *
 *   List of support / peripheral chips:
 *   10706   Clock generator
 *   10738   Bus interface
 *   11049   Interval timer
 *   10686   General purpose I/O
 *   10696   General purpose I/O
 *   10731   Telecommunications data interface
 *   10736   dot matrix printer controller
 *   10788   keyboard/display controller
 *   10789   printer controller
 *   10815   keyboard/printer controller
 *   10930   Serial data controller
 *   15380   dot matrix printer controller
 *
 *   Note: External clock should be divided by 18 (not implemented).
 *
 *   Pinouts:
 *              10660                               11660
 *
 *      +--------\  /--------+              +--------\  /--------+
 *  1  [| DIB-3   ++  DIA-3  |]  42     1  [| DIO-4       DIO-3  |]  42
 *  2 [-| DIA-2       DIB-4  |-] 41     2 [-| DIA-4       DIO-2  |-] 41
 *  3  [| DIB-2       DIA-4  |]  40     3  [| DIA-3       DIO-1  |]  40
 *  4 [-| DIA-1       NC     |-] 39     4 [-| DIA-2       Vdd    |-] 39
 *  5  [| DIB-1       A/B-1  |]  38     5  [| DIA-1       A/B-1  |]  38
 *  6 [-| Vdd         A/B-2  |-] 37     6 [-| I/O-5       A/B-2  |-] 37
 *  7  [| I/D-5       A/B-3  |]  36     7  [| I/O-6       A/B-3  |]  36
 *  8 [-| I/D-6       A/B-4  |-] 35     8 [-| I/O-7       A/B-4  |-] 35
 *  9  [| I/D-7       A/B-5  |]  34     9  [| I/O-8       A/B-5  |]  34
 * 10 [-| I/D-8       A/B-6  |-] 33    10 [-| I/O-1       A/B-6  |-] 33
 * 11  [| I/D-1       A/B-7  |]  32    11  [| I/O-4       A/B-7  |]  32
 * 12 [-| I/D-4       A/B-8  |-] 31    12 [-| I/O-2       A/B-8  |-] 31
 * 13  [| I/D-2       A/B-9  |]  30    13  [| I/O-3       A/B-9  |]  30
 * 14 [-| I/D-3       A/B-10 |-] 29    14 [-| W/IO        A/B-10 |-] 29
 * 15  [| W/IO        A/B-11 |]  28    15  [| CLK ~B      A/B-11 |]  28
 * 16 [-| CLK ~B      A/B-12 |-] 27    16 [-| CLK A       A/B-12 |-] 27
 * 17  [| CLK A       NC     |]  26    17  [| VCLK        DO-4   |]  26
 * 18 [-| PO          DO-3   |-] 25    18 [-| Xtal1       DO-3   |-] 25
 * 19  [| SPO         DO-4   |]  24    19  [| Xtal2       DO-2   |]  24
 * 20 [-| DO-2        NC     |-] 23    20 [-| Vss         DO-1   |-] 23
 * 21  [| DO-1        Vss    |]  22    21  [| SPO         TC1-14 |]  22
 *      +--------------------+              +--------------------+
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "pps4.h"


#define VERBOSE 0       //!< set to 1 to log certain instruction conditions

#if VERBOSE
#define LOG(x) logerror x
#else
#define LOG(x)
#endif

const device_type PPS4 = &device_creator<pps4_device>;

pps4_device::pps4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, PPS4, "PPS4", tag, owner, clock, "pps4", __FILE__ )
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 12)  // 4bit RAM
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8)  // 4bit IO
{
}

/**
 * @brief pps4_device::M Return the memory at address B
 * @return ROM/RAM(B)
 */
UINT8 pps4_device::M()
{
	UINT8 ret = m_data->read_byte(m_B & ~m_SAG);
	m_SAG = 0;
	return ret;
}


/**
 * @brief pps4_device::W Write to the memory address at B
 * @return ROM/RAM(B)
 */
void pps4_device::W(UINT8 data)
{
	m_data->write_byte(m_B & ~m_SAG, data);
	m_SAG = 0;
}

offs_t pps4_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( pps4 );
	return CPU_DISASSEMBLE_NAME(pps4)(this, buffer, pc, oprom, opram, options);
}

/**
 * @brief pps4_device::ROP Read the next opcode (instruction)
 * The previous opcode mask (upper four bits) is set from the
 * previous instruction. The new opcode is fetched and the
 * program counter is incremented. The icount is decremented.
 * @return m_I the next opcode
 */
inline UINT8 pps4_device::ROP()
{
	const UINT8 op = m_direct->read_byte(m_P & 0xFFF);
	m_Ip = m_I1;         // save previous opcode
	m_P = (m_P + 1) & 0xFFF;
	m_icount -= 1;
	return op;
}

/**
 * @brief pps4_device::ARG Read the next argument (instruction 2)
 * The byte at program counter is read from the unencrypted
 * direct space. The program count is incremented and the
 * icount is decremented.
 * @return m_I2 the next argument
 */
inline UINT8 pps4_device::ARG()
{
	const UINT8 arg = m_direct->read_byte(m_P & 0xFFF);
	m_P = (m_P + 1) & 0xFFF;
	m_icount -= 1;
	return arg;
}

/**
 * @brief Note3
 *
 * Instructions ADI, LD, EX, EXD, LDI, LB and LBL have a numeric
 * value coded as part of the instruction in the immediate field.
 * This numeric value must be in complementary form on the bus.
 * All of these immediate fields which are inverted are shown
 * in brackets.
 * For example: ADI 1, as written by the programmer who wishes
 * to add one to the value in the accumulator, is converted to
 * 0x6E = 01001 [1110]; the bracketed binary value is the value
 * as seen on the data bus.
 * If the programmer is using the Rockwell Assembler he does not
 * have to manually determine the proper inverted value as the
 * assembler does this for him.
 *
 * [And we do in MAME as well :-]
 */

/**
 * @brief pps4_device::iAD Add
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x0b  0000 1011      1  AD
 *
 * Symbolic equation
 * ----------------------------------
 * C, A <- A + M
 *
 * The result of the binary addition of contents of accumulator
 * and 4-bit contents of RAM currently addressed by B register,
 * replaces the contents of the accumulator. The resulting
 * carry-out is loaded into C flip-flop.
 */
void pps4_device::iAD()
{
	m_A = m_A + M();
	m_C = (m_A >> 4) & 1;
	m_A = m_A & 15;
}

/**
 * @brief pps4_device::iADC Add with carry-in
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x0a  0000 1010      1  ADC
 *
 * Symbolic equation
 * ----------------------------------
 * C, A <- A + M + C
 *
 * Same as AD except the C flip-flop serves as a carry-in
 * to the adder.
 */
void pps4_device::iADC()
{
	m_A = m_A + M() + m_C;
	m_C = m_A >> 4;
	m_A = m_A & 15;
}

/**
 * @brief pps4_device::iADSK Add and skip if carry-out
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x09  0000 1001      1  ADSK
 *
 * Symbolic equation
 * ----------------------------------
 * C, A <- A + M
 * Skip if C = 1
 *
 * Same as AD except the next ROM word will be
 * skipped (ignored) if a carry-out is generated.
 */
void pps4_device::iADSK()
{
	m_A = m_A + M();
	m_C = m_A >> 4;
	m_Skip = m_C;
	m_A = m_A & 15;
}

/**
 * @brief pps4_device::iADCSK Add with carry-in and skip if carry-out
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x08  0000 1000      1  ADCSK
 *
 * Symbolic equation
 * ----------------------------------
 * C, A <- A + M + C
 * Skip if C = 1
 *
 * Same as ADC except the next ROM word will be
 * skipped (ignored) if a carry-out is generated.
 */
void pps4_device::iADCSK()
{
	m_A = m_A + M() + m_C;
	m_C = m_A >> 4;
	m_Skip = m_C;
	m_A = m_A & 15;
}

/**
 * @brief pps4_device::iADI Add immediate
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x6*  0110 xxxx      1  ADI x
 *
 * Symbolic equation
 * ----------------------------------
 * A <- A + [I(4:1)]
 *
 * The result of the binary addition of contents of
 * accumulator and 4-bit immediate field of instruction
 * word replaces the contents of accumulator.
 * The next ROM word will be skipped (ignored) if a
 * carry-out is generated.
 * __ The instruction does not use or change the C flip-flop. __
 * The immediate field I(4:1) of this instruction may not
 * be equal to binary 0 (CYS) or 0101 (DC)
 *
 * See %Note3
 */
void pps4_device::iADI()
{
	const UINT8 imm = ~m_I1 & 15;
	m_A = m_A + imm;
	m_Skip = (m_A >> 4) & 1;
	m_A = m_A & 15;
}

/**
 * @brief pps4_device::iDC Decimal correction
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x65  0110 0101      1  DC
 *
 * Symbolic equation
 * ----------------------------------
 * A <- A + 1010
 *
 * Decimal correction of accumulator.
 * Binary 1010 is added to the contents of the accumulator.
 * Result is stored in accumulator. Instruction does not
 * use or change carry flip-flop or skip.
 */
void pps4_device::iDC()
{
	m_A = m_A + 10;
}

/**
 * @brief pps4_device::iAND Logical AND
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x0d  0000 1101      1  AND
 *
 * Symbolic equation
 * ----------------------------------
 * A <- A & M
 *
 * The result of logical AND of accumulator and
 * 4-bit contents of RAM currently addressed by
 * B register replaces contents of accumulator.
 */
void pps4_device::iAND()
{
	m_A = m_A & M();
}

/**
 * @brief pps4_device::iOR Logical OR
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x0f  0000 1111      1  OR
 *
 * Symbolic equation
 * ----------------------------------
 * A <- A | M
 *
 * The result of logical OR of accumulator and
 * 4-bit contents of RAM currently addressed by
 * B register replaces contents of accumulator.
 */
void pps4_device::iOR()
{
	m_A = m_A | M();
}

/**
 * @brief pps4_device::iEOR Logical exclusive-OR
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x0c  0000 1100      1  EOR
 *
 * Symbolic equation
 * ----------------------------------
 * A <- A ^ M
 *
 * The result of logical exclusive-OR of
 * accumulator and 4-bit contents of RAM
 * currently addressed by B register
 * replaces contents of accumulator.
 */
void pps4_device::iEOR()
{
	m_A = m_A ^ M();
}

/**
 * @brief pps4_device::iCOMP Complement
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x0e  0000 1110      1  COMP
 *
 * Symbolic equation
 * ----------------------------------
 * A <- ~A
 *
 * Each bit of the accumulator is logically
 * complemented and placed in accumulator.
 */
void pps4_device::iCOMP()
{
	m_A = m_A ^ 15;
}

/**
 * @brief pps4_device::iSC Set carry flip-flop
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x20  0010 0000      1  SC
 *
 * Symbolic equation
 * ----------------------------------
 * C <- 1
 *
 * The C flip-flop is set to 1.
 */
void pps4_device::iSC()
{
	m_C = 1;
}

/**
 * @brief pps4_device::iRC Reset carry flip-flop
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x28  0010 0100      1  RC
 *
 * Symbolic equation
 * ----------------------------------
 * C <- 0
 *
 * The C flip-flop is set to 0.
 */
void pps4_device::iRC()
{
	m_C = 0;
}

/**
 * @brief pps4_device::iSF1 Set flip-flop FF1
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x22  0010 0010      1  SF1
 *
 * Symbolic equation
 * ----------------------------------
 * FF1 <- 1
 *
 * The Flip-flop FF1 is set to 1.
 */
void pps4_device::iSF1()
{
	m_FF1 = 1;
}

/**
 * @brief pps4_device::iRF1 Reset flip-flop FF1
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x26  0010 0110      1  RF1
 *
 * Symbolic equation
 * ----------------------------------
 * FF1 <- 0
 *
 * The Flip-flop FF1 is set to 0.
 */
void pps4_device::iRF1()
{
	m_FF1 = 0;
}

/**
 * @brief pps4_device::iSF2 Set flip-flop FF2
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x21  0010 0001      1  SF2
 *
 * Symbolic equation
 * ----------------------------------
 * FF2 <- 1
 *
 * The Flip-flop FF2 is set to 1.
 */
void pps4_device::iSF2()
{
	m_FF2 = 1;
}

/**
 * @brief pps4_device::iRF2 Reset flip-flop FF2
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x25  0010 0101      1  RF2
 *
 * Symbolic equation
 * ----------------------------------
 * FF2 <- 0
 *
 * The flip-flop FF2 is set to 0.
 */
void pps4_device::iRF2()
{
	m_FF2 = 0;
}

/**
 * @brief pps4_device::iLD Load accumulator from memory
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x30+ 0011 0xxx      1  LD x
 *
 * Symbolic equation
 * ----------------------------------
 * A <- M
 * B(7:5) <- B(7:5) ^ [I(3:1)]
 *
 * The 4-bit contents of RAM currently addressed
 * by B register are placed in the accumulator.
 * The RAM address in the B register is then
 * modified by the result of an exclusive-OR of
 * the 3-b it immediate field I(3:1) and B(7:5)
 *
 * See %Note3
 */
void pps4_device::iLD()
{
	const UINT16 i3c = ~m_I1 & 7;
	m_A = M();
	m_B = m_B ^ (i3c << 4);
}

/**
 * @brief pps4_device::iEX Exchange accumulator and memory
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x38+ 0011 1xxx      1  EX x
 *
 * Symbolic equation
 * ----------------------------------
 * A <-> M
 * B(7:5) <- B(7:5) ^ [I(3:1)]
 *
 * The same as LD except the contents of accumulator
 * are also placed in currently addressed RAM location.
 *
 * See %Note3
 */
void pps4_device::iEX()
{
	const UINT16 i3c = ~m_I1 & 7;
	const UINT8 mem = M();
	W(m_A);
	m_A = mem;
	m_B = m_B ^ (i3c << 4);
}

/**
 * @brief pps4_device::iEXD Exchange accumulator and memory and decrement BL
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x28+ 0010 1xxx      1  EXD x
 *
 * Symbolic equation
 * ----------------------------------
 * A <-> M
 * B(7:5) <- B(7:5) ^ [I(3:1)]
 * BL <- BL - 1
 * Skip on BL = 1111b
 *
 * The same as EX except RAM address in B register
 * is further modified by decrementing BL by 1.
 * If the new contents of BL is 1111, the next
 * ROM word will be ignored.
 *
 * See %Note3
 */
void pps4_device::iEXD()
{
	const UINT8 i3c = ~m_I1 & 7;
	const UINT8 mem = M();
	UINT8 bl = m_B & 15;
	W(m_A);
	m_A = mem;
	m_B = m_B ^ (i3c << 4);
	// if decrement BL wraps to 1111b
	if (0 == bl) {
		bl = 15;
		m_Skip = 1;
	} else {
		bl = bl - 1;
	}
	m_B = (m_B & ~15) | bl;
}

/**
 * @brief pps4_device::iLDI Load accumualtor immediate
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x7*  0111 xxxx      1  LDI x
 *
 * Symbolic equation
 * ----------------------------------
 * A <- [I(4:1)]
 *
 * The 4-bit contents, immediate field I(4:1),
 * of the instruction are placed in the accumulator.
 *
 * Note: Only the first occurrence of an LDI in a consecutive
 * string of LDIs will be executed. The program will ignore
 * remaining LDIs and execute next valid instruction.
 *
 * See %Note3
 */
void pps4_device::iLDI()
{
	// previous LDI instruction?
	if (0x70 == (m_Ip & 0xf0)) {
		LOG(("%s: skip prev:%02x op:%02x\n", __FUNCTION__, m_Ip, m_I1));
		return;
	}
	m_A = ~m_I1 & 15;
}

/**
 * @brief pps4_device::iLAX
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x12  0001 0010      1  LAX
 *
 * Symbolic equation
 * ----------------------------------
 * A <- X
 *
 * The 4-bit contents of the X register are
 * placed in the accumulator.
 */
void pps4_device::iLAX()
{
	m_A = m_X;
}

/**
 * @brief pps4_device::iLXA
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x1b  0001 1011      1  LXA
 *
 * Symbolic equation
 * ----------------------------------
 * X <- A
 *
 * The contents of the accumulator are
 * tansferred to the X register.
 */
void pps4_device::iLXA()
{
	m_X = m_A;
}

/**
 * @brief pps4_device::iLABL
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x11  0001 0001      1  LABL
 *
 * Symbolic equation
 * ----------------------------------
 * A <- BL
 *
 * The contents of BL register are
 * tansferred to the accumulator.
 */
void pps4_device::iLABL()
{
	m_A = m_B & 15;
}

/**
 * @brief pps4_device::iLBMX
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x10  0001 0000      1  LBMX
 *
 * Symbolic equation
 * ----------------------------------
 * BM <- X
 *
 * The contents of X register are
 * tansferred to BM register.
 */
void pps4_device::iLBMX()
{
	m_B = (m_B & ~(15 << 4)) | (m_X << 4);
}

/**
 * @brief pps4_device::iLBUA
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x08  0000 0100      1  LBUA
 *
 * Symbolic equation
 * ----------------------------------
 * BU <- A
 * A <- M
 *
 * The contents of accumulator are tansferred to
 * BU register. Also, the contents of the currently
 * addressed RAM are transferred to accumulator.
 */
void pps4_device::iLBUA()
{
	m_B = (m_B & ~(15 << 8)) | (m_A << 8);
	m_A = M();
}

/**
 * @brief pps4_device::iXABL
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x19  0001 1001      1  XABL
 *
 * Symbolic equation
 * ----------------------------------
 * A <-> BL
 *
 * The contents of accumulator and BL register
 * are exchanged.
 */
void pps4_device::iXABL()
{
	// swap A and BL
	UINT8 bl = m_B & 15;
	m_B = (m_B & ~15) | m_A;
	m_A = bl;
}

/**
 * @brief pps4_device::iXMBX
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x18  0001 1000      1  XMBX
 *
 * Symbolic equation
 * ----------------------------------
 * X <-> BM
 *
 * The contents of accumulator and BL register
 * are exchanged.
 */
void pps4_device::iXBMX()
{
	// swap X and BM
	const UINT8 bm = (m_B >> 4) & 15;
	m_B = (m_B & ~(15 << 4)) | (m_X << 4);
	m_X = bm;
}

/**
 * @brief pps4_device::iXAX
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x1a  0001 1010      1  XAX
 *
 * Symbolic equation
 * ----------------------------------
 * A <-> X
 *
 * The contents of accumulator and X register
 * are exchanged.
 */
void pps4_device::iXAX()
{
	// swap A and X
	m_A ^= m_X;
	m_X ^= m_A;
	m_A ^= m_X;
}

/**
 * @brief pps4_device::iXS
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x06  0000 0110      1  XS
 *
 * Symbolic equation
 * ----------------------------------
 * SA <-> SB
 *
 * The 12-bit contents of SA and SB register
 * are exchanged.
 */
void pps4_device::iXS()
{
	// swap SA and SB
	m_SA ^= m_SB;
	m_SB ^= m_SA;
	m_SA ^= m_SB;
}

/**
 * @brief pps4_device::iCYS
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x6f  0110 1111      1   CYS
 *
 * Symbolic equation
 * ----------------------------------
 * A <- SA(4:1)
 * SA(4:1) <- SA(8:5)
 * SA(8:5) <- SA(12:9)
 * SA(12:9) <- A
 *
 * A 4-bit right shift of the SA register takes place
 * with the four bits which are shifted off the end
 * of SA being transferred into the accumulator.
 * The contents of the accumulator are placed in the
 * left end of the SA register
 *
 */
void pps4_device::iCYS()
{
	const UINT16 sa = (m_SA >> 4) | (m_A << 8);
	m_A = m_SA & 15;
	m_SA = sa;
}

/**
 * @brief pps4_device::iLB Load B indirect
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0xc*  1100 xxxx      2   LB x
 *
 * Symbolic equation
 * ----------------------------------
 * SB <- SA, SA <- P
 * P(12:5) <- 0000 1100
 * P(4:1) <- I(4:1)
 *
 * BU <- 0000
 * B(8:1) <- [I2(8:1)]
 * P <- SA, SA <-> SB
 *
 * Sixteen consecutive locations on ROM page 3 (I2) contain
 * data which can be loaded into the eight least significant
 * bits of the B register by use of any LB instruction.
 * The four most significant bits of B register will be loaded
 * with zeros. The contents of the SB register will be destroyed.
 * This instruction takes two cycles to execute but occupies
 * only one ROM word. (Automatic return)
 *
 * Only the first occurrence of an LB or LBL instruction in a
 * consecutive string of LB or LBL will be executed. The
 * program will ignore the remaining LB or LBL and execute
 * the next valid instruction. Within subroutines the LB
 * instruction must be used with caution because the contents
 * of SB have been modified.
 *
 * See %Note3 and %Note4
 */
void pps4_device::iLB()
{
	// previous LB or LBL instruction?
	if (0xc0 == (m_Ip & 0xf0) || 0x00 == m_Ip) {
		LOG(("%s: skip prev:%02x op:%02x\n", __FUNCTION__, m_Ip, m_I1));
		return;
	}
	m_SB = m_SA;
	m_SA = (m_P + 1) & 0xFFF;
	m_P = (3 << 6) | (m_I1 & 15);
	m_B = ~ARG() & 255;
	m_P = m_SA;
	// swap SA and SB
	m_SA ^= m_SB;
	m_SB ^= m_SA;
	m_SA ^= m_SB;
}

/**
 * @brief pps4_device::iLBL Load B long
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x00  0000 0000      2   LBL
 *
 * Symbolic equation
 * ----------------------------------
 * BU <- 0000
 * B(8:1) <- [I2(8:1)]
 *
 * This instruction occupies two ROM words, the second of
 * which will be loaded into the eight least significant
 * bits of the B register. The four most significant bits
 * of B (BU) will be loaded with zeroes.
 *
 * Only the first occurrence of an LB or LBL instruction in a
 * consecutive string of LB or LBL will be executed. The
 * program will ignore the remaining LB or LBL and execute
 * the next valid instruction.
 *
 * See %Note3
 */
void pps4_device::iLBL()
{
	m_I2 = ARG();
	// previous LB or LBL instruction?
	if (0xc0 == (m_Ip & 0xf0) || 0x00 == m_Ip) {
		LOG(("%s: skip prev:%02x op:%02x\n", __FUNCTION__, m_Ip, m_I1));
		return;
	}
	m_B = ~m_I2 & 255;  // Note: immediate is 1's complement
}

/**
 * @brief pps4_device::INCB Increment B lower, skip if 0000
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x17  0001 0111      1   INCB
 *
 * Symbolic equation
 * ----------------------------------
 * BL <- BL + 1
 * Skip on BL = 0000
 *
 * BL register (least significant four bits of B register)
 * is incremented by 1. If the new contents of BL is 0000b,
 * then the next ROM word will be ignored.
 */
void pps4_device::iINCB()
{
	UINT8 bl = m_B & 15;
	bl = (bl + 1) & 15;
	if (0 == bl) {
		LOG(("%s: skip BL=%x\n", __FUNCTION__, bl));
		m_Skip = 1;
	}
	m_B = (m_B & ~15) | bl;
}

/**
 * @brief pps4_device::iDECB Decrement B lower, skip if 1111
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x1f  0001 1111      1   DECB
 *
 * Symbolic equation
 * ----------------------------------
 * BL <- BL - 1
 * Skip on BL = 1111
 *
 * BL register is decremented by 1. If the new
 * contents of BL is 1111b, then the next ROM
 * word will be ignored.
 */
void pps4_device::iDECB()
{
	UINT8 bl = m_B & 15;
	bl = (bl - 1) & 15;
	if (15 == bl) {
		LOG(("%s: skip BL=%x\n", __FUNCTION__, bl));
		m_Skip = 1;
	}
	m_B = (m_B & ~15) | bl;
}

/**
 * @brief pps4_device::iT Transfer
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x80+ 10xx xxxx      1   T *xx
 *
 * Symbolic equation
 * ----------------------------------
 * P(6:1) <- I(6:1)
 *
 * An unconditional transfer to a ROM word on the current
 * page takes place. The least significant 6-bits of P
 * register P(6:1) are replaced by six bit immediate
 * field I(6:1)
 */
void pps4_device::iT()
{
	const UINT16 p = (m_P & ~63) | (m_I1 & 63);
	LOG(("%s: P=%03x I=%02x -> P=%03x\n", __FUNCTION__, m_P, m_I1, p));
	m_P = p;
}

/**
 * @brief pps4_device::iTM Transfer and mark indirect
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0xc0+ 11xx xxxx      2   TM x
 *       yyyy yyyy  from page 3
 *
 * Symbolic equation
 * ----------------------------------
 * SB <- SA, SA <- P
 * P(12:7) <- 000011
 * P(6:1) <- I1(6:1)
 *
 * P(12:9) <- 0001
 * P(8:1) <- I2(8:1)
 *
 * 48 consecutive locations on ROM page 3 contains pointer data
 * which indentify subroutine entry addresses. These subroutine
 * entry addresses are limited to pages 4 through 7. This TM
 * instruction will save the address of the next ROM word in
 * the SA register after loading the original contents of SA
 * into SB. A transfer then occurs to one of the subroutine
 * entry addresses. This instruction occupies one ROM word
 * but takes two cycles for execution.
 */
void pps4_device::iTM()
{
	m_SB = m_SA;
	m_SA = m_P;
	m_P = (3 << 6) | (m_I1 & 63);
	m_I2 = ARG();
	m_P = (1 << 8) | m_I2;
}

/**
 * @brief pps4_device::iTL Transfer long
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x5x  0101 xxxx      2   TL xyy
 *       yyyy yyyy
 *
 * Symbolic equation
 * ----------------------------------
 * P(12:9) <- I1(4:1)
 * P(8:1) <- I2(8:1)
 *
 * The instruction executes a transfer to any ROM word on any
 * page. It occupies two ROM words an requires two cycles for
 * execution. The first byte loads P(12:9) with field I1(4:1)
 * and then the second byte I2(8:1) is placed in P(8:1).
 */
void pps4_device::iTL()
{
	m_I2 = ARG();
	m_P = ((m_I1 & 15) << 8) | m_I2;
}

/**
 * @brief pps4_device::iTML Transfer and mark long
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x0*  0000 xxxx      2   TML xyy
 *       yyyy yyyy
 *
 * Symbolic equation
 * ----------------------------------
 * SB <- SA, SA <- P
 * P(12:9) <- I1(4:1)
 * P(8:1) <- I2(8:1)
 *
 * Note I1(2:1) != 00
 *
 * This instruction executes a transfer and mark to any
 * location on ROM pages 4 through 15. It occupies two
 * ROM words and requires two cycle times for execution.
 */
void pps4_device::iTML()
{
	m_I2 = ARG();
	m_SB = m_SA;
	m_SA = m_P;
	m_P = ((m_I1 & 15) << 8) | m_I2;
}

/**
 * @brief pps4_device::iSKC Skip on carry flip-flop
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x15  0001 0101      1   SKC
 *
 * Symbolic equation
 * ----------------------------------
 * Skip if C = 1
 *
 * The next ROM word will be ignored if C flip-flop is 1.
 */
void pps4_device::iSKC()
{
	m_Skip = m_C;
}

/**
 * @brief pps4_device::iSKC Skip on carry flip-flop
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x1e  0001 1110      1   SKZ
 *
 * Symbolic equation
 * ----------------------------------
 * Skip if A = 0
 *
 * The next ROM word will be ignored if C flip-flop is 1.
 */
void pps4_device::iSKZ()
{
	m_Skip = (0 == m_A) ? 1 : 0;
}

/**
 * @brief pps4_device::iSKBI Skip if BL equal to immediate
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x4*  0100 xxxx      1   SKBI x
 *
 * Symbolic equation
 * ----------------------------------
 * Skip if BL = I(4:1)
 *
 * The next ROM word will be ignored if the least significant
 * four bits of B register (BL) is equal to the 4-bit immediate
 * field I(4:1) of instruction.
 */
void pps4_device::iSKBI()
{
	const UINT8 i4 = m_I1 & 15;
	const UINT8 bl = m_B & 15;
	m_Skip = bl == i4 ? 1 : 0;
}

/**
 * @brief pps4_device::iSKF1 Skip if FF1 equals 1
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x16  0001 0110      1   SKF1
 *
 * Symbolic equation
 * ----------------------------------
 * Skip if FF1 = 1
 */
void pps4_device::iSKF1()
{
	m_Skip = m_FF1;
}

/**
 * @brief pps4_device::iSKF2 Skip if FF2 equals 1
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x14  0001 0100      1   SKF2
 *
 * Symbolic equation
 * ----------------------------------
 * Skip if FF2 = 1
 */
void pps4_device::iSKF2()
{
	m_Skip = m_FF2;
}

/**
 * @brief pps4_device::iRTN Return
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x05  0000 0101      1   RTN
 *
 * Symbolic equation
 * ----------------------------------
 * P <- SA, SA <-> SB
 *
 * This instruction executes a return from subroutine
 * by loading contents of SA register into P register
 * and interchanges the SB and SA registers.
 */
void pps4_device::iRTN()
{
	m_P = m_SA & 0xFFF;
	// swap SA and SB
	m_SA ^= m_SB;
	m_SB ^= m_SA;
	m_SA ^= m_SB;
}

/**
 * @brief pps4_device::iRTNSK Return and skip
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x07  0000 0111      1   RTNSK
 *
 * Symbolic equation
 * ----------------------------------
 * P <- SA, SA <-> SB
 * P <- P + 1
 *
 * Same as RTN except the first ROM word encountered
 * after the return from subroutine is skipped.
 */
void pps4_device::iRTNSK()
{
	m_P = m_SA & 0xFFF;
	// swap SA and SB
	m_SA ^= m_SB;
	m_SB ^= m_SA;
	m_SA ^= m_SB;
	m_Skip = 1; // next opcode is ignored
}

/**
 * @brief pps4_device::IOL Input / Output Long
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x1c  0001 1100      2   IOL yy
 *       yyyy yyyy
 *
 * Symbolic equation
 * ----------------------------------
 * ~A -> Data Bus
 * A <- ~Data Bus
 * I2 -> I/O device
 *
 * This instruction occupies two ROM words and requires two
 * cycles for execution. The first ROM word is received by
 * the CPU and sets up the I/O enable signal. The second
 * ROM word is then received by the I/O devices and decoded
 * for address and command. The contents of the accumulator
 * inverted are placed on the data lines for acceptance by
 * the I/O. At the same time, input data received by the I/O
 * device is transferred to the accumulator inverted.
 *
 * FIXME: Is BL on the I/D:8-5 lines during the I/O cycle?
 * The ROM, RAM, I/O chips A17xx suggest this, because they
 * expect the value of BL to address one of the sixteen
 * input/output lines.
 */
void pps4_device::iIOL()
{
	UINT8 ac = ((m_B & 15) << 4) | (~m_A & 15);
	m_I2 = ARG();
	m_io->write_byte(m_I2, ac);
	LOG(("%s: port:%02x <- %x\n", __FUNCTION__, m_I2, ac));
	ac = m_io->read_byte(m_I2) & 15;
	LOG(("%s: port:%02x -> %x\n", __FUNCTION__, m_I2, ac));
	m_A = ~ac & 15;
}

/**
 * @brief pps4_device::iDIA Discrete input group A
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x27  0010 0111      1   DIA
 *
 * Symbolic equation
 * ----------------------------------
 * A <- DIA
 *
 * Data at the inputs to discrete group A is
 * transferred to the accumulator.
 */
void pps4_device::iDIA()
{
	m_A = m_io->read_byte(PPS4_PORT_A) & 15;
}

/**
 * @brief pps4_device::iDIB Discrete input group B
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x23  0010 0011      1   DIB
 *
 * Symbolic equation
 * ----------------------------------
 * A <- DIB
 *
 * Data at the inputs to discrete group B is
 * transferred to the accumulator.
 */
void pps4_device::iDIB()
{
	m_A = m_io->read_byte(PPS4_PORT_B) & 15;
}

/**
 * @brief pps4_device::iDOA Discrete output
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x1d  0001 1101      1   DOA
 *
 * Symbolic equation
 * ----------------------------------
 * DOA <- A
 *
 * The contents of the accumulator are transferred
 * to the discrete output register.
 */
void pps4_device::iDOA()
{
	m_io->write_byte(PPS4_PORT_A, m_A);
}

/**
 * @brief pps4_device::iSAG Special address generation
 * HEX   BINARY    CYCLES  MNEMONIC
 * ----------------------------------
 * 0x2d  0001 0011      1   SAG
 *
 * Symbolic equation
 * ----------------------------------
 * A/B Bus (12:5) <- 0000 0000
 * A/B Bus (4:1) <- BL(4:1)
 * Contents of B remains unchanged
 *
 * The instruction causes the eight most significant bits
 * of the RAM address output to be zeroed during the next
 * cycle only. Note that this instruction does not alter
 * the contents of the B register.
 */
void pps4_device::iSAG()
{
	// mask bits 12:5 on next memory access
	m_SAG = 0xff0;
}

/***************************************************************************
    COMMON EXECUTION
***************************************************************************/
void pps4_device::execute_one()
{
	m_I1 = ROP();
	if (m_Skip) {
		m_Skip = 0;
		LOG(("%s: skip op:%02x\n", __FUNCTION__, m_I1));
		return;
	}
	switch (m_I1) {
	case 0x00:
		iLBL();
		break;
	case 0x01:
		iTML();
		break;
	case 0x02:
		iTML();
		break;
	case 0x03:
		iTML();
		break;
	case 0x04:
		iLBUA();
		break;
	case 0x05:
		iRTN();
		break;
	case 0x06:
		iXS();
		break;
	case 0x07:
		iRTNSK();
		break;
	case 0x08:
		iADCSK();
		break;
	case 0x09:
		iADSK();
		break;
	case 0x0a:
		iADC();
		break;
	case 0x0b:
		iAD();
		break;
	case 0x0c:
		iEOR();
		break;
	case 0x0d:
		iAND();
		break;
	case 0x0e:
		iCOMP();
		break;
	case 0x0f:
		iOR();
		break;

	case 0x10:
		iLBMX();
		break;
	case 0x11:
		iLABL();
		break;
	case 0x12:
		iLAX();
		break;
	case 0x13:
		iSAG();
		break;
	case 0x14:
		iSKF2();
		break;
	case 0x15:
		iSKC();
		break;
	case 0x16:
		iSKF1();
		break;
	case 0x17:
		iINCB();
		break;
	case 0x18:
		iXBMX();
		break;
	case 0x19:
		iXABL();
		break;
	case 0x1a:
		iXAX();
		break;
	case 0x1b:
		iLXA();
		break;
	case 0x1c:
		iIOL();
		break;
	case 0x1d:
		iDOA();
		break;
	case 0x1e:
		iSKZ();
		break;
	case 0x1f:
		iDECB();
		break;

	case 0x20:
		iSC();
		break;
	case 0x21:
		iSF2();
		break;
	case 0x22:
		iSF1();
		break;
	case 0x23:
		iDIB();
		break;
	case 0x24:
		iRC();
		break;
	case 0x25:
		iRF2();
		break;
	case 0x26:
		iRF1();
		break;
	case 0x27:
		iDIA();
		break;

	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		iEXD();
		break;

	case 0x30: case 0x31: case 0x32: case 0x33:
	case 0x34: case 0x35: case 0x36: case 0x37:
		iLD();
		break;

	case 0x38: case 0x39: case 0x3a: case 0x3b:
	case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		iEX();
		break;

	case 0x40: case 0x41: case 0x42: case 0x43:
	case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b:
	case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		iSKBI();
		break;

	case 0x50: case 0x51: case 0x52: case 0x53:
	case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b:
	case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		iTL();
		break;

	case 0x65:
		iDC();
		break;

	case 0x60: case 0x61: case 0x62: case 0x63:
	case 0x64:            case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b:
	case 0x6c: case 0x6d: case 0x6e:
		iADI();
		break;

	case 0x6f:
		iCYS();
		break;

	case 0x70: case 0x71: case 0x72: case 0x73:
	case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b:
	case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		iLDI();
		break;

	case 0x80: case 0x81: case 0x82: case 0x83:
	case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b:
	case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0x90: case 0x91: case 0x92: case 0x93:
	case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b:
	case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3:
	case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab:
	case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
	case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb:
	case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		iT();
		break;


	case 0xc0: case 0xc1: case 0xc2: case 0xc3:
	case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb:
	case 0xcc: case 0xcd: case 0xce: case 0xcf:
		iLB();
		break;

	default:
		iTM();
	}
}

void pps4_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, m_P);
		execute_one();

	} while (m_icount > 0);
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void pps4_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	save_item(NAME(m_A));
	save_item(NAME(m_X));
	save_item(NAME(m_P));
	save_item(NAME(m_SA));
	save_item(NAME(m_SB));
	save_item(NAME(m_Skip));
	save_item(NAME(m_SAG));
	save_item(NAME(m_B));
	save_item(NAME(m_C));
	save_item(NAME(m_FF1));
	save_item(NAME(m_FF2));
	save_item(NAME(m_I1));
	save_item(NAME(m_I2));
	save_item(NAME(m_Ip));

	state_add( PPS4_PC, "PC", m_P ).mask(0xFFF).formatstr("%03X");
	state_add( PPS4_A, "A",  m_A ).formatstr("%01X");
	state_add( PPS4_X, "X",  m_X ).formatstr("%01X");
	state_add( PPS4_SA, "SA", m_SA ).formatstr("%03X");
	state_add( PPS4_SB, "SB", m_SB ).formatstr("%03X");
	state_add( PPS4_Skip, "Skip",  m_Skip ).formatstr("%01X");
	state_add( PPS4_SAG, "SAG",  m_SAG ).formatstr("%03X");
	state_add( PPS4_B, "B",  m_B ).formatstr("%03X");
	state_add( PPS4_I1, "I1",  m_I1 ).formatstr("%02X").noshow();
	state_add( PPS4_I2, "I2",  m_I2 ).formatstr("%02X").noshow();
	state_add( PPS4_Ip, "Ip",  m_Ip ).formatstr("%02X").noshow();
	state_add( STATE_GENPC,    "GENPC", m_P ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_C).formatstr("%3s").noshow();

	m_icountptr = &m_icount;
}

void pps4_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c",
				m_C ? 'C':'.',
				m_FF1 ? '1':'.',
				m_FF2 ? '2':'.');
			break;
	}
}

/***************************************************************************
    COMMON RESET
***************************************************************************/

void pps4_device::device_reset()
{
	m_A = 0;        // Accumulator A(4:1)
	m_X = 0;        // X register X(4:1)
	m_P = 0;        // program counter P(12:1)
	m_SA = 0;       // Shift register SA(12:1)
	m_SB = 0;       // Shift register SB(12:1)
	m_SAG = 0;      // Special address generation mask
	m_B = 0;        // B address register B(12:1) (BL, BM and BU)
	m_C = 0;        // Carry flip-flop
	m_FF1 = 0;      // Flip-flop 1
	m_FF2 = 0;      // Flip-flop 2
	m_I1 = 0;        // Most recent instruction I(8:1)
	m_I2 = 0;       // Most recent parameter I2(8:1)
	m_Ip = 0;       // Previous instruction I(8:1)
}
