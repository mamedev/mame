// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

    Intel MCS-48/UPI-41 Portable Emulator

    Copyright Mirko Buffoni
    Based on the original work Copyright Dan Boris, an 8048 emulator

    TODO:
    - EA pin - defined by architecture, must implement:
      1 means external access, bypassing internal ROM
      reimplement as a push, not a pull
    - add CMOS devices, 1 new opcode (01 IDL)
    - add special 8022 opcodes (RAD, SEL AN0, SEL AN1, RETI)
    - according to the user manual, some opcodes(dis/enable timer/interrupt)
      don't increment the timer, does it affect the prescaler too?
    - IRQ timing is hacked due to WY-100 needing to take JNI branch before servicing interrupt

****************************************************************************

    Note that the default internal divisor for this chip is by 3 and
    then again by 5, or by 15 total.

    Chip   RAM  ROM  I/O
    ----   ---  ---  ---
    8021    64   1k   21  (ROM, reduced instruction set)
    8022    64   2k   26  (ROM, reduced instruction set, analog comparator)

    8035    64    0   27  (external ROM)
    8048    64   1k   27  (ROM)
    8648    64   1k   27  (OTPROM)
    8748    64   1k   27  (EPROM)
    8884    64   1k
    N7751  128   2k

    8039   128    0   27  (external ROM)
    8049   128   2k   27  (ROM)
    8749   128   2k   27  (EPROM)
    M58715 128    0       (external ROM)

    8040   256   4k   27  (external ROM)
    8050   256   4k   27  (ROM)

****************************************************************************

    UPI-41/42 chips are MCS-48 derived, with some opcode changes:

            MCS-48 opcode       UPI-41/42 opcode
            -------------       ----------------
        02: OUTL BUS,A          OUT  DBB,A
        08: INS  BUS,A          <illegal>
        22: <illegal>           IN   DBB,A
        75: ENT0 CLK            <illegal>
        80: MOVX A,@R0          <illegal>
        81: MOVX A,@R1          <illegal>
        86: JNI  <dest>         JOBF <dest>
        88: ORL  BUS,#n         <illegal>
        90: MOVX @R0,A          MOV  STS,A
        91: MOVX @R1,A          <illegal>
        98: ANL  BUS,#n         <illegal>
        D6: <illegal>           JNIBF <dest>
        E5: SEL  MB0            EN   DMA
        F5: SEL  MB1            EN   FLAGS

    Chip numbers are similar to the MCS-48 series:

    Chip   RAM  ROM  I/O
    ----   ---  ---  ---
    8041A   64   1k       (ROM)
    8041AH 128   1k       (ROM)
    8641A   64   1k       (OTPROM)
    8741A   64   1k       (EPROM)
    8741AH 128   1k       (EPROM)

    8042   128   2k       (ROM)
    8042AH 256   2k       (ROM)
    8642   128   2k       (OTPROM)
    8742   128   2k       (EPROM)
    8742AH 256   2k       (EPROM)

***************************************************************************/

#include "emu.h"
#include "mcs48.h"
#include "mcs48dsm.h"

#include "debugger.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* timer/counter enable bits */
#define TIMER_ENABLED   0x01
#define COUNTER_ENABLED 0x02

/* flag bits */
#define C_FLAG          0x80
#define A_FLAG          0x40
#define F_FLAG          0x20
#define B_FLAG          0x10

/* status bits (UPI-41) */
#define STS_F1          0x08
#define STS_F0          0x04
#define STS_IBF         0x02
#define STS_OBF         0x01

/* port 2 bits (UPI-41) */
#define P2_OBF          0x10
#define P2_NIBF         0x20
#define P2_DRQ          0x40
#define P2_NDACK        0x80

/* enable bits (UPI-41) */
#define ENABLE_FLAGS    0x01
#define ENABLE_DMA      0x02

/* feature masks */
#define MB_FEATURE      0x01
#define EXT_BUS_FEATURE 0x02
#define UPI41_FEATURE   0x04
#define I802X_FEATURE   0x08
#define I8048_FEATURE   (MB_FEATURE | EXT_BUS_FEATURE)


/***************************************************************************
    MACROS
***************************************************************************/

/* r0-r7 map to memory via the regptr */
#define R0              m_regptr[0]
#define R1              m_regptr[1]
#define R2              m_regptr[2]
#define R3              m_regptr[3]
#define R4              m_regptr[4]
#define R5              m_regptr[5]
#define R6              m_regptr[6]
#define R7              m_regptr[7]



DEFINE_DEVICE_TYPE(I8021,   i8021_device,   "i8021",   "Intel 8021")
DEFINE_DEVICE_TYPE(I8022,   i8022_device,   "i8022",   "Intel 8022")
DEFINE_DEVICE_TYPE(I8035,   i8035_device,   "i8035",   "Intel 8035")
DEFINE_DEVICE_TYPE(I8048,   i8048_device,   "i8048",   "Intel 8048")
DEFINE_DEVICE_TYPE(I8648,   i8648_device,   "i8648",   "Intel 8648")
DEFINE_DEVICE_TYPE(I8748,   i8748_device,   "i8748",   "Intel 8748")
DEFINE_DEVICE_TYPE(I8039,   i8039_device,   "i8039",   "Intel 8039")
DEFINE_DEVICE_TYPE(I8049,   i8049_device,   "i8049",   "Intel 8049")
DEFINE_DEVICE_TYPE(I8749,   i8749_device,   "i8749",   "Intel 8749")
DEFINE_DEVICE_TYPE(I8040,   i8040_device,   "i8040",   "Intel 8040")
DEFINE_DEVICE_TYPE(I8050,   i8050_device,   "i8050",   "Intel 8050")
DEFINE_DEVICE_TYPE(I8041A,  i8041a_device,  "i8041a",  "Intel 8041A")
DEFINE_DEVICE_TYPE(I8741A,  i8741a_device,  "i8741a",  "Intel 8741A")
DEFINE_DEVICE_TYPE(I8041AH, i8041ah_device, "i8041ah", "Intel 8041AH")
DEFINE_DEVICE_TYPE(I8741AH, i8741ah_device, "i8741ah", "Intel 8741AH")
DEFINE_DEVICE_TYPE(I8042,   i8042_device,   "i8042",   "Intel 8042")
DEFINE_DEVICE_TYPE(I8742,   i8742_device,   "i8742",   "Intel 8742")
DEFINE_DEVICE_TYPE(I8042AH, i8042ah_device, "i8042ah", "Intel 8042AH")
DEFINE_DEVICE_TYPE(I8742AH, i8742ah_device, "i8742ah", "Intel 8742AH")
DEFINE_DEVICE_TYPE(MB8884,  mb8884_device,  "mb8884",  "MB8884")
DEFINE_DEVICE_TYPE(N7751,   n7751_device,   "n7751",   "N7751")
DEFINE_DEVICE_TYPE(M58715,  m58715_device,  "m58715",  "M58715")


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

/* FIXME: the memory maps should probably support rom banking for EA */
void mcs48_cpu_device::program_10bit(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void mcs48_cpu_device::program_11bit(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void mcs48_cpu_device::program_12bit(address_map &map)
{
	map(0x000, 0xfff).rom();
}

void mcs48_cpu_device::data_6bit(address_map &map)
{
	map(0x00, 0x3f).ram().share("data");
}

void mcs48_cpu_device::data_7bit(address_map &map)
{
	map(0x00, 0x7f).ram().share("data");
}

void mcs48_cpu_device::data_8bit(address_map &map)
{
	map(0x00, 0xff).ram().share("data");
}


mcs48_cpu_device::mcs48_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int rom_size, int ram_size, uint8_t feature_mask, const mcs48_cpu_device::mcs48_ophandler *opcode_table)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, (feature_mask & MB_FEATURE) != 0 ? 12 : 11, 0
					   , (rom_size == 1024) ? address_map_constructor(FUNC(mcs48_cpu_device::program_10bit), this) : (rom_size == 2048) ? address_map_constructor(FUNC(mcs48_cpu_device::program_11bit), this) : (rom_size == 4096) ? address_map_constructor(FUNC(mcs48_cpu_device::program_12bit), this) : address_map_constructor())
	, m_data_config("data", ENDIANNESS_LITTLE, 8, ( ( ram_size == 64 ) ? 6 : ( ( ram_size == 128 ) ? 7 : 8 ) ), 0
					, (ram_size == 64) ? address_map_constructor(FUNC(mcs48_cpu_device::data_6bit), this) : (ram_size == 128) ? address_map_constructor(FUNC(mcs48_cpu_device::data_7bit), this) : address_map_constructor(FUNC(mcs48_cpu_device::data_8bit), this))
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
	, m_port_in_cb(*this)
	, m_port_out_cb(*this)
	, m_bus_in_cb(*this)
	, m_bus_out_cb(*this)
	, m_test_in_cb(*this)
	, m_t0_clk_func(*this)
	, m_prog_out_cb(*this)
	, m_psw(0)
	, m_dataptr(*this, "data")
	, m_feature_mask(feature_mask)
	, m_int_rom_size(rom_size)
	, m_opcode_table(opcode_table)
{
	// Sanity checks
	if ( ram_size != 64 && ram_size != 128 && ram_size != 256 )
	{
		fatalerror("mcs48: Invalid RAM size\n");
	}

	if ( rom_size != 0 && rom_size != 1024 && rom_size != 2048 && rom_size != 4096 )
	{
		fatalerror("mcs48: Invalid ROM size\n");
	}
}

i8021_device::i8021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8021, tag, owner, clock, 1024, 64, I802X_FEATURE, s_i8021_opcodes)
{
}

i8022_device::i8022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8022, tag, owner, clock, 2048, 128, I802X_FEATURE, s_i8022_opcodes)
{
}

i8035_device::i8035_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8035, tag, owner, clock, 0, 64, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8048_device::i8048_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8048, tag, owner, clock, 1024, 64, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8648_device::i8648_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8648, tag, owner, clock, 1024, 64, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8748_device::i8748_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8748, tag, owner, clock, 1024, 64, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8039_device::i8039_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8039, tag, owner, clock, 0, 128, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8049_device::i8049_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8049, tag, owner, clock, 2048, 128, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8749_device::i8749_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8749, tag, owner, clock, 2048, 128, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8040_device::i8040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8040, tag, owner, clock, 0, 256, I8048_FEATURE, s_mcs48_opcodes)
{
}

