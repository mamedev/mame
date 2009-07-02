/*

This CPU core has not been thoroughly tested. Several features may still be
incomplete, missing, or broken.  [Wilbert Pol]


TMS0980 CPU core

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

Instructions built from microinstructions:

ACACC   000111011*  CKP, ATN, C8, AUTA
        00111cccc
ACNAA   000111100*  CKP, NATN, AUTA
        01100cccc
ALEC    000111110*  CIN, CKP, NATN, C8
        01110cccc
ALEM    000000001   MTP, NATN, CIN, C8
AMAAC   000010101   ATN, MTP, C8, AUTA
BRANCH  10wwwwwww   no microinstructions
CALL    11wwwwwww   no microinstructions
CCLA    000010010   AUTA, SSS
CLA     000000110   AUTA
COMX    000000000   no microinstructions
COMX8   010110010   no microinstructions
CPAIZ   000110001   NATN, CIN, C8, AUTA
CTMDYN  000011000   YTP, 15TN, C8, CME, AUTY
DMAN    000000111   MTP, 15TN, C8, AUTA
DMEA    000010000   MTP, DMTP, SSS, AUTA
DNAA    000010001   DMTP, NATN, SSS, AUTA
DYN     000000100   YTP, 15TN, C8, AUTY
IMAC    000110010   MTP, CIN, C8, AUTA
IYC     000000101   YTP, CIN, C8, AUTY
KNEZ    000001110   CKP, NE
LDP     01000cccc   no microinstructions
LDX     01001cccc   no microinstructions
MNEA    000001001   MTP, ATN, NE
MNEZ    000110011   MTP, NE
NDMEA   000010011   MTN, NDMTP, SSS, AUTA
OFF     010110110
RBIT    0101001bb   no microinstructions
REAC    010110100   no microinstructions
RETN    010111111   no microinstructions
SAL     010110001   no microinstructions
SAMAN   000110000   MTP, NATN, CIN, C8, AUTA
SBIT    0101000bb   no microinstructions
SBL     010110011   no microinstructions
SEAC    010110101   no microinstructions
SETR    000001101   YTP, 15TN, AUTY, C8
TAM     000101111   STO
TAMACS  000111101*  STO, ATN, CKP, AUTA, SSE
        01101cccc
TAMDYN  000101100   STO, YTP, 15TN, AUTY, C8
TAMIYC  000101101   STO, YTP, CIN, C8, AUTY
TAMZA   000101110   STO, AUTA
TAY     000101000   ATN, AUTY
TBIT    0001000bb   CKP, CKN, MTP, NE
TCMIY   000111010*  CKM, YTP, CIN, AUTY
        00110cccc
TCY     000111000*  CKP, AUTY
        00100cccc
TDO     010110000   no microinstructions
TKA     000001000   CKP, AUTA
TKM     000001010   CKM
TMA     000101001   MTP, AUTA
TMY     000101010   MTP, AUTY
TYA     000101011   YTP, AUTA
XDA     000011001   DMTP, AUTA, STO
XMA     000000011   MTP, STO, AUTA
YMCY    000111111*  CIN, YTP, CKN, AUTY
        01111cccc
YNEA    000000010   YTP, ATN, NE
YNEC    000111001*  YTP, CKN, NE
        00101cccc
* = faked/special decode variant of instruction

Microinstructions:
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

unknown: CME, SSE, SSS

*/

#include "debugger.h"
#include "tms0980.h"
#include "deprecat.h"

#define LOG					1

#define MICRO_MASK			0x80000000
#define FIXED_INSTRUCTION	0x00000000

/* Standard/fixed intructions */
#define F_ILL				0x00000000
#define F_BR				0x00000001
#define F_CALL				0x00000002
#define F_CLO				0x00000004
#define F_COMX				0x00000008
#define F_COMX8				0x00000010
#define F_LDP				0x00000020
#define F_LDX				0x00000040
#define F_OFF				0x00000080
#define F_RBIT				0x00000100
#define F_REAC				0x00000200
#define F_RETN				0x00000400
#define F_SAL				0x00000800
#define F_SBIT				0x00001000
#define F_SBL				0x00002000
#define F_SEAC				0x00004000
#define F_SETR				0x00008000
#define F_TDO				0x00010000

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
#define M_YTP				0x00080000

