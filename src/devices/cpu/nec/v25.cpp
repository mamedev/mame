// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Alex W. Jackson
/****************************************************************************

    NEC V25/V35 emulator

    ---------------------------------------------

    TODO:

    Using V20/V30 cycle counts for now. V25/V35 cycle counts
    vary based on whether internal RAM access is enabled (RAMEN).

    BTCLR and STOP instructions not implemented.

    IBRK flag (trap I/O instructions) not implemented.

    Interrupt macro service function not implemented.

    Port implementation is incomplete: mode control registers are ignored.

    Timer implementation is incomplete: polling is not implemented
    (reading any of the registers just returns the last value written)

    Serial interface and DMA functions not implemented.
    Note that these functions differ considerably between
    the V25/35 and the V25+/35+.

    Make internal RAM into a real RAM region, and use an
    internal address map (remapped when IDB is written to)
    instead of memory access wrapper functions.
    That way the internal RAM would be visible to the debugger,
    among other benefits.

****************************************************************************/

#include "emu.h"
#include "debugger.h"

typedef UINT8 BOOLEAN;
typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef UINT32 DWORD;

#include "v25.h"
#include "v25priv.h"

const device_type V25 = &device_creator<v25_device>;
const device_type V35 = &device_creator<v35_device>;


v25_common_device::v25_common_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, bool is_16bit, offs_t fetch_xor, UINT8 prefetch_size, UINT8 prefetch_cycles, UINT32 chip_type)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, is_16bit ? 16 : 8, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, is_16bit ? 16 : 8, 17, 0)
	, m_fetch_xor(fetch_xor)
	, m_PCK(8)
	, m_prefetch_size(prefetch_size)
	, m_prefetch_cycles(prefetch_cycles)
	, m_chip_type(chip_type)
	, m_v25v35_decryptiontable(nullptr)
{
}


v25_device::v25_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: v25_common_device(mconfig, V25, "V25", tag, owner, clock, "v25", false, 0, 4, 4, V20_TYPE)
{
}


v35_device::v35_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: v25_common_device(mconfig, V35, "V35", tag, owner, clock, "v35", true, BYTE_XOR_LE(0), 6, 2, V30_TYPE)
{
}


TIMER_CALLBACK_MEMBER(v25_common_device::v25_timer_callback)
{
	m_pending_irq |= param;
}

void v25_common_device::prefetch()
{
	m_prefetch_count--;
}

void v25_common_device::do_prefetch(int previous_ICount)
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

UINT8 v25_common_device::fetch()
{
	prefetch();
	return m_direct->read_byte((Sreg(PS)<<4)+m_ip++, m_fetch_xor);
}

UINT16 v25_common_device::fetchword()
{
	UINT16 r = FETCH();
	r |= (FETCH()<<8);
	return r;
}

#define nec_common_device v25_common_device

#include "v25instr.h"
#include "necmacro.h"
#include "necea.h"
#include "necmodrm.h"

static UINT8 parity_table[256];

UINT8 v25_common_device::fetchop()
{
	UINT8 ret;

	prefetch();
	ret = m_direct->read_byte(( Sreg(PS)<<4)+m_ip++, m_fetch_xor);

	if (m_MF == 0)
		if (m_v25v35_decryptiontable)
		{
			ret = m_v25v35_decryptiontable[ret];
		}
	return ret;
}



/***************************************************************************/

void v25_common_device::device_reset()
{
	attotime time;

	m_ip = 0;
	m_IBRK = 1;
	m_F0 = 0;
	m_F1 = 0;
	m_TF = 0;
	m_IF = 0;
	m_DF = 0;
	m_SignVal = 0;
	m_AuxVal = 0;
	m_OverVal = 0;
	m_ZeroVal = 1;
	m_CarryVal = 0;
	m_ParityVal = 1;
	m_pending_irq = 0;
	m_unmasked_irq = INT_IRQ | NMI_IRQ;
	m_bankswitch_irq = 0;
	m_priority_inttu = 7;
	m_priority_intd = 7;
	m_priority_intp = 7;
	m_priority_ints0 = 7;
	m_priority_ints1 = 7;
	m_IRQS = m_ISPR = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_poll_state = 1;
	m_mode_state = m_MF = (m_v25v35_decryptiontable) ? 0 : 1;
	m_intp_state[0] = 0;
	m_intp_state[1] = 0;
	m_intp_state[2] = 0;
	m_halted = 0;

	m_TM0 = m_MD0 = m_TM1 = m_MD1 = 0;
	m_TMC0 = m_TMC1 = 0;

	m_RAMEN = 1;
	m_TB = 20;
	m_PCK = 8;
	m_IDB = 0xFFE00;

	int tmp = m_PCK << m_TB;
	time = attotime::from_hz(unscaled_clock()) * tmp;
	m_timers[3]->adjust(time, INTTB, time);

	m_timers[0]->adjust(attotime::never);
	m_timers[1]->adjust(attotime::never);
	m_timers[2]->adjust(attotime::never);

	SetRB(7);
	Sreg(PS) = 0xffff;
	Sreg(SS) = 0;
	Sreg(DS0) = 0;
	Sreg(DS1) = 0;

	CHANGE_PC;
}


