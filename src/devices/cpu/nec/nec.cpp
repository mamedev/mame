// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/****************************************************************************

    NEC V20/V30/V33 emulator

    ---------------------------------------------

    V20 = uPD70108 = 8-bit data bus @ 5MHz or 8MHz
    V20HL = uPD70108H = V20 with EMS support (24-bit address bus)

    V25 = uPD70320 = V20 with on-chip features:
            - 256 bytes on-chip RAM
            - 8 register banks
            - 4-bit input port
            - 20-bit I/O port
            - 2 channel serial interface
            - interrupt controller
            - 2 channel DMA controller
            - 2 channel 16-bit timer
            - new instructions: BTCLR, RETRBI, STOP, BRKCS, TSKSW,
                                MOVSPA, MOVSPB

    V25+ = uPD70325 = V25 @ 8MHz or 10MHz plus changes:
            - faster DMA
            - improved serial interface

    ---------------------------------------------

    V30 = uPD70116 = 16-bit data bus version of V20
    V30HL = uPD70116H = 16-bit data bus version of V20HL
    V30MX = V30HL with separate address and data busses

    V35 = uPD70330 = 16-bit data bus version of V25

    V35+ = uPD70335 = 16-bit data bus version of V25+

    ---------------------------------------------

    V40 = uPD70208 = 8-bit data bus @ 10MHz
    V40HL = uPD70208H = V40 with support up to 20Mhz

    ---------------------------------------------

    V50 = uPD70216 = 16-bit data bus version of V40
    V50HL = uPD70216H = 16-bit data bus version of V40HL

    ---------------------------------------------

    V41 = uPD70270

    V51 = uPD70280



    V33A = uPD70136A (interrupt vector #s compatible with x86)
    V53A = uPD70236A



    Instruction differences:
        V20, V30, V40, V50 have dedicated emulation instructions
            (BRKEM, RETEM, CALLN)

        V33 / V33A has dedicated address mode instructions (V53 / V53A are based on those cores with extra peripherals)
            (BRKXA, RETXA)



    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
    on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
    on the i286 emulator by Fabrice Frances which had initial work based on
    David Hedley's pcemu(!).

    This new core features 99% accurate cycle counts for each processor,
    there are still some complex situations where cycle counts are wrong,
    typically where a few instructions have differing counts for odd/even
    source and odd/even destination memory operands.

    Flag settings are also correct for the NEC processors rather than the
    I86 versions.

    Changelist:

    22/02/2003:
        Removed cycle counts from memory accesses - they are certainly wrong,
        and there is already a memory access cycle penalty in the opcodes
        using them.

        Fixed save states.

        Fixed ADJBA/ADJBS/ADJ4A/ADJ4S flags/return values for all situations.
        (Fixes bugs in Geostorm and Thunderblaster)

        Fixed carry flag on NEG (I thought this had been fixed circa Mame 0.58,
        but it seems I never actually submitted the fix).

        Fixed many cycle counts in instructions and bug in cycle count
        macros (odd word cases were testing for odd instruction word address
        not data address).

    Todo!
        Double check cycle timing is 100%.

****************************************************************************/

#include "emu.h"
#include "nec.h"
#include "necdasm.h"

#define LOG_BUSLOCK (1 << 1)
//#define VERBOSE (...)

#include "logmacro.h"

typedef uint8_t BOOLEAN;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

#include "necpriv.ipp"

DEFINE_DEVICE_TYPE(V20,  v20_device,  "v20",  "NEC V20")
DEFINE_DEVICE_TYPE(V30,  v30_device,  "v30",  "NEC V30")
DEFINE_DEVICE_TYPE(V33,  v33_device,  "v33",  "NEC V33")
DEFINE_DEVICE_TYPE(V33A, v33a_device, "v33a", "NEC V33A")


nec_common_device::nec_common_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_16bit, uint8_t prefetch_size, uint8_t prefetch_cycles, uint32_t chip_type, address_map_constructor internal_port_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, is_16bit ? 16 : 8, chip_type == V33_TYPE ? 24 : 20, 0, 20, chip_type == V33_TYPE ? 14 : 0)
	, m_io_config("io", ENDIANNESS_LITTLE, is_16bit ? 16 : 8, 16, 0, internal_port_map)
	, m_prefetch_size(prefetch_size)
	, m_prefetch_cycles(prefetch_cycles)
	, m_chip_type(chip_type)
	, m_v33_transtable(*this, "v33_transtable")
{
}


v20_device::v20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nec_common_device(mconfig, V20, tag, owner, clock, false, 4, 4, V20_TYPE)
{
}