i8050_device::i8050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, I8050, tag, owner, clock, 4096, 256, I8048_FEATURE, s_mcs48_opcodes)
{
}

mb8884_device::mb8884_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, MB8884, tag, owner, clock, 0, 64, I8048_FEATURE, s_mcs48_opcodes)
{
}

n7751_device::n7751_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, N7751, tag, owner, clock, 1024, 64, I8048_FEATURE, s_mcs48_opcodes)
{
}

m58715_device::m58715_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs48_cpu_device(mconfig, M58715, tag, owner, clock, 2048, 128, I8048_FEATURE, s_mcs48_opcodes)
{
}

upi41_cpu_device::upi41_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int rom_size, int ram_size)
	: mcs48_cpu_device(mconfig, type, tag, owner, clock, rom_size, ram_size, UPI41_FEATURE, s_upi41_opcodes)
{
}

i8041a_device::i8041a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8041A, tag, owner, clock, 1024, 64)
{
}

i8741a_device::i8741a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8741A, tag, owner, clock, 1024, 64)
{
}

i8041ah_device::i8041ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8041AH, tag, owner, clock, 1024, 128)
{
}

i8741ah_device::i8741ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8741AH, tag, owner, clock, 1024, 128)
{
}

i8042_device::i8042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8042, tag, owner, clock, 2048, 128)
{
}

i8742_device::i8742_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8742, tag, owner, clock, 2048, 128)
{
}

i8042ah_device::i8042ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8042AH, tag, owner, clock, 2048, 256)
{
}

i8742ah_device::i8742ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upi41_cpu_device(mconfig, I8742AH, tag, owner, clock, 2048, 256)
{
}

device_memory_interface::space_config_vector mcs48_cpu_device::memory_space_config() const
{
	if ((m_feature_mask & EXT_BUS_FEATURE) != 0)
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_DATA,    &m_data_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_DATA,    &m_data_config)
		};
}

std::unique_ptr<util::disasm_interface> mcs48_cpu_device::create_disassembler()
{
	return std::make_unique<mcs48_disassembler>((m_feature_mask & UPI41_FEATURE) != 0, (m_feature_mask & I802X_FEATURE) != 0);
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    opcode_fetch - fetch an opcode byte
-------------------------------------------------*/

uint8_t mcs48_cpu_device::opcode_fetch()
{
	uint16_t address = m_pc;
	m_pc = ((m_pc + 1) & 0x7ff) | (m_pc & 0x800);
	return m_program.read_byte(address);
}


/*-------------------------------------------------
    argument_fetch - fetch an opcode argument
    byte
-------------------------------------------------*/

uint8_t mcs48_cpu_device::argument_fetch()
{
	uint16_t address = m_pc;
	m_pc = ((m_pc + 1) & 0x7ff) | (m_pc & 0x800);
	return m_program.read_byte(address);
}


/*-------------------------------------------------
    update_regptr - update the regptr member to
    point to the appropriate register bank
-------------------------------------------------*/

void mcs48_cpu_device::update_regptr()
{
	m_regptr = &m_dataptr[(m_psw & B_FLAG) ? 24 : 0];
}


/*-------------------------------------------------
    push_pc_psw - push the m_pc and m_psw values onto
    the stack
-------------------------------------------------*/

void mcs48_cpu_device::push_pc_psw()
{
	uint8_t sp = m_psw & 0x07;
	ram_w(8 + 2*sp, m_pc);
	ram_w(9 + 2*sp, ((m_pc >> 8) & 0x0f) | (m_psw & 0xf0));
	m_psw = (m_psw & 0xf0) | ((sp + 1) & 0x07);
}


/*-------------------------------------------------
    pull_pc_psw - pull the PC and PSW values from
    the stack
-------------------------------------------------*/

void mcs48_cpu_device::pull_pc_psw()
{
	uint8_t sp = (m_psw - 1) & 0x07;
	m_pc = ram_r(8 + 2*sp);
	m_pc |= ram_r(9 + 2*sp) << 8;
	m_psw = ((m_pc >> 8) & 0xf0) | sp;
	m_pc &= 0xfff;
	update_regptr();
}


/*-------------------------------------------------
    pull_pc - pull the PC value from the stack,
    leaving the upper part of PSW intact
-------------------------------------------------*/

void mcs48_cpu_device::pull_pc()
{
	uint8_t sp = (m_psw - 1) & 0x07;
	m_pc = ram_r(8 + 2*sp);
	m_pc |= ram_r(9 + 2*sp) << 8;
	m_pc &= 0xfff;
	m_psw = (m_psw & 0xf0) | sp;
}


/*-------------------------------------------------
    execute_add - perform the logic of an ADD
    instruction
-------------------------------------------------*/

void mcs48_cpu_device::execute_add(uint8_t dat)
{
	uint16_t temp = m_a + dat;
	uint16_t temp4 = (m_a & 0x0f) + (dat & 0x0f);

	m_psw &= ~(C_FLAG | A_FLAG);
	m_psw |= (temp4 << 2) & A_FLAG;
	m_psw |= (temp >> 1) & C_FLAG;
	m_a = temp;
}


/*-------------------------------------------------
    execute_addc - perform the logic of an ADDC
    instruction
-------------------------------------------------*/

void mcs48_cpu_device::execute_addc(uint8_t dat)
{
	uint8_t carryin = (m_psw & C_FLAG) >> 7;
	uint16_t temp = m_a + dat + carryin;
	uint16_t temp4 = (m_a & 0x0f) + (dat & 0x0f) + carryin;

	m_psw &= ~(C_FLAG | A_FLAG);
	m_psw |= (temp4 << 2) & A_FLAG;
	m_psw |= (temp >> 1) & C_FLAG;
	m_a = temp;
}


/*-------------------------------------------------
    execute_jmp - perform the logic of a JMP
    instruction
-------------------------------------------------*/

void mcs48_cpu_device::execute_jmp(uint16_t address)
{
	uint16_t a11 = (m_irq_in_progress) ? 0 : m_a11;
	m_pc = address | a11;
}


/*-------------------------------------------------
    execute_call - perform the logic of a CALL
    instruction
-------------------------------------------------*/

void mcs48_cpu_device::execute_call(uint16_t address)
{
	push_pc_psw();
	execute_jmp(address);
}


/*-------------------------------------------------
    execute_jcc - perform the logic of a
    conditional jump instruction
-------------------------------------------------*/

void mcs48_cpu_device::execute_jcc(bool result)
{
	uint16_t pch = m_pc & 0xf00;
	uint8_t offset = argument_fetch();
	if (result != 0)
		m_pc = pch | offset;
}


/*-------------------------------------------------
    p2_mask - return the mask of bits that the
    code can directly affect
-------------------------------------------------*/

uint8_t mcs48_cpu_device::p2_mask()
{
	uint8_t result = 0xff;
	if ((m_feature_mask & UPI41_FEATURE) == 0)
		return result;
	if (m_flags_enabled)
		result &= ~(P2_OBF | P2_NIBF);
	if (m_dma_enabled)
		result &= ~(P2_DRQ | P2_NDACK);
	return result;
}


/*-------------------------------------------------
    expander_operation - perform an operation via
    the 8243 expander chip
-------------------------------------------------*/

void mcs48_cpu_device::expander_operation(expander_op operation, uint8_t port)
{
	// put opcode on low 4 bits of P2 (overwriting latch)
	port_w(2, m_p2 = (m_p2 & 0xf0) | (uint8_t(operation) << 2) | (port & 3));

	// generate high-to-low transition on PROG line
	prog_w(0);

	// transfer data on low 4 bits of P2
	if (operation != EXPANDER_OP_READ)
		port_w(2, m_p2 = (m_p2 & 0xf0) | (m_a & 0x0f));
	else
	{
		// place P20-P23 in input mode
		port_w(2, m_p2 |= 0x0f);

		// input data to lower 4 bits of A (upper 4 bits are cleared)
		m_a = port_r(2) & 0x0f;
	}

	// generate low-to-high transition on PROG line
	prog_w(1);
}



/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define OPHANDLER(_name) void mcs48_cpu_device::_name()


OPHANDLER( illegal )
{
	burn_cycles(1);
	logerror("MCS-48 PC:%04X - Illegal opcode = %02x\n", m_prevpc, program_r(m_prevpc));
}

OPHANDLER( add_a_r0 )       { burn_cycles(1); execute_add(R0); }
OPHANDLER( add_a_r1 )       { burn_cycles(1); execute_add(R1); }
OPHANDLER( add_a_r2 )       { burn_cycles(1); execute_add(R2); }
OPHANDLER( add_a_r3 )       { burn_cycles(1); execute_add(R3); }
OPHANDLER( add_a_r4 )       { burn_cycles(1); execute_add(R4); }
OPHANDLER( add_a_r5 )       { burn_cycles(1); execute_add(R5); }
OPHANDLER( add_a_r6 )       { burn_cycles(1); execute_add(R6); }
OPHANDLER( add_a_r7 )       { burn_cycles(1); execute_add(R7); }
OPHANDLER( add_a_xr0 )      { burn_cycles(1); execute_add(ram_r(R0)); }
OPHANDLER( add_a_xr1 )      { burn_cycles(1); execute_add(ram_r(R1)); }
OPHANDLER( add_a_n )        { burn_cycles(2); execute_add(argument_fetch()); }

OPHANDLER( adc_a_r0 )       { burn_cycles(1); execute_addc(R0); }
OPHANDLER( adc_a_r1 )       { burn_cycles(1); execute_addc(R1); }
OPHANDLER( adc_a_r2 )       { burn_cycles(1); execute_addc(R2); }
OPHANDLER( adc_a_r3 )       { burn_cycles(1); execute_addc(R3); }
OPHANDLER( adc_a_r4 )       { burn_cycles(1); execute_addc(R4); }
OPHANDLER( adc_a_r5 )       { burn_cycles(1); execute_addc(R5); }
OPHANDLER( adc_a_r6 )       { burn_cycles(1); execute_addc(R6); }
OPHANDLER( adc_a_r7 )       { burn_cycles(1); execute_addc(R7); }
OPHANDLER( adc_a_xr0 )      { burn_cycles(1); execute_addc(ram_r(R0)); }
OPHANDLER( adc_a_xr1 )      { burn_cycles(1); execute_addc(ram_r(R1)); }
OPHANDLER( adc_a_n )        { burn_cycles(2); execute_addc(argument_fetch()); }

OPHANDLER( anl_a_r0 )       { burn_cycles(1); m_a &= R0; }
OPHANDLER( anl_a_r1 )       { burn_cycles(1); m_a &= R1; }
OPHANDLER( anl_a_r2 )       { burn_cycles(1); m_a &= R2; }
OPHANDLER( anl_a_r3 )       { burn_cycles(1); m_a &= R3; }
OPHANDLER( anl_a_r4 )       { burn_cycles(1); m_a &= R4; }
OPHANDLER( anl_a_r5 )       { burn_cycles(1); m_a &= R5; }
OPHANDLER( anl_a_r6 )       { burn_cycles(1); m_a &= R6; }
OPHANDLER( anl_a_r7 )       { burn_cycles(1); m_a &= R7; }
OPHANDLER( anl_a_xr0 )      { burn_cycles(1); m_a &= ram_r(R0); }
OPHANDLER( anl_a_xr1 )      { burn_cycles(1); m_a &= ram_r(R1); }
OPHANDLER( anl_a_n )        { burn_cycles(2); m_a &= argument_fetch(); }

OPHANDLER( anl_bus_n )      { burn_cycles(2); bus_w(bus_r() & argument_fetch()); }
OPHANDLER( anl_p1_n )       { burn_cycles(2); port_w(1, m_p1 &= argument_fetch()); }
OPHANDLER( anl_p2_n )       { burn_cycles(2); port_w(2, m_p2 &= argument_fetch() | ~p2_mask()); }
OPHANDLER( anld_p4_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 4); }
OPHANDLER( anld_p5_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 5); }
OPHANDLER( anld_p6_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 6); }
OPHANDLER( anld_p7_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 7); }