void v25_common_device::nec_interrupt(unsigned int_num, int /*INTSOURCES*/ source)
{
	UINT32 dest_seg, dest_off;

	i_pushf();
	m_TF = m_IF = 0;
	m_MF = m_mode_state;

	switch(source)
	{
		case BRKN:  /* force native mode */
			m_MF = 1;
			break;
		case BRKS:  /* force secure mode */
			if (m_v25v35_decryptiontable)
				m_MF = 0;
			else
				logerror("%06x: BRKS executed with no decryption table\n",PC());
			break;
		case INT_IRQ:   /* get vector */
			int_num = standard_irq_callback(0);
			break;
		default:
			break;
	}

	dest_off = read_mem_word(int_num*4);
	dest_seg = read_mem_word(int_num*4+2);

	PUSH(Sreg(PS));
	PUSH(m_ip);
	m_ip = (WORD)dest_off;
	Sreg(PS) = (WORD)dest_seg;
	CHANGE_PC;
}

void v25_common_device::nec_bankswitch(unsigned bank_num)
{
	int tmp = CompressFlags();

	m_TF = m_IF = 0;
	m_MF = m_mode_state;

	SetRB(bank_num);

	Wreg(PSW_SAVE) = tmp;
	Wreg(PC_SAVE) = m_ip;
	m_ip = Wreg(VECTOR_PC);
	CHANGE_PC;
}

void v25_common_device::nec_trap()
{
	(this->*s_nec_instruction[fetchop()])();
	nec_interrupt(NEC_TRAP_VECTOR, BRK);
}

#define INTERRUPT(source, vector, priority) \
	if(pending & (source)) {                \
		m_IRQS = vector;               \
		m_ISPR |= (1 << (priority));   \
		m_pending_irq &= ~(source);    \
		if(m_bankswitch_irq & (source))    \
			nec_bankswitch(priority);    \
		else                                    \
			nec_interrupt(vector, source);   \
		break;  /* break out of loop */ \
	}

/* interrupt sources subject to priority control */
#define SOURCES (INTTU0 | INTTU1 | INTTU2 | INTD0 | INTD1 | INTP0 | INTP1 | INTP2 \
				| INTSER0 | INTSR0 | INTST0 | INTSER1 | INTSR1 | INTST1 | INTTB)

void v25_common_device::external_int()
{
	int pending = m_pending_irq & m_unmasked_irq;

	if (pending & NMI_IRQ)
	{
		nec_interrupt(NEC_NMI_VECTOR, NMI_IRQ);
		m_pending_irq &= ~NMI_IRQ;
	}
	else if (pending & SOURCES)
	{
		for(int i = 0; i < 8; i++)
		{
			if (m_ISPR & (1 << i)) break;

			if (m_priority_inttu == i)
			{
				INTERRUPT(INTTU0, NEC_INTTU0_VECTOR, i)
				INTERRUPT(INTTU1, NEC_INTTU1_VECTOR, i)
				INTERRUPT(INTTU2, NEC_INTTU2_VECTOR, i)
			}

			if (m_priority_intd == i)
			{
				INTERRUPT(INTD0, NEC_INTD0_VECTOR, i)
				INTERRUPT(INTD1, NEC_INTD1_VECTOR, i)
			}

			if (m_priority_intp == i)
			{
				INTERRUPT(INTP0, NEC_INTP0_VECTOR, i)
				INTERRUPT(INTP1, NEC_INTP1_VECTOR, i)
				INTERRUPT(INTP2, NEC_INTP2_VECTOR, i)
			}

			if (m_priority_ints0 == i)
			{
				INTERRUPT(INTSER0, NEC_INTSER0_VECTOR, i)
				INTERRUPT(INTSR0, NEC_INTSR0_VECTOR, i)
				INTERRUPT(INTST0, NEC_INTST0_VECTOR, i)
			}

			if (m_priority_ints1 == i)
			{
				INTERRUPT(INTSER1, NEC_INTSER1_VECTOR, i)
				INTERRUPT(INTSR1, NEC_INTSR1_VECTOR, i)
				INTERRUPT(INTST1, NEC_INTST1_VECTOR, i)
			}

			if (i == 7)
				INTERRUPT(INTTB, NEC_INTTB_VECTOR, 7)
		}
	}
	else if (pending & INT_IRQ)
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt((UINT32)-1, INT_IRQ);
		m_irq_state = CLEAR_LINE;
		m_pending_irq &= ~INT_IRQ;
	}
}

