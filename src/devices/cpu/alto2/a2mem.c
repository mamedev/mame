// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII memory interface
 *
 *****************************************************************************/
#include "alto2cpu.h"

#define PUT_EVEN(dword,word)            X_WRBITS(dword,32, 0,15,word)
#define GET_EVEN(dword)                 X_RDBITS(dword,32, 0,15)
#define PUT_ODD(dword,word)             X_WRBITS(dword,32,16,31,word)
#define GET_ODD(dword)                  X_RDBITS(dword,32,16,31)

#define GET_MESR_HAMMING(mesr)          X_RDBITS(mesr,16,0,5)
#define PUT_MESR_HAMMING(mesr,val)      X_WRBITS(mesr,16,0,5,val)
#define GET_MESR_PERR(mesr)             X_RDBITS(mesr,16,6,6)
#define PUT_MESR_PERR(mesr,val)         X_WRBITS(mesr,16,6,6,val)
#define GET_MESR_PARITY(mesr)           X_RDBITS(mesr,16,7,7)
#define PUT_MESR_PARITY(mesr,val)       X_WRBITS(mesr,16,7,7,val)
#define GET_MESR_SYNDROME(mesr)         X_RDBITS(mesr,16,8,13)
#define PUT_MESR_SYNDROME(mesr,val)     X_WRBITS(mesr,16,8,13,val)
#define GET_MESR_BANK(mesr)             X_RDBITS(mesr,16,14,15)
#define PUT_MESR_BANK(mesr,val)         X_WRBITS(mesr,16,14,15,val)

#define GET_MECR_SPARE1(mecr,val)       X_RDBITS(mecr,16,0,3)
#define PUT_MECR_SPARE1(mecr,val)       X_WRBITS(mecr,16,0,3,val)
#define GET_MECR_TEST_CODE(mecr)        X_RDBITS(mecr,16,4,10)
#define PUT_MECR_TEST_CODE(mecr,val)    X_WRBITS(mecr,16,4,10,val)
#define GET_MECR_TEST_MODE(mecr)        X_RDBITS(mecr,16,11,11)
#define PUT_MECR_TEST_MODE(mecr,val)    X_WRBITS(mecr,16,11,11,val)
#define GET_MECR_INT_SBERR(mecr)        X_RDBITS(mecr,16,12,12)
#define PUT_MECR_INT_SBERR(mecr,val)    X_WRBITS(mecr,16,12,12,val)
#define GET_MECR_INT_DBERR(mecr)        X_RDBITS(mecr,16,13,13)
#define PUT_MECR_INT_DBERR(mecr,val)    X_WRBITS(mecr,16,13,13,val)
#define GET_MECR_ERRCORR(mecr)          X_RDBITS(mecr,16,14,14)
#define PUT_MECR_ERRCORR(mecr,val)      X_WRBITS(mecr,16,14,14,val)
#define GET_MECR_SPARE2(mecr)           X_RDBITS(mecr,16,15,15)
#define PUT_MECR_SPARE2(mecr,val)       X_WRBITS(mecr,16,15,15,val)

