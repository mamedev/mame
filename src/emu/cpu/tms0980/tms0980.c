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



const device_type TMS0980 = &device_creator<tms0980_cpu_device>;
const device_type TMS1000 = &device_creator<tms1000_cpu_device>;
const device_type TMS1070 = &device_creator<tms1070_cpu_device>;
const device_type TMS1200 = &device_creator<tms1200_cpu_device>;
const device_type TMS1270 = &device_creator<tms1270_cpu_device>;
const device_type TMS1100 = &device_creator<tms1100_cpu_device>;
const device_type TMS1300 = &device_creator<tms1300_cpu_device>;


#define MICRO_MASK          0x80000000
#define FIXED_INSTRUCTION   0x00000000


/* Standard/fixed intructions */
#define F_ILL               0x00000000
#define F_BR                0x00000001
#define F_CALL              0x00000002
#define F_CLO               0x00000004
#define F_COMC              0x00000008
#define F_COMX              0x00000010
#define F_COMX8             0x00000020
#define F_LDP               0x00000040
#define F_LDX               0x00000080
#define F_OFF               0x00000100
#define F_RBIT              0x00000200
#define F_REAC              0x00000400
#define F_RETN              0x00000800
#define F_RSTR              0x00001000
#define F_SAL               0x00002000
#define F_SBIT              0x00004000
#define F_SBL               0x00008000
#define F_SEAC              0x00010000
#define F_SETR              0x00020000
#define F_TDO               0x00040000


/* Microinstructions */
#define M_15TN              0x00000001
#define M_ATN               0x00000002
#define M_AUTA              0x00000004
#define M_AUTY              0x00000008
#define M_C8                0x00000010
#define M_CIN               0x00000020
#define M_CKM               0x00000040
#define M_CKN               0x00000080
#define M_CKP               0x00000100
#define M_CME               0x00000200
#define M_DMTP              0x00000400
#define M_MTN               0x00000800
#define M_MTP               0x00001000
#define M_NATN              0x00002000
#define M_NDMTP             0x00004000
#define M_NE                0x00008000
#define M_SSE               0x00010000
#define M_SSS               0x00020000
#define M_STO               0x00040000
#define M_STSL              0x00080000
#define M_YTP               0x00100000


