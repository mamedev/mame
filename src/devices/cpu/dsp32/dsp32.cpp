// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dsp32.c
    Core implementation for the portable DSP32 emulator.

****************************************************************************

    Important note:

    At this time, the emulator is rather incomplete. However, it is
    sufficiently complete to run both Race Drivin' and Hard Drivin's
    Airborne, which is all I was after.

    Things that still need to be implemented:

        * interrupts
        * carry-reverse add operations
        * do loops
        * ieee/dsp conversions
        * input/output conversion
        * serial I/O

    In addition, there are several optimizations enabled which make
    assumptions about the code which may not be valid for other
    applications. Check dsp32ops.inc for details.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "dsp32.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define DETECT_MISALIGNED_MEMORY    0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// internal register numbering for PIO registers
#define PIO_PAR         0
#define PIO_PDR         1
#define PIO_EMR         2
#define PIO_ESR         3
#define PIO_PCR         4
#define PIO_PIR         5
#define PIO_PARE        6
#define PIO_PDR2        7
#define PIO_RESERVED    8

#define UPPER           (0x00ff << 8)
#define LOWER           (0xff00 << 8)

// bits in the PCR register
#define PCR_RESET       0x001
#define PCR_REGMAP      0x002
#define PCR_ENI         0x004
#define PCR_DMA         0x008
#define PCR_AUTO        0x010
#define PCR_PDFs        0x020
#define PCR_PIFs        0x040
#define PCR_RES         0x080
#define PCR_DMA32       0x100
#define PCR_PIO16       0x200
#define PCR_FLG         0x400

// internal flag bits
#define UFLAGBIT        1
#define VFLAGBIT        2



//**************************************************************************
//  MACROS
//**************************************************************************

// register mapping
#define R0              m_r[0]
#define R1              m_r[1]
#define R2              m_r[2]
#define R3              m_r[3]
#define R4              m_r[4]
#define R5              m_r[5]
#define R6              m_r[6]
#define R7              m_r[7]
#define R8              m_r[8]
#define R9              m_r[9]
#define R10             m_r[10]
#define R11             m_r[11]
#define R12             m_r[12]
#define R13             m_r[13]
#define R14             m_r[14]
#define PC              m_r[15]
#define R0_ALT          m_r[16]
#define R15             m_r[17]
#define R16             m_r[18]
#define R17             m_r[19]
#define R18             m_r[20]
#define R19             m_r[21]
#define RMM             m_r[22]
#define RPP             m_r[23]
#define R20             m_r[24]
#define R21             m_r[25]
#define DAUC            m_r[26]
#define IOC             m_r[27]
#define R22             m_r[29]
#define PCSH            m_r[30]

#define A0              m_a[0]
#define A1              m_a[1]
#define A2              m_a[2]
#define A3              m_a[3]
#define A_0             m_a[4]
#define A_1             m_a[5]

#define zFLAG           ((m_nzcflags & 0xffffff) == 0)
#define nFLAG           ((m_nzcflags & 0x800000) != 0)
#define cFLAG           ((m_nzcflags & 0x1000000) != 0)
#define vFLAG           ((m_vflags & 0x800000) != 0)
#define ZFLAG           (m_NZflags == 0)
#define NFLAG           (m_NZflags < 0)
#define UFLAG           (m_VUflags & UFLAGBIT)
#define VFLAG           (m_VUflags & VFLAGBIT)



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type DSP32C = &device_creator<dsp32c_device>;

//-------------------------------------------------
//  dsp32c_device - constructor
//-------------------------------------------------

