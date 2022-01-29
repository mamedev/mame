// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
High-speed CMOS 6805-compatible microcontrollers

The M68HC05 family uses the M6805 instruction set with a few additions
but runs at two clocks per machine cycle, and has incompatible on-board
peripherals.  It comes in mask ROM (M68HC05), EPROM (M68HC705) and
EEPROM (M68HC805) variants.  The suffix gives some indication of the
memory sizes and on-board peripherals, but there's not a lot of
consistency across the ROM/EPROM/EEPROM variants.

Most devices in this family have a 16-bit free-running counter fed from
the internal clock.  The counter value can be captured on an input edge,
and an output can be automatically set when the counter reaches a
certain value.  The lower-end devices instead have a 15-bit multifunction
ripple counter with a programmable selector for the last four stages that
determines both the COP watchdog timeout and the real-time interrupt rate
(this is not currently emulated).
*/
#include "emu.h"
#include "m68hc05.h"
#include "m6805defs.h"
#include "6805dasm.h"


/****************************************************************************
 * Configurable logging
 ****************************************************************************/

#define LOG_GENERAL (1U <<  0)
#define LOG_INT     (1U <<  1)
#define LOG_IOPORT  (1U <<  2)
#define LOG_TIMER   (1U <<  3)
#define LOG_COP     (1U <<  4)

//#define VERBOSE (LOG_GENERAL | LOG_INT | LOG_IOPORT | LOG_TIMER | LOG_COP)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGINT(...)     LOGMASKED(LOG_INT,    __VA_ARGS__)
#define LOGIOPORT(...)  LOGMASKED(LOG_IOPORT, __VA_ARGS__)
#define LOGTIMER(...)   LOGMASKED(LOG_TIMER,  __VA_ARGS__)
#define LOGCOP(...)     LOGMASKED(LOG_COP,    __VA_ARGS__)


namespace {

std::pair<u16, char const *> const m68hc05c4_syms[] = {
	{ 0x0000, "PORTA"  }, { 0x0001, "PORTB"  }, { 0x0002, "PORTC"  }, { 0x0003, "PORTD"  },
	{ 0x0004, "DDRA"   }, { 0x0005, "DDRB"   }, { 0x0006, "DDRC"   },
	{ 0x000a, "SPCR"   }, { 0x000b, "SPSR"   }, { 0x000c, "SPDR"   },
	{ 0x000d, "BAUD"   }, { 0x000e, "SCCR1"  }, { 0x000f, "SCCR2"  }, { 0x0010, "SCSR"   }, { 0x0011, "SCDR"   },
	{ 0x0012, "TCR"    }, { 0x0013, "TSR"    },
	{ 0x0014, "ICRH"   }, { 0x0015, "ICRL"   }, { 0x0016, "OCRH"   }, { 0x0017, "OCRL"   },
	{ 0x0018, "TRH"    }, { 0x0019, "TRL"    }, { 0x001a, "ATRH"   }, { 0x001b, "ATRL"   } };

std::pair<u16, char const *> const m68hc705c8a_syms[] = {
	{ 0x0000, "PORTA"  }, { 0x0001, "PORTB" }, { 0x0002, "PORTC" }, { 0x0003, "PORTD" },
	{ 0x0004, "DDRA"   }, { 0x0005, "DDRB"  }, { 0x0006, "DDRC"  },
	{ 0x000a, "SPCR"   }, { 0x000b, "SPSR"  }, { 0x000c, "SPDR"  },
	{ 0x000d, "BAUD"   }, { 0x000e, "SCCR1" }, { 0x000f, "SCCR2" }, { 0x0010, "SCSR"  }, { 0x0011, "SCDR"  },
	{ 0x0012, "TCR"    }, { 0x0013, "TSR"   },
	{ 0x0014, "ICRH"   }, { 0x0015, "ICRL"  }, { 0x0016, "OCRH"  }, { 0x0017, "OCRL"  },
	{ 0x0018, "TRH"    }, { 0x0019, "TRL"   }, { 0x001a, "ATRH"  }, { 0x001b, "ATRL"  },
	{ 0x001c, "PROG"   },
	{ 0x001d, "COPRST" }, { 0x001e, "COPCR" },
	{ 0x1fdf, "OPTION" } };

std::pair<u16, char const *> const m68hc705j1a_syms[] = {
	{ 0x0000, "PORTA"  }, { 0x0001, "PORTB" },
	{ 0x0004, "DDRA"   }, { 0x0005, "DDRB"  },
	{ 0x0008, "TSCR"   }, { 0x0009, "TCR"   }, { 0x000a, "ISCR"  },
	{ 0x0010, "PDRA"   }, { 0x0011, "PDRB"  },
	{ 0x0014, "EPROG"  },
	{ 0x07f0, "COPR"   }, { 0x07f1, "MOR"   } };

std::pair<u16, char const *> const m68hc05l9_syms[] = {
	{ 0x0000, "PORTA"  }, { 0x0001, "PORTB"  }, { 0x0002, "PORTC"  }, { 0x0003, "PORTD"  },
	{ 0x0004, "DDRA"   }, { 0x0005, "DDRB"   }, { 0x0006, "DDRC"   }, { 0x0007, "DDRD"   },
	{ 0x0008, "COUNT"  },
	{ 0x0009, "GCR1"   }, { 0x000a, "GCR2"   },
	{ 0x000b, "MINA"   }, { 0x000c, "HOURA"  },
	{ 0x000d, "BAUD"   }, { 0x000e, "SCCR1"  }, { 0x000f, "SCCR2"  }, { 0x0010, "SCSR"   }, { 0x0011, "SCDR"   },
	{ 0x0012, "TCR"    }, { 0x0013, "TSR"    },
	{ 0x0014, "ICRH"   }, { 0x0015, "ICRL"   }, { 0x0016, "OCRH"   }, { 0x0017, "OCRL"   },
	{ 0x0018, "TRH"    }, { 0x0019, "TRL"    }, { 0x001a, "ATRH"   }, { 0x001b, "ATRL"   },
	{ 0x001c, "RTCSR"  },
	{ 0x001d, "HOUR"   }, { 0x001e, "MIN"    }, { 0x001f, "SEC"    } };


ROM_START( m68hc705c4a )
	ROM_REGION(0x00f0, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x00f0, NO_DUMP)
ROM_END

ROM_START( m68hc705c8a )
	ROM_REGION(0x00f0, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x00f0, NO_DUMP)
ROM_END


//constexpr u16 M68HC05_VECTOR_SPI        = 0xfff4;
//constexpr u16 M68HC05_VECTOR_SCI        = 0xfff6;
constexpr u16 M68HC05_VECTOR_TIMER      = 0xfff8;
constexpr u16 M68HC05_VECTOR_IRQ        = 0xfffa;
constexpr u16 M68HC05_VECTOR_SWI        = 0xfffc;
//constexpr u16 M68HC05_VECTOR_RESET      = 0xfffe;

constexpr u16 M68HC05_INT_IRQ           = u16(1) << 0;
constexpr u16 M68HC05_INT_TIMER         = u16(1) << 1;

constexpr u16 M68HC05_INT_MASK          = M68HC05_INT_IRQ | M68HC05_INT_TIMER;

} // anonymous namespace