/**
 * <PRE>
 * AltoII Memory
 *
 * Address mapping
 *
 * The mapping of addresses to memory chips can be altered by the setting of
 * the "memory configuration switch". This switch is located at the top of the
 * backplane of the AltoII. If the switch is in the alternate position, the
 * first and second 32K portions of memory are exchanged.
 *
 * The AltoII memory system is organized around 32-bit doublewords. Stored
 * along with each doubleword is 6 bits of Hamming code and a Parity bit for
 * a total of 39 bits:
 *
 *  bits 0-15   even data word
 *  bits 16-31  odd data word
 *  bits 32-37  Hamming code
 *  bit 38      Parity bit
 *
 * Things are further complicated by the fact that two types of memory chips
 * are used: 16K chips in machines with extended memory and 4K chips for all
 * others.
 *
 * The bits in a 1-word deep slice of memory are called a group. A group
 * contains 4K oder 16K doublewords, depending on the chip type. The bits of
 * a group on a single board are called a subgroup. Thus a subgroup contains
 * 10 of the 40 bits in a group. There are 8 subgroups on a memory board.
 * Subgroups are numbered from the high 3 bits of the address; for 4K chips
 * this means MAR[0-2]; for 16K chips (i.e., an Alto with extended memory)
 * this means BANK,MAR[0]:
 *
 *  Subgroup    Chip Positions
 *     7          81-90
 *     6          71-80
 *     5          61-70
 *     4          51-60
 *     3          41-50
 *     2          31-40
 *     1          21-30
 *     0          11-20
 *
 * The location of the bits in group 0 is:
 *
 *  CARD 1          CARD2           CARD3           CARD4
 *  32 24 16 08 00  33 25 17 09 01  34 26 18 10 02  35 27 19 11 03
 *  36 28 20 12 04  37 29 21 13 05  38 30 22 14 06  xx 31 23 25 07
 *
 * Chips 15, 25, 35, 45, 65, 75 and 85 on board 4 aren't used. If you are out
 * of replacement memory chips, you can use one of these, but then the board
 * with the missing chips will only work in Slot 4.
 *
 *  o  WORD = 16 BITS
 *  o  ACCESS -> 2 WORDS AT A TIME
 *  o  -> 32 BITS + 6 BITS EC + PARITY + SPARE = 40 BITS
 *  o  10 BITS/MODULE    80 DRAMS/MODULE
 *  o  4 MODULES/ALTO   320 DRAMS/ALTO
 *
 *  ADDRESS A0-6, WE, CAS'
 *      | TO ALL DEVICES
 *      v
 *      +-----------------------------------------+
 *      | ^ 8 DEVICES (32K OR 128K FOR XM)        |
 *      | |                                       | CARD 1
 *     /| v  <------------ DATA OUT ---------->   |
 *    / |  0   1   2   3   4   5   6   7   8   9  |
 *   /  +-----------------------------------------+
 *  |      H4  H0  28  24  20  16  12  8   4   0
 *  |
 *  |   +-----------------------------------------+
 *  |  /|                                         | CARD 2
 *  | / +-----------------------------------------+
 * RAS     H5  H1  29  25  21  17  13  9   5   1
 * 0-7
 *  | \ +-----------------------------------------+
 *  |  \|                                         | CARD 3
 *  |   +-----------------------------------------+
 *  |      P   H2  30  26  22  18  14  10  6   2
 *   \
 *    \ +-----------------------------------------+
 *     \|                                         | CARD 4
 *      +-----------------------------------------+
 *         X   H3  31  27  23  19  15  11  7   3
 *
 *                 [  ODD WORD  ]  [ EVEN WORD ]
 *
 * <HR>
 *
 *      32K x 10 STORAGE MODULE
 *
 *      Table I
 *
 *  +-------+-------+-------+---------------+-------+
 *  |CIRCUIT| INPUT | SIGNAL| INVERTER      |       |
 *  |  NO.  | PINS  | NAME  | DEF?? ???     |RESIST.|
 *  +-------+-------+-------+---------------+-------+
 *  |       |   71  | RAS0  | A1  1 ->  2   | ?? R2 |
 *  |   1   +-------+-------+---------------+-------+
 *  |       |  110  | CS0   | A1  3 ->  4   | ?? R3 |
 *  +-------+-------+-------+---------------+-------+
 *  |       |   79  | RAS1  | A2  1 ->  2   | ?? R4 |
 *  |   2   +-------+-------+---------------+-------+
 *  |       |  110  | CS1   | A2  3 ->  4   | ?? R5 |
 *  +-------+-------+-------+---------------+-------+
 *  |       |   90  | RAS2  | A3  1 ->  2   | ?? R7 |
 *  |   3   +-------+-------+---------------+-------+
 *  |       |  110  | CS2   | A3  3 ->  4   | ?? R8 |
 *  +-------+-------+-------+---------------+-------+
 *  |       |   86  | RAS3  | A3 11 -> 10   | ?? R9 |
 *  |   4   +-------+-------+---------------+-------+
 *  |       |  110  | CS3   | A4 11 -> 10   | ?? R7 |
 *  +-------+-------+-------+---------------+-------+
 *  |       |  102  | RAS4  | A4  1 ->  2   | ?? R4 |
 *  |   5   +-------+-------+---------------+-------+
 *  |       |  110  | CS4   | A3 13 -> 12   | ?? R5 |
 *  +-------+-------+-------+---------------+-------+
 *  |       |  106  | RAS5  | A5 11 -> 10   | ?? R3 |
 *  |   6   +-------+-------+---------------+-------+
 *  |       |  110  | CS5   | A5  3 ->  4   | ?? R2 |
 *  +-------+-------+-------+---------------+-------+
 *  |       |  111  | RAS6  | A5  1 ->  2   | ?? R8 |
 *  |   7   +-------+-------+---------------+-------+
 *  |       |  110  | CS6   | A5 13 -> 12   | ?? R9 |
 *  +-------+-------+-------+---------------+-------+
 *  |       |   99  | RAS7  | A4 13 -> 12   | ?? R5 |
 *  |   8   +-------+-------+---------------+-------+
 *  |       |  110  | CS7   | A4  3 ->  4   | ?? R5 |
 *  +-------+-------+-------+---------------+-------+
 *
 *      Table II
 *
 *      MEMORY CHIP REFERENCE DESIGNATOR
 *
 *               CIRCUIT NO.
 *       ROW NO.    1       2       3       4       5       6       7       8
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *      |   1   | 15 20 | 25 30 | 35 40 | 45 50 | 55 60 | 65 70 | 75 80 | 85 90 |
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *      |   2   | 14 19 | 24 29 | 34 39 | 44 49 | 54 59 | 64 69 | 64 79 | 84 89 |
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *      |   3   | 13 18 | 23 28 | 33 38 | 43 48 | 53 58 | 63 68 | 73 78 | 83 88 |
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *      |   4   | 12 17 | 22 27 | 32 37 | 42 47 | 52 57 | 62 67 | 72 77 | 82 87 |
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *      |   5   | 11 16 | 21 26 | 31 36 | 41 46 | 52 56 | 61 66 | 71 76 | 81 86 |
 *      +-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *
 *
 * The Hamming code generator:
 *
 * WDxx is write data bit xx.
 * H(x) is Hammming code bit x.
 * HC(x) is generated Hamming code bit x.
 * HC(x/y) is an intermediate value.
 * HC(x)A and HC(x)B are also intermediate values.
 *
 * Chips used are:
 * 74S280 9-bit parity generator (A-I inputs, even and odd outputs)
 * 74S135 EX-OR/EX-NOR gates (5 inputs, 2 outputs)
 * 74S86  EX-OR gates (2 inputs, 1 output)
 *
 * chip A      B      C      D      E      F      G      H      I     even    odd
 * ---------------------------------------------------------------------------------
 * a75: WD01   WD04   WD08   WD11   WD15   WD19   WD23   WD26   WD30  ---     HC(0)A
 * a76: WD00   WD03   WD06   WD10   WD13   WD17   WD21   WD25   WD28  HC(0B1) ---
 * a86: WD02   WD05   WD09   WD12   WD16   WD20   WD24   WD27   WD31  HC(1)A  ---
 * a64: WD01   WD02   WD03   WD07   WD08   WD09   WD10   WD14   WD15  ---     HC(2)A
 * a85: WD16   WD17   WD22   WD23   WD24   WD25   WD29   WD30   WD31  HC(2)B  ---
 *
 * H(0)   ^ HC(0)A  ^ HC(0B1) -> HC(0)
 * H(1)   ^ HC(1)A  ^ HC(0B1) -> HC(1)
 * HC(2)A ^ HC(2)B  ^ H(2)    -> HC(2)
 * H(0)   ^ H(1)    ^ H(2)    -> H(0/2)
 *
 * chip A      B      C      D      E      F      G      H      I     even    odd
 * ---------------------------------------------------------------------------------
 * a66: WD04   WD05   WD06   WD07   WD08   WD09   WD10   H(3)   0     ---     HC(3)A
 * a84: WD18   WD19   WD20   WD21   WD22   WD23   WD24   WD25   0     HC(3/4) HCPA
 * a63: WD11   WD12   WD13   WD14   WD15   WD16   WD17   H(4)   0     ---     HC(4)A
 * a87: WD26   WD27   WD28   WD29   WD30   WD31   H(5)   0      0     HC(5)   HCPB
 *
 * HC(3)A ^ HC(3/4) -> HC(3)
 * HC(4)A ^ HC(3/4) -> HC(4)
 *
 * WD00 ^ WD01 -> XX01
 *
 * chip A      B      C      D      E      F      G      H      I     even    odd
 * ---------------------------------------------------------------------------------
 * a54: HC(3)A HC(4)A HCPA   HCPB   H(0/2) XX01   WD02   WD03   RP    PERR    ---
 * a65: WD00   WD01   WD02   WD04   WD05   WD07   WD10   WD11   WD12  ---     PCA
 * a74: WD14   WD17   WD18   WD21   WD23   WD24   WD26   WD27   WD29  PCB     ---
 *
 * PCA ^ PCB -> PC
 *
 * Whoa ;-)
 * </PRE>
 */