dsp32c_device::dsp32c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, DSP32C, "DSP32C", tag, owner, clock, "dsp32c", __FILE__),
		m_program_config("program", ENDIANNESS_LITTLE, 32, 24),
		m_pin(0),
		m_pout(0),
		m_ivtp(0),
		m_nzcflags(0),
		m_vflags(0),
		m_NZflags(0),
		m_VUflags(0),
		m_abuf_index(0),
		m_mbuf_index(0),
		m_par(0),
		m_pare(0),
		m_pdr(0),
		m_pdr2(0),
		m_pir(0),
		m_pcr(0),
		m_emr(0),
		m_esr(0),
		m_pcw(0),
		m_piop(0),
		m_ibuf(0),
		m_isr(0),
		m_obuf(0),
		m_osr(0),
		m_iotemp(0),
		m_lastp(0),
		m_icount(0),
		m_lastpins(0),
		m_ppc(0),
		m_program(nullptr),
		m_direct(nullptr),
		m_output_pins_changed(*this)
{
	// set our instruction counter
	m_icountptr = &m_icount;
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void dsp32c_device::device_start()
{
	m_output_pins_changed.resolve_safe();

	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_r[15]).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add(STATE_GENSP,     "GENSP",     m_r[21]).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_iotemp).callimport().callexport().formatstr("%6s").noshow();
	state_add(DSP32_PC,        "PC",        m_r[15]).mask(0xffffff);
	for (int regnum = 0; regnum <= 14; regnum++)
		state_add(DSP32_R0 + regnum, string_format("R%d", regnum).c_str(), m_r[regnum]).mask(0xffffff);
	state_add(DSP32_R15,       "R15",       m_r[17]).mask(0xffffff);
	state_add(DSP32_R16,       "R16",       m_r[18]).mask(0xffffff);
	state_add(DSP32_R17,       "R17",       m_r[19]).mask(0xffffff);
	state_add(DSP32_R18,       "R18",       m_r[20]).mask(0xffffff);
	state_add(DSP32_R19,       "R19",       m_r[21]).mask(0xffffff);
	state_add(DSP32_R20,       "R20",       m_r[24]).mask(0xffffff);
	state_add(DSP32_R21,       "R21",       m_r[25]).mask(0xffffff);
	state_add(DSP32_R22,       "R22",       m_r[29]).mask(0xffffff);
	state_add(DSP32_PIN,       "PIN",       m_pin).mask(0xffffff);
	state_add(DSP32_POUT,      "POUT",      m_pout).mask(0xffffff);
	state_add(DSP32_IVTP,      "IVTP",      m_ivtp).mask(0xffffff);
	state_add(DSP32_A0,        "A0",        m_a[0]).formatstr("%8s");
	state_add(DSP32_A1,        "A1",        m_a[1]).formatstr("%8s");
	state_add(DSP32_A2,        "A2",        m_a[2]).formatstr("%8s");
	state_add(DSP32_A3,        "A3",        m_a[3]).formatstr("%8s");
	state_add(DSP32_DAUC,      "DAUC",      m_r[26]).mask(0xff);
	state_add(DSP32_PAR,       "PAR",       m_par);
	state_add(DSP32_PDR,       "PDR",       m_pdr);
	state_add(DSP32_PIR,       "PIR",       m_pir);
	state_add(DSP32_PCR,       "PCR",       m_iotemp).mask(0x3ff).callimport();
	state_add(DSP32_EMR,       "EMR",       m_emr);
	state_add(DSP32_ESR,       "ESR",       m_esr);
	state_add(DSP32_PCW,       "PCW",       m_pcw);
	state_add(DSP32_PIOP,      "PIOP",      m_piop);
	state_add(DSP32_IBUF,      "IBUF",      m_ibuf);
	state_add(DSP32_ISR,       "ISR",       m_isr);
	state_add(DSP32_OBUF,      "OBUF",      m_obuf);
	state_add(DSP32_OSR,       "OSR" ,      m_osr);
	state_add(DSP32_IOC,       "IOC",       m_r[27]).mask(0xfffff);

	// register our state for saving
	save_item(NAME(m_r));
	save_item(NAME(m_pin));
	save_item(NAME(m_pout));
	save_item(NAME(m_ivtp));
	save_item(NAME(m_nzcflags));
	save_item(NAME(m_vflags));
	save_item(NAME(m_a));
	save_item(NAME(m_NZflags));
	save_item(NAME(m_VUflags));
	save_item(NAME(m_abuf));
	save_item(NAME(m_abufreg));
	save_item(NAME(m_abufVUflags));
	save_item(NAME(m_abufNZflags));
	save_item(NAME(m_abufcycle));
	save_item(NAME(m_abuf_index));
	save_item(NAME(m_mbufaddr));
	save_item(NAME(m_mbufdata));
	save_item(NAME(m_par));
	save_item(NAME(m_pare));
	save_item(NAME(m_pdr));
	save_item(NAME(m_pdr2));
	save_item(NAME(m_pir));
	save_item(NAME(m_pcr));
	save_item(NAME(m_emr));
	save_item(NAME(m_esr));
	save_item(NAME(m_pcw));
	save_item(NAME(m_piop));
	save_item(NAME(m_ibuf));
	save_item(NAME(m_isr));
	save_item(NAME(m_obuf));
	save_item(NAME(m_osr));
	save_item(NAME(m_lastpins));
	save_item(NAME(m_ppc));
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void dsp32c_device::device_reset()
{
	// reset goes to 0
	PC = 0;

	// clear some registers
	m_pcw &= 0x03ff;
	m_pcr = PCR_RESET;
	m_esr = 0;
	m_emr = 0xffff;

	// clear the output pins
	m_output_pins_changed(0);

	// initialize fixed registers
	R0 = R0_ALT = 0;
	RMM = -1;
	RPP = 1;
	A_0 = 0.0;
	A_1 = 1.0;

	// init internal stuff
	m_abufcycle[0] = m_abufcycle[1] = m_abufcycle[2] = m_abufcycle[3] = 12345678;
	m_mbufaddr[0] = m_mbufaddr[1] = m_mbufaddr[2] = m_mbufaddr[3] = 1;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *dsp32c_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr;
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void dsp32c_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		case DSP32_PCR:
			update_pcr(m_iotemp);
			break;

		default:
			fatalerror("dsp32c_device::state_import called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_export - export state out of the device
//-------------------------------------------------

void dsp32c_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			// no actual flags register, so just make something up
			m_iotemp =  (zFLAG ? 0x01 : 0) |
						(nFLAG ? 0x02 : 0) |
						(cFLAG ? 0x04 : 0) |
						(vFLAG ? 0x08 : 0) |
						(ZFLAG ? 0x10 : 0) |
						(NFLAG ? 0x20 : 0) |
						(UFLAG ? 0x40 : 0) |
						(VFLAG ? 0x80 : 0);
			break;

		case DSP32_PCR:
			m_iotemp = m_pcr;
			break;

		default:
			fatalerror("dsp32c_device::state_export called for unexpected value\n");
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void dsp32c_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				NFLAG ? 'N':'.',
				ZFLAG ? 'Z':'.',
				UFLAG ? 'U':'.',
				VFLAG ? 'V':'.',
				nFLAG ? 'n':'.',
				zFLAG ? 'z':'.',
				cFLAG ? 'c':'.',
				vFLAG ? 'v':'.');
			break;

		case DSP32_A0:
		case DSP32_A1:
		case DSP32_A2:
		case DSP32_A3:
			str = string_format("%8g", *(double *)entry.dataptr());
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 dsp32c_device::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 dsp32c_device::disasm_max_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t dsp32c_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( dsp32c );
	return CPU_DISASSEMBLE_NAME(dsp32c)(this, buffer, pc, oprom, opram, options);
}