/****************************************************************************
 * Global variables
 ****************************************************************************/

DEFINE_DEVICE_TYPE(M68HC05C4,   m68hc05c4_device,   "m68hc05c4",   "Motorola MC68HC05C4")
DEFINE_DEVICE_TYPE(M68HC05C8,   m68hc05c8_device,   "m68hc05c8",   "Motorola MC68HC05C8")
DEFINE_DEVICE_TYPE(M68HC705C4A, m68hc705c4a_device, "m68hc705c4a", "Motorola MC68HC705C4A")
DEFINE_DEVICE_TYPE(M68HC705C8A, m68hc705c8a_device, "m68hc705c8a", "Motorola MC68HC705C8A")
DEFINE_DEVICE_TYPE(M68HC705J1A, m68hc705j1a_device, "m68hc705j1a", "Motorola MC68HC705J1A")
DEFINE_DEVICE_TYPE(M68HC05L9,   m68hc05l9_device,   "m68hc05l9",   "Motorola MC68HC05L9")
DEFINE_DEVICE_TYPE(M68HC05L11,  m68hc05l11_device,  "m68hc05l11",  "Motorola MC68HC05L11")



/****************************************************************************
 * M68HC05 base device
 ****************************************************************************/

m68hc05_device::m68hc05_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock,
		device_type type,
		u32 addr_width,
		u16 vector_mask,
		address_map_constructor internal_map)
	: m6805_base_device(
			mconfig,
			tag,
			owner,
			clock,
			type,
			{ addr_width > 13 ? s_hc_b_ops : s_hc_s_ops, s_hc_cycles, addr_width, 0x00ff, 0x00c0, vector_mask, M68HC05_VECTOR_SWI },
			internal_map)
	, m_port_cb_r(*this)
	, m_port_cb_w(*this)
	, m_port_bits{ 0xff, 0xff, 0xff, 0xff }
	, m_port_interrupt{ 0x00, 0x00, 0x00, 0x00 }
	, m_port_input{ 0xff, 0xff, 0xff, 0xff }
	, m_port_latch{ 0xff, 0xff, 0xff, 0xff }
	, m_port_ddr{ 0x00, 0x00, 0x00, 0x00 }
	, m_port_irq_state(false)
	, m_irq_line_state(false)
	, m_irq_latch(0)
	, m_tcmp_cb(*this)
	, m_tcap_state(false)
	, m_tcr(0x00)
	, m_tsr(0x00), m_tsr_seen(0x00)
	, m_prescaler(0x00)
	, m_counter(0xfffc), m_icr(0x0000), m_ocr(0x0000)
	, m_inhibit_cap(false), m_inhibit_cmp(false)
	, m_trl_buf{ 0xfc, 0xfc }
	, m_trl_latched{ false, false }
	, m_pcop_cnt(0)
	, m_ncop_cnt(0)
	, m_coprst(0x00)
	, m_copcr(0x00)
	, m_ncope(0)
{
}


void m68hc05_device::set_port_bits(std::array<u8, PORT_COUNT> const &bits)
{
	if (configured() || started())
		throw emu_fatalerror("Attempt to set physical port bits after configuration");

	for (unsigned i = 0; PORT_COUNT > i; ++i)
		m_port_bits[i] = bits[i];
}

void m68hc05_device::set_port_interrupt(std::array<u8, PORT_COUNT> const &interrupt)
{
	u8 diff(0x00);
	for (unsigned i = 0; PORT_COUNT > i; ++i)
	{
		diff |= (m_port_interrupt[i] ^ interrupt[i]) & ~m_port_ddr[i];
		m_port_interrupt[i] = interrupt[i];
		if (interrupt[i] && !m_port_cb_r[i].isnull())
			logerror("PORT%c has interrupts enabled with pulled inputs, behaviour may be incorrect\n", 'A' + i);
	}
	if (diff) update_port_irq();
}

u8 m68hc05_device::port_read(offs_t offset)
{
	offset &= PORT_COUNT - 1;
	if (!machine().side_effects_disabled() && !m_port_cb_r[offset].isnull())
	{
		u8 const newval(m_port_cb_r[offset](0, ~m_port_ddr[offset] & m_port_bits[offset]) & m_port_bits[offset]);
		u8 const diff(newval ^ m_port_input[offset]);
		if (diff)
		{
			LOGIOPORT("read PORT%c: new input = %02X & %02X (was %02X)\n",
					char('A' + offset), newval, ~m_port_ddr[offset] & m_port_bits[offset], m_port_input[offset]);
		}
		m_port_input[offset] = newval;
		if (diff & m_port_interrupt[offset] & ~m_port_ddr[offset])
			update_port_irq();
	}
	return port_value(offset);
}

void m68hc05_device::port_latch_w(offs_t offset, u8 data)
{
	offset &= PORT_COUNT - 1;
	data &= m_port_bits[offset];
	u8 const diff = m_port_latch[offset] ^ data;
	if (diff)
	{
		LOGIOPORT("write PORT%c latch: %02X & %02X (was %02X)\n",
				char('A' + offset), data, m_port_ddr[offset], m_port_latch[offset]);
	}
	m_port_latch[offset] = data;
	if (diff & m_port_ddr[offset])
		m_port_cb_w[offset](0, port_value(offset), m_port_ddr[offset]);
}

u8 m68hc05_device::port_ddr_r(offs_t offset)
{
	return m_port_ddr[offset & (PORT_COUNT - 1)];
}

void m68hc05_device::port_ddr_w(offs_t offset, u8 data)
{
	offset &= PORT_COUNT - 1;
	data &= m_port_bits[offset];
	u8 const diff(data ^ m_port_ddr[offset]);
	if (diff)
	{
		LOGIOPORT("write DDR%c: %02X (was %02X)\n", char('A' + offset), data, m_port_ddr[offset]);
		m_port_ddr[offset] = data;
		if (diff & m_port_interrupt[offset])
		{
			if (!m_port_cb_r[offset].isnull())
			{
				u8 const newval(m_port_cb_r[offset](0, ~m_port_ddr[offset] & m_port_bits[offset]) & m_port_bits[offset]);
				u8 const diff(newval ^ m_port_input[offset]);
				if (diff)
				{
					LOGIOPORT("read PORT%c: new input = %02X & %02X (was %02X)\n",
							char('A' + offset), newval, ~m_port_ddr[offset] & m_port_bits[offset], m_port_input[offset]);
				}
				m_port_input[offset] = newval;
			}
			update_port_irq();
		}
		m_port_cb_w[offset](0, port_value(offset), m_port_ddr[offset]);
	}
}


