// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *  Portable F8 emulator (Fairchild 3850)
 *
 *  This work is based on Frank Palazzolo's F8 emulation in a standalone
 *  Fairchild Channel F emulator and the 'Fairchild F3850 CPU' data sheets.
 *
 *  TODO:
 *  - ROMC signals are supposed to be handled externally
 *
 *****************************************************************************
 *
 *  The 3850 CPU itself does not include the address bus, pointer registers
 *  or an interrupt controller. Those functions are provided by at least one
 *  of the following devices:
 *
 *  - 3851 Program Storage Unit (PSU)
 *  - 3852 Dynamic Memory Interface (DMI)
 *  - 3853 Static Memory Interface (SMI)
 *  - 3854 Direct Memory Access Controller (DMAC)
 *  - 3856 Program Storage Unit (PSU)
 *  - 38T56 Program Storage Unit (PSU)
 *  - 3861 Peripheral Input/Output (PIO)
 *  - 3871 Peripheral Input/Output (PIO)
 *
 *  Of these support devices, the 3851 PSU includes 1024 bytes of mask ROM,
 *  and the 3856 PSU includes 2048 bytes of mask ROM; addressing for the PSU
 *  is also determined by mask option. The 3853 SMI may be used with external
 *  program ROMs.
 *
 *  The PSU does not have DC0 and DC1, only DC0; as a result, it does not
 *  respond to the main CPU's DC0/DC1 swap instruction.  This may lead to two
 *  devices responding to the same DC0 address and attempting to place their
 *  bytes on the data bus simultaneously!
 *
 *  Combined packages:
 *  - 3859 = 3850 + 3851
 *  - 3870 = 3850 + 38T56
 *  - 3872 = 3870 + extra 2KB ROM
 *  - 3873 = 3870 + extra 64 bytes executable RAM
 *
 *****************************************************************************/

#include "emu.h"
#include "f8.h"
#include "f8dasm.h"


/* status flags */
static constexpr u8 S = 0x01; // sign
static constexpr u8 C = 0x02; // carry
static constexpr u8 Z = 0x04; // zero
static constexpr u8 O = 0x08; // overflow
static constexpr u8 I = 0x10; // interrupt control bit (ICB)

/* cycle (short/long) */
static constexpr int cS = 4;
static constexpr int cL = 6;


DEFINE_DEVICE_TYPE(F8, f8_cpu_device, "f8", "Fairchild F8")


f8_cpu_device::f8_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cpu_device(mconfig, F8, tag, owner, clock),
	m_program_config("program", ENDIANNESS_BIG, 8, 16, 0),
	m_regs_config("register", ENDIANNESS_BIG, 8, 6, 0, address_map_constructor(FUNC(f8_cpu_device::regs_map), this)),
	m_io_config("io", ENDIANNESS_BIG, 8, 8, 0),
	m_romc08_callback(*this)
{ }

void f8_cpu_device::regs_map(address_map &map)
{
	// 64-byte internal scratchpad RAM
	map(0x00, 0x3f).ram().share("regs");
}

device_memory_interface::space_config_vector f8_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_regs_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

std::unique_ptr<util::disasm_interface> f8_cpu_device::create_disassembler()
{
	return std::make_unique<f8_disassembler>();
}

void f8_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c",
					m_w & 0x10 ? 'I':'.',
					m_w & 0x08 ? 'O':'.',
					m_w & 0x04 ? 'Z':'.',
					m_w & 0x02 ? 'C':'.',
					m_w & 0x01 ? 'S':'.');
			break;
	}
}

void f8_cpu_device::device_resolve_objects()
{
	m_romc08_callback.resolve_safe(0);
}

void f8_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).cache(m_r);
	space(AS_IO).specific(m_ios);

	// zerofill
	m_debug_pc = 0;
	m_pc0 = 0;
	m_pc1 = 0;
	m_dc0 = 0;
	m_dc1 = 0;
	m_a = 0;
	m_w = 0;
	m_is = 0;
	m_dbus = 0;
	m_io = 0;
	m_irq_vector = 0;
	m_irq_request = 0;

	// register for savestates
	save_item(NAME(m_debug_pc));
	save_item(NAME(m_pc0));
	save_item(NAME(m_pc1));
	save_item(NAME(m_dc0));
	save_item(NAME(m_dc1));
	save_item(NAME(m_a));
	save_item(NAME(m_w));
	save_item(NAME(m_is));
	save_item(NAME(m_dbus));
	save_item(NAME(m_io));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_irq_request));

	// register for debugger
	state_add(F8_PC0, "PC0", m_pc0);
	state_add(F8_PC1, "PC1", m_pc1);
	state_add(F8_DC0, "DC0", m_dc0);
	state_add(F8_DC1, "DC1", m_dc1);
	state_add(F8_W,   "W",   m_w).mask(0x1f);
	state_add(F8_A,   "A",   m_a);
	state_add(F8_IS,  "IS",  m_is).mask(0x3f);

	u8 *regs = static_cast<u8 *>(memshare("regs")->ptr());
	for (int r = 0; r < 9; r++)
		state_add(F8_R0 + r, string_format("R%d", r).c_str(), regs[r]);
	state_add(F8_J, "J", regs[9]);
	state_add<u16>(F8_H, "H",
		[regs]() { return u16(regs[10]) << 8 | regs[11]; },
		[regs](u16 data) { regs[10] = data >> 8; regs[11] = data & 0xff; });
	state_add<u16>(F8_K, "K",
		[regs]() { return u16(regs[12]) << 8 | regs[13]; },
		[regs](u16 data) { regs[12] = data >> 8; regs[13] = data & 0xff; });
	state_add<u16>(F8_Q, "Q",
		[regs]() { return u16(regs[14]) << 8 | regs[15]; },
		[regs](u16 data) { regs[14] = data >> 8; regs[15] = data & 0xff; });
	state_add(F8_HU, "HU", regs[10]).noshow();
	state_add(F8_HL, "HL", regs[11]).noshow();
	state_add(F8_KU, "KU", regs[12]).noshow();
	state_add(F8_KL, "KL", regs[13]).noshow();
	state_add(F8_QU, "QU", regs[14]).noshow();
	state_add(F8_QL, "QL", regs[15]).noshow();

	state_add(STATE_GENPC, "GENPC", m_debug_pc).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_debug_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_w).formatstr("%5s").noshow();

	set_icountptr(m_icount);
}

void f8_cpu_device::device_reset()
{
	// save PC0 to PC1 and reset PC0, and clear ICB
	ROMC_08();
	m_w &= ~I;

	// fetch the first opcode
	ROMC_00(cS);
}


