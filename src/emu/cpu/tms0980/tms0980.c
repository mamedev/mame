/*


TMS0980/TMS1000-family CPU cores

The tms0980 and tms1000-family cpu cores are very similar. The tms0980 has a
slightly bigger addressable area and uses 9bit instructions where the tms1000
family uses 8bit instruction. The instruction set themselves are very similar
though. The table below shows the differences between the different models.

Mode     | ROM       | RAM      | R pins | O pins | K pins | ids
---------+-----------+----------+--------+--------+--------|----------
tms0970* | 1024 *  8 |  64 *  4 |        |        |        | tms0972
tms0920* |  511?*  9 |  40 *  5 |        |        |        | tmc0921
tms0980  | 2048 *  9 |  64 *  9 |        |        |        | tmc0981
tms1000  | 1024 *  8 |  64 *  4 |     11 |      8 |      4 | tms1001
tms1040* | 1024 *  8 |  64 *  4 |        |        |        | tms1043
tms1070  | 1024 *  8 |  64 *  4 |     11 |      8 |      4 | tms1071
tms1100  | 2048 *  8 | 128 *  4 |     11 |      8 |      4 | tms1111/tms1115
tms1170* | 2048 *  8 | 128 *  4 |        |        |        | tmc1172
tms1200  | 1024 *  8 |  64 *  4 |     13 |      8 |      4 | tms1215
tms1270  | 1024 *  8 |  64 *  4 |     13 |     10 |      4 | tms1278
tms1300  | 2048 *  8 | 128 *  4 |     16 |      8 |      4 | tms1309
tms1370* | 2048 *  8 | 128 *  4 |        |        |        | za0543
tms1400* | 4096 *  8 | 128 *  4 |        |        |        |
tms1470* | 4096 *  8 | 128 *  4 |        |        |        | tms1470
tms1500* | 2048 * 13 |  64 * 20 |        |        |        | tmc1501
tms1600* | 4096 *  8 | 128 *  4 |        |        |        |
tms1670* | 4096 *  8 | 128 *  4 |        |        |        |
tms1700* |  512 *  8 |  32 *  4 |        |        |        |
tms1980* | 2048 *  9 |  64 *  9 |        |        |        | tmc1982
tms1990* | 1024 *  8 |  64 *  4 |        |        |        | tmc1991
tp0310*  |  511?*  9 |  40 *  5 |        |        |        | tp0311
tp0320*  | 2048 *  9 |  64 * 13 |        |        |        | tp0321
tp0455*  |           |          |        |        |        | cd4501
tp0456*  |           |          |        |        |        | cd4555
tp0458*  |           |          |        |        |        | cd4812
tp0485*  |           |          |        |        |        | cd2901
tp0530*  |           |          |        |        |        | cd5402

* = not supported yet

The TMS1000 core has been tested with some example code, the other models
have not been tested lacking rom dumps.

Each instruction takes 12 cycles to execute in 2 phases: a fetch phase and an
execution phase. The execution phase takes place at the same time as the fetch
phase of the next instruction. So, during execution there are both fetch and
execution operations taking place. The operation can be split up as follows:
cycle #0
    - Fetch:
        1. ROM address 0
    - Execute:
        1. Read RAM
        2. Clear ALU inputs
        3. Execute BRANCH/CALL/RETN part #2
        4. K input valid
cycle #1
    - Fetch:
        1. ROM address 1
    - Execute:
        1. Update ALU inputs
cycle #2
    - Fetch:
        1. nothing/wait(?)
    - Execute:
        1. Perform ALU operation
        2. Write RAM
cycle #3
    - Fetch:
        1. Fetch/Update PC/RAM address #1
    - Execute:
        1. Register store part #1
cycle #4
    - Fetch:
        1. Fetch/Update PC/RAM address #2
    - Execute:
        1. Register store part #2
cycle #5
    - Fetch:
        1. Instruction decode
    - Execute:
        1. Execute BRANCH/CALL/RETN part #1


The CPU cores contains a set of fixed instructions and a set of
instructions created using microinstructions. A subset of the
instruction set could be defined from the microinstructions by
TI customers. Currently we only support the standard instruction
set as defined by TI.

The microinstructions are:
15TN  - 15 to -ALU
ATN   - ACC to -ALU
AUTA  - ALU to ACC
AUTY  - ALU to Y
C8    - CARRY8 to STATUS
CIN   - Carry In to ALU
CKM   - CKB to MEM
CKN   - CKB to -ALU
CKP   - CKB to +ALU
CME   - Conditional Memory Enable
DMTP  - DAM to +ALU
MTN   - MEM to -ALU
MTP   - MEM to +ALU
NATN  - ~ACC to -ALU
NDMTP - ~DAM to +ALU
NE    - COMP to STATUS
SSE   - Special Status Enable
SSS   - Special Status Sample
STO   - ACC to MEM
YTP   - Y to +ALU

cycle #0: 15TN, ATN, CIN, CKN, CKP, DMTP, MTN, MTP, NATN, NDMTP, YTP
cycle #2: C8(?), CKM, NE(?), STO
cycle #3,#4: AUTA, AUTY

unknown cycle: CME, SSE, SSS

*/

#include "emu.h"
#include "debugger.h"
#include "tms0980.h"

#define LOG					0

#define MICRO_MASK			0x80000000
#define FIXED_INSTRUCTION	0x00000000


/* Standard/fixed intructions */
#define F_ILL				0x00000000
#define F_BR				0x00000001
#define F_CALL				0x00000002
#define F_CLO				0x00000004
#define F_COMC				0x00000008
#define F_COMX				0x00000010
#define F_COMX8				0x00000020
#define F_LDP				0x00000040
#define F_LDX				0x00000080
#define F_OFF				0x00000100
#define F_RBIT				0x00000200
#define F_REAC				0x00000400
#define F_RETN				0x00000800
#define F_RSTR				0x00001000
#define F_SAL				0x00002000
#define F_SBIT				0x00004000
#define F_SBL				0x00008000
#define F_SEAC				0x00010000
#define F_SETR				0x00020000
#define F_TDO				0x00040000