OPHANDLER( call_0 )         { burn_cycles(2); execute_call(argument_fetch() | 0x000); }
OPHANDLER( call_1 )         { burn_cycles(2); execute_call(argument_fetch() | 0x100); }
OPHANDLER( call_2 )         { burn_cycles(2); execute_call(argument_fetch() | 0x200); }
OPHANDLER( call_3 )         { burn_cycles(2); execute_call(argument_fetch() | 0x300); }
OPHANDLER( call_4 )         { burn_cycles(2); execute_call(argument_fetch() | 0x400); }
OPHANDLER( call_5 )         { burn_cycles(2); execute_call(argument_fetch() | 0x500); }
OPHANDLER( call_6 )         { burn_cycles(2); execute_call(argument_fetch() | 0x600); }
OPHANDLER( call_7 )         { burn_cycles(2); execute_call(argument_fetch() | 0x700); }

OPHANDLER( clr_a )          { burn_cycles(1); m_a = 0; }
OPHANDLER( clr_c )          { burn_cycles(1); m_psw &= ~C_FLAG; }
OPHANDLER( clr_f0 )         { burn_cycles(1); m_psw &= ~F_FLAG; m_sts &= ~STS_F0; }
OPHANDLER( clr_f1 )         { burn_cycles(1); m_sts &= ~STS_F1; }

OPHANDLER( cpl_a )          { burn_cycles(1); m_a ^= 0xff; }
OPHANDLER( cpl_c )          { burn_cycles(1); m_psw ^= C_FLAG; }
OPHANDLER( cpl_f0 )         { burn_cycles(1); m_psw ^= F_FLAG; m_sts ^= STS_F0; }
OPHANDLER( cpl_f1 )         { burn_cycles(1); m_sts ^= STS_F1; }

OPHANDLER( da_a )
{
	burn_cycles(1);

	if ((m_a & 0x0f) > 0x09 || (m_psw & A_FLAG))
	{
		m_a += 0x06;
		if ((m_a & 0xf0) == 0x00)
			m_psw |= C_FLAG;
	}
	if ((m_a & 0xf0) > 0x90 || (m_psw & C_FLAG))
	{
		m_a += 0x60;
		m_psw |= C_FLAG;
	}
}

OPHANDLER( dec_a )          { burn_cycles(1); m_a--; }
OPHANDLER( dec_r0 )         { burn_cycles(1); R0--; }
OPHANDLER( dec_r1 )         { burn_cycles(1); R1--; }
OPHANDLER( dec_r2 )         { burn_cycles(1); R2--; }
OPHANDLER( dec_r3 )         { burn_cycles(1); R3--; }
OPHANDLER( dec_r4 )         { burn_cycles(1); R4--; }
OPHANDLER( dec_r5 )         { burn_cycles(1); R5--; }
OPHANDLER( dec_r6 )         { burn_cycles(1); R6--; }
OPHANDLER( dec_r7 )         { burn_cycles(1); R7--; }

OPHANDLER( dis_i )          { burn_cycles(1); m_xirq_enabled = false; }
OPHANDLER( dis_tcnti )      { burn_cycles(1); m_tirq_enabled = false; m_timer_overflow = false; }

OPHANDLER( djnz_r0 )        { burn_cycles(2); execute_jcc(--R0 != 0); }
OPHANDLER( djnz_r1 )        { burn_cycles(2); execute_jcc(--R1 != 0); }
OPHANDLER( djnz_r2 )        { burn_cycles(2); execute_jcc(--R2 != 0); }
OPHANDLER( djnz_r3 )        { burn_cycles(2); execute_jcc(--R3 != 0); }
OPHANDLER( djnz_r4 )        { burn_cycles(2); execute_jcc(--R4 != 0); }
OPHANDLER( djnz_r5 )        { burn_cycles(2); execute_jcc(--R5 != 0); }
OPHANDLER( djnz_r6 )        { burn_cycles(2); execute_jcc(--R6 != 0); }
OPHANDLER( djnz_r7 )        { burn_cycles(2); execute_jcc(--R7 != 0); }

OPHANDLER( en_i )           { burn_cycles(1); m_xirq_enabled = true; }
OPHANDLER( en_tcnti )       { burn_cycles(1); m_tirq_enabled = true; }
OPHANDLER( en_dma )         { burn_cycles(1); m_dma_enabled = true; port_w(2, m_p2); }
OPHANDLER( en_flags )       { burn_cycles(1); m_flags_enabled = true; port_w(2, m_p2); }
OPHANDLER( ent0_clk )
{
	burn_cycles(1);

	if (!m_t0_clk_func.isnull())
		m_t0_clk_func(clock() / 3);
	else
		logerror("T0 clock enabled\n");
}

OPHANDLER( in_a_p0 )        { burn_cycles(2); m_a = bus_r() & m_dbbo; }
OPHANDLER( in_a_p1 )        { burn_cycles(2); m_a = port_r(1) & m_p1; }
OPHANDLER( in_a_p2 )        { burn_cycles(2); m_a = port_r(2) & m_p2; }
OPHANDLER( ins_a_bus )      { burn_cycles(2); m_a = bus_r(); }
OPHANDLER( in_a_dbb )
{
	burn_cycles(2);

	/* acknowledge the IBF IRQ and clear the bit in STS */
	if ((m_sts & STS_IBF) != 0)
		standard_irq_callback(UPI41_INPUT_IBF);
	m_sts &= ~STS_IBF;

	/* if P2 flags are enabled, update the state of P2 */
	if (m_flags_enabled && (m_p2 & P2_NIBF) == 0)
		port_w(2, m_p2 |= P2_NIBF);
	m_a = m_dbbi;
}

OPHANDLER( inc_a )          { burn_cycles(1); m_a++; }
OPHANDLER( inc_r0 )         { burn_cycles(1); R0++; }
OPHANDLER( inc_r1 )         { burn_cycles(1); R1++; }
OPHANDLER( inc_r2 )         { burn_cycles(1); R2++; }
OPHANDLER( inc_r3 )         { burn_cycles(1); R3++; }
OPHANDLER( inc_r4 )         { burn_cycles(1); R4++; }
OPHANDLER( inc_r5 )         { burn_cycles(1); R5++; }
OPHANDLER( inc_r6 )         { burn_cycles(1); R6++; }
OPHANDLER( inc_r7 )         { burn_cycles(1); R7++; }
OPHANDLER( inc_xr0 )        { burn_cycles(1); ram_w(R0, ram_r(R0) + 1); }
OPHANDLER( inc_xr1 )        { burn_cycles(1); ram_w(R1, ram_r(R1) + 1); }

OPHANDLER( jb_0 )           { burn_cycles(2); execute_jcc((m_a & 0x01) != 0); }
OPHANDLER( jb_1 )           { burn_cycles(2); execute_jcc((m_a & 0x02) != 0); }
OPHANDLER( jb_2 )           { burn_cycles(2); execute_jcc((m_a & 0x04) != 0); }
OPHANDLER( jb_3 )           { burn_cycles(2); execute_jcc((m_a & 0x08) != 0); }
OPHANDLER( jb_4 )           { burn_cycles(2); execute_jcc((m_a & 0x10) != 0); }
OPHANDLER( jb_5 )           { burn_cycles(2); execute_jcc((m_a & 0x20) != 0); }
OPHANDLER( jb_6 )           { burn_cycles(2); execute_jcc((m_a & 0x40) != 0); }
OPHANDLER( jb_7 )           { burn_cycles(2); execute_jcc((m_a & 0x80) != 0); }
OPHANDLER( jc )             { burn_cycles(2); execute_jcc((m_psw & C_FLAG) != 0); }
OPHANDLER( jf0 )            { burn_cycles(2); execute_jcc((m_psw & F_FLAG) != 0); }
OPHANDLER( jf1 )            { burn_cycles(2); execute_jcc((m_sts & STS_F1) != 0); }
OPHANDLER( jnc )            { burn_cycles(2); execute_jcc((m_psw & C_FLAG) == 0); }
OPHANDLER( jni )            { burn_cycles(2); m_irq_polled = (m_irq_state == 0); execute_jcc(m_irq_state != 0); }
OPHANDLER( jnibf )          { burn_cycles(2); m_irq_polled = (m_sts & STS_IBF) != 0; execute_jcc((m_sts & STS_IBF) == 0); }
OPHANDLER( jnt_0 )          { burn_cycles(2); execute_jcc(test_r(0) == 0); }
OPHANDLER( jnt_1 )          { burn_cycles(2); execute_jcc(test_r(1) == 0); }
OPHANDLER( jnz )            { burn_cycles(2); execute_jcc(m_a != 0); }
OPHANDLER( jobf )           { burn_cycles(2); execute_jcc((m_sts & STS_OBF) != 0); }
OPHANDLER( jtf )            { burn_cycles(2); execute_jcc(m_timer_flag); m_timer_flag = false; }
OPHANDLER( jt_0 )           { burn_cycles(2); execute_jcc(test_r(0) != 0); }
OPHANDLER( jt_1 )           { burn_cycles(2); execute_jcc(test_r(1) != 0); }
OPHANDLER( jz )             { burn_cycles(2); execute_jcc(m_a == 0); }