u8 m68hc05_device::tcr_r()
{
	return m_tcr;
}

void m68hc05_device::tcr_w(u8 data)
{
	data &= 0xe3;
	LOGTIMER("write TCR: ICIE=%u OCIE=%u TOIE=%u IEDG=%u OLVL=%u\n",
			BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 1), BIT(data, 0));
	m_tcr = data;
	if (m_tcr & m_tsr & 0xe0)
		m_pending_interrupts |= M68HC05_INT_TIMER;
	else
		m_pending_interrupts &= ~M68HC05_INT_TIMER;
}

u8 m68hc05_device::tsr_r()
{
	if (!machine().side_effects_disabled())
	{
		u8 const events(m_tsr & ~m_tsr_seen);
		if (events)
		{
			LOGTIMER("read TSR: seen%s%s%s\n",
					BIT(events, 7) ? " ICF" : "", BIT(events, 6) ? " OCF" : "", BIT(events, 5) ? " TOF" : "");
		}
		m_tsr_seen = m_tsr;
	}
	return m_tsr;
}

u8 m68hc05_device::icr_r(offs_t offset)
{
	// reading IRCH inhibits capture until ICRL is read
	// reading ICRL after reading TCR with ICF set clears ICF

	u8 const low(BIT(offset, 0));
	if (!machine().side_effects_disabled())
	{
		if (low)
		{
			if (BIT(m_tsr_seen, 7))
			{
				LOGTIMER("read ICRL, clear ICF\n");
				m_tsr &= 0x7f;
				m_tsr_seen &= 0x7f;
				if (!(m_tcr & m_tsr & 0xe0)) m_pending_interrupts &= ~M68HC05_INT_TIMER;
			}
			if (m_inhibit_cap) LOGTIMER("read ICRL, enable capture\n");
			m_inhibit_cap = false;
		}
		else
		{
			if (!m_inhibit_cap) LOGTIMER("read ICRH, inhibit capture\n");
			m_inhibit_cap = true;
		}
	}
	return u8(m_icr >> (low ? 0 : 8));
}

u8 m68hc05_device::ocr_r(offs_t offset)
{
	// reading OCRL after reading TCR with OCF set clears OCF

	u8 const low(BIT(offset, 0));
	if (!machine().side_effects_disabled() && low && BIT(m_tsr_seen, 6))
	{
		LOGTIMER("read OCRL, clear OCF\n");
		m_tsr &= 0xbf;
		m_tsr_seen &= 0xbf;
		if (!(m_tcr & m_tsr & 0xe0)) m_pending_interrupts &= ~M68HC05_INT_TIMER;
	}
	return u8(m_ocr >> (low ? 0 : 8));
}

void m68hc05_device::ocr_w(offs_t offset, u8 data)
{
	// writing OCRH inhibits compare until OCRL is written
	// writing OCRL after reading TCR with OCF set clears OCF

	u8 const low(BIT(offset, 0));
	if (!machine().side_effects_disabled())
	{
		if (low)
		{
			if (BIT(m_tsr_seen, 6))
			{
				LOGTIMER("write OCRL, clear OCF\n");
				m_tsr &= 0xbf;
				m_tsr_seen &= 0xbf;
				if (!(m_tcr & m_tsr & 0xe0)) m_pending_interrupts &= ~M68HC05_INT_TIMER;
			}
			if (m_inhibit_cmp) LOGTIMER("write OCRL, enable compare\n");
			m_inhibit_cmp = false;
		}
		else
		{
			if (!m_inhibit_cmp) LOGTIMER("write OCRH, inhibit compare\n");
			m_inhibit_cmp = true;
		}
	}
	m_ocr = (m_ocr & (low ? 0xff00 : 0x00ff)) | (u16(data) << (low ? 0 : 8));
}

u8 m68hc05_device::timer_r(offs_t offset)
{
	// reading [A]TRH returns current counter MSB and latches [A]TRL buffer
	// reading [A]TRL returns current [A]TRL buffer and completes read sequence
	// reading TRL after reading TSR with TOF set clears TOF
	// reading ATRL doesn't affect TOF

	u8 const low(BIT(offset, 0));
	u8 const alt(BIT(offset, 1));
	if (low)
	{
		if (!machine().side_effects_disabled())
		{
			if (m_trl_latched[alt]) LOGTIMER("read %sTRL, read sequence complete\n", alt ? "A" : "");
			m_trl_latched[alt] = false;
			if (!alt && BIT(m_tsr_seen, 5))
			{
				LOGTIMER("read TRL, clear TOF\n");
				m_tsr &= 0xdf;
				m_tsr_seen &= 0xdf;
				if (!(m_tcr & m_tsr & 0xe0)) m_pending_interrupts &= ~M68HC05_INT_TIMER;
			}
		}
		return m_trl_buf[alt];
	}
	else
	{
		if (!machine().side_effects_disabled() && !m_trl_latched[alt])
		{
			LOGTIMER("read %sTRH, latch %sTRL\n", alt ? "A" : "", alt ? "A" : "");
			m_trl_latched[alt] = true;
			m_trl_buf[alt] = u8(m_counter);
		}
		return u8(m_counter >> 8);
	}
}


void m68hc05_device::coprst_w(u8 data)
{
	LOGCOP("write COPRST=%02x%s\n", data, ((0xaa == data) && (0x55 == m_coprst)) ? ", reset" : "");
	if (0x55 == data)
	{
		m_coprst = data;
	}
	else if (0xaa == data)
	{
		if (0x55 == m_coprst) m_pcop_cnt &= 0x00007fff;
		m_coprst = data;
	}
}

u8 m68hc05_device::copcr_r()
{
	u8 const result(m_copcr);
	if (!machine().side_effects_disabled())
	{
		if (copcr_copf()) LOGCOP("read COPCR, clear COPF\n");
		m_copcr &= 0xef;
	}
	return result;
}

void m68hc05_device::copcr_w(u8 data)
{
	LOGCOP("write COPCR: CME=%u PCOPE=%u [%s] CM=%u\n",
			BIT(data, 3), BIT(data, 2), (!copcr_pcope() && BIT(data, 2)) ? "set" : "ignored", data & 0x03);
	m_copcr = (m_copcr & 0xf4) | (data & 0x0f); // PCOPE is set-only, hence the mask overlap
}