#if USE_HAMMING_CHECK

#define WD(x) (1ul<<(31-x))

/* a75: WD01   WD04   WD08   WD11   WD15   WD19   WD23   WD26   WD30 ---     HC(0)A */
#define A75 (WD( 1)|WD( 4)|WD( 8)|WD(11)|WD(15)|WD(19)|WD(23)|WD(26)|WD(30))

/* a76: WD00   WD03   WD06   WD10   WD13   WD17   WD21   WD25   WD29 HC(0B1) ---    */
#define A76 (WD( 0)|WD( 3)|WD( 6)|WD(10)|WD(13)|WD(17)|WD(21)|WD(25)|WD(28))

/* a86: WD02   WD05   WD09   WD12   WD16   WD20   WD24   WD27   WD31 HC(1)A  ---    */
#define A86 (WD( 2)|WD( 5)|WD( 9)|WD(12)|WD(16)|WD(20)|WD(24)|WD(27)|WD(31))

/* a64: WD01   WD02   WD03   WD07   WD08   WD09   WD10   WD14   WD15 ---     HC(2)A */
#define A64 (WD( 1)|WD( 2)|WD( 3)|WD( 7)|WD( 8)|WD( 9)|WD(10)|WD(14)|WD(15))

/* a85: WD16   WD17   WD22   WD23   WD24   WD25   WD29   WD30   WD31 HC(2)B  ---    */
#define A85 (WD(16)|WD(17)|WD(22)|WD(23)|WD(24)|WD(25)|WD(29)|WD(30)|WD(31))

/* a66: WD04   WD05   WD06   WD07   WD08   WD09   WD10   H(3)   0    ---     HC(3)A */
#define A66 (WD( 4)|WD( 5)|WD( 6)|WD( 7)|WD( 8)|WD( 9)|WD(10))

/* a84: WD18   WD19   WD20   WD21   WD22   WD23   WD24   WD25   0    HC(3/4) HCPA   */
#define A84 (WD(18)|WD(19)|WD(20)|WD(21)|WD(22)|WD(23)|WD(24)|WD(25))

/* a63: WD11   WD12   WD13   WD14   WD15   WD16   WD17   H(4)   0    ---     HC(4)A */
#define A63 (WD(11)|WD(12)|WD(13)|WD(14)|WD(15)|WD(16)|WD(17))

/* a87: WD26   WD27   WD28   WD29   WD30   WD31   H(5)   0      0    HC(5)   HCPB   */
#define A87 (WD(26)|WD(27)|WD(28)|WD(29)|WD(30)|WD(31))

/* a54: HC(3)A HC(4)A HCPA   HCPB   H(0/2) XX01   WD02   WD03   P    PERR    ---    */
#define A54 (WD( 2)|WD( 3))

/* a65: WD00   WD01   WD02   WD04   WD05   WD07   WD10   WD11   WD12 ---     PCA    */
#define A65 (WD( 0)|WD( 1)|WD( 2)|WD( 4)|WD( 5)|WD( 7)|WD(10)|WD(11)|WD(12))

/* a74: WD14   WD17   WD18   WD21   WD23   WD24   WD26   WD27   WD29 PCB     ---    */
#define A74 (WD(14)|WD(17)|WD(18)|WD(21)|WD(23)|WD(24)|WD(26)|WD(27)|WD(29))