OPHANDLER( jmp_0 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x000); }
OPHANDLER( jmp_1 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x100); }
OPHANDLER( jmp_2 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x200); }
OPHANDLER( jmp_3 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x300); }
OPHANDLER( jmp_4 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x400); }
OPHANDLER( jmp_5 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x500); }
OPHANDLER( jmp_6 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x600); }
OPHANDLER( jmp_7 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x700); }
OPHANDLER( jmpp_xa )        { burn_cycles(2); m_pc &= 0xf00; m_pc |= program_r(m_pc | m_a); }

OPHANDLER( mov_a_n )        { burn_cycles(2); m_a = argument_fetch(); }
OPHANDLER( mov_a_psw )      { burn_cycles(1); m_a = m_psw | 0x08; }
OPHANDLER( mov_a_r0 )       { burn_cycles(1); m_a = R0; }
OPHANDLER( mov_a_r1 )       { burn_cycles(1); m_a = R1; }
OPHANDLER( mov_a_r2 )       { burn_cycles(1); m_a = R2; }
OPHANDLER( mov_a_r3 )       { burn_cycles(1); m_a = R3; }
OPHANDLER( mov_a_r4 )       { burn_cycles(1); m_a = R4; }
OPHANDLER( mov_a_r5 )       { burn_cycles(1); m_a = R5; }
OPHANDLER( mov_a_r6 )       { burn_cycles(1); m_a = R6; }
OPHANDLER( mov_a_r7 )       { burn_cycles(1); m_a = R7; }
OPHANDLER( mov_a_xr0 )      { burn_cycles(1); m_a = ram_r(R0); }
OPHANDLER( mov_a_xr1 )      { burn_cycles(1); m_a = ram_r(R1); }
OPHANDLER( mov_a_t )        { burn_cycles(1); m_a = m_timer; }

OPHANDLER( mov_psw_a )      { burn_cycles(1); m_psw = m_a & ~0x08; update_regptr(); }
OPHANDLER( mov_sts_a )      { burn_cycles(1); m_sts = (m_sts & 0x0f) | (m_a & 0xf0); }
OPHANDLER( mov_r0_a )       { burn_cycles(1); R0 = m_a; }
OPHANDLER( mov_r1_a )       { burn_cycles(1); R1 = m_a; }
OPHANDLER( mov_r2_a )       { burn_cycles(1); R2 = m_a; }
OPHANDLER( mov_r3_a )       { burn_cycles(1); R3 = m_a; }
OPHANDLER( mov_r4_a )       { burn_cycles(1); R4 = m_a; }
OPHANDLER( mov_r5_a )       { burn_cycles(1); R5 = m_a; }
OPHANDLER( mov_r6_a )       { burn_cycles(1); R6 = m_a; }
OPHANDLER( mov_r7_a )       { burn_cycles(1); R7 = m_a; }
OPHANDLER( mov_r0_n )       { burn_cycles(2); R0 = argument_fetch(); }
OPHANDLER( mov_r1_n )       { burn_cycles(2); R1 = argument_fetch(); }
OPHANDLER( mov_r2_n )       { burn_cycles(2); R2 = argument_fetch(); }
OPHANDLER( mov_r3_n )       { burn_cycles(2); R3 = argument_fetch(); }
OPHANDLER( mov_r4_n )       { burn_cycles(2); R4 = argument_fetch(); }
OPHANDLER( mov_r5_n )       { burn_cycles(2); R5 = argument_fetch(); }
OPHANDLER( mov_r6_n )       { burn_cycles(2); R6 = argument_fetch(); }
OPHANDLER( mov_r7_n )       { burn_cycles(2); R7 = argument_fetch(); }
OPHANDLER( mov_t_a )        { burn_cycles(1); m_timer = m_a; }
OPHANDLER( mov_xr0_a )      { burn_cycles(1); ram_w(R0, m_a); }
OPHANDLER( mov_xr1_a )      { burn_cycles(1); ram_w(R1, m_a); }
OPHANDLER( mov_xr0_n )      { burn_cycles(2); ram_w(R0, argument_fetch()); }
OPHANDLER( mov_xr1_n )      { burn_cycles(2); ram_w(R1, argument_fetch()); }

OPHANDLER( movd_a_p4 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 4); }
OPHANDLER( movd_a_p5 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 5); }
OPHANDLER( movd_a_p6 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 6); }
OPHANDLER( movd_a_p7 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 7); }
OPHANDLER( movd_p4_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 4); }
OPHANDLER( movd_p5_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 5); }
OPHANDLER( movd_p6_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 6); }
OPHANDLER( movd_p7_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 7); }

OPHANDLER( movp_a_xa )      { burn_cycles(2); m_a = program_r((m_pc & 0xf00) | m_a); }
OPHANDLER( movp3_a_xa )     { burn_cycles(2); m_a = program_r(0x300 | m_a); }

OPHANDLER( movx_a_xr0 )     { burn_cycles(2); m_a = ext_r(R0); }
OPHANDLER( movx_a_xr1 )     { burn_cycles(2); m_a = ext_r(R1); }
OPHANDLER( movx_xr0_a )     { burn_cycles(2); ext_w(R0, m_a); }
OPHANDLER( movx_xr1_a )     { burn_cycles(2); ext_w(R1, m_a); }

OPHANDLER( nop )            { burn_cycles(1); }

OPHANDLER( orl_a_r0 )       { burn_cycles(1); m_a |= R0; }
OPHANDLER( orl_a_r1 )       { burn_cycles(1); m_a |= R1; }
OPHANDLER( orl_a_r2 )       { burn_cycles(1); m_a |= R2; }
OPHANDLER( orl_a_r3 )       { burn_cycles(1); m_a |= R3; }
OPHANDLER( orl_a_r4 )       { burn_cycles(1); m_a |= R4; }
OPHANDLER( orl_a_r5 )       { burn_cycles(1); m_a |= R5; }
OPHANDLER( orl_a_r6 )       { burn_cycles(1); m_a |= R6; }
OPHANDLER( orl_a_r7 )       { burn_cycles(1); m_a |= R7; }
OPHANDLER( orl_a_xr0 )      { burn_cycles(1); m_a |= ram_r(R0); }
OPHANDLER( orl_a_xr1 )      { burn_cycles(1); m_a |= ram_r(R1); }
OPHANDLER( orl_a_n )        { burn_cycles(2); m_a |= argument_fetch(); }

OPHANDLER( orl_bus_n )      { burn_cycles(2); bus_w(bus_r() | argument_fetch()); }
OPHANDLER( orl_p1_n )       { burn_cycles(2); port_w(1, m_p1 |= argument_fetch()); }
OPHANDLER( orl_p2_n )       { burn_cycles(2); port_w(2, m_p2 |= argument_fetch() & p2_mask()); }
OPHANDLER( orld_p4_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 4); }
OPHANDLER( orld_p5_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 5); }
OPHANDLER( orld_p6_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 6); }
OPHANDLER( orld_p7_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 7); }

OPHANDLER( outl_bus_a )     { burn_cycles(2); bus_w(m_a); }
OPHANDLER( outl_p0_a )      { burn_cycles(2); bus_w(m_dbbo = m_a); }
OPHANDLER( outl_p1_a )      { burn_cycles(2); port_w(1, m_p1 = m_a); }
OPHANDLER( outl_p2_a )      { burn_cycles(2); uint8_t mask = p2_mask(); port_w(2, m_p2 = (m_p2 & ~mask) | (m_a & mask)); }
OPHANDLER( out_dbb_a )
{
	burn_cycles(2);

	/* copy to the DBBO and update the bit in STS */
	m_dbbo = m_a;
	m_sts |= STS_OBF;

	/* if P2 flags are enabled, update the state of P2 */
	if (m_flags_enabled && (m_p2 & P2_OBF) == 0)
		port_w(2, m_p2 |= P2_OBF);
}


OPHANDLER( ret )            { burn_cycles(2); pull_pc(); }
OPHANDLER( retr )
{
	burn_cycles(2);
	pull_pc_psw();

	/* implicitly clear the IRQ in progress flip flop */
	m_irq_in_progress = false;
}

OPHANDLER( rl_a )           { burn_cycles(1); m_a = (m_a << 1) | (m_a >> 7); }
OPHANDLER( rlc_a )          { burn_cycles(1); uint8_t newc = m_a & C_FLAG; m_a = (m_a << 1) | (m_psw >> 7); m_psw = (m_psw & ~C_FLAG) | newc; }

OPHANDLER( rr_a )           { burn_cycles(1); m_a = (m_a >> 1) | (m_a << 7); }
OPHANDLER( rrc_a )          { burn_cycles(1); uint8_t newc = (m_a << 7) & C_FLAG; m_a = (m_a >> 1) | (m_psw & C_FLAG); m_psw = (m_psw & ~C_FLAG) | newc; }

OPHANDLER( sel_mb0 )        { burn_cycles(1); m_a11 = 0x000; }
OPHANDLER( sel_mb1 )        { burn_cycles(1); m_a11 = 0x800; }

OPHANDLER( sel_rb0 )        { burn_cycles(1); m_psw &= ~B_FLAG; update_regptr(); }
OPHANDLER( sel_rb1 )        { burn_cycles(1); m_psw |=  B_FLAG; update_regptr(); }

OPHANDLER( stop_tcnt )      { burn_cycles(1); m_timecount_enabled = 0; }
OPHANDLER( strt_t )         { burn_cycles(1); m_timecount_enabled = TIMER_ENABLED; m_prescaler = 0; }
OPHANDLER( strt_cnt )
{
	burn_cycles(1);
	if (!(m_timecount_enabled & COUNTER_ENABLED))
		m_t1_history = test_r(1);

	m_timecount_enabled = COUNTER_ENABLED;
}

OPHANDLER( swap_a )         { burn_cycles(1); m_a = (m_a << 4) | (m_a >> 4); }