v30_device::v30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nec_common_device(mconfig, V30, tag, owner, clock, true, 6, 2, V30_TYPE)
{
}

device_memory_interface::space_config_vector nec_common_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


/* FIXME: Need information about prefetch size and cycles for V33.
 * complete guess below, nbbatman will not work
 * properly without. */
v33_base_device::v33_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_port_map)
	: nec_common_device(mconfig, type, tag, owner, clock, true, 6, 1, V33_TYPE, internal_port_map)
{
}


v33_device::v33_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: v33_base_device(mconfig, V33, tag, owner, clock, address_map_constructor(FUNC(v33_device::v33_internal_port_map), this))
{
}


v33a_device::v33a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: v33_base_device(mconfig, V33A, tag, owner, clock, address_map_constructor(FUNC(v33a_device::v33_internal_port_map), this))
{
}


uint16_t v33_base_device::xam_r()
{
	// Only bit 0 is defined
	return m_xa ? 1 : 0;
}

void v33_base_device::v33_internal_port_map(address_map &map)
{
	map(0xff00, 0xff7f).ram().share("v33_transtable");
	map(0xff80, 0xff81).r(FUNC(v33_base_device::xam_r)).unmapw();
	map(0xff82, 0xffff).unmaprw();
}


offs_t nec_common_device::v33_translate(offs_t addr)
{
	if (m_xa)
		return uint32_t(m_v33_transtable[(addr >> 14) & 63]) << 14 | (addr & 0x03fff);
	else
		return addr & 0xfffff;
}

bool v33_base_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);

	if (spacenum == AS_PROGRAM)
		address = v33_translate(address);
	return true;
}


std::unique_ptr<util::disasm_interface> nec_common_device::create_disassembler()
{
	return std::make_unique<nec_disassembler>(this);
}


void nec_common_device::prefetch()
{
	m_prefetch_count--;
}

void nec_common_device::do_prefetch(int previous_ICount)
{
	int diff = previous_ICount - (int) m_icount;

	/* The implementation is not accurate, but comes close.
	 * It does not respect that the V30 will fetch two bytes
	 * at once directly, but instead uses only 2 cycles instead
	 * of 4. There are however only very few sources publicly
	 * available and they are vague.
	 */
	while (m_prefetch_count<0)
	{
		m_prefetch_count++;
		if (diff>m_prefetch_cycles)
			diff -= m_prefetch_cycles;
		else
			m_icount -= m_prefetch_cycles;
	}

	if (m_prefetch_reset)
	{
		m_prefetch_count = 0;
		m_prefetch_reset = 0;
		return;
	}

	while (diff>=m_prefetch_cycles && m_prefetch_count < m_prefetch_size)
	{
		diff -= m_prefetch_cycles;
		m_prefetch_count++;
	}

}

uint8_t nec_common_device::fetch()
{
	prefetch();
	return m_dr8((Sreg(PS)<<4)+m_ip++);
}

uint16_t nec_common_device::fetchword()
{
	uint16_t r = fetch();
	r |= (fetch()<<8);
	return r;
}

#include "necinstr.h"
#include "necmacro.h"
#include "necea.h"
#include "necmodrm.h"

static uint8_t parity_table[256];

uint8_t nec_common_device::fetchop()
{
	prefetch();
	return m_dr8((Sreg(PS)<<4)+m_ip++);
}



/***************************************************************************/

void nec_common_device::device_reset()
{
	memset( &m_regs.w, 0, sizeof(m_regs.w));

	m_ip = 0;
	m_prev_ip = 0;
	m_TF = 0;
	m_IF = 0;
	m_DF = 0;
	m_MF = 1;
	m_em = 1;
	m_SignVal = 0;
	m_AuxVal = 0;
	m_OverVal = 0;
	m_ZeroVal = 1;
	m_CarryVal = 0;
	m_ParityVal = 1;
	m_pending_irq = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_poll_state = 1;
	m_halted = 0;

	if (m_chip_type == V33_TYPE)
		m_xa = false;

	Sreg(PS) = 0xffff;
	Sreg(SS) = 0;
	Sreg(DS0) = 0;
	Sreg(DS1) = 0;

	CHANGE_PC;
}


void nec_common_device::nec_interrupt(unsigned int_num, int/*INTSOURCES*/ source)
{
	uint32_t dest_seg, dest_off;

	i_pushf();
	m_TF = m_IF = 0;
	m_MF = 1;

	if (source == INT_IRQ)  /* get vector */
		int_num = standard_irq_callback(0, PC());
	debugger_exception_hook(int_num);

	dest_off = read_mem_word(int_num*4);
	dest_seg = read_mem_word(int_num*4+2);

	PUSH(Sreg(PS));
	PUSH(m_ip);
	m_prev_ip = m_ip = (WORD)dest_off;
	Sreg(PS) = (WORD)dest_seg;
	CHANGE_PC;
}

