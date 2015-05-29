// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Mostek MK3801 Serial Timer Interrupt Controller (Z80-STI) emulation

***************************************************************************/

/*

    TODO:

    - timers (other than delay mode)
    - serial I/O
    - reset behavior

*/

#include "emu.h"
#include "z80sti.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



// device type definition
const device_type Z80STI = &device_creator<z80sti_device>;



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// timer C/D control register
//const int TCDC_TARS  = 0x80;
//const int TCDC_TBRS  = 0x08;

// interrupt vector register
//const int PVR_ISE    = 0x08;
//const int PVR_VR4    = 0x10;

// general purpose I/O interrupt levels
const int z80sti_device::INT_LEVEL_GPIP[] =
{
	IR_P0, IR_P1, IR_P2, IR_P3, IR_P4, IR_P5, IR_P6, IR_P7
};

// timer interrupt levels
const int z80sti_device::INT_LEVEL_TIMER[] =
{
	IR_TA, IR_TB, IR_TC, IR_TD
};

// interrupt vectors
const UINT8 z80sti_device::INT_VECTOR[] =
{
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e
};

// timer prescaler divisors
const int z80sti_device::PRESCALER[] = { 0, 4, 10, 16, 50, 64, 100, 200 };



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80sti_device - constructor
//-------------------------------------------------

