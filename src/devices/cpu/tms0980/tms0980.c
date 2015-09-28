// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS0980/TMS1000-family MCU cores

  TODO:
  - emulate TMS1600 L-pins
  - fix debugger disasm view


The TMS0980 and TMS1000-family MCU cores are very similar. The TMS0980 has a
slightly bigger addressable area and uses 9bit instructions where the TMS1000
family uses 8bit instruction. The instruction set themselves are very similar
though.

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

*/

#include "tms0980.h"
#include "debugger.h"

/*

The MCU cores contains a set of fixed instructions and a set of
instructions created using microinstructions. A subset of the
instruction set could be defined from the microinstructions by
TI customers.

cycle #0: 15TN, ATN, CIN, CKN, CKP, DMTP, MTN, MTP, NATN, NDMTP, YTP
cycle #2: C8(?), CKM, NE(?), STO
cycle #3,#4: AUTA, AUTY

unknown cycle: CME, SSE, SSS

*/

/* Microinstructions */
#define M_15TN              (1<<0)  /* 15 to -ALU */
#define M_ATN               (1<<1)  /* ACC to -ALU */
#define M_AUTA              (1<<2)  /* ALU to ACC */
#define M_AUTY              (1<<3)  /* ALU to Y */
#define M_C8                (1<<4)  /* CARRY8 to STATUS */
#define M_CIN               (1<<5)  /* Carry In to ALU */
#define M_CKM               (1<<6)  /* CKB to MEM */
#define M_CKN               (1<<7)  /* CKB to -ALU */
#define M_CKP               (1<<8)  /* CKB to +ALU */
#define M_MTN               (1<<9)  /* MEM to -ALU */
#define M_MTP               (1<<10) /* MEM to +ALU */
#define M_NATN              (1<<11) /* ~ACC to -ALU */
#define M_NE                (1<<12) /* COMP to STATUS */
#define M_STO               (1<<13) /* ACC to MEM */
#define M_STSL              (1<<14) /* STATUS to Status Latch */
#define M_YTP               (1<<15) /* Y to +ALU */

#define M_CME               (1<<16) /* Conditional Memory Enable */
#define M_DMTP              (1<<17) /* DAM to +ALU */
#define M_NDMTP             (1<<18) /* ~DAM to +ALU */
#define M_SSE               (1<<19) /* Special Status Enable */
#define M_SSS               (1<<20) /* Special Status Sample */

#define M_RSTR              (1<<21) /* -> line #36, F_RSTR (TMS02x0 custom) */
#define M_UNK1              (1<<22) /* -> line #37, F_???? (TMS0270 custom) */

/* Standard/fixed instructions - these are documented more in their specific handlers below */
#define F_BR                (1<<0)
#define F_CALL              (1<<1)
#define F_CLO               (1<<2)
#define F_COMC              (1<<3)
#define F_COMX              (1<<4)
#define F_COMX8             (1<<5)
#define F_LDP               (1<<6)
#define F_LDX               (1<<7)
#define F_RBIT              (1<<8)
#define F_RETN              (1<<9)
#define F_RSTR              (1<<10)
#define F_SBIT              (1<<11)
#define F_SETR              (1<<12)
#define F_TDO               (1<<13)
#define F_TPC               (1<<14)

#define F_OFF               (1<<15)
#define F_REAC              (1<<16)
#define F_SAL               (1<<17)
#define F_SBL               (1<<18)
#define F_SEAC              (1<<19)
#define F_XDA               (1<<20)


// supported types:
// note: dice information assumes the orientation is pictured with RAM at the bottom-left, except where noted

// TMS1000
// - 64x4bit RAM array at the bottom-left
// - 1024x8bit ROM array at the bottom-right
//   * FYI, the row-selector to the left of it is laid out as:
//     3,4,11,12,19,20,27,28,35,36,43,44,51,52,59,60,0,7,8,15,16,23,24,31,32,39,40,47,48,55,56,63,
//     2,5,10,13,18,21,26,29,34,37,42,45,50,53,58,61,1,6,9,14,17,22,25,30,33,38,41,46,49,54,57,62
// - 30-term microinstructions PLA(mpla) at the top half, to the right of the midline, supporting 16 microinstructions
// - 20-term output PLA(opla) at the top-left
// - the ALU is between the opla and mpla
const device_type TMS1000 = &device_creator<tms1000_cpu_device>; // 28-pin DIP, 11 R pins
const device_type TMS1070 = &device_creator<tms1070_cpu_device>; // high voltage version
const device_type TMS1040 = &device_creator<tms1040_cpu_device>; // same as TMS1070 with just a different pinout?
const device_type TMS1200 = &device_creator<tms1200_cpu_device>; // 40-pin DIP, 13 R pins
// TMS1270 has 10 O pins, how does that work?

// TMS1100 is nearly the same as TMS1000, some different opcodes, and with double the RAM and ROM
const device_type TMS1100 = &device_creator<tms1100_cpu_device>; // 28-pin DIP, 11 R pins
const device_type TMS1170 = &device_creator<tms1170_cpu_device>; // high voltage version
const device_type TMS1300 = &device_creator<tms1300_cpu_device>; // 40-pin DIP, 16 R pins
const device_type TMS1370 = &device_creator<tms1370_cpu_device>; // high voltage version

// TMS1400 follows the TMS1100, it doubles the ROM size again (4 chapters of 16 pages), and adds a 3-level callstack
// - rotate the view and mirror the OR-mask to get the proper layout of the mpla, the default is identical to tms1100
// - the opla size is increased from 20 to 32 terms
const device_type TMS1400 = &device_creator<tms1400_cpu_device>; // 28-pin DIP, 11 R pins (TMS1400CR is same, but with TMS1100 pinout)
const device_type TMS1470 = &device_creator<tms1470_cpu_device>; // high voltage version, 1 R pin removed for Vdd