void m68hc05_device::copr_w(u8 data)
{
	LOGCOP("write COPR: COPC=%u\n", BIT(data, 0));
	if (!BIT(data, 0)) m_ncop_cnt = 0;
}


void m68hc05_device::device_start()
{
	m6805_base_device::device_start();

	// resolve callbacks
	m_port_cb_r.resolve_all();
	m_port_cb_w.resolve_all_safe();
	m_tcmp_cb.resolve_safe();

	// save digital I/O
	save_item(NAME(m_port_interrupt));
	save_item(NAME(m_port_input));
	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_ddr));
	save_item(NAME(m_port_irq_state));
	save_item(NAME(m_irq_line_state));
	save_item(NAME(m_irq_latch));

	// save timer/counter
	save_item(NAME(m_tcap_state));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_tsr_seen));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_counter));
	save_item(NAME(m_icr));
	save_item(NAME(m_ocr));
	save_item(NAME(m_inhibit_cap));
	save_item(NAME(m_inhibit_cmp));
	save_item(NAME(m_trl_buf));
	save_item(NAME(m_trl_latched));

	// save COP watchdogs
	save_item(NAME(m_pcop_cnt));
	save_item(NAME(m_ncop_cnt));
	save_item(NAME(m_coprst));
	save_item(NAME(m_copcr));
	save_item(NAME(m_ncope));

	// digital I/O state unaffected by reset
	std::fill(std::begin(m_port_interrupt), std::end(m_port_interrupt), 0x00);
	std::fill(std::begin(m_port_input), std::end(m_port_input), 0xff);
	std::fill(std::begin(m_port_latch), std::end(m_port_latch), 0xff);
	m_irq_line_state = false;

	// timer state unaffected by reset
	m_tcap_state = false;
	m_tcr = 0x00;
	m_tsr = 0x00;
	m_icr = 0x0000;
	m_ocr = 0x0000;

	// COP watchdog state unaffected by reset
	m_pcop_cnt = 0;
	m_coprst = 0x00;
	m_copcr = 0x00;
	m_ncope = 0;

	// expose most basic state to debugger
	state_add(M68HC05_IRQLATCH, "IRQLATCH", m_irq_latch).mask(0x01);
}

void m68hc05_device::device_reset()
{
	m6805_base_device::device_reset();

	// digital I/O reset
	std::fill(std::begin(m_port_ddr), std::end(m_port_ddr), 0x00);
	m_irq_latch = 0;
	update_port_irq();

	// timer reset
	m_tcr &= 0x02;
	m_tsr_seen = 0x00;
	m_prescaler = 0;
	m_counter = 0xfffc;
	m_inhibit_cap = m_inhibit_cmp = false;
	m_trl_buf[0] = m_trl_buf[1] = u8(m_counter);
	m_trl_latched[0] = m_trl_latched[1] = false;

	// COP watchdog reset
	m_ncop_cnt = 0;
	m_copcr &= 0x10;
}


void m68hc05_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case M68HC05_IRQ_LINE:
		if ((CLEAR_LINE != state) && !m_irq_line_state)
		{
			LOGINT("/IRQ edge%s\n", (m_port_irq_state || m_irq_latch) ? "" : ", set IRQ latch");
			if (!m_port_irq_state)
			{
				m_irq_latch = 1;
				m_pending_interrupts |= M68HC05_INT_IRQ;
			}
		}
		m_irq_line_state = ASSERT_LINE == state;
		break;
	case M68HC05_TCAP_LINE:
		if ((bool(state) != m_tcap_state) && (bool(state) == tcr_iedg()))
		{
			LOGTIMER("input capture %04X%s\n", m_counter, m_inhibit_cap ? " [inhibited]" : "");
			if (!m_inhibit_cap)
			{
				m_tsr |= 0x80;
				m_icr = m_counter;
				if (m_tcr & m_tsr & 0xe0) m_pending_interrupts |= M68HC05_INT_TIMER;
			}
		}
		m_tcap_state = bool(state);
		break;
	default:
		fatalerror("m68hc05[%s]: unknown input line %d", tag(), inputnum);
	}
}

u64 m68hc05_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 1) / 2;
}

u64 m68hc05_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 2;
}

std::unique_ptr<util::disasm_interface> m68hc05_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>();
}


void m68hc05_device::interrupt()
{
	if ((m_pending_interrupts & M68HC05_INT_MASK) && !(CC & IFLAG))
	{
		if (m_params.m_addr_width > 13) {
			pushword<true>(m_pc);
			pushbyte<true>(m_x);
			pushbyte<true>(m_a);
			pushbyte<true>(m_cc);
		}
		else
		{
			pushword<false>(m_pc);
			pushbyte<false>(m_x);
			pushbyte<false>(m_a);
			pushbyte<false>(m_cc);
		}
		SEI;
		standard_irq_callback(0);

		if (m_pending_interrupts & M68HC05_INT_IRQ)
		{
			LOGINT("servicing external interrupt\n");
			m_irq_latch = 0;
			m_pending_interrupts &= ~M68HC05_INT_IRQ;
			if (m_params.m_addr_width > 13)
				rm16<true>(M68HC05_VECTOR_IRQ & m_params.m_vector_mask, m_pc);
			else
				rm16<false>(M68HC05_VECTOR_IRQ & m_params.m_vector_mask, m_pc);
		}
		else if (m_pending_interrupts & M68HC05_INT_TIMER)
		{
			LOGINT("servicing timer interrupt\n");
			if (m_params.m_addr_width > 13)
				rm16<true>(M68HC05_VECTOR_TIMER & m_params.m_vector_mask, m_pc);
			else
				rm16<false>(M68HC05_VECTOR_TIMER & m_params.m_vector_mask, m_pc);
		}
		else
		{
			fatalerror("m68hc05[%s]: unknown pending interrupt(s) %x", tag(), m_pending_interrupts);
		}
		m_icount -= 10;
		burn_cycles(10);
	}
}

bool m68hc05_device::test_il()
{
	return m_irq_line_state;
}

