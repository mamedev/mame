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

const device_type M37450 = &device_creator<m37450_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m3745x_device - constructor
//-------------------------------------------------
m3745x_device::m3745x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, address_map_constructor internal_map, const char *shortname, const char *source) :
	m740_device(mconfig, type, name, tag, owner, clock, "m3745x", source),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, internal_map),
	read_p3(*this),
	read_p4(*this),
	read_p5(*this),
	read_p6(*this),
	write_p3(*this),
	write_p4(*this),
	write_p5(*this),
	write_p6(*this),
	read_ad_0(*this),
	read_ad_1(*this),
	read_ad_2(*this),
	read_ad_3(*this),
	read_ad_4(*this),
	read_ad_5(*this),
	read_ad_6(*this),
	read_ad_7(*this),
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
	read_p3.resolve_safe(0);
	read_p4.resolve_safe(0);
	read_p5.resolve_safe(0);
	read_p6.resolve_safe(0);
	write_p3.resolve_safe();
	write_p4.resolve_safe();
	write_p5.resolve_safe();
	write_p6.resolve_safe();
	read_ad_0.resolve_safe(0);
	read_ad_1.resolve_safe(0);
	read_ad_2.resolve_safe(0);
	read_ad_3.resolve_safe(0);
	read_ad_4.resolve_safe(0);
	read_ad_5.resolve_safe(0);
	read_ad_6.resolve_safe(0);
	read_ad_7.resolve_safe(0);

	for (int i = 0; i < NUM_TIMERS; i++)
	{
		m_timers[i] = timer_alloc(i, nullptr);
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

void m3745x_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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

		case M3745X_SET_OVERFLOW:   // the base 740 class can handle this
			m740_device::execute_set_input(M740_SET_OVERFLOW, state);
			break;
	}

	recalc_irqs();
}

void m3745x_device::recalc_irqs()
{
	UINT16 all_ints;
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

void m3745x_device::send_port(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0:
			write_p3(data);
			break;

		case 1:
			write_p4(data);
			break;

		case 2:
			write_p5(data);
			break;

		case 3:
			write_p6(data);
			break;
	}
}

UINT8 m3745x_device::read_port(UINT8 offset)
{
	UINT8 incoming = 0;

	switch (offset)
	{
		case 0:
			incoming = read_p3();
			break;

		case 1:
			incoming = read_p4();
			break;

		case 2:
			incoming = read_p5();
			break;

		case 3:
			incoming = read_p6();
			break;
	}

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// OR in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	return incoming;
}

READ8_MEMBER(m3745x_device::ports_r)
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

WRITE8_MEMBER(m3745x_device::ports_w)
{
	switch (offset)
	{
		case 0: // p3
			send_port(space, 0, data & m_ddrs[0]);
			m_ports[0] = data;
			break;

		case 1: // p3 ddr
			send_port(space, 0, m_ports[0] & data);
			m_ddrs[0] = data;
			break;

		case 2: // p4
			send_port(space, 1, data & m_ddrs[1]);
			m_ports[1] = data;
			break;

		case 4: // p5
			send_port(space, 2, data & m_ddrs[2]);
			m_ports[2] = data;
			break;

		case 5: // p5 ddr
			send_port(space, 2, m_ports[2] & data);
			m_ddrs[2] = data;
			break;

		case 6: // p6
			send_port(space, 3, data & m_ddrs[3]);
			m_ports[3] = data;
			break;

		case 7: // p6 ddr
			send_port(space, 3, m_ports[3] & data);
			m_ddrs[3] = data;
			break;
	}
}

READ8_MEMBER(m3745x_device::intregs_r)
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

WRITE8_MEMBER(m3745x_device::intregs_w)
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

READ8_MEMBER(m3745x_device::adc_r)
{
	UINT8 rv = 0;

	switch (offset)
	{
		case 0:
			m_intreq2 &= ~IRQ2_ADC;
			recalc_irqs();

			switch (m_adctrl & 7)
			{
				case 0:
					rv = read_ad_0();
					break;

				case 1:
					rv = read_ad_1();
					break;

				case 2:
					rv = read_ad_2();
					break;

				case 3:
					rv = read_ad_3();
					break;

				case 4:
					rv = read_ad_4();
					break;

				case 5:
					rv = read_ad_5();
					break;

				case 6:
					rv = read_ad_6();
					break;

				case 7:
					rv = read_ad_7();
					break;
			}
			return rv;

		case 1:
			return m_adctrl;
	}

	return 0;
}

WRITE8_MEMBER(m3745x_device::adc_w)
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
				double hz = (double)clock() / 50.0;
				m_timers[TIMER_ADC]->adjust(attotime::from_hz(hz));
			}
			break;
	}
}

/* M37450 - baseline for this familiy */
static ADDRESS_MAP_START( m37450_map, AS_PROGRAM, 8, m37450_device )
	AM_RANGE(0x0000, 0x00bf) AM_RAM
	AM_RANGE(0x00d6, 0x00dd) AM_READWRITE(ports_r, ports_w)
	AM_RANGE(0x00e2, 0x00e3) AM_READWRITE(adc_r, adc_w)
	AM_RANGE(0x00fc, 0x00ff) AM_READWRITE(intregs_r, intregs_w)
	AM_RANGE(0x0100, 0x01ff) AM_RAM
ADDRESS_MAP_END

m37450_device::m37450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m3745x_device(mconfig, M37450, "Mitsubishi M37450", tag, owner, clock, ADDRESS_MAP_NAME(m37450_map), "m3745x", __FILE__)
{
}

m37450_device::m37450_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	m3745x_device(mconfig, type, name, tag, owner, clock, ADDRESS_MAP_NAME(m37450_map), shortname, source)
{
}
