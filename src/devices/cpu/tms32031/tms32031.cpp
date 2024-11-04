// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    tms32031.cpp

    TMS320C3x family 32-bit floating point DSP emulator

    TMS320C3x family difference table:

    |-------------------|-------------------|-------------------|-------------------|
    | Feature           | 'C30              | 'C31/'VC33        | 'C32              |
    |-------------------|-------------------|-------------------|-------------------|
    | External Bus      | Two buses:        | One bus:          | One bus:          |
    |                   | Primary bus:      | 32-bit data       | 32-bit data       |
    |                   | 32-bit data       | 24-bit address    | 24-bit address    |
    |                   | 24-bit address    | STRB active for   | STRB active for   |
    |                   | STRB active for   | 000000-7FFFFFh    | 000000-7FFFFFh    |
    |                   | 000000-7FFFFFh    | and               | and               |
    |                   | and               | 80A000-FFFFFFh    | 880000-8FFFFFh    |
    |                   | 80A000-FFFFFFh    |                   | 8-, 16-, 32-bit   |
    |                   | Expansion bus:    |                   | data in           |
    |                   | 32-bit data       |                   | 8-, 16-, 32-bit   |
    |                   | 13-bit address    |                   | wide memory       |
    |                   | MSTRB active for  |                   | STRB1 active for  |
    |                   | 800000-801FFFh    |                   | 900000-FFFFFFh    |
    |                   | IOSTRB active for |                   | 8-, 16-, 32-bit   |
    |                   | 804000-805FFFh    |                   | data in           |
    |                   |                   |                   | 8-, 16-, 32-bit   |
    |                   |                   |                   | wide memory       |
    |                   |                   |                   | IOSTRB active for |
    |                   |                   |                   | 810000-82FFFFh    |
    |-------------------|-------------------|-------------------|-------------------|
    | ROM (Words)       | 4K                | No                | No                |
    |-------------------|-------------------|-------------------|-------------------|
    | Boot Loader       | No                | Yes               | Yes               |
    |-------------------|-------------------|-------------------|-------------------|
    | On-Chip RAM       | 2k                | 2k('31)/34k('33)  | 512               |
    | (Words)           | Address:          | Address:          | Address:          |
    |                   | 809800-809fff     | 809800-809fff     | 87fe00-87ffff     |
    |                   |                   | ('C31,'VC33)      |                   |
    |                   |                   | 800000-807fff     |                   |
    |                   |                   | ('VC33 only)      |                   |
    |-------------------|-------------------|-------------------|-------------------|
    | DMA               | 1 Channel         | 1 Channel         | 2 Channels        |
    |                   | CPU greater       | CPU greater       | Configurable      |
    |                   | priority then DMA | priority then DMA | priorities        |
    |-------------------|-------------------|-------------------|-------------------|
    | Serial Ports      | 2                 | 1                 | 1                 |
    |-------------------|-------------------|-------------------|-------------------|
    | Timers            | 2                 | 2                 | 2                 |
    |-------------------|-------------------|-------------------|-------------------|
    | Interrupts        | Level-Triggered   | Level-Triggered   | Level-Triggered   |
    |                   |                   |                   | or combination of |
    |                   |                   |                   | edge- and         |
    |                   |                   |                   | level-triggered   |
    |-------------------|-------------------|-------------------|-------------------|
    | Interrupt vector  | Fixed 0-3Fh       | Microprocessor:   | Relocatable       |
    | table             |                   | 0-3Fh fixed       |                   |
    |                   |                   | Boot loader:      |                   |
    |                   |                   | 809FC1-809FFF     |                   |
    |                   |                   | fixed             |                   |
    |-------------------|-------------------|-------------------|-------------------|

    TODO:
    - merge and implement internal peripheral emulations
    - implement chip family difference
    - interlocked operation
    - instruction pipelining

***************************************************************************/

#include "emu.h"
#include "tms32031.h"
#include "dis32031.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// indexes into the register file
enum
{
	TMR_R0 = 0,
	TMR_R1,
	TMR_R2,
	TMR_R3,
	TMR_R4,
	TMR_R5,
	TMR_R6,
	TMR_R7,
	TMR_AR0,
	TMR_AR1,
	TMR_AR2,
	TMR_AR3,
	TMR_AR4,
	TMR_AR5,
	TMR_AR6,
	TMR_AR7,
	TMR_DP,
	TMR_IR0,
	TMR_IR1,
	TMR_BK,
	TMR_SP,
	TMR_ST,
	TMR_IE,
	TMR_IF,
	TMR_IOF,
	TMR_RS,
	TMR_RE,
	TMR_RC,
	TMR_R8,     // 3204x only
	TMR_R9,     // 3204x only
	TMR_R10,    // 3204x only
	TMR_R11,    // 3204x only
	TMR_TEMP1,  // used by the interpreter
	TMR_TEMP2,  // used by the interpreter
	TMR_TEMP3   // used by the interpreter
};

// flags
const int CFLAG     = 0x0001;
const int VFLAG     = 0x0002;
const int ZFLAG     = 0x0004;
const int NFLAG     = 0x0008;
const int UFFLAG    = 0x0010;
const int LVFLAG    = 0x0020;
const int LUFFLAG   = 0x0040;
const int OVMFLAG   = 0x0080;
const int RMFLAG    = 0x0100;
//const int CFFLAG    = 0x0400;
//const int CEFLAG    = 0x0800;
//const int CCFLAG    = 0x1000;
const int GIEFLAG   = 0x2000;



//**************************************************************************
//  MACROS
//**************************************************************************

#define IREG(rnum)  (m_r[rnum].i32[0])



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TMS32030, tms32030_device, "tms32030", "Texas Instruments TMS320C30")
DEFINE_DEVICE_TYPE(TMS32031, tms32031_device, "tms32031", "Texas Instruments TMS320C31")
DEFINE_DEVICE_TYPE(TMS32032, tms32032_device, "tms32032", "Texas Instruments TMS320C32")
DEFINE_DEVICE_TYPE(TMS32033, tms32033_device, "tms32033", "Texas Instruments TMS320VC33")

