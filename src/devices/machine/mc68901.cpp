// license:BSD-3-Clause
// copyright-holders:Curt Coder, AJR
/**********************************************************************

    Motorola MC68901 Multi Function Peripheral emulation

    This chip was originally designed by Mostek (MK68901) as a 68000-
    oriented evolution of the Z80 STI.

**********************************************************************/

/*

    TODO:

    - daisy chaining
    - disable GPIO3/4 interrupts when timer A/B in pulse mode
    - spurious interrupt

        If you look at the MFP datasheet it is obvious that it can generate the conditions for a spurious interrupt.
        However the fact that they indeed happen in the ST is quite interesting.

        The MFP will generate a spurious interrupt if interrupts are disabled (by changing the IERA/IERB registers)
        at the 'precise point'. The precise point would be after the system (but not necessarily the CPU, see below)
        triggered an MFP interrupt, and before the CPU drives the interrupt acknowledge cycle.

        If the MFP was connected directly to the CPU, spurious interrupts probably couldn't happen. However in the
        ST, GLUE seats in the middle and handles all the interrupt timing. It is possible that GLUE introduces a
        delay between detecting a change in the MFP interrupt request signal and actually propagating the change to
        the CPU IPL signals (it is even possible that GLUE make some kind of latching). This would create a window
        long enough for the 'precise point' described above.

        "yes, the spurious interrupt occurs when i mask a timer. i did not notice an occurance of the SPI when changing data and control registers.
        if i kill interrupts with the status reg before masking the timer interrupt, then the SPI occurs as soon as the status register is set to re-enable interrupts."

        Well, more experiments show that it's somewhat incorrect, and
        the GLUE is essentially invisible w.r.t IPL.  The CPU and the
        MFP manage to add the delays all by themselves.

    - RR & TR outputs

*/

#include "emu.h"
#include "mc68901.h"
#include "cpu/m68000/m68000.h"

#define LOG_GENERAL (1 << 0U)
#define LOG_RCV     (1 << 1U)
#define LOG_XMIT    (1 << 2U)

//#define VERBOSE (LOG_GENERAL | LOG_RCV | LOG_XMIT)
#include "logmacro.h"



// device type definition
DEFINE_DEVICE_TYPE(MC68901, mc68901_device, "mc68901", "Motorola MC68901 MFP")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


enum : u8 {
	VR_S                  = 0x08
};

enum : u16 {
	IR_GPIP_0             = 0x0001,
	IR_GPIP_1             = 0x0002,
	IR_GPIP_2             = 0x0004,
	IR_GPIP_3             = 0x0008,
	IR_TIMER_D            = 0x0010,
	IR_TIMER_C            = 0x0020,
	IR_GPIP_4             = 0x0040,
	IR_GPIP_5             = 0x0080,
	IR_TIMER_B            = 0x0100,
	IR_XMIT_ERROR         = 0x0200,
	IR_XMIT_BUFFER_EMPTY  = 0x0400,
	IR_RCV_ERROR          = 0x0800,
	IR_RCV_BUFFER_FULL    = 0x1000,
	IR_TIMER_A            = 0x2000,
	IR_GPIP_6             = 0x4000,
	IR_GPIP_7             = 0x8000
};

enum : u8 {
	TCR_TIMER_STOPPED     = 0x00,
	TCR_TIMER_DELAY_4     = 0x01,
	TCR_TIMER_DELAY_10    = 0x02,
	TCR_TIMER_DELAY_16    = 0x03,
	TCR_TIMER_DELAY_50    = 0x04,
	TCR_TIMER_DELAY_64    = 0x05,
	TCR_TIMER_DELAY_100   = 0x06,
	TCR_TIMER_DELAY_200   = 0x07,
	TCR_TIMER_EVENT       = 0x08,
	TCR_TIMER_PULSE_4     = 0x09,
	TCR_TIMER_PULSE_10    = 0x0a,
	TCR_TIMER_PULSE_16    = 0x0b,
	TCR_TIMER_PULSE_50    = 0x0c,
	TCR_TIMER_PULSE_64    = 0x0d,
	TCR_TIMER_PULSE_100   = 0x0e,
	TCR_TIMER_PULSE_200   = 0x0f,
	TCR_TIMER_RESET       = 0x10
};

enum : u8 {
	UCR_PARITY_ENABLED    = 0x04,
	UCR_PARITY_EVEN       = 0x02,
	UCR_PARITY_ODD        = 0x00,
	UCR_WORD_LENGTH_8     = 0x00,
	UCR_WORD_LENGTH_7     = 0x20,
	UCR_WORD_LENGTH_6     = 0x40,
	UCR_WORD_LENGTH_5     = 0x60,
	UCR_WORD_LENGTH_MASK  = 0x60,
	UCR_START_STOP_0_0    = 0x00,
	UCR_START_STOP_1_1    = 0x08,
	UCR_START_STOP_1_15   = 0x10,
	UCR_START_STOP_1_2    = 0x18,
	UCR_CLOCK_DIVIDE_16   = 0x80,
	UCR_CLOCK_DIVIDE_1    = 0x00
};

enum : u8 {
	RSR_RCV_ENABLE        = 0x01,
	RSR_SYNC_STRIP_ENABLE = 0x02,
	RSR_MATCH             = 0x04,
	RSR_CHAR_IN_PROGRESS  = 0x04,
	RSR_FOUND_SEARCH      = 0x08,
	RSR_BREAK             = 0x08,
	RSR_FRAME_ERROR       = 0x10,
	RSR_PARITY_ERROR      = 0x20,
	RSR_OVERRUN_ERROR     = 0x40,
	RSR_BUFFER_FULL       = 0x80
};

enum : u8 {
	TSR_XMIT_ENABLE       = 0x01,
	TSR_OUTPUT_HI_Z       = 0x00,
	TSR_OUTPUT_LOW        = 0x02,
	TSR_OUTPUT_HIGH       = 0x04,
	TSR_OUTPUT_LOOP       = 0x06,
	TSR_OUTPUT_MASK       = 0x06,
	TSR_BREAK             = 0x08,
	TSR_END_OF_XMIT       = 0x10,
	TSR_AUTO_TURNAROUND   = 0x20,
	TSR_UNDERRUN_ERROR    = 0x40,
	TSR_BUFFER_EMPTY      = 0x80
};