/* Microinstructions */
#define M_15TN				0x00000001
#define M_ATN				0x00000002
#define M_AUTA				0x00000004
#define M_AUTY				0x00000008
#define M_C8				0x00000010
#define M_CIN				0x00000020
#define M_CKM				0x00000040
#define M_CKN				0x00000080
#define M_CKP				0x00000100
#define M_CME				0x00000200
#define M_DMTP				0x00000400
#define M_MTN				0x00000800
#define M_MTP				0x00001000
#define M_NATN				0x00002000
#define M_NDMTP				0x00004000
#define M_NE				0x00008000
#define M_SSE				0x00010000
#define M_SSS				0x00020000
#define M_STO				0x00040000
#define M_STSL				0x00080000
#define M_YTP				0x00100000


/* instructions built from microinstructions */
#define I_AC1AC		( MICRO_MASK | M_CKP | M_ATN | M_CIN | M_C8 | M_AUTA )
#define I_A6AAC		I_ACACC
#define I_A8AAC		I_ACACC
#define I_A10AAC	I_ACACC
#define I_ACACC		( MICRO_MASK | M_CKP | M_ATN | M_C8 | M_AUTA )
#define I_ACNAA		( MICRO_MASK | M_CKP | M_NATN | M_AUTA )
#define I_ALEC		( MICRO_MASK | M_CKP | M_NATN | M_CIN | M_C8 )
#define I_ALEM		( MICRO_MASK | M_MTP | M_NATN | M_CIN | M_C8 )
#define I_AMAAC		( MICRO_MASK | M_MTP | M_ATN | M_C8 | M_AUTA )
#define I_CCLA		( MICRO_MASK | M_AUTA | M_SSS )
#define I_CLA		( MICRO_MASK | M_AUTA )
#define I_CPAIZ		( MICRO_MASK | M_NATN | M_CIN | M_C8 | M_AUTA )
#define I_CTMDYN	( MICRO_MASK | M_YTP | M_15TN | M_C8 | M_AUTY | M_CME )
#define I_DAN		( MICRO_MASK | M_CKP | M_ATN | M_CIN | M_C8 | M_AUTA )
#define I_DMAN		( MICRO_MASK | M_MTP | M_15TN | M_C8 | M_AUTA )
#define I_DMEA		( MICRO_MASK | M_MTP | M_DMTP | M_SSS | M_AUTA )
#define I_DNAA		( MICRO_MASK | M_DMTP | M_NATN | M_SSS | M_AUTA )
#define I_DYN		( MICRO_MASK | M_YTP | M_15TN | M_C8 | M_AUTY )
#define I_IA		( MICRO_MASK | M_ATN | M_CIN | M_AUTA )
#define I_IMAC		( MICRO_MASK | M_MTP | M_CIN | M_C8 | M_AUTA )
#define I_IYC		( MICRO_MASK | M_YTP | M_CIN | M_C8 | M_AUTY )
#define I_KNEZ		( MICRO_MASK | M_CKP | M_NE )
#define I_MNEA		( MICRO_MASK | M_MTP | M_ATN | M_NE )
#define I_MNEZ		( MICRO_MASK | M_MTP | M_NE )
#define I_M_NDMEA	( MICRO_MASK | M_MTN | M_NDTMP | M_SSS | M_AUTA )
#define I_SAMAN		( MICRO_MASK | M_MTP | M_NATN | M_CIN | M_C8 | M_AUTA )
#define I_SETR		( MICRO_MASK | M_YTP | M_15TN | M_AUTY | M_C8 )
#define I_TAM		( MICRO_MASK | M_STO )
#define I_TAMACS	( MICRO_MASK | M_STO | M_ATN | M_CKP | M_AUTA | M_SSE )
#define I_TAMDYN	( MICRO_MASK | M_STO | M_YTP | M_15TN | M_AUTY | M_C8 )
#define I_TAMIY		( MICRO_MASK | M_STO | M_YTP | M_CIN | M_AUTY )
#define I_TAMIYC	( MICRO_MASK | M_STO | M_YTP | M_CIN | M_C8 | M_AUTY )
#define I_TAMZA		( MICRO_MASK | M_STO | M_AUTA )
#define I_TAY		( MICRO_MASK | M_ATN | M_AUTY )
#define I_TBIT		( MICRO_MASK | M_CKP | M_CKN | M_MTP | M_NE )
#define I_TCY		( MICRO_MASK | M_CKP | M_AUTY )
#define I_TCMIY		( MICRO_MASK | M_CKM | M_YTP | M_CIN | M_AUTY )
#define I_TKA		( MICRO_MASK | M_CKP | M_AUTA )
#define I_TKM		( MICRO_MASK | M_CKM )
#define I_TMA		( MICRO_MASK | M_MTP | M_AUTA )
#define I_TMY		( MICRO_MASK | M_MTP | M_AUTY )
#define I_TYA		( MICRO_MASK | M_YTP | M_AUTA )
#define I_XDA		( MICRO_MASK | M_DMTP | M_AUTA | M_STO )
#define I_XMA		( MICRO_MASK | M_MTP | M_STO | M_AUTA )
#define I_YMCY		( MICRO_MASK | M_CIN | M_YTP | M_CKN | M_AUTY )
#define I_YNEA		( MICRO_MASK | M_YTP | M_ATN | M_NE )
#define I_YNEC		( MICRO_MASK | M_YTP | M_CKN | M_NE )


struct tms0980_state
{
	UINT8	prev_pc;		/* previous program counter */
	UINT8	prev_pa;		/* previous page address register */
	UINT8	pc;				/* program counter is a 7 bit register on tms0980, 6 bit register on tms1000/1070/1200/1270/1100/1300 */
	UINT8	pa;				/* page address register is a 4 bit register */
	UINT8	sr;				/* subroutine return register is a 7 bit register */
	UINT8	pb;				/* page buffer register is a 4 bit register */
	UINT8	a;				/* Accumulator is a 4 bit register (?) */
	UINT8	x;				/* X-register is a 2, 3, or 4 bit register */
	UINT8	y;				/* Y-register is a 4 bit register */
	UINT8	dam;			/* DAM register is a 4 bit register */
	UINT8	ca;				/* Chapter address bit */
	UINT8	cb;				/* Chapter buffer bit */
	UINT8	cs;				/* Chapter subroutine bit */
	UINT16	r;
	UINT8	o;
	UINT8	cki_bus;		/* CKI bus */
	UINT8	p;				/* adder p-input */
	UINT8	n;				/* adder n-input */
	UINT8	adder_result;	/* adder result */
	UINT8	carry_in;		/* carry in */
	UINT8	status;
	UINT8	status_latch;
	UINT8	special_status;
	UINT8	call_latch;
	UINT8	add_latch;
	UINT8	branch_latch;
	int	subcycle;
	UINT8	ram_address;
	UINT16	ram_data;
	UINT16	rom_address;
	UINT16	opcode;
	UINT32	decode;
	int		icount;
	UINT16	o_mask;			/* mask to determine the number of O outputs */
	UINT16	r_mask;			/* mask to determine the number of R outputs */
	UINT8	pc_size;		/* how bits in the PC register */
	UINT8	byte_size;		/* 8 or 9 bit bytes */
	UINT8	m_x_bits;		/* determine the number of bits in the X register */
	const UINT32 *decode_table;
	const tms0980_config	*config;
	address_space *program;
	address_space *data;