z80sti_device::z80sti_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80STI, "Mostek MK3801", tag, owner, clock, "z80sti", __FILE__),
		device_serial_interface(mconfig, *this),
		device_z80daisy_interface(mconfig, *this),
		m_out_int_cb(*this),
		m_in_gpio_cb(*this),
		m_out_gpio_cb(*this),
		m_out_so_cb(*this),
		m_out_tao_cb(*this),
		m_out_tbo_cb(*this),
		m_out_tco_cb(*this),
		m_out_tdo_cb(*this),
		m_rx_clock(0),
		m_tx_clock(0),
		m_gpip(0),
		m_aer(0),
		m_ier(0),
		m_ipr(0),
		m_isr(0),
		m_imr(0)
{
	for (int i = 0; i < 16; i++)
	{
		m_int_state[i] = 0;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80sti_device::device_start()
{
	// resolve callbacks
	m_out_int_cb.resolve_safe();
	m_in_gpio_cb.resolve_safe(0);
	m_out_gpio_cb.resolve_safe();
	m_out_so_cb.resolve_safe();
	m_out_tao_cb.resolve_safe();
	m_out_tbo_cb.resolve_safe();
	m_out_tco_cb.resolve_safe();
	m_out_tdo_cb.resolve_safe();

	// create the counter timers
	m_timer[TIMER_A] = timer_alloc(TIMER_A);
	m_timer[TIMER_B] = timer_alloc(TIMER_B);
	m_timer[TIMER_C] = timer_alloc(TIMER_C);
	m_timer[TIMER_D] = timer_alloc(TIMER_D);

	// create serial receive clock timer
	if (m_rx_clock > 0)
	{
		set_rcv_rate(m_rx_clock);
	}

	// create serial transmit clock timer
	if (m_tx_clock > 0)
	{
		set_tra_rate(m_tx_clock);
	}

	// state saving
	save_item(NAME(m_gpip));
	save_item(NAME(m_aer));
	save_item(NAME(m_ddr));
	save_item(NAME(m_ier));
	save_item(NAME(m_ipr));
	save_item(NAME(m_isr));
	save_item(NAME(m_imr));
	save_item(NAME(m_pvr));
	save_item(NAME(m_int_state));
	save_item(NAME(m_tabc));
	save_item(NAME(m_tcdc));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tmc));
	save_item(NAME(m_to));
	save_item(NAME(m_scr));
	save_item(NAME(m_ucr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_udr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80sti_device::device_reset()
{
	memset(m_tmc, 0, sizeof(m_tmc));
	memset(m_to, 0, sizeof(m_to));

	transmit_register_reset();
	receive_register_reset();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void z80sti_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	timer_count(id);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void z80sti_device::tra_callback()
{
	m_out_so_cb(transmit_register_get_data_bit());
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void z80sti_device::tra_complete()
{
	// TODO
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void z80sti_device::rcv_complete()
{
	// TODO
}


//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------

int z80sti_device::z80daisy_irq_state()
{
	int state = 0, i;

	// loop over all interrupt sources
	for (i = 15; i >= 0; i--)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}

		if (BIT(m_imr, i))
		{
			state |= m_int_state[i];
		}
	}

	LOG(("Z80STI '%s' Interrupt State: %u\n", tag(), state));

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------

int z80sti_device::z80daisy_irq_ack()
{
	int i;

	// loop over all interrupt sources
	for (i = 15; i >= 0; i--)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[i] & Z80_DAISY_INT)
		{
			UINT8 vector = (m_pvr & 0xe0) | INT_VECTOR[i];

			// clear interrupt, switch to the IEO state, and update the IRQs
			m_int_state[i] = Z80_DAISY_IEO;

			// clear interrupt pending register bit
			m_ipr &= ~(1 << i);

			// set interrupt in-service register bit
			m_isr |= (1 << i);

			check_interrupts();

			LOG(("Z80STI '%s' Interrupt Acknowledge Vector: %02x\n", tag(), vector));

			return vector;
		}
	}

	logerror("z80sti_irq_ack: failed to find an interrupt to ack!\n");

	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------

void z80sti_device::z80daisy_irq_reti()
{
	int i;

	LOG(("Z80STI '%s' Return from Interrupt\n", tag()));

	// loop over all interrupt sources
	for (i = 15; i >= 0; i--)
	{
		// find the first channel with an IEO pending
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			// clear the IEO state and update the IRQs
			m_int_state[i] &= ~Z80_DAISY_IEO;

			// clear interrupt in-service register bit
			m_isr &= ~(1 << i);

			check_interrupts();
			return;
		}
	}

	logerror("z80sti_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  check_interrupts - set the interrupt request
//  line state
//-------------------------------------------------

void z80sti_device::check_interrupts()
{
	if (m_ipr & m_imr)
	{
		m_out_int_cb(ASSERT_LINE);
	}
	else
	{
		m_out_int_cb(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  take_interrupt - mark an interrupt pending
//-------------------------------------------------

void z80sti_device::take_interrupt(int level)
{
	// set interrupt pending register bit
	m_ipr |= 1 << level;

	// trigger interrupt
	m_int_state[level] |= Z80_DAISY_INT;

	check_interrupts();
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

READ8_MEMBER( z80sti_device::read )
{
	UINT8 data = 0;

	switch (offset & 0x0f)
	{
	case REGISTER_IR:
		switch (m_pvr & 0x07)
		{
		case REGISTER_IR_SCR:    data = m_scr; break;
		case REGISTER_IR_TDDR:   data = m_tmc[TIMER_D]; break;
		case REGISTER_IR_TCDR:   data = m_tmc[TIMER_C]; break;
		case REGISTER_IR_AER:    data = m_aer; break;
		case REGISTER_IR_IERB:   data = m_ier & 0xff; break;
		case REGISTER_IR_IERA:   data = m_ier >> 8; break;
		case REGISTER_IR_DDR:    data = m_ddr; break;
		case REGISTER_IR_TCDC:   data = m_tcdc; break;
		}
		break;

	case REGISTER_GPIP:  m_gpip = (m_in_gpio_cb(0) & ~m_ddr) | (m_gpip & m_ddr); data = m_gpip; break;
	case REGISTER_IPRB:  data = m_ipr & 0xff; break;
	case REGISTER_IPRA:  data = m_ipr >> 8; break;
	case REGISTER_ISRB:  data = m_isr & 0xff; break;
	case REGISTER_ISRA:  data = m_isr >> 8; break;
	case REGISTER_IMRB:  data = m_imr & 0xff; break;
	case REGISTER_IMRA:  data = m_imr >> 8; break;
	case REGISTER_PVR:   data = m_pvr; break;
	case REGISTER_TABC:  data = m_tabc; break;
	case REGISTER_TBDR:  data = m_tmc[TIMER_B]; break;
	case REGISTER_TADR:  data = m_tmc[TIMER_A]; break;
	case REGISTER_UCR:   data = m_ucr; break;
	case REGISTER_RSR:   data = m_rsr; break;
	case REGISTER_TSR:   data = m_tsr; break;
	case REGISTER_UDR:   data = m_udr; break;
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

WRITE8_MEMBER( z80sti_device::write )
{
	switch (offset & 0x0f)
	{
	case REGISTER_IR:
		switch (m_pvr & 0x07)
		{
		case REGISTER_IR_SCR:
			LOG(("Z80STI '%s' Sync Character Register: %x\n", tag(), data));
			m_scr = data;
			break;

		case REGISTER_IR_TDDR:
			LOG(("Z80STI '%s' Timer D Data Register: %x\n", tag(), data));
			m_tdr[TIMER_D] = data;
			break;

		case REGISTER_IR_TCDR:
			LOG(("Z80STI '%s' Timer C Data Register: %x\n", tag(), data));
			m_tdr[TIMER_C] = data;
			break;

		case REGISTER_IR_AER:
			LOG(("Z80STI '%s' Active Edge Register: %x\n", tag(), data));
			m_aer = data;
			break;

		case REGISTER_IR_IERB:
			LOG(("Z80STI '%s' Interrupt Enable Register B: %x\n", tag(), data));
			m_ier = (m_ier & 0xff00) | data;
			check_interrupts();
			break;

		case REGISTER_IR_IERA:
			LOG(("Z80STI '%s' Interrupt Enable Register A: %x\n", tag(), data));
			m_ier = (data << 8) | (m_ier & 0xff);
			check_interrupts();
			break;

		case REGISTER_IR_DDR:
			LOG(("Z80STI '%s' Data Direction Register: %x\n", tag(), data));
			m_ddr = data;
			break;

		case REGISTER_IR_TCDC:
			{
			int tcc = PRESCALER[(data >> 4) & 0x07];
			int tdc = PRESCALER[data & 0x07];

			m_tcdc = data;

			LOG(("Z80STI '%s' Timer C Prescaler: %u\n", tag(), tcc));
			LOG(("Z80STI '%s' Timer D Prescaler: %u\n", tag(), tdc));

			if (tcc)
				m_timer[TIMER_C]->adjust(attotime::from_hz(clock() / tcc), TIMER_C, attotime::from_hz(clock() / tcc));
			else
				m_timer[TIMER_C]->enable(false);

			if (tdc)
				m_timer[TIMER_D]->adjust(attotime::from_hz(clock() / tdc), TIMER_D, attotime::from_hz(clock() / tdc));
			else
				m_timer[TIMER_D]->enable(false);

			if (BIT(data, 7))
			{
				LOG(("Z80STI '%s' Timer A Reset\n", tag()));
				m_to[TIMER_A] = 0;

				m_out_tao_cb(m_to[TIMER_A]);
			}

			if (BIT(data, 3))
			{
				LOG(("Z80STI '%s' Timer B Reset\n", tag()));
				m_to[TIMER_B] = 0;

				m_out_tbo_cb(m_to[TIMER_B]);
			}
			}
			break;
		}
		break;

	case REGISTER_GPIP:
		LOG(("Z80STI '%s' General Purpose I/O Register: %x\n", tag(), data));
		m_gpip = data & m_ddr;
		m_out_gpio_cb((offs_t)0, m_gpip);
		break;

	case REGISTER_IPRB:
		{
		int i;
		LOG(("Z80STI '%s' Interrupt Pending Register B: %x\n", tag(), data));
		m_ipr &= (m_ipr & 0xff00) | data;

		for (i = 0; i < 16; i++)
		{
			if (!BIT(m_ipr, i) && (m_int_state[i] == Z80_DAISY_INT)) m_int_state[i] = 0;
		}

		check_interrupts();
		}
		break;

	case REGISTER_IPRA:
		{
		int i;
		LOG(("Z80STI '%s' Interrupt Pending Register A: %x\n", tag(), data));
		m_ipr &= (data << 8) | (m_ipr & 0xff);

		for (i = 0; i < 16; i++)
		{
			if (!BIT(m_ipr, i) && (m_int_state[i] == Z80_DAISY_INT)) m_int_state[i] = 0;
		}

		check_interrupts();
		}
		break;

	case REGISTER_ISRB:
		LOG(("Z80STI '%s' Interrupt In-Service Register B: %x\n", tag(), data));
		m_isr &= (m_isr & 0xff00) | data;
		break;

	case REGISTER_ISRA:
		LOG(("Z80STI '%s' Interrupt In-Service Register A: %x\n", tag(), data));
		m_isr &= (data << 8) | (m_isr & 0xff);
		break;

	case REGISTER_IMRB:
		LOG(("Z80STI '%s' Interrupt Mask Register B: %x\n", tag(), data));
		m_imr = (m_imr & 0xff00) | data;
		m_isr &= m_imr;
		check_interrupts();
		break;

	case REGISTER_IMRA:
		LOG(("Z80STI '%s' Interrupt Mask Register A: %x\n", tag(), data));
		m_imr = (data << 8) | (m_imr & 0xff);
		m_isr &= m_imr;
		check_interrupts();
		break;

	case REGISTER_PVR:
		LOG(("Z80STI '%s' Interrupt Vector: %02x\n", tag(), data & 0xe0));
		LOG(("Z80STI '%s' IR Address: %01x\n", tag(), data & 0x07));
		m_pvr = data;
		break;

	case REGISTER_TABC:
		{
		int tac = PRESCALER[(data >> 4) & 0x07];
		int tbc = PRESCALER[data & 0x07];

		m_tabc = data;

		LOG(("Z80STI '%s' Timer A Prescaler: %u\n", tag(), tac));
		LOG(("Z80STI '%s' Timer B Prescaler: %u\n", tag(), tbc));

		if (tac)
			m_timer[TIMER_A]->adjust(attotime::from_hz(clock() / tac), TIMER_A, attotime::from_hz(clock() / tac));
		else
			m_timer[TIMER_A]->enable(false);

		if (tbc)
			m_timer[TIMER_B]->adjust(attotime::from_hz(clock() / tbc), TIMER_B, attotime::from_hz(clock() / tbc));
		else
			m_timer[TIMER_B]->enable(false);
		}
		break;

	case REGISTER_TBDR:
		LOG(("Z80STI '%s' Timer B Data Register: %x\n", tag(), data));
		m_tdr[TIMER_B] = data;
		break;

	case REGISTER_TADR:
		LOG(("Z80STI '%s' Timer A Data Register: %x\n", tag(), data));
		m_tdr[TIMER_A] = data;
		break;

	case REGISTER_UCR:
		LOG(("Z80STI '%s' USART Control Register: %x\n", tag(), data));
		m_ucr = data;
		break;

	case REGISTER_RSR:
		LOG(("Z80STI '%s' Receiver Status Register: %x\n", tag(), data));
		m_rsr = data;
		break;

	case REGISTER_TSR:
		LOG(("Z80STI '%s' Transmitter Status Register: %x\n", tag(), data));
		m_tsr = data;
		break;

	case REGISTER_UDR:
		LOG(("Z80STI '%s' USART Data Register: %x\n", tag(), data));
		m_udr = data;
		break;
	}
}


//-------------------------------------------------
//  timer_count - timer count down
//-------------------------------------------------

void z80sti_device::timer_count(int index)
{
	if (m_tmc[index] == 0x01)
	{
		//LOG(("Z80STI '%s' Timer %c Expired\n", tag(), 'A' + index));

		// toggle timer output signal
		m_to[index] = !m_to[index];

		switch (index)
		{
			case TIMER_A:
				m_out_tao_cb(m_to[index]);
				break;
			case TIMER_B:
				m_out_tbo_cb(m_to[index]);
				break;
			case TIMER_C:
				m_out_tco_cb(m_to[index]);
				break;
			case TIMER_D:
				m_out_tdo_cb(m_to[index]);
				break;
		}

		if (m_ier & (1 << INT_LEVEL_TIMER[index]))
		{
			LOG(("Z80STI '%s' Interrupt Pending for Timer %c\n", tag(), 'A' + index));

			// signal timer elapsed interrupt
			take_interrupt(INT_LEVEL_TIMER[index]);
		}

		// load timer main counter
		m_tmc[index] = m_tdr[index];
	}
	else
	{
		// count down
		m_tmc[index]--;
	}
}


//-------------------------------------------------
//  gpip_input - GPIP input line write
//-------------------------------------------------

void z80sti_device::gpip_input(int bit, int state)
{
	int aer = BIT(m_aer, bit);
	int old_state = BIT(m_gpip, bit);

	if ((old_state ^ aer) && !(state ^ aer))
	{
		LOG(("Z80STI '%s' Edge Transition Detected on Bit: %u\n", tag(), bit));

		if (m_ier & (1 << INT_LEVEL_GPIP[bit]))
		{
			LOG(("Z80STI '%s' Interrupt Pending for P%u\n", tag(), bit));

			take_interrupt(INT_LEVEL_GPIP[bit]);
		}
	}

	m_gpip = (m_gpip & ~(1 << bit)) | (state << bit);
}

WRITE_LINE_MEMBER( z80sti_device::i0_w ) { gpip_input(0, state); }
WRITE_LINE_MEMBER( z80sti_device::i1_w ) { gpip_input(1, state); }
WRITE_LINE_MEMBER( z80sti_device::i2_w ) { gpip_input(2, state); }
WRITE_LINE_MEMBER( z80sti_device::i3_w ) { gpip_input(3, state); }
WRITE_LINE_MEMBER( z80sti_device::i4_w ) { gpip_input(4, state); }
WRITE_LINE_MEMBER( z80sti_device::i5_w ) { gpip_input(5, state); }
WRITE_LINE_MEMBER( z80sti_device::i6_w ) { gpip_input(6, state); }
WRITE_LINE_MEMBER( z80sti_device::i7_w ) { gpip_input(7, state); }


//-------------------------------------------------
//  rc_w - receiver clock
//-------------------------------------------------

WRITE_LINE_MEMBER( z80sti_device::rc_w )
{
	rx_clock_w(state);
}


//-------------------------------------------------
//  tc_w - transmitter clock
//-------------------------------------------------

WRITE_LINE_MEMBER( z80sti_device::tc_w )
{
	tx_clock_w(state);
}
