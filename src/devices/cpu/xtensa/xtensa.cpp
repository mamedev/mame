// license:BSD-3-Clause
// copyright-holders:AJR, David Haywood
/***************************************************************************

    Tensilica Xtensa

    preliminary core, many extensions not supported

***************************************************************************/

#include "emu.h"
#include "xtensa.h"
#include "xtensad.h"

#include "xtensa_helper.h"

#define LOG_UNHANDLED_OPS       (1U << 1)
#define LOG_UNHANDLED_CACHE_OPS (1U << 3)
#define LOG_UNHANDLED_SYNC_OPS  (1U << 4)
#define LOG_EXTREG_OPS          (1U << 8)
#define LOG_NAMED_REGS          (1U << 10)
#define LOG_TIMER_REGS          (1U << 11)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(XTENSA, xtensa_device, "xtensa", "Tensilica Xtensa core")

xtensa_device::xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, XTENSA, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_extregs_config("extregs", ENDIANNESS_LITTLE, 32, 8, -2, address_map_constructor(FUNC(xtensa_device::ext_regs), this))
	, m_pc(0)
{
	for (int i = 0; i < 32; i++)
		m_irq_vectors[i] = 0x00000000;

	m_startupvector = 0x00000000;
}

std::unique_ptr<util::disasm_interface> xtensa_device::create_disassembler()
{
	return std::make_unique<xtensa_disassembler>();
}

device_memory_interface::space_config_vector xtensa_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_config),
		std::make_pair(AS_EXTREGS, &m_extregs_config)
	};
}

/* Exceptions */