	devcb_resolved_read8 m_read_k;
	devcb_resolved_write16 m_write_o;
	devcb_resolved_write16 m_write_r;
};


static const UINT8 tms0980_c2_value[4] =
{
	0x00, 0x02, 0x01, 0x03
};
static const UINT8 tms0980_c3_value[8] =
{
	0x00, 0x04, 0x02, 0x06, 0x01, 0x05, 0x03, 0x07
};
static const UINT8 tms0980_c4_value[16] =
{
	0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F
};
static const UINT8 tms0980_bit_value[4] = { 1, 4, 2, 8 };
static const UINT8 tms0980_nbit_value[4] = { 0x0E, 0x0B, 0x0D, 0x07 };


static const UINT32 tms0980_decode[512] =
{
	/* 0x000 */
	F_COMX, I_ALEM, I_YNEA, I_XMA, I_DYN, I_IYC, I_CLA, I_DMAN,
	I_TKA, I_MNEA, I_TKM, F_ILL, F_ILL, F_SETR, I_KNEZ, F_ILL,
	I_DMEA, I_DNAA, I_CCLA, I_DMEA, F_ILL, I_AMAAC, F_ILL, F_ILL,
	I_CTMDYN, I_XDA, F_ILL, F_ILL, F_ILL, F_ILL, F_ILL, F_ILL,
	I_TBIT, I_TBIT, I_TBIT, I_TBIT, F_ILL, F_ILL, F_ILL, F_ILL,
	I_TAY, I_TMA, I_TMY, I_TYA, I_TAMDYN, I_TAMIYC, I_TAMZA, I_TAM,
	I_SAMAN, I_CPAIZ, I_IMAC, I_MNEZ, F_ILL, F_ILL, F_ILL, F_ILL,
	I_TCY, I_YNEC, I_TCMIY, I_ACACC, I_ACNAA, I_TAMACS, I_ALEC, I_YMCY,
	/* 0x040 */
	I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY,
	I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY,
	I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC,
	I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC,
	I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY,
	I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY,
	I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC,
	I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC, I_ACACC,
	/* 0x080 */
	F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP,
	F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP,
	F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX,
	F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX,
	F_SBIT, F_SBIT, F_SBIT, F_SBIT, F_RBIT, F_RBIT, F_RBIT, F_RBIT,
	F_ILL, F_ILL, F_ILL, F_ILL, F_ILL, F_ILL, F_ILL, F_ILL,
	F_TDO, F_SAL, F_COMX8, F_SBL, F_REAC, F_SEAC, F_OFF, F_ILL,
	F_ILL, F_ILL, F_ILL, F_ILL, F_ILL, F_ILL, F_ILL, F_RETN,
	/* 0x0c0 */
	I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA,
	I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA, I_ACNAA,
	I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS,
	I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS, I_TAMACS,
	I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC,
	I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC,
	I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY,
	I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY, I_YMCY,
	/* 0x100 */
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	/* 0x140 */
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	/* 0x180 */
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	/* 0x1c0 */
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL
};


static const UINT32 tms1000_default_decode[256] = {
	/* 0x00 */
	F_COMX, I_A8AAC, I_YNEA, I_TAM, I_TAMZA, I_A10AAC, I_A6AAC, I_DAN,
	I_TKA, I_KNEZ, F_TDO, F_CLO, F_RSTR, F_SETR, I_IA, F_RETN,
	F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP,
	F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP,
	/* 0x20 */
	I_TAMIY, I_TMA, I_TMY, I_TYA, I_TAY, I_AMAAC, I_MNEZ, I_SAMAN,
	I_IMAC, I_ALEM, I_DMAN, I_IYC, I_DYN, I_CPAIZ, I_XMA, I_CLA,
	F_SBIT, F_SBIT, F_SBIT, F_SBIT, F_RBIT, F_RBIT, F_RBIT, F_RBIT,
	I_TBIT, I_TBIT, I_TBIT, I_TBIT, F_LDX, F_LDX, F_LDX, F_LDX,
	/* 0x40 */
	I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY,
	I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY,
	I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC,
	I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC,
	/* 0x60 */
	I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY,
	I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY,
	I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC,
	I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC, I_ALEC,
	/* 0x80 */
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	/* 0xC0 */
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
};


static const UINT32 tms1100_default_decode[256] = {
	/* 0x00 */
	I_MNEA, I_ALEM, I_YNEA, I_XMA, I_DYN, I_IYC, I_AMAAC, I_DMAN,
	I_TKA, F_COMX, F_TDO, F_COMC, F_RSTR, F_SETR, I_KNEZ, F_RETN,
	F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP,
	F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP, F_LDP,
	/* 0x20 */
	I_TAY, I_TMA, I_TMY, I_TYA, I_TAMDYN, I_TAMIYC, I_TAMZA, I_TAM,
	F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX, F_LDX,
	F_SBIT, F_SBIT, F_SBIT, F_SBIT, F_RBIT, F_RBIT, F_RBIT, F_RBIT,
	I_TBIT, I_TBIT, I_TBIT, I_TBIT, I_SAMAN, I_CPAIZ, I_IMAC, I_MNEZ,
	/* 0x40 */
	I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY,
	I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY, I_TCY,
	I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC,
	I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC, I_YNEC,
	/* 0x60 */
	I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY,
	I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY, I_TCMIY,
	I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC,
	I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_AC1AC, I_CLA,
	/* 0x80 */
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR, F_BR,
	/* 0xC0 */
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
	F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL, F_CALL,
};