// memory map common to all 'C30 devices
// TODO: expand to cover all the standard internal peripherals
void tms3203x_device::common_3203x(address_map &map)
{
	//map(0x808000, 0x808000) DMA (0) Global control
	//map(0x808004, 0x808004) DMA (0) Source address
	//map(0x808006, 0x808006) DMA (0) Destination address
	//map(0x808008, 0x808008) DMA (0) Transfer Counter
	//map(0x808020, 0x808020) Timer 0 Global Control
	//map(0x808024, 0x808024) Timer 0 Counter
	//map(0x808028, 0x808028) Timer 0 Period Register
	//map(0x808030, 0x808030) Timer 1 Global Control
	//map(0x808034, 0x808034) Timer 1 Counter
	//map(0x808038, 0x808038) Timer 1 Period Register
	//map(0x808040, 0x808040) Serial Port (0) Global Control
	//map(0x808042, 0x808042) FSX/DX/CLKX Serial Port (0) Control
	//map(0x808043, 0x808043) FSR/DR/CLKR Serial Port (0) Control
	//map(0x808044, 0x808044) Serial Port (0) R/X Timer Control
	//map(0x808045, 0x808045) Serial Port (0) R/X Timer Counter
	//map(0x808046, 0x808046) Serial Port (0) R/X Timer Period Register
	//map(0x808048, 0x808048) Serial Port (0) Data-Transmit
	//map(0x80804c, 0x80804c) Serial Port (0) Data-Receive
	map(0x808064, 0x808064).rw(FUNC(tms3203x_device::primary_bus_control_r), FUNC(tms3203x_device::primary_bus_control_w));
}