void f8_cpu_device::execute_set_input( int inptnum, int state )
{
	assert (inptnum == F8_INPUT_LINE_INT_REQ);
	m_irq_request = state;
}

/*****************************************************************************/

/* clear all flags */
inline void f8_cpu_device::CLR_OZCS()
{
	m_w &= ~(O|Z|C|S);
}

/* set sign and zero flags (note: the S flag is complementary) */
inline void f8_cpu_device::SET_SZ(u8 n)
{
	if (n == 0)
		m_w |= Z;
	if (~n & 0x80)
		m_w |= S;
}

/* set overflow and carry flags */
inline u8 f8_cpu_device::do_add(u8 n, u8 m, u8 c)
{
	u16 r = n + m + c;
	if (r & 0x100)
		m_w |= C;
	if ((n^r) & (m^r) & 0x80)
		m_w |= O;

	return r & 0xff;
}

/* decimal add helper */
inline u8 f8_cpu_device::do_add_decimal(u8 augend, u8 addend)
{
	/* From F8 Guide To programming description of AMD
	 * binary add the addend to the binary sum of the augend and $66
	 * *NOTE* the binary addition of the augend to $66 is done before AMD is called
	 * record the status of the carry and intermediate carry
	 * add a factor to the sum based on the carry and intermediate carry:
	 * - no carry, no intermediate carry, add $AA
	 * - no carry, intermediate carry, add $A0
	 * - carry, no intermediate carry, add $0A
	 * - carry, intermediate carry, add $00
	 * any carry from the low-order digit is suppressed
	 * *NOTE* status flags are updated prior to the factor being added
	 */
	u8 tmp = augend + addend;

	u8 c = 0; // high order carry
	u8 ic = 0; // low order carry

	if (((augend + addend) & 0xff0) > 0xf0)
		c = 1;
	if ((augend & 0x0f) + (addend & 0x0f) > 0x0F)
		ic = 1;

	CLR_OZCS();
	do_add(augend,addend);
	SET_SZ(tmp);

	if (c == 0 && ic == 0)
		tmp = ((tmp + 0xa0) & 0xf0) + ((tmp + 0x0a) & 0x0f);
	if (c == 0 && ic == 1)
		tmp = ((tmp + 0xa0) & 0xf0) + (tmp & 0x0f);
	if (c == 1 && ic == 0)
		tmp = (tmp & 0xf0) + ((tmp + 0x0a) & 0x0f);

	return tmp;
}


/******************************************************************************
 * ROMC (ROM cycles)
 * This is what the Fairchild F8 CPUs use instead of an address bus
 * There are 5 control lines and each combination of those lines has
 * a special meaning. The devices attached to those control lines all
 * have their own program counters (PC0 and PC1) and at least one
 * data counter (DC0).
 * Currently the emulation does not handle distinct PCs and DCs, but
 * only one instance inside the CPU context.
 ******************************************************************************/
void f8_cpu_device::ROMC_00(int insttim)
{
	/*
	 * Instruction Fetch. The device whose address space includes the
	 * contents of the PC0 register must place on the data bus the op
	 * code addressed by PC0; then all devices increment the contents
	 * of PC0.
	 */

	m_dbus = m_program.read_byte(m_pc0);
	m_pc0 += 1;
	m_icount -= insttim; /* ROMC00 is usually short, not short+long, but DS is long */
}

