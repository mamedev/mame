// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m740.c

    Mitsubishi M740 series (M507xx/M509xx)

***************************************************************************/

#include "emu.h"
#include "m740.h"

DEFINE_DEVICE_TYPE(M740, m740_device, "m740", "Mitsubishi M740")

m740_device::m740_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m740_device(mconfig, M740, tag, owner, clock)
{
}

m740_device::m740_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock), m_irq_multiplex(0), m_irq_vector(0)
{
}

u32 m740_device::get_state_base() const
{
	return inst_state_base;
}

std::unique_ptr<util::disasm_interface> m740_device::create_disassembler()
{
	return std::make_unique<m740_disassembler>(this);
}

void m740_device::device_start()
{
	m6502_device::device_start();

	save_item(NAME(m_irq_multiplex));
	save_item(NAME(m_irq_vector));
}

void m740_device::device_reset()
{
	inst_state_base = 0;
	inst_state = STATE_RESET;
	inst_substate = 0;
	nmi_state = false;
	irq_state = false;
	m_irq_multiplex = 0;
	m_irq_vector = 0xfffc;
	apu_irq_state = false;
	irq_taken = false;
	v_state = false;
	sync = false;
	inhibit_interrupts = false;
	SP = 0x00ff;
}

void m740_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		str = string_format("%c%c%c%c%c%c%c",
						P & F_N ? 'N' : '.',
						P & F_V ? 'V' : '.',
						P & F_T ? 'T' : '.',
						P & F_D ? 'D' : '.',
						P & F_I ? 'I' : '.',
						P & F_Z ? 'Z' : '.',
						P & F_C ? 'C' : '.');
		break;
	}
}

uint8_t m740_device::do_clb(uint8_t in, uint8_t bit)
{
	return in & ~(1<<bit);
}

uint8_t m740_device::do_seb(uint8_t in, uint8_t bit)
{
	return in | (1<<bit);
}

// swap the two nibbles of the input (Rotate Right Four bits)
// doesn't affect the flags
uint8_t m740_device::do_rrf(uint8_t in)
{
		return ((in&0xf)<<4) | ((in&0xf0)>>4);
}

void m740_device::do_sbc_dt(uint8_t val)
{
	uint8_t c = P & F_C ? 0 : 1;
	P &= ~(F_N|F_V|F_Z|F_C);
	uint16_t diff = TMP2 - val - c;
	uint8_t al = (TMP2 & 15) - (val & 15) - c;
	if(int8_t(al) < 0)
		al -= 6;
	uint8_t ah = (TMP2 >> 4) - (val >> 4) - (int8_t(al) < 0);
	if(!uint8_t(diff))
		P |= F_Z;
	else if(diff & 0x80)
		P |= F_N;
	if((TMP2^val) & (TMP2^diff) & 0x80)
		P |= F_V;
	if(!(diff & 0xff00))
		P |= F_C;
	if(int8_t(ah) < 0)
		ah -= 6;
	TMP2 = (ah << 4) | (al & 15);
}

void m740_device::do_sbc_ndt(uint8_t val)
{
	uint16_t diff = TMP2 - val - (P & F_C ? 0 : 1);
	P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(diff))
		P |= F_Z;
	else if(int8_t(diff) < 0)
		P |= F_N;
	if((TMP2^val) & (TMP2^diff) & 0x80)
		P |= F_V;
	if(!(diff & 0xff00))
		P |= F_C;
	TMP2 = diff;
}

void m740_device::do_sbct(uint8_t val)
{
	if(P & F_D)
		do_sbc_dt(val);
	else
		do_sbc_ndt(val);
}

void m740_device::do_adc_dt(uint8_t val)
{
	uint8_t c = P & F_C ? 1 : 0;
	P &= ~(F_N|F_V|F_Z|F_C);
	uint8_t al = (TMP2 & 15) + (val & 15) + c;
	if(al > 9)
		al += 6;
	uint8_t ah = (TMP2 >> 4) + (val >> 4) + (al > 15);
	if(!uint8_t(TMP2 + val + c))
		P |= F_Z;
	else if(ah & 8)
		P |= F_N;
	if(~(TMP2^val) & (TMP2^(ah << 4)) & 0x80)
		P |= F_V;
	if(ah > 9)
		ah += 6;
	if(ah > 15)
		P |= F_C;
	TMP2 = (ah << 4) | (al & 15);
}

void m740_device::do_adc_ndt(uint8_t val)
{
	uint16_t sum;
	sum = TMP2 + val + (P & F_C ? 1 : 0);
	P &= ~(F_N|F_V|F_Z|F_C);
	if(!uint8_t(sum))
		P |= F_Z;
	else if(int8_t(sum) < 0)
		P |= F_N;
	if(~(TMP2^val) & (TMP2^sum) & 0x80)
		P |= F_V;
	if(sum & 0xff00)
		P |= F_C;
	TMP2 = sum;
}

void m740_device::do_adct(uint8_t val)
{
	if(P & F_D)
		do_adc_dt(val);
	else
		do_adc_ndt(val);
}

void m740_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum)
	{
		case M740_INT0_LINE:
		case M740_INT1_LINE:
		case M740_INT2_LINE:
		case M740_INT3_LINE:
		case M740_INT4_LINE:
		case M740_INT5_LINE:
		case M740_INT6_LINE:
		case M740_INT7_LINE:
		case M740_INT8_LINE:
		case M740_INT9_LINE:
		case M740_INT10_LINE:
		case M740_INT11_LINE:
		case M740_INT12_LINE:
		case M740_INT13_LINE:
		case M740_INT14_LINE:   // 37450 has 15 IRQ lines, no other known variant has that many
			set_irq_line(inputnum - M740_INT0_LINE, state);
			break;

		case V_LINE:
			if(!v_state && state == ASSERT_LINE)
			{
				P |= F_V;
			}
			v_state = state == ASSERT_LINE;
			break;
	}
}

void m740_device::set_irq_line(int line, int state)
{
	assert(line > 0);
	assert(line <= M740_MAX_INT_LINE);

	if (state == ASSERT_LINE)
	{
		m_irq_multiplex  |= (1<<line);
	}
	else
	{
		m_irq_multiplex &= ~(1<<line);
	}

	irq_state = (m_irq_multiplex != 0);

	if (irq_state)
	{
		for (int i = 0; i < M740_MAX_INT_LINE; i++)
		{
			if (m_irq_multiplex & (1 << i))
			{
				m_irq_vector = 0xfffc - (uint16_t)(2 * i);
				break;
			}
		}
	}

//  printf("M740 single IRQ state is %d (MPX %08x, vector %x)\n", irq_state, m_irq_multiplex, m_irq_vector);
}

#include "cpu/m6502/m740.hxx"