#define DIVISOR PRESCALER[data & 0x07]

const u16 mc68901_device::INT_MASK_GPIO[] =
{
	IR_GPIP_0, IR_GPIP_1, IR_GPIP_2, IR_GPIP_3,
	IR_GPIP_4, IR_GPIP_5, IR_GPIP_6, IR_GPIP_7
};


const u16 mc68901_device::INT_MASK_TIMER[] =
{
	IR_TIMER_A, IR_TIMER_B, IR_TIMER_C, IR_TIMER_D
};


const int mc68901_device::GPIO_TIMER[] =
{
	GPIP_4, GPIP_3
};


const int mc68901_device::PRESCALER[] = { 0, 4, 10, 16, 50, 64, 100, 200 };


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void mc68901_device::check_interrupts()
{
	if (m_ipr & m_imr)
	{
		m_out_irq_cb(ASSERT_LINE);
	}
	else
	{
		m_out_irq_cb(CLEAR_LINE);
	}
}

inline void mc68901_device::take_interrupt(u16 mask)
{
	m_ipr |= mask;

	check_interrupts();
}

inline void mc68901_device::tx_buffer_empty()
{
	if (m_ier & IR_XMIT_BUFFER_EMPTY)
	{
		take_interrupt(IR_XMIT_BUFFER_EMPTY);
	}
}

inline void mc68901_device::tx_error()
{
	if (m_ier & IR_XMIT_ERROR)
	{
		take_interrupt(IR_XMIT_ERROR);
	}
}

inline void mc68901_device::rx_buffer_full()
{
	if (m_ier & IR_RCV_BUFFER_FULL)
	{
		take_interrupt(IR_RCV_BUFFER_FULL);
	}
}

inline void mc68901_device::rx_error()
{
	if (m_ier & IR_RCV_ERROR)
	{
		take_interrupt(IR_RCV_ERROR);
	}
}

TIMER_CALLBACK_MEMBER(mc68901_device::timer_count)
{
	if (m_tmc[param] == 0x01)
	{
		/* toggle timer output signal */
		m_to[param] = !m_to[param];

		switch (param)
		{
		case TIMER_A:   m_out_tao_cb(m_to[param]);    break;
		case TIMER_B:   m_out_tbo_cb(m_to[param]);    break;
		case TIMER_C:   m_out_tco_cb(m_to[param]);    break;
		case TIMER_D:   m_out_tdo_cb(m_to[param]);    break;
		}

		if (m_ier & INT_MASK_TIMER[param])
		{
			/* signal timer elapsed interrupt */
			take_interrupt(INT_MASK_TIMER[param]);
		}

		/* load main counter */
		m_tmc[param] = m_tdr[param];
	}
	else
	{
		/* count down */
		m_tmc[param]--;
	}
}


inline void mc68901_device::timer_input(int index, int value)
{
	int bit = GPIO_TIMER[index];
	int aer = BIT(m_aer, bit);
	int cr = index ? m_tbcr : m_tacr;

	switch (cr & 0x0f)
	{
	case TCR_TIMER_EVENT:
		if (((m_ti[index] ^ aer) == 1) && ((value ^ aer) == 0))
		{
			timer_count(index);
		}

		m_ti[index] = value;
		break;

	case TCR_TIMER_PULSE_4:
	case TCR_TIMER_PULSE_10:
	case TCR_TIMER_PULSE_16:
	case TCR_TIMER_PULSE_50:
	case TCR_TIMER_PULSE_64:
	case TCR_TIMER_PULSE_100:
	case TCR_TIMER_PULSE_200:
		m_timer[index]->enable((value == aer));

		if (((m_ti[index] ^ aer) == 0) && ((value ^ aer) == 1))
		{
			if (m_ier & INT_MASK_GPIO[bit])
			{
				take_interrupt(INT_MASK_GPIO[bit]);
			}
		}

		m_ti[index] = value;
		break;
	}
}


inline void mc68901_device::gpio_input(int bit, int state)
{
	if (state != BIT(m_gpio_input, bit))
	{
		if (state == BIT(m_aer, bit))
		{
			LOG("MC68901 Edge Transition Detected on GPIO%u\n", bit);

			if (m_ier & INT_MASK_GPIO[bit]) // AND interrupt enabled bit is set...
			{
				LOG("MC68901 Interrupt Pending for GPIO%u\n", bit);

				take_interrupt(INT_MASK_GPIO[bit]); // set interrupt pending bit
			}
		}


		if (state)
			m_gpio_input |= (1 << bit);
		else
			m_gpio_input &= ~(1 << bit);
	}
}


