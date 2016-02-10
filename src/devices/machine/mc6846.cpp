// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Motorola 6846 emulation.

  The MC6846 chip provides ROM (2048 bytes), I/O (8-bit directional data port +
  2 control lines) and a programmable timer.
  It may be interfaced with a M6809 cpu.
  It is used in some Thomson computers.

  Not yet implemented:
  - external clock (CTC)
  - latching of port on CP1
  - gate input (CTG)
  - timer comparison modes (frequency and pulse width)
  - CP2 acknowledge modes

**********************************************************************/

#include "emu.h"
#include "mc6846.h"

#define VERBOSE 0


/******************* utility function and macros ********************/

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define PORT                                \
	((m_pdr & m_ddr) |                  \
		((!m_in_port_cb.isnull() ? m_in_port_cb( 0 ) : 0) & \
		~m_ddr))

#define CTO                             \
	((MODE == 0x30 || (m_tcr & 0x80)) ? m_cto : 0)

#define MODE (m_tcr & 0x38)

#define FACTOR ((m_tcr & 4) ? 8 : 1)


const device_type MC6846 = &device_creator<mc6846_device>;

mc6846_device::mc6846_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC6846, "MC6846 Programmable Timer", tag, owner, clock, "mc6846", __FILE__),
	m_out_port_cb(*this),
	m_out_cp1_cb(*this),
	m_out_cp2_cb(*this),
	m_in_port_cb(*this),
	m_out_cto_cb(*this),
	m_irq_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6846_device::device_start()
{
	m_interval = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc6846_device::timer_expire), this));
	m_one_shot = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc6846_device::timer_one_shot), this));

	m_out_port_cb.resolve();  /* 8-bit output */
	m_out_cp1_cb.resolve_safe();   /* 1-bit output */
	m_out_cp2_cb.resolve();   /* 1-bit output */

	/* CPU read from the outside through chip */
	m_in_port_cb.resolve(); /* 8-bit input */

	/* asynchronous timer output to outside world */
	m_out_cto_cb.resolve(); /* 1-bit output */

	/* timer interrupt */
	m_irq_cb.resolve();

	save_item(NAME(m_csr));
	save_item(NAME(m_pcr));
	save_item(NAME(m_ddr));
	save_item(NAME(m_pdr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_cp1));
	save_item(NAME(m_cp2));
	save_item(NAME(m_cp2_cpu));
	save_item(NAME(m_cto));
	save_item(NAME(m_time_MSB));
	save_item(NAME(m_csr0_to_be_cleared));
	save_item(NAME(m_csr1_to_be_cleared));
	save_item(NAME(m_csr2_to_be_cleared));
	save_item(NAME(m_latch));
	save_item(NAME(m_preset));
	save_item(NAME(m_timer_started));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6846_device::device_reset()
{
	m_cto   = 0;
	m_csr   = 0;
	m_pcr   = 0x80;
	m_ddr   = 0;
	m_pdr   = 0;
	m_tcr   = 1;
	m_cp1   = 0;
	m_cp2   = 0;
	m_cp2_cpu  = 0;
	m_latch    = 0xffff;
	m_preset   = 0xffff;
	m_time_MSB = 0;
	m_csr0_to_be_cleared = 0;
	m_csr1_to_be_cleared = 0;
	m_csr2_to_be_cleared = 0;
	m_timer_started = 0;
	m_old_cif = 0;
	m_old_cto = 0;
	m_interval->reset();
	m_one_shot->reset();
}


inline UINT16 mc6846_device::counter()
{
	if ( m_timer_started )
	{
		attotime delay = m_interval ->remaining( );
		return delay.as_ticks(1000000) / FACTOR;
	}
	else
		return m_preset;
}