void m68hc05_device::burn_cycles(unsigned count)
{
	// calculate new timer values (fixed prescaler of four)
	unsigned const ps_opt(4);
	unsigned const ps_mask((1 << ps_opt) - 1);
	unsigned const increments((count + (m_prescaler & ps_mask)) >> ps_opt);
	u32 const new_counter(u32(m_counter) + increments);
	bool const timer_rollover((0x010000 > m_counter) && (0x010000 <= new_counter));
	bool const output_compare_match((m_ocr > m_counter) && (m_ocr <= new_counter));
	m_prescaler = (count + m_prescaler) & ps_mask;
	m_counter = u16(new_counter);
	if (timer_rollover)
	{
		LOGTIMER("timer rollover\n");
		m_tsr |= 0x20;
	}
	if (output_compare_match)
	{
		LOGTIMER("output compare match %s\n", m_inhibit_cmp ? " [inhibited]" : "");
		if (!m_inhibit_cmp)
		{
			m_tsr |= 0x40;
			m_tcmp_cb(tcr_olvl() ? 1 : 0);
		}
	}
	if (m_tcr & m_tsr & 0xe0) m_pending_interrupts |= M68HC05_INT_TIMER;

	// run programmable COP
	u32 const pcop_timeout(u32(1) << ((copcr_cm() << 1) + 15));
	if (copcr_pcope() && (pcop_timeout <= ((m_pcop_cnt & (pcop_timeout - 1)) + count)))
	{
		LOGCOP("PCOP reset\n");
		m_copcr |= 0x10;
		pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
	m_pcop_cnt = (m_pcop_cnt + count) & ((u32(1) << 21) - 1);

	// run non-programmable COP
	m_ncop_cnt += count;
	if (m_ncope && ((u32(1) << 17) <= m_ncop_cnt))
	{
		pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		LOGCOP("NCOP reset\n");
	}
	m_ncop_cnt &= (u32(1) << 17) - 1;
}


void m68hc05_device::add_port_state(std::array<bool, PORT_COUNT> const &ddr)
{
	for (unsigned i = 0; PORT_COUNT > i; ++i)
	{
		if (m_port_bits[i])
			state_add(M68HC05_LATCHA + i, util::string_format("LATCH%c", 'A' + i).c_str(), m_port_latch[i]).mask(m_port_bits[i]);
	}
	for (unsigned i = 0; PORT_COUNT > i; ++i)
	{
		if (ddr[i] && m_port_bits[i])
			state_add(M68HC05_DDRA + i, util::string_format("DDR%c", 'A' + i).c_str(), m_port_ddr[i]).mask(m_port_bits[i]);
	}
}

void m68hc05_device::add_timer_state()
{
	state_add(M68HC05_TCR, "TCR", m_tcr).mask(0x7f);
	state_add(M68HC05_TSR, "TSR", m_tsr).mask(0xff);
	state_add(M68HC05_ICR, "ICR", m_icr).mask(0xffff);
	state_add(M68HC05_OCR, "OCR", m_ocr).mask(0xffff);
	state_add(M68HC05_PS, "PS", m_prescaler).mask(0x03);
	state_add(M68HC05_TR, "TR", m_counter).mask(0xffff);
}

void m68hc05_device::add_pcop_state()
{
	state_add(M68HC05_COPRST, "COPRST", m_coprst).mask(0xff);
	state_add(M68HC05_COPCR, "COPCR", m_copcr).mask(0x1f);
	state_add(M68HC05_PCOP, "PCOP", m_pcop_cnt).mask(0x001fffff);
}

void m68hc05_device::add_ncop_state()
{
	state_add(M68HC05_NCOPE, "NCOPE", m_ncope).mask(0x01);
	state_add(M68HC05_NCOP, "NCOP", m_ncop_cnt).mask(0x0001ffff);
}


u8 m68hc05_device::port_value(unsigned offset) const
{
	return (m_port_latch[offset] & m_port_ddr[offset]) | (m_port_input[offset] & ~m_port_ddr[offset]);
}

void m68hc05_device::update_port_irq()
{
	u8 state(0x00);
	for (unsigned i = 0; i < PORT_COUNT; ++i)
		state |= m_port_interrupt[i] & ~m_port_ddr[i] & ~m_port_input[i];

	if (bool(state) != m_port_irq_state)
	{
		LOGINT("I/O port IRQ state now %u%s\n",
				state ? 1 : 0, (!m_irq_line_state && state && !m_irq_latch) ? ", set IRQ latch" : "");
		m_port_irq_state = bool(state);
		if (!m_irq_line_state && state)
		{
			m_irq_latch = 1;
			m_pending_interrupts |= M68HC05_INT_IRQ;
		}
	}
}



/****************************************************************************
 * M68HC705 base device
 ****************************************************************************/

m68hc705_device::m68hc705_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock,
		device_type type,
		u32 addr_width,
		address_map_constructor internal_map)
	: m68hc05_device(mconfig, tag, owner, clock, type, addr_width, (1U << addr_width) - 1, internal_map)
{
}



/****************************************************************************
 * MC68HC05C4 device
 ****************************************************************************/

void m68hc05c4_device::c4_map(address_map &map)
{
	map.global_mask(0x1fff);
	map.unmap_value_high();

	map(0x0000, 0x0003).rw(FUNC(m68hc05c4_device::port_read), FUNC(m68hc05c4_device::port_latch_w));
	map(0x0004, 0x0006).rw(FUNC(m68hc05c4_device::port_ddr_r), FUNC(m68hc05c4_device::port_ddr_w));
	// 0x0007-0x0009 unused
	// 0x000a SPCR
	// 0x000b SPSR
	// 0x000c SPDR
	// 0x000d BAUD
	// 0x000e SCCR1
	// 0x000f SCCR2
	// 0x0010 SCSR
	// 0x0011 SCDR
	map(0x0012, 0x0012).rw(FUNC(m68hc05c4_device::tcr_r), FUNC(m68hc05c4_device::tcr_w));
	map(0x0013, 0x0013).r(FUNC(m68hc05c4_device::tsr_r));
	map(0x0014, 0x0015).r(FUNC(m68hc05c4_device::icr_r));
	map(0x0016, 0x0017).rw(FUNC(m68hc05c4_device::ocr_r), FUNC(m68hc05c4_device::ocr_w));
	map(0x0018, 0x001b).r(FUNC(m68hc05c4_device::timer_r));
	// 0x001c-0x001f unused
	map(0x0020, 0x004f).rom(); // user ROM
	map(0x0050, 0x00ff).ram(); // RAM/stack
	map(0x0100, 0x10ff).rom(); // user ROM
	// 0x1100-0x1eff unused
	map(0x1f00, 0x1fef).rom(); // self-check
	// 0x1ff0-0x1ff3 unused
	map(0x1ff4, 0x1fff).rom(); // user vectors
}


m68hc05c4_device::m68hc05c4_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m68hc05_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC05C4,
			13,
			0x1fff,
			address_map_constructor(FUNC(m68hc05c4_device::c4_map), this))
{
	set_port_bits(std::array<u8, PORT_COUNT>{{ 0xff, 0xff, 0xff, 0xbf }});
}