void mc68901_device::gpio_output()
{
	u8 new_gpio_output = m_gpip & m_ddr;

	if (m_gpio_output != new_gpio_output)
	{
		m_gpio_output = new_gpio_output;
		m_out_gpio_cb((offs_t)0, m_gpio_output);
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc68901_device - constructor
//-------------------------------------------------

mc68901_device::mc68901_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MC68901, tag, owner, clock),
		m_timer_clock(0),
		m_out_irq_cb(*this),
		m_out_gpio_cb(*this),
		m_out_tao_cb(*this),
		m_out_tbo_cb(*this),
		m_out_tco_cb(*this),
		m_out_tdo_cb(*this),
		m_out_so_cb(*this),
		//m_out_rr_cb(*this),
		//m_out_tr_cb(*this),
		m_iack_chain_cb(*this),
		m_aer(0),
		m_ier(0),
		m_scr(0),
		m_scr_parity(false),
		m_transmit_buffer(0),
		m_receive_buffer(0),
		m_gpio_input(0),
		m_gpio_output(0xff),
		m_rframe(0),
		m_rclk(0),
		m_rbits(0),
		m_si_scan(0xff),
		m_next_rsr(0),
		m_rc(true),
		m_si(true),
		m_last_si(true),
		m_rparity(false),
		m_osr(0),
		m_tclk(0),
		m_tbits(0),
		m_tc(true),
		m_so(false),
		m_tparity(false),
		m_underrun(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc68901_device::device_start()
{
	/* resolve callbacks */
	m_out_irq_cb.resolve_safe();
	m_out_gpio_cb.resolve_safe();
	m_out_tao_cb.resolve_safe();
	m_out_tbo_cb.resolve_safe();
	m_out_tco_cb.resolve_safe();
	m_out_tdo_cb.resolve_safe();
	m_out_so_cb.resolve_safe();
	//m_out_rr_cb.resolve_safe();
	//m_out_tr_cb.resolve_safe();
	m_iack_chain_cb.resolve();

	/* create the timers */
	m_timer[TIMER_A] = timer_alloc(FUNC(mc68901_device::timer_count), this);
	m_timer[TIMER_B] = timer_alloc(FUNC(mc68901_device::timer_count), this);
	m_timer[TIMER_C] = timer_alloc(FUNC(mc68901_device::timer_count), this);
	m_timer[TIMER_D] = timer_alloc(FUNC(mc68901_device::timer_count), this);

	/* register for state saving */
	save_item(NAME(m_gpip));
	save_item(NAME(m_aer));
	save_item(NAME(m_ddr));
	save_item(NAME(m_ier));
	save_item(NAME(m_ipr));
	save_item(NAME(m_isr));
	save_item(NAME(m_imr));
	save_item(NAME(m_vr));
	save_item(NAME(m_tacr));
	save_item(NAME(m_tbcr));
	save_item(NAME(m_tcdcr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tmc));
	save_item(NAME(m_to));
	save_item(NAME(m_ti));
	save_item(NAME(m_scr));
	save_item(NAME(m_scr_parity));
	save_item(NAME(m_ucr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_transmit_buffer));
	save_item(NAME(m_receive_buffer));
	save_item(NAME(m_gpio_input));
	save_item(NAME(m_gpio_output));
	save_item(NAME(m_rframe));
	save_item(NAME(m_rclk));
	save_item(NAME(m_rbits));
	save_item(NAME(m_si_scan));
	save_item(NAME(m_next_rsr));
	save_item(NAME(m_rc));
	save_item(NAME(m_si));
	save_item(NAME(m_last_si));
	save_item(NAME(m_rparity));
	save_item(NAME(m_osr));
	save_item(NAME(m_tclk));
	save_item(NAME(m_tbits));
	save_item(NAME(m_tc));
	save_item(NAME(m_so));
	save_item(NAME(m_tparity));
	save_item(NAME(m_underrun));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc68901_device::device_reset()
{
	m_rsr = 0;
	m_tsr = TSR_BUFFER_EMPTY;
	m_underrun = false;
	m_rclk = 0;
	m_tclk = 0;

	// Avoid read-before-write
	m_ipr = m_imr = 0;

	m_rframe = 0x100;
	m_next_rsr = 0;

	memset(m_tmc, 0, sizeof(m_tmc));
	memset(m_ti, 0, sizeof(m_ti));
	memset(m_to, 0, sizeof(m_to));

	write(REGISTER_GPIP, 0);
	write(REGISTER_AER, 0);
	write(REGISTER_DDR, 0);
	write(REGISTER_IERA, 0);
	write(REGISTER_IERB, 0);
	write(REGISTER_IPRA, 0);
	write(REGISTER_IPRB, 0);
	write(REGISTER_ISRA, 0);
	write(REGISTER_ISRB, 0);
	write(REGISTER_IMRA, 0);
	write(REGISTER_IMRB, 0);
	write(REGISTER_VR, 0);
	write(REGISTER_TACR, 0);
	write(REGISTER_TBCR, 0);
	write(REGISTER_TCDCR, 0);
	write(REGISTER_SCR, 0);
	write(REGISTER_UCR, 0);

	set_so(true);
}


//-------------------------------------------------
//  read - read from one MFP register
//-------------------------------------------------

u8 mc68901_device::read(offs_t offset)
{
	switch (offset)
	{
	case REGISTER_GPIP:  return (m_gpio_input & ~m_ddr) | (m_gpip & m_ddr);

	case REGISTER_AER:   return m_aer;
	case REGISTER_DDR:   return m_ddr;

	case REGISTER_IERA:  return m_ier >> 8;
	case REGISTER_IERB:  return m_ier & 0xff;
	case REGISTER_IPRA:  return m_ipr >> 8;
	case REGISTER_IPRB:  return m_ipr & 0xff;
	case REGISTER_ISRA:  return m_isr >> 8;
	case REGISTER_ISRB:  return m_isr & 0xff;
	case REGISTER_IMRA:  return m_imr >> 8;
	case REGISTER_IMRB:  return m_imr & 0xff;
	case REGISTER_VR:    return m_vr;

	case REGISTER_TACR:  return m_tacr;
	case REGISTER_TBCR:  return m_tbcr;
	case REGISTER_TCDCR: return m_tcdcr;
	case REGISTER_TADR:  return m_tmc[TIMER_A];
	case REGISTER_TBDR:  return m_tmc[TIMER_B];
	case REGISTER_TCDR:  return m_tmc[TIMER_C];
	case REGISTER_TDDR:  return m_tmc[TIMER_D];

	case REGISTER_SCR:   return m_scr;
	case REGISTER_UCR:   return m_ucr;
	case REGISTER_RSR:
		{
			u8 rsr = m_rsr;
			if (!machine().side_effects_disabled())
				m_rsr &= ~RSR_OVERRUN_ERROR;
			return rsr;
		}

	case REGISTER_TSR:
		{
			/* clear UE bit (in reality, this won't be cleared until one full clock cycle of the transmitter has passed since the bit was set) */
			u8 tsr = m_tsr;
			if (!machine().side_effects_disabled() && !m_underrun)
				m_tsr &= ~TSR_UNDERRUN_ERROR;
			return tsr;
		}

	case REGISTER_UDR:
		if (!machine().side_effects_disabled())
		{
			m_rsr &= ~RSR_BUFFER_FULL;
			if (m_next_rsr != 0)
			{
				m_rsr |= m_next_rsr;
				m_next_rsr = 0;
				rx_error();
			}
			if ((m_rsr & RSR_BREAK) && BIT(m_rframe, 9))
			{
				m_rsr &= ~RSR_BREAK;
				rx_error();
			}
		}
		return m_receive_buffer;

	default:                      return 0;
	}
}



//-------------------------------------------------
//  write - write to one MFP register
//-------------------------------------------------

void mc68901_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case REGISTER_GPIP:
		LOG("MC68901 General Purpose I/O : %x\n", data);
		m_gpip = data;
		gpio_output();
		break;

	case REGISTER_AER:
		LOG("MC68901 Active Edge Register : %x\n", data);
		m_aer = data;
		break;

	case REGISTER_DDR:
		LOG("MC68901 Data Direction Register : %x\n", data);
		m_ddr = data;
		gpio_output();
		break;

	case REGISTER_IERA:
		LOG("MC68901 Interrupt Enable Register A : %x\n", data);
		m_ier = (data << 8) | (m_ier & 0xff);
		m_ipr &= m_ier;
		check_interrupts();
		break;

	case REGISTER_IERB:
		LOG("MC68901 Interrupt Enable Register B : %x\n", data);
		m_ier = (m_ier & 0xff00) | data;
		m_ipr &= m_ier;
		check_interrupts();
		break;

	case REGISTER_IPRA:
		LOG("MC68901 Interrupt Pending Register A : %x\n", data);
		m_ipr &= (data << 8) | (m_ipr & 0xff);
		check_interrupts();
		break;

	case REGISTER_IPRB:
		LOG("MC68901 Interrupt Pending Register B : %x\n", data);
		m_ipr &= (m_ipr & 0xff00) | data;
		check_interrupts();
		break;

	case REGISTER_ISRA:
		LOG("MC68901 Interrupt In-Service Register A : %x\n", data);
		m_isr &= (data << 8) | (m_isr & 0xff);
		break;

	case REGISTER_ISRB:
		LOG("MC68901 Interrupt In-Service Register B : %x\n", data);
		m_isr &= (m_isr & 0xff00) | data;
		break;

	case REGISTER_IMRA:
		LOG("MC68901 Interrupt Mask Register A : %x\n", data);
		m_imr = (data << 8) | (m_imr & 0xff);
		m_isr &= m_imr;
		check_interrupts();
		break;

	case REGISTER_IMRB:
		LOG("MC68901 Interrupt Mask Register B : %x\n", data);
		m_imr = (m_imr & 0xff00) | data;
		m_isr &= m_imr;
		check_interrupts();
		break;

	case REGISTER_VR:
		LOG("MC68901 Interrupt Vector : %x\n", data & 0xf0);

		m_vr = data & 0xf8;

		if (m_vr & VR_S)
		{
			LOG("MC68901 Software End-Of-Interrupt Mode\n");
		}
		else
		{
			LOG("MC68901 Automatic End-Of-Interrupt Mode\n");

			m_isr = 0;
		}
		break;

	case REGISTER_TACR:
		m_tacr = data & 0x1f;

		switch (m_tacr & 0x0f)
		{
		case TCR_TIMER_STOPPED:
			LOG("MC68901 Timer A Stopped\n");
			m_timer[TIMER_A]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
				int divisor = PRESCALER[m_tacr & 0x07];
				LOG("MC68901 Timer A Delay Mode : %u Prescale\n", divisor);
				m_timer[TIMER_A]->adjust(attotime::from_hz(m_timer_clock / divisor), TIMER_A, attotime::from_hz(m_timer_clock / divisor));
			}
			break;

		case TCR_TIMER_EVENT:
			LOG("MC68901 Timer A Event Count Mode\n");
			m_timer[TIMER_A]->enable(false);
			break;

		case TCR_TIMER_PULSE_4:
		case TCR_TIMER_PULSE_10:
		case TCR_TIMER_PULSE_16:
		case TCR_TIMER_PULSE_50:
		case TCR_TIMER_PULSE_64:
		case TCR_TIMER_PULSE_100:
		case TCR_TIMER_PULSE_200:
			LOG("MC68901 Timer A Pulse Width Mode\n");
			m_timer[TIMER_A]->adjust(attotime::never);
			break;
		}

		if (m_tacr & TCR_TIMER_RESET)
		{
			LOG("MC68901 Timer A Reset\n");

			m_to[TIMER_A] = 0;

			m_out_tao_cb(m_to[TIMER_A]);
		}
		break;

	case REGISTER_TBCR:
		m_tbcr = data & 0x1f;

		switch (m_tbcr & 0x0f)
		{
		case TCR_TIMER_STOPPED:
			LOG("MC68901 Timer B Stopped\n");
			m_timer[TIMER_B]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
				int divisor = PRESCALER[m_tbcr & 0x07];
				LOG("MC68901 Timer B Delay Mode : %u Prescale\n", divisor);
				m_timer[TIMER_B]->adjust(attotime::from_hz(m_timer_clock / divisor), TIMER_B, attotime::from_hz(m_timer_clock / divisor));
			}
			break;

		case TCR_TIMER_EVENT:
			LOG("MC68901 Timer B Event Count Mode\n");
			m_timer[TIMER_B]->enable(false);
			break;

		case TCR_TIMER_PULSE_4:
		case TCR_TIMER_PULSE_10:
		case TCR_TIMER_PULSE_16:
		case TCR_TIMER_PULSE_50:
		case TCR_TIMER_PULSE_64:
		case TCR_TIMER_PULSE_100:
		case TCR_TIMER_PULSE_200:
			LOG("MC68901 Timer B Pulse Width Mode\n");
			m_timer[TIMER_B]->adjust(attotime::never);
			break;
		}

		if (m_tacr & TCR_TIMER_RESET)
		{
			LOG("MC68901 Timer B Reset\n");

			m_to[TIMER_B] = 0;

			m_out_tbo_cb(m_to[TIMER_B]);
		}
		break;

	case REGISTER_TCDCR:
		m_tcdcr = data & 0x77;

		switch (m_tcdcr & 0x07)
		{
		case TCR_TIMER_STOPPED:
			LOG("MC68901 Timer D Stopped\n");
			m_timer[TIMER_D]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
				int divisor = PRESCALER[m_tcdcr & 0x07];
				LOG("MC68901 Timer D Delay Mode : %u Prescale\n", divisor);
				m_timer[TIMER_D]->adjust(attotime::from_hz(m_timer_clock / divisor), TIMER_D, attotime::from_hz(m_timer_clock / divisor));
			}
			break;
		}

		switch ((m_tcdcr >> 4) & 0x07)
		{
		case TCR_TIMER_STOPPED:
			LOG("MC68901 Timer C Stopped\n");
			m_timer[TIMER_C]->enable(false);
			break;

		case TCR_TIMER_DELAY_4:
		case TCR_TIMER_DELAY_10:
		case TCR_TIMER_DELAY_16:
		case TCR_TIMER_DELAY_50:
		case TCR_TIMER_DELAY_64:
		case TCR_TIMER_DELAY_100:
		case TCR_TIMER_DELAY_200:
			{
				int divisor = PRESCALER[(m_tcdcr >> 4) & 0x07];
				LOG("MC68901 Timer C Delay Mode : %u Prescale\n", divisor);
				m_timer[TIMER_C]->adjust(attotime::from_hz(m_timer_clock / divisor), TIMER_C, attotime::from_hz(m_timer_clock / divisor));
			}
			break;
		}
		break;

	case REGISTER_TADR:
		LOG("MC68901 Timer A Data Register : %x\n", data);

		m_tdr[TIMER_A] = data;

		if (!m_timer[TIMER_A]->enabled())
		{
			m_tmc[TIMER_A] = data;
		}
		break;

	case REGISTER_TBDR:
		LOG("MC68901 Timer B Data Register : %x\n", data);

		m_tdr[TIMER_B] = data;

		if (!m_timer[TIMER_B]->enabled())
		{
			m_tmc[TIMER_B] = data;
		}
		break;

	case REGISTER_TCDR:
		LOG("MC68901 Timer C Data Register : %x\n", data);

		m_tdr[TIMER_C] = data;

		if (!m_timer[TIMER_C]->enabled())
		{
			m_tmc[TIMER_C] = data;
		}
		break;

	case REGISTER_TDDR:
		LOG("MC68901 Timer D Data Register : %x\n", data);

		m_tdr[TIMER_D] = data;

		if (!m_timer[TIMER_D]->enabled())
		{
			m_tmc[TIMER_D] = data;
		}
		break;

	case REGISTER_SCR:
		LOG("MC68901 Sync Character : %x\n", data);

		m_scr = data;
		m_scr_parity = BIT(population_count_32(data), 0);
		break;

	case REGISTER_UCR:
		{
		int data_bit_count;

		switch (data & 0x60)
		{
		case UCR_WORD_LENGTH_8: default: data_bit_count = 8; break;
		case UCR_WORD_LENGTH_7: data_bit_count = 7; break;
		case UCR_WORD_LENGTH_6: data_bit_count = 6; break;
		case UCR_WORD_LENGTH_5: data_bit_count = 5; break;
		}

		if (data & UCR_PARITY_ENABLED)
		{
			if (data & UCR_PARITY_EVEN)
			{
				LOG("MC68901 Parity : Even\n");
			}
			else
			{
				LOG("MC68901 Parity : Odd\n");
			}
		}
		else
		{
			LOG("MC68901 Parity : Disabled\n");
		}

		LOG("MC68901 Word Length : %u bits\n", data_bit_count);


		switch (data & 0x18)
		{
		case UCR_START_STOP_0_0:
		default:
			LOG("MC68901 Start Bits : 0, Stop Bits : 0, Format : synchronous\n");
			break;

		case UCR_START_STOP_1_1:
			LOG("MC68901 Start Bits : 1, Stop Bits : 1, Format : asynchronous\n");
			break;

		case UCR_START_STOP_1_15:
			LOG("MC68901 Start Bits : 1, Stop Bits : 1.5, Format : asynchronous\n");
			break;

		case UCR_START_STOP_1_2:
			LOG("MC68901 Start Bits : 1, Stop Bits : 2, Format : asynchronous\n");
			break;
		}

		if (data & UCR_CLOCK_DIVIDE_16)
		{
			LOG("MC68901 Rx/Tx Clock Divisor : 16\n");
		}
		else
		{
			LOG("MC68901 Rx/Tx Clock Divisor : 1\n");
		}

		m_ucr = data;
		}
		break;

	case REGISTER_RSR:
		if ((data & RSR_RCV_ENABLE) == 0)
		{
			LOG("MC68901 Receiver Disabled\n");
			m_rsr = 0;
		}
		else
		{
			LOG("MC68901 Receiver Enabled\n");
			m_rsr |= RSR_RCV_ENABLE;

			if (data & RSR_SYNC_STRIP_ENABLE)
			{
				LOG("MC68901 Sync Strip Enabled\n");
				m_rsr |= RSR_SYNC_STRIP_ENABLE;
			}
			else
			{
				LOG("MC68901 Sync Strip Disabled\n");
				m_rsr &= ~RSR_SYNC_STRIP_ENABLE;
			}

			if ((m_ucr & UCR_START_STOP_1_2) == UCR_START_STOP_0_0)
			{
				if (data & RSR_FOUND_SEARCH)
				{
					LOG("MC68901 Receiver Search Mode Disabled\n");
					m_rsr |= RSR_FOUND_SEARCH;
				}
				else
				{
					LOG("MC68901 Receiver Search Mode Disabled\n");
					m_rsr &= ~RSR_FOUND_SEARCH;
				}
			}
		}
		break;

	case REGISTER_TSR:
		m_tsr = (m_tsr & (TSR_BUFFER_EMPTY | TSR_UNDERRUN_ERROR | TSR_END_OF_XMIT)) | (data & ~(TSR_BUFFER_EMPTY | TSR_UNDERRUN_ERROR | TSR_END_OF_XMIT));

		if ((data & TSR_XMIT_ENABLE) == 0)
		{
			m_tsr &= ~TSR_UNDERRUN_ERROR;
			m_underrun = false;

			if (m_tbits == 0)
				set_so((m_tsr & TSR_OUTPUT_MASK) != TSR_OUTPUT_LOW);
		}
		else
		{
			LOG("MC68901 Transmitter Enabled\n");

			switch (data & 0x06)
			{
			case TSR_OUTPUT_HI_Z:
				LOG("MC68901 Transmitter Disabled Output State : Hi-Z\n");
				break;
			case TSR_OUTPUT_LOW:
				LOG("MC68901 Transmitter Disabled Output State : 0\n");
				break;
			case TSR_OUTPUT_HIGH:
				LOG("MC68901 Transmitter Disabled Output State : 1\n");
				break;
			case TSR_OUTPUT_LOOP:
				LOG("MC68901 Transmitter Disabled Output State : Loop\n");
				break;
			}

			if (data & TSR_BREAK)
			{
				LOG("MC68901 Transmitter Break Enabled\n");
			}
			else
			{
				LOG("MC68901 Transmitter Break Disabled\n");
			}

			if (data & TSR_AUTO_TURNAROUND)
			{
				LOG("MC68901 Transmitter Auto Turnaround Enabled\n");
			}
			else
			{
				LOG("MC68901 Transmitter Auto Turnaround Disabled\n");
			}

			m_tsr &= ~TSR_END_OF_XMIT;
		}
		break;

	case REGISTER_UDR:
		LOG("MC68901 UDR %x\n", data);
		m_transmit_buffer = data;
		m_tsr &= ~TSR_BUFFER_EMPTY;
		break;
	}
}