OPHANDLER( xch_a_r0 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R0; R0 = tmp; }
OPHANDLER( xch_a_r1 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R1; R1 = tmp; }
OPHANDLER( xch_a_r2 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R2; R2 = tmp; }
OPHANDLER( xch_a_r3 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R3; R3 = tmp; }
OPHANDLER( xch_a_r4 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R4; R4 = tmp; }
OPHANDLER( xch_a_r5 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R5; R5 = tmp; }
OPHANDLER( xch_a_r6 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R6; R6 = tmp; }
OPHANDLER( xch_a_r7 )       { burn_cycles(1); uint8_t tmp = m_a; m_a = R7; R7 = tmp; }
OPHANDLER( xch_a_xr0 )      { burn_cycles(1); uint8_t tmp = m_a; m_a = ram_r(R0); ram_w(R0, tmp); }
OPHANDLER( xch_a_xr1 )      { burn_cycles(1); uint8_t tmp = m_a; m_a = ram_r(R1); ram_w(R1, tmp); }

OPHANDLER( xchd_a_xr0 )     { burn_cycles(1); uint8_t oldram = ram_r(R0); ram_w(R0, (oldram & 0xf0) | (m_a & 0x0f)); m_a = (m_a & 0xf0) | (oldram & 0x0f); }
OPHANDLER( xchd_a_xr1 )     { burn_cycles(1); uint8_t oldram = ram_r(R1); ram_w(R1, (oldram & 0xf0) | (m_a & 0x0f)); m_a = (m_a & 0xf0) | (oldram & 0x0f); }

OPHANDLER( xrl_a_r0 )       { burn_cycles(1); m_a ^= R0; }
OPHANDLER( xrl_a_r1 )       { burn_cycles(1); m_a ^= R1; }
OPHANDLER( xrl_a_r2 )       { burn_cycles(1); m_a ^= R2; }
OPHANDLER( xrl_a_r3 )       { burn_cycles(1); m_a ^= R3; }
OPHANDLER( xrl_a_r4 )       { burn_cycles(1); m_a ^= R4; }
OPHANDLER( xrl_a_r5 )       { burn_cycles(1); m_a ^= R5; }
OPHANDLER( xrl_a_r6 )       { burn_cycles(1); m_a ^= R6; }
OPHANDLER( xrl_a_r7 )       { burn_cycles(1); m_a ^= R7; }
OPHANDLER( xrl_a_xr0 )      { burn_cycles(1); m_a ^= ram_r(R0); }
OPHANDLER( xrl_a_xr1 )      { burn_cycles(1); m_a ^= ram_r(R1); }
OPHANDLER( xrl_a_n )        { burn_cycles(2); m_a ^= argument_fetch(); }



/***************************************************************************
    OPCODE TABLES
***************************************************************************/

#define OP(_a) &mcs48_cpu_device::_a

