// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/***************************************************************************
    mos tri port interface 6525
    mos triple interface adapter 6523

    peter.trauner@jk.uni-linz.ac.at

    used in commodore b series
    used in commodore c1551 floppy disk drive
***************************************************************************/

/*
 mos tpi 6525
 40 pin package
 3 8 bit ports (pa, pb, pc)
 8 registers to pc
 0 port a 0 in low
 1 port a data direction register 1 output
 2 port b
 3 port b ddr
 4 port c
  handshaking, interrupt mode
  0 interrupt 0 input, 1 interrupt enabled
   interrupt set on falling edge
  1 interrupt 1 input
  2 interrupt 2 input
  3 interrupt 3 input
  4 interrupt 4 input
  5 irq output
  6 ca handshake line (read handshake answer on I3 preferred)
  7 cb handshake line (write handshake clear on I4 preferred)
 5 port c ddr

 6 cr configuration register
  0 mc
    0 port c normal input output mode like port a and b
    1 port c used for handshaking and interrupt input
  1 1 interrupt prioritized
  2 i3 configure edge
    1 interrupt set on positive edge
  3 i4 configure edge
  5,4 ca handshake
   00 on read
      rising edge of i3 sets ca high
      read a data from computers sets ca low
   01 pulse output
      1 microsecond low after read a operation
   10 manual output low
   11 manual output high
  7,6 cb handshake
   00 handshake on write
      write b data from computer sets cb 0
      rising edge of i4 sets cb high
   01 pulse output
      1 microsecond low after write b operation
   10 manual output low
   11 manual output high
 7 air active interrupt register
   0 I0 occurred
   1 I1 occurred
   2 I2 occurred
   3 I3 occurred
   4 I4 occurred
   read clears interrupt

 non prioritized interrupts
  interrupt is set when occurred
  read clears all interrupts

 prioritized interrupts
  I4>I3>I2>I1>I0
  highest interrupt can be found in air register
  read clears highest interrupt
*/

#include "emu.h"
#include "6525tpi.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VERBOSE_LEVEL 0
#define DBG_LOG( MACHINE, N, M, A ) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", MACHINE.time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)


#define INTERRUPT_MODE (m_cr & 1)
#define PRIORIZED_INTERRUPTS (m_cr & 2)
#define INTERRUPT3_RISING_EDGE (m_cr & 4)
#define INTERRUPT4_RISING_EDGE (m_cr & 8)
#define CA_MANUAL_OUT (m_cr & 0x20)
#define CA_MANUAL_LEVEL ((m_cr & 0x10) ? 1 : 0)
#define CB_MANUAL_OUT (m_cr & 0x80)
#define CB_MANUAL_LEVEL ((m_cr & 0x40) ? 1 : 0)


const device_type TPI6525 = &device_creator<tpi6525_device>;