u8 mc68901_device::get_vector()
{
	for (int ch = 15; ch >= 0; ch--)
	{
		if (BIT(m_imr, ch) && BIT(m_ipr, ch))
		{
			if (!machine().side_effects_disabled())
			{
				if (m_vr & VR_S)
				{
					/* set interrupt-in-service bit */
					m_isr |= (1 << ch);
				}

				/* clear interrupt pending bit */
				m_ipr &= ~(1 << ch);

				check_interrupts();
			}

			return (m_vr & 0xf0) | ch;
		}
	}

	if (!m_iack_chain_cb.isnull())
		return m_iack_chain_cb();
	else
		return 0x18; // Spurious irq
}

WRITE_LINE_MEMBER( mc68901_device::i0_w ) { gpio_input(0, state); }
WRITE_LINE_MEMBER( mc68901_device::i1_w ) { gpio_input(1, state); }
WRITE_LINE_MEMBER( mc68901_device::i2_w ) { gpio_input(2, state); }
WRITE_LINE_MEMBER( mc68901_device::i3_w ) { gpio_input(3, state); }
WRITE_LINE_MEMBER( mc68901_device::i4_w ) { gpio_input(4, state); }
WRITE_LINE_MEMBER( mc68901_device::i5_w ) { gpio_input(5, state); }
WRITE_LINE_MEMBER( mc68901_device::i6_w ) { gpio_input(6, state); }
WRITE_LINE_MEMBER( mc68901_device::i7_w ) { gpio_input(7, state); }