#define H0(hpb) X_BIT(hpb,8,0)      //!< get Hamming code bit 0 from hpb data (really bit 32)
#define H1(hpb) X_BIT(hpb,8,1)      //!< get Hamming code bit 1 from hpb data (really bit 33)
#define H2(hpb) X_BIT(hpb,8,2)      //!< get Hamming code bit 2 from hpb data (really bit 34)
#define H3(hpb) X_BIT(hpb,8,3)      //!< get Hamming code bit 3 from hpb data (really bit 35)
#define H4(hpb) X_BIT(hpb,8,4)      //!< get Hamming code bit 4 from hpb data (really bit 36)
#define H5(hpb) X_BIT(hpb,8,5)      //!< get Hamming code bit 5 from hpb data (really bit 37)
#define RH(hpb) X_RDBITS(hpb,8,0,5) //!< get Hamming code from hpb data (bits 32 to 37)
#define RP(hpb) X_BIT(hpb,8,6)      //!< get parity bit from hpb data (really bit 38)

/** @brief return even parity of a (masked) 32 bit value */
static __inline UINT8 parity_even(UINT32 val)
{
		val -= ((val >> 1) & 0x55555555);
		val = (((val >> 2) & 0x33333333) + (val & 0x33333333));
		val = (((val >> 4) + val) & 0x0f0f0f0f);
		val += (val >> 8);
		val += (val >> 16);
		return (val & 1);
}

/** @brief return odd parity of a (masked) 32 bit value */
#define parity_odd(val) (parity_even(val)^1)

/**
 * @brief lookup table to convert a Hamming syndrome into a bit number to correct
 */
static const int hamming_lut[64] = {
	-1, -1, -1,  0, -1,  1,  2,  3, /* A69: HR(5):0 HR(4):0 HR(3):0 */
	-1,  4,  5,  6,  7,  8,  9, 10, /* A79: HR(5):0 HR(4):0 HR(3):1 */
	-1, 11, 12, 13, 14, 15, 16, 17, /* A67: HR(5):0 HR(4):1 HR(3):0 */
	-1, -1, -1, -1, -1,  1, -1, -1, /* non chip selected */
	-1, 26, 27, 28, 29, 30, 31, -1, /* A68: HR(5):1 HR(4):0 HR(3):0 */
	-1, -1, -1, -1, -1,  1, -1, -1, /* non chip selected */
	18, 19, 20, 21, 22, 23, 24, 25, /* A78: HR(5):1 HR(4):1 HR(3):0 */
	-1, -1, -1, -1, -1,  1, -1, -1  /* non chip selected */
};

/**
 * @brief read or write a memory double-word and caluclate its Hamming code
 *
 * Hamming code generation according to the schematics described above.
 * It's certainly overkill to do this on a modern PC, but I think we'll
 * need it for perfect emulation anyways (Hamming code hardware checking).
 *
 * @param write non-zero if this is a memory write (don't check for error)
 * @param dw_addr the double-word address
 * @param dw_data the double-word data to write
 * @return dw_data
 */