//**************************************************************************
//  MEMORY ACCESSORS
//**************************************************************************

inline UINT32 dsp32c_device::ROPCODE(offs_t pc)
{
	return m_direct->read_dword(pc);
}

inline UINT8 dsp32c_device::RBYTE(offs_t addr)
{
	return m_program->read_byte(addr);
}

inline void dsp32c_device::WBYTE(offs_t addr, UINT8 data)
{
	m_program->write_byte(addr, data);
}

inline UINT16 dsp32c_device::RWORD(offs_t addr)
{
#if DETECT_MISALIGNED_MEMORY
	if (!WORD_ALIGNED(addr))
		osd_printf_error("Unaligned word read @ %06X, PC=%06X\n", addr, PC);
#endif
	return m_program->read_word(addr);
}

inline UINT32 dsp32c_device::RLONG(offs_t addr)
{
#if DETECT_MISALIGNED_MEMORY
	if (!DWORD_ALIGNED(addr))
		osd_printf_error("Unaligned long read @ %06X, PC=%06X\n", addr, PC);
#endif
	return m_program->read_dword(addr);
}

inline void dsp32c_device::WWORD(offs_t addr, UINT16 data)
{
#if DETECT_MISALIGNED_MEMORY
	if (!WORD_ALIGNED(addr))
		osd_printf_error("Unaligned word write @ %06X, PC=%06X\n", addr, PC);
#endif
	m_program->write_word(addr, data);
}

inline void dsp32c_device::WLONG(offs_t addr, UINT32 data)
{
#if DETECT_MISALIGNED_MEMORY
	if (!DWORD_ALIGNED(addr))
		osd_printf_error("Unaligned long write @ %06X, PC=%06X\n", addr, PC);
#endif
	m_program->write_dword(addr, data);
}