WRITE_LINE_MEMBER( mc68901_device::tai_w )
{
	timer_input(TIMER_A, state);
}


WRITE_LINE_MEMBER( mc68901_device::tbi_w )
{
	timer_input(TIMER_B, state);
}

//**************************************************************************
//  USART
//**************************************************************************

//-------------------------------------------------
//  si_w - serial data input for receiver
//-------------------------------------------------

WRITE_LINE_MEMBER(mc68901_device::si_w)
{
	m_si = state;
}

//-------------------------------------------------
//  rc_w - receiver clock input
//-------------------------------------------------

WRITE_LINE_MEMBER(mc68901_device::rc_w)
{
	if (state != m_rc)
	{
		// receiver active on rising edge
		m_rc = state;
		if (state && (m_rsr & RSR_RCV_ENABLE) && (m_tsr & TSR_OUTPUT_MASK) != TSR_OUTPUT_LOOP)
			rx_clock(m_si);
	}
}

//-------------------------------------------------
//  tc_w - transmitter clock input
//-------------------------------------------------

WRITE_LINE_MEMBER(mc68901_device::tc_w)
{
	if (state != m_tc)
	{
		// transmitter active on falling edge
		m_tc = state;
		if (!state && ((m_tsr & TSR_XMIT_ENABLE) || !(m_tsr & TSR_END_OF_XMIT)))
			tx_clock();
		else if (state && (m_rsr & RSR_RCV_ENABLE) && (m_tsr & TSR_OUTPUT_MASK) == TSR_OUTPUT_LOOP)
			rx_clock(m_so);
	}
}