inline void mc6846_device::update_irq()
{
	int cif = 0;
	/* composite interrupt flag */
	if ( ( (m_csr & 1) && (m_tcr & 0x40) ) ||
			( (m_csr & 2) && (m_pcr & 1) ) ||
			( (m_csr & 4) && (m_pcr & 8) && ! (m_pcr & 0x20) ) )
		cif = 1;
	if ( m_old_cif != cif )
	{
		LOG (( "%f: mc6846 interrupt %i (time=%i cp1=%i cp2=%i)\n",
				machine().time().as_double(), cif,
				m_csr & 1, (m_csr >> 1 ) & 1, (m_csr >> 2 ) & 1 ));
		m_old_cif = cif;
	}
	if ( cif )
	{
		m_csr |= 0x80;
		if ( !m_irq_cb.isnull() )
			m_irq_cb( 1 );
	}
	else
	{
		m_csr &= ~0x80;
		if ( !m_irq_cb.isnull() )
			m_irq_cb( 0 );
	}
}



inline void mc6846_device::update_cto()
{
	int cto = CTO;
	if ( cto != m_old_cto )
	{
		LOG (( "%f: mc6846 CTO set to %i\n", machine().time().as_double(), cto ));
		m_old_cto = cto;
	}
	if ( !m_out_cto_cb.isnull() )
		m_out_cto_cb( (offs_t) 0, cto );
}



inline void mc6846_device::timer_launch()
{
	int delay = FACTOR * (m_preset+1);
	LOG (( "%f: mc6846 timer launch called, mode=%i, preset=%i (x%i)\n", machine().time().as_double(), MODE, m_preset, FACTOR ));

	if ( ! (m_tcr & 2) )
	{
		logerror( "mc6846 external clock CTC not implemented\n" );
	}

	switch( MODE )
	{
	case 0x00:
	case 0x10: /* continuous */
		m_cto = 0;
		break;

	case 0x20: /* single-shot */
		m_cto = 0;
		m_one_shot->reset( attotime::from_usec(FACTOR) );
		break;

	case 0x30:  /* cascaded single-shot */
		break;

	default:
		logerror( "mc6846 timer mode %i not implemented\n", MODE );
		m_interval->reset();
		m_timer_started = 0;
		return;
	}

	m_interval->reset( attotime::from_usec(delay) );
	m_timer_started = 1;

	m_csr &= ~1;
	update_cto();
	update_irq();
}



/******************* timer callbacks *********************************/

TIMER_CALLBACK_MEMBER( mc6846_device::timer_expire )
{
	int delay = FACTOR * (m_latch+1);

	LOG (( "%f: mc6846 timer expire called, mode=%i, latch=%i (x%i)\n", machine().time().as_double(), MODE, m_latch, FACTOR ));

	/* latch => counter */
	m_preset = m_latch;

	if ( ! (m_tcr & 2) )
		logerror( "mc6846 external clock CTC not implemented\n" );

	switch ( MODE )
	{
	case 0x00:
	case 0x10: /* continuous */
		m_cto = 1 ^ m_cto;
		break;

	case 0x20: /* single-shot */
		m_cto = 0;
		break;

	case 0x30:  /* cascaded single-shot */
		m_cto = ( m_tcr & 0x80 ) ? 1 : 0;
		break;

	default:
		logerror( "mc6846 timer mode %i not implemented\n", MODE );
		m_interval->reset(  );
		m_timer_started = 0;
		return;
	}

	m_interval->reset( attotime::from_usec(delay) );

	m_csr |= 1;
	update_cto();
	update_irq();
}



TIMER_CALLBACK_MEMBER( mc6846_device::timer_one_shot )
{
	LOG (( "%f: mc6846 timer one shot called\n", machine().time().as_double() ));

	/* 1 micro second after one-shot launch, we put cto to high */
	m_cto = 1;
	update_cto();
}



/************************** CPU interface ****************************/