/* instructions built from microinstructions */
#define I_AC1AC     ( MICRO_MASK | M_CKP | M_ATN | M_CIN | M_C8 | M_AUTA )
#define I_A6AAC     I_ACACC
#define I_A8AAC     I_ACACC
#define I_A10AAC    I_ACACC
#define I_ACACC     ( MICRO_MASK | M_CKP | M_ATN | M_C8 | M_AUTA )
#define I_ACNAA     ( MICRO_MASK | M_CKP | M_NATN | M_AUTA )
#define I_ALEC      ( MICRO_MASK | M_CKP | M_NATN | M_CIN | M_C8 )
#define I_ALEM      ( MICRO_MASK | M_MTP | M_NATN | M_CIN | M_C8 )
#define I_AMAAC     ( MICRO_MASK | M_MTP | M_ATN | M_C8 | M_AUTA )
#define I_CCLA      ( MICRO_MASK | M_AUTA | M_SSS )
#define I_CLA       ( MICRO_MASK | M_AUTA )
#define I_CPAIZ     ( MICRO_MASK | M_NATN | M_CIN | M_C8 | M_AUTA )
#define I_CTMDYN    ( MICRO_MASK | M_YTP | M_15TN | M_C8 | M_AUTY | M_CME )
#define I_DAN       ( MICRO_MASK | M_CKP | M_ATN | M_CIN | M_C8 | M_AUTA )
#define I_DMAN      ( MICRO_MASK | M_MTP | M_15TN | M_C8 | M_AUTA )
#define I_DMEA      ( MICRO_MASK | M_MTP | M_DMTP | M_SSS | M_AUTA )
#define I_DNAA      ( MICRO_MASK | M_DMTP | M_NATN | M_SSS | M_AUTA )
#define I_DYN       ( MICRO_MASK | M_YTP | M_15TN | M_C8 | M_AUTY )
#define I_IA        ( MICRO_MASK | M_ATN | M_CIN | M_AUTA )
#define I_IMAC      ( MICRO_MASK | M_MTP | M_CIN | M_C8 | M_AUTA )
#define I_IYC       ( MICRO_MASK | M_YTP | M_CIN | M_C8 | M_AUTY )
#define I_KNEZ      ( MICRO_MASK | M_CKP | M_NE )
#define I_MNEA      ( MICRO_MASK | M_MTP | M_ATN | M_NE )
#define I_MNEZ      ( MICRO_MASK | M_MTP | M_NE )
#define I_M_NDMEA   ( MICRO_MASK | M_MTN | M_NDTMP | M_SSS | M_AUTA )
#define I_SAMAN     ( MICRO_MASK | M_MTP | M_NATN | M_CIN | M_C8 | M_AUTA )
#define I_SETR      ( MICRO_MASK | M_YTP | M_15TN | M_AUTY | M_C8 )
#define I_TAM       ( MICRO_MASK | M_STO )
#define I_TAMACS    ( MICRO_MASK | M_STO | M_ATN | M_CKP | M_AUTA | M_SSE )
#define I_TAMDYN    ( MICRO_MASK | M_STO | M_YTP | M_15TN | M_AUTY | M_C8 )
#define I_TAMIY     ( MICRO_MASK | M_STO | M_YTP | M_CIN | M_AUTY )
#define I_TAMIYC    ( MICRO_MASK | M_STO | M_YTP | M_CIN | M_C8 | M_AUTY )
#define I_TAMZA     ( MICRO_MASK | M_STO | M_AUTA )
#define I_TAY       ( MICRO_MASK | M_ATN | M_AUTY )
#define I_TBIT      ( MICRO_MASK | M_CKP | M_CKN | M_MTP | M_NE )
#define I_TCY       ( MICRO_MASK | M_CKP | M_AUTY )
#define I_TCMIY     ( MICRO_MASK | M_CKM | M_YTP | M_CIN | M_AUTY )
#define I_TKA       ( MICRO_MASK | M_CKP | M_AUTA )
#define I_TKM       ( MICRO_MASK | M_CKM )
#define I_TMA       ( MICRO_MASK | M_MTP | M_AUTA )
#define I_TMY       ( MICRO_MASK | M_MTP | M_AUTY )
#define I_TYA       ( MICRO_MASK | M_YTP | M_AUTA )
#define I_XDA       ( MICRO_MASK | M_DMTP | M_AUTA | M_STO )
#define I_XMA       ( MICRO_MASK | M_MTP | M_STO | M_AUTA )
#define I_YMCY      ( MICRO_MASK | M_CIN | M_YTP | M_CKN | M_AUTY )
#define I_YNEA      ( MICRO_MASK | M_YTP | M_ATN | M_NE )
#define I_YNEC      ( MICRO_MASK | M_YTP | M_CKN | M_NE )