//-------------------------------------------------
//  set_so - set serial output
//-------------------------------------------------

void mc68901_device::set_so(bool state)
{
	if (m_so != state)
	{
		m_so = state;
		m_out_so_cb(state);
	}
}

//-------------------------------------------------
//  rx_frame_start - begin a new frame of received
//  data (following start bit for async mode)
//-------------------------------------------------

void mc68901_device::rx_frame_start()
{
	m_rframe = 0;
	m_rbits = (m_ucr & UCR_WORD_LENGTH_MASK) >> 5;
	m_rparity = (m_ucr & UCR_PARITY_EVEN) == UCR_PARITY_ODD;
}

//-------------------------------------------------
//  rx_sync_found - notify that a sync character
//  was found
//-------------------------------------------------

void mc68901_device::rx_sync_found()
{
	m_rsr |= RSR_FOUND_SEARCH;
	LOGMASKED(LOG_RCV, "USART sync character found (%02X)\n", m_scr);

	// causes error interrupt, but does not fill receiver buffer
	rx_error();
}

//-------------------------------------------------
//  rx_async_frame_complete - finish receiving one
//  character in asynchronous mode
//-------------------------------------------------

void mc68901_device::rx_async_frame_complete()
{
	if (m_rsr & RSR_BUFFER_FULL)
	{
		LOGMASKED(LOG_RCV, "USART discarding received character %02X (%s)\n", m_rframe & 0xff, (m_rframe == 0 && !m_last_si) ? "break" : "overrun");
		m_next_rsr |= (m_rframe == 0 && !m_last_si) ? RSR_BREAK : RSR_OVERRUN_ERROR;
	}
	else if (m_rsr & RSR_OVERRUN_ERROR)
	{
		if (m_rframe == 0 && !m_last_si)
			m_rsr |= RSR_BREAK;
	}
	else
	{
		// load the receiver buffer
		m_receive_buffer = m_rframe & 0xff;
		m_rsr |= RSR_BUFFER_FULL;

		// set error flags
		m_rsr &= ~(RSR_PARITY_ERROR | RSR_FRAME_ERROR | RSR_BREAK);
		if (m_rparity)
			m_rsr |= RSR_PARITY_ERROR;
		if (!m_last_si)
			m_rsr |= m_rframe == 0 ? RSR_BREAK : RSR_FRAME_ERROR;
		LOGMASKED(LOG_RCV, "USART received character: %02X (PE = %d, FE = %d, B = %d)\n", m_receive_buffer,
			m_rparity,
			!m_last_si && m_rframe != 0,
			 !m_last_si && m_rframe == 0);

		// set normal or error interrupt (if the latter is disabled, always use the former)
		if ((m_rparity || !m_last_si) && (m_ier & IR_RCV_ERROR))
			rx_error();
		else
			rx_buffer_full();
	}
}