//**************************************************************************
//  IRQ HANDLING
//**************************************************************************

void dsp32c_device::check_irqs()
{
	// finish me!
}


void dsp32c_device::set_irq_line(int irqline, int state)
{
	// finish me!
}



//**************************************************************************
//  REGISTER HANDLING
//**************************************************************************

void dsp32c_device::update_pcr(UINT16 newval)
{
	UINT16 oldval = m_pcr;
	m_pcr = newval;

	// reset the chip if we get a reset
	if ((oldval & PCR_RESET) == 0 && (newval & PCR_RESET) != 0)
		reset();
}



//**************************************************************************
//  OUTPUT HANDLING
//**************************************************************************

void dsp32c_device::update_pins(void)
{
	if (m_pcr & PCR_ENI)
	{
		UINT16 newoutput = 0;

		if (m_pcr & PCR_PIFs)
			newoutput |= DSP32_OUTPUT_PIF;

		if (m_pcr & PCR_PDFs)
			newoutput |= DSP32_OUTPUT_PDF;

		if (newoutput != m_lastpins)
		{
			m_lastpins = newoutput;
			m_output_pins_changed(newoutput);
		}
	}
}



//**************************************************************************
//  CORE INCLUDE
//**************************************************************************

#include "dsp32ops.inc"



//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 dsp32c_device::execute_min_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 dsp32c_device::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 dsp32c_device::execute_input_lines() const
{
	return 2;
}


void dsp32c_device::execute_set_input(int inputnum, int state)
{
}


void dsp32c_device::execute_run()
{
	// skip if halted
	if ((m_pcr & PCR_RESET) == 0)
	{
		m_icount = 0;
		return;
	}

	// update buffered accumulator values
	m_abufcycle[0] += m_icount;
	m_abufcycle[1] += m_icount;
	m_abufcycle[2] += m_icount;
	m_abufcycle[3] += m_icount;

	// handle interrupts
	check_irqs();

	while (m_icount > 0)
		execute_one();

	// normalize buffered accumulator values
	m_abufcycle[0] -= m_icount;
	m_abufcycle[1] -= m_icount;
	m_abufcycle[2] -= m_icount;
	m_abufcycle[3] -= m_icount;
}



//**************************************************************************
//  PARALLEL INTERFACE WRITES
//**************************************************************************

const UINT32 dsp32c_device::s_regmap[4][16] =
{
	{   // DSP32 compatible mode
		PIO_PAR|LOWER, PIO_PAR|UPPER, PIO_PDR|LOWER, PIO_PDR|UPPER,
		PIO_EMR|LOWER, PIO_EMR|UPPER, PIO_ESR|LOWER, PIO_PCR|LOWER,
		PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER,
		PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER
	},
	{   // DSP32C 8-bit mode
		PIO_PAR|LOWER, PIO_PAR|UPPER, PIO_PDR|LOWER, PIO_PDR|UPPER,
		PIO_EMR|LOWER, PIO_EMR|UPPER, PIO_ESR|LOWER, PIO_PCR|LOWER,
		PIO_PIR|LOWER, PIO_PIR|UPPER, PIO_PCR|UPPER, PIO_PARE|LOWER,
		PIO_PDR2|LOWER,PIO_PDR2|UPPER,PIO_RESERVED,  PIO_RESERVED
	},
	{   // DSP32C illegal mode
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED
	},
	{   // DSP32C 16-bit mode
		PIO_PAR,       PIO_RESERVED,  PIO_PDR,       PIO_RESERVED,
		PIO_EMR,       PIO_RESERVED,  PIO_ESR|LOWER, PIO_PCR,
		PIO_PIR,       PIO_RESERVED,  PIO_RESERVED,  PIO_PARE|LOWER,
		PIO_PDR2,      PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED
	}
};



//**************************************************************************
//  PARALLEL INTERFACE WRITES
//**************************************************************************

void dsp32c_device::dma_increment()
{
	if (m_pcr & PCR_AUTO)
	{
		int amount = (m_pcr & PCR_DMA32) ? 4 : 2;
		m_par += amount;
		if (m_par < amount)
			m_pare++;
	}
}


