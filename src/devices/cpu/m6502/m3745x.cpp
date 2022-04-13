// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*
    Mitsubishi M3745x 8-bit microcontroller family
*/

#include "emu.h"
#include "m3745x.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// Interrupt control bits (interpolated from C68 program; need 7450 Group manual badly)

#define IRQ1_INT1       (0x04)
#define IRQ1_INT2       (0x08)  // guess, not used in C68
#define IRQ1_INT3       (0x10)  // guess, not used in C68

#define IRQ2_SERIALRX   (0x08)
#define IRQ2_SERIALTX   (0x10)
#define IRQ2_ADC        (0x20)

#define ADCTRL_CH_MASK  (0x07)  // AD ctrl reg. channel mask
#define ADCTRL_COMPLETE (0x08)  // AD ctrl "start"/"complete" bit

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(M37450, m37450_device, "m37450", "Mitsubishi M37450")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m3745x_device - constructor
//-------------------------------------------------
m3745x_device::m3745x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map) :
	m740_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, internal_map),
	m_read_p(*this),
	m_write_p(*this),
	m_read_ad(*this),
	m_intreq1(0),
	m_intreq2(0),
	m_intctrl1(0),
	m_intctrl2(0),
	m_adctrl(0),
	m_last_all_ints(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m3745x_device::device_start()
{
	m_read_p.resolve_all_safe(0);
	m_write_p.resolve_all_safe();
	m_read_ad.resolve_all_safe(0);

	for (int i = 0; i < NUM_TIMERS; i++)
	{
		m_timers[i] = timer_alloc(i);
	}

	m740_device::device_start();

	save_item(NAME(m_ports));
	save_item(NAME(m_ddrs));
	save_item(NAME(m_intreq1));
	save_item(NAME(m_intreq2));
	save_item(NAME(m_intctrl1));
	save_item(NAME(m_intctrl2));
	save_item(NAME(m_adctrl));
	save_item(NAME(m_last_all_ints));

	// all ports reset to input on startup
	memset(m_ddrs, 0, sizeof(m_ddrs));
	memset(m_ports, 0, sizeof(m_ports));
	m_intreq1 = m_intreq2 = m_intctrl1 = m_intctrl2 = 0;
	m_adctrl = 0;
	m_last_all_ints = 0;
}

device_memory_interface::space_config_vector m3745x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m3745x_device::device_reset()
{
	m740_device::device_reset();

	SP = 0x01ff;    // we have the "traditional" stack in page 1, not 0 like some M740 derivatives

	for (auto & elem : m_timers)
	{
		elem->adjust(attotime::never);
	}

	// all ports reset to input on startup
	memset(m_ddrs, 0, sizeof(m_ddrs));
	memset(m_ports, 0, sizeof(m_ports));
	m_intreq1 = m_intreq2 = m_intctrl1 = m_intctrl2 = 0;
	m_adctrl = 0;
	m_last_all_ints = 0;
}

void m3745x_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TIMER_ADC:
			m_timers[TIMER_ADC]->adjust(attotime::never);

			m_adctrl |= ADCTRL_COMPLETE;
			m_intreq2 |= IRQ2_ADC;
			recalc_irqs();
			break;

		default:
			printf("M3775x: unknown timer expire %d\n", id);
			break;
	}
}

void m3745x_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case M3745X_INT1_LINE:
			if (state == ASSERT_LINE)
			{
				m_intreq1 |= IRQ1_INT1;
			}
			else
			{
				m_intreq1 &= ~IRQ1_INT1;
			}
			break;

		case M3745X_INT2_LINE:
			if (state == ASSERT_LINE)
			{
				m_intreq1 |= IRQ1_INT2;
			}
			else
			{
				m_intreq1 &= ~IRQ1_INT2;
			}
			break;

		case M3745X_INT3_LINE:
			if (state == ASSERT_LINE)
			{
				m_intreq1 |= IRQ1_INT3;
			}
			else
			{
				m_intreq1 &= ~IRQ1_INT3;
			}
			break;
	}

	recalc_irqs();
}