const mcs48_cpu_device::mcs48_ophandler mcs48_cpu_device::s_mcs48_opcodes[256] =
{
	OP(nop),        OP(illegal),    OP(outl_bus_a),OP(add_a_n),   OP(jmp_0),     OP(en_i),       OP(illegal),   OP(dec_a),         /* 00 */
	OP(ins_a_bus),  OP(in_a_p1),    OP(in_a_p2),   OP(illegal),   OP(movd_a_p4), OP(movd_a_p5),  OP(movd_a_p6), OP(movd_a_p7),
	OP(inc_xr0),    OP(inc_xr1),    OP(jb_0),      OP(adc_a_n),   OP(call_0),    OP(dis_i),      OP(jtf),       OP(inc_a),         /* 10 */
	OP(inc_r0),     OP(inc_r1),     OP(inc_r2),    OP(inc_r3),    OP(inc_r4),    OP(inc_r5),     OP(inc_r6),    OP(inc_r7),
	OP(xch_a_xr0),  OP(xch_a_xr1),  OP(illegal),   OP(mov_a_n),   OP(jmp_1),     OP(en_tcnti),   OP(jnt_0),     OP(clr_a),         /* 20 */
	OP(xch_a_r0),   OP(xch_a_r1),   OP(xch_a_r2),  OP(xch_a_r3),  OP(xch_a_r4),  OP(xch_a_r5),   OP(xch_a_r6),  OP(xch_a_r7),
	OP(xchd_a_xr0), OP(xchd_a_xr1), OP(jb_1),      OP(illegal),   OP(call_1),    OP(dis_tcnti),  OP(jt_0),      OP(cpl_a),         /* 30 */
	OP(illegal),    OP(outl_p1_a),  OP(outl_p2_a), OP(illegal),   OP(movd_p4_a), OP(movd_p5_a),  OP(movd_p6_a), OP(movd_p7_a),
	OP(orl_a_xr0),  OP(orl_a_xr1),  OP(mov_a_t),   OP(orl_a_n),   OP(jmp_2),     OP(strt_cnt),   OP(jnt_1),     OP(swap_a),        /* 40 */
	OP(orl_a_r0),   OP(orl_a_r1),   OP(orl_a_r2),  OP(orl_a_r3),  OP(orl_a_r4),  OP(orl_a_r5),   OP(orl_a_r6),  OP(orl_a_r7),
	OP(anl_a_xr0),  OP(anl_a_xr1),  OP(jb_2),      OP(anl_a_n),   OP(call_2),    OP(strt_t),     OP(jt_1),      OP(da_a),          /* 50 */
	OP(anl_a_r0),   OP(anl_a_r1),   OP(anl_a_r2),  OP(anl_a_r3),  OP(anl_a_r4),  OP(anl_a_r5),   OP(anl_a_r6),  OP(anl_a_r7),
	OP(add_a_xr0),  OP(add_a_xr1),  OP(mov_t_a),   OP(illegal),   OP(jmp_3),     OP(stop_tcnt),  OP(illegal),   OP(rrc_a),         /* 60 */
	OP(add_a_r0),   OP(add_a_r1),   OP(add_a_r2),  OP(add_a_r3),  OP(add_a_r4),  OP(add_a_r5),   OP(add_a_r6),  OP(add_a_r7),
	OP(adc_a_xr0),  OP(adc_a_xr1),  OP(jb_3),      OP(illegal),   OP(call_3),    OP(ent0_clk),   OP(jf1),       OP(rr_a),          /* 70 */
	OP(adc_a_r0),   OP(adc_a_r1),   OP(adc_a_r2),  OP(adc_a_r3),  OP(adc_a_r4),  OP(adc_a_r5),   OP(adc_a_r6),  OP(adc_a_r7),
	OP(movx_a_xr0), OP(movx_a_xr1), OP(illegal),   OP(ret),       OP(jmp_4),     OP(clr_f0),     OP(jni),       OP(illegal),       /* 80 */
	OP(orl_bus_n),  OP(orl_p1_n),   OP(orl_p2_n),  OP(illegal),   OP(orld_p4_a), OP(orld_p5_a),  OP(orld_p6_a), OP(orld_p7_a),
	OP(movx_xr0_a), OP(movx_xr1_a), OP(jb_4),      OP(retr),      OP(call_4),    OP(cpl_f0),     OP(jnz),       OP(clr_c),         /* 90 */
	OP(anl_bus_n),  OP(anl_p1_n),   OP(anl_p2_n),  OP(illegal),   OP(anld_p4_a), OP(anld_p5_a),  OP(anld_p6_a), OP(anld_p7_a),
	OP(mov_xr0_a),  OP(mov_xr1_a),  OP(illegal),   OP(movp_a_xa), OP(jmp_5),     OP(clr_f1),     OP(illegal),   OP(cpl_c),         /* A0 */
	OP(mov_r0_a),   OP(mov_r1_a),   OP(mov_r2_a),  OP(mov_r3_a),  OP(mov_r4_a),  OP(mov_r5_a),   OP(mov_r6_a),  OP(mov_r7_a),
	OP(mov_xr0_n),  OP(mov_xr1_n),  OP(jb_5),      OP(jmpp_xa),   OP(call_5),    OP(cpl_f1),     OP(jf0),       OP(illegal),       /* B0 */
	OP(mov_r0_n),   OP(mov_r1_n),   OP(mov_r2_n),  OP(mov_r3_n),  OP(mov_r4_n),  OP(mov_r5_n),   OP(mov_r6_n),  OP(mov_r7_n),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(jmp_6),     OP(sel_rb0),    OP(jz),        OP(mov_a_psw),     /* C0 */
	OP(dec_r0),     OP(dec_r1),     OP(dec_r2),    OP(dec_r3),    OP(dec_r4),    OP(dec_r5),     OP(dec_r6),    OP(dec_r7),
	OP(xrl_a_xr0),  OP(xrl_a_xr1),  OP(jb_6),      OP(xrl_a_n),   OP(call_6),    OP(sel_rb1),    OP(illegal),   OP(mov_psw_a),     /* D0 */
	OP(xrl_a_r0),   OP(xrl_a_r1),   OP(xrl_a_r2),  OP(xrl_a_r3),  OP(xrl_a_r4),  OP(xrl_a_r5),   OP(xrl_a_r6),  OP(xrl_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(movp3_a_xa),OP(jmp_7),     OP(sel_mb0),    OP(jnc),       OP(rl_a),          /* E0 */
	OP(djnz_r0),    OP(djnz_r1),    OP(djnz_r2),   OP(djnz_r3),   OP(djnz_r4),   OP(djnz_r5),    OP(djnz_r6),   OP(djnz_r7),
	OP(mov_a_xr0),  OP(mov_a_xr1),  OP(jb_7),      OP(illegal),   OP(call_7),    OP(sel_mb1),    OP(jc),        OP(rlc_a),         /* F0 */
	OP(mov_a_r0),   OP(mov_a_r1),   OP(mov_a_r2),  OP(mov_a_r3),  OP(mov_a_r4),  OP(mov_a_r5),   OP(mov_a_r6),  OP(mov_a_r7)
};

const mcs48_cpu_device::mcs48_ophandler mcs48_cpu_device::s_upi41_opcodes[256] =
{
	OP(nop),        OP(illegal),    OP(out_dbb_a), OP(add_a_n),   OP(jmp_0),     OP(en_i),       OP(illegal),   OP(dec_a),         /* 00 */
	OP(illegal),    OP(in_a_p1),    OP(in_a_p2),   OP(illegal),   OP(movd_a_p4), OP(movd_a_p5),  OP(movd_a_p6), OP(movd_a_p7),
	OP(inc_xr0),    OP(inc_xr1),    OP(jb_0),      OP(adc_a_n),   OP(call_0),    OP(dis_i),      OP(jtf),       OP(inc_a),         /* 10 */
	OP(inc_r0),     OP(inc_r1),     OP(inc_r2),    OP(inc_r3),    OP(inc_r4),    OP(inc_r5),     OP(inc_r6),    OP(inc_r7),
	OP(xch_a_xr0),  OP(xch_a_xr1),  OP(in_a_dbb),  OP(mov_a_n),   OP(jmp_1),     OP(en_tcnti),   OP(jnt_0),     OP(clr_a),         /* 20 */
	OP(xch_a_r0),   OP(xch_a_r1),   OP(xch_a_r2),  OP(xch_a_r3),  OP(xch_a_r4),  OP(xch_a_r5),   OP(xch_a_r6),  OP(xch_a_r7),
	OP(xchd_a_xr0), OP(xchd_a_xr1), OP(jb_1),      OP(illegal),   OP(call_1),    OP(dis_tcnti),  OP(jt_0),      OP(cpl_a),         /* 30 */
	OP(illegal),    OP(outl_p1_a),  OP(outl_p2_a), OP(illegal),   OP(movd_p4_a), OP(movd_p5_a),  OP(movd_p6_a), OP(movd_p7_a),
	OP(orl_a_xr0),  OP(orl_a_xr1),  OP(mov_a_t),   OP(orl_a_n),   OP(jmp_2),     OP(strt_cnt),   OP(jnt_1),     OP(swap_a),        /* 40 */
	OP(orl_a_r0),   OP(orl_a_r1),   OP(orl_a_r2),  OP(orl_a_r3),  OP(orl_a_r4),  OP(orl_a_r5),   OP(orl_a_r6),  OP(orl_a_r7),
	OP(anl_a_xr0),  OP(anl_a_xr1),  OP(jb_2),      OP(anl_a_n),   OP(call_2),    OP(strt_t),     OP(jt_1),      OP(da_a),          /* 50 */
	OP(anl_a_r0),   OP(anl_a_r1),   OP(anl_a_r2),  OP(anl_a_r3),  OP(anl_a_r4),  OP(anl_a_r5),   OP(anl_a_r6),  OP(anl_a_r7),
	OP(add_a_xr0),  OP(add_a_xr1),  OP(mov_t_a),   OP(illegal),   OP(jmp_3),     OP(stop_tcnt),  OP(illegal),   OP(rrc_a),         /* 60 */
	OP(add_a_r0),   OP(add_a_r1),   OP(add_a_r2),  OP(add_a_r3),  OP(add_a_r4),  OP(add_a_r5),   OP(add_a_r6),  OP(add_a_r7),
	OP(adc_a_xr0),  OP(adc_a_xr1),  OP(jb_3),      OP(illegal),   OP(call_3),    OP(illegal),    OP(jf1),       OP(rr_a),          /* 70 */
	OP(adc_a_r0),   OP(adc_a_r1),   OP(adc_a_r2),  OP(adc_a_r3),  OP(adc_a_r4),  OP(adc_a_r5),   OP(adc_a_r6),  OP(adc_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(ret),       OP(jmp_4),     OP(clr_f0),     OP(jobf),      OP(illegal),       /* 80 */
	OP(illegal),    OP(orl_p1_n),   OP(orl_p2_n),  OP(illegal),   OP(orld_p4_a), OP(orld_p5_a),  OP(orld_p6_a), OP(orld_p7_a),
	OP(mov_sts_a),  OP(illegal),    OP(jb_4),      OP(retr),      OP(call_4),    OP(cpl_f0),     OP(jnz),       OP(clr_c),         /* 90 */
	OP(illegal),    OP(anl_p1_n),   OP(anl_p2_n),  OP(illegal),   OP(anld_p4_a), OP(anld_p5_a),  OP(anld_p6_a), OP(anld_p7_a),
	OP(mov_xr0_a),  OP(mov_xr1_a),  OP(illegal),   OP(movp_a_xa), OP(jmp_5),     OP(clr_f1),     OP(illegal),   OP(cpl_c),         /* A0 */
	OP(mov_r0_a),   OP(mov_r1_a),   OP(mov_r2_a),  OP(mov_r3_a),  OP(mov_r4_a),  OP(mov_r5_a),   OP(mov_r6_a),  OP(mov_r7_a),
	OP(mov_xr0_n),  OP(mov_xr1_n),  OP(jb_5),      OP(jmpp_xa),   OP(call_5),    OP(cpl_f1),     OP(jf0),       OP(illegal),       /* B0 */
	OP(mov_r0_n),   OP(mov_r1_n),   OP(mov_r2_n),  OP(mov_r3_n),  OP(mov_r4_n),  OP(mov_r5_n),   OP(mov_r6_n),  OP(mov_r7_n),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(jmp_6),     OP(sel_rb0),    OP(jz),        OP(mov_a_psw),     /* C0 */
	OP(dec_r0),     OP(dec_r1),     OP(dec_r2),    OP(dec_r3),    OP(dec_r4),    OP(dec_r5),     OP(dec_r6),    OP(dec_r7),
	OP(xrl_a_xr0),  OP(xrl_a_xr1),  OP(jb_6),      OP(xrl_a_n),   OP(call_6),    OP(sel_rb1),    OP(jnibf),     OP(mov_psw_a),     /* D0 */
	OP(xrl_a_r0),   OP(xrl_a_r1),   OP(xrl_a_r2),  OP(xrl_a_r3),  OP(xrl_a_r4),  OP(xrl_a_r5),   OP(xrl_a_r6),  OP(xrl_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(movp3_a_xa),OP(jmp_7),     OP(en_dma),     OP(jnc),       OP(rl_a),          /* E0 */
	OP(djnz_r0),    OP(djnz_r1),    OP(djnz_r2),   OP(djnz_r3),   OP(djnz_r4),   OP(djnz_r5),    OP(djnz_r6),   OP(djnz_r7),
	OP(mov_a_xr0),  OP(mov_a_xr1),  OP(jb_7),      OP(illegal),   OP(call_7),    OP(en_flags),   OP(jc),        OP(rlc_a),         /* F0 */
	OP(mov_a_r0),   OP(mov_a_r1),   OP(mov_a_r2),  OP(mov_a_r3),  OP(mov_a_r4),  OP(mov_a_r5),   OP(mov_a_r6),  OP(mov_a_r7)
};

const mcs48_cpu_device::mcs48_ophandler mcs48_cpu_device::s_i8021_opcodes[256] =
{
	OP(nop),        OP(illegal),    OP(illegal),   OP(add_a_n),   OP(jmp_0),     OP(illegal),    OP(illegal),   OP(dec_a),         /* 00 */
	OP(in_a_p0),    OP(in_a_p1),    OP(in_a_p2),   OP(illegal),   OP(movd_a_p4), OP(movd_a_p5),  OP(movd_a_p6), OP(movd_a_p7),
	OP(inc_xr0),    OP(inc_xr1),    OP(illegal),   OP(adc_a_n),   OP(call_0),    OP(illegal),    OP(jtf),       OP(inc_a),         /* 10 */
	OP(inc_r0),     OP(inc_r1),     OP(inc_r2),    OP(inc_r3),    OP(inc_r4),    OP(inc_r5),     OP(inc_r6),    OP(inc_r7),
	OP(xch_a_xr0),  OP(xch_a_xr1),  OP(illegal),   OP(mov_a_n),   OP(jmp_1),     OP(illegal),    OP(illegal),   OP(clr_a),         /* 20 */
	OP(xch_a_r0),   OP(xch_a_r1),   OP(xch_a_r2),  OP(xch_a_r3),  OP(xch_a_r4),  OP(xch_a_r5),   OP(xch_a_r6),  OP(xch_a_r7),
	OP(xchd_a_xr0), OP(xchd_a_xr1), OP(illegal),   OP(illegal),   OP(call_1),    OP(illegal),    OP(illegal),   OP(cpl_a),         /* 30 */
	OP(illegal),    OP(outl_p1_a),  OP(outl_p2_a), OP(illegal),   OP(movd_p4_a), OP(movd_p5_a),  OP(movd_p6_a), OP(movd_p7_a),
	OP(orl_a_xr0),  OP(orl_a_xr1),  OP(mov_a_t),   OP(orl_a_n),   OP(jmp_2),     OP(strt_cnt),   OP(jnt_1),     OP(swap_a),        /* 40 */
	OP(orl_a_r0),   OP(orl_a_r1),   OP(orl_a_r2),  OP(orl_a_r3),  OP(orl_a_r4),  OP(orl_a_r5),   OP(orl_a_r6),  OP(orl_a_r7),
	OP(anl_a_xr0),  OP(anl_a_xr1),  OP(illegal),   OP(anl_a_n),   OP(call_2),    OP(strt_t),     OP(jt_1),      OP(da_a),          /* 50 */
	OP(anl_a_r0),   OP(anl_a_r1),   OP(anl_a_r2),  OP(anl_a_r3),  OP(anl_a_r4),  OP(anl_a_r5),   OP(anl_a_r6),  OP(anl_a_r7),
	OP(add_a_xr0),  OP(add_a_xr1),  OP(mov_t_a),   OP(illegal),   OP(jmp_3),     OP(stop_tcnt),  OP(illegal),   OP(rrc_a),         /* 60 */
	OP(add_a_r0),   OP(add_a_r1),   OP(add_a_r2),  OP(add_a_r3),  OP(add_a_r4),  OP(add_a_r5),   OP(add_a_r6),  OP(add_a_r7),
	OP(adc_a_xr0),  OP(adc_a_xr1),  OP(illegal),   OP(illegal),   OP(call_3),    OP(illegal),    OP(illegal),   OP(rr_a),          /* 70 */
	OP(adc_a_r0),   OP(adc_a_r1),   OP(adc_a_r2),  OP(adc_a_r3),  OP(adc_a_r4),  OP(adc_a_r5),   OP(adc_a_r6),  OP(adc_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(ret),       OP(jmp_4),     OP(illegal),    OP(illegal),   OP(illegal),       /* 80 */
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(orld_p4_a), OP(orld_p5_a),  OP(orld_p6_a), OP(orld_p7_a),
	OP(outl_p0_a),  OP(illegal),    OP(illegal),   OP(illegal),   OP(call_4),    OP(illegal),    OP(jnz),       OP(clr_c),         /* 90 */
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(anld_p4_a), OP(anld_p5_a),  OP(anld_p6_a), OP(anld_p7_a),
	OP(mov_xr0_a),  OP(mov_xr1_a),  OP(illegal),   OP(movp_a_xa), OP(jmp_5),     OP(illegal),    OP(illegal),   OP(cpl_c),         /* A0 */
	OP(mov_r0_a),   OP(mov_r1_a),   OP(mov_r2_a),  OP(mov_r3_a),  OP(mov_r4_a),  OP(mov_r5_a),   OP(mov_r6_a),  OP(mov_r7_a),
	OP(mov_xr0_n),  OP(mov_xr1_n),  OP(illegal),   OP(jmpp_xa),   OP(call_5),    OP(illegal),    OP(illegal),   OP(illegal),       /* B0 */
	OP(mov_r0_n),   OP(mov_r1_n),   OP(mov_r2_n),  OP(mov_r3_n),  OP(mov_r4_n),  OP(mov_r5_n),   OP(mov_r6_n),  OP(mov_r7_n),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(jmp_6),     OP(illegal),    OP(jz),        OP(illegal),       /* C0 */
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(illegal),   OP(illegal),    OP(illegal),   OP(illegal),
	OP(xrl_a_xr0),  OP(xrl_a_xr1),  OP(illegal),   OP(xrl_a_n),   OP(call_6),    OP(illegal),    OP(illegal),   OP(illegal),       /* D0 */
	OP(xrl_a_r0),   OP(xrl_a_r1),   OP(xrl_a_r2),  OP(xrl_a_r3),  OP(xrl_a_r4),  OP(xrl_a_r5),   OP(xrl_a_r6),  OP(xrl_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(jmp_7),     OP(illegal),    OP(jnc),       OP(rl_a),          /* E0 */
	OP(djnz_r0),    OP(djnz_r1),    OP(djnz_r2),   OP(djnz_r3),   OP(djnz_r4),   OP(djnz_r5),    OP(djnz_r6),   OP(djnz_r7),
	OP(mov_a_xr0),  OP(mov_a_xr1),  OP(illegal),   OP(illegal),   OP(call_7),    OP(illegal),    OP(jc),        OP(rlc_a),         /* F0 */
	OP(mov_a_r0),   OP(mov_a_r1),   OP(mov_a_r2),  OP(mov_a_r3),  OP(mov_a_r4),  OP(mov_a_r5),   OP(mov_a_r6),  OP(mov_a_r7)
};

const mcs48_cpu_device::mcs48_ophandler mcs48_cpu_device::s_i8022_opcodes[256] =
{
	OP(nop),        OP(illegal),    OP(illegal),   OP(add_a_n),   OP(jmp_0),     OP(en_i),       OP(illegal),   OP(dec_a),         /* 00 */
	OP(in_a_p0),    OP(in_a_p1),    OP(in_a_p2),   OP(illegal),   OP(movd_a_p4), OP(movd_a_p5),  OP(movd_a_p6), OP(movd_a_p7),
	OP(inc_xr0),    OP(inc_xr1),    OP(illegal),   OP(adc_a_n),   OP(call_0),    OP(dis_i),      OP(jtf),       OP(inc_a),         /* 10 */
	OP(inc_r0),     OP(inc_r1),     OP(inc_r2),    OP(inc_r3),    OP(inc_r4),    OP(inc_r5),     OP(inc_r6),    OP(inc_r7),
	OP(xch_a_xr0),  OP(xch_a_xr1),  OP(illegal),   OP(mov_a_n),   OP(jmp_1),     OP(en_tcnti),   OP(jnt_0),     OP(clr_a),         /* 20 */
	OP(xch_a_r0),   OP(xch_a_r1),   OP(xch_a_r2),  OP(xch_a_r3),  OP(xch_a_r4),  OP(xch_a_r5),   OP(xch_a_r6),  OP(xch_a_r7),
	OP(xchd_a_xr0), OP(xchd_a_xr1), OP(illegal),   OP(illegal),   OP(call_1),    OP(dis_tcnti),  OP(jt_0),      OP(cpl_a),         /* 30 */
	OP(illegal),    OP(outl_p1_a),  OP(outl_p2_a), OP(illegal),   OP(movd_p4_a), OP(movd_p5_a),  OP(movd_p6_a), OP(movd_p7_a),
	OP(orl_a_xr0),  OP(orl_a_xr1),  OP(mov_a_t),   OP(orl_a_n),   OP(jmp_2),     OP(strt_cnt),   OP(jnt_1),     OP(swap_a),        /* 40 */
	OP(orl_a_r0),   OP(orl_a_r1),   OP(orl_a_r2),  OP(orl_a_r3),  OP(orl_a_r4),  OP(orl_a_r5),   OP(orl_a_r6),  OP(orl_a_r7),
	OP(anl_a_xr0),  OP(anl_a_xr1),  OP(illegal),   OP(anl_a_n),   OP(call_2),    OP(strt_t),     OP(jt_1),      OP(da_a),          /* 50 */
	OP(anl_a_r0),   OP(anl_a_r1),   OP(anl_a_r2),  OP(anl_a_r3),  OP(anl_a_r4),  OP(anl_a_r5),   OP(anl_a_r6),  OP(anl_a_r7),
	OP(add_a_xr0),  OP(add_a_xr1),  OP(mov_t_a),   OP(illegal),   OP(jmp_3),     OP(stop_tcnt),  OP(illegal),   OP(rrc_a),         /* 60 */
	OP(add_a_r0),   OP(add_a_r1),   OP(add_a_r2),  OP(add_a_r3),  OP(add_a_r4),  OP(add_a_r5),   OP(add_a_r6),  OP(add_a_r7),
	OP(adc_a_xr0),  OP(adc_a_xr1),  OP(illegal),   OP(illegal),   OP(call_3),    OP(illegal),    OP(illegal),   OP(rr_a),          /* 70 */
	OP(adc_a_r0),   OP(adc_a_r1),   OP(adc_a_r2),  OP(adc_a_r3),  OP(adc_a_r4),  OP(adc_a_r5),   OP(adc_a_r6),  OP(adc_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(ret),       OP(jmp_4),     OP(illegal),    OP(illegal),   OP(illegal),       /* 80 */
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(orld_p4_a), OP(orld_p5_a),  OP(orld_p6_a), OP(orld_p7_a),
	OP(outl_p0_a),  OP(illegal),    OP(illegal),   OP(retr),      OP(call_4),    OP(illegal),    OP(jnz),       OP(clr_c),         /* 90 */
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(anld_p4_a), OP(anld_p5_a),  OP(anld_p6_a), OP(anld_p7_a),
	OP(mov_xr0_a),  OP(mov_xr1_a),  OP(illegal),   OP(movp_a_xa), OP(jmp_5),     OP(illegal),    OP(illegal),   OP(cpl_c),         /* A0 */
	OP(mov_r0_a),   OP(mov_r1_a),   OP(mov_r2_a),  OP(mov_r3_a),  OP(mov_r4_a),  OP(mov_r5_a),   OP(mov_r6_a),  OP(mov_r7_a),
	OP(mov_xr0_n),  OP(mov_xr1_n),  OP(illegal),   OP(jmpp_xa),   OP(call_5),    OP(illegal),    OP(illegal),   OP(illegal),       /* B0 */
	OP(mov_r0_n),   OP(mov_r1_n),   OP(mov_r2_n),  OP(mov_r3_n),  OP(mov_r4_n),  OP(mov_r5_n),   OP(mov_r6_n),  OP(mov_r7_n),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(jmp_6),     OP(illegal),    OP(jz),        OP(illegal),       /* C0 */
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(illegal),   OP(illegal),    OP(illegal),   OP(illegal),
	OP(xrl_a_xr0),  OP(xrl_a_xr1),  OP(illegal),   OP(xrl_a_n),   OP(call_6),    OP(illegal),    OP(illegal),   OP(illegal),       /* D0 */
	OP(xrl_a_r0),   OP(xrl_a_r1),   OP(xrl_a_r2),  OP(xrl_a_r3),  OP(xrl_a_r4),  OP(xrl_a_r5),   OP(xrl_a_r6),  OP(xrl_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(jmp_7),     OP(illegal),    OP(jnc),       OP(rl_a),          /* E0 */
	OP(djnz_r0),    OP(djnz_r1),    OP(djnz_r2),   OP(djnz_r3),   OP(djnz_r4),   OP(djnz_r5),    OP(djnz_r6),   OP(djnz_r7),
	OP(mov_a_xr0),  OP(mov_a_xr1),  OP(illegal),   OP(illegal),   OP(call_7),    OP(illegal),    OP(jc),        OP(rlc_a),         /* F0 */
	OP(mov_a_r0),   OP(mov_a_r1),   OP(mov_a_r2),  OP(mov_a_r3),  OP(mov_a_r4),  OP(mov_a_r5),   OP(mov_a_r6),  OP(mov_a_r7)
};



/***************************************************************************
    INITIALIZATION/RESET
***************************************************************************/

void mcs48_cpu_device::device_config_complete()
{
	m_t0_clk_func.resolve();
	if (!m_t0_clk_func.isnull())
		m_t0_clk_func(clock() / 3);
}

/*-------------------------------------------------
    mcs48_init - generic MCS-48 initialization
-------------------------------------------------*/

void mcs48_cpu_device::device_start()
{
	// zerofill
	m_prevpc = 0;
	m_pc = 0;

	m_a = 0;
	m_psw = 0;
	m_p1 = 0;
	m_p2 = 0;
	m_timer = 0;
	m_prescaler = 0;
	m_t1_history = 0;
	m_dbbi = 0;
	m_dbbo = 0;

	m_irq_state = false;
	m_irq_polled = false;
	m_irq_in_progress = false;
	m_timer_overflow = false;
	m_timer_flag = false;
	m_tirq_enabled = false;
	m_xirq_enabled = false;
	m_timecount_enabled = 0;
	m_flags_enabled = false;
	m_dma_enabled = false;
	m_a11 = 0;

	// External access line, EA=1: read from external rom, EA=0: read from internal rom
	// FIXME: Current implementation suboptimal
	m_ea = (m_int_rom_size ? 0 : 1);

	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).specific(m_data);
	if(m_feature_mask & EXT_BUS_FEATURE)
		space(AS_IO).specific(m_io);

	// resolve callbacks
	m_port_in_cb.resolve_all_safe(0xff);
	m_port_out_cb.resolve_all_safe();
	m_bus_in_cb.resolve_safe(0xff);
	m_bus_out_cb.resolve_safe();
	m_test_in_cb.resolve_all_safe(0);
	m_prog_out_cb.resolve_safe();

	// ensure that regptr is valid before get_info gets called
	update_regptr();

	// set up the state table
	{
		state_add(MCS48_PC,        "PC",        m_pc).mask(0xfff);
		state_add(STATE_GENPC,     "GENPC",     m_pc).mask(0xfff).noshow();
		state_add(STATE_GENPCBASE, "CURPC",     m_prevpc).mask(0xfff).noshow();
		state_add(STATE_GENSP,     "GENSP",     m_psw).mask(0x7).noshow();
		state_add(STATE_GENFLAGS,  "GENFLAGS",  m_psw).noshow().formatstr("%11s");
		state_add(MCS48_A,         "A",         m_a);
		state_add(MCS48_TC,        "TC",        m_timer);
		state_add(MCS48_TPRE,      "TPRE",      m_prescaler).mask(0x1f);

		if (m_feature_mask & I802X_FEATURE)
			state_add(MCS48_P0,    "P0",        m_dbbo);
		state_add(MCS48_P1,        "P1",        m_p1);
		state_add(MCS48_P2,        "P2",        m_p2);

		for (int regnum = 0; regnum < 8; regnum++) {
			state_add(MCS48_R0 + regnum, string_format("R%d", regnum).c_str(), m_rtemp).callimport().callexport();
		}

		if (m_feature_mask & EXT_BUS_FEATURE)
			state_add(MCS48_EA,    "EA",        m_ea).mask(0x1);

		if (m_feature_mask & UPI41_FEATURE)
		{
			state_add(MCS48_STS,   "STS",       m_sts);
			state_add(MCS48_DBBI,  "DBBI",      m_dbbi);
			state_add(MCS48_DBBO,  "DBBO",      m_dbbo);
		}

	}

	// register for savestates
	save_item(NAME(m_prevpc));
	save_item(NAME(m_pc));

	save_item(NAME(m_a));
	save_item(NAME(m_psw));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_ea));
	save_item(NAME(m_timer));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_t1_history));
	save_item(NAME(m_sts));
	save_item(NAME(m_dbbi));
	save_item(NAME(m_dbbo));

	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_polled));
	save_item(NAME(m_irq_in_progress));
	save_item(NAME(m_timer_overflow));
	save_item(NAME(m_timer_flag));
	save_item(NAME(m_tirq_enabled));
	save_item(NAME(m_xirq_enabled));
	save_item(NAME(m_timecount_enabled));
	save_item(NAME(m_flags_enabled));
	save_item(NAME(m_dma_enabled));

	save_item(NAME(m_a11));

	set_icountptr(m_icount);
}