READ8_MEMBER(mc6846_device::read)
{
	switch ( offset )
	{
	case 0:
	case 4:
		LOG (( "%s %f: mc6846 CSR read $%02X intr=%i (timer=%i, cp1=%i, cp2=%i)\n",
				machine().describe_context(), space.machine().time().as_double(),
				m_csr, (m_csr >> 7) & 1,
				m_csr & 1, (m_csr >> 1) & 1, (m_csr >> 2) & 1 ));
		m_csr0_to_be_cleared = m_csr & 1;
		m_csr1_to_be_cleared = m_csr & 2;
		m_csr2_to_be_cleared = m_csr & 4;
		return m_csr;

	case 1:
		LOG (( "%s %f: mc6846 PCR read $%02X\n", machine().describe_context(), space.machine().time().as_double(), m_pcr ));
		return m_pcr;

	case 2:
		LOG (( "%s %f: mc6846 DDR read $%02X\n", machine().describe_context(), space.machine().time().as_double(), m_ddr ));
		return m_ddr;

	case 3:
		LOG (( "%s %f: mc6846 PORT read $%02X\n", machine().describe_context(), space.machine().time().as_double(), PORT ));
		if ( ! (m_pcr & 0x80) )
		{
			if ( m_csr1_to_be_cleared )
				m_csr &= ~2;
			if ( m_csr2_to_be_cleared )
				m_csr &= ~4;
			update_irq();
			m_csr1_to_be_cleared = 0;
			m_csr2_to_be_cleared = 0;
		}
		return PORT;

	case 5:
		LOG (( "%s %f: mc6846 TCR read $%02X\n",machine().describe_context(), space.machine().time().as_double(), m_tcr ));
		return m_tcr;

	case 6:
		LOG (( "%s %f: mc6846 COUNTER hi read $%02X\n", machine().describe_context(), space.machine().time().as_double(), counter() >> 8 ));
		if ( m_csr0_to_be_cleared )
		{
			m_csr &= ~1;
			update_irq();
		}
		m_csr0_to_be_cleared = 0;
		return counter() >> 8;

	case 7:
		LOG (( "%s %f: mc6846 COUNTER low read $%02X\n", machine().describe_context(), space.machine().time().as_double(), counter() & 0xff ));
		if ( m_csr0_to_be_cleared )
		{
			m_csr &= ~1;
			update_irq();
		}
		m_csr0_to_be_cleared = 0;
		return counter() & 0xff;

	default:
		logerror( "%s mc6846 invalid read offset %i\n", machine().describe_context(), offset );
	}
	return 0;
}