INLINE tms0980_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS0980 ||
			device->type() == TMS1000 ||
			device->type() == TMS1070 ||
			device->type() == TMS1100 ||
			device->type() == TMS1200 ||
			device->type() == TMS1270 ||
			device->type() == TMS1300 );
	return (tms0980_state *)downcast<legacy_cpu_device *>(device)->token();
}


static ADDRESS_MAP_START(tms0980_internal_rom, AS_PROGRAM, 16, legacy_cpu_device)
	AM_RANGE( 0x0000, 0x0FFF ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(tms0980_internal_ram, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE( 0x0000, 0x0FFF ) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(program_10bit_8, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE( 0x000, 0x3ff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(program_11bit_8, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE( 0x000, 0x7ff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_6bit, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE( 0x00, 0x3f ) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_7bit, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE( 0x00, 0x7f ) AM_RAM
ADDRESS_MAP_END


static void cpu_init_tms_common( legacy_cpu_device *device, const UINT32* decode_table, UINT16 o_mask, UINT16 r_mask, UINT8 pc_size, UINT8 byte_size, UINT8 x_bits )
{
	tms0980_state *cpustate = get_safe_token( device );

	cpustate->config = (const tms0980_config *) device->static_config();

	assert( cpustate->config != NULL );

	cpustate->decode_table = decode_table;
	cpustate->o_mask = o_mask;
	cpustate->r_mask = r_mask;
	cpustate->pc_size = pc_size;
	cpustate->byte_size = byte_size;
	cpustate->m_x_bits = x_bits;

	cpustate->program = &device->space( AS_PROGRAM );
	cpustate->data = &device->space( AS_DATA );

	cpustate->m_read_k.resolve(cpustate->config->read_k, *device);
	cpustate->m_write_o.resolve(cpustate->config->write_o, *device);
	cpustate->m_write_r.resolve(cpustate->config->write_r, *device);


	device->save_item( NAME(cpustate->prev_pc) );
	device->save_item( NAME(cpustate->prev_pa) );
	device->save_item( NAME(cpustate->pc) );
	device->save_item( NAME(cpustate->pa) );
	device->save_item( NAME(cpustate->sr) );
	device->save_item( NAME(cpustate->pb) );
	device->save_item( NAME(cpustate->a) );
	device->save_item( NAME(cpustate->x) );
	device->save_item( NAME(cpustate->y) );
	device->save_item( NAME(cpustate->dam) );
	device->save_item( NAME(cpustate->ca) );
	device->save_item( NAME(cpustate->cb) );
	device->save_item( NAME(cpustate->cs) );
	device->save_item( NAME(cpustate->r) );
	device->save_item( NAME(cpustate->o) );
	device->save_item( NAME(cpustate->cki_bus) );
	device->save_item( NAME(cpustate->p) );
	device->save_item( NAME(cpustate->n) );
	device->save_item( NAME(cpustate->adder_result) );
	device->save_item( NAME(cpustate->carry_in) );
	device->save_item( NAME(cpustate->status) );
	device->save_item( NAME(cpustate->status_latch) );
	device->save_item( NAME(cpustate->special_status) );
	device->save_item( NAME(cpustate->call_latch) );
	device->save_item( NAME(cpustate->add_latch) );
	device->save_item( NAME(cpustate->branch_latch) );
	device->save_item( NAME(cpustate->subcycle) );
	device->save_item( NAME(cpustate->ram_address) );
	device->save_item( NAME(cpustate->ram_data) );
	device->save_item( NAME(cpustate->rom_address) );
	device->save_item( NAME(cpustate->opcode) );
	device->save_item( NAME(cpustate->decode) );
}


static CPU_INIT( tms0980 )
{
	cpu_init_tms_common( device, tms0980_decode, 0x00ff, 0x07ff, 7, 9, 4 );
}


static CPU_INIT( tms1000 )
{
	cpu_init_tms_common( device, tms1000_default_decode, 0x00ff, 0x07ff, 6, 8, 2 );
}


static CPU_INIT( tms1070 )
{
	cpu_init_tms_common( device, tms1000_default_decode, 0x00ff, 0x07ff, 6, 8, 2 );
}


static CPU_INIT( tms1200 )
{
	cpu_init_tms_common( device, tms1000_default_decode, 0x00ff, 0x1fff, 6, 8, 2 );
}


static CPU_INIT( tms1270 )
{
	cpu_init_tms_common( device, tms1000_default_decode, 0x03ff, 0x1fff, 6, 8, 2 );
}


static CPU_INIT( tms1100 )
{
	cpu_init_tms_common( device, tms1100_default_decode, 0x00ff, 0x07ff, 6, 8, 3 );
}


static CPU_INIT( tms1300 )
{
	cpu_init_tms_common( device, tms1100_default_decode, 0x00ff, 0xffff, 6, 8, 3 );
}


static CPU_RESET( tms0980 )
{
	tms0980_state *cpustate = get_safe_token( device );

	cpustate->pa = 0x0F;
	cpustate->pb = 0x0F;
	cpustate->pc = 0;
	cpustate->dam = 0;
	cpustate->ca = 0;
	cpustate->cb = 0;
	cpustate->cs = 0;
	cpustate->subcycle = 0;
	cpustate->status = 1;
	cpustate->status_latch = 0;
	cpustate->call_latch = 0;
	cpustate->add_latch = 0;
	cpustate->branch_latch = 0;
	cpustate->r = 0;
	cpustate->o = 0;
	cpustate->ram_address = 0;
	cpustate->decode = F_ILL;
	cpustate->opcode = 0;
}


/*
The program counter is implemented using PRNG logic and gets incremented as follows:

00, 01, 03, 07, 0F, 1F, 3F, 3E,
3D, 3B, 37, 2F, 1E, 3C, 39, 33
27, 0E, 1D, 3A, 35, 2B, 16, 2C,
18, 30, 21, 02, 05, 0B, 17, 2E,
1C, 38, 31, 23, 06, 0D, 1B, 36,
2D, 1A, 34, 29, 12, 24, 08, 11,
22, 04, 09, 13, 26, 0C, 19, 32,
25, 0A, 15, 2A, 14, 28, 10, 20

There is also a strange address (AD) to location (LOC) mapping performed by the
tms1000 family.

From tms1000 family pdf:
AD          LOC
000 000000  003 000011
001 000001  004 000100
003 000011  00C 001100
007 000111  01C 011100
00F 001111  03C 111100
01F 011111  03F 111111
03F 111111  03E 111110
03E 111110  039 111001
03D 111101  036 110110
03B 111011  02E 101110
037 110111  01E 011110
02F 101111  03D 111101
01E 011110  038 111000
03C 111100  031 110001
039 111001  026 100110
033 110011  00E 001110
027 100111  01D 011101
00E 001110  03B 111011
01D 011101  037 110111
03A 111010  029 101001
035 110101  016 010110
02B 101011  02D 101101
016 010110  018 011000
02C 101100  032 110010
018 011000  020 100000
030 110000  001 000001
021 100001  005 000101
002 000010  00B 001011
005 000101  014 010100
00B 001011  02C 101100
017 010111  01F 011111
02E 101110  03A 111010
01C 011100  030 110000
038 111000  021 100001
031 110001  006 000110
023 100011  00D 001101
006 000110  01B 011011
00D 001101  034 110100
01B 011011  02F 101111
036 110110  019 011001
02D 101101  035 110101
01A 011010  028 101000
034 110100  011 010001
029 101001  025 100101
012 010010  008 001000
024 100100  012 010010
008 001000  023 100011
011 010001  007 000111
022 100010  00A 001010
004 000100  013 010011
009 001001  024 100100
013 010011  00F 001111
026 100110  01A 011010
00C 001100  033 110011
019 011001  027 100111
032 110010  009 001001
025 100101  015 010101
00A 001010  02B 101011
015 010101  017 010111
02A 101010  02A 101010
014 010100  010 010000
028 101000  022 100010
010 010000  000 000000
020 100000  002 000010

The following formula seems to be used to decode a program counter
into a rom address:
location{5:2} = pc{3:0}
location{1:0} =  ( pc{5:4} == 00 && pc{0} == 0 ) => 11
                 ( pc{5:4} == 00 && pc{0} == 1 ) => 00
                 ( pc{5:4} == 01 && pc{0} == 0 ) => 00
                 ( pc{5:4} == 01 && pc{0} == 1 ) => 11
                 ( pc{5:4} == 10 && pc{0} == 0 ) => 10
                 ( pc{5:4} == 10 && pc{0} == 1 ) => 01
                 ( pc{5:4} == 11 && pc{0} == 0 ) => 01
                 ( pc{5:4} == 11 && pc{0} == 1 ) => 10

*/
static const UINT8 tms1000_next_pc[64] = {
	0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F,
	0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3F,
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
	0x21, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33, 0x35, 0x37, 0x39, 0x3B, 0x3D, 0x3E,
};

/* emulator for the program counter increment on the tms0980/tmc0980 mcu;
 see patent 4064554 figure 19 (on page 13) for an explanation of feedback:

  nand324 = NAND of PC0 through pc4, i.e. output is true if ((pc&0x1f) != 0x1f)
  nand323 = NAND of pc5, pc6 and nand324
      i.e. output is true, if ((pc&0x1f)==0x1f) || pc5 is 0 || pc 6 is 0
  or321 = OR of pc5 and pc6, i.e. output is true if ((pc&0x60) != 0)
  nand322 = NAND of pc0 through pc5 plus /pc6,
      i.e. output is true if (pc != 0x3f)
  nand325 = nand pf nand323, or321 and nand322
      This one is complex:
      / or321 means if pc&0x60 is zero, output MUST be true
      \ nand323 means if (pc&0x60=0x60) && (pc&0x1f != 0x1f), output MUST be true
      nand322 means if pc = 0x3f, output MUST be true
      hence, nand325 is if pc = 0x7f, false. if pc = 0x3f, true. if pc&0x60 is zero OR pc&0x60 is 0x60, true. otherwise, false.

      tms0980_nect_pc below implements an indentical function to this in a somewhat more elegant way.
*/
INLINE void tms0980_next_pc( tms0980_state *cpustate )
{
	if ( cpustate->byte_size > 8 )
	{
		UINT8	xorval = ( cpustate->pc & 0x3F ) == 0x3F ? 1 : 0;
		UINT8	new_bit = ( ( cpustate->pc ^ ( cpustate->pc << 1 ) ) & 0x40 ) ? xorval : 1 - xorval;

		cpustate->pc = ( cpustate->pc << 1 ) | new_bit;
	}
	else
	{
		cpustate->pc = tms1000_next_pc[ cpustate->pc & 0x3f ];
	}
}


static const UINT8 tms1000_pc_decode[64] =
{
	0x03, 0x04, 0x0B, 0x0C, 0x13, 0x14, 0x1B, 0x1C,
	0x23, 0x24, 0x2B, 0x2C, 0x33, 0x34, 0x3B, 0x3C,
	0x00, 0x07, 0x08, 0x0F, 0x10, 0x17, 0x18, 0x1F,
	0x20, 0x27, 0x28, 0x2F, 0x30, 0x37, 0x38, 0x3F,
	0x02, 0x05, 0x0A, 0x0D, 0x12, 0x15, 0x1A, 0x1D,
	0x22, 0x25, 0x2A, 0x2D, 0x32, 0x35, 0x3A, 0x3D,
	0x01, 0x06, 0x09, 0x0E, 0x11, 0x16, 0x19, 0x1E,
	0x21, 0x26, 0x29, 0x2E, 0x31, 0x36, 0x39, 0x3E
};


static void tms0980_set_cki_bus( device_t *device )
{
	tms0980_state *cpustate = get_safe_token( device );

	switch( cpustate->opcode & 0x1F8 )
	{
	case 0x008:
		if ( !cpustate->m_read_k.isnull() )
		{
			cpustate->cki_bus = cpustate->m_read_k( 0, 0xff );
		}
		else
		{
			cpustate->cki_bus = 0x0F;
		}
		break;
	case 0x020: case 0x028:
		cpustate->cki_bus = 0;
		break;
	case 0x030: case 0x038:
		cpustate->cki_bus = tms0980_nbit_value[ cpustate->opcode & 0x03 ];
		break;
	case 0x000:
	case 0x040: case 0x048:
	case 0x050: case 0x058:
	case 0x060: case 0x068:
	case 0x070: case 0x078:
	case 0x080: case 0x088:
	case 0x090: case 0x098:
	case 0x0c0: case 0x0c8:
	case 0x0d0: case 0x0d8:
	case 0x0e0: case 0x0e8:
	case 0x0f0: case 0x0f8:
		cpustate->cki_bus = tms0980_c4_value[ cpustate->opcode & 0x0F ];
		break;
	default:
		cpustate->cki_bus = 0x0F;
		break;
	}
}


static CPU_EXECUTE( tms0980 )
{
	tms0980_state *cpustate = get_safe_token( device );

	do
	{
//      debugger_instruction_hook( device, ( ( cpustate->pa << cpustate->pc_size ) | cpustate->pc ) << 1 );
		cpustate->icount--;
		switch( cpustate->subcycle )
		{
		case 0:
			/* fetch: rom address 0 */
			/* execute: read ram, alu input, execute br/call, k input valid */
			tms0980_set_cki_bus( device );
			cpustate->ram_data = cpustate->data->read_byte( cpustate->ram_address );
			cpustate->status = 1;
			cpustate->p = 0;
			cpustate->n = 0;
			cpustate->carry_in = 0;
			break;
		case 1:
			/* fetch: rom address 1 */
			if ( cpustate->pc_size == 6 )
//				cpustate->rom_address = ( cpustate->pa << 6 ) | tms1000_pc_decode[ cpustate->pc ];
				cpustate->rom_address = ( cpustate->pa << 6 ) | cpustate->pc;
			else
				cpustate->rom_address = ( cpustate->pa << 7 ) | cpustate->pc;
			/* execute: k input valid */
			if ( cpustate->decode & MICRO_MASK )
			{
				/* Check N inputs */
				if ( cpustate->decode & ( M_15TN | M_ATN | M_CKN | M_MTN | M_NATN ) )
				{
					cpustate->n = 0;
					if ( cpustate->decode & M_15TN )
					{
						cpustate->n |= 0x0F;
					}
					if ( cpustate->decode & M_ATN )
					{
						cpustate->n |= cpustate->a;
					}
					if ( cpustate->decode & M_CKN )
					{
						cpustate->n |= cpustate->cki_bus;
					}
					if ( cpustate->decode & M_MTN )
					{
						cpustate->n |= cpustate->ram_data;
					}
					if ( cpustate->decode & M_NATN )
					{
						cpustate->n |= ( ( ~cpustate->a ) & 0x0F );
					}
				}


				/* Check P inputs */
				if ( cpustate->decode & ( M_CKP | M_DMTP | M_MTP | M_NDMTP | M_YTP ) )
				{
					cpustate->p = 0;
					if ( cpustate->decode & M_CKP )
					{
						cpustate->p |= cpustate->cki_bus;
					}
					if ( cpustate->decode & M_DMTP )
					{
						cpustate->p |= cpustate->dam;
					}
					if ( cpustate->decode & M_MTP )
					{
						cpustate->p |= cpustate->ram_data;
					}
					if ( cpustate->decode & M_NDMTP )
					{
						cpustate->p |= ( ( ~cpustate->dam ) & 0x0F );
					}
					if ( cpustate->decode & M_YTP )
					{
						cpustate->p |= cpustate->y;
					}
				}

				/* Carry In input */
				if ( cpustate->decode & M_CIN )
				{
					cpustate->carry_in = 1;
				}
			}
			break;
		case 2:
			/* fetch: nothing */
			/* execute: write ram */
			/* perform adder logic */
			cpustate->adder_result = cpustate->p + cpustate->n + cpustate->carry_in;
			if ( cpustate->decode & MICRO_MASK )
			{
				if ( cpustate->decode & M_NE )
				{
					if ( cpustate->n == cpustate->p )
					{
						cpustate->status = 0;
					}
				}
				if ( cpustate->decode & M_C8 )
				{
					cpustate->status = cpustate->adder_result >> 4;
				}
				if ( cpustate->decode & M_STO )
				{
					cpustate->data->write_byte( cpustate->ram_address, cpustate->a );
				}
				if ( cpustate->decode & M_CKM )
				{
					cpustate->data->write_byte( cpustate->ram_address, cpustate->cki_bus );
				}
			}
			else
			{
				if ( cpustate->decode & F_SBIT )
				{
					cpustate->data->write_byte( cpustate->ram_address, cpustate->ram_data | tms0980_bit_value[ cpustate->opcode & 0x03 ] );
				}
				if ( cpustate->decode & F_RBIT )
				{
					cpustate->data->write_byte( cpustate->ram_address, cpustate->ram_data & tms0980_nbit_value[ cpustate->opcode & 0x03 ] );
				}
				if ( cpustate->decode & F_SETR )
				{
					cpustate->r = cpustate->r | ( 1 << cpustate->y );
					if ( !cpustate->m_write_r.isnull() )
					{
						cpustate->m_write_r( 0, cpustate->r & cpustate->r_mask, 0xffff );
					}
				}
				if ( cpustate->decode & F_RSTR )
				{
					cpustate->r = cpustate->r & ( ~( 1 << cpustate->y ) );
					if ( !cpustate->m_write_r.isnull() )
					{
						cpustate->m_write_r( 0, cpustate->r & cpustate->r_mask, 0xffff );
					}
				}
				if ( cpustate->decode & F_TDO )
				{
					int i = 0;

					/* Calculate O-outputs based on status latch, A, and the output PLA configuration */
					cpustate->o = 0;
					for ( i = 0; i < 20; i++ )
					{
						if ( ( ( cpustate->status_latch << 4 ) | cpustate->a ) == cpustate->config->o_pla[i].value )
						{
							cpustate->o = cpustate->config->o_pla[i].output;
						}
					}

					if ( !cpustate->m_write_o.isnull() )
					{
						cpustate->m_write_o( 0, cpustate->o & cpustate->o_mask, 0xffff );
					}
				}
				if ( cpustate->decode & F_CLO )
				{
					cpustate->o = 0;
					if ( !cpustate->m_write_o.isnull() )
					{
						cpustate->m_write_o( 0, cpustate->o & cpustate->o_mask, 0xffff );
					}
				}
				if ( cpustate->decode & F_LDX )
				{
					switch( cpustate->m_x_bits )
					{
						case 2:
							cpustate->x = tms0980_c2_value[ cpustate->opcode & 0x03 ];
							break;
						case 3:
							cpustate->x = tms0980_c3_value[ cpustate->opcode & 0x07 ];
							break;
						case 4:
							cpustate->x = tms0980_c4_value[ cpustate->opcode & 0x0f ];
							break;
					}
				}
				if ( cpustate->decode & F_COMX )
				{
					cpustate->x = cpustate->x ^ 0x03;
				}
				if ( cpustate->decode & F_COMC )
				{
					cpustate->cb = cpustate->cb ^ 0x01;
				}
				if ( cpustate->decode & F_LDP )
				{
					cpustate->pb = tms0980_c4_value[ cpustate->opcode & 0x0F ];
				}
				if ( cpustate->decode & F_REAC )
				{
					cpustate->special_status = 0;
				}
				if ( cpustate->decode & F_SEAC )
				{
					cpustate->special_status = 1;
				}
				if ( cpustate->decode == F_SAL )
				{
					cpustate->add_latch = 1;
				}
				if ( cpustate->decode == F_SBL )
				{
					cpustate->branch_latch = 1;
				}
			}
			break;
		case 3:
			/* fetch: fetch, update pc, ram address */
			/* execute: register store */
			break;
		case 4:
			/* execute: register store */
			if ( cpustate->decode & MICRO_MASK )
			{
				if ( cpustate->decode & M_AUTA )
				{
					cpustate->a = cpustate->adder_result & 0x0F;
				}
				if ( cpustate->decode & M_AUTY )
				{
					cpustate->y = cpustate->adder_result & 0x0F;
				}
				if ( cpustate->decode & M_STSL )
				{
					cpustate->status_latch = cpustate->status;
				}
			}
			/* fetch: fetch, update pc, ram address */
			if ( cpustate->byte_size > 8 )
			{
				debugger_instruction_hook( device, cpustate->rom_address << 1 );
				cpustate->opcode = cpustate->program->read_word( cpustate->rom_address << 1 ) & 0x1FF;
			}
			else
			{
				debugger_instruction_hook( device, cpustate->rom_address );
				cpustate->opcode = cpustate->program->read_byte( cpustate->rom_address );
			}
			tms0980_next_pc( cpustate );
			if (LOG)
				logerror( "tms0980: read opcode %04x from %04x. Set pc to %04x\n", cpustate->opcode, cpustate->rom_address, cpustate->pc );

			/* ram address */
			cpustate->ram_address = ( cpustate->x << 4 ) | cpustate->y;
			break;
		case 5:
			/* fetch: instruction decode */
			cpustate->decode = cpustate->decode_table[ cpustate->opcode ];
			/* execute: execute br/call */
			if ( cpustate->status )
			{
				if ( cpustate->decode == F_BR )
				{
					if ( cpustate->call_latch == 0 )
					{
						cpustate->pa = cpustate->pb;
					}
					cpustate->pc = cpustate->opcode & ( ( 1 << cpustate->pc_size ) - 1 );
				}
				if ( cpustate->decode == F_CALL )
				{
					UINT8 t = cpustate->pa;
					if ( cpustate->call_latch == 0 )
					{
						cpustate->sr = cpustate->pc;
						cpustate->call_latch = 1;
						cpustate->pa = cpustate->pb;
					}
					cpustate->pb = t;
					cpustate->pc = cpustate->opcode & ( ( 1 << cpustate->pc_size ) - 1 );
				}
			}
			if ( cpustate->decode == F_RETN )
			{
				if ( cpustate->call_latch == 1 )
				{
					cpustate->pc = cpustate->sr;
					cpustate->call_latch = 0;
				}
				cpustate->add_latch = 0;
				cpustate->pa = cpustate->pb;
			} else {
				cpustate->branch_latch = 0;
			}
			break;
		}
		cpustate->subcycle = ( cpustate->subcycle + 1 ) % 6;
	} while( cpustate->icount > 0 );
}


static CPU_SET_INFO( tms0980 )
{
	tms0980_state *cpustate = get_safe_token( device );

	switch( state )
	{
		case CPUINFO_INT_PC:										cpustate->pc = ( info->i >> 1 ) & 0x7f; cpustate->pa = info->i >> 8; break;
		case CPUINFO_INT_REGISTER + TMS0980_PC:						cpustate->pc = info->i; break;
		case CPUINFO_INT_REGISTER + TMS0980_SR:						cpustate->sr = info->i; break;
		case CPUINFO_INT_REGISTER + TMS0980_PA:						cpustate->pa = info->i; break;
		case CPUINFO_INT_REGISTER + TMS0980_PB:						cpustate->pb = info->i; break;
		case CPUINFO_INT_REGISTER + TMS0980_A:						cpustate->a = info->i; break;
		case CPUINFO_INT_REGISTER + TMS0980_X:						cpustate->x = info->i; break;
		case CPUINFO_INT_REGISTER + TMS0980_Y:						cpustate->y = info->i; break;
		case CPUINFO_INT_REGISTER + TMS0980_STATUS:					cpustate->status = info->i; break;
	}
}


static CPU_GET_INFO( tms_generic )
{
	tms0980_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		case CPUINFO_INT_CONTEXT_SIZE:									info->i = sizeof(tms0980_state); break;
		case CPUINFO_INT_INPUT_LINES:									info->i = 1; break;
		case CPUINFO_INT_ENDIANNESS:									info->i = ENDIANNESS_BIG; break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:								info->i = 1; break;
		case CPUINFO_INT_CLOCK_DIVIDER:									info->i = 1; break;
		case CPUINFO_INT_MIN_CYCLES:									info->i = 1; break;
		case CPUINFO_INT_MAX_CYCLES:									info->i = 6; break;

		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:			info->i = 0; break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 8 /* 4 */; break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0; break;

		case CPUINFO_INT_PREVIOUSPC:									info->i = ( ( cpustate->prev_pa << 7 ) | cpustate->prev_pc ) << 1; break;
		case CPUINFO_INT_PC:											info->i = ( ( cpustate->pa << 7 ) | cpustate->pc ) << 1; break;
		case CPUINFO_INT_SP:											info->i = 0xFFFF; break;
		case CPUINFO_INT_REGISTER + TMS0980_PC:							info->i = cpustate->pc; break;
		case CPUINFO_INT_REGISTER + TMS0980_SR:							info->i = cpustate->sr; break;
		case CPUINFO_INT_REGISTER + TMS0980_PA:							info->i = cpustate->pa; break;
		case CPUINFO_INT_REGISTER + TMS0980_PB:							info->i = cpustate->pb; break;
		case CPUINFO_INT_REGISTER + TMS0980_A:							info->i = cpustate->a; break;
		case CPUINFO_INT_REGISTER + TMS0980_X:							info->i = cpustate->x; break;
		case CPUINFO_INT_REGISTER + TMS0980_Y:							info->i = cpustate->y; break;
		case CPUINFO_INT_REGISTER + TMS0980_STATUS:						info->i = cpustate->status; break;

		case CPUINFO_FCT_SET_INFO:										info->setinfo = CPU_SET_INFO_NAME( tms0980 ); break;
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms0980 ); break;
		case CPUINFO_FCT_RESET:											info->reset = CPU_RESET_NAME( tms0980 ); break;
		case CPUINFO_FCT_EXECUTE:										info->execute = CPU_EXECUTE_NAME( tms0980 ); break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:							info->icount = &cpustate->icount; break;

		case CPUINFO_STR_FAMILY:										strcpy( info->s, "Texas Instruments TMS0980/TMS1000" ); break;
		case CPUINFO_STR_VERSION:										strcpy( info->s, "0.2" ); break;
		case CPUINFO_STR_SOURCE_FILE:									strcpy( info->s, __FILE__ ); break;
		case CPUINFO_STR_CREDITS:										strcpy( info->s, "Copyright the MESS and MAME teams" ); break;

		case CPUINFO_STR_FLAGS:											strcpy( info->s, "N/A" ); break;

		case CPUINFO_STR_REGISTER + TMS0980_PC:							sprintf( info->s, "PC:%02X", cpustate->pc ); break;
		case CPUINFO_STR_REGISTER + TMS0980_SR:							sprintf( info->s, "SR:%01X", cpustate->sr ); break;
		case CPUINFO_STR_REGISTER + TMS0980_PA:							sprintf( info->s, "PA:%01X", cpustate->pa ); break;
		case CPUINFO_STR_REGISTER + TMS0980_PB:							sprintf( info->s, "PB:%01X", cpustate->pb ); break;
		case CPUINFO_STR_REGISTER + TMS0980_A:							sprintf( info->s, "A:%01X", cpustate->a ); break;
		case CPUINFO_STR_REGISTER + TMS0980_X:							sprintf( info->s, "X:%01X", cpustate->x ); break;
		case CPUINFO_STR_REGISTER + TMS0980_Y:							sprintf( info->s, "Y:%01X", cpustate->y ); break;
		case CPUINFO_STR_REGISTER + TMS0980_STATUS:						sprintf( info->s, "STATUS:%01X", cpustate->status ); break;

	}
}


CPU_GET_INFO( tms0980 )
{
	tms0980_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:							info->i = 2; break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:							info->i = 2; break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 16 /* 9 */; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 12; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 7; break;
		case CPUINFO_INT_PREVIOUSPC:									info->i = ( ( cpustate->prev_pa << 7 ) | cpustate->prev_pc ) << 1; break;
		case CPUINFO_INT_PC:											info->i = ( ( cpustate->pa << 7 ) | cpustate->pc ) << 1; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:					info->internal_map16 = ADDRESS_MAP_NAME( tms0980_internal_rom ); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:						info->internal_map8 = ADDRESS_MAP_NAME( tms0980_internal_ram ); break;
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms0980 ); break;
		case CPUINFO_FCT_DISASSEMBLE:									info->disassemble = CPU_DISASSEMBLE_NAME( tms0980 ); break;
		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS0980" ); break;
		default:														CPU_GET_INFO_CALL( tms_generic );
	}
}


CPU_GET_INFO( tms1000 )
{
	tms0980_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:							info->i = 1; break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:							info->i = 1; break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 10; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 6; break;
		case CPUINFO_INT_PREVIOUSPC:									info->i = ( cpustate->prev_pa << 6 ) | tms1000_pc_decode[ cpustate->prev_pc ]; break;
		case CPUINFO_INT_PC:											info->i = ( cpustate->pa << 6 ) | tms1000_pc_decode[ cpustate->pc ]; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:					info->internal_map8 = ADDRESS_MAP_NAME( program_10bit_8 ); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:						info->internal_map8 = ADDRESS_MAP_NAME( data_6bit ); break;
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms1000 ); break;
		case CPUINFO_FCT_DISASSEMBLE:									info->disassemble = CPU_DISASSEMBLE_NAME( tms1000 ); break;
		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS1000" ); break;
		default:														CPU_GET_INFO_CALL( tms_generic );
	}
}