static const UINT8 tms0980_c2_value[4] = { 0, 2, 1, 3 };
static const UINT8 tms0980_c3_value[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
static const UINT8 tms0980_c4_value[16] = { 0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF };
static const UINT8 tms0980_bit_value[4] = { 1, 4, 2, 8 };
static const UINT8 tms0980_nbit_value[4] = { 0xE, 0xB, 0xD, 0x7 };


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


static const UINT32 tms1000_default_decode[256] =
{
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


static const UINT32 tms1100_default_decode[256] =
{
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


static ADDRESS_MAP_START(tms0980_internal_rom, AS_PROGRAM, 16, tms1xxx_cpu_device)
	AM_RANGE( 0x0000, 0x0FFF ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(tms0980_internal_ram, AS_DATA, 8, tms1xxx_cpu_device)
	AM_RANGE( 0x0000, 0x0FFF ) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(program_10bit_8, AS_PROGRAM, 8, tms1xxx_cpu_device)
	AM_RANGE( 0x000, 0x3ff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(program_11bit_8, AS_PROGRAM, 8, tms1xxx_cpu_device)
	AM_RANGE( 0x000, 0x7ff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_6bit, AS_DATA, 8, tms1xxx_cpu_device)
	AM_RANGE( 0x00, 0x3f ) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_7bit, AS_DATA, 8, tms1xxx_cpu_device)
	AM_RANGE( 0x00, 0x7f ) AM_RAM
ADDRESS_MAP_END


void tms1xxx_cpu_device::device_start()
{
	m_program = &space( AS_PROGRAM );
	m_data = &space( AS_DATA );

	m_read_k.resolve_safe(0xff);
	m_write_o.resolve_safe();
	m_write_r.resolve_safe();

	save_item( NAME(m_prev_pc) );
	save_item( NAME(m_prev_pa) );
	save_item( NAME(m_pc) );
	save_item( NAME(m_pa) );
	save_item( NAME(m_sr) );
	save_item( NAME(m_pb) );
	save_item( NAME(m_a) );
	save_item( NAME(m_x) );
	save_item( NAME(m_y) );
	save_item( NAME(m_dam) );
	save_item( NAME(m_ca) );
	save_item( NAME(m_cb) );
	save_item( NAME(m_cs) );
	save_item( NAME(m_r) );
	save_item( NAME(m_o) );
	save_item( NAME(m_cki_bus) );
	save_item( NAME(m_p) );
	save_item( NAME(m_n) );
	save_item( NAME(m_adder_result) );
	save_item( NAME(m_carry_in) );
	save_item( NAME(m_status) );
	save_item( NAME(m_status_latch) );
	save_item( NAME(m_special_status) );
	save_item( NAME(m_call_latch) );
	save_item( NAME(m_add_latch) );
	save_item( NAME(m_branch_latch) );
	save_item( NAME(m_subcycle) );
	save_item( NAME(m_ram_address) );
	save_item( NAME(m_ram_data) );
	save_item( NAME(m_rom_address) );
	save_item( NAME(m_opcode) );
	save_item( NAME(m_decode) );

	// Register state for debugger
	state_add( TMS0980_PC,     "PC",     m_pc     ).callimport().callexport().formatstr("%02X");
	state_add( TMS0980_SR,     "SR",     m_sr     ).callimport().callexport().formatstr("%01X");
	state_add( TMS0980_PA,     "PA",     m_pa     ).callimport().callexport().formatstr("%01X");
	state_add( TMS0980_PB,     "PB",     m_pb     ).callimport().callexport().formatstr("%01X");
	state_add( TMS0980_A,      "A",      m_a      ).callimport().callexport().formatstr("%01X");
	state_add( TMS0980_X,      "X",      m_x      ).callimport().callexport().formatstr("%01X");
	state_add( TMS0980_Y,      "Y",      m_y      ).callimport().callexport().formatstr("%01X");
	state_add( TMS0980_STATUS, "STATUS", m_status ).callimport().callexport().formatstr("%01X");

	state_add(STATE_GENPC, "curpc", m_pc).callimport().callexport().formatstr("%8s").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_sr).callimport().callexport().formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}


void tms1xxx_cpu_device::device_reset()
{
	m_pa = 0xF;
	m_pb = 0xF;
	m_pc = 0;
	m_dam = 0;
	m_ca = 0;
	m_cb = 0;
	m_cs = 0;
	m_subcycle = 0;
	m_status = 1;
	m_status_latch = 0;
	m_call_latch = 0;
	m_add_latch = 0;
	m_branch_latch = 0;
	m_r = 0;
	m_o = 0;
	m_ram_address = 0;
	m_decode = F_ILL;
	m_opcode = 0;
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
static const UINT8 tms1000_next_pc[64] =
{
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
void tms1xxx_cpu_device::next_pc()
{
	if ( m_byte_size > 8 )
	{
		UINT8   xorval = ( m_pc & 0x3F ) == 0x3F ? 1 : 0;
		UINT8   new_bit = ( ( m_pc ^ ( m_pc << 1 ) ) & 0x40 ) ? xorval : 1 - xorval;

		m_pc = ( m_pc << 1 ) | new_bit;
	}
	else
	{
		m_pc = tms1000_next_pc[ m_pc & 0x3f ];
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


void tms1xxx_cpu_device::set_cki_bus()
{
	switch( m_opcode & 0x1F8 )
	{
	case 0x008:
		m_cki_bus = m_read_k( 0, 0xff );
		break;
	case 0x020: case 0x028:
		m_cki_bus = 0;
		break;
	case 0x030: case 0x038:
		m_cki_bus = tms0980_nbit_value[ m_opcode & 0x03 ];
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
		m_cki_bus = tms0980_c4_value[ m_opcode & 0x0F ];
		break;
	default:
		m_cki_bus = 0x0F;
		break;
	}
}


void tms1xxx_cpu_device::execute_run()
{
	do
	{
		m_icount--;
		switch( m_subcycle )
		{
		case 0:
			/* fetch: rom address 0 */
			/* execute: read ram, alu input, execute br/call, k input valid */
			set_cki_bus();
			m_ram_data = m_data->read_byte( m_ram_address );
			m_status = 1;
			m_p = 0;
			m_n = 0;
			m_carry_in = 0;
			break;
		case 1:
			/* fetch: rom address 1 */
			m_rom_address = ( m_ca << ( m_pc_size + 4 ) ) | ( m_pa << m_pc_size ) | m_pc;
			/* execute: k input valid */
			if ( m_decode & MICRO_MASK )
			{
				/* Check N inputs */
				if ( m_decode & ( M_15TN | M_ATN | M_CKN | M_MTN | M_NATN ) )
				{
					m_n = 0;
					if ( m_decode & M_15TN )
					{
						m_n |= 0x0F;
					}
					if ( m_decode & M_ATN )
					{
						m_n |= m_a;
					}
					if ( m_decode & M_CKN )
					{
						m_n |= m_cki_bus;
					}
					if ( m_decode & M_MTN )
					{
						m_n |= m_ram_data;
					}
					if ( m_decode & M_NATN )
					{
						m_n |= ( ( ~m_a ) & 0x0F );
					}
				}


				/* Check P inputs */
				if ( m_decode & ( M_CKP | M_DMTP | M_MTP | M_NDMTP | M_YTP ) )
				{
					m_p = 0;
					if ( m_decode & M_CKP )
					{
						m_p |= m_cki_bus;
					}
					if ( m_decode & M_DMTP )
					{
						m_p |= m_dam;
					}
					if ( m_decode & M_MTP )
					{
						m_p |= m_ram_data;
					}
					if ( m_decode & M_NDMTP )
					{
						m_p |= ( ( ~m_dam ) & 0x0F );
					}
					if ( m_decode & M_YTP )
					{
						m_p |= m_y;
					}
				}

				/* Carry In input */
				if ( m_decode & M_CIN )
				{
					m_carry_in = 1;
				}
			}
			break;
		case 2:
			/* fetch: nothing */
			/* execute: write ram */
			/* perform adder logic */
			m_adder_result = m_p + m_n + m_carry_in;
			if ( m_decode & MICRO_MASK )
			{
				if ( m_decode & M_NE )
				{
					if ( m_n == m_p )
					{
						m_status = 0;
					}
				}
				if ( m_decode & M_C8 )
				{
					m_status = m_adder_result >> 4;
				}
				if ( m_decode & M_STO )
				{
					m_data->write_byte( m_ram_address, m_a );
				}
				if ( m_decode & M_CKM )
				{
					m_data->write_byte( m_ram_address, m_cki_bus );
				}
			}
			else
			{
				if ( m_decode & F_SBIT )
				{
					m_data->write_byte( m_ram_address, m_ram_data | tms0980_bit_value[ m_opcode & 0x03 ] );
				}
				if ( m_decode & F_RBIT )
				{
					m_data->write_byte( m_ram_address, m_ram_data & tms0980_nbit_value[ m_opcode & 0x03 ] );
				}
				if ( m_decode & F_SETR )
				{
					m_r = m_r | ( 1 << m_y );
					m_write_r( 0, m_r & m_r_mask, 0xffff );
				}
				if ( m_decode & F_RSTR )
				{
					m_r = m_r & ( ~( 1 << m_y ) );
					m_write_r( 0, m_r & m_r_mask, 0xffff );
				}
				if ( m_decode & F_TDO )
				{
					/* Calculate O-outputs based on status latch, A, and the output PLA configuration */
					m_o = c_output_pla[ ( m_status_latch << 4 ) | m_a ];
					if ( ( c_output_pla[ ( m_status_latch << 4 ) | m_a ] & 0xFF00 ) == 0xFF00 )
					{
						logerror("unknown output pla mapping for status latch = %d and a = %X\n", m_status_latch, m_a);
					}

					m_write_o( 0, m_o & m_o_mask, 0xffff );
				}
				if ( m_decode & F_CLO )
				{
					m_o = 0;
					m_write_o( 0, m_o & m_o_mask, 0xffff );
				}
				if ( m_decode & F_LDX )
				{
					switch( m_x_bits )
					{
						case 2:
							m_x = tms0980_c2_value[ m_opcode & 0x03 ];
							break;
						case 3:
							m_x = tms0980_c3_value[ m_opcode & 0x07 ];
							break;
						case 4:
							m_x = tms0980_c4_value[ m_opcode & 0x0f ];
							break;
					}
				}
				if ( m_decode & F_COMX )
				{
					switch ( m_x_bits )
					{
						case 2:
							m_x = m_x ^ 0x03;
							break;
						case 3:
							m_x = m_x ^ 0x07;
							break;
						case 4:
							m_x = m_x ^ 0x0f;
							break;
					}
				}
				if ( m_decode & F_COMC )
				{
					m_cb = m_cb ^ 0x01;
				}
				if ( m_decode & F_LDP )
				{
					m_pb = tms0980_c4_value[ m_opcode & 0x0F ];
				}
				if ( m_decode & F_REAC )
				{
					m_special_status = 0;
				}
				if ( m_decode & F_SEAC )
				{
					m_special_status = 1;
				}
				if ( m_decode == F_SAL )
				{
					m_add_latch = 1;
				}
				if ( m_decode == F_SBL )
				{
					m_branch_latch = 1;
				}
			}
			break;
		case 3:
			/* fetch: fetch, update pc, ram address */
			/* execute: register store */
			break;
		case 4:
			/* execute: register store */
			if ( m_decode & MICRO_MASK )
			{
				if ( m_decode & M_AUTA )
				{
					m_a = m_adder_result & 0x0F;
				}
				if ( m_decode & M_AUTY )
				{
					m_y = m_adder_result & 0x0F;
				}
				if ( m_decode & M_STSL )
				{
					m_status_latch = m_status;
				}
			}
			/* fetch: fetch, update pc, ram address */
			if ( m_byte_size > 8 )
			{
				debugger_instruction_hook( this, m_rom_address << 1 );
				m_opcode = m_program->read_word( m_rom_address << 1 ) & 0x1FF;
			}
			else
			{
				debugger_instruction_hook( this, m_rom_address );
				m_opcode = m_program->read_byte( m_rom_address );
			}
			next_pc();

			/* ram address */
			m_ram_address = ( m_x << 4 ) | m_y;
			break;
		case 5:
			/* fetch: instruction decode */
			m_decode = m_decode_table[ m_opcode ];
			/* execute: execute br/call */
			if ( m_status )
			{
				if ( m_decode == F_BR )
				{
					m_ca = m_cb;
					if ( m_call_latch == 0 )
					{
						m_pa = m_pb;
					}
					m_pc = m_opcode & ( ( 1 << m_pc_size ) - 1 );
				}
				if ( m_decode == F_CALL )
				{
					UINT8 t = m_pa;
					if ( m_call_latch == 0 )
					{
						m_sr = m_pc;
						m_call_latch = 1;
						m_pa = m_pb;
						m_cs = m_ca;
					}
					m_ca = m_cb;
					m_pb = t;
					m_pc = m_opcode & ( ( 1 << m_pc_size ) - 1 );
				}
			}
			if ( m_decode == F_RETN )
			{
				if ( m_call_latch == 1 )
				{
					m_pc = m_sr;
					m_call_latch = 0;
					m_ca = m_cs;
				}
				m_add_latch = 0;
				m_pa = m_pb;
			} else {
				m_branch_latch = 0;
			}
			break;
		}
		m_subcycle = ( m_subcycle + 1 ) % 6;
	} while( m_icount > 0 );
}


void tms0980_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch( entry.index() )
	{
		case STATE_GENPC:
			string.printf( "%03X", ( ( m_pa << 7 ) | m_pc ) << 1 );
			break;
	}
}


void tms1000_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch( entry.index() )
	{
		case STATE_GENPC:
			string.printf( "%03X", ( m_pa << 6 ) | tms1000_pc_decode[ m_pc ] );
			break;
	}
}


void tms1100_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch( entry.index() )
	{
		case STATE_GENPC:
			string.printf( "%03X", ( m_ca << 10 ) | ( m_pa << 6 ) | m_pc );
			break;
	}
}


tms0980_cpu_device::tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1xxx_cpu_device( mconfig, TMS0980, "TMS0980", tag, owner, clock, tms0980_decode, 0x00ff, 0x07ff, 7, 9, 4
						, 12, ADDRESS_MAP_NAME( tms0980_internal_rom ), 7, ADDRESS_MAP_NAME( tms0980_internal_ram ), "tms0980", __FILE__)
{
}


offs_t tms0980_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms0980 );
	return CPU_DISASSEMBLE_NAME(tms0980)(this, buffer, pc, oprom, opram, options);
}


tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1xxx_cpu_device( mconfig, TMS1000, "TMS1000", tag, owner, clock, tms1000_default_decode, 0x00ff, 0x07ff, 6, 8, 2
						, 11, ADDRESS_MAP_NAME( program_11bit_8 ), 7, ADDRESS_MAP_NAME( data_7bit ), "tms1000", __FILE__)
{
}


tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, device_type type, const char*name, const char *tag, device_t *owner, UINT32 clock, UINT16 o_mask, UINT16 r_mask, const char *shortname, const char *source)
	: tms1xxx_cpu_device( mconfig, type, name, tag, owner, clock, tms1000_default_decode, o_mask, r_mask, 6, 8, 2
						, 10, ADDRESS_MAP_NAME( program_10bit_8 ), 6, ADDRESS_MAP_NAME( data_6bit ), shortname, source )
{
}


offs_t tms1000_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms1000 );
	return CPU_DISASSEMBLE_NAME(tms1000)(this, buffer, pc, oprom, opram, options);
}