UINT32 alto2_cpu_device::hamming_code(int write, UINT32 dw_addr, UINT32 dw_data)
{
	register UINT8 hpb = write ? 0 : m_mem.hpb[dw_addr];
	register UINT8 hc_0_a;
	register UINT8 hc_0b1;
	register UINT8 hc_1_a;
	register UINT8 hc_2_a;
	register UINT8 hc_2_b;
	register UINT8 hc_0;
	register UINT8 hc_1;
	register UINT8 hc_2;
	register UINT8 h_0_2;
	register UINT8 hc_3_a;
	register UINT8 hc_3_4;
	register UINT8 hcpa;
	register UINT8 hc_4_a;
	register UINT8 hc_3;
	register UINT8 hc_4;
	register UINT8 hc_5;
	register UINT8 hcpb;
	register UINT8 perr;
	register UINT8 pca;
	register UINT8 pcb;
	register UINT8 pc;
	register int syndrome;

	/* a75: WD01   WD04   WD08   WD11   WD15   WD19   WD23   WD26   WD30 ---     HC(0)A */
	hc_0_a = parity_odd (dw_data & A75);
	/* a76: WD00   WD03   WD06   WD10   WD13   WD17   WD21   WD25   WD29 HC(0B1) ---    */
	hc_0b1 = parity_even(dw_data & A76);
	/* a86: WD02   WD05   WD09   WD12   WD16   WD20   WD24   WD27   WD31 HC(1)A  ---    */
	hc_1_a = parity_even(dw_data & A86);
	/* a64: WD01   WD02   WD03   WD07   WD08   WD09   WD10   WD14   WD15 ---     HC(2)A */
	hc_2_a = parity_odd (dw_data & A64);
	/* a85: WD16   WD17   WD22   WD23   WD24   WD25   WD29   WD30   WD31 HC(2)B  ---    */
	hc_2_b = parity_even(dw_data & A85);

	hc_0  = H0(hpb) ^ hc_0_a ^ hc_0b1;
	hc_1  = H1(hpb) ^ hc_1_a ^ hc_0b1;
	hc_2  = hc_2_a ^ hc_2_b ^ H2(hpb);
	h_0_2 = H0(hpb) ^ H1(hpb) ^ H2(hpb);

	/* a66: WD04   WD05   WD06   WD07   WD08   WD09   WD10   H(3)   0    ---     HC(3)A */
	hc_3_a = parity_odd ((dw_data & A66) ^ H3(hpb));
	/* a84: WD18   WD19   WD20   WD21   WD22   WD23   WD24   WD25   0    HC(3/4) HCPA   */
	hcpa   = parity_odd (dw_data & A84);
	hc_3_4 = hcpa ^ 1;
	/* a63: WD11   WD12   WD13   WD14   WD15   WD16   WD17   H(4)   0    ---     HC(4)A */
	hc_4_a = parity_odd ((dw_data & A63) ^ H4(hpb));

	/* a87: WD26   WD27   WD28   WD29   WD30   WD31   H(5)   0      0    HC(5)   HCPB   */
	hcpb   = parity_odd ((dw_data & A87) ^ H5(hpb));
	hc_3   = hc_3_a ^ hc_3_4;
	hc_4   = hc_4_a ^ hc_3_4;
	hc_5   = hcpb ^ 1;

	syndrome = (hc_0<<5)|(hc_1<<4)|(hc_2<<3)|(hc_3<<2)|(hc_4<<1)|(hc_5);

	/*
	 * Note: Here I XOR all the non dw_data inputs into bit 0,
	 * which has the same effect as spreading them over some bits
	 * and then counting them... I hope ;-)
	 */
	/* a54: HC(3)A HC(4)A HCPA   HCPB   H(0/2) XX01   WD02   WD03   P    PERR    ---    */
	perr = parity_even(
				hc_3_a ^
				hc_4_a ^
				hcpa ^
				hcpb ^
				h_0_2 ^
				(X_RDBITS(dw_data,32,0,0) ^ X_RDBITS(dw_data,32,1,1)) ^
				(dw_data & A54) ^
				RP(hpb) ^
				1);

	/* a65: WD00   WD01   WD02   WD04   WD05   WD07   WD10   WD11   WD12 ---     PCA    */
	pca = parity_odd (dw_data & A65);
	/* a74: WD14   WD17   WD18   WD21   WD23   WD24   WD26   WD27   WD29 PCB     ---    */
	pcb = parity_even(dw_data & A74);
	pc = pca ^ pcb;

	if (write) {
		/* update the hamming code and parity bit store */
		m_mem.hpb[dw_addr] = (syndrome << 2) | (pc << 1);
		return dw_data;

	}

	/**
	 * <PRE>
	 * A22 (74H30) 8-input NAND to check for error
	 *  input   signal
	 *  -------------------------
	 *  1   POK = PERR'
	 *  4   NER(08) = HC(0)'
	 *  3   NER(09) = HC(1)'
	 *  2   NER(10) = HC(2)'
	 *  6   NER(11) = HC(3)'
	 *  5   NER(12) = HC(4)'
	 *  12  NER(13) = HC(5)'
	 *  11  1 (VPUL3)
	 *
	 *  output  signal
	 *  -------------------------
	 *  8   ERROR
	 *
	 * Remembering De Morgan this can be simplified:
	 * ERROR is 0, whenever all of PERR and HC(0) to HC(5) are 0.
	 * Or the other way round: any of perr or syndrome non-zero means ERROR=1.
	 * </PRE>
	 */
	if (perr || syndrome) {
		/* latch data on the first error */
		if (!m_mem.error) {
			m_mem.error = true;
			PUT_MESR_HAMMING(m_mem.mesr, RH(hpb));
			PUT_MESR_PERR(m_mem.mesr, perr);
			PUT_MESR_PARITY(m_mem.mesr, RP(hpb));
			PUT_MESR_SYNDROME(m_mem.mesr, syndrome);
			PUT_MESR_BANK(m_mem.mesr, (dw_addr >> 15));
			/* latch memory address register */
			m_mem.mear = m_mem.mar & 0177777;
			LOG((LOG_MEM,5,"    memory error at dword addr:%07o data:%011o check:%03o\n", dw_addr * 2, dw_data, hpb));
			LOG((LOG_MEM,6,"    MEAR: %06o\n", m_mem.mear));
			LOG((LOG_MEM,6,"    MESR: %06o\n", m_mem.mesr ^ 0177777));
			LOG((LOG_MEM,7,"        Hamming code read    : %#o\n", GET_MESR_HAMMING(m_mem.mesr)));
			LOG((LOG_MEM,7,"        Parity error         : %o\n", GET_MESR_PERR(m_mem.mesr)));
			LOG((LOG_MEM,7,"        Memory parity bit    : %o\n", GET_MESR_PARITY(m_mem.mesr)));
			LOG((LOG_MEM,7,"        Hamming syndrome     : %#o (bit #%d)\n", GET_MESR_SYNDROME(m_mem.mesr), hamming_lut[GET_MESR_SYNDROME(m_mem.mesr)]));
			LOG((LOG_MEM,7,"        Memory bank          : %#o\n", GET_MESR_BANK(m_mem.mesr)));
			LOG((LOG_MEM,6,"    MECR: %06o\n", m_mem.mecr ^ 0177777));
			LOG((LOG_MEM,7,"        Test Hamming code    : %#o\n", GET_MECR_TEST_CODE(m_mem.mecr)));
			LOG((LOG_MEM,7,"        Test mode            : %s\n", GET_MECR_TEST_MODE(m_mem.mecr) ? "on" : "off"));
			LOG((LOG_MEM,7,"        INT on single-bit err: %s\n", GET_MECR_INT_SBERR(m_mem.mecr) ? "on" : "off"));
			LOG((LOG_MEM,7,"        INT on double-bit err: %s\n", GET_MECR_INT_DBERR(m_mem.mecr) ? "on" : "off"));
			LOG((LOG_MEM,7,"        Error correction     : %s\n", GET_MECR_ERRCORR(m_mem.mecr) ? "off" : "on"));
		}
		if (-1 == hamming_lut[syndrome]) {
			/* double-bit error: wake task_part, if we're told so */
			if (GET_MECR_INT_DBERR(m_mem.mecr))
				m_task_wakeup |= 1 << task_part;
		} else {
			/* single-bit error: wake task_part, if we're told so */
			if (GET_MECR_INT_SBERR(m_mem.mecr))
				m_task_wakeup |= 1 << task_part;
			/* should we correct the single bit error ? */
			if (0 == GET_MECR_ERRCORR(m_mem.mecr)) {
				LOG((LOG_MEM,0,"    correct bit #%d addr:%07o data:%011o check:%03o\n", hamming_lut[syndrome], dw_addr * 2, dw_data, hpb));
				dw_data ^= 1ul << hamming_lut[syndrome];
			}
		}
	}
	return dw_data;
}
#endif  /* USE_HAMMING_CHECK */