void m3745x_device::recalc_irqs()
{
	uint16_t all_ints;
	int static const irq_lines[16] =
	{
		-1, -1, -1, M740_INT11_LINE, M740_INT12_LINE, M740_INT13_LINE, -1, -1,
		-1, -1, M740_INT2_LINE, M740_INT3_LINE, M740_INT4_LINE, -1, -1, -1
	};

	all_ints = (m_intreq1 & m_intctrl1) << 8;
	all_ints |= (m_intreq2 & m_intctrl2);

//  printf("recalc_irqs: last_all_ints = %04x last_ints = %04x (req1 %02x ctrl1 %02x req2 %02x ctrl2 %02x)\n", all_ints, m_last_all_ints, m_intreq1, m_intctrl1, m_intreq2, m_intctrl2);

	// check all 16 IRQ bits for changes
	for (int i = 0; i < 16; i++)
	{
		// if bit is set now
		if (all_ints & (1 << i))
		{
			// and wasn't last time
			if (!(m_last_all_ints & (1 << i)))
			{
//              printf("    asserting irq %d (%d)\n", i, irq_lines[i]);
				if (irq_lines[i] != -1)
				{
					m740_device::execute_set_input(irq_lines[i], ASSERT_LINE);
				}
			}
		}
		else    // bit is clear now
		{
			// ...and wasn't clear last time
			if (m_last_all_ints & (1 << i))
			{
//              printf("    clearing irq %d (%d)\n", i, irq_lines[i]);
				if (irq_lines[i] != -1)
				{
					m740_device::execute_set_input(irq_lines[i], CLEAR_LINE);
				}
			}
		}
	}

	m_last_all_ints = all_ints;
}

void m3745x_device::send_port(uint8_t offset, uint8_t data)
{
	m_write_p[offset](data);
}

uint8_t m3745x_device::read_port(uint8_t offset)
{
	uint8_t incoming = m_read_p[offset]();

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// OR in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	return incoming;
}

uint8_t m3745x_device::ports_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return read_port(0);

		case 1:
			return m_ddrs[0];

		case 2:
			return read_port(1);

		case 4:
			return read_port(2);

		case 5:
			return m_ddrs[2];

		case 6:
			return read_port(3);

		case 7:
			return m_ddrs[3];
	}

	return 0xff;
}

void m3745x_device::ports_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // p3
			send_port(0, data & m_ddrs[0]);
			m_ports[0] = data;
			break;

		case 1: // p3 ddr
			send_port(0, m_ports[0] & data);
			m_ddrs[0] = data;
			break;

		case 2: // p4
			send_port(1, data & m_ddrs[1]);
			m_ports[1] = data;
			break;

		case 4: // p5
			send_port(2, data & m_ddrs[2]);
			m_ports[2] = data;
			break;

		case 5: // p5 ddr
			send_port(2, m_ports[2] & data);
			m_ddrs[2] = data;
			break;

		case 6: // p6
			send_port(3, data & m_ddrs[3]);
			m_ports[3] = data;
			break;

		case 7: // p6 ddr
			send_port(3, m_ports[3] & data);
			m_ddrs[3] = data;
			break;
	}
}

uint8_t m3745x_device::intregs_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_intreq1;

		case 1:
			return m_intreq2;

		case 2:
			return m_intctrl1;

		case 3:
			return m_intctrl2;
	}

	// this should never happen
	assert(0);
	return 0;
}

void m3745x_device::intregs_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_intreq1 = data;
			break;

		case 1:
			m_intreq2 = data;
			break;

		case 2:
			m_intctrl1 = data;
			break;

		case 3:
			m_intctrl2 = data;
			break;
	}

	recalc_irqs();
}

uint8_t m3745x_device::adc_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			m_intreq2 &= ~IRQ2_ADC;
			recalc_irqs();
			return m_read_ad[m_adctrl & 7]();

		case 1:
			return m_adctrl;
	}

	return 0;
}

void m3745x_device::adc_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			printf("M3745x: Write %02x to ADC output?!\n", data);
			break;

		case 1:
			m_adctrl = data;

			// starting a conversion?  this takes 50 cycles.
			if (!(m_adctrl & ADCTRL_COMPLETE))
			{
				m_timers[TIMER_ADC]->adjust(cycles_to_attotime(50));
			}
			break;
	}
}

/* M37450 - baseline for this familiy */
void m37450_device::m37450_map(address_map &map)
{
	map(0x0000, 0x00bf).ram();
	map(0x00d6, 0x00dd).rw(FUNC(m37450_device::ports_r), FUNC(m37450_device::ports_w));
	map(0x00e2, 0x00e3).rw(FUNC(m37450_device::adc_r), FUNC(m37450_device::adc_w));
	map(0x00fc, 0x00ff).rw(FUNC(m37450_device::intregs_r), FUNC(m37450_device::intregs_w));
	map(0x0100, 0x01ff).ram();
}

m37450_device::m37450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m37450_device(mconfig, M37450, tag, owner, clock)
{
}

m37450_device::m37450_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m3745x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(m37450_device::m37450_map), this))
{
}