CPU_GET_INFO( tms1070 )
{
	switch(state)
	{
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms1070 ); break;
		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS1070" ); break;
		default:														CPU_GET_INFO_CALL( tms1000 );
	}
}


CPU_GET_INFO( tms1200 )
{
	switch(state)
	{
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms1200 ); break;
		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS1200" ); break;
		default:														CPU_GET_INFO_CALL( tms1000 );
	}
}


CPU_GET_INFO( tms1270 )
{
	switch(state)
	{
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms1270 ); break;
		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS1270" ); break;
		default:														CPU_GET_INFO_CALL( tms1000 );
	}
}



CPU_GET_INFO( tms1100 )
{
	tms0980_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:							info->i = 1; break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:							info->i = 1; break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 11; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 7; break;
		case CPUINFO_INT_PREVIOUSPC:									info->i = ( cpustate->prev_pa << 6 ) | tms1000_pc_decode[ cpustate->prev_pc ]; break;
		case CPUINFO_INT_PC:											info->i = ( cpustate->pa << 6 ) | cpustate->pc; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:					info->internal_map8 = ADDRESS_MAP_NAME( program_11bit_8 ); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:						info->internal_map8 = ADDRESS_MAP_NAME( data_7bit ); break;
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms1100 ); break;
		case CPUINFO_FCT_DISASSEMBLE:									info->disassemble = CPU_DISASSEMBLE_NAME( tms1100 ); break;
		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS1100" ); break;
		default:														CPU_GET_INFO_CALL( tms_generic );
	}
}


CPU_GET_INFO( tms1300 )
{
	switch(state)
	{
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms1300 ); break;
		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS1300" ); break;
		default:														CPU_GET_INFO_CALL( tms1100 );
	}
}


DEFINE_LEGACY_CPU_DEVICE(TMS0980, tms0980);

DEFINE_LEGACY_CPU_DEVICE(TMS1000, tms1000);
DEFINE_LEGACY_CPU_DEVICE(TMS1070, tms1070);
DEFINE_LEGACY_CPU_DEVICE(TMS1100, tms1100);
DEFINE_LEGACY_CPU_DEVICE(TMS1200, tms1200);
DEFINE_LEGACY_CPU_DEVICE(TMS1270, tms1270);
DEFINE_LEGACY_CPU_DEVICE(TMS1300, tms1300);