void dsp32c_device::dma_load()
{
	// only process if DMA is enabled
	if (m_pcr & PCR_DMA)
	{
		UINT32 addr = m_par | (m_pare << 16);

		// 16-bit case
		if (!(m_pcr & PCR_DMA32))
			m_pdr = RWORD(addr & 0xfffffe);

		// 32-bit case
		else
		{
			UINT32 temp = RLONG(addr & 0xfffffc);
			m_pdr = temp >> 16;
			m_pdr2 = temp & 0xffff;
		}

		// set the PDF flag to indicate we have data ready
		update_pcr(m_pcr | PCR_PDFs);
	}
}


void dsp32c_device::dma_store()
{
	// only process if DMA is enabled
	if (m_pcr & PCR_DMA)
	{
		UINT32 addr = m_par | (m_pare << 16);

		// 16-bit case
		if (!(m_pcr & PCR_DMA32))
			WWORD(addr & 0xfffffe, m_pdr);

		// 32-bit case
		else
			WLONG(addr & 0xfffffc, (m_pdr << 16) | m_pdr2);

		// clear the PDF flag to indicate we have taken the data
		update_pcr(m_pcr & ~PCR_PDFs);
	}
}


void dsp32c_device::pio_w(int reg, int data)
{
	UINT16 mask;
	UINT8 mode;

	// look up register and mask
	mode = ((m_pcr >> 8) & 2) | ((m_pcr >> 1) & 1);
	reg = s_regmap[mode][reg];
	mask = reg >> 8;
	if (mask == 0x00ff) data <<= 8;
	data &= ~mask;
	reg &= 0xff;

	// switch off the register
	switch (reg)
	{
		case PIO_PAR:
			m_par = (m_par & mask) | data;

			// trigger a load on the upper half
			if (!(mask & 0xff00))
				dma_load();
			break;

		case PIO_PARE:
			m_pare = (m_pare & mask) | data;
			break;

		case PIO_PDR:
			m_pdr = (m_pdr & mask) | data;

			// trigger a write and PDF setting on the upper half
			if (!(mask & 0xff00))
			{
				dma_store();
				dma_increment();
				update_pins();
			}
			break;

		case PIO_PDR2:
			m_pdr2 = (m_pdr2 & mask) | data;
			break;

		case PIO_EMR:
			m_emr = (m_emr & mask) | data;
			break;

		case PIO_ESR:
			m_esr = (m_esr & mask) | data;
			break;

		case PIO_PCR:
			mask |= 0x0060;
			data &= ~mask;
			update_pcr((m_pcr & mask) | data);
			break;

		case PIO_PIR:
			m_pir = (m_pir & mask) | data;

			// set PIF on upper half
			if (!(mask & 0xff00))
			{
				update_pcr(m_pcr | PCR_PIFs);
				update_pins();
			}
			break;

		// error case
		default:
			logerror("dsp32_pio_w called on invalid register %d\n", reg);
			break;
	}
}



//**************************************************************************
//  PARALLEL INTERFACE READS
//**************************************************************************

int dsp32c_device::pio_r(int reg)
{
	UINT16 mask, result = 0xffff;
	UINT8 mode, shift = 0;

	// look up register and mask
	mode = ((m_pcr >> 8) & 2) | ((m_pcr >> 1) & 1);
	reg = s_regmap[mode][reg];
	mask = reg >> 8;
	if (mask == 0x00ff) mask = 0xff00, shift = 8;
	reg &= 0xff;

	// switch off the register
	switch (reg)
	{
		case PIO_PAR:
			result = m_par | 1;
			break;

		case PIO_PARE:
			result = m_pare;
			break;

		case PIO_PDR:
			result = m_pdr;

			// trigger an increment on the lower half
			if (shift != 8)
				dma_increment();

			// trigger a fetch on the upper half
			if (!(mask & 0xff00))
			{
				dma_load();
				update_pins();
			}
			break;

		case PIO_PDR2:
			result = m_pdr2;
			break;

		case PIO_EMR:
			result = m_emr;
			break;

		case PIO_ESR:
			result = m_esr;
			break;

		case PIO_PCR:
			result = m_pcr;
			break;

		case PIO_PIR:
			if (!(mask & 0xff00))
			{
				update_pcr(m_pcr & ~PCR_PIFs);  // clear PIFs
				update_pins();
			}
			result = m_pir;
			break;

		// error case
		default:
			logerror("dsp32_pio_w called on invalid register %d\n", reg);
			break;
	}

	return (result >> shift) & ~mask;
}