// internal memory maps
void tms32030_device::internal_32030(address_map &map)
{
	common_3203x(map);

	//map(0x000000, 0x7fffff) STRB
	//map(0x800000, 0x801fff) MSTRB
	//map(0x804000, 0x805fff) IOSTRB
	//map(0x808050, 0x808050) Serial Port 1 Global Control
	//map(0x808052, 0x808052) FSX/DX/CLKX Serial Port 1 Control
	//map(0x808053, 0x808053) FSR/DR/CLKR Serial Port 1 Control
	//map(0x808054, 0x808054) Serial Port 1 R/X Timer Control
	//map(0x808055, 0x808055) Serial Port 1 R/X Timer Counter
	//map(0x808056, 0x808056) Serial Port 1 R/X Timer Period Register
	//map(0x808058, 0x808058) Serial Port 1 Data-Transmit
	//map(0x80805c, 0x80805c) Serial Port 1 Data-Receive
	//map(0x808060, 0x808060) Expansion-Bus Control
	//map(0x808064, 0x808064) Primary-Bus Control
	map(0x809800, 0x809fff).ram();
	//map(0x809800, 0x809bff).ram(); // RAM block 0
	//map(0x809c00, 0x809fff).ram(); // RAM block 1
	//map(0x80a000, 0xffffff) STRB
}

void tms32031_device::internal_32031(address_map &map)
{
	common_3203x(map);

	//map(0x000000, 0x7fffff) STRB
	//map(0x808064, 0x808064) Primary-Bus Control
	map(0x809800, 0x809fff).ram();
	//map(0x809800, 0x809bff).ram(); // RAM block 0
	//map(0x809c00, 0x809fff).ram(); // RAM block 1
	//map(0x80a000, 0xffffff) STRB
}

void tms32032_device::internal_32032(address_map &map)
{
	common_3203x(map);

	//map(0x000000, 0x7fffff) STRB0
	//map(0x808010, 0x808010) DMA 1 Global control
	//map(0x808014, 0x808014) DMA 1 Source address
	//map(0x808016, 0x808016) DMA 1 Destination address
	//map(0x808018, 0x808018) DMA 1 Transfer Counter
	//map(0x808060, 0x808060) IOSTRB Bus Control
	//map(0x808064, 0x808064) STRB0 Bus Control
	//map(0x808068, 0x808068) STRB1 Bus Control
	//map(0x810000, 0x82ffff) IOSTRB
	map(0x87fe00, 0x87ffff).ram();
	//map(0x87fe00, 0x87feff).ram(); // RAM block 0
	//map(0x87ff00, 0x87ffff).ram(); // RAM block 1
	//map(0x880000, 0x8fffff) STRB0
	//map(0x900000, 0xffffff) STRB1
}

void tms32033_device::internal_32033(address_map &map)
{
	common_3203x(map);

	//map(0x000000, 0x7fffff) STRB
	map(0x800000, 0x807fff).ram();
	//map(0x800000, 0x803fff).ram(); // RAM block 2
	//map(0x804000, 0x807fff).ram(); // RAM block 3
	//map(0x808064, 0x808064) Primary-Bus Control
	map(0x809800, 0x809fff).ram();
	//map(0x809800, 0x809bff).ram(); // RAM block 0
	//map(0x809c00, 0x809fff).ram(); // RAM block 1
	//map(0x80a000, 0xffffff) STRB
}


// ROM definitions for the internal boot loader programs
// (Using assembled versions until the code ROMs are extracted from both DSPs)
ROM_START( tms32030 )
	ROM_REGION(0x4000, "internal_rom", 0)
	ROM_LOAD( "c30boot.bin", 0x0000, 0x4000, BAD_DUMP CRC(bddc2763) SHA1(96b2170ecee5bec5abaa1741bb2d3b6096ecc262)) // TODO: programmable?
ROM_END

ROM_START( tms32031 )
	ROM_REGION(0x4000, "internal_rom", 0)
	ROM_LOAD( "c31boot.bin", 0x0000, 0x4000, BAD_DUMP CRC(bddc2763) SHA1(96b2170ecee5bec5abaa1741bb2d3b6096ecc262) ) // Assembled from c31boot.asm (02-07-92)
ROM_END

ROM_START( tms32032 )
	ROM_REGION(0x4000, "internal_rom", 0)
	ROM_LOAD( "c32boot.bin", 0x0000, 0x4000, BAD_DUMP CRC(ecf84729) SHA1(4d32ead450f921f563514b061ea561a222283616) ) // Assembled from c32boot.asm (03-04-96)
ROM_END



//**************************************************************************
//  TMSREG REGISTER
//**************************************************************************

