/*****************************************************************************
 *
 *   Portable Xerox AltoII CPU core
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ALTO2 = &device_creator<alto2_cpu_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  alto2_cpu_device - constructor
//-------------------------------------------------

alto2_cpu_device::alto2_cpu_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	cpu_device(mconfig, ALTO2, "Xerox Alto-II", tag, owner, clock, "alto2", __FILE__),
	/* name, endianness_t endian, datawidth, addrwidth, addrshift = 0, address_map_constructor internal = NULL, address_map_constructor defmap = NULL */
	m_ucode_config("microcode", ENDIANNESS_BIG, 32, 16, 0),
	m_const_config("constants", ENDIANNESS_BIG, 16, 8, 0),
	m_ram_config("memory", ENDIANNESS_BIG, 16, 17, 0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

// FIXME
void alto2_cpu_device::device_start()
{
	m_ucode = &space(AS_PROGRAM);
	m_const = &space(AS_DATA);
	m_ram = &space(AS_IO);

	save_item(NAME(m_task_mpc));
	save_item(NAME(m_task_next2));
	save_item(NAME(m_ntime));
	save_item(NAME(m_task));
	save_item(NAME(m_next_task));
	save_item(NAME(m_next2_task));
	save_item(NAME(m_mpc));
	save_item(NAME(m_mir));
	save_item(NAME(m_rsel));
	save_item(NAME(m_next));
	save_item(NAME(m_next2));
	save_item(NAME(m_r));
	save_item(NAME(m_s));
	save_item(NAME(m_bus));
	save_item(NAME(m_t));
	save_item(NAME(m_alu));
	save_item(NAME(m_aluc0));
	save_item(NAME(m_l));
	save_item(NAME(m_shifter));
	save_item(NAME(m_laluc0));
	save_item(NAME(m_m));
	save_item(NAME(m_cram_addr));
	save_item(NAME(m_task_wakeup));
	save_item(NAME(m_reset_mode));
	save_item(NAME(m_rdram_flag));
	save_item(NAME(m_wrtram_flag));
	save_item(NAME(m_s_reg_bank));
	save_item(NAME(m_bank_reg));
	save_item(NAME(m_ether_enable));
	save_item(NAME(m_ewfct));
	save_item(NAME(m_dsp_time));
	save_item(NAME(m_dsp_state));
	save_item(NAME(m_unload_time));
	save_item(NAME(m_unload_word));
	save_item(NAME(m_mouse.x));
	save_item(NAME(m_mouse.y));
	save_item(NAME(m_mouse.dx));
	save_item(NAME(m_mouse.dy));
	save_item(NAME(m_mouse.latch));
	save_item(NAME(m_unit_selected));
	save_item(NAME(m_head_selected));
	save_item(NAME(m_dsk.drive));
	save_item(NAME(m_dsk.kaddr));
	save_item(NAME(m_dsk.kadr));
	save_item(NAME(m_dsk.kstat));
	save_item(NAME(m_dsk.kcom));
	save_item(NAME(m_dsk.krecno));
	save_item(NAME(m_dsk.shiftin));
	save_item(NAME(m_dsk.shiftout));
	save_item(NAME(m_dsk.datain));
	save_item(NAME(m_dsk.dataout));
	save_item(NAME(m_dsk.krwc));
	save_item(NAME(m_dsk.kfer));
	save_item(NAME(m_dsk.wdtskena));
	save_item(NAME(m_dsk.wdinit0));
	save_item(NAME(m_dsk.wdinit));
	save_item(NAME(m_dsk.strobe));
	save_item(NAME(m_dsk.bitclk));
	save_item(NAME(m_dsk.datin));
	save_item(NAME(m_dsk.bitcount));
	save_item(NAME(m_dsk.carry));
	save_item(NAME(m_dsk.seclate));
	save_item(NAME(m_dsk.seekok));
	save_item(NAME(m_dsk.ok_to_run));
	save_item(NAME(m_dsk.ready_mf31a));
	save_item(NAME(m_dsk.seclate_mf31b));
#if	0
	save_item(NAME(m_dsk.ff_21a));
	save_item(NAME(m_dsk.ff_21a_old));
	save_item(NAME(m_dsk.ff_21b));
	save_item(NAME(m_dsk.ff_22a));
	save_item(NAME(m_dsk.ff_22b));
	save_item(NAME(m_dsk.ff_43b));
	save_item(NAME(m_dsk.ff_53a));
	save_item(NAME(m_dsk.ff_43a));
	save_item(NAME(m_dsk.ff_53b));
	save_item(NAME(m_dsk.ff_44a));
	save_item(NAME(m_dsk.ff_44b));
	save_item(NAME(m_dsk.ff_45a));
	save_item(NAME(m_dsk.ff_45b));
#endif
	save_item(NAME(m_dsp.hlc));
	save_item(NAME(m_dsp.a63));
	save_item(NAME(m_dsp.a66));
	save_item(NAME(m_dsp.setmode));
	save_item(NAME(m_dsp.inverse));
	save_item(NAME(m_dsp.halfclock));
	save_item(NAME(m_dsp.clr));
	save_item(NAME(m_dsp.fifo));
	save_item(NAME(m_dsp.fifo_wr));
	save_item(NAME(m_dsp.fifo_rd));
	save_item(NAME(m_dsp.dht_blocks));
	save_item(NAME(m_dsp.dwt_blocks));
	save_item(NAME(m_dsp.curt_blocks));
	save_item(NAME(m_dsp.curt_wakeup));
	save_item(NAME(m_dsp.vblank));
	save_item(NAME(m_dsp.xpreg));
	save_item(NAME(m_dsp.csr));
	save_item(NAME(m_dsp.curword));
	save_item(NAME(m_dsp.curdata));
	save_item(NAME(m_mem.mar));
	save_item(NAME(m_mem.rmdd));
	save_item(NAME(m_mem.wmdd));
	save_item(NAME(m_mem.md));
	save_item(NAME(m_mem.cycle));
	save_item(NAME(m_mem.access));
	save_item(NAME(m_mem.error));
	save_item(NAME(m_mem.mear));
	save_item(NAME(m_mem.mecr));
	save_item(NAME(m_emu.ir));
	save_item(NAME(m_emu.skip));
	save_item(NAME(m_emu.cy));
	save_item(NAME(m_eth.fifo));
	save_item(NAME(m_eth.fifo_rd));
	save_item(NAME(m_eth.fifo_wr));
	save_item(NAME(m_eth.status));
	save_item(NAME(m_eth.rx_crc));
	save_item(NAME(m_eth.tx_crc));
	save_item(NAME(m_eth.rx_count));
	save_item(NAME(m_eth.tx_count));
	save_item(NAME(m_eth.duckbreath));

	state_add( A2_AC3, "AC(3)", m_r[A2_AC3]).formatstr("%04X");
	state_add( A2_AC2, "AC(2)", m_r[A2_AC2]).formatstr("%04X");
	state_add( A2_AC1, "AC(1)", m_r[A2_AC1]).formatstr("%04X");
	state_add( A2_AC0, "AC(0)", m_r[A2_AC0]).formatstr("%04X");
	state_add( A2_R04, "R04",   m_r[A2_R04]).formatstr("%04X");
	state_add( A2_R05, "R05",   m_r[A2_R05]).formatstr("%04X");
	state_add( A2_PC,  "PC",    m_r[A2_PC]).formatstr("%04X");
	state_add( A2_R07, "R07",   m_r[A2_R07]).formatstr("%04X");
	state_add( A2_R10, "R10",   m_r[A2_R10]).formatstr("%04X");
	state_add( A2_R11, "R11",   m_r[A2_R11]).formatstr("%04X");
	state_add( A2_R12, "R12",   m_r[A2_R12]).formatstr("%04X");
	state_add( A2_R13, "R13",   m_r[A2_R13]).formatstr("%04X");
	state_add( A2_R14, "R14",   m_r[A2_R14]).formatstr("%04X");
	state_add( A2_R15, "R15",   m_r[A2_R15]).formatstr("%04X");
	state_add( A2_R16, "R16",   m_r[A2_R16]).formatstr("%04X");
	state_add( A2_R17, "R17",   m_r[A2_R17]).formatstr("%04X");
	state_add( A2_R20, "R20",   m_r[A2_R20]).formatstr("%04X");
	state_add( A2_R21, "R21",   m_r[A2_R21]).formatstr("%04X");
	state_add( A2_R22, "R22",   m_r[A2_R22]).formatstr("%04X");
	state_add( A2_R23, "R23",   m_r[A2_R23]).formatstr("%04X");
	state_add( A2_R24, "R24",   m_r[A2_R24]).formatstr("%04X");
	state_add( A2_R25, "R25",   m_r[A2_R25]).formatstr("%04X");
	state_add( A2_R26, "R26",   m_r[A2_R26]).formatstr("%04X");
	state_add( A2_R27, "R27",   m_r[A2_R27]).formatstr("%04X");
	state_add( A2_R30, "R30",   m_r[A2_R30]).formatstr("%04X");
	state_add( A2_R31, "R31",   m_r[A2_R31]).formatstr("%04X");
	state_add( A2_R32, "R32",   m_r[A2_R32]).formatstr("%04X");
	state_add( A2_R33, "R33",   m_r[A2_R33]).formatstr("%04X");
	state_add( A2_R34, "R34",   m_r[A2_R34]).formatstr("%04X");
	state_add( A2_R35, "R35",   m_r[A2_R35]).formatstr("%04X");
	state_add( A2_R36, "R36",   m_r[A2_R36]).formatstr("%04X");
	state_add( A2_R37, "R37",   m_r[A2_R37]).formatstr("%04X");

	state_add(STATE_GENPC, "curpc", m_mpc).formatstr("%03X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_aluc0).formatstr("%5s").noshow();

	m_icountptr = &m_icount;

	// reverse dwords and invert hardware specific bits
	for (UINT32 addr = 0; addr < ALTO2_UCODE_PAGE_SIZE; addr++) {
		UINT32 ucode = m_ucode->read_dword(4*addr);
		ucode ^= ALTO2_UCODE_INVERTED;
		m_ucode->write_dword(4*addr, ucode);
	}
	for (UINT32 addr = 0; addr < ALTO2_UCODE_PAGE_SIZE; addr++) {
		UINT32 ucode = m_ucode->read_dword(4*(ALTO2_UCODE_PAGE_SIZE+addr));
		ucode ^= ALTO2_UCODE_INVERTED;
		m_ucode->write_dword(4*(ALTO2_UCODE_PAGE_SIZE+addr), ucode);
	}

	for (UINT32 addr = 0; addr < 256; addr++) {
		printf("%02x: %04x\n", addr, m_const->read_word(addr));
	}

	hard_reset();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

// FIXME
void alto2_cpu_device::device_reset()
{
	soft_reset();
}

//-------------------------------------------------
//  execute_set_input - act on a changed input/
//  interrupt line
//-------------------------------------------------

// FIXME
void alto2_cpu_device::execute_set_input(int inputnum, int state)
{
}

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

// FIXME
void alto2_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
}

// FIXME
#if	ALTO2_DEBUG
void alto2_cpu_device::logprintf(int type, int level, const char* format, ...)
{
}
#endif

// FIXME
void alto2_cpu_device::fatal(int exitcode, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	emu_fatalerror error(exitcode, format, ap);
	va_end(ap);
}

/** @brief task names */
const char* alto2_cpu_device::task_name(int task)
{
	switch (task) {
	case 000: return "emu";
	case 001: return "task01";
	case 002: return "task02";
	case 003: return "task03";
	case 004: return "ksec";
	case 005: return "task05";
	case 006: return "task06";
	case 007: return "ether";
	case 010: return "mrt";
	case 011: return "dwt";
	case 012: return "curt";
	case 013: return "dht";
	case 014: return "dvt";
	case 015: return "part";
	case 016: return "kwd";
	case 017: return "task17";
	}
	return "???";
}

/** @brief register names (as used by the microcode) */
const char* alto2_cpu_device::r_name(UINT8 reg)
{
	switch (reg) {
	case 000: return "ac(3)";
	case 001: return "ac(2)";
	case 002: return "ac(1)";
	case 003: return "ac(0)";
	case 004: return "nww";
	case 005: return "r05";
	case 006: return "pc";
	case 007: return "r07";
	case 010: return "xh";
	case 011: return "r11";
	case 012: return "ecntr";
	case 013: return "epntr";
	case 014: return "r14";
	case 015: return "r15";
	case 016: return "r16";
	case 017: return "r17";
	case 020: return "curx";
	case 021: return "curdata";
	case 022: return "cba";
	case 023: return "aecl";
	case 024: return "slc";
	case 025: return "mtemp";
	case 026: return "htab";
	case 027: return "ypos";
	case 030: return "dwa";
	case 031: return "kwdctw";
	case 032: return "cksumrw";
	case 033: return "knmarw";
	case 034: return "dcbr";
	case 035: return "dwax";
	case 036: return "mask";
	case 037: return "r37";
	}
	return "???";
}

/** @brief ALU function names */
const char* alto2_cpu_device::aluf_name(UINT8 aluf)
{
	switch (aluf) {
	case 000: return "bus";
	case 001: return "t";
	case 002: return "bus or t";
	case 003: return "bus and t";
	case 004: return "bus xor t";
	case 005: return "bus + 1";
	case 006: return "bus - 1";
	case 007: return "bus + t";
	case 010: return "bus - t";
	case 011: return "bus - t - 1";
	case 012: return "bus + t + 1";
	case 013: return "bus + skip";
	case 014: return "bus, t";
	case 015: return "bus and not t";
	case 016: return "0 (undef)";
	case 017: return "0 (undef)";
	}
	return "???";
}

/** @brief BUS source names */
const char* alto2_cpu_device::bs_name(UINT8 bs)
{
	switch (bs) {
	case 000: return "read_r";
	case 001: return "load_r";
	case 002: return "no_source";
	case 003: return "task_3";
	case 004: return "task_4";
	case 005: return "read_md";
	case 006: return "mouse";
	case 007: return "disp";
	}
	return "???";
}

/** @brief F1 function names */
const char* alto2_cpu_device::f1_name(UINT8 f1)
{
	switch (f1) {
	case 000: return "nop";
	case 001: return "load_mar";
	case 002: return "task";
	case 003: return "block";
	case 004: return "l_lsh_1";
	case 005: return "l_rsh_1";
	case 006: return "l_lcy_8";
	case 007: return "const";
	case 010: return "task_10";
	case 011: return "task_11";
	case 012: return "task_12";
	case 013: return "task_13";
	case 014: return "task_14";
	case 015: return "task_15";
	case 016: return "task_16";
	case 017: return "task_17";
	}
	return "???";
}

/** @brief F2 function names */
const char* alto2_cpu_device::f2_name(UINT8 f2)
{
	switch (f2) {
	case 000: return "nop";
	case 001: return "bus=0";
	case 002: return "shifter<0";
	case 003: return "shifter=0";
	case 004: return "bus";
	case 005: return "alucy";
	case 006: return "load_md";
	case 007: return "const";
	case 010: return "task_10";
	case 011: return "task_11";
	case 012: return "task_12";
	case 013: return "task_13";
	case 014: return "task_14";
	case 015: return "task_15";
	case 016: return "task_16";
	case 017: return "task_17";
	}
	return "???";
}

UINT8 cram3k_a37[256];

UINT8 madr_a64[256];

UINT8 madr_a65[256];

/** @brief fatal exit on unitialized dynamic phase BUS source */
void alto2_cpu_device::fn_bs_bad_0()
{
	fatal(9,"fatal: bad early bus source pointer for task %s, mpc:%05o bs:%s\n",
		task_name(m_task), m_mpc, bs_name(MIR_BS(m_mir)));
}

/** @brief fatal exit on unitialized latching phase BUS source */
void alto2_cpu_device::fn_bs_bad_1()
{
	fatal(9,"fatal: bad late bus source pointer for task %s, mpc:%05o bs: %s\n",
		task_name(m_task), m_mpc, bs_name(MIR_BS(m_mir)));
}

/** @brief fatal exit on unitialized dynamic phase F1 function */
void alto2_cpu_device::fn_f1_bad_0()
{
	fatal(9,"fatal: bad early f1 function pointer for task %s, mpc:%05o f1: %s\n",
		task_name(m_task), m_mpc, f1_name(MIR_F1(m_mir)));
}

/** @brief fatal exit on unitialized latching phase F1 function */
void alto2_cpu_device::fn_f1_bad_1()
{
	fatal(9,"fatal: bad late f1 function pointer for task %s, mpc:%05o f1: %s\n",
		task_name(m_task), m_mpc, f1_name(MIR_F1(m_mir)));
}

/** @brief fatal exit on unitialized dynamic phase F2 function */
void alto2_cpu_device::fn_f2_bad_0()
{
	fatal(9,"fatal: bad early f2 function pointer for task %s, mpc:%05o f2: %s\n",
		task_name(m_task), m_mpc, f2_name(MIR_F2(m_mir)));
}

/** @brief fatal exit on unitialized latching phase F2 function */
void alto2_cpu_device::fn_f2_bad_1()
{
	fatal(9,"fatal: bad late f2 function pointer for task %s, mpc:%05o f2: %s\n",
		  task_name(m_task), m_mpc, f2_name(MIR_F2(m_mir)));
}

/**
 * @brief read bank register in memory mapped I/O range
 *
 * The bank registers are stored in a 16x4-bit RAM 74S189.
 */
UINT16 alto2_cpu_device::bank_reg_r(UINT32 address)
{
	int task = address & 017;
	int bank = m_bank_reg[task] | 0177760;
	return bank;
}

/**
 * @brief write bank register in memory mapped I/O range
 *
 * The bank registers are stored in a 16x4-bit RAM 74S189.
 */
void alto2_cpu_device::bank_reg_w(UINT32 address, UINT16 data)
{
	int task = address & 017;
	m_bank_reg[task] = data & 017;
	LOG((0,0,"	write bank[%02o]=%#o normal:%o extended:%o (%s)\n",
		task, data,
		GET_BANK_NORMAL(data),
		GET_BANK_EXTENDED(data),
		task_name(task)));
}

/**
 * @brief bs_read_r early: drive bus by R register
 */
void alto2_cpu_device::bs_read_r_0()
{
	UINT16 r = m_r[m_rsel];
	LOG((0,2,"	<-R%02o; %s (%#o)\n", m_rsel, r_name(m_rsel), r));
	m_bus &= r;
}

/**
 * @brief bs_load_r early: load R places 0 on the BUS
 */
void alto2_cpu_device::bs_load_r_0()
{
	UINT16 r = 0;
	LOG((0,2,"	R%02o<-; %s (BUS&=0)\n", m_rsel, r_name(m_rsel)));
	m_bus &= r;
}

/**
 * @brief bs_load_r late: load R from SHIFTER
 */
void alto2_cpu_device::bs_load_r_1()
{
	if (MIR_F2(m_mir) != f2_emu_load_dns) {
		m_r[m_rsel] = m_shifter;
		LOG((0,2,"	R%02o<-; %s = SHIFTER (%#o)\n", m_rsel, r_name(m_rsel), m_shifter));
		/* HACK: programs writing r37 with xxx3 make the cursor
		 * display go nuts. Until I found the real reason for this
		 * obviously buggy display, I just clear the two
		 * least significant bits of r37 if they are set at once.
		 */
		if (m_rsel == 037 && ((m_shifter & 3) == 3)) {
			printf("writing r37 = %#o\n", m_shifter);
			m_r[037] &= ~3;
		}
	}
}

/**
 * @brief bs_read_md early: drive BUS from read memory data
 */
void alto2_cpu_device::bs_read_md_0()
{
#if	ALTO2_DEBUG
	UINT32 mar = m_mem.mar;
#endif
	UINT16 md = read_mem();
	LOG((0,2,"	<-MD; BUS&=MD (%#o=[%#o])\n", md, mar));
	m_bus &= md;
}

/**
 * @brief bs_mouse early: drive bus by mouse
 */
void alto2_cpu_device::bs_mouse_0()
{
	UINT16 r = mouse_read();
	LOG((0,2,"	<-MOUSE; BUS&=MOUSE (%#o)\n", r));
	m_bus &= r;
}

/**
 * @brief bs_disp early: drive bus by displacement (which?)
 */
void alto2_cpu_device::bs_disp_0()
{
	UINT16 r = 0177777;
	LOG((0,0,"BS <-DISP not handled by task %s mpc:%04x\n", task_name(m_task), m_mpc));
	LOG((0,2,"	<-DISP; BUS&=DISP ?? (%#o)\n", r));
	m_bus &= r;
}

/**
 * @brief f1_load_mar late: load memory address register
 *
 * Load memory address register from the ALU output;
 * start main memory reference (see section 2.3).
 */
void alto2_cpu_device::f1_load_mar_1()
{
	UINT8 bank = m_bank_reg[m_task];
	UINT32 msb;
	if (MIR_F2(m_mir) == f2_load_md) {
		msb = GET_BANK_EXTENDED(bank) << 16;
		LOG((0,7, "	XMAR %#o\n", msb | m_alu));
	} else {
		msb = GET_BANK_NORMAL(bank) << 16;

	}
	load_mar(m_rsel, msb | m_alu);
}

#if	USE_PRIO_F9318
/** @brief F9318 input lines */
typedef enum {
	PRIO_IN_EI = (1<<8),
	PRIO_IN_I7 = (1<<7),
	PRIO_IN_I6 = (1<<6),
	PRIO_IN_I5 = (1<<5),
	PRIO_IN_I4 = (1<<4),
	PRIO_IN_I3 = (1<<3),
	PRIO_IN_I2 = (1<<2),
	PRIO_IN_I1 = (1<<1),
	PRIO_IN_I0 = (1<<0),
	/* masks */
	PRIO_I7 = PRIO_IN_I7,
	PRIO_I6_I7 = (PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I5_I7 = (PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I4_I7 = (PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I3_I7 = (PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I2_I7 = (PRIO_IN_I2 | PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I1_I7 = (PRIO_IN_I1 | PRIO_IN_I2 | PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
	PRIO_I0_I7 = (PRIO_IN_I0 | PRIO_IN_I1 | PRIO_IN_I2 | PRIO_IN_I3 | PRIO_IN_I4 | PRIO_IN_I5 | PRIO_IN_I6 | PRIO_IN_I7),
}	f9318_in_t;

/** @brief F9318 output lines */
typedef enum {
	PRIO_OUT_Q0 = (1<<0),
	PRIO_OUT_Q1 = (1<<1),
	PRIO_OUT_Q2 = (1<<2),
	PRIO_OUT_EO = (1<<3),
	PRIO_OUT_GS = (1<<4),
	/* masks */
	PRIO_OUT_QZ = (PRIO_OUT_Q0 | PRIO_OUT_Q1 | PRIO_OUT_Q2)
}	f9318_out_t;

/**
 * @brief F9318 priority encoder 8 to 3-bit
 *
 * Emulation of the F9318 chip (pin compatible with 74348).
 *
 * <PRE>
 *            F9318
 *         +---+-+---+
 *         |   +-+   |         +---------------------------------+----------------+
 *    I4' -|1      16|-  Vcc   |              input              |     output     |
 *         |         |         +---------------------------------+----------------+
 *    I5' -|2      15|-  EO'   |      EI I0 I1 I2 I3 I4 I5 I6 I7 | GS Q0 Q1 Q2 EO |
 *         |         |         +---------------------------------+----------------+
 *    I6' -|3      14|-  GS'   | (a)  H  x  x  x  x  x  x  x  x  | H  H  H  H  H  |
 *         |         |         | (b)  L  H  H  H  H  H  H  H  H  | H  H  H  H  L  |
 *    I7' -|4      13|-  I3'   +---------------------------------+----------------+
 *         |         |         | (c)  L  x  x  x  x  x  x  x  L  | L  L  L  L  H  |
 *    EI' -|5      12|-  I2'   | (d)  L  x  x  x  x  x  x  L  H  | L  H  L  L  H  |
 *         |         |         | (e)  L  x  x  x  x  x  L  H  H  | L  L  H  L  H  |
 *    Q2' -|6      11|-  I1'   | (f)  L  x  x  x  x  L  H  H  H  | L  H  H  L  H  |
 *         |         |         | (g)  L  x  x  x  L  H  H  H  H  | L  L  L  H  H  |
 *    Q1' -|7      10|-  I0'   | (h)  L  x  x  L  H  H  H  H  H  | L  H  L  H  H  |
 *         |         |         | (i)  L  x  L  H  H  H  H  H  H  | L  L  H  H  H  |
 *   GND  -|8       9|-  Q0'   | (j)  L  L  H  H  H  H  H  H  H  | L  H  H  H  H  |
 *         |         |         +---------------------------------+----------------+
 *         +---------+
 * </PRE>
 */
static __inline f9318_out_t f9318(f9318_in_t in)
{
	f9318_out_t out;

	if (in & PRIO_IN_EI) {
		out = PRIO_OUT_EO | PRIO_OUT_GS | PRIO_OUT_QZ;
		LOG((0,2,"	f9318 case (a) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (0 == (in & PRIO_I7)) {
		out = PRIO_OUT_EO;
		LOG((0,2,"	f9318 case (c) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I7 == (in & PRIO_I6_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0;
		LOG((0,2,"	f9318 case (d) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I6_I7 == (in & PRIO_I5_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q1;
		LOG((0,2,"	f9318 case (e) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I5_I7 == (in & PRIO_I4_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0 | PRIO_OUT_Q1;
		LOG((0,2,"	f9318 case (f) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I4_I7 == (in & PRIO_I3_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q2;
		LOG((0,2,"	f9318 case (g) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I3_I7 == (in & PRIO_I2_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0 | PRIO_OUT_Q2;
		LOG((0,2,"	f9318 case (h) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I2_I7 == (in & PRIO_I1_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q1 | PRIO_OUT_Q2;
		LOG((0,2,"	f9318 case (i) in:%#o out:%#o\n", in, out));
		return out;
	}

	if (PRIO_I1_I7 == (in & PRIO_I0_I7)) {
		out = PRIO_OUT_EO | PRIO_OUT_Q0 | PRIO_OUT_Q1 | PRIO_OUT_Q2;
		LOG((0,2,"	f9318 case (j) in:%#o out:%#o\n", in, out));
		return out;
	}

	out = PRIO_OUT_QZ | PRIO_OUT_GS;
	LOG((0,2,"	f9318 case (b) in:%#o out:%#o\n", in, out));
	return out;
}
#endif

/**
 * @brief f1_task early: task switch
 *
 * The priority encoder finds the highest task requesting service
 * and switches the task number after the next cycle.
 *
 * <PRE>
 *	CT       PROM    NEXT'     RDCT'
 *	1 2 4 8  DATA   6 7 8 9   1 2 4 8
 *	---------------------------------
 *	0 0 0 0  0367   1 1 1 1   0 1 1 1
 *	1 0 0 0  0353   1 1 1 0   1 0 1 1
 *	0 1 0 0  0323   1 1 0 1   0 0 1 1
 *	1 1 0 0  0315   1 1 0 0   1 1 0 1
 *	0 0 1 0  0265   1 0 1 1   0 1 0 1
 *	1 0 1 0  0251   1 0 1 0   1 0 0 1
 *	0 1 1 0  0221   1 0 0 1   0 0 0 1
 *	1 1 1 0  0216   1 0 0 0   1 1 1 0
 *	0 0 0 1  0166   0 1 1 1   0 1 1 0
 *	1 0 0 1  0152   0 1 1 0   1 0 1 0
 *	0 1 0 1  0122   0 1 0 1   0 0 1 0
 *	1 1 0 1  0114   0 1 0 0   1 1 0 0
 *	0 0 1 1  0064   0 0 1 1   0 1 0 0
 *	1 0 1 1  0050   0 0 1 0   1 0 0 0
 *	0 1 1 1  0020   0 0 0 1   0 0 0 0
 *	1 1 1 1  0017   0 0 0 0   1 1 1 1
 *
 * The various task wakeups are encoded using two 8:3-bit priority encoders F9318,
 * which are pin-compatible to the 74348 (inverted inputs and outputs).
 * Their part numbers are U1 and U2.
 * The two encoders are chained (EO of U1 goes to EI of U2):
 *
 * The outputs are fed into some NAND gates (74H10 and 74H00) to decode
 * the task number to latch (CT1-CT4) after a F1 TASK. The case where all
 * of RDCT1' to RDCT8' are high (1) is decoded as RESET'.
 *
 * signal	function
 * --------------------------------------------------
 * CT1		(U1.Q0' & U2.Q0' & RDCT1')'
 * CT2		(U1.Q1' & U2.Q1' & RDCT2')'
 * CT4		(U1.Q2' & U2.Q2' & RDCT4')'
 * CT8		(U1.GS' & RDCT8')'
 * RESET'	RDCT1' & RDCT2' & RDCT4' & RDCT8'
 *
 * In the tables below "x" is RDCTx' of current task
 *
 * signal          input   output, if first 0        CT1  CT2  CT4  CT8
 * ----------------------------------------------------------------------------------------
 * WAKE17' (T19?)   4 I7   Q2:0 Q1:0 Q0:0 GS:0 EO:1  1    1    1    1
 * WAKEKWDT'        3 I6   Q2:0 Q1:0 Q0:1 GS:0 EO:1  x    1    1    1
 * WAKEPART'        2 I5   Q2:0 Q1:1 Q0:0 GS:0 EO:1  1    x    1    1
 * WAKEDVT'         1 I4   Q2:0 Q1:1 Q0:1 GS:0 EO:1  x    x    1    1
 * WAKEDHT'        13 I3   Q2:1 Q1:0 Q0:0 GS:0 EO:1  1    1    x    1
 * WAKECURT'       12 I2   Q2:1 Q1:0 Q0:1 GS:0 EO:1  x    1    x    1
 * WAKEDWT'        11 I1   Q2:1 Q1:1 Q0:0 GS:0 EO:1  1    x    x    1
 * WAKEMRT'        10 I0   Q2:1 Q1:1 Q0:1 GS:0 EO:1  x    x    x    1
 * otherwise               Q2:1 Q1:1 Q0:1 GS:1 EO:0  x    x    x    x
 *
 * signal          input   output, if first 0
 * ----------------------------------------------------------------------------------------
 * WAKEET'          4 I7   Q2:0 Q1:0 Q0:0 GS:0 EO:1  1    1    1    x
 * WAKE6'           3 I6   Q2:0 Q1:0 Q0:1 GS:0 EO:1  x    1    1    x
 * WAKE5'           2 I5   Q2:0 Q1:1 Q0:0 GS:0 EO:1  1    x    1    x
 * WAKEKST'         1 I4   Q2:0 Q1:1 Q0:1 GS:0 EO:1  x    x    1    x
 * WAKE3' (T23?)   13 I3   Q2:1 Q1:0 Q0:0 GS:0 EO:1  1    1    x    x
 * WAKE2'          12 I2   Q2:1 Q1:0 Q0:1 GS:0 EO:1  x    1    x    x
 * WAKE1'          11 I1   Q2:1 Q1:1 Q0:0 GS:0 EO:1  1    x    x    x
 * 0 (GND)         10 I0   Q2:1 Q1:1 Q0:1 GS:0 EO:1  x    x    x    x
 * </PRE>
 */
void alto2_cpu_device::f1_task_0()
{
#if	USE_PRIO_F9318
	/* Doesn't work yet */
	register f9318_in_t wakeup_hi;
	register f9318_out_t u1;
	register f9318_in_t wakeup_lo;
	register f9318_out_t u2;
	register int addr = 017;
	register int rdct1, rdct2, rdct4, rdct8;
	register int ct1, ct2, ct4, ct8;
	register int wakeup, ct;

	LOG((0,2, "	TASK %02o:%s\n", m_task, task_name(m_task)));

	if (m_task > task_emu && (m_task_wakeup & (1 << m_task)))
		addr = m_task;
	LOG((0,2,"	ctl2k_u38[%02o] = %04o\n", addr, ctl2k_u38[addr] & 017));

	rdct1 = (ctl2k_u38[addr] >> U38_RDCT1) & 1;
	rdct2 = (ctl2k_u38[addr] >> U38_RDCT2) & 1;
	rdct4 = (ctl2k_u38[addr] >> U38_RDCT4) & 1;
	rdct8 = (ctl2k_u38[addr] >> U38_RDCT8) & 1;

	/* wakeup signals are active low */
	wakeup = ~m_task_wakeup;

	/* U1
	 * task wakeups 017 to 010 on I7 to I0
	 * EI is 0 (would be 1 at reset)
	 */
	wakeup_hi = (wakeup >> 8) & PRIO_I0_I7;
	u1 = f9318(wakeup_hi);

	/* U2
	 * task wakeups 007 to 001 on I7 to I1, I0 is 0
	 * EO of U1 chained to EI
	 */
	wakeup_lo = wakeup & PRIO_I0_I7;
	if (u1 & PRIO_OUT_EO)
		wakeup_lo |= PRIO_IN_EI;
	u2 = f9318(wakeup_lo);

	/* CT1 = (U1.Q0' & U2.Q0' & RDCT1')' */
	ct1 = !((u1 & PRIO_OUT_Q0) && (u2 & PRIO_OUT_Q0) && rdct1);
	LOG((0,2,"	  CT1:%o U1.Q0':%o U2.Q0':%o RDCT1':%o\n",
		ct1, (u1 & PRIO_OUT_Q0)?1:0, (u2 & PRIO_OUT_Q0)?1:0, rdct1));
	/* CT2 = (U1.Q1' & U2.Q1' & RDCT2')' */
	ct2 = !((u1 & PRIO_OUT_Q1) && (u2 & PRIO_OUT_Q1) && rdct2);
	LOG((0,2,"	  CT2:%o U1.Q1':%o U2.Q1':%o RDCT2':%o\n",
		ct2, (u1 & PRIO_OUT_Q1)?1:0, (u2 & PRIO_OUT_Q1)?1:0, rdct2));
	/* CT4 = (U1.Q2' & U2.Q2' & RDCT4')' */
	ct4 = !((u1 & PRIO_OUT_Q2) && (u2 & PRIO_OUT_Q2) && rdct4);
	LOG((0,2,"	  CT4:%o U1.Q2':%o U2.Q2':%o RDCT4':%o\n",
		ct4, (u1 & PRIO_OUT_Q2)?1:0, (u2 & PRIO_OUT_Q2)?1:0, rdct4));
	/* CT8 */
	ct8 = !((u1 & PRIO_OUT_GS) && rdct8);
	LOG((0,2,"	  CT8:%o U1.GS':%o RDCT8':%o\n",
		ct8, (u1 & PRIO_OUT_GS)?1:0, rdct8));

	ct = 8*ct8 + 4*ct4 + 2*ct2 + ct1;

	if (ct != m_next_task) {
		LOG((0,2, "		switch to %02o\n", ct));
		m_next2_task = ct;
	} else {
		LOG((0,2, "		no switch\n"));
	}
#else	/* USE_PRIO_F9318 */
	int i;

	LOG((0,2, "	TASK %02o:%s", m_task, task_name(m_task)));
	for (i = 15; i >= 0; i--) {
		if (m_task_wakeup & (1 << i)) {
			m_next2_task = i;
			if (m_next2_task != m_next_task) {
				LOG((0,2, " switch to %02o:%s\n", m_next2_task, task_name(m_next2_task)));
			} else {
				LOG((0,2, " no switch\n"));
			}
			return;
		}
	}
	fatal(3, "no tasks requesting service\n");
#endif	/* !USE_PRIO_F9318 */
}

#ifdef	f1_block0_unused
/**
 * @brief f1_block early: block task
 *
 * The task request for the active task is cleared
 */
void alto2_cpu_device::f1_block_0()
{
	CPU_CLR_TASK_WAKEUP(m_task);
	LOG((0,2, "	BLOCK %02o:%s\n", m_task, task_name(m_task)));
}
#endif

/**
 * @brief f2_bus_eq_zero late: branch on bus equals zero
 */
void alto2_cpu_device::f2_bus_eq_zero_1()
{
	UINT16 r = m_bus == 0 ? 1 : 0;
	LOG((0,2, "	BUS=0; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_shifter_lt_zero late: branch on shifter less than zero
 */
void alto2_cpu_device::f2_shifter_lt_zero_1()
{
	UINT16 r = (m_shifter & 0100000) ? 1 : 0;
	LOG((0,2, "	SH<0; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_shifter_eq_zero late: branch on shifter equals zero
 */
void alto2_cpu_device::f2_shifter_eq_zero_1()
{
	UINT16 r = m_shifter == 0 ? 1 : 0;
	LOG((0,2, "	SH=0; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_bus late: branch on bus bits BUS[6-15]
 */
void alto2_cpu_device::f2_bus_1()
{
	UINT16 r = A2_GET32(m_bus,16,6,15);
	LOG((0,2, "	BUS; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_alucy late: branch on latched ALU carry
 */
void alto2_cpu_device::f2_alucy_1()
{
	UINT16 r = m_laluc0;
	LOG((0,2, "	ALUCY; %sbranch (%#o|%#o)\n", r ? "" : "no ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_load_md late: load memory data
 *
 * Deliver BUS data to memory.
 */
void alto2_cpu_device::f2_load_md_1()
{
#if	ALTO2_DEBUG
	UINT16 mar = m_mem.mar;
#endif
	if (MIR_F1(m_mir) == f1_load_mar) {
		/* part of an XMAR */
		LOG((0,2, "	XMAR %#o (%#o)\n", mar, m_bus));
	} else {
		write_mem(m_bus);
		LOG((0,2, "	MD<- BUS ([%#o]=%#o)\n", mar, m_bus));
	}
}

/**
 * @brief read the microcode ROM/RAM halfword
 *
 * Note: HALFSEL is selecting the even (0) or odd (1) half of the
 * microcode RAM 32-bit word. Here's how the demultiplexers (74298)
 * u8, u18, u28 and u38 select the bits:
 *
 *           SN74298
 *         +---+-+---+
 *         |   +-+   |
 *    B2  -|1      16|-  Vcc
 *         |         |
 *    A2  -|2      15|-  QA
 *         |         |
 *    A1  -|3      14|-  QB
 *         |         |
 *    B1  -|4      13|-  QC
 *         |         |
 *    C2  -|5      12|-  QD
 *         |         |
 *    D2  -|6      11|-  CLK
 *         |         |
 *    D1  -|7      10|-  SEL
 *         |         |
 *   GND  -|8       9|-  C1
 *         |         |
 *         +---------+
 *
 *	chip	out pin	BUS	in pin HSEL=0		in pin HSEL=1
 *	--------------------------------------------------------------
 *	u8	QA  15	0	A1   3 DRSEL(0)'	A2   2 DF2(0)
 *	u8	QB  14	1	B1   4 DRSEL(1)'	B2   1 DF2(1)'
 *	u8	QC  13	2	C1   9 DRSEL(2)'	C2   5 DF2(2)'
 *	u8	QD  12	3	D1   7 DRSEL(3)'	D2   6 DF2(3)'
 *
 *	u18	QA  15	4	A1   3 DRSEL(4)'	A2   2 LOADT'
 *	u18	QB  14	5	B1   4 DALUF(0)'	B2   1 LOADL
 *	u18	QC  13	6	C1   9 DALUF(1)'	C2   5 NEXT(00)'
 *	u18	QD  12	7	D1   7 DALUF(2)'	D2   6 NEXT(01)'
 *
 *	u28	QA  15	8	A1   3 DALUF(3)'	A2   2 NEXT(02)'
 *	u28	QB  14	9	B1   4 DBS(0)'		B2   1 NEXT(03)'
 *	u28	QC  13	10	C1   9 DBS(1)'		C2   5 NEXT(04)'
 *	u28	QD  12	11	D1   7 DBS(2)'		D2   6 NEXT(05)'
 *
 *	u38	QA  15	12	A1   3 DF1(0)		A2   2 NEXT(06)'
 *	u38	QB  14	13	B1   4 DF1(1)'		B2   1 NEXT(07)'
 *	u38	QC  13	14	C1   9 DF1(2)'		C2   5 NEXT(08)'
 *	u38	QD  12	15	D1   7 DF1(3)'		D2   6 NEXT(09)'
 *
 * The HALFSEL signal to the demultiplexers is the inverted bit BUS(5):
 * BUS(5)=1, HALFSEL=0, A1,B1,C1,D1 inputs, upper half of the 32-bit word
 * BUS(5)=0, HALFSEL=1, A2,B2,C2,D2 inputs, lower half of the 32-bit word
 */
void alto2_cpu_device::rdram()
{
	UINT32 addr, val;
	UINT32 bank = GET_CRAM_BANKSEL(m_cram_addr) % ALTO2_UCODE_RAM_PAGES;
	UINT32 wordaddr = GET_CRAM_WORDADDR(m_cram_addr);

	m_rdram_flag = 0;
	if (GET_CRAM_RAMROM(m_cram_addr)) {
		/* read ROM 0 at current mpc */
		addr = m_mpc & 01777;
		LOG((0,0,"	rdram: ROM [%05o] ", addr));
	} else {
		/* read RAM 0,1,2 */
		addr = ALTO2_UCODE_RAM_BASE + bank * ALTO2_UCODE_PAGE_SIZE + wordaddr;
		LOG((0,0,"	rdram: RAM%d [%04o] ", bank, wordaddr));
	}

	if (addr >= ALTO2_UCODE_SIZE) {
		val = 0177777;	/* ??? */
		LOG((0,0,"invalid address (%06o)\n", val));
		return;
	}
	val = m_ucode->read_dword(addr) ^ ALTO2_UCODE_INVERTED;
	if (GET_CRAM_HALFSEL(m_cram_addr)) {
		val = val >> 16;
		LOG((0,0,"upper:%06o\n", val));
	} else {
		val = val & 0177777;
		LOG((0,0,"lower:%06o\n", val));
	}
	m_bus &= val;
}

/**
 * @brief write the microcode RAM from M register and ALU
 *
 * Note: M is a latch (MYL, i.e. memory L) on the CRAM board that latches
 * the ALU whenever LOADL and GOODTASK are met. GOODTASK is the Emulator
 * task and something I have not yet found out about: TASKA' and TASKB'.
 *
 * There's also an undumped PROM u21 which is addressed by GOODTASK and
 * 7 other signals...
 */
void alto2_cpu_device::wrtram()
{
	UINT32 addr;
	UINT32 bank = GET_CRAM_BANKSEL(m_cram_addr) % ALTO2_UCODE_RAM_PAGES;
	UINT32 wordaddr = GET_CRAM_WORDADDR(m_cram_addr);

	m_wrtram_flag = 0;

	/* write RAM 0,1,2 */
	addr = ALTO2_UCODE_RAM_BASE + bank * ALTO2_UCODE_PAGE_SIZE + wordaddr;
	LOG((0,0,"	wrtram: RAM%d [%04o] upper:%06o lower:%06o", bank, wordaddr, m_m, m_alu));
	if (addr >= ALTO2_UCODE_SIZE) {
		LOG((0,0," invalid address\n"));
		return;
	}
	LOG((0,0,"\n"));
	m_ucode->write_dword(addr, ((m_m << 16) | m_alu) ^ ALTO2_UCODE_INVERTED);
}

#if	USE_ALU_74181
/**
 * <PRE>
 * Functional description of the 4-bit ALU 74181
 *
 * The 74181 is a 4-bit high speed parallel Arithmetic Logic Unit (ALU).
 * Controlled by four Function Select inputs (S0-S3) and the Mode Control
 * input (M), it can perform all the 16 possible logic operations or 16
 * different arithmetic operations on active HIGH or active LOW operands.
 * The Function Table lists these operations.
 *
 * When the Mode Control input (M) is HIGH, all internal carries are
 * inhibited and the device performs logic operations on the individual
 * bits as listed. When the Mode Control input is LOW, the carries are
 * enabled and the device performs arithmetic operations on the two 4-bit
 * words. The device incorporates full internal carry lookahead and
 * provides for either ripple carry between devices using the Cn+4 output,
 * or for carry lookahead between packages using the signals P' (Carry
 * Propagate) and G' (Carry Generate). In the ADD mode, P' indicates that
 * F' is 15 or more, while G' indicates that F' is 16 or more. In the
 * SUBTRACT mode, P' indicates that F' is zero or less, while G' indicates
 * that F' is less than zero. P' and G' are not affected by carry in.
 * When speed requirements are not stringent, it can be used in a simple
 * ripple carry mode by connecting the Carry output (Cn+4) signal to the
 * Carry input (Cn) of the next unit. For high speed operation the device
 * is used in conjunction with the 74182 carry lookahead circuit. One
 * carry lookahead package is required for each group of four 74181 devices.
 * Carry lookahead can be provided at various levels and offers high speed
 * capability over extremely long word lengths.
 *
 * The A=B output from the device goes HIGH when all four F' outputs are
 * HIGH and can be used to indicate logic equivalence over four bits when
 * the unit is in the subtract mode. The A=B output is open collector and
 * can be wired-AND with other A=B outputs to give a comparison for more
 * than four bits. The A=B signal can also be used with the Cn+4 signal
 * to indicated A>B and A<B.
 *
 * The Function Table lists the arithmetic operations that are performed
 * without a carry in. An incoming carry adds a one to each operation.
 * Thus, select code 0110 generates A minus B minus 1 (2's complement
 * notation) without a carry in and generates A minus B when a carry is
 * applied. Because subtraction is actually performed by the complementary
 * addition (1's complement), a carry out means borrow; thus a carry is
 * generated when there is no underflow and no carry is generated when
 * there is underflow. As indicated, this device can be used with either
 * active LOW inputs producing active LOW outputs or with active HIGH
 * inputs producing active HIGH outputs. For either case the table lists
 * the operations that are performed to the operands labeled inside the
 * logic symbol.
 *
 * The AltoI/II use four 74181s and a 74182 carry lookahead circuit,
 * and the inputs and outputs are all active HIGH.
 *
 * Active HIGH operands:
 *
 * +-------------------+-------------+------------------------+------------------------+
 * |    Mode Select    |   Logic     | Arithmetic w/o carry   | Arithmetic w/ carry    |
 * |      Inputs       |             |                        |                        |
 * |  S3  S2  S1  S0   |   (M=1)     | (M=0) (Cn=1)           | (M=0) (Cn=0)           |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   0   0   | A'          | A                      | A + 1                  |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   0   1   | A' | B'     | A | B                  | (A | B) + 1            |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   1   0   | A' & B      | A | B'                 | (A | B') + 1           |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   0   1   1   | logic 0     | - 1                    | -1 + 1                 |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   0   0   | (A & B)'    | A + (A & B')           | A + (A & B') + 1       |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   0   1   | B'          | (A | B) + (A & B')     | (A | B) + (A & B') + 1 |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   1   0   | A ^ B       | A - B - 1              | A - B - 1 + 1          |
 * +-------------------+-------------+------------------------+------------------------+
 * |   0   1   1   1   | A & B'      | (A & B) - 1            | (A & B) - 1 + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   0   0   | A' | B      | A + (A & B)            | A + (A & B) + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   0   1   | A' ^ B'     | A + B                  | A + B + 1              |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   1   0   | B           | (A | B') + (A & B)     | (A | B') + (A & B) + 1 |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   0   1   1   | A & B       | (A & B) - 1            | (A & B) - 1 + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   0   0   | logic 1     | A + A                  | A + A + 1              |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   0   1   | A | B'      | (A | B) + A            | (A | B) + A + 1        |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   1   0   | A | B       | (A | B') + A           | (A | B') + A + 1       |
 * +-------------------+-------------+------------------------+------------------------+
 * |   1   1   1   1   | A           | A - 1                  | A - 1 + 1              |
 * +-------------------+-------------+------------------------+------------------------+
 * </PRE>
 */
#define	SMC(s3,s2,s1,s0,m,c) (32*(s3)+16*(s2)+8*(s1)+4*(s0)+2*(m)+(c))

/**
 * @brief Compute the 74181 ALU operation smc
 * @param smc S function, arithmetic/logic flag, carry
 * @return resulting ALU output
 */
UINT32 alto2_cpu_device::alu_74181(UINT32 smc)
{
	register UINT32 a = m_bus;
	register UINT32 b = m_t;
	register UINT32 s = 0;
	register UINT32 f = 0;

	switch (smc) {
	case SMC(0,0,0,0, 0, 0): // 0000: A + 1
		s = 0;
		f = a + 1;
		break;

	case SMC(0,0,0,0, 0, 1): // 0000: A
		s = 0;
		f = a;
		break;

	case SMC(0,0,0,1, 0, 0): // 0001: (A | B) + 1
		s = 0;
		f = (a | b) + 1;
		break;

	case SMC(0,0,0,1, 0, 1): // 0001: A | B
		s = 0;
		f = a | b;
		break;

	case SMC(0,0,1,0, 0, 0): // 0010: (A | B') + 1
		s = 0;
		f = (a | ~b) + 1;
		break;

	case SMC(0,0,1,0, 0, 1): // 0010: A | B'
		s = 0;
		f = a | ~b;
		break;

	case SMC(0,0,1,1, 0, 0): // 0011: -1 + 1
		s = 1;
		f = -1 + 1;
		break;

	case SMC(0,0,1,1, 0, 1): // 0011: -1
		s = 1;
		f = -1;
		break;

	case SMC(0,1,0,0, 0, 0): // 0100: A + (A & B') + 1
		s = 0;
		f = a + (a & ~b) + 1;
		break;

	case SMC(0,1,0,0, 0, 1): // 0100: A + (A & B')
		s = 0;
		f = a + (a & ~b);
		break;

	case SMC(0,1,0,1, 0, 0): // 0101: (A | B) + (A & B') + 1
		s = 0;
		f = (a | b) + (a & ~b) + 1;
		break;

	case SMC(0,1,0,1, 0, 1): // 0101: (A | B) + (A & B')
		s = 0;
		f = (a | b) + (a & ~b);
		break;

	case SMC(0,1,1,0, 0, 0): // 0110: A - B - 1 + 1
		s = 1;
		f = a - b - 1 + 1;
		break;

	case SMC(0,1,1,0, 0, 1): // 0110: A - B - 1
		s = 1;
		f = a - b - 1;
		break;

	case SMC(0,1,1,1, 0, 0): // 0111: (A & B) - 1 + 1
		s = 1;
		f = (a & b) - 1 + 1;
		break;

	case SMC(0,1,1,1, 0, 1): // 0111: (A & B) - 1
		s = 1;
		f = (a & b) - 1;
		break;

	case SMC(1,0,0,0, 0, 0): // 1000: A + (A & B) + 1
		s = 0;
		f = a + (a & b) + 1;
		break;

	case SMC(1,0,0,0, 0, 1): // 1000: A + (A & B)
		s = 0;
		f = a + (a & b);
		break;

	case SMC(1,0,0,1, 0, 0): // 1001: A + B + 1
		s = 0;
		f = a + b + 1;
		break;

	case SMC(1,0,0,1, 0, 1): // 1001: A + B
		s = 0;
		f = a + b;
		break;

	case SMC(1,0,1,0, 0, 0): // 1010: (A | B') + (A & B) + 1
		s = 0;
		f = (a | ~b) + (a & b) + 1;
		break;

	case SMC(1,0,1,0, 0, 1): // 1010: (A | B') + (A & B)
		s = 0;
		f = (a | ~b) + (a & b);
		break;

	case SMC(1,0,1,1, 0, 0): // 1011: (A & B) - 1 + 1
		s = 1;
		f = (a & b) - 1 + 1;
		break;

	case SMC(1,0,1,1, 0, 1): // 1011: (A & B) - 1
		s = 1;
		f = (a & b) - 1;
		break;

	case SMC(1,1,0,0, 0, 0): // 1100: A + A + 1
		s = 0;
		f = a + a + 1;
		break;

	case SMC(1,1,0,0, 0, 1): // 1100: A + A
		s = 0;
		f = a + a;
		break;

	case SMC(1,1,0,1, 0, 0): // 1101: (A | B) + A + 1
		s = 0;
		f = (a | b) + a + 1;
		break;

	case SMC(1,1,0,1, 0, 1): // 1101: (A | B) + A
		s = 0;
		f = (a | b) + a;
		break;

	case SMC(1,1,1,0, 0, 0): // 1110: (A | B') + A + 1
		s = 0;
		f = (a | ~b) + a + 1;
		break;

	case SMC(1,1,1,0, 0, 1): // 1110: (A | B') + A
		s = 0;
		f = (a | ~b) + a;
		break;

	case SMC(1,1,1,1, 0, 0): // 1111: A - 1 + 1
		s = 1;
		f = a - 1 + 1;
		break;

	case SMC(1,1,1,1, 0, 1): // 1111: A - 1
		s = 1;
		f = a - 1;
		break;

	case SMC(0,0,0,0, 1, 0): // 0000: A'
	case SMC(0,0,0,0, 1, 1):
		f = ~a;
		break;

	case SMC(0,0,0,1, 1, 0): // 0001: A' | B'
	case SMC(0,0,0,1, 1, 1):
		f = ~a | ~b;
		break;

	case SMC(0,0,1,0, 1, 0): // 0010: A' & B
	case SMC(0,0,1,0, 1, 1):
		f = ~a & b;
		break;

	case SMC(0,0,1,1, 1, 0): // 0011: logic 0
	case SMC(0,0,1,1, 1, 1):
		f = 0;
		break;

	case SMC(0,1,0,0, 1, 0): // 0100: (A & B)'
	case SMC(0,1,0,0, 1, 1):
		f = ~(a & b);
		break;

	case SMC(0,1,0,1, 1, 0): // 0101: B'
	case SMC(0,1,0,1, 1, 1):
		f = ~b;
		break;

	case SMC(0,1,1,0, 1, 0): // 0110: A ^ B
	case SMC(0,1,1,0, 1, 1):
		f = a ^ b;
		break;

	case SMC(0,1,1,1, 1, 0): // 0111: A & B'
	case SMC(0,1,1,1, 1, 1):
		f = a & ~b;
		break;

	case SMC(1,0,0,0, 1, 0): // 1000: A' | B
	case SMC(1,0,0,0, 1, 1):
		f = ~a | b;
		break;

	case SMC(1,0,0,1, 1, 0): // 1001: A' ^ B'
	case SMC(1,0,0,1, 1, 1):
		f = ~a ^ ~b;
		break;

	case SMC(1,0,1,0, 1, 0): // 1010: B
	case SMC(1,0,1,0, 1, 1):
		f = b;
		break;

	case SMC(1,0,1,1, 1, 0): // 1011: A & B
	case SMC(1,0,1,1, 1, 1):
		f = a & b;
		break;

	case SMC(1,1,0,0, 1, 0): // 1100: logic 1
	case SMC(1,1,0,0, 1, 1):
		f = ~0;
		break;

	case SMC(1,1,0,1, 1, 0): // 1101: A | B'
	case SMC(1,1,0,1, 1, 1):
		f = a | ~b;
		break;

	case SMC(1,1,1,0, 1, 0): // 1110: A | B
	case SMC(1,1,1,0, 1, 1):
		f = a | b;
		break;

	case SMC(1,1,1,1, 1, 0): // 1111: A
	case SMC(1,1,1,1, 1, 1):
		f = a;
		break;
	}
	if (smc & 2) {
		m_aluc0 = ((f >> 16) ^ s) & 1;
	} else {
		m_aluc0 = 1;
	}
	return f;
}
#endif

/** @brief flag that tells whether to load the T register from BUS or ALU */
#define	TSELECT	1

/** @brief flag that tells wheter operation was 0: logic (M=1) or 1: arithmetic (M=0) */
#define	ALUM2	2

/** @brief execute the CPU for at most nsecs nano seconds */
void alto2_cpu_device::execute_run()
{
	m_alto_leave = 0;

	m_next = m_task_mpc[m_task];		// get current task's next mpc and address modifier
	m_next2 = m_task_next2[m_task];

	do {
		int do_bs, flags;
		UINT32 alu;
		UINT8 aluf;
		UINT8 bs;
		UINT8 f1;
		UINT8 f2;

		// FIXME: cycle counting?
		if (m_alto_leave)
			break;

		/*
		 * Subtract the microcycle time from the display time accu.
		 * If it underflows, call the display state machine and add
		 * the time for 24 pixel clocks to the accu.
		 * This is very close to every seventh CPU cycle.
		 */
		m_dsp_time -= ALTO2_UCYCLE;
		if (m_dsp_time < 0) {
			m_dsp_state = display_state_machine(m_dsp_state);
			m_dsp_time += ALTO2_DISPLAY_BITTIME(24);
		}
		if (m_unload_time >= 0) {
			/*
			 * Subtract the microcycle time from the unload time accu.
			 * If it underflows call the unload word function which adds
			 * the time for 16 or 32 pixel clocks to the accu, or ends
			 * the unloading by leaving m_unload_time at -1.
			 */
			m_unload_time -= ALTO2_UCYCLE;
			if (m_unload_time < 0) {
				m_unload_word = unload_word(m_unload_word);
			}
		}

		m_cycle++;
		/* nano seconds per cycle */
		m_ntime[m_task] += ALTO2_UCYCLE;

		/* next instruction's mpc */
		m_mpc = m_next;
		debugger_instruction_hook(this, m_next);
		m_mir = m_ucode->read_dword(m_mpc);
		m_rsel = MIR_RSEL(m_mir);
		m_next = MIR_NEXT(m_mir) | m_next2;
		m_next2 = A2_GET32(m_ucode->read_dword(m_next), 32, NEXT0, NEXT9) | (m_next2 & ~ALTO2_UCODE_PAGE_MASK);
		aluf = MIR_ALUF(m_mir);
		bs = MIR_BS(m_mir);
		f1 = MIR_F1(m_mir);
		f2 = MIR_F2(m_mir);
		LOG((0,2,"\n%s-%04o: r:%02o af:%02o bs:%02o f1:%02o f2:%02o t:%o l:%o next:%05o next2:%05o cycle:%lld\n",
			task_name(m_task), m_mpc, m_rsel, aluf, bs, f1, f2, MIR_T(m_mir), MIR_L(m_mir), m_next, m_next2, cycle()));

		/*
		 * This bus source decoding is not performed if f1 = 7 or f2 = 7.
		 * These functions use the BS field to provide part of the address
		 * to the constant ROM
		 */
		do_bs = !(f1 == f1_const || f2 == f2_const);

		if (f1 == f1_load_mar) {
			if (check_mem_load_mar_stall(m_rsel)) {
				LOG((0,3, "	MAR<- stall\n"));
				m_next2 = m_next;
				m_next = m_mpc;
				continue;
			}
		} else if (f2 == f2_load_md) {
			if (check_mem_write_stall()) {
				LOG((0,3, "	MD<- stall\n"));
				m_next2 = m_next;
				m_next = m_mpc;
				continue;
			}
		}
		if (do_bs && bs == bs_read_md) {
			if (check_mem_read_stall()) {
				LOG((0,3, "	<-MD stall\n"));
				m_next2 = m_next;
				m_next = m_mpc;
				continue;
			}
		}

		m_bus = 0177777;

		if (m_rdram_flag)
			rdram();

		/*
		 * The constant memory is gated to the bus by F1 = 7, F2 = 7, or BS >= 4
		 */
		if (!do_bs || bs >= 4) {
			int addr = 8 * m_rsel + bs;
			LOG((0,2,"	%#o; BUS &= CONST[%03o]\n", m_const_prom[addr], addr));
			m_bus &= m_const_prom[addr];
		}

		/*
		 * early f2 has to be done before early bs, because the
		 * emulator f2 acsource or acdest may change rsel
		 */
		if (m_f2[0][m_task][f2])
			((*this).*m_f2[0][m_task][f2])();

		/*
		 * early bs can be done now
		 */
		if (do_bs)
			if (m_bs[0][m_task][bs])
				((*this).*m_bs[0][m_task][bs])();

		/*
		 * early f1
		 */
		if (m_f1[0][m_task][f1])
			((*this).*m_f1[0][m_task][f1])();

		/* compute the ALU function */
		switch (aluf) {
		/**
		 * 00: ALU <- BUS
		 * PROM data for S3-0:1111 M:1 C:0
		 * 74181 function F=A
		 * T source is ALU
		 */
		case aluf_bus__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,1,1,1, 1, 0));
#else
			alu = m_bus;
			m_aluc0 = 1;
#endif
			flags = TSELECT;
			LOG((0,2,"	ALU<- BUS (%#o := %#o)\n", alu, m_bus));
			break;

		/**
		 * 01: ALU <- T
		 * PROM data for S3-0:1010 M:1 C:0
		 * 74181 function F=B
		 * T source is BUS
		 */
		case aluf_treg:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,1,0, 1, 0));
#else
			alu = m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((0,2,"	ALU<- T (%#o := %#o)\n", alu, m_t));
			break;

		/**
		 * 02: ALU <- BUS | T
		 * PROM data for S3-0:1110 M:1 C:0
		 * 74181 function F=A|B
		 * T source is ALU
		 */
		case aluf_bus_or_t__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,1,1,0, 1, 0));
#else
			alu = m_bus | m_t;
			m_aluc0 = 1;
#endif
			flags = TSELECT;
			LOG((0,2,"	ALU<- BUS OR T (%#o := %#o | %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 03: ALU <- BUS & T
		 * PROM data for S3-0:1011 M:1 C:0
		 * 74181 function F=A&B
		 * T source is BUS
		 */
		case aluf_bus_and_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,1,1, 1, 0));
#else
			alu = m_bus & m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((0,2,"	ALU<- BUS AND T (%#o := %#o & %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 04: ALU <- BUS ^ T
		 * PROM data for S3-0:0110 M:1 C:0
		 * 74181 function F=A^B
		 * T source is BUS
		 */
		case aluf_bus_xor_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,0, 1, 0));
#else
			alu = m_bus ^ m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((0,2,"	ALU<- BUS XOR T (%#o := %#o ^ %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 05: ALU <- BUS + 1
		 * PROM data for S3-0:0000 M:0 C:0
		 * 74181 function F=A+1
		 * T source is ALU
		 */
		case aluf_bus_plus_1__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,0,0, 0, 0));
#else
			alu = m_bus + 1;
			m_aluc0 = (alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((0,2,"	ALU<- BUS + 1 (%#o := %#o + 1)\n", alu, m_bus));
			break;

		/**
		 * 06: ALU <- BUS - 1
		 * PROM data for S3-0:1111 M:0 C:1
		 * 74181 function F=A-1
		 * T source is ALU
		 */
		case aluf_bus_minus_1__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,1,1,1, 0, 1));
#else
			alu = m_bus + 0177777;
			m_aluc0 = (~m_alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((0,2,"	ALU<- BUS - 1 (%#o := %#o - 1)\n", alu, m_bus));
			break;

		/**
		 * 07: ALU <- BUS + T
		 * PROM data for S3-0:1001 M:0 C:1
		 * 74181 function F=A+B
		 * T source is BUS
		 */
		case aluf_bus_plus_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,0,1, 0, 1));
#else
			alu = m_bus + m_t;
			m_aluc0 = (m_alu >> 16) & 1;
#endif
			flags = ALUM2;
			LOG((0,2,"	ALU<- BUS + T (%#o := %#o + %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 10: ALU <- BUS - T
		 * PROM data for S3-0:0110 M:0 C:0
		 * 74181 function F=A-B
		 * T source is BUS
		 */
		case aluf_bus_minus_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,0, 0, 0));
#else
			alu = m_bus + ~m_t + 1;
			m_aluc0 = (~m_alu >> 16) & 1;
#endif
			flags = ALUM2;
			LOG((0,2,"	ALU<- BUS - T (%#o := %#o - %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 11: ALU <- BUS - T - 1
		 * PROM data for S3-0:0110 M:0 C:1
		 * 74181 function F=A-B-1
		 * T source is BUS
		 */
		case aluf_bus_minus_t_minus_1:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,0, 0, 1));
#else
			alu = m_bus + ~m_t;
			m_aluc0 = (~m_alu >> 16) & 1;
#endif
			flags = ALUM2;
			LOG((0,2,"	ALU<- BUS - T - 1 (%#o := %#o - %#o - 1)\n", alu, m_bus, m_t));
			break;

		/**
		 * 12: ALU <- BUS + T + 1
		 * PROM data for S3-0:1001 M:0 C:0
		 * 74181 function F=A+B+1
		 * T source is ALU
		 */
		case aluf_bus_plus_t_plus_1__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,0,1, 0, 0));
#else
			alu = m_bus + m_t + 1;
			m_aluc0 = (m_alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((0,2,"	ALU<- BUS + T + 1 (%#o := %#o + %#o + 1)\n", alu, m_bus, m_t));
			break;

		/**
		 * 13: ALU <- BUS + SKIP
		 * PROM data for S3-0:0000 M:0 C:SKIP
		 * 74181 function F=A (SKIP=1) or F=A+1 (SKIP=0)
		 * T source is ALU
		 */
		case aluf_bus_plus_skip__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,0,0, 0, m_emu.skip^1));
#else
			alu = m_bus + m_emu.skip;
			m_aluc0 = (m_alu >> 16) & 1;
#endif
			flags = ALUM2 | TSELECT;
			LOG((0,2,"	ALU<- BUS + SKIP (%#o := %#o + %#o)\n", alu, m_bus, m_emu.skip));
			break;

		/**
		 * 14: ALU <- BUS,T
		 * PROM data for S3-0:1011 M:1 C:0
		 * 74181 function F=A&B
		 * T source is ALU
		 */
		case aluf_bus_and_t__alut:
#if	USE_ALU_74181
			alu = alu_74181(SMC(1,0,1,1, 1, 0));
#else
			alu = m_bus & m_t;
			m_aluc0 = 1;
#endif
			flags = TSELECT;
			LOG((0,2,"	ALU<- BUS,T (%#o := %#o & %#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 15: ALU <- BUS & ~T
		 * PROM data for S3-0:0111 M:1 C:0
		 * 74181 function F=A&~B
		 * T source is BUS
		 */
		case aluf_bus_and_not_t:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,1,1,1, 1, 0));
#else
			alu = m_bus & ~m_t;
			m_aluc0 = 1;
#endif
			flags = 0;
			LOG((0,2,"	ALU<- BUS AND NOT T (%#o := %#o & ~%#o)\n", alu, m_bus, m_t));
			break;

		/**
		 * 16: ALU <- ???
		 * PROM data for S3-0:???? M:? C:?
		 * 74181 perhaps F=0 (0011/0/0)
		 * T source is BUS
		 */
		case aluf_undef_16:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,1,1, 0, 0));
#else
			alu = 0;
			m_aluc0 = 1;
#endif
			flags = ALUM2;
			LOG((0,0,"	ALU<- 0 (illegal aluf in task %s, mpc:%05o aluf:%02o)\n", task_name(m_task), m_mpc, aluf));
			break;

		/**
		 * 17: ALU <- ???
		 * PROM data for S3-0:???? M:? C:?
		 * 74181 perhaps F=~0 (0011/0/1)
		 * T source is BUS
		 */
		case aluf_undef_17:
		default:
#if	USE_ALU_74181
			alu = alu_74181(SMC(0,0,1,1, 0, 1));
#else
			alu = 0177777;
			m_aluc0 = 1;
#endif
			flags = ALUM2;
			LOG((0,0,"	ALU<- 0 (illegal aluf in task %s, mpc:%05o aluf:%02o)\n", task_name(m_task), m_mpc, aluf));
		}
		m_alu = static_cast<UINT16>(alu);

		/* WRTRAM now, before L is changed */
		if (m_wrtram_flag)
			wrtram();

		switch (f1) {
		case f1_l_lsh_1:
			if (m_task == task_emu) {
				if (f2 == f2_emu_magic) {
					m_shifter = ((m_l << 1) | (m_t >> 15)) & 0177777;
					LOG((0,2,"	SHIFTER <-L MLSH 1 (%#o := %#o<<1|%#o)\n", m_shifter, m_l, m_t >> 15));
					break;
				}
				if (f2 == f2_emu_load_dns) {
					/* shifter is done in F2 */
					break;
				}
			}
			m_shifter = (m_l << 1) & 0177777;
			LOG((0,2,"	SHIFTER <-L LSH 1 (%#o := %#o<<1)\n", m_shifter, m_l));
			break;

		case f1_l_rsh_1:
			if (m_task == task_emu) {
				if (f2 == f2_emu_magic) {
					m_shifter = ((m_l >> 1) | (m_t << 15)) & 0177777;
					LOG((0,2,"	SHIFTER <-L MRSH 1 (%#o := %#o>>1|%#o)\n", m_shifter, m_l, (m_t << 15) & 0100000));
					break;
				}
				if (f2 == f2_emu_load_dns) {
					/* shifter is done in F2 */
					break;
				}
			}
			m_shifter = m_l >> 1;
			LOG((0,2,"	SHIFTER <-L RSH 1 (%#o := %#o>>1)\n", m_shifter, m_l));
			break;

		case f1_l_lcy_8:
			m_shifter = ((m_l >> 8) | (m_l << 8)) & 0177777;
			LOG((0,2,"	SHIFTER <-L LCY 8 (%#o := bswap %#o)\n", m_shifter, m_l));
			break;

		default:
			/* shifter passes L, if F1 is not one of L LSH 1, L RSH 1 or L LCY 8 */
			m_shifter = m_l;
		}

		/* late F1 is done now, if any */
		if (m_f1[1][m_task][f1])
			((*this).*m_f1[1][m_task][f1])();

		/* late F2 is done now, if any */
		if (m_f2[1][m_task][f2])
			((*this).*m_f2[1][m_task][f2])();

		/* late BS is done now, if no constant was put on the bus */
		if (do_bs)
			if (m_bs[1][m_task][bs])
				((*this).*m_bs[1][m_task][bs])();

		/*
		 * update L register and LALUC0, and also M register,
		 * if a RAM related task is active
		 */
		if (MIR_L(m_mir)) {
			/* load L from ALU */
			m_l = m_alu;
			if (flags & ALUM2) {
				m_laluc0 = m_aluc0;
				LOG((0,2, "	L<- ALU (%#o); LALUC0<- ALUC0 (%o)\n", m_alu, m_laluc0));
			} else {
				m_laluc0 = 0;
				LOG((0,2, "	L<- ALU (%#o); LALUC0<- %o\n", m_alu, m_laluc0));
			}
			if (m_ram_related[m_task]) {
				/* load M from ALU, if 'GOODTASK' */
				m_m = m_alu;
				/* also writes to S[bank][0], which can't be read */
				m_s[m_s_reg_bank[m_task]][0] = m_alu;
				LOG((0,2, "	M<- ALU (%#o)\n", m_alu));
			}
		}

		/* update T register, if LOADT is set */
		if (MIR_T(m_mir)) {
			m_cram_addr = m_alu;
			if (flags & TSELECT) {
				LOG((0,2, "	T<- ALU (%#o)\n", m_alu));
				m_t = m_alu;
			} else {
				LOG((0,2, "	T<- BUS (%#o)\n", m_bus));
				m_t = m_bus;
			}
		}

		if (m_task != m_next2_task) {
			/* switch now? */
			if (m_task == m_next_task) {
				/* one more microinstruction */
				m_next_task = m_next2_task;
			} else {
				/* save this task's mpc */
				m_task_mpc[m_task] = m_next;
				m_task_next2[m_task] = m_next2;
				m_task = m_next_task;
				LOG((0,1, "task switch to %02o:%s (cycle %lld)\n", m_task, task_name(m_task), cycle()));
				/* get new task's mpc */
				m_next = m_task_mpc[m_task];
				/* get address modifier after task switch (?) */
				m_next2 = m_task_next2[m_task];

				if (m_active_callback[m_task]) {
					/*
					 * let the task know it becomes active now
					 * and (most probably) reset the wakeup
					 */
					((*this).*m_active_callback[m_task])();
				}
			}
		}
	} while (m_icount-- > 0);

	/* save this task's mpc and address modifier */
	m_task_mpc[m_task] = m_next;
	m_task_next2[m_task] = m_next2;
}

/** @brief reset the various registers */
void alto2_cpu_device::hard_reset()
{
	static const UINT8 ctl2k_u3[256] = {
		/* 0000 */ 000,000,013,016,012,016,014,016,000,001,013,016,012,016,016,016,
		/* 0020 */ 010,001,013,016,012,016,015,016,010,001,013,016,012,016,017,016,
		/* 0040 */ 004,001,013,017,012,017,014,017,004,001,013,017,012,017,016,017,
		/* 0060 */ 014,001,013,017,012,017,015,017,014,001,013,017,012,017,017,017,
		/* 0100 */ 002,001,013,016,012,016,014,016,002,001,013,016,012,016,016,016,
		/* 0120 */ 012,001,013,016,012,016,015,016,012,001,013,016,012,016,017,016,
		/* 0140 */ 006,001,013,017,012,017,014,017,006,013,013,017,012,017,016,017,
		/* 0160 */ 017,001,013,017,012,017,015,017,017,001,013,017,012,017,017,017,
		/* 0200 */ 011,001,013,016,012,016,014,016,011,016,013,016,012,016,016,016,
		/* 0220 */ 001,001,013,016,012,016,015,016,001,001,013,016,012,016,017,016,
		/* 0240 */ 005,001,013,017,012,017,014,017,005,013,013,017,012,017,016,017,
		/* 0260 */ 015,001,013,017,012,017,015,017,015,014,013,017,012,017,017,017,
		/* 0300 */ 003,001,013,016,012,016,014,016,003,001,013,016,012,016,016,016,
		/* 0320 */ 013,001,013,016,012,016,015,016,013,001,013,016,012,016,017,016,
		/* 0340 */ 007,001,013,017,012,017,014,017,007,001,013,017,012,017,016,017,
		/* 0360 */ 016,001,013,017,012,017,015,017,016,015,013,017,012,017,017,017
	};
	memcpy(m_ctl2k_u3, ctl2k_u3, sizeof(m_ctl2k_u3));

	static const UINT8 ctl2k_u38[32] = {
		/* 0000 */ 0367,0353,0323,0315,0265,0251,0221,0216,
		/* 0010 */ 0166,0152,0122,0114,0064,0050,0020,0017,
		/* 0020 */ 0000,0000,0000,0000,0000,0000,0000,0000,
		/* 0030 */ 0000,0000,0000,0000,0000,0000,0000,0000
	};
	memcpy(m_ctl2k_u38, ctl2k_u38, sizeof(m_ctl2k_u38));

	static const UINT8 ctl2k_u76[256] = {
		/* 0000 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0020 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0040 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0060 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0100 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0120 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0140 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0160 */ 000,014,000,000,000,000,000,000,014,000,000,000,000,000,000,000,
		/* 0200 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0220 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0240 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0260 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0300 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0320 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0340 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000,
		/* 0360 */ 000,014,000,000,000,000,000,000,000,014,000,000,000,000,000,000
	};
	memcpy(m_ctl2k_u76, ctl2k_u76, sizeof(m_ctl2k_u76));

	static const UINT8 cram3k_a37[256] = {
		/* 0000 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0020 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0040 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0060 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0100 */ 014,014,014,014,014,014,014,014,014,014,014,014,014,014,014,014,
		/* 0120 */ 014,014,014,016,014,014,014,004,014,014,014,010,014,014,014,015,
		/* 0140 */ 015,015,015,015,015,015,015,015,015,015,015,015,015,015,015,015,
		/* 0160 */ 015,015,015,017,015,015,015,005,015,015,015,011,015,015,015,014,
		/* 0200 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0220 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0240 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0260 */ 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
		/* 0300 */ 014,014,014,014,014,014,014,014,014,014,014,014,014,014,014,014,
		/* 0320 */ 014,014,014,016,014,014,014,014,014,014,014,014,014,014,014,015,
		/* 0340 */ 015,015,015,015,015,015,015,015,015,015,015,015,015,015,015,015,
		/* 0360 */ 015,015,015,017,015,015,015,015,015,015,015,015,015,015,015,014
	};
	memcpy(m_cram3k_a37, cram3k_a37, sizeof(m_cram3k_a37));

	static const UINT8 madr_a64[256] = {
		/* 0000 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0020 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0040 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0060 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0100 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0120 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0140 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0160 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0200 */ 004,004,004,004,004,004,000,000,004,004,004,004,004,004,004,004,
		/* 0220 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,004,
		/* 0240 */ 004,004,004,004,004,004,000,000,004,004,004,004,004,004,004,004,
		/* 0260 */ 004,004,004,004,004,004,004,014,004,004,004,004,004,005,004,004,
		/* 0300 */ 004,004,004,004,004,004,000,000,004,004,004,004,004,004,004,004,
		/* 0320 */ 004,004,004,004,004,004,006,006,004,004,004,004,004,004,004,004,
		/* 0340 */ 004,004,004,004,004,004,000,000,004,004,004,004,004,004,004,004,
		/* 0360 */ 004,004,004,004,004,004,004,004,004,004,004,004,004,005,004,004
	};
	memcpy(m_madr_a64, madr_a64, sizeof(m_madr_a64));

	static const UINT8 madr_a65[256] = {
		/* 0000 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0020 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0040 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0060 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0100 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0120 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0140 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0160 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007,
		/* 0200 */ 007,007,007,007,007,017,007,017,007,007,007,007,007,014,007,014,
		/* 0220 */ 007,007,007,007,007,015,007,015,007,007,007,007,007,013,007,016,
		/* 0240 */ 007,007,007,007,007,017,007,017,007,007,007,007,007,014,007,014,
		/* 0260 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,011,007,016,
		/* 0300 */ 007,007,007,007,007,017,007,017,007,007,007,007,007,014,007,014,
		/* 0320 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,012,007,016,
		/* 0340 */ 007,007,007,007,007,017,007,017,007,007,007,007,007,014,007,014,
		/* 0360 */ 007,007,007,007,007,007,007,007,007,007,007,007,007,010,007,016
	};
	memcpy(m_madr_a65, madr_a65, sizeof(m_madr_a65));

	/* all tasks start in ROM0 */
	m_reset_mode = 0xffff;

	memset(&m_ram_related, 0, sizeof(m_ram_related));

	// install standard handlers in all tasks
	for (int task = 0; task < ALTO2_TASKS; task++) {

		// every task starts at mpc = task number, in either ROM0 or RAM0
		m_task_mpc[task] = (m_ctl2k_u38[task] >> 4) ^ 017;
		if (0 == (m_reset_mode & (1 << task)))
			m_task_mpc[task] |= ALTO2_UCODE_RAM_BASE;

		set_bs(task, bs_read_r,			&alto2_cpu_device::bs_read_r_0,	0);
		set_bs(task, bs_load_r,			&alto2_cpu_device::bs_load_r_0,	&alto2_cpu_device::bs_load_r_1);
		set_bs(task, bs_no_source,		0, 0);
		set_bs(task, bs_task_3,			&alto2_cpu_device::fn_bs_bad_0,	&alto2_cpu_device::fn_bs_bad_1);	// task specific
		set_bs(task, bs_task_4,			&alto2_cpu_device::fn_bs_bad_0,	&alto2_cpu_device::fn_bs_bad_1);	// task specific
		set_bs(task, bs_read_md,		&alto2_cpu_device::bs_read_md_0, 0);
		set_bs(task, bs_mouse,			&alto2_cpu_device::bs_mouse_0, 0);
		set_bs(task, bs_disp,			&alto2_cpu_device::bs_disp_0, 0);

		set_f1(task, f1_nop,			0, 0);
		set_f1(task, f1_load_mar,		0, &alto2_cpu_device::f1_load_mar_1);
		set_f1(task, f1_task,			&alto2_cpu_device::f1_task_0, 0);
		set_f1(task, f1_block,			&alto2_cpu_device::fn_f1_bad_0, &alto2_cpu_device::fn_f1_bad_1);	// not all tasks have the f1_block
		set_f1(task, f1_l_lsh_1,		0, 0);			// inlined in execute()
		set_f1(task, f1_l_rsh_1,		0, 0);			// inlined in execute()
		set_f1(task, f1_l_lcy_8,		0, 0);			// inlined in execute()
		set_f1(task, f1_const,			0, 0);
		set_f1(task, f1_task_10,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_11,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_12,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_13,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_14,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_15,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_16,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific
		set_f1(task, f1_task_17,		&alto2_cpu_device::fn_f1_bad_0,	&alto2_cpu_device::fn_f1_bad_1);	// f1_task_10 to f1_task_17 are task specific

		set_f2(task, f2_nop,			0, 0);
		set_f2(task, f2_bus_eq_zero,	0, &alto2_cpu_device::f2_bus_eq_zero_1);
		set_f2(task, f2_shifter_lt_zero,0, &alto2_cpu_device::f2_shifter_lt_zero_1);
		set_f2(task, f2_shifter_eq_zero,0, &alto2_cpu_device::f2_shifter_eq_zero_1);
		set_f2(task, f2_bus,			0, &alto2_cpu_device::f2_bus_1);
		set_f2(task, f2_alucy,			0, &alto2_cpu_device::f2_alucy_1);
		set_f2(task, f2_load_md,		0, &alto2_cpu_device::f2_load_md_1);
		set_f2(task, f2_const,			0, 0);
		set_f2(task, f2_task_10,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_11,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_12,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_13,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_14,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_15,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_16,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
		set_f2(task, f2_task_17,		&alto2_cpu_device::fn_f2_bad_0,	&alto2_cpu_device::fn_f2_bad_1);	// f2_task_10 to f2_task_17 are task specific
	}

	init_disk();
	init_disp();

	init_emu(task_emu);
	init_001(task_1);
	init_002(task_2);
	init_003(task_3);
	init_ksec(task_ksec);
	init_005(task_5);
	init_006(task_6);
	init_ether(task_ether);
	init_mrt(task_mrt);
	init_dwt(task_dwt);
	init_curt(task_curt);
	init_dht(task_dht);
	init_dvt(task_dvt);
	init_part(task_part);
	init_kwd(task_kwd);
	init_017(task_17);

	install_mmio_fn(0177740, 0177757, &alto2_cpu_device::bank_reg_r, &alto2_cpu_device::bank_reg_w);

	m_dsp_time = 0;			// reset the display state machine values
	m_dsp_state = 020;

	m_task = 0;						// start with task 0
	m_task_wakeup |= 1 << 0;		// set wakeup flag
}

/** @brief software initiated reset (STARTF) */
int alto2_cpu_device::soft_reset()
{
	int task;

	for (task = 0; task < ALTO2_TASKS; task++) {
		// every task starts at mpc = task number, in either ROM0 or RAM0
		m_task_mpc[task] = (m_ctl2k_u38[task] >> 4) ^ 017;
		if (0 == (m_reset_mode & (1 << task)))
			m_task_mpc[task] |= ALTO2_UCODE_RAM_BASE;
	}
	m_next2_task = 0;		// switch to task 0
	m_reset_mode = 0xffff;	// all tasks start in ROM0 again

	m_dsp_time = 0;			// reset the display state machine values
	m_dsp_state = 020;

	return m_next_task;		// return next task (?)
}

void alto2_cpu_device::init_001(int task)
{

}

void alto2_cpu_device::init_002(int task)
{

}

void alto2_cpu_device::init_003(int task)
{

}

void alto2_cpu_device::init_005(int task)
{

}

void alto2_cpu_device::init_006(int task)
{

}

void alto2_cpu_device::init_017(int task)
{

}