//-------------------------------------------------
//  rx_sync_frame_complete - finish receiving one
//  character in synchronous mode (error flags are
//  different from asynchronous mode)
//-------------------------------------------------

void mc68901_device::rx_sync_frame_complete()
{
	// check if sync character matches
	bool match = (m_rframe & 0xff) == (m_scr & (0xff >> ((m_ucr & UCR_WORD_LENGTH_MASK) >> 5))) && !m_rparity;

	// suppress sync characters if strip option set
	if (!match || !(m_rsr & RSR_SYNC_STRIP_ENABLE))
	{
		if (m_rsr & RSR_BUFFER_FULL)
			m_next_rsr |= RSR_OVERRUN_ERROR;
		else if (!(m_rsr & RSR_OVERRUN_ERROR))
		{
			// load the receiver buffer
			m_receive_buffer = m_rframe & 0xff;
			m_rsr |= RSR_BUFFER_FULL;

			// set error flags
			m_rsr &= ~(RSR_FRAME_ERROR | RSR_PARITY_ERROR | RSR_MATCH);
			if (m_rparity)
				m_rsr |= RSR_PARITY_ERROR;
			if (match)
				m_rsr |= RSR_MATCH;
			LOGMASKED(LOG_RCV, "USART received character: %02X (PE = %d, sync %smatched)\n", m_receive_buffer,
				m_rparity,
				match ? "not " : "");

			// set normal or error interrupt (if the latter is disabled, always use the former)
			if ((m_rparity || match) && (m_ier & IR_RCV_ERROR))
				rx_error();
			else
				rx_buffer_full();
		}
	}
	else
		LOGMASKED(LOG_RCV, "USART sync character stripped (%02X)\n", m_rframe & 0xff);
}

//-------------------------------------------------
//  rx_clock - process one active transition on
//  the receiver clock
//-------------------------------------------------

void mc68901_device::rx_clock(bool si)
{
	m_rclk++;
	if (m_rclk >= 244)
		m_rclk &= 15;
	m_si_scan = (m_si_scan >> 1) | (si ? 0x80 : 0);
	bool rclk_sync = (m_ucr & UCR_CLOCK_DIVIDE_16) == UCR_CLOCK_DIVIDE_1 || (m_rclk >= 4 && (m_si_scan & 0xe0) == (m_last_si ? 0 : 0xe0));
	bool sync_mode = (m_ucr & UCR_START_STOP_1_2) == UCR_START_STOP_0_0;
	if (rclk_sync)
	{
		LOGMASKED(LOG_RCV, "SI = %d (synchronized); RSR = %02X; rframe = %X; %d rbits, %d rclks\n", si, m_rsr, m_rframe, m_rbits, m_rclk);
		m_last_si = si;
		if (si && !sync_mode && !(m_rsr & RSR_CHAR_IN_PROGRESS))
		{
			m_rframe = 0x100;
			if (!(m_rsr & RSR_BUFFER_FULL) && (m_rsr & RSR_BREAK))
			{
				// valid 0 to 1 transition ends break condition
				m_rsr &= ~RSR_BREAK;
				rx_error();
			}
		}
		m_rclk = 0;
	}

	if ((m_ucr & UCR_CLOCK_DIVIDE_16) == UCR_CLOCK_DIVIDE_1 || (m_rclk & 15) == 8)
	{
		if (sync_mode && !(m_rsr & RSR_FOUND_SEARCH))
		{
			// search mode: continuous comparison
			m_rframe = (m_rframe >> 1) | (m_last_si ? 0x100 : 0);
			if ((m_ucr & UCR_PARITY_ENABLED) && (m_ucr & UCR_WORD_LENGTH_MASK) == UCR_WORD_LENGTH_8)
			{
				// check calculated parity of 8-bit sync character
				if ((m_rframe & 0xff) == m_scr && m_last_si == ((m_ucr & UCR_PARITY_EVEN) ? m_scr_parity : !m_scr_parity))
				{
					rx_sync_found();
					rx_frame_start();
				}
			}
			else
			{
				// parity, if any, must be included in SCR when words are less than 8 bits
				int frame_bits = ((m_ucr & UCR_PARITY_ENABLED) ? 9 : 8) - ((m_ucr & UCR_WORD_LENGTH_MASK) >> 5);
				if ((m_rframe >> (9 - frame_bits)) == (m_scr & ((1 << frame_bits) - 1)))
				{
					rx_sync_found();
					rx_frame_start();
				}
			}
		}
		else if (sync_mode || (m_rsr & RSR_CHAR_IN_PROGRESS))
		{
			if (m_rbits > 8)
			{
				rx_async_frame_complete();
				m_rframe = m_last_si ? 0x100 : 0;
				m_rsr &= ~RSR_CHAR_IN_PROGRESS;
			}
			else
			{
				LOGMASKED(LOG_RCV, "USART shifting in %d %s bit\n", m_last_si, m_rbits < 8 ? "data" : "parity");
				m_rframe = (m_rframe >> 1) | (m_last_si ? 0x100 : 0);
				if (m_last_si)
					m_rparity = !m_rparity;
				m_rbits++;

				if (m_rbits == 8)
				{
					// adjust for fewer than 8 data bits
					m_rframe >>= (m_ucr & UCR_WORD_LENGTH_MASK) >> 5;

					// adjust for no parity
					if (!(m_ucr & UCR_PARITY_ENABLED))
					{
						m_rframe >>= 1;
						m_rparity = false;
						m_rbits++;
					}
				}

				if (m_rbits > 8 && sync_mode)
				{
					rx_sync_frame_complete();

					// one character follows another in sync mode
					rx_frame_start();
				}
			}
		}
		else if (!m_last_si && BIT(m_rframe, 8))
		{
			// start bit valid
			LOGMASKED(LOG_RCV, "USART received start bit\n");
			m_rsr |= RSR_CHAR_IN_PROGRESS;
			rx_frame_start();
		}
	}
}