tpi6525_device::tpi6525_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TPI6525, "6525 TPI", tag, owner, clock, "tpi6525", __FILE__),
	m_out_irq_cb(*this),
	m_in_pa_cb(*this),
	m_out_pa_cb(*this),
	m_in_pb_cb(*this),
	m_out_pb_cb(*this),
	m_in_pc_cb(*this),
	m_out_pc_cb(*this),
	m_out_ca_cb(*this),
	m_out_cb_cb(*this),
	m_port_a(0),
	m_ddr_a(0),
	m_in_a(0),
	m_port_b(0),
	m_ddr_b(0),
	m_in_b(0),
	m_port_c(0),
	m_ddr_c(0),
	m_in_c(0),
	m_ca_level(0),
	m_cb_level(0),
	m_interrupt_level(0),
	m_cr(0),
	m_air(0)
{
	for (auto & elem : m_irq_level)
	{
		elem = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tpi6525_device::device_start()
{
	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_in_pa_cb.resolve();
	m_out_pa_cb.resolve_safe();
	m_in_pb_cb.resolve();
	m_out_pb_cb.resolve_safe();
	m_in_pc_cb.resolve();
	m_out_pc_cb.resolve_safe();
	m_out_ca_cb.resolve_safe();
	m_out_cb_cb.resolve_safe();

	/* register for state saving */
	save_item(NAME(m_port_a));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_in_a));
	save_item(NAME(m_port_b));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_in_b));
	save_item(NAME(m_port_c));
	save_item(NAME(m_ddr_c));
	save_item(NAME(m_in_c));
	save_item(NAME(m_ca_level));
	save_item(NAME(m_cb_level));
	save_item(NAME(m_interrupt_level));
	save_item(NAME(m_cr));
	save_item(NAME(m_air));
	save_item(NAME(m_irq_level));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tpi6525_device::device_reset()
{
	/* setup some initial values */
	m_in_a = 0xff;
	m_in_b = 0xff;
	m_in_c = 0xff;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void tpi6525_device::set_interrupt()
{
	if (!m_interrupt_level && (m_air != 0))
	{
		m_interrupt_level = 1;

		DBG_LOG(machine(), 3, "tpi6525", ("%s set interrupt\n", tag()));

		m_out_irq_cb(m_interrupt_level);
	}
}


void tpi6525_device::clear_interrupt()
{
	if (m_interrupt_level && (m_air == 0))
	{
		m_interrupt_level = 0;

		DBG_LOG(machine(), 3, "tpi6525", ("%s clear interrupt\n", tag()));

		m_out_irq_cb(m_interrupt_level);
	}
}


WRITE_LINE_MEMBER( tpi6525_device::i0_w )
{
	if (INTERRUPT_MODE && (state != m_irq_level[0]))
	{
		m_irq_level[0] = state;

		if ((state == 0) && !(m_air & 1) && (m_ddr_c & 1))
		{
			m_air |= 1;
			set_interrupt();
		}
	}
}


WRITE_LINE_MEMBER( tpi6525_device::i1_w )
{
	if (INTERRUPT_MODE && (state != m_irq_level[1]))
	{
		m_irq_level[1] = state;

		if ((state == 0) && !(m_air & 2) && (m_ddr_c & 2))
		{
			m_air |= 2;
			set_interrupt();
		}
	}
}


WRITE_LINE_MEMBER( tpi6525_device::i2_w )
{
	if (INTERRUPT_MODE && (state != m_irq_level[2]))
	{
		m_irq_level[2] = state;

		if ((state == 0) && !(m_air & 4) && (m_ddr_c & 4))
		{
			m_air |= 4;
			set_interrupt();
		}
	}
}


WRITE_LINE_MEMBER( tpi6525_device::i3_w )
{
	if (INTERRUPT_MODE && (state != m_irq_level[3]))
	{
		m_irq_level[3] = state;

		if (((INTERRUPT3_RISING_EDGE && (state == 1))
			|| (!INTERRUPT3_RISING_EDGE && (state == 0)))
			&& !(m_air & 8) && (m_ddr_c & 8))
		{
			m_air |= 8;
			set_interrupt();
		}
	}
}


WRITE_LINE_MEMBER( tpi6525_device::i4_w )
{
	if (INTERRUPT_MODE && (state != m_irq_level[4]) )
	{
		m_irq_level[4] = state;

		if (((INTERRUPT4_RISING_EDGE && (state == 1))
			||(!INTERRUPT4_RISING_EDGE&&(state == 0)))
			&& !(m_air & 0x10) && (m_ddr_c & 0x10))
		{
			m_air |= 0x10;
			set_interrupt();
		}
	}
}

READ8_MEMBER( tpi6525_device::pa_r )
{
	UINT8 data = m_in_a;

	if (!m_in_pa_cb.isnull())
		data = m_in_pa_cb(offset);

	data = (data & ~m_ddr_a) | (m_ddr_a & m_port_a);

	return data;
}


WRITE8_MEMBER( tpi6525_device::pa_w )
{
	m_in_a = data;
}


READ8_MEMBER( tpi6525_device::pb_r )
{
	UINT8 data = m_in_b;

	if (!m_in_pb_cb.isnull())
		data = m_in_pb_cb(offset);

	data = (data & ~m_ddr_b) | (m_ddr_b & m_port_b);

	return data;
}


WRITE8_MEMBER( tpi6525_device::pb_w )
{
	m_in_b = data;
}


READ8_MEMBER( tpi6525_device::pc_r )
{
	UINT8 data = m_in_c;

	if (!m_in_pc_cb.isnull())
		data &= m_in_pc_cb(offset);

	data = (data & ~m_ddr_c) | (m_ddr_c & m_port_c);

	return data;
}


WRITE8_MEMBER( tpi6525_device::pc_w )
{
	m_in_c = data;
}


READ8_MEMBER( tpi6525_device::read )
{
	UINT8 data = 0xff;

	switch (offset & 7)
	{
	case 0:
		data = m_in_a;

		if (!m_in_pa_cb.isnull())
			data &= m_in_pa_cb(0);

		data = (data & ~m_ddr_a) | (m_ddr_a & m_port_a);

		break;

	case 1:
		data = m_in_b;

		if (!m_in_pb_cb.isnull())
			data &= m_in_pb_cb(0);

		data = (data & ~m_ddr_b) | (m_ddr_b & m_port_b);

		break;

	case 2:
		if (INTERRUPT_MODE)
		{
			data = 0;

			if (m_irq_level[0]) data |= 0x01;
			if (m_irq_level[1]) data |= 0x02;
			if (m_irq_level[2]) data |= 0x04;
			if (m_irq_level[3]) data |= 0x08;
			if (m_irq_level[4]) data |= 0x10;
			if (!m_interrupt_level) data |= 0x20;
			if (m_ca_level) data |= 0x40;
			if (m_cb_level) data |= 0x80;
		}
		else
		{
			data = m_in_c;

			if (!m_in_pc_cb.isnull())
				data &= m_in_pc_cb(0);

			data = (data & ~m_ddr_c) | (m_ddr_c & m_port_c);
		}

		DBG_LOG(machine(), 2, "tpi6525", ("%s read %.2x %.2x\n", tag(), offset, data));
		break;

	case 3:
		data = m_ddr_a;
		break;

	case 4:
		data = m_ddr_b;
		break;

	case 5:
		data = m_ddr_c;
		break;

	case 6:
		data = m_cr;
		break;

	case 7: /* air */
		if (PRIORIZED_INTERRUPTS)
		{
			if (m_air & 0x10)
			{
				data = 0x10;
				m_air &= ~0x10;
			}
			else if (m_air & 8)
			{
				data = 8;
				m_air &= ~8;
			}
			else if (m_air & 4)
			{
				data = 4;
				m_air &= ~4;
			}
			else if (m_air & 2)
			{
				data = 2;
				m_air &= ~2;
			}
			else if (m_air & 1)
			{
				data = 1;
				m_air &= ~1;
			}
		}
		else
		{
			data = m_air;
			m_air = 0;
		}

		clear_interrupt();
		break;

	}

	DBG_LOG(machine(), 3, "tpi6525", ("%s read %.2x %.2x\n", tag(), offset, data));

	return data;
}


WRITE8_MEMBER( tpi6525_device::write )
{
	DBG_LOG(machine(), 2, "tpi6525", ("%s write %.2x %.2x\n", tag(), offset, data));

	switch (offset & 7)
	{
	case 0:
		m_port_a = data;
		m_out_pa_cb((offs_t)0, (m_port_a & m_ddr_a) | (m_ddr_a ^ 0xff));
		break;

	case 1:
		m_port_b = data;
		m_out_pb_cb((offs_t)0, (m_port_b & m_ddr_b) | (m_ddr_b ^ 0xff));
		break;

	case 2:
		m_port_c = data;

		if (!INTERRUPT_MODE)
			m_out_pc_cb((offs_t)0, (m_port_c & m_ddr_c) | (m_ddr_c ^ 0xff));
		break;

	case 3:
		m_ddr_a = data;
		m_out_pa_cb((offs_t)0, (m_port_a & m_ddr_a) | (m_ddr_a ^ 0xff));
		break;

	case 4:
		m_ddr_b = data;
		m_out_pb_cb((offs_t)0, (m_port_b & m_ddr_b) | (m_ddr_b ^ 0xff));
		break;

	case 5:
		m_ddr_c = data;

		if (!INTERRUPT_MODE)
			m_out_pc_cb((offs_t)0, (m_port_c & m_ddr_c) | (m_ddr_c ^ 0xff));
		break;

	case 6:
		m_cr = data;

		if (INTERRUPT_MODE)
		{
			if (CA_MANUAL_OUT)
			{
				if (m_ca_level != CA_MANUAL_LEVEL)
				{
					m_ca_level = CA_MANUAL_LEVEL;
					m_out_ca_cb(m_ca_level);
				}
			}
			if (CB_MANUAL_OUT)
			{
				if (m_cb_level != CB_MANUAL_LEVEL)
				{
					m_cb_level = CB_MANUAL_LEVEL;
					m_out_cb_cb(m_cb_level);
				}
			}
		}

		break;

	case 7:
		/* m_air = data; */
		break;
	}
}

void tpi6525_device::port_line_w(UINT8 &port, int line, int state)
{
	port &= ~(1 << line);
	port |= state << line;
}

/* this should probably be done better, needed for amigacd.c */

UINT8 tpi6525_device::get_ddr_a()
{
	return m_ddr_a;
}

UINT8 tpi6525_device::get_ddr_b()
{
	return m_ddr_b;
}

UINT8 tpi6525_device::get_ddr_c()
{
	return m_ddr_c;
}