void m68hc05c4_device::device_start()
{
	m68hc05_device::device_start();

	add_port_state(std::array<bool, PORT_COUNT>{{ true, true, true, false }});
	add_timer_state();
}


std::unique_ptr<util::disasm_interface> m68hc05c4_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>(m68hc05c4_syms);
}



/****************************************************************************
 * MC68HC05C8 device
 ****************************************************************************/

void m68hc05c8_device::c8_map(address_map &map)
{
	map.global_mask(0x1fff);
	map.unmap_value_high();

	map(0x0000, 0x0003).rw(FUNC(m68hc05c8_device::port_read), FUNC(m68hc05c8_device::port_latch_w));
	map(0x0004, 0x0006).rw(FUNC(m68hc05c8_device::port_ddr_r), FUNC(m68hc05c8_device::port_ddr_w));
	// 0x0007-0x0009 unused
	// 0x000a SPCR
	// 0x000b SPSR
	// 0x000c SPDR
	// 0x000d BAUD
	// 0x000e SCCR1
	// 0x000f SCCR2
	// 0x0010 SCSR
	// 0x0011 SCDR
	map(0x0012, 0x0012).rw(FUNC(m68hc05c8_device::tcr_r), FUNC(m68hc05c8_device::tcr_w));
	map(0x0013, 0x0013).r(FUNC(m68hc05c8_device::tsr_r));
	map(0x0014, 0x0015).r(FUNC(m68hc05c8_device::icr_r));
	map(0x0016, 0x0017).rw(FUNC(m68hc05c8_device::ocr_r), FUNC(m68hc05c8_device::ocr_w));
	map(0x0018, 0x001b).r(FUNC(m68hc05c8_device::timer_r));
	// 0x001c-0x001f unused
	map(0x0020, 0x004f).rom(); // user ROM
	map(0x0050, 0x00ff).ram(); // RAM/stack
	map(0x0100, 0x1eff).rom(); // user ROM
	map(0x1f00, 0x1fef).rom(); // self-check
	// 0x1ff0-0x1ff3 unused
	map(0x1ff4, 0x1fff).rom(); // user vectors
}


m68hc05c8_device::m68hc05c8_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m68hc05_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC05C8,
			13,
			0x1fff,
			address_map_constructor(FUNC(m68hc05c8_device::c8_map), this))
{
	set_port_bits(std::array<u8, PORT_COUNT>{{ 0xff, 0xff, 0xff, 0xbf }});
}


void m68hc05c8_device::device_start()
{
	m68hc05_device::device_start();

	add_port_state(std::array<bool, PORT_COUNT>{{ true, true, true, false }});
	add_timer_state();
}


std::unique_ptr<util::disasm_interface> m68hc05c8_device::create_disassembler()
{
	// same I/O registers as MC68HC05C4
	return std::make_unique<m68hc05_disassembler>(m68hc05c4_syms);
}



/****************************************************************************
 * MC68HC705C4A device
 ****************************************************************************/

void m68hc705c4a_device::c4a_map(address_map &map)
{
	map.global_mask(0x1fff);
	map.unmap_value_high();

	map(0x0000, 0x0003).rw(FUNC(m68hc705c4a_device::port_read), FUNC(m68hc705c4a_device::port_latch_w));
	map(0x0004, 0x0006).rw(FUNC(m68hc705c4a_device::port_ddr_r), FUNC(m68hc705c4a_device::port_ddr_w));
	// 0x0007-0x0009 unused
	// 0x000a SPCR
	// 0x000b SPSR
	// 0x000c SPDR
	// 0x000d BAUD
	// 0x000e SCCR1
	// 0x000f SCCR2
	// 0x0010 SCSR
	// 0x0011 SCDR
	map(0x0012, 0x0012).rw(FUNC(m68hc705c4a_device::tcr_r), FUNC(m68hc705c4a_device::tcr_w));
	map(0x0013, 0x0013).r(FUNC(m68hc705c4a_device::tsr_r));
	map(0x0014, 0x0015).r(FUNC(m68hc705c4a_device::icr_r));
	map(0x0016, 0x0017).rw(FUNC(m68hc705c4a_device::ocr_r), FUNC(m68hc705c4a_device::ocr_w));
	map(0x0018, 0x001b).r(FUNC(m68hc705c4a_device::timer_r));
	// 0x001c PROG
	// 0x001d-0x001f unused
	map(0x0020, 0x004f).rom();                                 // user PROM
	map(0x0050, 0x00ff).ram();                                 // RAM/stack
	map(0x0100, 0x10ff).rom();                                 // user PROM
	// 0x1100-0x1eff unused
	map(0x1f00, 0x1fde).rom().region("bootstrap", 0x0000);  // bootloader
	map(0x1fdf, 0x1fdf).lw8(NAME([this] (u8 data) { m_option = data; }));
	map(0x1fe0, 0x1fef).rom().region("bootstrap", 0x00e0);  // boot ROM vectors
	map(0x1ff0, 0x1ff0).w(FUNC(m68hc705c4a_device::copr_w));
	map(0x1ff0, 0x1fff).rom();                                 // user vectors
}


m68hc705c4a_device::m68hc705c4a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m68hc705_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC705C4A,
			13,
			address_map_constructor(FUNC(m68hc705c4a_device::c4a_map), this))
	, m_rom(*this, DEVICE_SELF)
{
	set_port_bits(std::array<u8, PORT_COUNT>{{ 0xff, 0xff, 0xff, 0xbf }});
}


tiny_rom_entry const *m68hc705c4a_device::device_rom_region() const
{
	return ROM_NAME(m68hc705c4a);
}


void m68hc705c4a_device::device_start()
{
	m68hc705_device::device_start();

	add_port_state(std::array<bool, PORT_COUNT>{{ true, true, true, false }});
	add_timer_state();
	add_ncop_state();

	state_add(M68HC705C8A_OPTION, "OPTION", m_option).mask(0xff);

	save_item(NAME(m_option));
}

void m68hc705c4a_device::device_reset()
{
	m68hc705_device::device_reset();

	// latch MOR registers on reset
	set_port_interrupt(std::array<u8, PORT_COUNT>{{ 0x00, u8(rdmem<false>(0xfff0)), 0x00, 0x00 }});
	set_ncope(BIT(rdmem<false>(0xfff1), 0));

	// IRQ negative edge and level sensitive
	m_option = 0x02;
}


std::unique_ptr<util::disasm_interface> m68hc705c4a_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>(m68hc705c8a_syms);
}




/****************************************************************************
 * MC68HC705C8A device
 ****************************************************************************/