void f8_cpu_device::ROMC_01()
{
	/*
	 * The device whose address space includes the contents of the PC0
	 * register must place on the data bus the contents of the memory
	 * location addressed by PC0; then all devices add the 8-bit value
	 * on the data bus as signed binary number to PC0.
	 */
	m_dbus = m_program.read_byte(m_pc0);
	m_pc0 += (s8)m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_02()
{
	/*
	 * The device whose DC0 addresses a memory word within the address
	 * space of that device must place on the data bus the contents of
	 * the memory location addressed by DC0; then all devices increment
	 * DC0.
	 */
	m_dbus = m_program.read_byte(m_dc0);
	m_dc0 += 1;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_03(int insttim)
{
	/*
	 * Similiar to 0x00, except that it is used for immediate operands
	 * fetches (using PC0) instead of instruction fetches.
	 */
	m_dbus = m_io = m_program.read_byte(m_pc0);
	m_pc0 += 1;
	m_icount -= insttim;
}

void f8_cpu_device::ROMC_04()
{
	/*
	 * Copy the contents of PC1 into PC0
	 */
	m_pc0 = m_pc1;
	m_icount -= cS;
}

void f8_cpu_device::ROMC_05()
{
	/*
	 * Store the data bus contents into the memory location pointed
	 * to by DC0; increment DC0.
	 */
	m_program.write_byte(m_dc0, m_dbus);
	m_dc0 += 1;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_06()
{
	/*
	 * Place the high order byte of DC0 on the data bus.
	 */
	m_dbus = m_dc0 >> 8;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_07()
{
	/*
	 * Place the high order byte of PC1 on the data bus.
	 */
	m_dbus = m_pc1 >> 8;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_08()
{
	/*
	 * All devices copy the contents of PC0 into PC1. The CPU outputs
	 * zero on the data bus in this ROMC state. Load the data bus into
	 * both halves of PC0, thus clearing the register.
	 */
	m_pc1 = m_pc0;
	m_dbus = 0;
	m_pc0 = m_romc08_callback() * 0x0101;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_09()
{
	/*
	 * The device whose address space includes the contents of the DC0
	 * register must place the low order byte of DC0 onto the data bus.
	 */
	m_dbus = m_dc0 & 0xff;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0A()
{
	/*
	 * All devices add the 8-bit value on the data bus, treated as
	 * signed binary number, to the data counter.
	 */
	m_dc0 += (s8)m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0B()
{
	/*
	 * The device whose address space includes the value in PC1
	 * must place the low order byte of PC1 onto the data bus.
	 */
	m_dbus = m_pc1 & 0xff;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0C()
{
	/*
	 * The device whose address space includes the contents of the PC0
	 * register must place the contents of the memory word addressed
	 * by PC0 into the data bus; then all devices move the value that
	 * has just been placed on the data bus into the low order byte of PC0.
	 */
	m_dbus = m_program.read_byte(m_pc0);
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0D()
{
	/*
	 * All devices store in PC1 the current contents of PC0, incremented
	 * by 1; PC0 is unaltered.
	 */
	m_pc1 = m_pc0 + 1;
	m_icount -= cS;
}

void f8_cpu_device::ROMC_0E()
{
	/*
	 * The device whose address space includes the contents of the PC0
	 * register must place the word addressed by PC0 into the data bus.
	 * The value on the data bus is then moved to the low order byte
	 * of DC0 by all devices.
	 */
	m_dbus = m_program.read_byte(m_pc0);
	m_dc0 = (m_dc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0F()
{
	/*
	 * The interrupting device with highest priority must place the
	 * low order byte of the interrupt vector on the data bus.
	 * All devices must copy the contents of PC0 into PC1. All devices
	 * must move the contents of the data bus into the low order
	 * byte of PC0.
	 */
	m_irq_vector = standard_irq_callback(F8_INPUT_LINE_INT_REQ);
	m_dbus = m_irq_vector & 0x00ff;
	m_pc1 = m_pc0;
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_10()
{
	/*
	 * Inhibit any modification to the interrupt priority logic.
	 */
	// TODO
	m_icount -= cL;
}

void f8_cpu_device::ROMC_11()
{
	/*
	 * The device whose address space includes the contents of PC0
	 * must place the contents of the addressed memory word on the
	 * data bus. All devices must then move the contents of the
	 * data bus to the upper byte of DC0.
	 */
	m_dbus = m_program.read_byte(m_pc0);
	m_dc0 = (m_dc0 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_12()
{
	/*
	 * All devices copy the contents of PC0 into PC1. All devices then
	 * move the contents of the data bus into the low order byte of PC0.
	 */
	m_pc1 = m_pc0;
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_13()
{
	/*
	 * The interrupting device with highest priority must move the high
	 * order half of the interrupt vector onto the data bus. All devices
	 * must then move the contents of the data bus into the high order
	 * byte of PC0. The interrupting device resets its interrupt circuitry
	 * (so that it is no longer requesting CPU servicing and can respond
	 * to another interrupt).
	 */
	m_dbus = m_irq_vector >> 8;
	m_pc0 = (m_pc0 & 0x00ff) | (m_dbus << 8);
	m_w&=~I;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_14()
{
	/*
	 * All devices move the contents of the data bus into the high
	 * order byte of PC0.
	 */
	m_pc0 = (m_pc0 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_15()
{
	/*
	 * All devices move the contents of the data bus into the high
	 * order byte of PC1.
	 */
	m_pc1 = (m_pc1 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_16()
{
	/*
	 * All devices move the contents of the data bus into the high
	 * order byte of DC0.
	 */
	m_dc0 = (m_dc0 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_17()
{
	/*
	 * All devices move the contents of the data bus into the low
	 * order byte of PC0.
	 */
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_18()
{
	/*
	 * All devices move the contents of the data bus into the low
	 * order byte of PC1.
	 */
	m_pc1 = (m_pc1 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_19()
{
	/*
	 * All devices move the contents of the data bus into the low
	 * order byte of DC0.
	 */
	m_dc0 = (m_dc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1A()
{
	/*
	 * During the prior cycle, an I/O port timer or interrupt control
	 * register was addressed; the device containing the addressed port
	 * must place the contents of the data bus into the address port.
	 */
	m_ios.write_byte(m_io, m_dbus);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1B()
{
	/*
	 * During the prior cycle, the data bus specified the address of an
	 * I/O port. The device containing the addressed I/O port must place
	 * the contents of the I/O port on the data bus. (Note that the
	 * contents of timer and interrupt control registers cannot be read
	 * back onto the data bus).
	 */
	m_dbus = m_ios.read_byte(m_io);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1C(int insttim)
{
	/*
	 * None.
	 */
	m_icount -= insttim;
}

void f8_cpu_device::ROMC_1D()
{
	/*
	 * Devices with DC0 and DC1 registers must switch registers.
	 * Devices without a DC1 register perform no operation.
	 */
	u16 tmp = m_dc0;
	m_dc0 = m_dc1;
	m_dc1 = tmp;
	m_icount -= cS;
}

void f8_cpu_device::ROMC_1E()
{
	/*
	 * The devices whose address space includes the contents of PC0
	 * must place the low order byte of PC0 onto the data bus.
	 */
	m_dbus = m_pc0 & 0xff;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1F()
{
	/*
	 * The devices whose address space includes the contents of PC0
	 * must place the high order byte of PC0 onto the data bus.
	 */
	m_dbus = (m_pc0 >> 8) & 0xff;
	m_icount -= cL;
}

/***********************************
 *  illegal opcodes
 ***********************************/
void f8_cpu_device::illegal()
{
	logerror("f8 illegal opcode at 0x%04x: %02x\n", m_pc0, m_dbus);
}

/***************************************************
 *  O Z C S 0000 0000
 *  - - - - LR  A,KU
 ***************************************************/
void f8_cpu_device::f8_lr_a_ku()
{
	m_a = m_r.read_byte(12);
}

/***************************************************
 *  O Z C S 0000 0001
 *  - - - - LR  A,KL
 ***************************************************/
void f8_cpu_device::f8_lr_a_kl()
{
	m_a = m_r.read_byte(13);
}

/***************************************************
 *  O Z C S 0000 0010
 *  - - - - LR  A,QU
 ***************************************************/
void f8_cpu_device::f8_lr_a_qu()
{
	m_a = m_r.read_byte(14);
}

/***************************************************
 *  O Z C S 0000 0011
 *  - - - - LR  A,QL
 ***************************************************/
void f8_cpu_device::f8_lr_a_ql()
{
	m_a = m_r.read_byte(15);
}

/***************************************************
 *  O Z C S 0000 0100
 *  - - - - LR  KU,A
 ***************************************************/
void f8_cpu_device::f8_lr_ku_a()
{
	m_r.write_byte(12, m_a);
}

/***************************************************
 *  O Z C S 0000 0101
 *  - - - - LR  KL,A
 ***************************************************/
void f8_cpu_device::f8_lr_kl_a()
{
	m_r.write_byte(13, m_a);
}

/***************************************************
 *  O Z C S 0000 0110
 *  - - - - LR  QU,A
 ***************************************************/
void f8_cpu_device::f8_lr_qu_a()
{
	m_r.write_byte(14, m_a);
}

/***************************************************
 *  O Z C S 0000 0111
 *  - - - - LR  QL,A
 ***************************************************/
void f8_cpu_device::f8_lr_ql_a()
{
	m_r.write_byte(15, m_a);
}

/***************************************************
 *  O Z C S 0000 1000
 *  - - - - LR  K,P
 ***************************************************/
void f8_cpu_device::f8_lr_k_p()
{
	ROMC_07();
	m_r.write_byte(12, m_dbus);
	ROMC_0B();
	m_r.write_byte(13, m_dbus);
}

/***************************************************
 *  O Z C S 0000 1001
 *  - - - - LR  P,K
 ***************************************************/
void f8_cpu_device::f8_lr_p_k()
{
	m_dbus = m_r.read_byte(12);
	ROMC_15();
	m_dbus = m_r.read_byte(13);
	ROMC_18();
}

/***************************************************
 *  O Z C S 0000 1010
 *  - - - - LR  A,IS
 ***************************************************/
void f8_cpu_device::f8_lr_a_is()
{
	m_a = m_is;
}

/***************************************************
 *  O Z C S 0000 1011
 *  - - - - LR  IS,A
 ***************************************************/
void f8_cpu_device::f8_lr_is_a()
{
	m_is = m_a & 0x3f;
}

/***************************************************
 *  O Z C S 0000 1100
 *  - - - - PK
 ***************************************************/
void f8_cpu_device::f8_pk()
{
	m_dbus = m_r.read_byte(13);
	ROMC_12();
	m_dbus = m_r.read_byte(12);
	ROMC_14();
}

/***************************************************
 *  O Z C S 0000 1101
 *  - - - - LR  P0,Q
 ***************************************************/
void f8_cpu_device::f8_lr_p0_q()
{
	m_dbus = m_r.read_byte(15);
	ROMC_17();
	m_dbus = m_r.read_byte(14);
	ROMC_14();
}

/***************************************************
 *  O Z C S 0000 1110
 *  - - - - LR   Q,DC
 ***************************************************/
void f8_cpu_device::f8_lr_q_dc()
{
	ROMC_06();
	m_r.write_byte(14, m_dbus);
	ROMC_09();
	m_r.write_byte(15, m_dbus);
}

/***************************************************
 *  O Z C S 0000 1111
 *  - - - - LR   DC,Q
 ***************************************************/
void f8_cpu_device::f8_lr_dc_q()
{
	m_dbus = m_r.read_byte(14);
	ROMC_16();
	m_dbus = m_r.read_byte(15);
	ROMC_19();
}

/***************************************************
 *  O Z C S 0001 0000
 *  - - - - LR   DC,H
 ***************************************************/
void f8_cpu_device::f8_lr_dc_h()
{
	m_dbus = m_r.read_byte(10);
	ROMC_16();
	m_dbus = m_r.read_byte(11);
	ROMC_19();
}

/***************************************************
 *  O Z C S 0001 0001
 *  - - - - LR   H,DC
 ***************************************************/
void f8_cpu_device::f8_lr_h_dc()
{
	ROMC_06();
	m_r.write_byte(10, m_dbus);
	ROMC_09();
	m_r.write_byte(11, m_dbus);
}

/***************************************************
 *  O Z C S 0001 0010
 *  0 x 0 1 SR   1
 ***************************************************/
void f8_cpu_device::f8_sr_1()
{
	m_a >>= 1;
	CLR_OZCS();
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0011
 *  0 x 0 x SL   1
 ***************************************************/
void f8_cpu_device::f8_sl_1()
{
	m_a <<= 1;
	CLR_OZCS();
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0100
 *  0 x 0 1 SR   4
 ***************************************************/
void f8_cpu_device::f8_sr_4()
{
	m_a >>= 4;
	CLR_OZCS();
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0101
 *  0 x 0 x SL   4
 ***************************************************/
void f8_cpu_device::f8_sl_4()
{
	m_a <<= 4;
	CLR_OZCS();
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0110
 *  - - - - LM
 ***************************************************/
void f8_cpu_device::f8_lm()
{
	ROMC_02();
	m_a = m_dbus;
}

/***************************************************
 *  O Z C S 0001 0111
 *  - - - - ST
 ***************************************************/
void f8_cpu_device::f8_st()
{
	m_dbus = m_a;
	ROMC_05();
}

/***************************************************
 *  O Z C S 0001 1000
 *  0 x 0 x COM
 ***************************************************/
void f8_cpu_device::f8_com()
{
	m_a = ~m_a;
	CLR_OZCS();
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 1001
 *  x x x x LNK
 ***************************************************/
void f8_cpu_device::f8_lnk()
{
	bool c = (m_w & C) != 0;
	CLR_OZCS();
	if (c)
		m_a = do_add(m_a,1);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 1010
 *          DI
 ***************************************************/
void f8_cpu_device::f8_di()
{
	ROMC_1C(cS);
	m_w &= ~I;
}

/***************************************************
 *  O Z C S 0001 1011
 *          EI
 ***************************************************/
void f8_cpu_device::f8_ei()
{
	ROMC_1C(cS);
	m_w |= I;
}

/***************************************************
 *  O Z C S 0001 1100
 *          POP
 ***************************************************/
void f8_cpu_device::f8_pop()
{
	ROMC_04();
}

/***************************************************
 *  O Z C S 0001 1101
 *  x x x x LR   W,J
 ***************************************************/
void f8_cpu_device::f8_lr_w_j()
{
	ROMC_1C(cS);
	m_w = m_r.read_byte(9) & 0x1f;
}

/***************************************************
 *  O Z C S 0001 1110
 *  - - - - LR   J,W
 ***************************************************/
void f8_cpu_device::f8_lr_j_w()
{
	m_r.write_byte(9, m_w);
}

/***************************************************
 *  O Z C S 0001 1111
 *  x x x x INC
 ***************************************************/
void f8_cpu_device::f8_inc()
{
	CLR_OZCS();
	m_a = do_add(m_a,1);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0000   aaaa aaaa
 *  - - - - LI  aa
 ***************************************************/
void f8_cpu_device::f8_li()
{
	ROMC_03(cL);
	m_a = m_dbus;
}

/***************************************************
 *  O Z C S 0010 0001   aaaa aaaa
 *  0 x 0 x NI   aa
 ***************************************************/
void f8_cpu_device::f8_ni()
{
	ROMC_03(cL);
	CLR_OZCS();
	m_a &= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0010   aaaa aaaa
 *  0 x 0 x OI   aa
 ***************************************************/
void f8_cpu_device::f8_oi()
{
	ROMC_03(cL);
	CLR_OZCS();
	m_a |= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0011   aaaa aaaa
 *  0 x 0 x XI   aa
 ***************************************************/
void f8_cpu_device::f8_xi()
{
	ROMC_03(cL);
	CLR_OZCS();
	m_a ^= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0100   aaaa aaaa
 *  x x x x AI   aa
 ***************************************************/
void f8_cpu_device::f8_ai()
{
	ROMC_03(cL);
	CLR_OZCS();
	m_a = do_add(m_a,m_dbus);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0101   aaaa aaaa
 *  x x x x CI   aa
 ***************************************************/
void f8_cpu_device::f8_ci()
{
	ROMC_03(cL);
	CLR_OZCS();
	SET_SZ(do_add(~m_a,m_dbus,1));
}

/***************************************************
 *  O Z C S 0010 0110   aaaa aaaa
 *  0 x 0 x IN   aa
 ***************************************************/
void f8_cpu_device::f8_in()
{
	ROMC_03(cL);
	CLR_OZCS();
	ROMC_1B();
	m_a = m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0111   aaaa aaaa
 *  - - - - OUT  aa
 ***************************************************/
void f8_cpu_device::f8_out()
{
	ROMC_03(cL);
	m_dbus = m_a;
	ROMC_1A();
}

/***************************************************
 *  O Z C S 0010 1000   iiii iiii   jjjj jjjj
 *  - - - - PI   iijj
 ***************************************************/
void f8_cpu_device::f8_pi()
{
	ROMC_03(cL);
	m_a = m_dbus;
	ROMC_0D();
	ROMC_0C();
	m_dbus = m_a;
	ROMC_14();
}

/***************************************************
 *  O Z C S 0010 1001   iiii iiii   jjjj jjjj
 *  - - - - JMP  iijj
 ***************************************************/
void f8_cpu_device::f8_jmp()
{
	ROMC_03(cL);
	m_a = m_dbus;
	ROMC_0C();
	m_dbus = m_a;
	ROMC_14();
}

/***************************************************
 *  O Z C S 0010 1010   iiii iiii   jjjj jjjj
 *  - - - - DCI  iijj
 ***************************************************/
void f8_cpu_device::f8_dci()
{
	ROMC_11();
	ROMC_03(cS);
	ROMC_0E();
	ROMC_03(cS);
}

/***************************************************
 *  O Z C S 0010 1011
 *  - - - - NOP
 ***************************************************/
void f8_cpu_device::f8_nop()
{
}

/***************************************************
 *  O Z C S 0010 1100
 *  - - - - XDC
 ***************************************************/
void f8_cpu_device::f8_xdc()
{
	ROMC_1D();
}

/***************************************************
 *  O Z C S 0011 rrrr
 *  x x x x DS   r
 ***************************************************/
void f8_cpu_device::f8_ds_r(int r)
{
	CLR_OZCS();
	int d = do_add(m_r.read_byte(r), 0xff);
	m_r.write_byte(r, d);
	SET_SZ(d);
}

/***************************************************
 *  O Z C S 0011 1100
 *  x x x x DS   ISAR
 ***************************************************/
void f8_cpu_device::f8_ds_isar()
{
	CLR_OZCS();
	int d = do_add(m_r.read_byte(m_is), 0xff);
	m_r.write_byte(m_is, d);
	SET_SZ(d);
}

/***************************************************
 *  O Z C S 0011 1101
 *  x x x x DS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_ds_isar_i()
{
	CLR_OZCS();
	int d = do_add(m_r.read_byte(m_is), 0xff);
	m_r.write_byte(m_is, d);
	SET_SZ(d);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0011 1110
 *  x x x x DS  ISAR--
 ***************************************************/
void f8_cpu_device::f8_ds_isar_d()
{
	CLR_OZCS();
	int d = do_add(m_r.read_byte(m_is), 0xff);
	m_r.write_byte(m_is, d);
	SET_SZ(d);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0100 rrrr
 *  - - - - LR  A,r
 ***************************************************/
void f8_cpu_device::f8_lr_a_r(int r)
{
	m_a = m_r.read_byte(r);
}

/***************************************************
 *  O Z C S 0100 1100
 *  - - - - LR  A,ISAR
 ***************************************************/
void f8_cpu_device::f8_lr_a_isar()
{
	m_a = m_r.read_byte(m_is);
}

/***************************************************
 *  O Z C S 0100 1101
 *  - - - - LR  A,ISAR++
 ***************************************************/
void f8_cpu_device::f8_lr_a_isar_i()
{
	m_a = m_r.read_byte(m_is);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0100 1110
 *  - - - - LR  A,ISAR--
 ***************************************************/
void f8_cpu_device::f8_lr_a_isar_d()
{
	m_a = m_r.read_byte(m_is);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0101 rrrr
 *  - - - - LR  r,A
 ***************************************************/
void f8_cpu_device::f8_lr_r_a(int r)
{
	m_r.write_byte(r, m_a);
}

/***************************************************
 *  O Z C S 0101 1100
 *  - - - - LR  ISAR,A
 ***************************************************/
void f8_cpu_device::f8_lr_isar_a()
{
	m_r.write_byte(m_is, m_a);
}

/***************************************************
 *  O Z C S 0101 1101
 *  - - - - LR  ISAR++,A
 ***************************************************/
void f8_cpu_device::f8_lr_isar_i_a()
{
	m_r.write_byte(m_is, m_a);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0101 1110
 *  - - - - LR  ISAR--,A
 ***************************************************/
void f8_cpu_device::f8_lr_isar_d_a()
{
	m_r.write_byte(m_is, m_a);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0110 0eee
 *  - - - - LISU e
 ***************************************************/
void f8_cpu_device::f8_lisu(int e)
{
	m_is = (m_is & 0x07) | e;
}

/***************************************************
 *  O Z C S 0110 1eee
 *  - - - - LISL e
 ***************************************************/
void f8_cpu_device::f8_lisl(int e)
{
	m_is = (m_is & 0x38) | e;
}

/***************************************************
 *  O Z C S 0111 iiii
 *  - - - - LIS  i
 ***************************************************/
void f8_cpu_device::f8_lis(int i)
{
	m_a = i;
}

/***************************************************
 *  O Z C S 1000 0eee   aaaa aaaa
 *          BT   e,aa
 ***************************************************/
void f8_cpu_device::f8_bt(int e)
{
	ROMC_1C(cS);
	if (m_w & e)
		ROMC_01(); // take the relative branch
	else
		ROMC_03(cS); // just read the argument on the data bus
}

/***************************************************
 *  O Z C S 1000 1000
 *  x x x x AM
 ***************************************************/
void f8_cpu_device::f8_am()
{
	ROMC_02();
	CLR_OZCS();
	m_a = do_add(m_a, m_dbus);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1001
 *  x x x x AMD
 ***************************************************/
void f8_cpu_device::f8_amd()
{
	ROMC_02();
	m_a = do_add_decimal(m_a, m_dbus);
}

/***************************************************
 *  O Z C S 1000 1010
 *  0 x 0 x NM
 ***************************************************/
void f8_cpu_device::f8_nm()
{
	ROMC_02();
	CLR_OZCS();
	m_a &= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1011
 *  0 x 0 x OM
 ***************************************************/
void f8_cpu_device::f8_om()
{
	ROMC_02();
	CLR_OZCS();
	m_a |= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1100
 *  0 x 0 x XM
 ***************************************************/
void f8_cpu_device::f8_xm()
{
	ROMC_02();
	CLR_OZCS();
	m_a ^= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1101
 *  x x x x CM
 ***************************************************/
void f8_cpu_device::f8_cm()
{
	ROMC_02();
	CLR_OZCS();
	SET_SZ(do_add(~m_a,m_dbus,1));
}

/***************************************************
 *  O Z C S 1000 1110
 *  - - - - ADC
 ***************************************************/
void f8_cpu_device::f8_adc()
{
	m_dbus = m_a;
	ROMC_0A(); // add data bus value to DC0
}

/***************************************************
 *  O Z C S 1000 1111
 *  - - - - BR7
 ***************************************************/
void f8_cpu_device::f8_br7()
{
	if ((m_is & 7) == 7)
		ROMC_03(cS); // just read the argument on the data bus
	else
		ROMC_01(); // take the relative branch
}

/***************************************************
 *  O Z C S 1001 tttt   aaaa aaaa
 *  - - - - BF   t,aa
 ***************************************************/
void f8_cpu_device::f8_bf(int t)
{
	ROMC_1C(cS);
	if (m_w & t)
		ROMC_03(cS); // just read the argument on the data bus
	else
		ROMC_01(); // take the relative branch
}

/***************************************************
 *  O Z C S 1010 000n
 *  0 x 0 x INS  n              (n = 0-1)
 ***************************************************/
void f8_cpu_device::f8_ins_0(int n)
{
	ROMC_1C(cS);
	CLR_OZCS();
	m_a = m_ios.read_byte(n);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1010 nnnn
 *  0 x 0 x INS  n              (n = 4-F)
 ***************************************************/
void f8_cpu_device::f8_ins_1(int n)
{
	ROMC_1C(cL);
	m_io = n;
	ROMC_1B();
	CLR_OZCS();
	m_a = m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1011 000n
 *  - - - - OUTS n              (n = 0-1)
 ***************************************************/
void f8_cpu_device::f8_outs_0(int n)
{
	ROMC_1C(cS);
	m_ios.write_byte(n, m_a);
}

/***************************************************
 *  O Z C S 1011 nnnn
 *  - - - - OUTS n              (n = 4-F)
 ***************************************************/
void f8_cpu_device::f8_outs_1(int n)
{
	ROMC_1C(cL);
	m_io = n;
	m_dbus = m_a;
	ROMC_1A();
}

/***************************************************
 *  O Z C S 1100 rrrr
 *  x x x x AS   r
 ***************************************************/
void f8_cpu_device::f8_as(int r)
{
	CLR_OZCS();
	m_a = do_add(m_a, m_r.read_byte(r));
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1100 1100
 *  x x x x AS   ISAR
 ***************************************************/
void f8_cpu_device::f8_as_isar()
{
	CLR_OZCS();
	m_a = do_add(m_a, m_r.read_byte(m_is));
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1100 1101
 *  x x x x AS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_as_isar_i()
{
	CLR_OZCS();
	m_a = do_add(m_a, m_r.read_byte(m_is));
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1100 1110
 *  x x x x AS   ISAR--
 ***************************************************/
void f8_cpu_device::f8_as_isar_d()
{
	CLR_OZCS();
	m_a = do_add(m_a, m_r.read_byte(m_is));
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1101 rrrr
 *  x x x x ASD  r
 ***************************************************/
void f8_cpu_device::f8_asd(int r)
{
	ROMC_1C(cS);
	m_a = do_add_decimal(m_a, m_r.read_byte(r));
}

/***************************************************
 *  O Z C S 1101 1100
 *  x x x x ASD  ISAR
 ***************************************************/
void f8_cpu_device::f8_asd_isar()
{
	ROMC_1C(cS);
	m_a = do_add_decimal(m_a, m_r.read_byte(m_is));
}

/***************************************************
 *  O Z C S 1101 1101
 *  x x x x ASD  ISAR++
 ***************************************************/
void f8_cpu_device::f8_asd_isar_i()
{
	ROMC_1C(cS);
	m_a = do_add_decimal(m_a, m_r.read_byte(m_is));
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1101 1110
 *  x x x x ASD  ISAR--
 ***************************************************/
void f8_cpu_device::f8_asd_isar_d()
{
	ROMC_1C(cS);
	m_a = do_add_decimal(m_a, m_r.read_byte(m_is));
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1110 rrrr
 *  0 x 0 x XS   r
 ***************************************************/
void f8_cpu_device::f8_xs(int r)
{
	CLR_OZCS();
	m_a ^= m_r.read_byte(r);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1110 1100
 *  0 x 0 x XS   ISAR
 ***************************************************/
void f8_cpu_device::f8_xs_isar()
{
	CLR_OZCS();
	m_a ^= m_r.read_byte(m_is);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1110 1101
 *  0 x 0 x XS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_xs_isar_i()
{
	CLR_OZCS();
	m_a ^= m_r.read_byte(m_is);
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1110 1110
 *  0 x 0 x XS   ISAR--
 ***************************************************/
void f8_cpu_device::f8_xs_isar_d()
{
	CLR_OZCS();
	m_a ^= m_r.read_byte(m_is);
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1111 rrrr
 *  0 x 0 x NS   r
 ***************************************************/
void f8_cpu_device::f8_ns(int r)
{
	CLR_OZCS();
	m_a &= m_r.read_byte(r);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1111 1100
 *  0 x 0 x NS   ISAR
 ***************************************************/
void f8_cpu_device::f8_ns_isar()
{
	CLR_OZCS();
	m_a &= m_r.read_byte(m_is);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1111 1101
 *  0 x 0 x NS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_ns_isar_i()
{
	CLR_OZCS();
	m_a &= m_r.read_byte(m_is);
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1111 1110
 *  0 x 0 x NS   ISAR--
 ***************************************************/
void f8_cpu_device::f8_ns_isar_d()
{
	CLR_OZCS();
	m_a &= m_r.read_byte(m_is);
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}


/***************************************************
 *  Execute cycles
 ***************************************************/

void f8_cpu_device::execute_run()
{
	do
	{
		u8 op = m_dbus;

		m_debug_pc = (m_pc0 - 1) & 0xffff;
		debugger_instruction_hook(m_debug_pc);

		switch (op)
		{
			case 0x00: f8_lr_a_ku(); break;
			case 0x01: f8_lr_a_kl(); break;
			case 0x02: f8_lr_a_qu(); break;
			case 0x03: f8_lr_a_ql(); break;
			case 0x04: f8_lr_ku_a(); break;
			case 0x05: f8_lr_kl_a(); break;
			case 0x06: f8_lr_qu_a(); break;
			case 0x07: f8_lr_ql_a(); break;
			case 0x08: f8_lr_k_p(); break;
			case 0x09: f8_lr_p_k(); break;
			case 0x0a: f8_lr_a_is(); break;
			case 0x0b: f8_lr_is_a(); break;
			case 0x0c: f8_pk(); break;
			case 0x0d: f8_lr_p0_q(); break;
			case 0x0e: f8_lr_q_dc(); break;
			case 0x0f: f8_lr_dc_q(); break;

			case 0x10: f8_lr_dc_h(); break;
			case 0x11: f8_lr_h_dc(); break;
			case 0x12: f8_sr_1(); break;
			case 0x13: f8_sl_1(); break;
			case 0x14: f8_sr_4(); break;
			case 0x15: f8_sl_4(); break;
			case 0x16: f8_lm(); break;
			case 0x17: f8_st(); break;
			case 0x18: f8_com(); break;
			case 0x19: f8_lnk(); break;
			case 0x1a: f8_di(); break;
			case 0x1b: f8_ei(); break;
			case 0x1c: f8_pop(); break;
			case 0x1d: f8_lr_w_j(); break;
			case 0x1e: f8_lr_j_w(); break;
			case 0x1f: f8_inc(); break;

			case 0x20: f8_li(); break;
			case 0x21: f8_ni(); break;
			case 0x22: f8_oi(); break;
			case 0x23: f8_xi(); break;
			case 0x24: f8_ai(); break;
			case 0x25: f8_ci(); break;
			case 0x26: f8_in(); break;
			case 0x27: f8_out(); break;
			case 0x28: f8_pi(); break;
			case 0x29: f8_jmp(); break;
			case 0x2a: f8_dci(); break;
			case 0x2b: f8_nop(); break;
			case 0x2c: f8_xdc(); break;
			case 0x2d: illegal(); break;
			case 0x2e: illegal(); break;
			case 0x2f: illegal(); break;

			case 0x30: f8_ds_r(0); break;
			case 0x31: f8_ds_r(1); break;
			case 0x32: f8_ds_r(2); break;
			case 0x33: f8_ds_r(3); break;
			case 0x34: f8_ds_r(4); break;
			case 0x35: f8_ds_r(5); break;
			case 0x36: f8_ds_r(6); break;
			case 0x37: f8_ds_r(7); break;
			case 0x38: f8_ds_r(8); break;
			case 0x39: f8_ds_r(9); break;
			case 0x3a: f8_ds_r(10); break;
			case 0x3b: f8_ds_r(11); break;
			case 0x3c: f8_ds_isar(); break;
			case 0x3d: f8_ds_isar_i(); break;
			case 0x3e: f8_ds_isar_d(); break;
			case 0x3f: illegal(); break;

			case 0x40: f8_lr_a_r(0); break;
			case 0x41: f8_lr_a_r(1); break;
			case 0x42: f8_lr_a_r(2); break;
			case 0x43: f8_lr_a_r(3); break;
			case 0x44: f8_lr_a_r(4); break;
			case 0x45: f8_lr_a_r(5); break;
			case 0x46: f8_lr_a_r(6); break;
			case 0x47: f8_lr_a_r(7); break;
			case 0x48: f8_lr_a_r(8); break;
			case 0x49: f8_lr_a_r(9); break;
			case 0x4a: f8_lr_a_r(10); break;
			case 0x4b: f8_lr_a_r(11); break;
			case 0x4c: f8_lr_a_isar(); break;
			case 0x4d: f8_lr_a_isar_i(); break;
			case 0x4e: f8_lr_a_isar_d(); break;
			case 0x4f: illegal(); break;

			case 0x50: f8_lr_r_a(0); break;
			case 0x51: f8_lr_r_a(1); break;
			case 0x52: f8_lr_r_a(2); break;
			case 0x53: f8_lr_r_a(3); break;
			case 0x54: f8_lr_r_a(4); break;
			case 0x55: f8_lr_r_a(5); break;
			case 0x56: f8_lr_r_a(6); break;
			case 0x57: f8_lr_r_a(7); break;
			case 0x58: f8_lr_r_a(8); break;
			case 0x59: f8_lr_r_a(9); break;
			case 0x5a: f8_lr_r_a(10); break;
			case 0x5b: f8_lr_r_a(11); break;
			case 0x5c: f8_lr_isar_a(); break;
			case 0x5d: f8_lr_isar_i_a(); break;
			case 0x5e: f8_lr_isar_d_a(); break;
			case 0x5f: illegal(); break;

			case 0x60: f8_lisu(0x00); break;
			case 0x61: f8_lisu(0x08); break;
			case 0x62: f8_lisu(0x10); break;
			case 0x63: f8_lisu(0x18); break;
			case 0x64: f8_lisu(0x20); break;
			case 0x65: f8_lisu(0x28); break;
			case 0x66: f8_lisu(0x30); break;
			case 0x67: f8_lisu(0x38); break;
			case 0x68: f8_lisl(0x00); break;
			case 0x69: f8_lisl(0x01); break;
			case 0x6a: f8_lisl(0x02); break;
			case 0x6b: f8_lisl(0x03); break;
			case 0x6c: f8_lisl(0x04); break;
			case 0x6d: f8_lisl(0x05); break;
			case 0x6e: f8_lisl(0x06); break;
			case 0x6f: f8_lisl(0x07); break;

			case 0x70: f8_lis(0x0); break;
			case 0x71: f8_lis(0x1); break;
			case 0x72: f8_lis(0x2); break;
			case 0x73: f8_lis(0x3); break;
			case 0x74: f8_lis(0x4); break;
			case 0x75: f8_lis(0x5); break;
			case 0x76: f8_lis(0x6); break;
			case 0x77: f8_lis(0x7); break;
			case 0x78: f8_lis(0x8); break;
			case 0x79: f8_lis(0x9); break;
			case 0x7a: f8_lis(0xa); break;
			case 0x7b: f8_lis(0xb); break;
			case 0x7c: f8_lis(0xc); break;
			case 0x7d: f8_lis(0xd); break;
			case 0x7e: f8_lis(0xe); break;
			case 0x7f: f8_lis(0xf); break;

			case 0x80: f8_bt(0); break;
			case 0x81: f8_bt(1); break;
			case 0x82: f8_bt(2); break;
			case 0x83: f8_bt(3); break;
			case 0x84: f8_bt(4); break;
			case 0x85: f8_bt(5); break;
			case 0x86: f8_bt(6); break;
			case 0x87: f8_bt(7); break;
			case 0x88: f8_am(); break;
			case 0x89: f8_amd(); break;
			case 0x8a: f8_nm(); break;
			case 0x8b: f8_om(); break;
			case 0x8c: f8_xm(); break;
			case 0x8d: f8_cm(); break;
			case 0x8e: f8_adc(); break;
			case 0x8f: f8_br7(); break;

			case 0x90: f8_bf(0x0); break;
			case 0x91: f8_bf(0x1); break;
			case 0x92: f8_bf(0x2); break;
			case 0x93: f8_bf(0x3); break;
			case 0x94: f8_bf(0x4); break;
			case 0x95: f8_bf(0x5); break;
			case 0x96: f8_bf(0x6); break;
			case 0x97: f8_bf(0x7); break;
			case 0x98: f8_bf(0x8); break;
			case 0x99: f8_bf(0x9); break;
			case 0x9a: f8_bf(0xa); break;
			case 0x9b: f8_bf(0xb); break;
			case 0x9c: f8_bf(0xc); break;
			case 0x9d: f8_bf(0xd); break;
			case 0x9e: f8_bf(0xe); break;
			case 0x9f: f8_bf(0xf); break;

			case 0xa0: f8_ins_0(0x0); break;
			case 0xa1: f8_ins_0(0x1); break;
			case 0xa2: illegal(); break;
			case 0xa3: illegal(); break;
			case 0xa4: f8_ins_1(0x4); break;
			case 0xa5: f8_ins_1(0x5); break;
			case 0xa6: f8_ins_1(0x6); break;
			case 0xa7: f8_ins_1(0x7); break;
			case 0xa8: f8_ins_1(0x8); break;
			case 0xa9: f8_ins_1(0x9); break;
			case 0xaa: f8_ins_1(0xa); break;
			case 0xab: f8_ins_1(0xb); break;
			case 0xac: f8_ins_1(0xc); break;
			case 0xad: f8_ins_1(0xd); break;
			case 0xae: f8_ins_1(0xe); break;
			case 0xaf: f8_ins_1(0xf); break;

			case 0xb0: f8_outs_0(0x0); break;
			case 0xb1: f8_outs_0(0x1); break;
			case 0xb2: illegal(); break;
			case 0xb3: illegal(); break;
			case 0xb4: f8_outs_1(0x4); break;
			case 0xb5: f8_outs_1(0x5); break;
			case 0xb6: f8_outs_1(0x6); break;
			case 0xb7: f8_outs_1(0x7); break;
			case 0xb8: f8_outs_1(0x8); break;
			case 0xb9: f8_outs_1(0x9); break;
			case 0xba: f8_outs_1(0xa); break;
			case 0xbb: f8_outs_1(0xb); break;
			case 0xbc: f8_outs_1(0xc); break;
			case 0xbd: f8_outs_1(0xd); break;
			case 0xbe: f8_outs_1(0xe); break;
			case 0xbf: f8_outs_1(0xf); break;

			case 0xc0: f8_as(0x0); break;
			case 0xc1: f8_as(0x1); break;
			case 0xc2: f8_as(0x2); break;
			case 0xc3: f8_as(0x3); break;
			case 0xc4: f8_as(0x4); break;
			case 0xc5: f8_as(0x5); break;
			case 0xc6: f8_as(0x6); break;
			case 0xc7: f8_as(0x7); break;
			case 0xc8: f8_as(0x8); break;
			case 0xc9: f8_as(0x9); break;
			case 0xca: f8_as(0xa); break;
			case 0xcb: f8_as(0xb); break;
			case 0xcc: f8_as_isar(); break;
			case 0xcd: f8_as_isar_i(); break;
			case 0xce: f8_as_isar_d(); break;
			case 0xcf: illegal(); break;

			case 0xd0: f8_asd(0x0); break;
			case 0xd1: f8_asd(0x1); break;
			case 0xd2: f8_asd(0x2); break;
			case 0xd3: f8_asd(0x3); break;
			case 0xd4: f8_asd(0x4); break;
			case 0xd5: f8_asd(0x5); break;
			case 0xd6: f8_asd(0x6); break;
			case 0xd7: f8_asd(0x7); break;
			case 0xd8: f8_asd(0x8); break;
			case 0xd9: f8_asd(0x9); break;
			case 0xda: f8_asd(0xa); break;
			case 0xdb: f8_asd(0xb); break;
			case 0xdc: f8_asd_isar(); break;
			case 0xdd: f8_asd_isar_i(); break;
			case 0xde: f8_asd_isar_d(); break;
			case 0xdf: illegal(); break;

			case 0xe0: f8_xs(0x0); break;
			case 0xe1: f8_xs(0x1); break;
			case 0xe2: f8_xs(0x2); break;
			case 0xe3: f8_xs(0x3); break;
			case 0xe4: f8_xs(0x4); break;
			case 0xe5: f8_xs(0x5); break;
			case 0xe6: f8_xs(0x6); break;
			case 0xe7: f8_xs(0x7); break;
			case 0xe8: f8_xs(0x8); break;
			case 0xe9: f8_xs(0x9); break;
			case 0xea: f8_xs(0xa); break;
			case 0xeb: f8_xs(0xb); break;
			case 0xec: f8_xs_isar(); break;
			case 0xed: f8_xs_isar_i(); break;
			case 0xee: f8_xs_isar_d(); break;
			case 0xef: illegal(); break;

			case 0xf0: f8_ns(0x0); break;
			case 0xf1: f8_ns(0x1); break;
			case 0xf2: f8_ns(0x2); break;
			case 0xf3: f8_ns(0x3); break;
			case 0xf4: f8_ns(0x4); break;
			case 0xf5: f8_ns(0x5); break;
			case 0xf6: f8_ns(0x6); break;
			case 0xf7: f8_ns(0x7); break;
			case 0xf8: f8_ns(0x8); break;
			case 0xf9: f8_ns(0x9); break;
			case 0xfa: f8_ns(0xa); break;
			case 0xfb: f8_ns(0xb); break;
			case 0xfc: f8_ns_isar(); break;
			case 0xfd: f8_ns_isar_i(); break;
			case 0xfe: f8_ns_isar_d(); break;
			case 0xff: illegal(); break;
		}
		switch (op)
		{
			case 0x0c: case 0x1b: case 0x1c: case 0x1d:
			case 0x27: case 0x28: case 0x29:
			case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb:
			case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				// don't handle irq after privileged instruction
				ROMC_00(cS);
				break;

			default:
				// 'freeze cycle' for handling interrupts
				if (m_w&I && m_irq_request)
				{
					ROMC_10();
					ROMC_1C(cL);
					ROMC_0F();
					ROMC_13();
				}

				// fetch next instruction (DS is long cycle)
				if ((op >= 0x30) && (op <= 0x3f))
					ROMC_00(cL);
				else
					ROMC_00(cS);

				break;
		}
	} while( m_icount > 0 );
}