tms1070_cpu_device::tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device( mconfig, TMS1070, "TMS1070", tag, owner, clock, 0x00ff, 0x07ff, "tms1070", __FILE__)
{
}


tms1200_cpu_device::tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device( mconfig, TMS1200, "TMS1200", tag, owner, clock, 0x00ff, 0x1fff, "tms1200", __FILE__)
{
}


tms1270_cpu_device::tms1270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device( mconfig, TMS1270, "TMS1270", tag, owner, clock, 0x03ff, 0x1fff, "tms1270", __FILE__)
{
}


tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1xxx_cpu_device( mconfig, TMS1100, "TMS1100", tag, owner, clock, tms1100_default_decode, 0x00ff, 0x07ff, 6, 8, 3
						, 11, ADDRESS_MAP_NAME( program_11bit_8 ), 7, ADDRESS_MAP_NAME( data_7bit ), "tms1100", __FILE__ )
{
}


tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, device_type type, const char*name, const char *tag, device_t *owner, UINT32 clock, UINT16 o_mask, UINT16 r_mask, const char *shortname, const char *source)
	: tms1xxx_cpu_device( mconfig, type, name, tag, owner, clock, tms1100_default_decode, o_mask, r_mask, 6, 8, 3
						, 11, ADDRESS_MAP_NAME( program_11bit_8 ), 7, ADDRESS_MAP_NAME( data_7bit ), shortname, source )
{
}


offs_t tms1100_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms1100 );
	return CPU_DISASSEMBLE_NAME(tms1100)(this, buffer, pc, oprom, opram, options);
}


tms1300_cpu_device::tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device( mconfig, TMS1300, "TMS1300", tag, owner, clock, 0x00ff, 0xffff, "tms1300", __FILE__ )
{
}