/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

#include "necinstr.inc"
#include "v25instr.inc"

/*****************************************************************************/

void v25_common_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
		case 0:
			m_irq_state = state;
			if (state == CLEAR_LINE)
				m_pending_irq &= ~INT_IRQ;
			else
			{
				m_pending_irq |= INT_IRQ;
				m_halted = 0;
			}
			break;
		case INPUT_LINE_NMI:
			if (m_nmi_state == state) return;
			m_nmi_state = state;
			if (state != CLEAR_LINE)
			{
				m_pending_irq |= NMI_IRQ;
				m_halted = 0;
			}
			break;
		case NEC_INPUT_LINE_INTP0:
		case NEC_INPUT_LINE_INTP1:
		case NEC_INPUT_LINE_INTP2:
			irqline -= NEC_INPUT_LINE_INTP0;
			if (m_intp_state[irqline] == state) return;
			m_intp_state[irqline] = state;
			if (state != CLEAR_LINE)
				m_pending_irq |= (INTP0 << irqline);
			break;
		case NEC_INPUT_LINE_POLL:
			m_poll_state = state;
			break;
	}
}

offs_t v25_common_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern int necv_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, const UINT8 *decryption_table);

	return necv_dasm_one(buffer, pc, oprom, m_v25v35_decryptiontable);
}

void v25_common_device::device_start()
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

	for (i = 0; i < 4; i++)
		m_timers[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(v25_common_device::v25_timer_callback),this));

	save_item(NAME(m_ram.w));
	save_item(NAME(m_intp_state));

	save_item(NAME(m_ip));
	save_item(NAME(m_IBRK));
	save_item(NAME(m_F0));
	save_item(NAME(m_F1));
	save_item(NAME(m_TF));
	save_item(NAME(m_IF));
	save_item(NAME(m_DF));
	save_item(NAME(m_MF));
	save_item(NAME(m_RBW));
	save_item(NAME(m_RBB));
	save_item(NAME(m_SignVal));
	save_item(NAME(m_AuxVal));
	save_item(NAME(m_OverVal));
	save_item(NAME(m_ZeroVal));
	save_item(NAME(m_CarryVal));
	save_item(NAME(m_ParityVal));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_unmasked_irq));
	save_item(NAME(m_bankswitch_irq));
	save_item(NAME(m_priority_inttu));
	save_item(NAME(m_priority_intd));
	save_item(NAME(m_priority_intp));
	save_item(NAME(m_priority_ints0));
	save_item(NAME(m_priority_ints1));
	save_item(NAME(m_IRQS));
	save_item(NAME(m_ISPR));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_poll_state));
	save_item(NAME(m_mode_state));
	save_item(NAME(m_no_interrupt));
	save_item(NAME(m_halted));
	save_item(NAME(m_TM0));
	save_item(NAME(m_MD0));
	save_item(NAME(m_TM1));
	save_item(NAME(m_MD1));
	save_item(NAME(m_TMC0));
	save_item(NAME(m_TMC1));
	save_item(NAME(m_RAMEN));
	save_item(NAME(m_TB));
	save_item(NAME(m_PCK));
	save_item(NAME(m_IDB));
	save_item(NAME(m_prefetch_count));
	save_item(NAME(m_prefetch_reset));

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	state_add( V25_PC,    "PC", m_debugger_temp).callimport().callexport().formatstr("%05X");
	state_add( V25_IP,    "IP", m_ip).formatstr("%04X");
	state_add( V25_SP,    "SP", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_FLAGS, "F", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_AW,    "AW", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_CW,    "CW", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_DW,    "DW", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_BW,    "BW", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_BP,    "BP", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_IX,    "IX", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_IY,    "IY", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_ES,    "DS1", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_CS,    "PS", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_SS,    "SS", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( V25_DS,    "DS0", m_debugger_temp).callimport().callexport().formatstr("%04X");

	state_add( STATE_GENPC, "GENPC", m_debugger_temp).callimport().callexport().noshow();
	state_add( STATE_GENSP, "GENSP", m_debugger_temp).callimport().callexport().noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%16s").noshow();

	m_icountptr = &m_icount;
}


void v25_common_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	UINT16 flags = CompressFlags();

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c %d %c%c%c%c%c%c%c%c%c%c%c%c",
				flags & 0x8000 ? 'N':'S',
				(flags & 0x7000) >> 12,
				flags & 0x0800 ? 'O':'.',
				flags & 0x0400 ? 'D':'.',
				flags & 0x0200 ? 'I':'.',
				flags & 0x0100 ? 'T':'.',
				flags & 0x0080 ? 'S':'.',
				flags & 0x0040 ? 'Z':'.',
				flags & 0x0020 ? '1':'.',
				flags & 0x0010 ? 'A':'.',
				flags & 0x0008 ? '0':'.',
				flags & 0x0004 ? 'P':'.',
				flags & 0x0002 ? '.':'I',
				flags & 0x0001 ? 'C':'.');
			break;
	}
}