//-------------------------------------------------
//  as_float - interpret the contents of a tmsreg
//  as a DSP-encoded floating-point value, and
//  extract a 32-bit IEEE float from it
//-------------------------------------------------

float tms3203x_device::tmsreg::as_float() const
{
	int_double id;

	// map 0 to 0
	if (mantissa() == 0 && exponent() == -128)
		return 0;

	// handle positive numbers
	else if (mantissa() >= 0)
	{
		int exp = (exponent() + 127) << 23;
		id.i[0] = exp + (mantissa() >> 8);
	}

	// handle negative numbers
	else
	{
		int exp = (exponent() + 127) << 23;
		int32_t man = -mantissa();
		id.i[0] = 0x80000000 + exp + ((man >> 8) & 0x00ffffff);
	}

	// return the converted float
	return id.f[0];
}


//-------------------------------------------------
//  as_double - interpret the contents of a tmsreg
//  as a DSP-encoded floating-point value, and
//  extract a 64-bit IEEE double from it
//-------------------------------------------------

double tms3203x_device::tmsreg::as_double() const
{
	int_double id;

	// map 0 to 0
	if (mantissa() == 0 && exponent() == -128)
		return 0;

	// handle positive numbers
	else if (mantissa() >= 0)
	{
		int exp = (exponent() + 1023) << 20;
		id.i[BYTE_XOR_BE(0)] = exp + (mantissa() >> 11);
		id.i[BYTE_XOR_BE(1)] = (mantissa() << 21) & 0xffe00000;
	}

	// handle negative numbers
	else
	{
		int exp = (exponent() + 1023) << 20;
		int32_t man = -mantissa();
		id.i[BYTE_XOR_BE(0)] = 0x80000000 + exp + ((man >> 11) & 0x001fffff);
		id.i[BYTE_XOR_BE(1)] = (man << 21) & 0xffe00000;
	}

	// return the converted double
	return id.d;
}


//-------------------------------------------------
//  from_double - import a 64-bit IEEE double into
//  the DSP's internal floating point format
//-------------------------------------------------

void tms3203x_device::tmsreg::from_double(double val)
{
	// extract mantissa and exponent from the IEEE input
	int_double id;
	id.d = val;
	int32_t mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xffe00000) >> 21);
	int32_t exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023;

	// if we're too small, map to 0
	if (exponent < -128)
	{
		set_mantissa(0);
		set_exponent(-128);
	}

	// if we're too large, map to the maximum value
	else if (exponent > 127)
	{
		if ((int32_t)id.i[BYTE_XOR_BE(0)] >= 0)
			set_mantissa(0x7fffffff);
		else
			set_mantissa(0x80000001);
		set_exponent(127);
	}

	// if we're positive, map directly
	else if ((int32_t)id.i[BYTE_XOR_BE(0)] >= 0)
	{
		set_mantissa(mantissa);
		set_exponent(exponent);
	}

	// if we're negative with a non-zero mantissa, remove the leading sign bit
	else if (mantissa != 0)
	{
		set_mantissa(0x80000000 | -mantissa);
		set_exponent(exponent);
	}

	// if we're negative with a zero mantissa, normalize
	else
	{
		set_mantissa(0x80000000);
		set_exponent(exponent - 1);
	}
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  tms3203x_device - constructor
//-------------------------------------------------

tms3203x_device::tms3203x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t chiptype, int clock_per_inst, address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock),
		m_program_config("program", ENDIANNESS_LITTLE, 32, 24, -2, internal_map),
		m_chip_type(chiptype),
		m_pc(0),
		m_bkmask(0),
		m_primary_bus_control(0),
		m_irq_state(0),
		m_delayed(false),
		m_irq_pending(false),
		m_is_idling(false),
		m_icount(0),
		m_clock_per_inst(clock_per_inst),  // 1('VC33)/2 clocks per cycle
		m_internal_rom(*this, "internal_rom"),
		m_mcbl_mode(false),
		m_is_lopower(false),
		m_xf0_cb(*this),
		m_xf1_cb(*this),
		m_iack_cb(*this),
		m_holda_cb(*this)
{
	// initialize remaining state
	memset(&m_r, 0, sizeof(m_r));

	// set our instruction counter
	set_icountptr(m_icount);

#if (TMS_3203X_LOG_OPCODE_USAGE)
	memset(m_hits, 0, sizeof(m_hits));
#endif
}