// TMS1600 adds more I/O to the TMS1400, input pins are doubled with added L1,2,4,8
// - rotate the view and mirror the OR-mask to get the proper layout of the mpla, the default is identical to tms1100
// - the opla size is increased from 20 to 32 terms
const device_type TMS1600 = &device_creator<tms1600_cpu_device>; // 40-pin DIP, 16 R pins
const device_type TMS1670 = &device_creator<tms1670_cpu_device>; // high voltage version

// TMS0980
// - 64x9bit RAM array at the bottom-left (set up as 144x4)
// - 2048x9bit ROM array at the bottom-left
// - main instructions PLA at the top half, to the right of the midline
// - 64-term microinstructions PLA between the RAM and ROM, supporting 20 microinstructions
// - 16-term output PLA and segment PLA above the RAM (rotate opla 90 degrees)
const device_type TMS0980 = &device_creator<tms0980_cpu_device>; // 28-pin DIP, 9 R pins

// TMS1980 is a TMS0980 with a TMS1x00 style opla
// - RAM, ROM, and main instructions PLA is exactly the same as TMS0980
// - one of the microinstructions redirects to a RSTR instruction, like on TMS0270
// - 32-term output PLA above the RAM, 7 bits! (rotate opla 270 degrees)
const device_type TMS1980 = &device_creator<tms1980_cpu_device>; // 28-pin DIP, 7 O pins, ? R pins, high voltage
// TMS0260 is same?

// TMS0970 is a stripped-down version of the TMS0980, itself acting more like a TMS1000
// - RAM and ROM is exactly the same as TMS1000
// - main instructions PLA at the top half, to the right of the midline
// - 32-term microinstructions PLA between the RAM and ROM, supporting 15 microinstructions
// - 16-term output PLA and segment PLA above the RAM (rotate opla 90 degrees)
const device_type TMS0970 = &device_creator<tms0970_cpu_device>; // 28-pin DIP, 11 R pins (note: pinout may slightly differ from chip to chip)
const device_type TMS1990 = &device_creator<tms1990_cpu_device>; // 28-pin DIP, ? R pins..
// TMS0950 is same?

// TMS0270 on the other hand, is a TMS0980 with earrings and a new hat. The new changes look like a quick afterthought, almost hacky
// - RAM, ROM, and main instructions PLA is exactly the same as TMS0980
// - 64-term microinstructions PLA between the RAM and ROM, supporting 20 microinstructions plus optional separate lines for custom opcode handling
// - 48-term output PLA above the RAM (rotate opla 90 degrees)
const device_type TMS0270 = &device_creator<tms0270_cpu_device>; // 40-pin DIP, 16 O pins, 8+ R pins (some R pins are internally hooked up to support more I/O)
// newer TMS0270 chips (eg. Speak & Math) have 42 pins


// internal memory maps
static ADDRESS_MAP_START(program_11bit_9, AS_PROGRAM, 16, tms1xxx_cpu_device)
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_10bit_8, AS_PROGRAM, 8, tms1xxx_cpu_device)
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_11bit_8, AS_PROGRAM, 8, tms1xxx_cpu_device)
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_12bit_8, AS_PROGRAM, 8, tms1xxx_cpu_device)
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, tms1xxx_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_128x4, AS_DATA, 8, tms1xxx_cpu_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_64x9_as4, AS_DATA, 8, tms1xxx_cpu_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
	AM_RANGE(0x80, 0x8f) AM_RAM AM_MIRROR(0x70) // DAM
ADDRESS_MAP_END


// device definitions
tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1xxx_cpu_device(mconfig, TMS1000, "TMS1000", tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 2 /* x width */, 10 /* prg width */, ADDRESS_MAP_NAME(program_10bit_8), 6 /* data width */, ADDRESS_MAP_NAME(data_64x4), "tms1000", __FILE__)
{ }

tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1xxx_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1070_cpu_device::tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1070, "TMS1070", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1070", __FILE__)
{ }

tms1040_cpu_device::tms1040_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1040, "TMS1040", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1040", __FILE__)
{ }

tms1200_cpu_device::tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1200, "TMS1200", tag, owner, clock, 8, 13, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1200", __FILE__)
{ }


tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1100, "TMS1100", tag, owner, clock, 8, 11, 6, 8, 3, 11, ADDRESS_MAP_NAME(program_11bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1100", __FILE__)
{ }

tms1100_cpu_device::tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1000_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1170_cpu_device::tms1170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device(mconfig, TMS1170, "TMS1170", tag, owner, clock, 8, 11, 6, 8, 3, 11, ADDRESS_MAP_NAME(program_11bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1170", __FILE__)
{ }

tms1300_cpu_device::tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device(mconfig, TMS1300, "TMS1300", tag, owner, clock, 8, 16, 6, 8, 3, 11, ADDRESS_MAP_NAME(program_11bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1300", __FILE__)
{ }

tms1370_cpu_device::tms1370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device(mconfig, TMS1370, "TMS1370", tag, owner, clock, 8, 16, 6, 8, 3, 11, ADDRESS_MAP_NAME(program_11bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1370", __FILE__)
{ }


tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1100_cpu_device(mconfig, TMS1400, "TMS1400", tag, owner, clock, 8, 11, 6, 8, 3, 12, ADDRESS_MAP_NAME(program_12bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1400", __FILE__)
{ }

tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1100_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1470_cpu_device::tms1470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1400_cpu_device(mconfig, TMS1470, "TMS1470", tag, owner, clock, 8, 10, 6, 8, 3, 12, ADDRESS_MAP_NAME(program_12bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1470", __FILE__)
{ }


tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1400_cpu_device(mconfig, TMS1600, "TMS1600", tag, owner, clock, 8, 16, 6, 8, 3, 12, ADDRESS_MAP_NAME(program_12bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1600", __FILE__)
{ }

tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1400_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1670_cpu_device::tms1670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1600_cpu_device(mconfig, TMS1670, "TMS1670", tag, owner, clock, 8, 16, 6, 8, 3, 12, ADDRESS_MAP_NAME(program_12bit_8), 7, ADDRESS_MAP_NAME(data_128x4), "tms1670", __FILE__)
{ }


tms0970_cpu_device::tms0970_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS0970, "TMS0970", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms0970", __FILE__)
{ }

tms0970_cpu_device::tms0970_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1000_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1990_cpu_device::tms1990_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0970_cpu_device(mconfig, TMS1990, "TMS1990", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1990", __FILE__)
{ }


tms0980_cpu_device::tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0970_cpu_device(mconfig, TMS0980, "TMS0980", tag, owner, clock, 8, 9, 7, 9, 4, 12, ADDRESS_MAP_NAME(program_11bit_9), 8, ADDRESS_MAP_NAME(data_64x9_as4), "tms0980", __FILE__)
{ }

tms0980_cpu_device::tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms0970_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1980_cpu_device::tms1980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0980_cpu_device(mconfig, TMS1980, "TMS1980", tag, owner, clock, 7, 11, 7, 9, 4, 12, ADDRESS_MAP_NAME(program_11bit_9), 8, ADDRESS_MAP_NAME(data_64x9_as4), "tms1980", __FILE__)
{ }


tms0270_cpu_device::tms0270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0980_cpu_device(mconfig, TMS0270, "TMS0270", tag, owner, clock, 16, 16, 7, 9, 4, 12, ADDRESS_MAP_NAME(program_11bit_9), 8, ADDRESS_MAP_NAME(data_64x9_as4), "tms0270", __FILE__)
	, m_read_ctl(*this)
	, m_write_ctl(*this)
	, m_write_pdc(*this)
{ }


// machine configs
static MACHINE_CONFIG_FRAGMENT(tms1000)

	// microinstructions PLA, output PLA
	MCFG_PLA_ADD("mpla", 8, 16, 30)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 5, 8, 20)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms1000_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms1000);
}


static MACHINE_CONFIG_FRAGMENT(tms1400)

	// microinstructions PLA, output PLA
	MCFG_PLA_ADD("mpla", 8, 16, 30)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 5, 8, 32)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms1400_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms1400);
}


static MACHINE_CONFIG_FRAGMENT(tms0970)

	// main opcodes PLA, microinstructions PLA, output PLA, segment PLA
	MCFG_PLA_ADD("ipla", 8, 15, 18)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 5, 15, 32)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 4, 8, 16)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("spla", 3, 8, 8)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms0970_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms0970);
}


static MACHINE_CONFIG_FRAGMENT(tms0980)

	// main opcodes PLA, microinstructions PLA, output PLA, segment PLA
	MCFG_PLA_ADD("ipla", 9, 22, 24)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 6, 20, 64)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 4, 8, 16)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("spla", 3, 8, 8)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms0980_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms0980);
}


static MACHINE_CONFIG_FRAGMENT(tms1980)

	// main opcodes PLA, microinstructions PLA, output PLA
	MCFG_PLA_ADD("ipla", 9, 22, 24)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 6, 21, 64)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 5, 7, 32)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms1980_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms1980);
}


static MACHINE_CONFIG_FRAGMENT(tms0270)

	// main opcodes PLA, microinstructions PLA, output PLA
	MCFG_PLA_ADD("ipla", 9, 22, 24)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 6, 22, 64)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 6, 16, 48)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms0270_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms0270);
}


// disasm
offs_t tms1000_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tms1000);
	return CPU_DISASSEMBLE_NAME(tms1000)(this, buffer, pc, oprom, opram, options);
}

offs_t tms1100_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tms1100);
	return CPU_DISASSEMBLE_NAME(tms1100)(this, buffer, pc, oprom, opram, options);
}

offs_t tms0980_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tms0980);
	return CPU_DISASSEMBLE_NAME(tms0980)(this, buffer, pc, oprom, opram, options);
}

void tms1xxx_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			strprintf(str, "%03X", m_rom_address << ((m_byte_bits > 8) ? 1 : 0));
			break;
	}
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	TMS1XXX_PC=1, TMS1XXX_SR, TMS1XXX_PA, TMS1XXX_PB,
	TMS1XXX_A, TMS1XXX_X, TMS1XXX_Y, TMS1XXX_STATUS
};