u32 xtensa_device::extreg_exccause_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_exccause read\n"); return 0x4; /* m_extreg_exccause;*/ }
void xtensa_device::extreg_exccause_w(u32 data) { m_extreg_exccause = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_exccause set to %08x\n", data); }

u32 xtensa_device::extreg_epc1_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc1 read\n"); return m_extreg_epc1; }
void xtensa_device::extreg_epc1_w(u32 data) { m_extreg_epc1 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc1 set to %08x\n", data); }
u32 xtensa_device::extreg_epc2_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc2 read\n"); return m_extreg_epc2; }
void xtensa_device::extreg_epc2_w(u32 data) { m_extreg_epc2 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc2 set to %08x\n", data); }
u32 xtensa_device::extreg_epc3_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc3 read\n"); return m_extreg_epc3; }
void xtensa_device::extreg_epc3_w(u32 data) { m_extreg_epc3 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc3 set to %08x\n", data); }
u32 xtensa_device::extreg_epc4_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc4 read\n"); return m_extreg_epc4; }
void xtensa_device::extreg_epc4_w(u32 data) { m_extreg_epc4 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc4 set to %08x\n", data); }
u32 xtensa_device::extreg_epc5_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc5 read\n"); return m_extreg_epc5; }
void xtensa_device::extreg_epc5_w(u32 data) { m_extreg_epc5 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_epc5 set to %08x\n", data); }

u32 xtensa_device::extreg_eps2_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps2 read\n"); return m_extreg_eps2; }
void xtensa_device::extreg_eps2_w(u32 data) { m_extreg_eps2 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps2 set to %08x\n", data); }
u32 xtensa_device::extreg_eps3_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps3 read\n"); return m_extreg_eps3; }
void xtensa_device::extreg_eps3_w(u32 data) { m_extreg_eps3 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps3 set to %08x\n", data); }
u32 xtensa_device::extreg_eps4_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps4 read\n"); return m_extreg_eps4; }
void xtensa_device::extreg_eps4_w(u32 data) { m_extreg_eps4 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps4 set to %08x\n", data); }
u32 xtensa_device::extreg_eps5_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps5 read\n"); return m_extreg_eps5; }
void xtensa_device::extreg_eps5_w(u32 data) { m_extreg_eps5 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_eps5 set to %08x\n", data); }

u32 xtensa_device::extreg_excsave1_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave1 read\n"); return m_extreg_excsave1; }
void xtensa_device::extreg_excsave1_w(u32 data) { m_extreg_excsave1 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave1 set to %08x\n", data); }
u32 xtensa_device::extreg_excsave2_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave2 read\n"); return m_extreg_excsave2; }
void xtensa_device::extreg_excsave2_w(u32 data) { m_extreg_excsave2 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave2 set to %08x\n", data); }
u32 xtensa_device::extreg_excsave3_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave3 read\n"); return m_extreg_excsave3; }
void xtensa_device::extreg_excsave3_w(u32 data) { m_extreg_excsave3 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave3 set to %08x\n", data); }
u32 xtensa_device::extreg_excsave4_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave4 read\n"); return m_extreg_excsave4; }
void xtensa_device::extreg_excsave4_w(u32 data) { m_extreg_excsave4 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave4 set to %08x\n", data); }
u32 xtensa_device::extreg_excsave5_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave5 read\n"); return m_extreg_excsave5; }
void xtensa_device::extreg_excsave5_w(u32 data) { m_extreg_excsave5 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_excsave5 set to %08x\n", data); }

/* Interrupts */

u32 xtensa_device::extreg_intenable_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_intenable read\n"); return m_extreg_intenable; }
void xtensa_device::extreg_intenable_w(u32 data) { m_extreg_intenable = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_intenable set to %08x\n", data); }
u32 xtensa_device::extreg_intclr_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_intclr read\n"); return m_extreg_intclr; }
void xtensa_device::extreg_intclr_w(u32 data) { m_extreg_intclr = data; m_extreg_intset &= ~data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_intclr set to %08x\n", data); }
u32 xtensa_device::extreg_intset_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_intset read\n"); return m_extreg_intset; }
void xtensa_device::extreg_intset_w(u32 data) { m_extreg_intset = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_intset set to %08x\n", data); }

/* Window handling */

u32 xtensa_device::extreg_windowbase_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_windowbase read\n"); return m_extreg_windowbase; }
void xtensa_device::extreg_windowbase_w(u32 data) { m_extreg_windowbase = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_windowbase set to %08x\n", data); switch_windowbase(0); }
u32 xtensa_device::extreg_windowstart_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_windowstart read\n"); return m_extreg_windowstart; }
void xtensa_device::extreg_windowstart_w(u32 data) { m_extreg_windowstart = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_windowstart set to %08x\n", data); }

/* General processor state */

u32 xtensa_device::extreg_ps_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_ps read\n"); return m_extreg_ps; }
void xtensa_device::extreg_ps_w(u32 data) { m_extreg_ps = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_ps set to %08x\n", data); }

/* Loop handling */

u32 xtensa_device::extreg_lbeg_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_lbeg read\n"); return m_extreg_lbeg; }
void xtensa_device::extreg_lbeg_w(u32 data) { m_extreg_lbeg = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_lbeg set to %08x\n", data); }
u32 xtensa_device::extreg_lend_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_lend read\n"); return m_extreg_lend; }
void xtensa_device::extreg_lend_w(u32 data) { m_extreg_lend = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_lend set to %08x\n", data); }
u32 xtensa_device::extreg_lcount_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_lcount read\n"); return m_extreg_lcount; }
void xtensa_device::extreg_lcount_w(u32 data) { m_extreg_lcount = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_lcount set to %08x\n", data); }

/* Shifter */

u32 xtensa_device::extreg_sar_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_sar read\n"); return m_extreg_sar; }
void xtensa_device::extreg_sar_w(u32 data) { m_extreg_sar = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_sar set to %08x\n", data); }

/* Debugging */

u32 xtensa_device::extreg_ibreaka0_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_ibreaka0 read\n"); return m_extreg_ibreaka0; }
void xtensa_device::extreg_ibreaka0_w(u32 data) { m_extreg_ibreaka0 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_ibreaka0 set to %08x\n", data); }
u32 xtensa_device::extreg_dbreaka0_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_dbreaka0 read\n"); return m_extreg_dbreaka0; }

void xtensa_device::extreg_dbreaka0_w(u32 data) { m_extreg_dbreaka0 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_dbreaka0 set to %08x\n", data); }
u32 xtensa_device::extreg_dbreakc0_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_dbreakc0 read\n"); return m_extreg_dbreakc0; }
void xtensa_device::extreg_dbreakc0_w(u32 data) { m_extreg_dbreakc0 = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_dbreakc0 set to %08x\n", data); }

u32 xtensa_device::extreg_icountlevel_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_icountlevel read\n"); return m_extreg_icountlevel; }
void xtensa_device::extreg_icountlevel_w(u32 data) { m_extreg_icountlevel = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_icountlevel set to %08x\n", data); }

/* Misc */

u32 xtensa_device::extreg_cacheattr_r() { LOGMASKED(LOG_NAMED_REGS, "m_extreg_cacheattr read\n"); return m_extreg_cacheattr; }
void xtensa_device::extreg_cacheattr_w(u32 data) { m_extreg_cacheattr = data; LOGMASKED(LOG_NAMED_REGS, "m_extreg_cacheattr set to %08x\n", data); }

/* Timer */

u32 xtensa_device::extreg_ccompare0_r()
{
	LOGMASKED(LOG_TIMER_REGS, "m_extreg_ccompare0 read\n");
	return m_extreg_ccompare0;
}

void xtensa_device::extreg_ccompare0_w(u32 data)
{
	// writing this clears the timer interrupt
	m_extreg_ccompare0 = data;
	LOGMASKED(LOG_TIMER_REGS, "m_extreg_ccompare0 set to %08x (clear timer interrupt)\n", data);
}

u32 xtensa_device::extreg_ccount_r()
{
	LOGMASKED(LOG_TIMER_REGS, "m_extreg_ccount read (%08x)\n", m_extreg_ccount);
	return m_extreg_ccount;
}

void xtensa_device::extreg_ccount_w(u32 data)
{
	m_extreg_ccount = data;
	LOGMASKED(LOG_TIMER_REGS, "m_extreg_ccount set to %08x\n", data);
}


void xtensa_device::set_irqpri(u8 val)
{
	m_extreg_ps |= (val & 0xf);
}

void xtensa_device::clear_irqpri(u8 val)
{
	m_extreg_ps &= ~(val & 0xf);
}

u8 xtensa_device::get_irqpri()
{
	return m_extreg_ps & 0xf;
}

void xtensa_device::set_callinc(u8 val)
{
	m_extreg_ps = (m_extreg_ps & 0xfffcffff) | ((val & 3) << 16);
}

u8 xtensa_device::get_callinc()
{
	return (m_extreg_ps >> 16) & 3;
}

void xtensa_device::ext_regs(address_map &map)
{
	// Loop Option (0-2),
	map(0x00, 0x00).rw(FUNC(xtensa_device::extreg_lbeg_r), FUNC(xtensa_device::extreg_lbeg_w)); // "lbeg" LOOP BEGIN
	map(0x01, 0x01).rw(FUNC(xtensa_device::extreg_lend_r), FUNC(xtensa_device::extreg_lend_w)); // "lend" LOOP END
	map(0x02, 0x02).rw(FUNC(xtensa_device::extreg_lcount_r), FUNC(xtensa_device::extreg_lcount_w)); // "lcount" LOOP COUNT

	// Core Architecture (3)
	map(0x03, 0x03).rw(FUNC(xtensa_device::extreg_sar_r), FUNC(xtensa_device::extreg_sar_w)); // "sar" Shift Amount

	// Boolean Option (4)
	//map(0x04, 0x04) // "br",

	// Extended L32R Option (5)
	//map(0x05, 0x05) // "litbase",

	// Conditional Store Option (12)
	//map(0x0c, 0x0c) // "scompare1",

	// MAC16 Option (16-17)
	//map(0x10, 0x10) // "acclo",
	//map(0x11, 0x11) // "acchi",

	// MAC16 Option (32-35)
	//map(0x20, 0x20) // "m0",
	//map(0x21, 0x21) // "m1",
	//map(0x22, 0x22) // "m2",
	//map(0x23, 0x23) // "m3",

	 // Windowed Register Option (72-73)
	map(0x48, 0x48).rw(FUNC(xtensa_device::extreg_windowbase_r), FUNC(xtensa_device::extreg_windowbase_w)); // "WindowBase",
	map(0x49, 0x49).rw(FUNC(xtensa_device::extreg_windowstart_r), FUNC(xtensa_device::extreg_windowstart_w));// "WindowStart",

	// MMU Option (83)
	//map(0x53, 0x53) // "ptevaddr",

	// Trace Port Option (89)
	//map(0x59, 0x59) // "mmid",

	// MMU Option (90-92)
	//map(0x5a, 0x5a) // "rasid",
	//map(0x5b, 0x5b) // "itlbcfg",
	//map(0x5c, 0x5c) // "dtlbcfg",

	// Debug Option (96)
	//map(0x60, 0x60) // "ibreakenable",

	// XEA1 Only (98)
	map(0x62, 0x62).rw(FUNC(xtensa_device::extreg_cacheattr_r), FUNC(xtensa_device::extreg_cacheattr_w));// "cacheattr"

	// Conditional Store Option (99)
	//map(0x63, 0x63) // "atomctl",

	// Debug Option (104)
	//map(0x68, 0x68) // "ddr",

	// Memory ECC/Parity Option (106-111)
	//map(0x6a, 0x6a) // "mepc",
	//map(0x6b, 0x6b) // "meps",
	//map(0x6c, 0x6c) // "mesave",
	//map(0x6d, 0x6d) // "mesr",
	//map(0x6e, 0x6e) // "mecr",
	//map(0x6f, 0x6f) // "mevaddr",

	// Debug Option (128-129)
	map(0x80, 0x80).rw(FUNC(xtensa_device::extreg_ibreaka0_r), FUNC(xtensa_device::extreg_ibreaka0_w));// "ibreaka0"
	//map(0x81, 0x81) // "ibreaka1",

	// Debug Option (144-145)
	map(0x90, 0x90).rw(FUNC(xtensa_device::extreg_dbreaka0_r), FUNC(xtensa_device::extreg_dbreaka0_w));// "dbreaka0"
	//map(0x91, 0x91) // "dbreaka1",

	// Debug Option (160-161)
	map(0xa0, 0xa0).rw(FUNC(xtensa_device::extreg_dbreakc0_r), FUNC(xtensa_device::extreg_dbreakc0_w));// "dbreakc0"
	//map(0xa1, 0xa1) // "dbreakc1",

	// Exception Option (177)
	map(0xb1, 0xb1).rw(FUNC(xtensa_device::extreg_epc1_r), FUNC(xtensa_device::extreg_epc1_w));// "epc1"

	// High-Priority Interrupt Option (178-183)
	map(0xb2, 0xb2).rw(FUNC(xtensa_device::extreg_epc2_r), FUNC(xtensa_device::extreg_epc2_w));// "epc2"
	map(0xb3, 0xb3).rw(FUNC(xtensa_device::extreg_epc3_r), FUNC(xtensa_device::extreg_epc3_w));// "epc3"
	map(0xb4, 0xb4).rw(FUNC(xtensa_device::extreg_epc4_r), FUNC(xtensa_device::extreg_epc4_w));// "epc4"
	map(0xb5, 0xb5).rw(FUNC(xtensa_device::extreg_epc5_r), FUNC(xtensa_device::extreg_epc5_w));// "epc5"
	//map(0xb6, 0xb6) // "epc6",
	//map(0xb7, 0xb7) // "epc7",

	// Exception Option (192)
	//map(0xc0, 0xc0) // "depc",

	// High-Priority Interrupt Option (194-199)
	map(0xc2, 0xc2).rw(FUNC(xtensa_device::extreg_eps2_r), FUNC(xtensa_device::extreg_eps2_w));// "eps2"
	map(0xc3, 0xc3).rw(FUNC(xtensa_device::extreg_eps3_r), FUNC(xtensa_device::extreg_eps3_w));// "eps3"
	map(0xc4, 0xc4).rw(FUNC(xtensa_device::extreg_eps4_r), FUNC(xtensa_device::extreg_eps4_w));// "eps4"
	map(0xc5, 0xc5).rw(FUNC(xtensa_device::extreg_eps5_r), FUNC(xtensa_device::extreg_eps5_w));// "eps5"
	//map(0xc6, 0xc6) // "eps6",
	//map(0xc7, 0xc7) // "eps7",

	// Exception Option (209)
	map(0xd1, 0xd1).rw(FUNC(xtensa_device::extreg_excsave1_r), FUNC(xtensa_device::extreg_excsave1_w));// "excsave1"

	// High-Priority Interrupt Option (210-215)
	map(0xd2, 0xd2).rw(FUNC(xtensa_device::extreg_excsave2_r), FUNC(xtensa_device::extreg_excsave2_w));// "excsave2"
	map(0xd3, 0xd3).rw(FUNC(xtensa_device::extreg_excsave3_r), FUNC(xtensa_device::extreg_excsave3_w));// "excsave3"
	map(0xd4, 0xd4).rw(FUNC(xtensa_device::extreg_excsave4_r), FUNC(xtensa_device::extreg_excsave4_w));// "excsave4"
	map(0xd5, 0xd5).rw(FUNC(xtensa_device::extreg_excsave5_r), FUNC(xtensa_device::extreg_excsave5_w));// "excsave5"
	//map(0xd6, 0xd6) // "excsave6",
	//map(0xd7, 0xd7) // "excsave7",

	// Coprocessor Option (224)
	//map(0xe0, 0xe0) // "cpenable",

	// Interrupt Option (226-228)
	map(0xe2, 0xe2).rw(FUNC(xtensa_device::extreg_intset_r), FUNC(xtensa_device::extreg_intset_w)); // "intset"
	map(0xe3, 0xe3).rw(FUNC(xtensa_device::extreg_intclr_r), FUNC(xtensa_device::extreg_intclr_w)); // "intclr"
	map(0xe4, 0xe4).rw(FUNC(xtensa_device::extreg_intenable_r), FUNC(xtensa_device::extreg_intenable_w)); // "intenable"

	// various options (230)
	map(0xe6, 0xe6).rw(FUNC(xtensa_device::extreg_ps_r), FUNC(xtensa_device::extreg_ps_w)); // "ps" PROCESSOR STATE

	// Relocatable Vector Option (231)
	//map(0xe7, 0xe7) // "vecbase",

	// Exception Option (232)
	map(0xe8, 0xe8).rw(FUNC(xtensa_device::extreg_exccause_r), FUNC(xtensa_device::extreg_exccause_w)); // "exccause"

	// Debug Option (233)
	//map(0xe9, 0xe9) // "debugcause",

	// Timer Interrupt Option (234)
	map(0xea, 0xea).rw(FUNC(xtensa_device::extreg_ccount_r), FUNC(xtensa_device::extreg_ccount_w)); // "ccount"

	// Processor ID Option (235)
	//map(0xeb, 0xeb) // "prid",

	// Debug Option (236-237)
	//map(0xec, 0xec) // "icount",
	map(0xed, 0xed).rw(FUNC(xtensa_device::extreg_icountlevel_r), FUNC(xtensa_device::extreg_icountlevel_w)); // "icountlevel"

	// Exception Option (238)
	//map(0xee, 0xee) // "excvaddr",

	// Timer Interrupt Option (240-242)
	map(0xf0, 0xf0).rw(FUNC(xtensa_device::extreg_ccompare0_r), FUNC(xtensa_device::extreg_ccompare0_w)); // "ccompare0"
	//map(0xf1, 0xf1) // "ccompare1",
	//map(0xf2, 0xf2) // "ccompare2",

	// Miscellaneous Special Registers Option (244-247)
	//map(0xf4, 0xf4) // "misc0",
	//map(0xf5, 0xf5) // "misc1",
	//map(0xf6, 0xf6) // "misc2",
	//map(0xf7, 0xf7) // "misc3",

}


void xtensa_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_space);

	std::fill(std::begin(m_a), std::end(m_a), 0);

	m_num_physical_regs = 2048; // just set this higher than it should be for now, until we emulate window exceptions (marimba startup check suggests 32, with it wrapping around when exceptions are disabled)
	m_a_real.resize(m_num_physical_regs);

	set_icountptr(m_icount);

	state_add(XTENSA_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(XTENSA_WINDOW, "WinBase", m_extreg_windowbase);
	state_add(XTENSA_INTENABLE, "IntEnable", m_extreg_intenable);
	state_add(XTENSA_LOOPBEGIN, "LoopBegin", m_extreg_lbeg);
	state_add(XTENSA_LOOPEND, "LoopEnd", m_extreg_lend);
	state_add(XTENSA_LOOPCOUNT, "LoopCount", m_extreg_lcount);

	for (int i = 0; i < 16; i++)
		state_add(XTENSA_A0 + i, string_format("a%d", i).c_str(), m_a[i]);

	save_item(NAME(m_a_real));
	save_item(NAME(m_a));
	save_item(NAME(m_pc));
	save_item(NAME(m_icount));
	save_item(NAME(m_extreg_windowbase));
	save_item(NAME(m_extreg_windowstart));
	save_item(NAME(m_extreg_sar));
	save_item(NAME(m_extreg_lbeg));
	save_item(NAME(m_extreg_lend));
	save_item(NAME(m_extreg_lcount));
	save_item(NAME(m_extreg_ps));
	save_item(NAME(m_extreg_cacheattr));
	save_item(NAME(m_extreg_epc1));
	save_item(NAME(m_extreg_epc2));
	save_item(NAME(m_extreg_epc3));
	save_item(NAME(m_extreg_epc4));
	save_item(NAME(m_extreg_epc5));
	save_item(NAME(m_extreg_eps2));
	save_item(NAME(m_extreg_eps3));
	save_item(NAME(m_extreg_eps4));
	save_item(NAME(m_extreg_eps5));
	save_item(NAME(m_extreg_excsave1));
	save_item(NAME(m_extreg_excsave2));
	save_item(NAME(m_extreg_excsave3));
	save_item(NAME(m_extreg_excsave4));
	save_item(NAME(m_extreg_excsave5));
	save_item(NAME(m_extreg_ibreaka0));
	save_item(NAME(m_extreg_dbreaka0));
	save_item(NAME(m_extreg_dbreakc0));
	save_item(NAME(m_extreg_icountlevel));
	save_item(NAME(m_extreg_ccompare0));
	save_item(NAME(m_extreg_intenable));
	save_item(NAME(m_extreg_intclr));
	save_item(NAME(m_extreg_intset));
	save_item(NAME(m_extreg_ccount));
	save_item(NAME(m_extreg_exccause));
	save_item(NAME(m_nextpc));
}

void xtensa_device::device_reset()
{
	m_extreg_windowbase = 0;
	m_extreg_windowstart = 0;
	switch_windowbase(0);

	for (int i = 0; i < m_num_physical_regs; i++)
		m_a_real[i] = 0;

	for (int i = 0; i < 16; i++)
		m_a[i] = 0;

	m_extreg_sar = 0;
	m_extreg_lbeg = 0;
	m_extreg_lend = 0;
	m_extreg_lcount = 0;
	m_extreg_ps = 0;
	m_extreg_cacheattr = 0;
	m_extreg_epc1 = 0;
	m_extreg_epc2 = 0;
	m_extreg_epc3 = 0;
	m_extreg_epc4 = 0;
	m_extreg_epc5 = 0;
	m_extreg_eps2 = 0;
	m_extreg_eps3 = 0;
	m_extreg_eps4 = 0;
	m_extreg_eps5 = 0;
	m_extreg_excsave1 = 0;
	m_extreg_excsave2 = 0;
	m_extreg_excsave3 = 0;
	m_extreg_excsave4 = 0;
	m_extreg_excsave5 = 0;
	m_extreg_ibreaka0 = 0;
	m_extreg_dbreaka0 = 0;
	m_extreg_dbreakc0 = 0;
	m_extreg_icountlevel = 0;
	m_extreg_ccompare0 = 0;
	m_extreg_intenable = 0;
	m_extreg_intclr = 0;
	m_extreg_intset = 0;
	m_extreg_ccount = 0;
	m_extreg_exccause = 0;
	m_nextpc = 0;

	m_pc = m_startupvector;
}

void xtensa_device::handle_reserved(u32 inst)
{
	LOGMASKED(LOG_UNHANDLED_OPS, "%-8s0x%02X ; reserved\n", "db", inst & 0xff);
	m_nextpc = m_pc + 1;
}

u32 xtensa_device::get_reg(const u8 reg)
{
	return m_a[reg];
}

void xtensa_device::set_reg(const u8 reg, u32 value)
{
	m_a[reg] = value;
}

void xtensa_device::switch_windowbase(s32 change)
{
	for (int i=0;i<16;i++)
	{
		s32 realreg = (i + m_extreg_windowbase * 4) & (m_num_physical_regs-1);
		m_a_real[realreg] = m_a[i];
	}

	m_extreg_windowbase += change;

	for (int i=0;i<16;i++)
	{
		s32 realreg = (i + m_extreg_windowbase * 4) & (m_num_physical_regs-1);
		m_a[i] = m_a_real[realreg];
	}
}


u32 xtensa_device::get_mem32(u32 addr)
{
	//if (addr & 3)
	//  logerror("get_mem32 unaligned\n");

	return m_space.read_dword(addr & ~3);
}

void xtensa_device::set_mem32(u32 addr, u32 data)
{
	//if (addr & 3)
	//  logerror("set_mem32 unaligned\n");

	m_space.write_dword(addr &~ 3, data);
}

u8 xtensa_device::get_mem8(u32 addr)
{
	return m_space.read_byte(addr);
}

void xtensa_device::set_mem8(u32 addr, u8 data)
{
	m_space.write_byte(addr, data);
}

u16 xtensa_device::get_mem16(u32 addr)
{
	if (addr & 1)
		logerror("get_mem16 unaligned\n");

	return m_space.read_word(addr & ~1);
}

void xtensa_device::set_mem16(u32 addr, u16 data)
{
	if (addr & 1)
		logerror("set_mem16 unaligned\n");

	m_space.write_word(addr & ~1, data);
}

void xtensa_device::handle_retw()
{
	// TODO: exceptions etc.
	u32 addr = get_reg(0);
	u8 xval = (addr >> 30) & 3;
	u32 newaddr = (m_pc & 0xc0000000) | (addr & 0x3fffffff);
	switch_windowbase(-xval);
	m_nextpc = newaddr;
}
bool xtensa_device::handle_bz(u32 inst)
{
	const u8 reg = BIT(inst, 8, 4);
	u32 addr = m_pc + 4 + util::sext(inst >> 12, 12);

	switch (BIT(inst, 6, 2))
	{
	case 0b00:// beqz
	{
		if (get_reg(reg) == 0)
		{
			m_nextpc = addr;
			return true; // avoid loop check
		}
		break;
	}
	case 0b01:// bnez
	{
		if (get_reg(reg) != 0)
		{
			m_nextpc = addr;
			return true; // avoid loop check
		}
		break;
	}
	case 0b10:// bltz
	{
		if (get_reg(reg) & 0x80000000)
		{
			m_nextpc = addr;
			return true; // avoid loop check
		}
		break;
	}
	case 0b11:// bgez
	{
		if (!(get_reg(reg) & 0x80000000))
		{
			m_nextpc = addr;
			return true; // avoid loop check
		}
		break;
	}
	break;
	}

	return false;
}

void xtensa_device::getop_and_execute()
{
	m_nextpc = m_pc + 2;
	u32 inst = m_cache.read_byte(m_pc);
	inst |= m_cache.read_byte(m_pc+1)<<8;

	const u8 op0 = BIT(inst, 0, 4);
	if (op0 < 0b1000)
	{
		inst |= u32(m_cache.read_byte(m_pc+2)) << 16;
		m_nextpc = m_pc + 3;
	}

	switch (op0)
	{
	case 0b0000: // QRST
		switch (BIT(inst, 16, 4))
		{
		case 0b0000: // RST0
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // ST0
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // SNM0
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // ILL
						LOGMASKED(LOG_UNHANDLED_OPS, "ill\n");
						break;

					case 0b1000: // RET
					{
						m_nextpc = get_reg(0);
						m_pc = m_nextpc; return; // avoid loop check
						break;
					}

					case 0b1001: // RETW (with Windowed Register Option)
					{
						handle_retw();
						break;
					}


					case 0b1010: // JX
					{
						const u8 reg = BIT(inst, 8, 4);
						m_nextpc = get_reg(reg);
						m_pc = m_nextpc; return; // avoid loop check
						break;
					}

					case 0b1100: // CALLX0
					{
						const u8 reg = BIT(inst, 8, 4);
						const u32 next = get_reg(reg);
						set_reg(0, m_nextpc);
						m_nextpc = next;
						m_pc = m_nextpc; return; // avoid loop check
						break;
					}

					case 0b1101: // CALLX4 (with Windowed Register Option)
					case 0b1110: // CALLX8 (with Windowed Register Option)
					case 0b1111: // CALLX12 (with Windowed Register Option)
					{
						const u8 reg = BIT(inst, 8, 4);
						const u8 xval = BIT(inst, 4, 2);
						set_callinc(xval);
						const u32 next = get_reg(reg);
						set_reg(xval*4, (m_nextpc & 0x3fffffff) | (xval << 30));
						m_nextpc = next;
						m_pc = m_nextpc; return; // avoid loop check
						break;
					}

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0001: // MOVSP (with Windowed Register Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d", "movsp\n", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // SYNC
					switch (BIT(inst, 4, 8))
					{
					case 0b00000000: // ISYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "isync\n");
						break;

					case 0b00000001: // RSYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "rsync\n");
						break;

					case 0b00000010: // ESYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "esync\n");
						break;

					case 0b00000011: // DSYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "dsync\n");
						break;

					case 0b00001000: // EXCW (with Exception Option)
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "excw\n");
						break;

					case 0b00001100: // MEMW
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "memw\n");
						break;

					case 0b00001101: // EXTW (added in RA-2004.1)
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "extw\n");
						break;

					case 0b00001111: // NOP (added in RA-2004.1; was assembly macro previously)
						// nothing
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0011: // RFEI
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFET
						switch (BIT(inst, 8, 4))
						{
						case 0b0000: // RFE (with Exception Option)
						{
							m_nextpc = m_extreg_epc1;
							clear_irqpri(2);
							clear_irqpri(4);
							break;
						}

						case 0b0001: // RFUE (with Exception Option; XEA1 only)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfue\n");
							break;

						case 0b0010: // RFDE (with Exception Option)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfde\n");
							break;

						case 0b0100: // RFWO (with Windowed Register option)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfwo\n");
							break;

						case 0b0101: // RFWU (with Windowed Register option)
							LOGMASKED(LOG_UNHANDLED_OPS, "rfwu\n");
							break;

						default:
							handle_reserved(inst);
							break;
						}
						break;

					case 0b0001: // RFI (with High-Priority Interrupt Option)
					{
						u8 level = BIT(inst, 8, 4);

						switch (level)
						{
						case 2:
							m_extreg_ps = m_extreg_eps2;
							m_nextpc = m_extreg_epc2;
							break;

						case 3:
							m_extreg_ps = m_extreg_eps3;
							m_nextpc = m_extreg_epc3;
							clear_irqpri(1);
							break;

						case 4:
							m_extreg_ps = m_extreg_eps4;
							m_nextpc = m_extreg_epc4;
							break;

						case 5:
							m_extreg_ps = m_extreg_eps5;
							m_nextpc = m_extreg_epc5;
							break;

						default:
							LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d\n", "rfi", level);
							break;
						}

						break;
					}

					case 0b0010: // RFME (with Memory ECC/Parity Option)
						LOGMASKED(LOG_UNHANDLED_OPS, "rfme\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0100: // BREAK (with Debug Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d, %d\n", "break", BIT(inst, 8, 4), BIT(inst, 4, 4));
					break;

				case 0b0101: // SYSCALL (with Exception Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "syscall\n");
					break;

				case 0b0110: // RSIL (with Interrupt Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, %d\n", "rsil", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0111: // WAITI (with Interrupt Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d\n", "waiti", BIT(inst, 8, 4));
					break;

				case 0b1000: // ANY4 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "any4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // ALL4 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "all4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1010: // ANY8 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "any8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1011: // ALL8 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "all8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0001: // AND
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(dstreg, get_reg(reg_s) & get_reg(reg_t));
				break;
			}
			case 0b0010: // OR / MOV
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(dstreg, get_reg(reg_s) | get_reg(reg_t));
			}
			break;

			case 0b0011: // XOR
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(dstreg, get_reg(reg_s) ^ get_reg(reg_t));
				break;
			}

			case 0b0100: // ST1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // SSR - Set Shift Amount for Right Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = get_reg(srcreg) & 0x1f;
					break;
				}
				case 0b0001: // SSL - Set Shift Amount for Left Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = 32 - (get_reg(srcreg) & 0x1f);
					break;
				}
				case 0b0010: // SSA8L - Set Shift Amount for LE Byte Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = (get_reg(srcreg) & 0x3)<<3;
					break;
				}

				case 0b0011: // SSA8B - Set Shift Amount for BE Byte Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = 32 - ((get_reg(srcreg) & 0x3)<<3);
					break;
				}

				case 0b0100: // SSAI
				{
					const u8 imm = BIT(inst, 8, 4) + (inst & 0x000010);
					m_extreg_sar = imm;
					break;
				}

				case 0b0110: // RER - Read External Register
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", xtensa_helper::s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0111: // WER - Write External Register
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", xtensa_helper::s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1110: // NSA (with Miscellaneous Operations Option) - Normalization Shift Amount
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", xtensa_helper::s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1111: // NSAU (with Miscellaneous Operations Option) - Normalization Shift Amount Unsigned
				{
					const u8 dstreg = BIT(inst, 4, 4);
					const u8 srcreg = BIT(inst, 8, 4);
					u32 result;
					const u32 srcval = get_reg(srcreg);

					if (srcval == 0)
					{
						result = 32;
					}
					else
					{
						result = count_leading_zeros_32(srcval);
					}
					set_reg(dstreg, result);
					break;
				}

				case 0b1000: // ROTW (with Windowed Register Option)
				{
					const s8 imm = util::sext(inst >> 4, 4);
					switch_windowbase(imm);
					break;
				}

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0101: // TLB (with Region Translation Option or MMU Option)
				switch (BIT(inst, 12, 4))
				{
				case 0b0011: case 0b0101: case 0b0110: case 0b0111: // RITLB0, PITLB, WITLB, RITLB1
				case 0b1011: case 0b1101: case 0b1110: case 0b1111: // RDTLB0, PDTLB, WDTLB, RDTLB1
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", xtensa_helper::s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: case 0b1100: // IITLB, IDTLB
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d\n", xtensa_helper::s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0110: // RT0
				switch (BIT(inst, 8, 4))
				{
				case 0b0000: // NEG
				{
					const u8 dstreg = BIT(inst, 12, 4);
					const u8 srcreg = BIT(inst, 4, 4);
					const u32 src = get_reg(srcreg);
					u32 result;
					result = (src ^ 0xffffffff) + 1;
					set_reg(dstreg, result);
					break;
				}

				case 0b0001: // ABS
				{
					const u8 dstreg = BIT(inst, 12, 4);
					const u8 srcreg = BIT(inst, 4, 4);
					const u32 src = get_reg(srcreg);
					u32 result;
					if (src & 0x80000000)
						result = 0x80000000 - (src & 0x7fffffff);
					else
						result = src;
					set_reg(dstreg, result);
					break;
				}
				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000:// ADD
			case 0b1001:// ADDX2
			case 0b1010:// ADDX4
			case 0b1011:// ADDX8
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				const u8 shift = BIT(inst, 20, 2);
				set_reg(dstreg, (get_reg(reg_s)<<shift)+get_reg(reg_t));
				break;
			}

			case 0b1100: // SUB
			case 0b1101: // SUBX2
			case 0b1110: // SUBX4
			case 0b1111: // SUBX8
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				const u8 shift = BIT(inst, 20, 2);
				set_reg(dstreg, (get_reg(reg_s)<<shift)-get_reg(reg_t));
				break;
			}

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0001: // RST1
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // SLLI (shift count is 1..31) - Shift Left Logical Immediate
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 8, 4);
				const u16 shift = 32 - (BIT(inst, 4, 4) + (BIT(inst, 20) ? 16 : 0));
				if (shift == 32)
				{
					// undefined behavior
				}
				else
				{
					set_reg(dstreg, get_reg(srcreg) << shift);
				}
				break;
			}

			case 0b0010: // SRAI (shift count is 0..31) - Shift Right Arithmetic Immediate
			case 0b0011:
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				const u8 amount = BIT(inst, 8, 4) + (BIT(inst, 20) ? 16 : 0);
				const u32 source = get_reg(srcreg);
				u32 result = source >> amount;
				if (source & 0x80000000)
					result |= 0xffffffff << (31 - (amount & 0x1f));

				set_reg(dstreg, result);
				break;
			}

			case 0b0100: // SRLI (shift count is 0..15) - Shift Right Logical Immediate
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				const u8 amount = BIT(inst, 8, 4);
				set_reg(dstreg, get_reg(srcreg) >> amount);
				break;
			}

			case 0b0110: // XSR (added in T1040)
			{
				const u8 spcreg = BIT(inst, 8, 8);
				const u8 reg = BIT(inst, 4, 4);
				const u32 spcregval = space(AS_EXTREGS).read_dword(spcreg);
				const u32 regval = get_reg(reg);
				space(AS_EXTREGS).write_dword(spcreg, regval);
				set_reg(reg, spcregval);
				break;
			}
			case 0b0111: // ACCER (added in RC-2009.0)
				switch (BIT(inst, 20, 4))
				{
				case 0b0000: // RER
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "rer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // WER
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "wer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000: // SRC -  Shift Right Combined
			{
				const u8 reg_r = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				const u64 fullvalue = get_reg(reg_s) | (((u64)get_reg(reg_t))<<32);
				const u32 result = fullvalue >> (m_extreg_sar & 0x1f);
				set_reg(reg_r, result);

				break;
			}

			case 0b1001: // SRL
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				set_reg(dstreg, get_reg(srcreg) >> m_extreg_sar);
				break;
			}
			case 0b1010: // SLL - Shift Left Logical
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 8, 4);
				// we also do the "32 -" part in SSL, which sets m_extreg_sar.  is this correct?
				set_reg(dstreg, get_reg(srcreg) << (32 - m_extreg_sar));
				break;
			}

			case 0b1011: // SRA
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				const u32 source = get_reg(srcreg);
				u32 result = source >> m_extreg_sar;
				if (source & 0x80000000)
					result |= 0xffffffff << (31 - (m_extreg_sar & 0x1f));
				set_reg(dstreg,result);
				break;
			}

			case 0b1100: // MUL16U (with 16-bit Integer Multiply Option)
			{
				const u8 reg_r = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(reg_r, (get_reg(reg_s)&0xffff) * (get_reg(reg_t)&0xffff));
				break;
			}

			case 0b1101: // MUL16S (with 16-bit Integer Multiply Option)
			{
				const u8 reg_r = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(reg_r, s16((get_reg(reg_s)&0xffff)) * s16((get_reg(reg_t)&0xffff)));
				break;
			}

			case 0b1111: // IMP (Implementation-Specific)
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // LICT (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "lict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // SICT (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // LICW (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "licw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0011: // SICW (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sicw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // LDCT (with Data Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "ldct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // SDCT (with Data Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sdct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1110: // RFDX (with On-Chip Debug)
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFDO
						LOGMASKED(LOG_UNHANDLED_OPS, "rfdo\n");
						break;

					case 0b0001: // RFDD
						LOGMASKED(LOG_UNHANDLED_OPS, "rfdd\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // RST2
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: case 0b0100: // ANDB, ANDBC, ORB, ORBC, XORB (with Boolean Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d, b%d\n", xtensa_helper::s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1010: case 0b1011: // MULL, MULUH, MULSH (with 32-bit Integer Multiply Option)
			case 0b1100: case 0b1101: case 0b1110: case 0b1111: // QUOU, QUOS, REMU, REMS (with 32-bit Integer Divide Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", xtensa_helper::s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // RST3
			switch (BIT(inst, 20, 4))
			{
			case 0b0000:// RSR
			{
				const u8 spcreg = BIT(inst, 8, 8);
				const u8 reg = BIT(inst, 4, 4);
				LOGMASKED(LOG_EXTREG_OPS, "%s.%-3d a%d\n", "rsr", xtensa_helper::special_reg(spcreg, BIT(inst, 20)), reg);
				set_reg(reg, space(AS_EXTREGS).read_dword(spcreg));
				break;
			}

			case 0b0001:// WSR
			{
				const u8 spcreg = BIT(inst, 8, 8);
				const u8 reg = BIT(inst, 4, 4);
				LOGMASKED(LOG_EXTREG_OPS, "%s.%-3d a%d\n", "wsr", xtensa_helper::special_reg(spcreg, BIT(inst, 20)), reg);
				space(AS_EXTREGS).write_dword(spcreg, get_reg(reg));
				break;
			}

			case 0b0010: // SEXT (with Miscellaneous Operations Option)
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 8, 4);
				const u8 bit = BIT(inst, 4, 4) + 7;
				set_reg(dstreg, util::sext(get_reg(srcreg), bit+1));
				break;
			}

			case 0b0011: // CLAMPS (with Miscellaneous Operations Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4) + 7);
				break;

			case 0b0100: // MIN (with Miscellaneous Operations Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0101: // MAX (with Miscellaneous Operations Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0110: // MINU (with Miscellaneous Operations Option) - Minimum Value Unsigned
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);

				if (get_reg(reg_s) < get_reg(reg_t))
					set_reg(dstreg, get_reg(reg_s));
				else
					set_reg(dstreg, get_reg(reg_t));

				break;
			}

			case 0b0111: // MAXU (with Miscellaneous Operations Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, a%d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: // MOVEQZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if (get_reg(reg_t) == 0)
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}
			case 0b1001: // MOVNEZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if (get_reg(reg_t) != 0)
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}

			case 0b1010: // MOVLTZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if ((get_reg(reg_t) & 0x80000000))
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}

			case 0b1011: // MOVGEZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if (!(get_reg(reg_t) & 0x80000000))
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}

			case 0b1100: case 0b1101: // MOVF, MOVT (with Boolean Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, b%d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1110: case 0b1111: // RUR, WUR (TODO: TIE user_register names)
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.u%-2d a%d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 4, 8), BIT(inst, 12, 4));
				break;
			}
			break;

		case 0b0100: case 0b0101: // EXTUI
		{
			const u8 dstreg = BIT(inst, 12, 4);
			const u8 srcreg = BIT(inst, 4, 4);
			const u8 shift = BIT(inst, 8, 4) + (BIT(inst, 16) ? 16 : 0);
			const u8 numbits = BIT(inst, 20, 4) + 1;
			set_reg(dstreg, (get_reg(srcreg) >> shift) & ((1 << numbits)-1));
			break;
		}

		case 0b0110: case 0b0111: // CUST0, CUST1
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8s0x%02X ; cust%d?\n", "db", inst & 0xff, BIT(inst, 16));
			m_nextpc = m_pc + 1;
			break;

		case 0b1000: // LSCX (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // LSX
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "lsx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0001: // LSXU
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "lsxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // SSX
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "ssx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0101: // SSXU
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "ssxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1001: // LSC4 (with Windowed Register Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // L32E
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", "l32e", BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			case 0b0100: // S32E
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", "s32e", BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(int(BIT(inst, 12, 4)) * 4 - 64));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // FP0 (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0100: case 0b0101: // ADD.S, SUB.S, MUL.S, MADD.S, MSUB.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, f%d\n", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: case 0b1110: // ROUND.S, TRUNC.S, FLOOR.S, CEIL.S, UTRUNC.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-7s a%d, f%d, %d\n", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // FLOAT.S, UFLOAT.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-7s f%d, a%d, %d\n", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // FP1OP
				switch (BIT(inst, 4, 4))
				{
				case 0b0000: // MOV.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "mov.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // ABS.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "abs.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: // RFR
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, f%d\n", "rfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0101: // WFR
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d\n", "wfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0110: // NEG.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "neg.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1011: // FP1 (with Floating-Point Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0001: case 0b0010: case 0b0011: case 0b0100: case 0b0101: case 0b0110: case 0b0111: // UN.S, OEQ.S, UEQ.S, OLT.S, ULT.S, OLE.S, ULE.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, f%d, f%d\n", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ.S, MOVNEZ.S, MOVLTZ.S, MOVGEZ.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, a%d\n", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF.S, MOVT.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, b%d\n", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0001: // L32R (virtual address is always aligned)
	{
		const u8 reg = BIT(inst, 4, 4);
		const u32 addr = (m_pc + 3 - 0x40000 + (inst >> 8) * 4) & 0xfffffffc;
		set_reg(reg, get_mem32(addr));
		break;
	}

	case 0b0010: // LSAI
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: // L8UI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16);
			set_reg(dstreg, get_mem8(get_reg(basereg) + imm));
			break;
		}

		case 0b0100: // S8I
		{
			const u8 reg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u8 offset = inst >> 16;
			const u32 addr = get_reg(basereg) + offset;
			set_mem8(addr, get_reg(reg)&0xff);
			break;
		}

		case 0b0001: // L16UI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 2;
			set_reg(dstreg, get_mem16(get_reg(basereg) + imm));
			break;
		}

		case 0b0101: // S16I
		{
			const u8 reg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u16 offset = (inst >> 16) * 2;
			const u32 addr = get_reg(basereg) + offset;
			set_mem16(addr, get_reg(reg)&0xffff);
			break;
		}

		case 0b1001: // L16SI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 2;
			u32 value = get_mem16(get_reg(basereg) + imm);
			if (value & 0x00008000)
				value |= 0xffff0000;
			set_reg(dstreg, value);
			break;
		}

		case 0b0010: // L32I
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 4;
			set_reg(dstreg, get_mem32(get_reg(basereg) + imm));
			break;
		}

		case 0b0110: // S32I
		{
			const u8 srcreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 4;
			set_mem32(get_reg(basereg) + imm, get_reg(srcreg));
			break;
		}

		case 0b1011: // L32AI (with Multiprocessor Synchronization Option)
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", xtensa_helper::s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 4));
			break;

		case 0b1111: // S32RI (with Multiprocessor Synchronization Option)
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", xtensa_helper::s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 4));
			break;

		case 0b1110: // S32C1I (with Conditional Store Option)
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, %s\n", xtensa_helper::s_lsai_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 4));
			break;

		case 0b0111: // CACHE
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: // DPFR, DPFW, DPFRO, DPFWO (with Data Cache Option)
			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // DHWB, DHWBI, DHI, DII (with Data Cache Option)
			case 0b1100: case 0b1110: case 0b1111: // IPF, IHI, III (with Instruction Cache Option)
				LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", xtensa_helper::s_cache_ops[BIT(inst, 4, 4)], BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 4));
				break;

			case 0b1000: // DCE (with Data Cache Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // DPFL (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "dpfl", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // DHU (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "dhu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // DIU (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0100: // DIWB (added in T1050)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diwb", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0101: // DIWBI (added in T1050)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diwbi", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;
				}
				break;

			case 0b1101: // ICE (with Instruction Cache Index Lock Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // IPFL
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "ipfl", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // IHU
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "ihu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // IIU
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "iiu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // MOVI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const s32 imm = util::sext((inst & 0x000f00) + (inst >> 16), 12);
			set_reg(dstreg, imm);
			break;
		}

		case 0b1100: // ADDI
		{
			const u8 reg_s = BIT(inst, 8, 4);
			const u8 reg_t = BIT(inst, 4, 4);
			const s32 imm = s8(u8(inst >> 16));
			set_reg(reg_t, get_reg(reg_s)+imm);
			break;
		}

		case 0b1101: // ADDMI
		{
			const u8 reg_s = BIT(inst, 8, 4);
			const u8 reg_t = BIT(inst, 4, 4);
			const s32 imm = s8(u8(inst >> 16)) * 256;
			set_reg(reg_t, get_reg(reg_s)+imm);
			break;
		}

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0011: // LSCI (with Floating-Point Coprocessor Option)
		if (BIT(inst, 12, 2) == 0)
		{
			// LSI, SSI, LSIU, SSIU
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, %s\n", xtensa_helper::s_lsci_ops[BIT(inst, 14, 2)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(BIT(inst, 16, 8) * 4));
			break;
		}
		else
		{
			handle_reserved(inst);
			break;
		}

	case 0b0100: // MAC16 (with MAC16 Option)
		switch (BIT(inst, 20, 4))
		{
		case 0b0000: case 0b0001: // MACID, MACCD
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.dd.%s.%s m%d, a%d, m%d, m%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)],
											xtensa_helper::s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0100: case 0b0101: // MACIA, MACCA
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.da.%s.%s m%d, a%d, m%d, a%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)],
											xtensa_helper::s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // MACDD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.dd.%s m%d, m%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // MACAD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.ad.%s a%d, m%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0110: // MACDA
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.da.%s m%d, a%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0111: // MACAA
			LOGMASKED(LOG_UNHANDLED_OPS, "%s.aa.%s a%d, a%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 4, 4));
			break;

		case 0b1000: case 0b1001: // MACI, MACC
			switch (BIT(inst, 16, 4))
			{
			case 0b0000: // LDINC, LDDEC
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sm%d, a%d\n", BIT(inst, 20) ? "lddec" : "ldinc", BIT(inst, 12, 2), BIT(inst, 8, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0101: // CALLN (target address is always aligned)
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // CALL0
		{
			const u32 addr = (m_pc & 0xfffffffc) + 4 + util::sext(inst >> 6, 18) * 4;
			const u32 next = addr;
			set_reg(0, m_nextpc);
			m_nextpc = next;
			m_pc = m_nextpc; return; // avoid loop check
			break;
		}

		case 0b01: // CALL4 (with Windowed Register Option)
		case 0b10: // CALL8 (with Windowed Register Option)
		case 0b11: // CALL12 (with Windowed Register Option)
		{
			const u32 addr = (m_pc & 0xfffffffc) + 4 + util::sext(inst >> 6, 18) * 4;
			const u8 xval = BIT(inst, 4, 2);
			set_callinc(xval);
			const u32 next = addr;
			set_reg(xval*4, (m_nextpc & 0x3fffffff) | (xval << 30));
			m_nextpc = next;
			m_pc = m_nextpc; return; // avoid loop check
			break;
		}
		}
		break;

	case 0b0110: // SI
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // J
		{
			const u32 newpc = m_pc + 4 + util::sext(inst >> 6, 18);
			m_nextpc = newpc;
			m_pc = m_nextpc; return; // avoid loop check
			break;
		}

		case 0b01: // BZ
		{
			if (handle_bz(inst))
			{
				m_pc = m_nextpc;
				return;
			}

			break;
		}

		case 0b10: // BI0
		{
			const u8 reg = BIT(inst, 8, 4);
			const u32 imm = xtensa_helper::s_b4const[BIT(inst, 12, 4)];
			const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
			const u8 optype = BIT(inst, 6, 2);
			switch (optype)
			{
			case 0b00: // beqi
			{
				if (imm == get_reg(reg))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			}
			case 0b01: // bnei
			{
				if (imm != get_reg(reg))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			}
			case 0b10: // blti
				if ((s32)get_reg(reg) < (s32)imm)
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b11: // bgei
				if ((s32)get_reg(reg) >= (s32)imm)
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			}
			break;
		}

		case 0b11: // BI1
			switch (BIT(inst, 6, 2))
			{
			case 0b00: // ENTRY
			{
				// TODO window exception checking etc.
				const u8 reg = BIT(inst, 8, 4);
				const u32 stacksize = (inst >> 12) << 3;
				u32 stack = get_reg(reg);
				switch_windowbase(get_callinc());
				stack -= stacksize;
				set_reg(reg, stack);
				break;
			}

			case 0b01: // B1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: // BF, BT (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, 0x%08X\n", BIT(inst, 12) ? "bt" : "bf", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1000: // LOOP (with Loop Option)
				{
					const u8 reg = BIT(inst, 8, 4);
					const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
					m_extreg_lcount = get_reg(reg)-1;
					m_extreg_lbeg = m_nextpc;
					m_extreg_lend = addr;
					break;
				}

				case 0b1001: // LOOPNEZ (with Loop Option)
				{
					const u8 reg = BIT(inst, 8, 4);
					const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
					m_extreg_lcount = get_reg(reg)-1;
					m_extreg_lbeg = m_nextpc;
					m_extreg_lend = addr;
					if (!get_reg(reg))
					{
						m_nextpc = m_extreg_lend;
						m_pc = m_nextpc; return; // avoid loop check
					}
					break;
				}

				case 0b1010: // LOOPGTZ (with Loop Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, 0x%08X\n", "loopgtz", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b10: // BLTUI - Branch if Less Than Unsigned Immediate
			{
				const u8 reg = BIT(inst, 8, 4);
				const u32 imm = xtensa_helper::s_b4constu[BIT(inst, 12, 4)];
				const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
				if (get_reg(reg) < imm)
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			}

			case 0b11: // BGEUI - Branch if Greater Than or Eq Unsigned Immediate
			{
				const u8 reg = BIT(inst, 8, 4);
				const u32 imm = xtensa_helper::s_b4constu[BIT(inst, 12, 4)];
				const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
				if (get_reg(reg) >= imm)
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			}
			}
			break;
		}
		break;

	case 0b0111: // B
		if (BIT(inst, 13, 2) == 0b11)
		{
			// BBCI, BBSI
			const u8 reg = BIT(inst, 8, 4);
			const u8 imm =  BIT(inst, 4, 4) + (BIT(inst, 12) ? 16 : 0);
			const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
			if (BIT(inst, 15)) // BBSI
			{
				if ((BIT(get_reg(reg), imm)))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
			}
			else // BBCI
			{
				if (!(BIT(get_reg(reg), imm)))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
			}
			break;
		}
		else
		{
			// BNONE, BEQ, BLT, BLTU, BALL, BBC, BBCI, BANY, BNE, BGE, BGEU, BNALL, BBS
			const u8 as = BIT(inst, 8, 4);
			const u8 at = BIT(inst, 4, 4);
			const u32 addr = m_pc + 4 + s8(u8(inst >> 16));

			switch (BIT(inst, 12, 4))
			{
			case 0b0000:// bnone
				if (!(get_reg(as) & get_reg(at)))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b0001:// beq
				if ((get_reg(as) == get_reg(at)))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b0010:// blt
				if ((s32)get_reg(as) < (s32)get_reg(at))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b0011:// bltu
				if (get_reg(as) < get_reg(at))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b0100:// ball
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "ball", as, at, addr);
				break;
			case 0b0101:// bbc
				if (!(BIT(get_reg(as), get_reg(at)&0x1f)))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}

				break;
			//case 0b0110:// bbci
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbci", as, at, addr);
			//  break;
			//case 0b0111:// bbci
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbci", as, at, addr);
			//  break;
			case 0b1000:// bany
				if (get_reg(as) & get_reg(at))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b1001:// bne
				if ((get_reg(as) != get_reg(at)))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b1010:// bge
				if ((s32)get_reg(as) >= (s32)get_reg(at))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b1011:// bgeu
				if (get_reg(as) >= get_reg(at))
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			case 0b1100:// bnall
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bnall", as, at, addr);
				break;
			case 0b1101:// bbs
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbs", as, at, addr);
				break;
			//case 0b1110:// bbsi
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbsi", as, at, addr);
			//  break;
			//case 0b1111:// bbsih
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbsih", as, at, addr);
			//  break;
			default:
				break;
			}
		}
		break;

	case 0b1000: // L32I.N (with Code Density Option)
	{
		const u8 dstreg = BIT(inst, 4, 4);
		const u8 basereg = BIT(inst, 8, 4);
		const u32 imm = BIT(inst, 12, 4) * 4;
		set_reg(dstreg, get_mem32(get_reg(basereg) + imm));
		break;
	}

	case 0b1001: // S32I.N (with Code Density Option)
	{
		const u8 srcreg = BIT(inst, 4, 4);
		const u8 basereg = BIT(inst, 8, 4);
		const u32 imm = BIT(inst, 12, 4) * 4;
		set_mem32(get_reg(basereg) + imm, get_reg(srcreg));
		break;
	}

	case 0b1010: // ADD.N (with Code Density Option)
	{
		const u8 dstreg = BIT(inst, 12, 4);
		const u8 reg_s = BIT(inst, 8, 4);
		const u8 reg_t = BIT(inst, 4, 4);
		set_reg(dstreg, get_reg(reg_s)+get_reg(reg_t));
		break;
	}

	case 0b1011: // ADDI.N (with Code Density Option)
	{
		const u8 dstreg = BIT(inst, 12, 4);
		const u8 srcreg = BIT(inst, 8, 4);
		const s32 imm = BIT(inst, 4, 4) == 0 ? -1 : int(BIT(inst, 4, 4));
		set_reg(dstreg, get_reg(srcreg)+imm);
		break;
	}

	case 0b1100: // ST2 (with Code Density Option)
		if (!BIT(inst, 7))
		{
			// 7-bit immediate field uses asymmetric sign extension (range is -32..95)
			const u8 dstreg = BIT(inst, 8, 4);
			const s32 imm = int((inst & 0x0070) + BIT(inst, 12, 4) - (BIT(inst, 5, 2) == 0b11 ? 128 : 0));
			set_reg(dstreg,imm);
			break;
		}
		else
		{
			if (BIT(inst, 6))
			{
				// 6-bit immediate field is zero-extended (these forms can branch forward only)
				const u8 reg = BIT(inst, 8, 4);
				const u32 addr = m_pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4);
				if (get_reg(reg) != 0)
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			}
			else
			{
				// 6-bit immediate field is zero-extended (these forms can branch forward only)
				const u8 reg = BIT(inst, 8, 4);
				const u32 addr = m_pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4);
				if (get_reg(reg) == 0)
				{
					m_nextpc = addr;
					m_pc = m_nextpc; return; // avoid loop check
				}
				break;
			}
			break;
		}

	case 0b1101: // ST3 (with Code Density Option)
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: // MOV.N
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 srcreg = BIT(inst, 8, 4);
			set_reg(dstreg, get_reg(srcreg));
			break;
		}

		case 0b1111: // S3
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: // RET.N
			{
				m_nextpc = get_reg(0);
				m_pc = m_nextpc; return; // avoid loop check
				break;
			}

			case 0b0001: // RETW.N (with Windowed Register Option)
			{
				handle_retw();
				break;
			}

			case 0b0010: // BREAK.N (with Debug Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d\n", "break.n", BIT(inst, 8, 4));
				break;

			case 0b0011: // NOP.N
				// nothing
				break;

			case 0b0110: // ILL.N
				LOGMASKED(LOG_UNHANDLED_OPS, "ill.n\n");
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	default:
		handle_reserved(inst);
		break;
	}

	// NOTE, if a branch or jump got us here the loop check isn't done!
	// handle zero overhead loops
	if (m_nextpc == m_extreg_lend)
	{
		m_extreg_lcount--;

		if (m_extreg_lcount != 0xffffffff)
		{
			m_nextpc = m_extreg_lbeg;
		}
	}

	m_pc = m_nextpc;
}