tms32030_device::tms32030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms3203x_device(mconfig, TMS32030, tag, owner, clock, CHIP_TYPE_TMS32030, 2, address_map_constructor(FUNC(tms32030_device::internal_32030), this))
{
}

tms32031_device::tms32031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms3203x_device(mconfig, TMS32031, tag, owner, clock, CHIP_TYPE_TMS32031, 2, address_map_constructor(FUNC(tms32031_device::internal_32031), this))
{
}

tms32032_device::tms32032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms3203x_device(mconfig, TMS32032, tag, owner, clock, CHIP_TYPE_TMS32032, 2, address_map_constructor(FUNC(tms32032_device::internal_32032), this))
{
}

tms32033_device::tms32033_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms3203x_device(mconfig, TMS32033, tag, owner, clock, CHIP_TYPE_TMS32031, 1, address_map_constructor(FUNC(tms32033_device::internal_32033), this))
{
}

//-------------------------------------------------
//  ~tms3203x_device - destructor
//-------------------------------------------------

tms3203x_device::~tms3203x_device()
{
#if (TMS_3203X_LOG_OPCODE_USAGE)
	for (int i = 0; i < std::size(m_hits); i++)
		if (m_hits[i] != 0)
			printf("%10d - %03X.%X\n", m_hits[i], i / 4, i % 4);
#endif
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *tms3203x_device::device_rom_region() const
{
	switch (m_chip_type)
	{
		default:
		case CHIP_TYPE_TMS32030:    return ROM_NAME( tms32030 );
		case CHIP_TYPE_TMS32031:    return ROM_NAME( tms32031 );
		case CHIP_TYPE_TMS32032:    return ROM_NAME( tms32032 );
	}
}

//-------------------------------------------------
//  ROPCODE - fetch an opcode
//-------------------------------------------------

inline uint32_t tms3203x_device::ROPCODE(offs_t pc)
{
	return m_cache.read_dword(pc);
}


//-------------------------------------------------
//  RMEM - read memory
//-------------------------------------------------

inline uint32_t tms3203x_device::RMEM(offs_t addr)
{
	return m_program.read_dword(addr);
}


//-------------------------------------------------
//  WMEM - write memory
//-------------------------------------------------

inline void tms3203x_device::WMEM(offs_t addr, uint32_t data)
{
	m_program.write_dword(addr, data);
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void tms3203x_device::device_start()
{
	// find address spaces
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	// set up the internal boot loader ROM
	if (m_mcbl_mode)
	{
		if (m_internal_rom->base() != nullptr)
			m_program.space().install_rom(0x000000, 0x000fff, m_internal_rom->base());
		else
			m_program.space().unmap_read(0x000000, 0x000fff);
	}

	// save state
	save_item(NAME(m_pc));
	for (int regnum = 0; regnum < 36; regnum++)
		save_item(NAME(m_r[regnum].i32), regnum);
	save_item(NAME(m_bkmask));
	save_item(NAME(m_primary_bus_control));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_delayed));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_is_idling));
	save_item(NAME(m_mcbl_mode));
	save_item(NAME(m_hold_state));
	save_item(NAME(m_is_lopower));

	// register our state for the debugger
	state_add(TMS3203X_PC,      "PC",        m_pc);
	state_add(STATE_GENPC,      "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE,  "CURPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS",  m_r[TMR_ST].i32[0]).mask(0xff).noshow().formatstr("%8s");
	state_add(TMS3203X_R0,      "R0",        m_r[TMR_R0].i32[0]);
	state_add(TMS3203X_R1,      "R1",        m_r[TMR_R1].i32[0]);
	state_add(TMS3203X_R2,      "R2",        m_r[TMR_R2].i32[0]);
	state_add(TMS3203X_R3,      "R3",        m_r[TMR_R3].i32[0]);
	state_add(TMS3203X_R4,      "R4",        m_r[TMR_R4].i32[0]);
	state_add(TMS3203X_R5,      "R5",        m_r[TMR_R5].i32[0]);
	state_add(TMS3203X_R6,      "R6",        m_r[TMR_R6].i32[0]);
	state_add(TMS3203X_R7,      "R7",        m_r[TMR_R7].i32[0]);
	state_add(TMS3203X_R0F,     "R0F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R1F,     "R1F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R2F,     "R2F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R3F,     "R3F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R4F,     "R4F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R5F,     "R5F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R6F,     "R6F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R7F,     "R7F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_AR0,     "AR0",       m_r[TMR_AR0].i32[0]);
	state_add(TMS3203X_AR1,     "AR1",       m_r[TMR_AR1].i32[0]);
	state_add(TMS3203X_AR2,     "AR2",       m_r[TMR_AR2].i32[0]);
	state_add(TMS3203X_AR3,     "AR3",       m_r[TMR_AR3].i32[0]);
	state_add(TMS3203X_AR4,     "AR4",       m_r[TMR_AR4].i32[0]);
	state_add(TMS3203X_AR5,     "AR5",       m_r[TMR_AR5].i32[0]);
	state_add(TMS3203X_AR6,     "AR6",       m_r[TMR_AR6].i32[0]);
	state_add(TMS3203X_AR7,     "AR7",       m_r[TMR_AR7].i32[0]);
	state_add(TMS3203X_DP,      "DP",        m_r[TMR_DP].i32[0]).mask(0xff);
	state_add(TMS3203X_IR0,     "IR0",       m_r[TMR_IR0].i32[0]);
	state_add(TMS3203X_IR1,     "IR1",       m_r[TMR_IR1].i32[0]);
	state_add(TMS3203X_BK,      "BK",        m_r[TMR_BK].i32[0]);
	state_add(TMS3203X_SP,      "SP",        m_r[TMR_SP].i32[0]);
	state_add(TMS3203X_ST,      "ST",        m_r[TMR_ST].i32[0]);
	state_add(TMS3203X_IE,      "IE",        m_r[TMR_IE].i32[0]);
	state_add(TMS3203X_IF,      "IF",        m_r[TMR_IF].i32[0]);
	state_add(TMS3203X_IOF,     "IOF",       m_r[TMR_IOF].i32[0]);
	state_add(TMS3203X_RS,      "RS",        m_r[TMR_RS].i32[0]);
	state_add(TMS3203X_RE,      "RE",        m_r[TMR_RE].i32[0]);
	state_add(TMS3203X_RC,      "RC",        m_r[TMR_RC].i32[0]);
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void tms3203x_device::device_reset()
{
	m_pc = RMEM(0);

	// reset some registers
	IREG(TMR_IE) = 0;
	IREG(TMR_IF) = 0;
	IREG(TMR_ST) = 0;
	IREG(TMR_IOF) = 0;

	// update IF with the external interrupt state (required for boot loader operation)
	IREG(TMR_IF) |= m_irq_state & 0x0f;

	// reset peripheral registers
	m_primary_bus_control = 0x000010f8;

	// reset internal stuff
	m_delayed = m_irq_pending = m_is_idling = m_is_lopower = false;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector tms3203x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void tms3203x_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].from_double(*(float *)&m_iotemp);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(tms3203x) called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_export - export state into the device,
//  before returning it to the caller
//-------------------------------------------------

void tms3203x_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			*(float *)&m_iotemp = m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].as_float();
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(tms3203x) called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void tms3203x_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			str = string_format("%12g", m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].as_double());
			break;

		case STATE_GENFLAGS:
			uint32_t temp = m_r[TMR_ST].i32[0];
			str = string_format("%c%c%c%c%c%c%c%c",
				(temp & 0x80) ? 'O':'.',
				(temp & 0x40) ? 'U':'.',
				(temp & 0x20) ? 'V':'.',
				(temp & 0x10) ? 'u':'.',
				(temp & 0x08) ? 'n':'.',
				(temp & 0x04) ? 'z':'.',
				(temp & 0x02) ? 'v':'.',
				(temp & 0x01) ? 'c':'.');
			break;
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> tms3203x_device::create_disassembler()
{
	return std::make_unique<tms32031_disassembler>();
}