void mcs48_cpu_device::device_reset()
{
	/* confirmed from reset description */
	m_pc = 0;
	m_psw = m_psw & (C_FLAG | A_FLAG);
	update_regptr();
	m_a11 = 0x000;
	m_dbbo = 0xff;
	bus_w(0xff);
	m_p1 = 0xff;
	m_p2 = 0xff;
	port_w(1, m_p1);
	port_w(2, m_p2);
	m_tirq_enabled = false;
	m_xirq_enabled = false;
	m_timecount_enabled = 0;
	m_timer_flag = false;
	m_sts = 0;
	m_flags_enabled = false;
	m_dma_enabled = false;
	if (!m_t0_clk_func.isnull())
		m_t0_clk_func(0);

	/* confirmed from interrupt logic description */
	m_irq_in_progress = false;
	m_timer_overflow = false;

	m_irq_polled = false;
}


/***************************************************************************
    EXECUTION
***************************************************************************/

/*-------------------------------------------------
    check_irqs - check for and process IRQs
-------------------------------------------------*/

void mcs48_cpu_device::check_irqs()
{
	/* if something is in progress, we do nothing */
	if (m_irq_in_progress)
		return;

	/* external interrupts take priority */
	else if ((m_irq_state || (m_sts & STS_IBF) != 0) && m_xirq_enabled)
	{
		burn_cycles(2);
		m_irq_in_progress = true;

		// force JNI to be taken (hack)
		if (m_irq_polled)
		{
			m_pc = ((m_prevpc + 1) & 0x7ff) | (m_prevpc & 0x800);
			execute_jcc(true);
		}

		/* transfer to location 0x03 */
		push_pc_psw();
		m_pc = 0x03;

		/* indicate we took the external IRQ */
		standard_irq_callback(0);
	}

	/* timer overflow interrupts follow */
	else if (m_timer_overflow && m_tirq_enabled)
	{
		burn_cycles(2);
		m_irq_in_progress = true;

		/* transfer to location 0x07 */
		push_pc_psw();
		m_pc = 0x07;

		/* timer overflow flip-flop is reset once taken */
		m_timer_overflow = false;
	}
}