void v25_common_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case V25_PC:
			if( m_debugger_temp - (Sreg(PS)<<4) < 0x10000 )
			{
				m_ip = m_debugger_temp - (Sreg(PS)<<4);
			}
			else
			{
				Sreg(PS) = m_debugger_temp >> 4;
				m_ip = m_debugger_temp & 0x0000f;
			}
			break;

		case V25_SP:
			Wreg(SP) = m_debugger_temp;
			break;

		case V25_FLAGS:
			ExpandFlags(m_debugger_temp);
			break;

		case V25_AW:
			Wreg(AW) = m_debugger_temp;
			break;

		case V25_CW:
			Wreg(CW) = m_debugger_temp;
			break;

		case V25_DW:
			Wreg(DW) = m_debugger_temp;
			break;

		case V25_BW:
			Wreg(BW) = m_debugger_temp;
			break;

		case V25_BP:
			Wreg(BP) = m_debugger_temp;
			break;

		case V25_IX:
			Wreg(IX) = m_debugger_temp;
			break;

		case V25_IY:
			Wreg(IY) = m_debugger_temp;
			break;

		case V25_ES:
			Sreg(DS1) = m_debugger_temp;
			break;

		case V25_CS:
			Sreg(PS) = m_debugger_temp;
			break;

		case V25_SS:
			Sreg(SS) = m_debugger_temp;
			break;

		case V25_DS:
			Sreg(DS0) = m_debugger_temp;
			break;
	}
}


void v25_common_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case V25_PC:
			m_debugger_temp = (Sreg(PS)<<4) + m_ip;
			break;

		case STATE_GENSP:
			m_debugger_temp = (Sreg(SS)<<4) + Wreg(SP);
			break;

		case V25_SP:
			m_debugger_temp = Wreg(SP);
			break;

		case V25_FLAGS:
			m_debugger_temp = CompressFlags();
			break;

		case V25_AW:
			m_debugger_temp = Wreg(AW);
			break;

		case V25_CW:
			m_debugger_temp = Wreg(CW);
			break;

		case V25_DW:
			m_debugger_temp = Wreg(DW);
			break;

		case V25_BW:
			m_debugger_temp = Wreg(BW);
			break;

		case V25_BP:
			m_debugger_temp = Wreg(BP);
			break;

		case V25_IX:
			m_debugger_temp = Wreg(IX);
			break;

		case V25_IY:
			m_debugger_temp = Wreg(IY);
			break;

		case V25_ES:
			m_debugger_temp = Sreg(DS1);
			break;

		case V25_CS:
			m_debugger_temp = Sreg(PS);
			break;

		case V25_SS:
			m_debugger_temp = Sreg(SS);
			break;

		case V25_DS:
			m_debugger_temp = Sreg(DS0);
			break;
	}
}


void v25_common_device::execute_run()
{
	int prev_ICount;

	int pending = m_pending_irq & m_unmasked_irq;

	if (m_halted && pending)
	{
		for(int i = 0; i < 8; i++)
		{
			if (m_ISPR & (1 << i)) break;

			if (m_priority_inttu == i && (pending & (INTTU0|INTTU1|INTTU2)))
				m_halted = 0;

			if (m_priority_intd == i && (pending & (INTD0|INTD1)))
				m_halted = 0;

			if (m_priority_intp == i && (pending & (INTP0|INTP1|INTP2)))
				m_halted = 0;

			if (m_priority_ints0 == i && (pending & (INTSER0|INTSR0|INTST0)))
				m_halted = 0;

			if (m_priority_ints1 == i && (pending & (INTSER1|INTSR1|INTST1)))
				m_halted = 0;

			if (i == 7 && (pending & INTTB))
				m_halted = 0;
		}
	}

	if (m_halted)
	{
		m_icount = 0;
		debugger_instruction_hook(this, (Sreg(PS)<<4) + m_ip);
		return;
	}

	while(m_icount>0) {
		/* Dispatch IRQ */
		if (m_no_interrupt==0 && (m_pending_irq & m_unmasked_irq))
		{
			if (m_pending_irq & NMI_IRQ)
				external_int();
			else if (m_IF)
				external_int();
		}

		/* No interrupt allowed between last instruction and this one */
		if (m_no_interrupt)
			m_no_interrupt--;

		debugger_instruction_hook(this, (Sreg(PS)<<4) + m_ip);
		prev_ICount = m_icount;
		(this->*s_nec_instruction[fetchop()])();
		do_prefetch(prev_ICount);
	}
}