//**************************************************************************
//  PUBLIC INTERFACES
//**************************************************************************

//-------------------------------------------------
//  fp_to_float - convert a 32-bit value from DSP
//  floating-point format a 32-bit IEEE float
//-------------------------------------------------

float tms3203x_device::fp_to_float(uint32_t floatdata)
{
	tmsreg gen(floatdata << 8, (int32_t)floatdata >> 24);
	return gen.as_float();
}


//-------------------------------------------------
//  fp_to_double - convert a 32-bit value from DSP
//  floating-point format a 64-bit IEEE double
//-------------------------------------------------

double tms3203x_device::fp_to_double(uint32_t floatdata)
{
	tmsreg gen(floatdata << 8, (int32_t)floatdata >> 24);
	return gen.as_double();
}


//-------------------------------------------------
//  float_to_fp - convert a 32-bit IEEE float to
//  a 32-bit DSP floating-point value
//-------------------------------------------------

uint32_t tms3203x_device::float_to_fp(float fval)
{
	tmsreg gen(fval);
	return (gen.exponent() << 24) | ((uint32_t)gen.mantissa() >> 8);
}


//-------------------------------------------------
//  double_to_fp - convert a 64-bit IEEE double to
//  a 32-bit DSP floating-point value
//-------------------------------------------------