void m68hc705c8a_device::c8a_map(address_map &map)
{
	map.global_mask(0x1fff);
	map.unmap_value_high();

	map(0x0000, 0x0003).rw(FUNC(m68hc705c8a_device::port_read), FUNC(m68hc705c8a_device::port_latch_w));
	map(0x0004, 0x0006).rw(FUNC(m68hc705c8a_device::port_ddr_r), FUNC(m68hc705c8a_device::port_ddr_w));
	// 0x0007-0x0009 unused
	// 0x000a SPCR
	// 0x000b SPSR
	// 0x000c SPDR
	// 0x000d BAUD
	// 0x000e SCCR1
	// 0x000f SCCR2
	// 0x0010 SCSR
	// 0x0011 SCDR
	map(0x0012, 0x0012).rw(FUNC(m68hc705c8a_device::tcr_r), FUNC(m68hc705c8a_device::tcr_w));
	map(0x0013, 0x0013).r(FUNC(m68hc705c8a_device::tsr_r));
	map(0x0014, 0x0015).r(FUNC(m68hc705c8a_device::icr_r));
	map(0x0016, 0x0017).rw(FUNC(m68hc705c8a_device::ocr_r), FUNC(m68hc705c8a_device::ocr_w));
	map(0x0018, 0x001b).r(FUNC(m68hc705c8a_device::timer_r));
	// 0x001c PROG
	map(0x001d, 0x001d).w(FUNC(m68hc705c8a_device::coprst_w));
	map(0x001e, 0x001e).rw(FUNC(m68hc705c8a_device::copcr_r), FUNC(m68hc705c8a_device::copcr_w));
	// 0x001f unused
	map(0x0020, 0x004f).rw(FUNC(m68hc705c8a_device::ram0_r), FUNC(m68hc705c8a_device::ram0_w)); // PROM/RAM
	map(0x0050, 0x00ff).ram();                                 // RAM/stack
	map(0x0100, 0x015f).rw(FUNC(m68hc705c8a_device::ram1_r), FUNC(m68hc705c8a_device::ram1_w)); // PROM/RAM
	map(0x0160, 0x1eff).rom();                                 // user PROM
	map(0x1f00, 0x1fde).rom().region("bootstrap", 0x0000);  // bootloader
	map(0x1fdf, 0x1fdf).lw8(NAME([this] (u8 data) { m_option = data; }));
	map(0x1fe0, 0x1fef).rom().region("bootstrap", 0x00e0);  // boot ROM vectors
	map(0x1ff0, 0x1ff0).w(FUNC(m68hc705c8a_device::copr_w));
	map(0x1ff0, 0x1fff).rom();                                 // user vectors
}


m68hc705c8a_device::m68hc705c8a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m68hc705_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC705C8A,
			13,
			address_map_constructor(FUNC(m68hc705c8a_device::c8a_map), this))
	, m_rom(*this, DEVICE_SELF)
{
	set_port_bits(std::array<u8, PORT_COUNT>{{ 0xff, 0xff, 0xff, 0xbf }});
}


tiny_rom_entry const *m68hc705c8a_device::device_rom_region() const
{
	return ROM_NAME(m68hc705c8a);
}


void m68hc705c8a_device::device_start()
{
	m68hc705_device::device_start();

	add_port_state(std::array<bool, PORT_COUNT>{{ true, true, true, false }});
	add_timer_state();
	add_pcop_state();
	add_ncop_state();

	state_add(M68HC705C8A_OPTION, "OPTION", m_option).mask(0xff);

	save_item(NAME(m_ram));
	save_item(NAME(m_option));

	// clear RAM
	std::fill(std::begin(m_ram), std::end(m_ram), 0x00);
}

void m68hc705c8a_device::device_reset()
{
	m68hc705_device::device_reset();

	// latch MOR registers on reset
	set_port_interrupt(std::array<u8, PORT_COUNT>{{ 0x00, u8(rdmem<false>(0xfff0)), 0x00, 0x00 }});
	set_ncope(BIT(rdmem<false>(0xfff1), 0));

	// RAM disabled, IRQ negative edge and level sensitive
	m_option = 0x02;
}


std::unique_ptr<util::disasm_interface> m68hc705c8a_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>(m68hc705c8a_syms);
}


u8 m68hc705c8a_device::ram0_r(offs_t offset)
{
	if (BIT(m_option, 7))
		return offset >= 0x10 ? m_ram[offset - 0x10] : 0x00; // 20-2f reserved
	else
		return m_rom[0x20 + offset];
}

void m68hc705c8a_device::ram0_w(offs_t offset, u8 data)
{
	if (BIT(m_option, 7) && offset >= 0x10)
		m_ram[offset - 0x10] = data;
}

u8 m68hc705c8a_device::ram1_r(offs_t offset)
{
	if (BIT(m_option, 6))
		return m_ram[0x20 + offset];
	else
		return m_rom[0x100 + offset];
}

void m68hc705c8a_device::ram1_w(offs_t offset, u8 data)
{
	if (BIT(m_option, 6))
		m_ram[0x20 + offset] = data;
}



/****************************************************************************
 * MC68HC05J1A device
 ****************************************************************************/

void m68hc705j1a_device::j1a_map(address_map &map)
{
	map.global_mask(0x07ff);
	map.unmap_value_high();

	map(0x0000, 0x0001).rw(FUNC(m68hc705j1a_device::port_read), FUNC(m68hc705j1a_device::port_latch_w));
	map(0x0004, 0x0005).rw(FUNC(m68hc705j1a_device::port_ddr_r), FUNC(m68hc705j1a_device::port_ddr_w));
	// 0x0008 TSCR (bits 7 and 6 are read-only; bits 3 and 2 are write-only)
	// 0x0009 TCR (read-only)
	// 0x000a ISCR (bits 7 and 3 are readable; bits 7, 4 and 1 are writeable)
	// 0x0010 PDRA (write-only)
	// 0x0011 PDRB (write-only)
	// 0x0014 EPROG
	// 0x001f reserved
	map(0x00c0, 0x00ff).ram();
	map(0x0300, 0x07cf).rom(); // EPROM
	map(0x07ee, 0x07ef).rom(); // test ROM
	map(0x07f0, 0x07f0).w(FUNC(m68hc705j1a_device::copr_w));
	map(0x07f1, 0x07f1).rom(); // MOR
	// 0x07f2-0x07f7 reserved
	map(0x07f8, 0x07ff).rom(); // user vectors
}


m68hc705j1a_device::m68hc705j1a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m68hc705_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC705J1A,
			11,
			address_map_constructor(FUNC(m68hc705j1a_device::j1a_map), this))
{
	set_port_bits(std::array<u8, PORT_COUNT>{{ 0xff, 0x3f, 0x00, 0x00 }});
}