/**
 * @brief memory error address register read
 *
 * This register is a 'shadow MAR'; it holds the address of the
 * first error since the error status was last reset. If no error
 * has occurred, MEAR reports the address of the most recent
 * memory access. Note that MEAR is set whenever an error of
 * _any kind_ (single-bit or double-bit) is detected.
 */
READ16_MEMBER( alto2_cpu_device::mear_r )
{
	int data = m_mem.error ? m_mem.mear : m_mem.mar;
	if (!space.debugger_access()) {
		LOG((LOG_MEM,2,"    MEAR read %07o\n", data));
	}
	return data;
}

/**
 * @brief memory error status register read
 *
 * This register reports specifics of the first error that
 * occurred since MESR was last reset. Storing anything into
 * this register resets the error logic and enables it to
 * detect a new error. Bits are "low true", i.e. if the bit
 * is 0, the conidition is true.
 * <PRE>
 * MESR[0-5]    Hamming code reported from error
 * MESR[6]  Parity error
 * MESR[7]  Memory parity bit
 * MESR[8-13]   Syndrome bits
 * MESR[14-15]  Bank number in which error occurred
 * </PRE>
 */
READ16_MEMBER( alto2_cpu_device::mesr_r )
{
	UINT16 data = m_mem.mesr ^ 0177777;
	if (!space.debugger_access()) {
		LOG((LOG_MEM,2,"    MESR read %07o\n", data));
		LOG((LOG_MEM,6,"        Hamming code read    : %#o\n", GET_MESR_HAMMING(data)));
		LOG((LOG_MEM,6,"        Parity error         : %o\n", GET_MESR_PERR(data)));
		LOG((LOG_MEM,6,"        Memory parity bit    : %o\n", GET_MESR_PARITY(data)));
#if USE_HAMMING_CHECK
		LOG((LOG_MEM,6,"        Hamming syndrome     : %#o (bit #%d)\n", GET_MESR_SYNDROME(data), hamming_lut[GET_MESR_SYNDROME(data)]));
#else
		LOG((LOG_MEM,6,"        Hamming syndrome     : %#o\n", GET_MESR_SYNDROME(data)));
#endif
		LOG((LOG_MEM,6,"        Memory bank          : %#o\n", GET_MESR_BANK(data)));
	}
	return data;
}

WRITE16_MEMBER( alto2_cpu_device::mesr_w )
{
	if (!space.debugger_access()) {
		LOG((LOG_MEM,2,"    MESR write %07o (clear MESR; was %07o)\n", data, m_mem.mesr));
	}
	m_mem.mesr = 0;     // set all bits to 0
	m_mem.error = 0;    // reset the error flag
	m_task_wakeup &= ~(1 << task_part); // clear the task wakeup for the parity error task
}

/**
 * @brief memory error control register write
 *
 * Storing into this register is the means for controlling
 * the memory error logic. This register is set to all ones
 * (disable all interrupts) when the alto is bootstrapped
 * and when the parity error task first detects an error.
 * When an error has occurred, MEAR and MESR should be read
 * before setting MECR. Bits are "low true", i.e. a 0 bit
 * enables the condition.
 *
 * <PRE>
 * MECR[0-3]    Spare
 * MECR[4-10]   Test hamming code (used only for special diagnostics)
 * MECR[11] Test mode (used only for special diagnostics)
 * MECR[12] Cause interrupt on single-bit errors if zero
 * MECR[13] Cause interrupt on double-bit errors if zero
 * MECR[14] Do not use error correction if zero
 * MECR[15] Spare
 * </PRE>
 */
WRITE16_MEMBER( alto2_cpu_device::mecr_w )
{
	m_mem.mecr = data ^ 0177777;
	X_WRBITS(m_mem.mecr,16, 0, 3,0);
	X_WRBITS(m_mem.mecr,16,15,15,0);
	if (!space.debugger_access()) {
		LOG((LOG_MEM,2,"    MECR write %07o\n", data));
		LOG((LOG_MEM,6,"        Test Hamming code    : %#o\n", GET_MECR_TEST_CODE(m_mem.mecr)));
		LOG((LOG_MEM,6,"        Test mode            : %s\n", GET_MECR_TEST_MODE(m_mem.mecr) ? "on" : "off"));
		LOG((LOG_MEM,6,"        INT on single-bit err: %s\n", GET_MECR_INT_SBERR(m_mem.mecr) ? "on" : "off"));
		LOG((LOG_MEM,6,"        INT on double-bit err: %s\n", GET_MECR_INT_DBERR(m_mem.mecr) ? "on" : "off"));
		LOG((LOG_MEM,6,"        Error correction     : %s\n", GET_MECR_ERRCORR(m_mem.mecr) ? "off" : "on"));
	}
}

/**
 * @brief memory error control register read
 */
READ16_MEMBER( alto2_cpu_device::mecr_r )
{
	UINT16 data = m_mem.mecr ^ 0177777;
	/* set all spare bits */
	if (!space.debugger_access()) {
		LOG((LOG_MEM,2,"    MECR read %07o\n", data));
		LOG((LOG_MEM,6,"        Test Hamming code    : %#o\n", GET_MECR_TEST_CODE(data)));
		LOG((LOG_MEM,6,"        Test mode            : %s\n", GET_MECR_TEST_MODE(data) ? "on" : "off"));
		LOG((LOG_MEM,6,"        INT on single-bit err: %s\n", GET_MECR_INT_SBERR(data) ? "on" : "off"));
		LOG((LOG_MEM,6,"        INT on double-bit err: %s\n", GET_MECR_INT_DBERR(data) ? "on" : "off"));
		LOG((LOG_MEM,6,"        Error correction     : %s\n", GET_MECR_ERRCORR(data) ? "off" : "on"));
	}
	return data;
}