uint32_t tms3203x_device::double_to_fp(double dval)
{
	tmsreg gen(dval);
	return (gen.exponent() << 24) | ((uint32_t)gen.mantissa() >> 8);
}



//**************************************************************************
//  EXECUTION
//**************************************************************************

//-------------------------------------------------
//  check_irqs - check for pending IRQs and take
//  them if enabled
//-------------------------------------------------

void tms3203x_device::check_irqs()
{
	// determine if we have any live interrupts
	uint16_t validints = IREG(TMR_IF) & IREG(TMR_IE) & 0x0fff;
	if (validints == 0 || (IREG(TMR_ST) & GIEFLAG) == 0)
		return;

	// find the lowest signalled value
	int whichtrap = 0;
	for (int i = 0; i < 12; i++)
		if (validints & (1 << i))
		{
			whichtrap = i + 1;
			break;
		}

	// no longer idling if we get here
	m_is_idling = false;
	if (!m_delayed)
	{
		uint16_t intmask = 1 << (whichtrap - 1);

		// bit in IF is cleared when interrupt is taken
		IREG(TMR_IF) &= ~intmask;
		trap(whichtrap);

		// after auto-clearing the interrupt bit, we need to re-trigger
		// level-sensitive interrupts
		if (m_chip_type == CHIP_TYPE_TMS32031 || (IREG(TMR_ST) & 0x4000) == 0)
			IREG(TMR_IF) |= m_irq_state & 0x0f;
	}
	else
		m_irq_pending = true;
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t tms3203x_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t tms3203x_device::execute_max_cycles() const noexcept
{
	return 5 * 16; // max opcode cycle * low power operation mode
}


//-------------------------------------------------
//  execute_clocks_to_cycles - convert the raw
//  clock into cycles per second
//-------------------------------------------------

uint64_t tms3203x_device::execute_clocks_to_cycles(uint64_t clocks) const noexcept
{
	return (clocks + m_clock_per_inst - 1) / m_clock_per_inst;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert a cycle
//  count back to raw clocks
//-------------------------------------------------

uint64_t tms3203x_device::execute_cycles_to_clocks(uint64_t cycles) const noexcept
{
	return cycles * m_clock_per_inst;
}


//-------------------------------------------------
//  execute_set_input - set input and IRQ lines
//-------------------------------------------------

void tms3203x_device::execute_set_input(int inputnum, int state)
{
	if (inputnum == TMS3203X_MCBL)
	{
		// switch between microcomputer/boot loader and microprocessor modes
		bool old_mode = m_mcbl_mode;
		m_mcbl_mode = (state == ASSERT_LINE);
		if (m_mcbl_mode != old_mode)
		{
			if (m_mcbl_mode && (m_internal_rom->base() != nullptr))
				m_program.space().install_rom(0x000000, 0x000fff, m_internal_rom->base());
			else
				m_program.space().unmap_read(0x000000, 0x000fff);
		}
		return;
	}

	if (inputnum == TMS3203X_HOLD)
	{
		m_hold_state = (state == ASSERT_LINE);

		// FIXME: "there is a minimum of one cycle delay from the time when
		// the processor recognises /HOLD = 0 until /HOLDA = 0"
		if (m_hold_state)
		{
			// assert hold acknowledge if external hold enabled
			if (!(m_primary_bus_control & NOHOLD))
			{
				m_primary_bus_control |= HOLDST;
				m_holda_cb(ASSERT_LINE);
			}
		}
		else
		{
			// clear hold acknowledge if port is held externally
			if ((m_primary_bus_control & HOLDST) && !(m_primary_bus_control & HIZ))
			{
				m_primary_bus_control &= ~HOLDST;
				m_holda_cb(CLEAR_LINE);
			}
		}
		return;
	}

	// update the external state
	uint16_t intmask = 1 << inputnum;
	if (state == ASSERT_LINE)
	{
		m_irq_state |= intmask;
		IREG(TMR_IF) |= intmask;
	}
	else
		m_irq_state &= ~intmask;

	// external interrupts are level-sensitive on the '31 and can be
	// configured as such on the '32; in that case, if the external
	// signal is high, we need to update the value in IF accordingly
	if (m_chip_type != CHIP_TYPE_TMS32032 || (IREG(TMR_ST) & 0x4000) == 0)
		IREG(TMR_IF) |= m_irq_state & 0x0f;
}


//-------------------------------------------------
//  execute_run - execute until our icount expires
//-------------------------------------------------

void tms3203x_device::execute_run()
{
	// check IRQs up front
	check_irqs();

	// if we're idling, just eat the cycles
	if (m_is_idling)
	{
		m_icount = 0;
		return;
	}

	// non-debug case
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		while (m_icount > 0)
		{
			if ((IREG(TMR_ST) & RMFLAG) && m_pc == IREG(TMR_RE) + 1)
			{
				if ((int32_t)--IREG(TMR_RC) >= 0)
					m_pc = IREG(TMR_RS);
				else
				{
					IREG(TMR_ST) &= ~RMFLAG;
					if (m_delayed)
					{
						m_delayed = false;
						if (m_irq_pending)
						{
							m_irq_pending = false;
							check_irqs();
						}
					}
				}
				continue;
			}

			execute_one();
		}
	}

	// debugging case
	else
	{
		while (m_icount > 0)
		{
			// watch for out-of-range stack pointers
			if (IREG(TMR_SP) & 0xff000000)
				machine().debug_break();
			if ((IREG(TMR_ST) & RMFLAG) && m_pc == IREG(TMR_RE) + 1)
			{
				if ((int32_t)--IREG(TMR_RC) >= 0)
					m_pc = IREG(TMR_RS);
				else
				{
					IREG(TMR_ST) &= ~RMFLAG;
					if (m_delayed)
					{
						m_delayed = false;
						if (m_irq_pending)
						{
							m_irq_pending = false;
							check_irqs();
						}
					}
				}
				continue;
			}

			debugger_instruction_hook(m_pc);
			execute_one();
		}
	}
}

// internal peripherals
void tms3203x_device::primary_bus_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// change in internal hold state
	if ((m_primary_bus_control ^ data) & HIZ)
	{
		if (m_primary_bus_control & HOLDST)
			m_primary_bus_control &= ~HOLDST;
		else
			m_primary_bus_control |= HOLDST;
		m_holda_cb(data & HIZ ? ASSERT_LINE : CLEAR_LINE);
	}

	// enable of external hold with hold pending
	if ((m_primary_bus_control & NOHOLD) && !(data & NOHOLD) && m_hold_state)
	{
		m_primary_bus_control |= HOLDST;
		m_holda_cb(ASSERT_LINE);
	}

	// update register
	m_primary_bus_control = (m_primary_bus_control & ~(mem_mask | WMASK)) | (data & mem_mask & WMASK);
}


//**************************************************************************
//  CORE OPCODES
//**************************************************************************

#include "32031ops.hxx"