//-------------------------------------------------
//  tx_frame_load - load one character into the
//  shift register for transmission
//-------------------------------------------------

void mc68901_device::tx_frame_load(u8 data)
{
	// set up output shift register
	m_osr = data;
	m_tbits = 9 - ((m_ucr & UCR_WORD_LENGTH_MASK) >> 5);
	m_tparity = (m_ucr & UCR_PARITY_EVEN) == UCR_PARITY_ODD;

	// add start and stop bits for asynchronous mode
	if ((m_ucr & UCR_START_STOP_1_2) != UCR_START_STOP_0_0)
	{
		m_osr = (m_osr << 1) | (1 << m_tbits);
		m_tbits += 2;
	}
}

//-------------------------------------------------
//  tx_clock - process one active edge on the
//  transmitter clock
//-------------------------------------------------

void mc68901_device::tx_clock()
{
	if (m_tclk != 0)
	{
		m_tclk--;
		return;
	}

	m_underrun = false;

	bool sync_mode = (m_ucr & UCR_START_STOP_1_2) == UCR_START_STOP_0_0;
	if (m_tbits == (sync_mode ? 2 : 3))
	{
		// inject the calculated parity or skip that bit
		if (m_ucr & UCR_PARITY_ENABLED)
			m_osr = (m_osr << 1) | m_tparity;
		else
			m_tbits--;
	}

	bool send_break = !sync_mode && (m_tsr & TSR_BREAK);
	bool tbusy = false;
	if (m_tbits != 0)
	{
		m_tbits--;
		if (m_tbits != 0)
			tbusy = true;
		else if (!(m_tsr & TSR_XMIT_ENABLE))
		{
			LOGMASKED(LOG_XMIT, "USART transmitter disabled\n");

			// transmitter is now effectively disabled
			m_tsr |= TSR_END_OF_XMIT;
			tx_error();

			// automatic turnaround enables the receiver
			if (m_tsr & TSR_AUTO_TURNAROUND)
			{
				m_rsr |= RSR_RCV_ENABLE;
				m_tsr &= ~TSR_AUTO_TURNAROUND;
			}
		}
		else if ((m_tsr & TSR_BUFFER_EMPTY) && !(m_tsr & TSR_UNDERRUN_ERROR) && !send_break)
		{
			LOGMASKED(LOG_XMIT, "USART transmitter underrun\n");

			// underrun error condition
			m_tsr |= TSR_UNDERRUN_ERROR;
			m_underrun = true;
			tx_error();
		}
	}

	if (!tbusy && (m_tsr & TSR_XMIT_ENABLE))
	{
		// break inhibits reload
		if (!(m_tsr & TSR_BUFFER_EMPTY) && !send_break)
		{
			LOGMASKED(LOG_XMIT, "USART loading character (%02X)\n", m_transmit_buffer);

			// empty buffer into shift register
			m_tsr |= TSR_BUFFER_EMPTY;
			tx_buffer_empty();
			tx_frame_load(m_transmit_buffer);
			tbusy = true;
		}
		else if (sync_mode)
		{
			LOGMASKED(LOG_XMIT, "USART loading sync character (%02X)\n", m_scr);

			// transmit sync characters if nothing else is loaded
			tx_frame_load(m_scr);
			tbusy = true;
		}
	}

	if (tbusy)
	{
		LOGMASKED(LOG_XMIT, "USART shifting out %d %s bit\n", BIT(m_osr, 0),
			m_tbits == 1 && !sync_mode ? "stop" : m_tbits == (sync_mode ? 1 : 2) ? "parity" : "data or start");

		// shift out one bit
		set_so(BIT(m_osr, 0));
		if (BIT(m_osr, 0))
			m_tparity = !m_tparity;
		m_osr >>= 1;

		if (m_tbits == 1 && (m_ucr & UCR_START_STOP_1_2) >= UCR_START_STOP_1_15)
		{
			// 1½ or 2 stop bits selected
			if (m_ucr & UCR_CLOCK_DIVIDE_16)
				m_tclk = (m_ucr & UCR_START_STOP_1_2) == UCR_START_STOP_1_2 ? 31 : 23;
			else
				m_tclk = (m_ucr & UCR_START_STOP_1_2) == UCR_START_STOP_1_2 ? 1 : 0;
		}
		else if (m_ucr & UCR_CLOCK_DIVIDE_16)
			m_tclk = 15;
	}
	else if (!(m_tsr & TSR_XMIT_ENABLE))
	{
		// high/low output on SO (Hi-Z not supported)
		set_so((m_tsr & TSR_OUTPUT_MASK) != TSR_OUTPUT_LOW);
	}
	else if (send_break)
	{
		set_so(false);
		m_tbits = 1;
	}
	else
	{
		// asynchronous marking condition
		set_so(true);
	}
}