void xtensa_device::check_interrupts()
{
	// TODO: this is not accurate(!)

	if ((m_extreg_intenable & 0x10) && (m_extreg_intset & 0x10) && (get_irqpri() < 1))
	{
		m_extreg_eps3 = m_extreg_ps;
		m_extreg_epc3 = m_nextpc;
		m_pc = m_irq_vectors[4];
		set_irqpri(1);
	}
	else if ((m_extreg_intenable & 0x02) && (m_extreg_intset & 0x02) && (get_irqpri() < 2))
	{
		m_extreg_epc1 = m_nextpc;
		m_pc = m_irq_vectors[1];
		set_irqpri(2);
	}
	else if ((m_extreg_intenable & 0x04) && (m_extreg_intset & 0x04) && (get_irqpri() < 3))
	{
		m_extreg_epc1 = m_nextpc;
		m_pc = m_irq_vectors[1];
		set_irqpri(4);
	}
}


void xtensa_device::execute_run()
{
	while (m_icount > 0)
	{
		check_interrupts();

		debugger_instruction_hook(m_pc);

		m_extreg_ccount++;

		getop_and_execute();
		m_icount--;
	}
}

void xtensa_device::execute_set_input(int inputnum, int state)
{
	if (inputnum == 0x10)
	{
		if (state == ASSERT_LINE)
			m_extreg_intset |= 0x10;
		else
			m_extreg_intset &= ~0x10;
	}

	if (inputnum == 0x2)
	{
		if (state == ASSERT_LINE)
			m_extreg_intset |= 0x02;
		else
			m_extreg_intset &= ~0x02;
	}

	if (inputnum == 0x4)
	{
		if (state == ASSERT_LINE)
			m_extreg_intset |= 0x04;
		else
			m_extreg_intset &= ~0x04;
	}
}