WRITE8_MEMBER(mc6846_device::write)
{
	switch ( offset )
	{
	case 0:
	case 4:
		/* CSR is read-only */
		break;

	case 1:
	{
		static const char *const cp2[8] =
		{
			"in,neg-edge", "in,neg-edge,intr", "in,pos-edge", "in,pos-edge,intr",
			"out,intr-ack", "out,i/o-ack", "out,0", "out,1"
		};
		static const char *const cp1[8] =
		{
			"neg-edge", "neg-edge,intr", "pos-edge", "pos-edge,intr",
			"latched,neg-edge", "latched,neg-edge,intr",
			"latcged,pos-edge", "latcged,pos-edge,intr"
		};
		LOG (( "%s %f: mc6846 PCR write $%02X reset=%i cp2=%s cp1=%s\n",
				machine().describe_context(), space.machine().time().as_double(), data,
				(data >> 7) & 1, cp2[ (data >> 3) & 7 ], cp1[ data & 7 ] ));

	}
	m_pcr = data;
	if ( data & 0x80 )
	{      /* data reset */
		m_pdr = 0;
		m_ddr = 0;
		m_csr &= ~6;
		update_irq();
	}
	if ( data & 4 )
		logerror( "%s mc6846 CP1 latching not implemented\n", machine().describe_context() );
	if (data & 0x20)
	{
		if (data & 0x10)
		{
			m_cp2_cpu = (data >> 3) & 1;
			if ( !m_out_cp2_cb.isnull() )
				m_out_cp2_cb( (offs_t) 0, m_cp2_cpu );
		}
		else
			logerror( "%s mc6846 acknowledge not implemented\n", machine().describe_context() );
	}
	break;

	case 2:
		LOG (( "%s %f: mc6846 DDR write $%02X\n", machine().describe_context(), space.machine().time().as_double(), data ));
		if ( ! (m_pcr & 0x80) )
		{
			m_ddr = data;
			if ( !m_out_port_cb.isnull() )
				m_out_port_cb( (offs_t) 0, m_pdr & m_ddr );
		}
		break;

	case 3:
		LOG (( "%s %f: mc6846 PORT write $%02X (mask=$%02X)\n", machine().describe_context(), space.machine().time().as_double(), data,m_ddr ));
		if ( ! (m_pcr & 0x80) )
		{
			m_pdr = data;
			if ( !m_out_port_cb.isnull() )
				m_out_port_cb( (offs_t) 0, m_pdr & m_ddr );
			if ( m_csr1_to_be_cleared && (m_csr & 2) )
			{
				m_csr &= ~2;
				LOG (( "%s %f: mc6846 CP1 intr reset\n", machine().describe_context(), space.machine().time().as_double() ));
			}
			if ( m_csr2_to_be_cleared && (m_csr & 4) )
			{
				m_csr &= ~4;
				LOG (( "%s %f: mc6846 CP2 intr reset\n", machine().describe_context(), space.machine().time().as_double() ));
			}
			m_csr1_to_be_cleared = 0;
			m_csr2_to_be_cleared = 0;
			update_irq();
		}
		break;

	case 5:
	{
		static const char *const mode[8] =
			{
				"continuous", "cascaded", "continuous", "one-shot",
				"freq-cmp", "freq-cmp", "pulse-cmp", "pulse-cmp"
			};
		LOG (( "%s %f: mc6846 TCR write $%02X reset=%i clock=%s scale=%i mode=%s out=%s\n",
				machine().describe_context(), space.machine().time().as_double(), data,
				(data >> 7) & 1, (data & 0x40) ? "extern" : "sys",
				(data & 0x40) ? 1 : 8, mode[ (data >> 1) & 7 ],
				(data & 1) ? "enabled" : "0" ));

		m_tcr = data;
		if ( m_tcr & 1 )
		{
			/* timer preset = initialization without launch */
			m_preset = m_latch;
			m_csr &= ~1;
			if ( MODE != 0x30 )
				m_cto = 0;
			update_cto();
			m_interval->reset();
			m_one_shot->reset();
			m_timer_started = 0;
		}
		else
		{
			/* timer launch */
			if ( ! m_timer_started )
				timer_launch();
		}
		update_irq();
	}
	break;

	case 6:
		m_time_MSB = data;
		break;

	case 7:
		m_latch = ( ((UINT16) m_time_MSB) << 8 ) + data;
		LOG (( "%s %f: mc6846 COUNT write %i\n", machine().describe_context(), space.machine().time().as_double(), m_latch  ));
		if (!(m_tcr & 0x38))
		{
			/* timer initialization */
			m_preset = m_latch;
			m_csr &= ~1;
			update_irq();
			m_cto = 0;
			update_cto();
			/* launch only if started */
			if (!(m_tcr & 1))
				timer_launch();
		}
		break;

	default:
		logerror( "%s mc6846 invalid write offset %i\n", machine().describe_context(), offset );
	}
}



/******************** outside world interface ************************/



void mc6846_device::set_input_cp1(int data)
{
	data = (data != 0 );
	if ( data == m_cp1 )
		return;
	m_cp1 = data;
	LOG (( "%f: mc6846 input CP1 set to %i\n",  machine().time().as_double(), data ));
	if (( data &&  (m_pcr & 2)) || (!data && !(m_pcr & 2)))
	{
		m_csr |= 2;
		update_irq();
	}
}

void mc6846_device::set_input_cp2(int data)
{
	data = (data != 0 );
	if ( data == m_cp2 )
		return;
	m_cp2 = data;
	LOG (( "%f: mc6846 input CP2 set to %i\n", machine().time().as_double(), data ));
	if (m_pcr & 0x20)
	{
		if (( data &&  (m_pcr & 0x10)) || (!data && !(m_pcr & 0x10)))
		{
			m_csr |= 4;
			update_irq();
		}
	}
}



/************************ accessors **********************************/



UINT8 mc6846_device::get_output_port()
{
	return PORT;
}



UINT8 mc6846_device::get_output_cto()
{
	return CTO;
}



UINT8 mc6846_device::get_output_cp2()
{
	return m_cp2_cpu;
}



UINT16 mc6846_device::get_preset()
{
	return m_preset;
}