void nec_common_device::nec_trap()
{
	(this->*s_nec_instruction[fetchop()])();
	nec_interrupt(NEC_TRAP_VECTOR, BRK);
}

void nec_common_device::nec_brk(unsigned int_num)
{
	if (m_chip_type != V33_TYPE)
	{
		m_em = 0;
		m_MF = 0;
		i_pushf();
		PUSH(Sreg(PS));
		PUSH(m_ip);
	}
	m_prev_ip = m_ip = read_mem_word(int_num*4);
	Sreg(PS) = read_mem_word(int_num*4+2);
	CHANGE_PC;
}

void nec_common_device::external_int()
{
	if (m_pending_irq & NMI_IRQ)
	{
		nec_interrupt(NEC_NMI_VECTOR, NMI_IRQ);
		m_pending_irq &= ~NMI_IRQ;
	}
	else if (m_pending_irq)
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt((uint32_t)-1, INT_IRQ);
		m_irq_state = CLEAR_LINE;
		m_pending_irq &= ~INT_IRQ;
	}
}

/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

#include "necinstr.hxx"
#include "nec80inst.hxx"

/*****************************************************************************/

void nec_common_device::set_int_line(int state)
{
	m_irq_state = state;
	if (state == CLEAR_LINE)
		m_pending_irq &= ~INT_IRQ;
	else
	{
		m_pending_irq |= INT_IRQ;
		m_halted = 0;
	}
}

void nec_common_device::set_nmi_line(int state)
{
	if (m_nmi_state == state)
		return;
	m_nmi_state = state;
	if (state != CLEAR_LINE)
	{
		m_pending_irq |= NMI_IRQ;
		m_halted = 0;
	}
}

void nec_common_device::set_poll_line(int state)
{
	m_poll_state = state;
}

void nec_common_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case 0:
		set_int_line(state);
		break;
	case INPUT_LINE_NMI:
		set_nmi_line(state);
		break;
	case NEC_INPUT_LINE_POLL:
		set_poll_line(state);
		break;
	}
}

void nec_common_device::device_start()
{
	unsigned int i, j, c;

	static const WREGS wreg_name[8]={ AW, CW, DW, BW, SP, BP, IX, IY };
	static const BREGS breg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	for (i = 0; i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;
		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = breg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = wreg_name[(i & 0x38) >> 3];
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = wreg_name[i & 7];
		Mod_RM.RM.b[i] = breg_name[i & 7];
	}

	m_no_interrupt = 0;
	m_prefetch_count = 0;
	m_prefetch_reset = 0;
	m_prefix_base = 0;
	m_seg_prefix = 0;
	m_EA = 0;
	m_EO = 0;
	m_E16 = 0;
	m_debugger_temp = 0;
	m_ip = 0;
	m_prev_ip = 0;

	memset(m_regs.w, 0x00, sizeof(m_regs.w));
	memset(m_sregs, 0x00, sizeof(m_sregs));

	save_item(NAME(m_regs.w));
	save_item(NAME(m_sregs));

	save_item(NAME(m_ip));
	save_item(NAME(m_prev_ip));
	save_item(NAME(m_TF));
	save_item(NAME(m_IF));
	save_item(NAME(m_DF));
	save_item(NAME(m_MF));
	save_item(NAME(m_SignVal));
	save_item(NAME(m_AuxVal));
	save_item(NAME(m_OverVal));
	save_item(NAME(m_ZeroVal));
	save_item(NAME(m_CarryVal));
	save_item(NAME(m_ParityVal));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_poll_state));
	save_item(NAME(m_no_interrupt));
	save_item(NAME(m_halted));
	save_item(NAME(m_prefetch_count));
	save_item(NAME(m_prefetch_reset));

	m_program = &space(AS_PROGRAM);
	if (m_program->data_width() == 8)
	{
		m_program->cache(m_cache8);
		m_dr8 = [this](offs_t address) -> u8 { return m_cache8.read_byte(address); };
	}
	else if (m_chip_type == V33_TYPE)
	{
		save_item(NAME(m_xa));
		m_program->cache(m_cache16);
		m_dr8 = [this](offs_t address) -> u8 { return m_cache16.read_byte(v33_translate(address)); };
	}
	else
	{
		m_program->cache(m_cache16);
		m_dr8 = [this](offs_t address) -> u8 { return m_cache16.read_byte(address); };
	}

	m_io = &space(AS_IO);

	state_add( NEC_PC,  "PC", m_ip).formatstr("%04X");
	state_add( NEC_PSW, "PSW", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( NEC_AW,  "AW", Wreg(AW)).formatstr("%04X");
	state_add( NEC_CW,  "CW", Wreg(CW)).formatstr("%04X");
	state_add( NEC_DW,  "DW", Wreg(DW)).formatstr("%04X");
	state_add( NEC_BW,  "BW", Wreg(BW)).formatstr("%04X");
	state_add( NEC_SP,  "SP", Wreg(SP)).formatstr("%04X");
	state_add( NEC_BP,  "BP", Wreg(BP)).formatstr("%04X");
	state_add( NEC_IX,  "IX", Wreg(IX)).formatstr("%04X");
	state_add( NEC_IY,  "IY", Wreg(IY)).formatstr("%04X");
	state_add( NEC_DS1, "DS1", Sreg(DS1)).formatstr("%04X");
	state_add( NEC_PS,  "PS", Sreg(PS)).formatstr("%04X");
	state_add( NEC_SS,  "SS", Sreg(SS)).formatstr("%04X");
	state_add( NEC_DS0, "DS0", Sreg(DS0)).formatstr("%04X");

	state_add( NEC_AL, "AL", Breg(AL)).noshow();
	state_add( NEC_AH, "AH", Breg(AH)).noshow();
	state_add( NEC_CL, "CL", Breg(CL)).noshow();
	state_add( NEC_CH, "CH", Breg(CH)).noshow();
	state_add( NEC_DL, "DL", Breg(DL)).noshow();
	state_add( NEC_DH, "DH", Breg(DH)).noshow();
	state_add( NEC_BL, "BL", Breg(BL)).noshow();
	state_add( NEC_BH, "BH", Breg(BH)).noshow();

	if (m_chip_type == V33_TYPE)
		state_add(NEC_XA, "XA", m_xa);

	state_add( STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_debugger_temp).callexport().noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%16s").noshow();

	set_icountptr(m_icount);
}