void tms1xxx_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	m_o_mask = (1 << m_o_pins) - 1;
	m_r_mask = (1 << m_r_pins) - 1;
	m_pc_mask = (1 << m_pc_bits) - 1;
	m_x_mask = (1 << m_x_bits) - 1;

	// resolve callbacks
	m_read_k.resolve_safe(0);
	m_write_o.resolve_safe();
	m_write_r.resolve_safe();
	m_power_off.resolve_safe();

	// zerofill
	m_pc = 0;
	m_sr = 0;
	m_pa = 0;
	m_pb = 0;
	m_ps = 0;
	m_a = 0;
	m_x = 0;
	m_y = 0;
	m_ca = 0;
	m_cb = 0;
	m_cs = 0;
	m_r = 0;
	m_o = 0;
	m_cki_bus = 0;
	m_c4 = 0;
	m_p = 0;
	m_n = 0;
	m_adder_out = 0;
	m_carry_in = 0;
	m_carry_out = 0;
	m_status = 0;
	m_status_latch = 0;
	m_eac = 0;
	m_clatch = 0;
	m_add = 0;
	m_bl = 0;

	m_ram_in = 0;
	m_dam_in = 0;
	m_ram_out = 0;
	m_ram_address = 0;
	m_rom_address = 0;
	m_opcode = 0;
	m_fixed = 0;
	m_micro = 0;
	m_subcycle = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_sr));
	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_ps));
	save_item(NAME(m_a));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_ca));
	save_item(NAME(m_cb));
	save_item(NAME(m_cs));
	save_item(NAME(m_r));
	save_item(NAME(m_o));
	save_item(NAME(m_cki_bus));
	save_item(NAME(m_c4));
	save_item(NAME(m_p));
	save_item(NAME(m_n));
	save_item(NAME(m_adder_out));
	save_item(NAME(m_carry_in));
	save_item(NAME(m_carry_out));
	save_item(NAME(m_status));
	save_item(NAME(m_status_latch));
	save_item(NAME(m_eac));
	save_item(NAME(m_clatch));
	save_item(NAME(m_add));
	save_item(NAME(m_bl));

	save_item(NAME(m_ram_in));
	save_item(NAME(m_dam_in));
	save_item(NAME(m_ram_out));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_opcode));
	save_item(NAME(m_fixed));
	save_item(NAME(m_micro));
	save_item(NAME(m_subcycle));

	// register state for debugger
	state_add(TMS1XXX_PC,     "PC",     m_pc    ).formatstr("%02X");
	state_add(TMS1XXX_SR,     "SR",     m_sr    ).formatstr("%01X");
	state_add(TMS1XXX_PA,     "PA",     m_pa    ).formatstr("%01X");
	state_add(TMS1XXX_PB,     "PB",     m_pb    ).formatstr("%01X");
	state_add(TMS1XXX_A,      "A",      m_a     ).formatstr("%01X");
	state_add(TMS1XXX_X,      "X",      m_x     ).formatstr("%01X");
	state_add(TMS1XXX_Y,      "Y",      m_y     ).formatstr("%01X");
	state_add(TMS1XXX_STATUS, "STATUS", m_status).formatstr("%01X");

	state_add(STATE_GENPC, "curpc", m_rom_address).formatstr("%03X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_sr).formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}

void tms0270_cpu_device::device_start()
{
	// common init
	tms1xxx_cpu_device::device_start();

	m_read_ctl.resolve_safe(0);
	m_write_ctl.resolve_safe();
	m_write_pdc.resolve_safe();

	// zerofill
	m_r_prev = 0;
	m_chipsel = 0;
	m_ctl_dir = 0;
	m_ctl_out = 0;
	m_pdc = -1; // !

	m_o_latch_low = 0;
	m_o_latch = 0;
	m_o_latch_prev = 0;

	// register for savestates
	save_item(NAME(m_r_prev));
	save_item(NAME(m_chipsel));
	save_item(NAME(m_ctl_dir));
	save_item(NAME(m_ctl_out));
	save_item(NAME(m_pdc));

	save_item(NAME(m_o_latch_low));
	save_item(NAME(m_o_latch));
	save_item(NAME(m_o_latch_prev));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms1xxx_cpu_device::device_reset()
{
	m_pa = 0xf;
	m_pb = 0xf;
	m_pc = 0;
	m_ca = 0;
	m_cb = 0;
	m_cs = 0;

	m_eac = 0;
	m_bl = 0;
	m_add = 0;

	m_opcode = 0;
	m_micro = 0;
	m_fixed = 0;

	m_subcycle = 0;

	// clear outputs
	m_r = 0;
	m_write_r(0, m_r & m_r_mask, 0xffff);
	write_o_output(0);
	m_write_r(0, m_r & m_r_mask, 0xffff);
	m_power_off(0);
}


void tms1000_cpu_device::device_reset()
{
	// common reset
	tms1xxx_cpu_device::device_reset();

	// pre-decode instructionset
	m_fixed_decode.resize(0x100);
	memset(&m_fixed_decode[0], 0, 0x100*sizeof(UINT32));
	m_micro_decode.resize(0x100);
	memset(&m_micro_decode[0], 0, 0x100*sizeof(UINT32));

	for (int op = 0; op < 0x100; op++)
	{
		//                                              _____              _____  ______  _____  ______  _____  _____  _____  _____
		const UINT32 md[16] = { M_STSL, M_AUTY, M_AUTA, M_CIN, M_C8, M_NE, M_CKN, M_15TN, M_MTN, M_NATN, M_ATN, M_MTP, M_YTP, M_CKP, M_CKM, M_STO };
		UINT16 mask = m_mpla->read(op);
		mask ^= 0x3fc8; // invert active-negative

		for (int bit = 0; bit < 16; bit++)
			if (mask & (1 << bit))
				m_micro_decode[op] |= md[bit];
	}

	// the fixed instruction set is not programmable
	m_fixed_decode[0x00] = F_COMX;
	m_fixed_decode[0x0a] = F_TDO;
	m_fixed_decode[0x0b] = F_CLO;
	m_fixed_decode[0x0c] = F_RSTR;
	m_fixed_decode[0x0d] = F_SETR;
	m_fixed_decode[0x0f] = F_RETN;

	for (int i = 0x10; i < 0x20; i++) m_fixed_decode[i] = F_LDP;
	for (int i = 0x30; i < 0x34; i++) m_fixed_decode[i] = F_SBIT;
	for (int i = 0x34; i < 0x38; i++) m_fixed_decode[i] = F_RBIT;
	for (int i = 0x3c; i < 0x40; i++) m_fixed_decode[i] = F_LDX;

	for (int i = 0x80; i < 0xc0; i++) m_fixed_decode[i] = F_BR;
	for (int i = 0xc0; i < 0x100; i++) m_fixed_decode[i] = F_CALL;
}

void tms1100_cpu_device::device_reset()
{
	tms1000_cpu_device::device_reset();

	// small differences in 00-3f area
	m_fixed_decode[0x00] = 0;
	m_fixed_decode[0x09] = F_COMX8; // !
	m_fixed_decode[0x0b] = F_COMC;

	for (int i = 0x28; i < 0x30; i++) m_fixed_decode[i] = F_LDX;
	for (int i = 0x3c; i < 0x40; i++) m_fixed_decode[i] = 0;
}

void tms1400_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	// small differences in 00-3f area
	m_fixed_decode[0x0b] = F_TPC;
}


void tms0970_cpu_device::device_reset()
{
	// common reset
	tms1xxx_cpu_device::device_reset();

	// pre-decode instructionset
	m_fixed_decode.resize(0x100);
	memset(&m_fixed_decode[0], 0, 0x100*sizeof(UINT32));
	m_micro_decode.resize(0x100);
	memset(&m_micro_decode[0], 0, 0x100*sizeof(UINT32));

	for (int op = 0; op < 0x100; op++)
	{
		// upper half of the opcodes is always branch/call
		if (op & 0x80)
			m_fixed_decode[op] = (op & 0x40) ? F_CALL: F_BR;

		// 5 output bits select a microinstruction index
		UINT32 imask = m_ipla->read(op);
		UINT8 msel = imask & 0x1f;

		// but if (from bottom to top) term 1 is active and output bit 5 is 0, R2,R4-R7 directly select a microinstruction index
		if (imask & 0x40 && (imask & 0x20) == 0)
			msel = (op & 0xf) | (op >> 1 & 0x10);

		msel = BITSWAP8(msel,7,6,5,0,1,2,3,4); // lines are reversed
		UINT32 mmask = m_mpla->read(msel);
		mmask ^= 0x09fe; // invert active-negative

		//                             _____  _____  _____  _____  ______  _____  ______  _____              _____
		const UINT32 md[15] = { M_CKM, M_CKP, M_YTP, M_MTP, M_ATN, M_NATN, M_MTN, M_15TN, M_CKN, M_NE, M_C8, M_CIN, M_AUTA, M_AUTY, M_STO };

		for (int bit = 0; bit < 15; bit++)
			if (mmask & (1 << bit))
				m_micro_decode[op] |= md[bit];

		// the other ipla terms each select a fixed instruction
		const UINT32 id[8] = { F_LDP, F_TDO, F_COMX, F_LDX, F_SBIT, F_RBIT, F_SETR, F_RETN };

		for (int bit = 0; bit < 8; bit++)
			if (imask & (0x80 << bit))
				m_fixed_decode[op] |= id[bit];
	}
}


UINT32 tms0980_cpu_device::decode_micro(UINT8 sel)
{
	UINT32 decode = 0;

	sel = BITSWAP8(sel,7,6,0,1,2,3,4,5); // lines are reversed
	UINT32 mask = m_mpla->read(sel);
	mask ^= 0x43fc3; // invert active-negative

	// M_RSTR is specific to TMS02x0/TMS1980, it redirects to F_RSTR
	// M_UNK1 is specific to TMS0270, unknown yet
	//                      _______  ______                                _____  _____  _____  _____  ______  _____  ______  _____                            _____
	const UINT32 md[22] = { M_NDMTP, M_DMTP, M_AUTY, M_AUTA, M_CKM, M_SSE, M_CKP, M_YTP, M_MTP, M_ATN, M_NATN, M_MTN, M_15TN, M_CKN, M_NE, M_C8, M_SSS, M_CME, M_CIN, M_STO, M_RSTR, M_UNK1 };

	for (int bit = 0; bit < 22 && bit < m_mpla->outputs(); bit++)
		if (mask & (1 << bit))
			decode |= md[bit];

	return decode;
}

void tms0980_cpu_device::device_reset()
{
	// common reset
	tms1xxx_cpu_device::device_reset();

	// pre-decode instructionset
	m_fixed_decode.resize(0x200);
	memset(&m_fixed_decode[0], 0, 0x200*sizeof(UINT32));
	m_micro_decode.resize(0x200);
	memset(&m_micro_decode[0], 0, 0x200*sizeof(UINT32));

	for (int op = 0; op < 0x200; op++)
	{
		// upper half of the opcodes is always branch/call
		if (op & 0x100)
			m_fixed_decode[op] = (op & 0x80) ? F_CALL: F_BR;

		UINT32 imask = m_ipla->read(op);

		// 6 output bits select a microinstruction index
		m_micro_decode[op] = decode_micro(imask & 0x3f);

		// the other ipla terms each select a fixed instruction
		const UINT32 id[15] = { F_LDP, F_SBL, F_OFF, F_RBIT, F_SAL, F_XDA, F_REAC, F_SETR, F_RETN, F_SBIT, F_TDO, F_COMX8, F_COMX, F_LDX, F_SEAC };

		for (int bit = 0; bit < 15; bit++)
			if (imask & (0x80 << bit))
				m_fixed_decode[op] |= id[bit];
	}

	// like on TMS0970, one of the terms directly select a microinstruction index (via R4-R8),
	// but it can't be pre-determined when it's active
	m_micro_direct.resize(0x40);
	memset(&m_micro_decode[0], 0, 0x40*sizeof(UINT32));

	for (int op = 0; op < 0x40; op++)
		m_micro_direct[op] = decode_micro(op);
}

void tms0270_cpu_device::device_reset()
{
	// common reset
	tms0980_cpu_device::device_reset();

	m_o_latch_low = 0;
	m_o_latch = 0;
	m_o_latch_prev = 0;
}



//-------------------------------------------------
//  program counter/opcode decode
//-------------------------------------------------

void tms1xxx_cpu_device::next_pc()
{
	// The program counter is a LFSR. To put it simply, the feedback bit is a XOR of the two highest bits,
	// but it makes an exception when all low bits are set (eg. in TMS1000 case, when PC is 0x1f or 0x3f).
	int high = 1 << (m_pc_bits - 1);
	int fb = (m_pc << 1 & high) == (m_pc & high);

	if (m_pc == (m_pc_mask >> 1))
		fb = 1;
	else if (m_pc == m_pc_mask)
		fb = 0;

	m_pc = (m_pc << 1 | fb) & m_pc_mask;
}

void tms1xxx_cpu_device::read_opcode()
{
	debugger_instruction_hook(this, m_rom_address);
	m_opcode = m_program->read_byte(m_rom_address);
	m_c4 = BITSWAP8(m_opcode,7,6,5,4,0,1,2,3) & 0xf; // opcode operand is bitswapped for most opcodes

	m_fixed = m_fixed_decode[m_opcode];
	m_micro = m_micro_decode[m_opcode];

	next_pc();
}

void tms0980_cpu_device::read_opcode()
{
	debugger_instruction_hook(this, m_rom_address << 1);
	m_opcode = m_program->read_word(m_rom_address << 1) & 0x1ff;
	m_c4 = BITSWAP8(m_opcode,7,6,5,4,0,1,2,3) & 0xf; // opcode operand is bitswapped for most opcodes

	m_fixed = m_fixed_decode[m_opcode];

	// if ipla term 0 is active, R4-R8 directly select a microinstruction index when R0 or R0^BL is 0
	int r0 = m_opcode >> 8 & 1;
	if (m_ipla->read(m_opcode) & 0x40 && !((r0 & m_bl) ^ r0))
		m_micro = m_micro_direct[m_opcode & 0x3f];
	else
		m_micro = m_micro_decode[m_opcode];

	// TMS02x0/TMS1980: RSTR is on the mpla
	if (m_micro & M_RSTR)
		m_fixed |= F_RSTR;

	next_pc();
}



//-------------------------------------------------
//  i/o handling
//-------------------------------------------------

void tms1xxx_cpu_device::write_o_output(UINT8 index)
{
	// a hardcoded table is supported if the output pla is unknown
	m_o = (m_output_pla_table == NULL) ? m_opla->read(index) : m_output_pla_table[index];
	m_write_o(0, m_o & m_o_mask, 0xffff);
}

void tms0970_cpu_device::write_o_output(UINT8 index)
{
	m_o = m_spla->read(index);
	m_write_o(0, m_o & m_o_mask, 0xffff);
}


void tms0270_cpu_device::dynamic_output()
{
	// R11: TMS5100 CTL port direction (0=read from TMS5100, 1=write to TMS5100)
	m_ctl_dir = m_r >> 11 & 1;

	// R12: chip select (off=display via OPLA, on=TMS5100 via ACC/CKB)
	m_chipsel = m_r >> 12 & 1;

	if (m_chipsel)
	{
		// ACC via SEG B,C,D,G: TMS5100 CTL pins
		if (m_ctl_dir && m_a != m_ctl_out)
		{
			m_ctl_out = m_a;
			m_write_ctl(0, m_ctl_out, 0xff);
		}

		// R10 via SEG E: TMS5100 PDC pin
		if (m_pdc != (m_r >> 10 & 1))
		{
			m_pdc = m_r >> 10 & 1;
			m_write_pdc(m_pdc);
		}
	}
	else
	{
		// standard O-output
		if (m_o_latch != m_o_latch_prev)
		{
			write_o_output(m_o_latch);
			m_o_latch_prev = m_o_latch;
		}
	}

	// standard R-output
	if (m_r != m_r_prev)
	{
		m_write_r(0, m_r & m_r_mask, 0xffff);
		m_r_prev = m_r;
	}
}


UINT8 tms1xxx_cpu_device::read_k_input()
{
	// K1,2,4,8 (KC test pin is not emulated)
	return m_read_k(0, 0xff) & 0xf;
}

UINT8 tms0980_cpu_device::read_k_input()
{
	UINT8 k = m_read_k(0, 0xff) & 0x1f;
	UINT8 k3 = (k & 0x10) ? 3: 0; // the TMS0980 K3 line is simply K1|K2
	return (k & 0xf) | k3;
}

UINT8 tms0270_cpu_device::read_k_input()
{
	// external: TMS5100 CTL port via SEG B,C,D,G
	if (m_chipsel)
		return (m_ctl_dir) ? m_ctl_out : m_read_ctl(0, 0xff) & 0xf;

	// standard K-input otherwise
	UINT8 k = m_read_k(0, 0xff) & 0x1f;
	return (k & 0x10) ? 0xf : k; // the TMS0270 KF line asserts all K-inputs
}


void tms1xxx_cpu_device::set_cki_bus()
{
	switch (m_opcode & 0xf8)
	{
		// 00001XXX: K-inputs
		case 0x08:
			m_cki_bus = read_k_input();
			break;

		// 0011XXXX: select bit
		case 0x30: case 0x38:
			m_cki_bus = 1 << (m_c4 >> 2) ^ 0xf;
			break;

		// 01XXXXXX: constant
		case 0x00: // R2,3,4 are NANDed with eachother, and then ORed with R1, making 00000XXX valid too
		case 0x40: case 0x48: case 0x50: case 0x58: case 0x60: case 0x68: case 0x70: case 0x78:
			m_cki_bus = m_c4;
			break;

		default:
			m_cki_bus = 0;
			break;
	}
}

void tms0980_cpu_device::set_cki_bus()
{
	switch (m_opcode & 0x1f8)
	{
		// 000001XXX: K-inputs
		case 0x008:
			m_cki_bus = read_k_input();
			break;

		// 0X0100XXX: select bit
		case 0x020: case 0x0a0:
			m_cki_bus = 1 << (m_c4 >> 2) ^ 0xf;
			break;

		// 0X1XXXXXX: constant
		case 0x040: case 0x048: case 0x050: case 0x058: case 0x060: case 0x068: case 0x070: case 0x078:
		case 0x0c0: case 0x0c8: case 0x0d0: case 0x0d8: case 0x0e0: case 0x0e8: case 0x0f0: case 0x0f8:
			m_cki_bus = m_c4;
			break;

		default:
			m_cki_bus = 0;
			break;
	}
}



//-------------------------------------------------
//  fixed opcode set
//-------------------------------------------------

// handle branches:

// TMS1000/common
// note: add(latch) and bl(branch latch) are specific to 0980 series,
// c(chapter) bits are specific to 1100(and 1400) series

void tms1xxx_cpu_device::op_br()
{
	// BR/BL: conditional branch
	if (m_status)
	{
		if (m_clatch == 0)
			m_pa = m_pb;
		m_ca = m_cb;
		m_pc = m_opcode & m_pc_mask;
	}
}

void tms1xxx_cpu_device::op_call()
{
	// CALL/CALLL: conditional call
	if (m_status)
	{
		UINT8 prev_pa = m_pa;

		if (m_clatch == 0)
		{
			m_clatch = 1;
			m_sr = m_pc;
			m_pa = m_pb;
			m_cs = m_ca;
		}
		m_ca = m_cb;
		m_pb = prev_pa;
		m_pc = m_opcode & m_pc_mask;
	}
}

void tms1xxx_cpu_device::op_retn()
{
	// RETN: return from subroutine
	if (m_clatch == 1)
	{
		m_clatch = 0;
		m_pc = m_sr;
		m_ca = m_cs;
	}
	m_add = 0;
	m_bl = 0;
	m_pa = m_pb;
}


// TMS1400-specific

void tms1400_cpu_device::op_br()
{
	// BR/BL: conditional branch
	if (m_status)
	{
		m_pa = m_pb; // don't care about clatch
		m_ca = m_cb;
		m_pc = m_opcode & m_pc_mask;
	}
}

void tms1400_cpu_device::op_call()
{
	// CALL/CALLL: conditional call
	if (m_status)
	{
		// 3-level stack, mask clatch 3 bits (no need to mask others)
		m_clatch = (m_clatch << 1 | 1) & 7;

		m_sr = m_sr << m_pc_bits | m_pc;
		m_pc = m_opcode & m_pc_mask;

		m_ps = m_ps << 4 | m_pa;
		m_pa = m_pb;

		m_cs = m_cs << 2 | m_ca;
		m_ca = m_cb;
	}
	else
	{
		m_pb = m_pa;
		m_cb = m_ca;
	}
}

void tms1400_cpu_device::op_retn()
{
	// RETN: return from subroutine
	if (m_clatch & 1)
	{
		m_clatch >>= 1;

		m_pc = m_sr & m_pc_mask;
		m_sr >>= m_pc_bits;

		m_pa = m_pb = m_ps & 0xf;
		m_ps >>= 4;

		m_ca = m_cb = m_cs & 3;
		m_cs >>= 2;
	}
}


// handle other:

// TMS1000/common

void tms1xxx_cpu_device::op_sbit()
{
	// SBIT: set memory bit
	if (m_ram_out == -1)
		m_ram_out = m_ram_in;
	m_ram_out |= (m_cki_bus ^ 0xf);
}

void tms1xxx_cpu_device::op_rbit()
{
	// RBIT: reset memory bit
	if (m_ram_out == -1)
		m_ram_out = m_ram_in;
	m_ram_out &= m_cki_bus;
}

void tms1xxx_cpu_device::op_setr()
{
	// SETR: set one R-output line
	m_r = m_r | (1 << m_y);
	m_write_r(0, m_r & m_r_mask, 0xffff);
}

void tms1xxx_cpu_device::op_rstr()
{
	// RSTR: reset one R-output line
	m_r = m_r & ~(1 << m_y);
	m_write_r(0, m_r & m_r_mask, 0xffff);
}

void tms1xxx_cpu_device::op_tdo()
{
	// TDO: transfer accumulator and status latch to O-output
	write_o_output(m_status_latch << 4 | m_a);
}

void tms1xxx_cpu_device::op_clo()
{
	// CLO: clear O-output
	write_o_output(0);
}

void tms1xxx_cpu_device::op_ldx()
{
	// LDX: load X register with (x_bits) constant
	m_x = m_c4 >> (4-m_x_bits);
}

void tms1xxx_cpu_device::op_comx()
{
	// COMX: complement X register
	m_x ^= m_x_mask;
}

void tms1xxx_cpu_device::op_comx8()
{
	// COMX8: complement MSB of X register
	// note: on TMS1100, the mnemonic is simply called "COMX"
	m_x ^= 1 << (m_x_bits-1);
}

void tms1xxx_cpu_device::op_ldp()
{
	// LDP: load page buffer with constant
	m_pb = m_c4;
}


// TMS1100-specific

void tms1100_cpu_device::op_setr()
{
	// SETR: same, but X register MSB must be clear
	if (~m_x & (1 << (m_x_bits-1)))
		tms1xxx_cpu_device::op_setr();
}

void tms1100_cpu_device::op_rstr()
{
	// RSTR: same, but X register MSB must be clear
	if (~m_x & (1 << (m_x_bits-1)))
		tms1xxx_cpu_device::op_rstr();
}

void tms1xxx_cpu_device::op_comc()
{
	// COMC: complement chapter buffer
	m_cb ^= 1;
}


// TMS1400-specific

void tms1xxx_cpu_device::op_tpc()
{
	// TPC: transfer page buffer to chapter buffer
	m_cb = m_pb & 3;
}


// TMS0970-specific (and possibly child classes)
void tms0970_cpu_device::op_setr()
{
	// SETR: set output register
	// DDIG line is a coincidence between the selected output pla row(s) and segment pla row(s)
	int ddig = (m_opla->read(m_a) & m_o) ? 0 : 1;
	m_r = (m_r & ~(1 << m_y)) | (ddig << m_y);
}

void tms0970_cpu_device::op_tdo()
{
	// TDO: transfer digits to output
	write_o_output(m_a & 0x7);
	m_write_r(0, m_r & m_r_mask, 0xffff);
}


// TMS0980-specific (and possibly child classes)

void tms0980_cpu_device::op_comx()
{
	// COMX: complement X register, but not the MSB
	m_x ^= (m_x_mask >> 1);
}

void tms1xxx_cpu_device::op_xda()
{
	// XDA: exchange DAM and A
	// note: setting A to DAM is done with DMTP and AUTA during this instruction
	m_ram_address |= (0x10 << (m_x_bits-1));
}

void tms1xxx_cpu_device::op_off()
{
	// OFF: request auto power-off
	m_power_off(1);
}

void tms1xxx_cpu_device::op_seac()
{
	// SEAC: set end around carry
	m_eac = 1;
}

void tms1xxx_cpu_device::op_reac()
{
	// REAC: reset end around carry
	m_eac = 0;
}

void tms1xxx_cpu_device::op_sal()
{
	// SAL: set add latch (reset is done with RETN)
	m_add = 1;
}

void tms1xxx_cpu_device::op_sbl()
{
	// SBL: set branch latch (reset is done with RETN)
	m_bl = 1;
}


// TMS1980-specific

void tms1980_cpu_device::op_tdo()
{
	// TDO: transfer accumulator and status(not status_latch!) to O-output
	write_o_output(m_status << 4 | m_a);
}


// TMS0270-specific

void tms0270_cpu_device::op_setr()
{
	// same as default, but handle write to output in dynamic_output
	m_r = m_r | (1 << m_y);
}

void tms0270_cpu_device::op_rstr()
{
	// same as default, but handle write to output in dynamic_output
	m_r = m_r & ~(1 << m_y);
}

void tms0270_cpu_device::op_tdo()
{
	// TDO: transfer data out
	if (m_status)
		m_o_latch_low = m_a;
	else
		m_o_latch = m_o_latch_low | (m_a << 4 & 0x30);

	// write to output is done in dynamic_output
}



//-------------------------------------------------
//  execute_run
//-------------------------------------------------

void tms1xxx_cpu_device::execute_run()
{
	do
	{
		m_icount--;
		switch (m_subcycle)
		{
		case 0:
			// fetch: rom address 1/2

			// execute: br/call 2/2
			if (m_fixed & F_BR)    op_br();
			if (m_fixed & F_CALL)  op_call();
			if (m_fixed & F_RETN)  op_retn();

			// execute: k input valid, read ram, clear alu inputs
			dynamic_output();
			set_cki_bus();
			m_ram_in = m_data->read_byte(m_ram_address) & 0xf;
			m_dam_in = m_data->read_byte(m_ram_address | (0x10 << (m_x_bits-1))) & 0xf;
			m_p = 0;
			m_n = 0;
			m_carry_in = 0;

			break;

		case 1:
			// fetch: rom address 2/2
			m_rom_address = (m_ca << (m_pc_bits+4)) | (m_pa << m_pc_bits) | m_pc;

			// execute: update alu inputs
			// N inputs
			if (m_micro & M_15TN)  m_n |= 0xf;
			if (m_micro & M_ATN)   m_n |= m_a;
			if (m_micro & M_NATN)  m_n |= (~m_a & 0xf);
			if (m_micro & M_CKN)   m_n |= m_cki_bus;
			if (m_micro & M_MTN)   m_n |= m_ram_in;

			// P inputs
			if (m_micro & M_CKP)   m_p |= m_cki_bus;
			if (m_micro & M_MTP)   m_p |= m_ram_in;
			if (m_micro & M_YTP)   m_p |= m_y;
			if (m_micro & M_DMTP)  m_p |= m_dam_in;
			if (m_micro & M_NDMTP) m_p |= (~m_dam_in & 0xf);

			// carry input
			if (m_micro & M_CIN)   m_carry_in |= 1;
			if (m_micro & M_SSS)   m_carry_in |= m_eac;

			break;

		case 2:
		{
			// fetch: nothing

			// execute: perform alu logic
			// note: officially, only 1 alu operation is allowed per opcode
			m_adder_out = m_p + m_n + m_carry_in;
			int carry_out = m_adder_out >> 4 & 1;
			int status = 1;
			m_ram_out = -1;

			if (m_micro & M_C8)    status &= carry_out;
			if (m_micro & M_NE)    status &= (m_n != m_p); // COMP
			if (m_micro & M_CKM)   m_ram_out = m_cki_bus;

			// special status circuit
			if (m_micro & M_SSE)
			{
				m_eac = m_carry_out;
				if (m_add)
					m_eac |= carry_out;
			}
			m_carry_out = carry_out;

			if (m_micro & M_STO || (m_micro & M_CME && m_eac == m_add))
				m_ram_out = m_a;

			// handle the other fixed opcodes here
			if (m_fixed & F_SBIT)  op_sbit();
			if (m_fixed & F_RBIT)  op_rbit();
			if (m_fixed & F_SETR)  op_setr();
			if (m_fixed & F_RSTR)  op_rstr();
			if (m_fixed & F_TDO)   op_tdo();
			if (m_fixed & F_CLO)   op_clo();
			if (m_fixed & F_LDX)   op_ldx();
			if (m_fixed & F_COMX)  op_comx();
			if (m_fixed & F_COMX8) op_comx8();
			if (m_fixed & F_LDP)   op_ldp();
			if (m_fixed & F_COMC)  op_comc();
			if (m_fixed & F_TPC)   op_tpc();
			if (m_fixed & F_OFF)   op_off();
			if (m_fixed & F_SEAC)  op_seac();
			if (m_fixed & F_REAC)  op_reac();
			if (m_fixed & F_SAL)   op_sal();
			if (m_fixed & F_SBL)   op_sbl();
			if (m_fixed & F_XDA)   op_xda();

			// after fixed opcode handling: store status, write ram
			m_status = status;
			if (m_ram_out != -1)
				m_data->write_byte(m_ram_address, m_ram_out);

			break;
		}

		case 3:
			// fetch: update pc, ram address 1/2
			// execute: register store 1/2
			break;

		case 4:
			// execute: register store 2/2
			if (m_micro & M_AUTA)  m_a = m_adder_out & 0xf;
			if (m_micro & M_AUTY)  m_y = m_adder_out & 0xf;
			if (m_micro & M_STSL)  m_status_latch = m_status;

			// fetch: update pc, ram address 2/2
			read_opcode();
			m_ram_address = m_x << 4 | m_y;
			break;

		case 5:
			// fetch: instruction decode (handled above, before next_pc)
			// execute: br/call 1/2
			break;
		}
		m_subcycle = (m_subcycle + 1) % 6;
	} while (m_icount > 0);
}