/*-------------------------------------------------
    burn_cycles - burn cycles, processing timers
    and counters
-------------------------------------------------*/

void mcs48_cpu_device::burn_cycles(int count)
{
	if (count == 0)
		return;

	bool timerover = false;

	/* if the timer is enabled, accumulate prescaler cycles */
	if (m_timecount_enabled & TIMER_ENABLED)
	{
		uint8_t oldtimer = m_timer;
		m_prescaler += count;
		m_timer += m_prescaler >> 5;
		m_prescaler &= 0x1f;
		timerover = (oldtimer != 0 && m_timer == 0);
	}

	/* if the counter is enabled, poll the T1 test input once for each cycle */
	else if (m_timecount_enabled & COUNTER_ENABLED)
		for ( ; count > 0; count--, m_icount--)
		{
			m_t1_history = (m_t1_history << 1) | (test_r(1) & 1);
			if ((m_t1_history & 3) == 2)
			{
				if (++m_timer == 0)
					timerover = true;
			}
		}

	/* if timer counter was disabled, adjust icount here (otherwise count is 0) */
	m_icount -= count;

	/* if either source caused a timer overflow, set the flags and check IRQs */
	if (timerover)
	{
		m_timer_flag = true;

		/* according to the docs, if an overflow occurs with interrupts disabled, the overflow is not stored */
		if (m_tirq_enabled)
			m_timer_overflow = true;
	}
}


/*-------------------------------------------------
    mcs48_execute - execute until we run out
    of cycles
-------------------------------------------------*/

void mcs48_cpu_device::execute_run()
{
	update_regptr();

	// iterate over remaining cycles, guaranteeing at least one instruction
	do
	{
		// check interrupts
		check_irqs();
		m_irq_polled = false;

		m_prevpc = m_pc;
		debugger_instruction_hook(m_pc);

		// fetch and process opcode
		unsigned opcode = opcode_fetch();
		(this->*m_opcode_table[opcode])();

	} while (m_icount > 0);
}



/***************************************************************************
    DATA ACCESS HELPERS
***************************************************************************/

/*-------------------------------------------------
    upi41_master_r - master CPU data/status
    read
-------------------------------------------------*/

uint8_t upi41_cpu_device::upi41_master_r(offs_t offset)
{
	/* if just reading the status, return it */
	if ((offset & 1) != 0)
		return m_sts;

	/* if the output buffer was full, it gets cleared now */
	if (m_sts & STS_OBF)
	{
		m_sts &= ~STS_OBF;
		if (m_flags_enabled)
			port_w(2, m_p2 &= ~P2_OBF);
	}
	return m_dbbo;
}


/*-------------------------------------------------
    upi41_master_w - master CPU command/data
    write
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( upi41_cpu_device::master_callback )
{
	uint8_t a0 = (param >> 8) & 1;
	uint8_t data = param;

	/* data always goes to the input buffer */
	m_dbbi = data;

	/* set the appropriate flags */
	if ((m_sts & STS_IBF) == 0)
	{
		m_sts |= STS_IBF;
		if (m_flags_enabled)
			port_w(2, m_p2 &= ~P2_NIBF);
	}

	/* set F1 accordingly */
	if (a0 == 0)
		m_sts &= ~STS_F1;
	else
		m_sts |= STS_F1;
}

void upi41_cpu_device::upi41_master_w(offs_t offset, uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(upi41_cpu_device::master_callback), this), (offset << 8) | data);
}


/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

/*-------------------------------------------------
    mcs48_import_state - import state from the
    debugger into our internal format
-------------------------------------------------*/

void mcs48_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case MCS48_R0:
		case MCS48_R1:
		case MCS48_R2:
		case MCS48_R3:
		case MCS48_R4:
		case MCS48_R5:
		case MCS48_R6:
		case MCS48_R7:
			m_regptr[entry.index() - MCS48_R0] = m_rtemp;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(mcs48) called for unexpected value\n");
	}
}


/*-------------------------------------------------
    mcs48_export_state - prepare state for
    exporting to the debugger
-------------------------------------------------*/

void mcs48_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case MCS48_R0:
		case MCS48_R1:
		case MCS48_R2:
		case MCS48_R3:
		case MCS48_R4:
		case MCS48_R5:
		case MCS48_R6:
		case MCS48_R7:
			m_rtemp = m_regptr[entry.index() - MCS48_R0];
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(mcs48) called for unexpected value\n");
	}
}

void mcs48_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c %c%c%c%c%c%c%c%c",
				m_irq_state ? 'I':'.',
				m_a11       ? 'M':'.',
				m_psw & 0x80 ? 'C':'.',
				m_psw & 0x40 ? 'A':'.',
				m_psw & 0x20 ? 'F':'.',
				m_psw & 0x10 ? 'B':'.',
				m_psw & 0x08 ? '?':'.',
				m_psw & 0x04 ? '4':'.',
				m_psw & 0x02 ? '2':'.',
				m_psw & 0x01 ? '1':'.');
			break;
	}
}


void mcs48_cpu_device::execute_set_input(int inputnum, int state)
{
	switch( inputnum )
	{
		case MCS48_INPUT_IRQ:
			m_irq_state = (state != CLEAR_LINE);
			break;

		case MCS48_INPUT_EA:
			m_ea = (state != CLEAR_LINE);
			break;
	}
}