void nec_common_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	uint16_t flags = CompressFlags();

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				flags & 0x8000 ? 'N':'E',
				flags & 0x4000 ? '?':'.',
				flags & 0x2000 ? '?':'.',
				flags & 0x1000 ? '?':'.',
				flags & 0x0800 ? 'O':'.',
				flags & 0x0400 ? 'D':'.',
				flags & 0x0200 ? 'I':'.',
				flags & 0x0100 ? 'T':'.',
				flags & 0x0080 ? 'S':'.',
				flags & 0x0040 ? 'Z':'.',
				flags & 0x0020 ? '?':'.',
				flags & 0x0010 ? 'A':'.',
				flags & 0x0008 ? '?':'.',
				flags & 0x0004 ? 'P':'.',
				flags & 0x0002 ? '.':'?',
				flags & 0x0001 ? 'C':'.');
			break;
	}
}

void nec_common_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			if (m_debugger_temp - (Sreg(PS)<<4) < 0x10000)
			{
				m_ip = m_debugger_temp - (Sreg(PS)<<4);
			}
			else
			{
				Sreg(PS) = m_debugger_temp >> 4;
				m_ip = m_debugger_temp & 0x0000f;
			}
			m_prev_ip = m_ip;
			break;

		case NEC_PSW:
			ExpandFlags(m_debugger_temp);
			break;
	}
}


void nec_common_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			m_debugger_temp = (Sreg(PS)<<4) + m_ip;
			break;

		case STATE_GENPCBASE:
			m_debugger_temp = (Sreg(PS)<<4) + m_prev_ip;
			break;

		case NEC_PSW:
			m_debugger_temp = CompressFlags();
			break;
	}
}


void nec_common_device::execute_run()
{
	int prev_ICount;

	if (m_halted)
	{
		m_icount = 0;
		debugger_instruction_hook((Sreg(PS)<<4) + m_ip);
		return;
	}

	while(m_icount>0) {
		m_prev_ip = m_ip;

		/* Dispatch IRQ */
		if (m_pending_irq && m_no_interrupt==0)
		{
			if (m_pending_irq & NMI_IRQ)
				external_int();
			else if (m_IF)
				external_int();
		}

		/* No interrupt allowed between last instruction and this one */
		if (m_no_interrupt)
			m_no_interrupt--;

		debugger_instruction_hook((Sreg(PS)<<4) + m_ip);
		prev_ICount = m_icount;
		if (m_MF)
			(this->*s_nec_instruction[fetchop()])();
		else
			(this->*s_nec80_instruction[fetchop()])();
		do_prefetch(prev_ICount);
	}
}