/* instructions built from microinstructions */
#define I_ACACC		( MICRO_MASK | M_CKP | M_ATN | M_C8 | M_AUTA )
#define I_ACNAA		( MICRO_MASK | M_CKP | M_NATN | M_AUTA )
#define I_ALEC		( MICRO_MASK | M_CKP | M_NATN | M_CIN | M_C8 )
#define I_ALEM		( MICRO_MASK | M_MTP | M_NATN | M_CIN | M_C8 )
#define I_AMAAC		( MICRO_MASK | M_MTP | M_ATN | M_C8 | M_AUTA )
#define I_CCLA		( MICRO_MASK | M_AUTA | M_SSS )
#define I_CLA		( MICRO_MASK | M_AUTA )
#define I_CPAIZ		( MICRO_MASK | M_NATN | M_CIN | M_C8 | M_AUTA )
#define I_CTMDYN	( MICRO_MASK | M_YTP | M_15TN | M_C8 | M_AUTY | M_CME )
#define I_DMAN		( MICRO_MASK | M_MTP | M_15TN | M_C8 | M_AUTA )
#define I_DMEA		( MICRO_MASK | M_MTP | M_DMTP | M_SSS | M_AUTA )
#define I_DNAA		( MICRO_MASK | M_DMTP | M_NATN | M_SSS | M_AUTA )
#define I_DYN		( MICRO_MASK | M_YTP | M_15TN | M_C8 | M_AUTY )
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


typedef struct _tms0980_state tms0980_state;
struct _tms0980_state
{
	UINT8	prev_pc;		/* previous program counter */
	UINT8	prev_pa;		/* previous page address register */
	UINT8	pc;				/* program counter is a 7 bit register */
	UINT8	pa;				/* page address register is a 4 bit register */
	UINT8	sr;				/* subroutine return register is a 7 bit register */
	UINT8	pb;				/* page buffer register is a 4 bit register */
	UINT8	a;				/* Accumulator is a 4 bit register (?) */
	UINT8	x;				/* X-register is a 2 bit register */
	UINT8	y;				/* Y-register is a 4 bit register */
	UINT8	dam;			/* DAM register is a 4 bit register */
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
	read8_device_func	read_k;
	write8_device_func	write_o;
	write16_device_func	write_r;
	int	subcycle;
	UINT8	ram_address;
	UINT16	ram_data;
	UINT16	rom_address;
	UINT16	opcode;
	UINT32	decode;
	int		icount;
	const tms0980_config	*config;
	const address_space *program;
	const address_space *data;
};


static const UINT8 tms0980_c_value[16] =
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


INLINE tms0980_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_TMS0980);
	return (tms0980_state *)device->token;
}