void m68hc705j1a_device::device_start()
{
	m68hc705_device::device_start();

	add_port_state(std::array<bool, PORT_COUNT>{{ true, true, false, false }});
	add_ncop_state();
}

void m68hc705j1a_device::device_reset()
{
	m68hc705_device::device_reset();

	// latch MOR register on reset
	set_ncope(BIT(rdmem<false>(0x07f1), 0)); // FIXME: this is more like C8A's PCOP
}


std::unique_ptr<util::disasm_interface> m68hc705j1a_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>(m68hc705j1a_syms);
}



/****************************************************************************
 * MC68HC05L9 device
 ****************************************************************************/

void m68hc05l9_device::l9_map(address_map &map)
{
	map.global_mask(0xffff);
	map.unmap_value_high();

	map(0x0000, 0x0003).rw(FUNC(m68hc05l9_device::port_read), FUNC(m68hc05l9_device::port_latch_w));
	map(0x0004, 0x0007).rw(FUNC(m68hc05l9_device::port_ddr_r), FUNC(m68hc05l9_device::port_ddr_w));
	// 0x0008 count down
	// 0x0009-0x000a configuration
	// 0x000b minute alarm
	// 0x000c hour alarm
	// 0x000d BAUD
	// 0x000e SCCR1
	// 0x000f SCCR2
	// 0x0010 SCSR
	// 0x0011 SCDR
	map(0x0012, 0x0012).rw(FUNC(m68hc05l9_device::tcr_r), FUNC(m68hc05l9_device::tcr_w));
	map(0x0013, 0x0013).r(FUNC(m68hc05l9_device::tsr_r));
	map(0x0014, 0x0015).r(FUNC(m68hc05l9_device::icr_r));
	map(0x0016, 0x0017).rw(FUNC(m68hc05l9_device::ocr_r), FUNC(m68hc05l9_device::ocr_w));
	map(0x0018, 0x001b).r(FUNC(m68hc05l9_device::timer_r));
	// 0x001c RTC status and clock control
	// 0x001d hours
	// 0x001e minutes
	// 0x001f seconds
	map(0x0020, 0x004f).rom(); // user ROM
	map(0x0050, 0x00ff).ram(); // RAM/stack
	// 0x0100-0x01ff unused
	map(0x0200, 0x027f).ram(); // display RAM (128x5)
	// 0x0280-0x048f reserved for slaves (528x5)
	// 0x0490-0x07ff unused
	map(0x0800, 0x1e69).rom(); // user ROM
	map(0x1e6a, 0x1fef).rom(); // self-test (vectors at 0x1fe0-0x1fef)
	map(0x1ff0, 0x1fff).rom(); // user vectors
	// 0x2000-0xffff external memory
}


m68hc05l9_device::m68hc05l9_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m68hc05_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC05L9,
			16,
			0x1fff,
			address_map_constructor(FUNC(m68hc05l9_device::l9_map), this))
{
	set_port_bits(std::array<u8, PORT_COUNT>{{ 0xff, 0xff, 0xff, 0x1f }});
}



void m68hc05l9_device::device_start()
{
	m68hc05_device::device_start();

	add_port_state(std::array<bool, PORT_COUNT>{{ true, true, true, false }});
	add_timer_state();
}


std::unique_ptr<util::disasm_interface> m68hc05l9_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>(m68hc05l9_syms);
}



/****************************************************************************
 * MC68HC05L11 device
 ****************************************************************************/

void m68hc05l11_device::l11_map(address_map &map)
{
	map(0x0000, 0x0003).rw(FUNC(m68hc05l11_device::port_read), FUNC(m68hc05l11_device::port_latch_w));
	// 0x0004 port E
	// 0x0005 port F
	map(0x0006, 0x0008).rw(FUNC(m68hc05l11_device::port_ddr_r), FUNC(m68hc05l11_device::port_ddr_w));
	// 0x0009 port E direction
	// 0x000a port F direction
	// 0x000b minute alarm
	// 0x000c hour alarm
	// 0x000d BAUD
	// 0x000e SCCR1
	// 0x000f SCCR2
	// 0x0010 SCSR
	// 0x0011 SCDR
	map(0x0012, 0x0012).rw(FUNC(m68hc05l11_device::tcr_r), FUNC(m68hc05l11_device::tcr_w));
	map(0x0013, 0x0013).r(FUNC(m68hc05l11_device::tsr_r));
	map(0x0014, 0x0015).r(FUNC(m68hc05l11_device::icr_r));
	map(0x0016, 0x0017).rw(FUNC(m68hc05l11_device::ocr_r), FUNC(m68hc05l11_device::ocr_w));
	map(0x0018, 0x001b).r(FUNC(m68hc05l11_device::timer_r));
	// 0x001c-0x001d output compare 2
	// 0x001e reserved
	// 0x001f RTC interrupt status
	// 0x0020 count down
	// 0x0021 control 1
	// 0x0022 SPCR
	// 0x0023 SPSR
	// 0x0024 SPDR
	// 0x0025 control 2
	// 0x0026 TONEA
	// 0x0027 TONEB
	// 0x0028-0x0033 LCD registers
	// 0x0033 reserved
	// 0x0034-0x003b MMU registers
	// 0x003c hours
	// 0x003d minutes
	// 0x003e seconds
	// 0x003f reserved
	map(0x0040, 0x01ff).ram(); // RAM/stack
	// 0x0200-0x6fff external memory (common area)
	map(0x7000, 0x7dff).rom().region(DEVICE_SELF, 0); // user ROM
	map(0x7e00, 0x7fef).rom().region(DEVICE_SELF, 0xe00); // self-check (incl. vectors)
	map(0x7ff0, 0x7fff).rom().region(DEVICE_SELF, 0xff0); // user vectors
	// 0x8000-0x7fffff external memory (banked in four 8K segments)
}


m68hc05l11_device::m68hc05l11_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m68hc05_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC05L11,
			16, // FIXME: 16 logical mapped to 23 physical
			0x7fff,
			address_map_constructor(FUNC(m68hc05l11_device::l11_map), this))
{
	set_port_bits(std::array<u8, PORT_COUNT>{{ 0xff, 0xff, 0xff, 0xff }});
}



void m68hc05l11_device::device_start()
{
	m68hc05_device::device_start();

	add_port_state(std::array<bool, PORT_COUNT>{{ true, true, true, false }});
	add_timer_state();
}


std::unique_ptr<util::disasm_interface> m68hc05l11_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>(m68hc05c4_syms);
}