//! read i/o space RAM
READ16_MEMBER ( alto2_cpu_device::ioram_r )
{
	offs_t dword_addr = offset / 2;
	return static_cast<UINT16>(offset & 1 ? GET_ODD(m_mem.ram[dword_addr]) : GET_EVEN(m_mem.ram[dword_addr]));
}

//! write i/o space RAM
WRITE16_MEMBER( alto2_cpu_device::ioram_w )
{
	offs_t dword_addr = offset / 2;
	if (offset & 1)
		PUT_ODD(m_mem.ram[dword_addr], data);
	else
		PUT_EVEN(m_mem.ram[dword_addr], data);
}

/**
 * @brief load the memory address register with some value
 *
 * @param rsel selected register (to detect refresh cycles)
 * @param addr memory address
 */
void alto2_cpu_device::load_mar(UINT8 rsel, UINT32 addr)
{
	if (rsel == 037) {
		/*
		 * starting a memory refresh cycle
		 * currently we don't do anything special
		 */
		LOG((LOG_MEM,5, "   MAR<-; refresh cycle @ %#o\n", addr));
		m_mem.mar = addr;
		m_mem.access = ALTO2_MEM_REFRESH;
		m_mem.cycle = cycle();
		return;
	}

	m_mem.mar = addr;
	if (addr < m_mem.size) {
		LOG((LOG_MEM,2, "   MAR<-; mar = %#o\n", addr));
		m_mem.access = ALTO2_MEM_RAM;
		// fetch the memory double-word to the read/write latches
		m_mem.rmdd = m_mem.wmdd = m_mem.ram[m_mem.mar/2];
		// keep track of the current CPU cycle
		m_mem.cycle = cycle();
	} else {
		m_mem.access = ALTO2_MEM_INVALID;
		m_mem.rmdd = m_mem.wmdd = ~0;
	}
}

/**
 * @brief read memory or memory mapped I/O from the address in mar to md
 *
 * @result returns value from memory (RAM or MMIO)
 */
UINT16 alto2_cpu_device::read_mem()
{
	UINT32 base_addr;

	if (ALTO2_MEM_NONE == m_mem.access) {
		LOG((LOG_MEM,0,"    fatal: mem read with no preceding address\n"));
		return 0177777;
	}

	if (cycle() > m_mem.cycle + 4) {
		LOG((LOG_MEM,0,"    fatal: mem read (MAR %#o) too late (+%lld cyc)\n", m_mem.mar, cycle() - m_mem.cycle));
		m_mem.access = ALTO2_MEM_NONE;
		return 0177777;
	}

	base_addr = m_mem.mar & 0177777;
	if (base_addr >= ALTO2_IO_PAGE_BASE && m_mem.mar < ALTO2_RAM_SIZE) {
		m_mem.md = m_iomem->read_word(m_iomem->address_to_byte(base_addr));
		LOG((LOG_MEM,6,"    MD = MMIO[%#o] (%#o)\n", base_addr, m_mem.md));
		m_mem.access = ALTO2_MEM_NONE;
#if ALTO2_DEBUG
		watch_read(m_mem.mar, m_mem.md);
#endif
		return m_mem.md;
	}

#if USE_HAMMING_CHECK
	/* check for errors on the first access */
	if (!(m_mem.access & ALTO2_MEM_ODD))
		m_mem.rmdd = hamming_code(0, m_mem.mar/2, m_mem.rmdd);
#endif
	m_mem.md = (m_mem.mar & ALTO2_MEM_ODD) ? GET_ODD(m_mem.rmdd) : GET_EVEN(m_mem.rmdd);
	LOG((LOG_MEM,6,"    MD = RAM[%#o] (%#o)\n", m_mem.mar, m_mem.md));

#if ALTO2_DEBUG
	watch_read(m_mem.mar, m_mem.md);
#endif

	if (m_mem.access & ALTO2_MEM_ODD) {
		// after reading the odd word, reset the access flag
		m_mem.access = ALTO2_MEM_NONE;
	} else {
		// after reading the even word word, toggle access flag (and address) to the odd word
		m_mem.mar ^= ALTO2_MEM_ODD;
		m_mem.access ^= ALTO2_MEM_ODD;
		// extend the read succeeds window by one cycle
		m_mem.cycle++;
	}
	return m_mem.md;
}

/**
 * @brief write memory or memory mapped I/O from md to the address in mar
 *
 * @param data data to write to RAM or MMIO
 */