static ADDRESS_MAP_START(tms0980_internal_rom, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE( 0x0000, 0x0FFF ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(tms0980_internal_ram, ADDRESS_SPACE_DATA, 8)
	AM_RANGE( 0x0000, 0x0FFF ) AM_RAM
ADDRESS_MAP_END


static CPU_INIT( tms0980 )
{
	tms0980_state *cpustate = get_safe_token( device );

	cpustate->config = (const tms0980_config *) device->static_config;

	cpustate->program = memory_find_address_space( device, ADDRESS_SPACE_PROGRAM );
	cpustate->data = memory_find_address_space( device, ADDRESS_SPACE_DATA );

	state_save_register_device_item( device, 0, cpustate->prev_pc );
	state_save_register_device_item( device, 0, cpustate->prev_pa );
	state_save_register_device_item( device, 0, cpustate->pc );
	state_save_register_device_item( device, 0, cpustate->pa );
	state_save_register_device_item( device, 0, cpustate->sr );
	state_save_register_device_item( device, 0, cpustate->pb );
	state_save_register_device_item( device, 0, cpustate->a );
	state_save_register_device_item( device, 0, cpustate->x );
	state_save_register_device_item( device, 0, cpustate->y );
	state_save_register_device_item( device, 0, cpustate->dam );
	state_save_register_device_item( device, 0, cpustate->r );
	state_save_register_device_item( device, 0, cpustate->o );
	state_save_register_device_item( device, 0, cpustate->cki_bus );
	state_save_register_device_item( device, 0, cpustate->p );
	state_save_register_device_item( device, 0, cpustate->n );
	state_save_register_device_item( device, 0, cpustate->adder_result );
	state_save_register_device_item( device, 0, cpustate->carry_in );
	state_save_register_device_item( device, 0, cpustate->status );
	state_save_register_device_item( device, 0, cpustate->status_latch );
	state_save_register_device_item( device, 0, cpustate->special_status );
	state_save_register_device_item( device, 0, cpustate->call_latch );
	state_save_register_device_item( device, 0, cpustate->add_latch );
	state_save_register_device_item( device, 0, cpustate->branch_latch );
	state_save_register_device_item( device, 0, cpustate->subcycle );
	state_save_register_device_item( device, 0, cpustate->ram_address );
	state_save_register_device_item( device, 0, cpustate->ram_data );
	state_save_register_device_item( device, 0, cpustate->rom_address );
	state_save_register_device_item( device, 0, cpustate->opcode );
	state_save_register_device_item( device, 0, cpustate->decode );
}


static CPU_RESET( tms0980 )
{
	tms0980_state *cpustate = get_safe_token( device );

	cpustate->pa = 0x0F;
	cpustate->pb = 0x0F;
	cpustate->pc = 0;
	cpustate->dam = 0;
	cpustate->subcycle = 0;
	cpustate->status = 1;
	cpustate->call_latch = 0;
	cpustate->add_latch = 0;
	cpustate->branch_latch = 0;
	cpustate->r = 0;
	cpustate->o = 0;
	cpustate->ram_address = 0;
	cpustate->decode = 0;
	cpustate->opcode = 0;
	cpustate->read_k = NULL;
	cpustate->write_o = NULL;
	cpustate->write_r = NULL;
	if ( cpustate->config )
	{
		cpustate->read_k = cpustate->config->read_k;
		cpustate->write_o = cpustate->config->write_o;
		cpustate->write_r = cpustate->config->write_r;
	}
}


/* emulator for the program counter increment on the tms0980/tmc0980 mcu;
 see patent 4064554 figure 19 (on page 13) for an explanation of feedback:

  nand324 = NAND of PC0 thru pc4, i.e. output is true if ((pc&0x1f) != 0x1f)
  nand323 = NAND of pc5, pc6 and nand324
      i.e. output is true, if ((pc&0x1f)==0x1f) || pc5 is 0 || pc 6 is 0
  or321 = OR of pc5 and pc6, i.e. output is true if ((pc&0x60) != 0)
  nand322 = NAND of pc0 thru pc5 plus /pc6,
      i.e. output is true if (pc != 0x3f)
  nand325 = nand pf nand323, or321 and nand322
      This one is complex:
      / or321 means if pc&0x60 is zero, output MUST be true
      \ nand323 means if (pc&0x60=0x60) && (pc&0x1f != 0x1f), output MUST be true
      nand322 means if pc = 0x3f, output MUST be true
      hence, nand325 is if pc = 0x7f, false. if pc = 0x3f, true. if pc&0x60 is zero OR pc&0x60 is 0x60, true. otherwise, false.

      tms0980_nect_pc below implements an indentical function to this in a somewhat more elegant way.
*/
INLINE UINT8 tms0980_next_pc( UINT8 pc )
{
	UINT8	xor = ( pc & 0x3F ) == 0x3F ? 1 : 0;
	UINT8	new_bit = ( ( pc ^ ( pc << 1 ) ) & 0x40 ) ? xor : 1 - xor;

	return ( pc << 1 ) | new_bit;
}


static void tms0980_set_cki_bus( const device_config *device )
{
	tms0980_state *cpustate = get_safe_token( device );

	switch( cpustate->opcode & 0xF8 )
	{
	case 0x08:
		if ( cpustate->read_k )
		{
			cpustate->cki_bus = cpustate->read_k( device, 0 );
		}
		else
		{
			cpustate->cki_bus = 0x0F;
		}
		break;
	case 0x20: case 0x28:
		cpustate->cki_bus = 0;
		break;
	case 0x30:
	case 0x38:
		cpustate->cki_bus = tms0980_nbit_value[ cpustate->opcode & 0x03 ];
		break;
	case 0x00:
	case 0x40: case 0x48:
	case 0x50: case 0x58:
	case 0x60: case 0x68:
	case 0x70: case 0x78:
		cpustate->cki_bus = tms0980_c_value[ cpustate->opcode & 0x0F ];
		break;
	default:
		cpustate->cki_bus = 0x0F;
		break;
	}
	/* Case 000001XXX */
	if ( ( cpustate->opcode & 0x1F8 ) == 0x008 )
	{
	}
	/* Case 00001XXXX */
	if ( ( cpustate->opcode & 0x1F0 ) == 0x010 )
	{
	}
	/* Case 00100XXXX - TCY */
	if ( ( cpustate->opcode & 0x1F0 ) == 0x040 )
	{
		cpustate->cki_bus = cpustate->opcode & 0x0F;
	}
	/* Case 01XXXXXXX */
	if ( ( cpustate->opcode & 0x180 ) == 0x080 )
	{
	}
}


static CPU_EXECUTE( tms0980 )
{
	tms0980_state *cpustate = get_safe_token( device );

	cpustate->icount = cycles;

	do
	{
//      debugger_instruction_hook( device, ( ( cpustate->pa << 7 ) | cpustate->pc ) << 1 );
		cpustate->icount--;
		switch( cpustate->subcycle )
		{
		case 0:
			/* fetch: rom address 0 */
			/* execute: read ram, alu input, execute br/call, k input valid */
			tms0980_set_cki_bus( device );
			cpustate->ram_data = memory_read_byte_8le( cpustate->data, cpustate->ram_address );
			cpustate->status = 1;
			cpustate->p = 0;
			cpustate->n = 0;
			cpustate->carry_in = 0;
			break;
		case 1:
			/* fetch: rom address 1 */
			cpustate->rom_address = ( cpustate->pa << 7 ) | cpustate->pc;
			/* execute: k input valid */
			if ( cpustate->decode & MICRO_MASK )
			{
				/* Check N inputs */
				if ( cpustate->decode & ( M_15TN | M_ATN | M_CKN | M_MTN | M_NATN ) )
				{
					cpustate->n = 0x0F;
					if ( cpustate->decode & M_15TN )
					{
						cpustate->n &= 0x0F;
					}
					if ( cpustate->decode & M_ATN )
					{
						cpustate->n &= cpustate->a;
					}
					if ( cpustate->decode & M_CKN )
					{
						cpustate->n &= cpustate->cki_bus;
					}
					if ( cpustate->decode & M_MTN )
					{
						cpustate->n &= cpustate->ram_data;
					}
					if ( cpustate->decode & M_NATN )
					{
						cpustate->n &= ( ( ~cpustate->a ) & 0x0F );
					}
				}


				/* Check P inputs */
				if ( cpustate->decode & ( M_CKP | M_DMTP | M_MTP | M_NDMTP | M_YTP ) )
				{
					cpustate->p = 0x0F;
					if ( cpustate->decode & M_CKP )
					{
						cpustate->p &= cpustate->cki_bus;
					}
					if ( cpustate->decode & M_DMTP )
					{
						cpustate->p &= cpustate->dam;
					}
					if ( cpustate->decode & M_MTP )
					{
						cpustate->p &= cpustate->ram_data;
					}
					if ( cpustate->decode & M_NDMTP )
					{
						cpustate->p &= ( ( ~cpustate->dam ) & 0x0F );
					}
					if ( cpustate->decode & M_YTP )
					{
						cpustate->p &= cpustate->y;
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
					memory_write_byte_8le( cpustate->data, cpustate->ram_address, cpustate->a );
				}
				if ( cpustate->decode & M_CKM )
				{
					memory_write_byte_8le( cpustate->data, cpustate->ram_address, cpustate->cki_bus );
				}
			}
			else
			{
				if ( cpustate->decode & F_SBIT )
				{
					memory_write_byte_8le( cpustate->data, cpustate->ram_address, cpustate->ram_data | tms0980_bit_value[ cpustate->opcode & 0x03 ] );
				}
				if ( cpustate->decode & F_RBIT )
				{
					memory_write_byte_8le( cpustate->data, cpustate->ram_address, cpustate->ram_data & tms0980_nbit_value[ cpustate->opcode & 0x03 ] );
				}
				if ( cpustate->decode & F_SETR )
				{
					cpustate->r = cpustate->r | ( 1 << cpustate->y );
					if ( cpustate->write_r )
					{
						cpustate->write_r( device, 0, cpustate->r, 0xFFFF );
					}
				}
				if ( cpustate->decode & F_TDO )
				{
					cpustate->o = ( cpustate->status_latch << 4 ) | cpustate->a;
					if ( cpustate->write_o )
					{
						cpustate->write_o( device, 0, cpustate->o );
					}
				}
				if ( cpustate->decode & F_CLO )
				{
					cpustate->o = 0;
					if ( cpustate->write_o )
					{
						cpustate->write_o( device, 0, cpustate->o );
					}
				}
				if ( cpustate->decode & F_LDX )
				{
					cpustate->x = tms0980_c_value[ cpustate->opcode & 0x0F ];
				}
				if ( cpustate->decode & F_COMX )
				{
					cpustate->x = cpustate->x ^ 0x03;
				}
				if ( cpustate->decode & F_LDP )
				{
					cpustate->pb = tms0980_c_value[ cpustate->opcode & 0x0F ];
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
			cpustate->ram_address = ( cpustate->x << 4 ) | cpustate->y;
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
//              if ( cpustate->decode & M_STSL )
//              {
//                  cpustate->status_latch = cpustate->status;
//              }
			}
			debugger_instruction_hook( device, cpustate->rom_address << 1 );
			/* fetch: fetch, update pc, ram address */
			cpustate->opcode = memory_read_word_16be( cpustate->program, cpustate->rom_address << 1 );
			cpustate->opcode &= 0x1FF;
			cpustate->pc = tms0980_next_pc( cpustate->pc );
			if (LOG)
				logerror( "tms0980: read opcode %04x. Set pc to %04x\n", cpustate->opcode, cpustate->pc );
			break;
		case 5:
			/* fetch: instruction decode */
			cpustate->decode = tms0980_decode[ cpustate->opcode ];
			/* execute: execute br/call */
			if ( cpustate->status )
			{
				if ( cpustate->decode == F_BR )
				{
					if ( cpustate->call_latch == 0 )
					{
						cpustate->pa = cpustate->pb;
					}
					cpustate->pc = cpustate->opcode & 0x7F;
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
					cpustate->pc = cpustate->opcode & 0x7F;
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

	return cycles - cpustate->icount;
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


CPU_GET_INFO( tms0980 )
{
	tms0980_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		case CPUINFO_INT_CONTEXT_SIZE:									info->i = sizeof(tms0980_state); break;
		case CPUINFO_INT_INPUT_LINES:									info->i = 1; break;
		case CPUINFO_INT_ENDIANNESS:									info->i = ENDIANNESS_BIG; break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:								info->i = 1; break;
		case CPUINFO_INT_CLOCK_DIVIDER:									info->i = 1; break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:							info->i = 2; break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:							info->i = 2; break;
		case CPUINFO_INT_MIN_CYCLES:									info->i = 1; break;
		case CPUINFO_INT_MAX_CYCLES:									info->i = 6; break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:			info->i = 16 /* 9 */; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:			info->i = 12; break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:			info->i = 0; break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 8 /* 4 */; break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 7; break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:			info->i = 0; break;

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

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map16 = ADDRESS_MAP_NAME( tms0980_internal_rom ); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:		info->internal_map8 = ADDRESS_MAP_NAME( tms0980_internal_ram ); break;

		case CPUINFO_FCT_SET_INFO:										info->setinfo = CPU_SET_INFO_NAME( tms0980 ); break;
		case CPUINFO_FCT_INIT:											info->init = CPU_INIT_NAME( tms0980 ); break;
		case CPUINFO_FCT_RESET:											info->reset = CPU_RESET_NAME( tms0980 ); break;
		case CPUINFO_FCT_EXECUTE:										info->execute = CPU_EXECUTE_NAME( tms0980 ); break;
		case CPUINFO_FCT_DISASSEMBLE:									info->disassemble = CPU_DISASSEMBLE_NAME( tms0980 ); break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:							info->icount = &cpustate->icount; break;

		case CPUINFO_STR_NAME:											strcpy( info->s, "TMS0980" ); break;
		case CPUINFO_STR_CORE_FAMILY:									strcpy( info->s, "Texas Instruments TMS0980" ); break;
		case CPUINFO_STR_CORE_VERSION:									strcpy( info->s, "0.1" ); break;
		case CPUINFO_STR_CORE_FILE:										strcpy( info->s, __FILE__ ); break;
		case CPUINFO_STR_CORE_CREDITS:									strcpy( info->s, "Copyright the MESS and MAME teams" ); break;

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