void alto2_cpu_device::write_mem(UINT16 data)
{
	int base_addr;

	m_mem.md = data & 0177777;
	if (ALTO2_MEM_NONE == m_mem.access) {
		LOG((LOG_MEM,0,"    fatal: mem write with no preceding address\n"));
		return;
	}

	if (cycle() > m_mem.cycle + 4) {
		LOG((LOG_MEM,0,"    fatal: mem write (MAR %#o, data %#o) too late (+%lld cyc)\n", m_mem.mar, data, cycle() - m_mem.cycle));
		m_mem.access = ALTO2_MEM_NONE;
		return;
	}

	base_addr = m_mem.mar & 0177777;
	if (base_addr >= ALTO2_IO_PAGE_BASE && m_mem.mar < ALTO2_RAM_SIZE) {
		m_iomem->write_word(m_iomem->address_to_byte(base_addr), m_mem.md);
		LOG((LOG_MEM,6, "   MMIO[%#o] = MD (%#o)\n", base_addr, m_mem.md));
		m_mem.access = ALTO2_MEM_NONE;
#if ALTO2_DEBUG
		watch_write(m_mem.mar, m_mem.md);
#endif
		return;
	}

	LOG((LOG_MEM,6, "   RAM[%#o] = MD (%#o)\n", m_mem.mar, m_mem.md));
	if (m_mem.mar & ALTO2_MEM_ODD)
		PUT_ODD(m_mem.wmdd, m_mem.md);
	else
		PUT_EVEN(m_mem.wmdd, m_mem.md);

#if USE_HAMMING_CHECK
	if (m_mem.access & ALTO2_MEM_RAM)
		m_mem.ram[m_mem.mar/2] = hamming_code(1, m_mem.mar/2, m_mem.wmdd);
#else
	if (m_mem.access & ALTO2_MEM_RAM)
		m_mem.ram[m_mem.mar/2] = m_mem.wmdd;
#endif

#if ALTO2_DEBUG
	watch_write(m_mem.mar, m_mem.md);
#endif
	// Toggle the odd/even word access flag
	// NB: don't reset mem.access to permit double word exchange
	m_mem.mar ^= ALTO2_MEM_ODD;
	m_mem.access ^= ALTO2_MEM_ODD;
	// extend the write succeeds window by one cycle
	m_mem.cycle++;
}

/**
 * @brief debugger interface to read memory
 *
 * @param addr address to read
 * @return memory contents at address (16 bits)
 */
UINT16 alto2_cpu_device::debug_read_mem(UINT32 addr)
{
	space(AS_2).set_debugger_access(true);
	int base_addr = addr & 0177777;
	int data = 0177777;
	if (base_addr >= ALTO2_IO_PAGE_BASE && addr < ALTO2_RAM_SIZE) {
		data = m_iomem->read_word(m_iomem->address_to_byte(base_addr));
	} else {
		data = (addr & ALTO2_MEM_ODD) ? GET_ODD(m_mem.ram[addr/2]) : GET_EVEN(m_mem.ram[addr/2]);
	}
	space(AS_2).set_debugger_access(false);
	return data;
}

/**
 * @brief debugger interface to write memory
 *
 * @param addr address to write
 * @param data data to write (16 bits used)
 */
void alto2_cpu_device::debug_write_mem(UINT32 addr, UINT16 data)
{
	space(AS_2).set_debugger_access(true);
	int base_addr = addr & 0177777;
	if (base_addr >= ALTO2_IO_PAGE_BASE && addr < ALTO2_RAM_SIZE) {
		m_iomem->write_word(m_iomem->address_to_byte(base_addr), data);
	} else if (addr & ALTO2_MEM_ODD) {
		PUT_ODD(m_mem.ram[addr/2], data);
	} else {
		PUT_EVEN(m_mem.ram[addr/2], data);
	}
	space(AS_2).set_debugger_access(false);
}

/**
 * @brief initialize the memory system
 *
 * Zeroes the memory context, including RAM and installs dummy
 * handlers for the memory mapped I/O area.
 * Sets handlers for access to the memory error address, status,
 * and control registers at 0177024 to 0177026.
 */
void alto2_cpu_device::init_memory()
{
	memset(&m_mem, 0, sizeof(m_mem));
	save_item(NAME(m_mem.mar));
	save_item(NAME(m_mem.rmdd));
	save_item(NAME(m_mem.wmdd));
	save_item(NAME(m_mem.md));
	save_item(NAME(m_mem.cycle));
	save_item(NAME(m_mem.access));
	save_item(NAME(m_mem.error));
	save_item(NAME(m_mem.mear));
	save_item(NAME(m_mem.mecr));
}

void alto2_cpu_device::exit_memory()
{
	// no need for this since it free on exit by itself
	// if (m_mem.ram) {
		// auto_free(machine(), m_mem.ram);
		// m_mem.ram = 0;
	// }
	// if (m_mem.hpb) {
		// auto_free(machine(), m_mem.hpb);
		// m_mem.hpb = 0;
	// }
}

void alto2_cpu_device::reset_memory()
{
	if (m_mem.ram) {
		auto_free(machine(), m_mem.ram);
		m_mem.ram = 0;
	}
	if (m_mem.hpb) {
		auto_free(machine(), m_mem.hpb);
		m_mem.hpb = 0;
	}
	// allocate 64K or 128K words of main memory
	ioport_port* config = ioport(":CONFIG");
	// config should be valid, unless the driver doesn't define it
	if (config)
		m_mem.size = config->read() & 1 ? ALTO2_RAM_SIZE : 2 * ALTO2_RAM_SIZE;
	else
		m_mem.size = ALTO2_RAM_SIZE;
	logerror("Main memory %u KiB\n", static_cast<UINT32>(sizeof(UINT16) * m_mem.size / 1024));

	m_mem.ram = auto_alloc_array_clear(machine(), UINT32, sizeof(UINT16) * m_mem.size);
	m_mem.hpb = auto_alloc_array_clear(machine(), UINT8,  sizeof(UINT16) * m_mem.size);

#if USE_HAMMING_CHECK
	// Initialize the hamming codes and parity bit
	for (UINT32 addr = 0; addr < ALTO2_IO_PAGE_BASE; addr++) {
		hamming_code(1, addr, 0);
		hamming_code(1, 0200000 + addr, 0);
	}
#endif
	m_mem.mar = 0;
	m_mem.rmdd = 0;
	m_mem.wmdd = 0;
	m_mem.md = 0;
	m_mem.cycle = 0;
	m_mem.access = 0;
	m_mem.error = false;
	m_mem.mear = 0;
	m_mem.mesr = 0;
	m_mem.mecr = 0;
}
