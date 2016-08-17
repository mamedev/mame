// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    NEC uPD65031 'BLINK' emulation

    the uPD65031 manages almost everything in the Z88:
    - memory bankswitch
    - interrupts
    - RTC
    - LCD
    - keyboard
    - serial
    - speaker

    TODO:
    - coma and snooze mode
    - speaker controlled by txd
    - EPROM programming
    - UART

*********************************************************************/


#include "upd65031.h"


// device type definition
const device_type UPD65031 = &device_creator<upd65031_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define SPEAKER_ALARM_FREQ   attotime::from_hz(3200)

// internal registers
enum
{
	// write registers
	REG_PB0  = 0x70,        // pixel base 0
	REG_PB1  = 0x71,        // pixel base 1
	REG_PB2  = 0x72,        // pixel base 2
	REG_PB3  = 0x73,        // pixel base 3
	REG_SBR  = 0x74,        // screen base register

	REG_COM  = 0xb0,        // command register
	REG_INT  = 0xb1,        // interrupt control
	REG_EPR  = 0xb3,        // EPROM programming
	REG_TACK = 0xb4,        // RTC acknowledge
	REG_TMK  = 0xb5,        // RTC interrupt mask
	REG_ACK  = 0xb6,        // interrupt acknowledge

	REG_SR0  = 0xd0,        // segment register 0
	REG_SR1  = 0xd1,        // segment register 1
	REG_SR2  = 0xd2,        // segment register 2
	REG_SR3  = 0xd3,        // segment register 3

	REG_RXC  = 0xe2,        // UART receiver control
	REG_TXD  = 0xe3,        // UART transmit data
	REG_TXC  = 0xe4,        // UART transmit control
	REG_UMK  = 0xe5,        // UART interrupt mask
	REG_UAK  = 0xe6,        // UART interrupt acknowledge


	// read registers
	REG_STA  = 0xb1,        // interrupt status
	REG_KBD  = 0xb2,        // keyboard read
	REG_TSTA = 0xb5,        // RTC interrupt status

	REG_TIM0 = 0xd0,        // RTC 5ms counter
	REG_TIM1 = 0xd1,        // RTC seconds counter (6 bits)
	REG_TIM2 = 0xd2,        // RTC minutes counter
	REG_TIM3 = 0xd3,        // RTC minutes/256 counter
	REG_TIM4 = 0xd4,        // RTC minutes/65536 counter (5 bits)

	REG_RXD  = 0xe0,        // UART receive data register
	REG_RXE  = 0xe1,        // UART extended receiver data
	REG_UIT  = 0xe5         // UART interrupt status
};

//mode
enum
{
	STATE_AWAKE = 0,
	STATE_SNOOZE,
	STATE_COMA
};

// interrupt status
#define STA_FLAPOPEN        0x80
#define STA_A19             0x40
#define STA_FLAP            0x20
#define STA_UART            0x10
#define STA_BTL             0x08
#define STA_KEY             0x04
#define STA_TIME            0x01

// interrupt control
#define INT_KWAIT           0x80
#define INT_A19             0x40
#define INT_FLAP            0x20
#define INT_UART            0x10
#define INT_BTL             0x08
#define INT_KEY             0x04
#define INT_TIME            0x02
#define INT_GINT            0x01

// command register
#define COM_SRUN            0x80
#define COM_SBIT            0x40
#define COM_OVERP           0x20
#define COM_RESTIM          0x10
#define COM_PROGRAM         0x08
#define COM_RAMS            0x04
#define COM_VPPON           0x02
#define COM_LCDON           0x01

// EPROM programming register
#define EPR_PD1             0x80
#define EPR_PD0             0x40
#define EPR_PGMD            0x20
#define EPR_EOED            0x10
#define EPR_SE3D            0x08
#define EPR_PGMP            0x04
#define EPR_EOEP            0x02
#define EPR_SE3P            0x01

// RTC interrupt status
#define TSTA_MIN            0x04
#define TSTA_SEC            0x02
#define TSTA_TICK           0x01

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void upd65031_device::interrupt_refresh()
{
	if ((m_int & INT_GINT) && ((m_int & m_sta & 0x7c) || ((m_int & INT_TIME) && (m_sta & STA_TIME))))
	{
		if (LOG) logerror("uPD65031 '%s': set int\n", tag());

		m_write_int(ASSERT_LINE);
	}
	else
	{
		if (LOG) logerror("uPD65031 '%s': clear int\n", tag());

		m_write_int(CLEAR_LINE);
	}
}


inline void upd65031_device::update_rtc_interrupt()
{
	// any ints occurred?
	if ((m_int & INT_GINT) && (m_int & INT_TIME) && (m_tsta & (TSTA_MIN | TSTA_SEC | TSTA_TICK)))
		m_sta |= STA_TIME;
	else
		m_sta &= ~STA_TIME;
}


inline void upd65031_device::set_mode(int mode)
{
	if (m_mode != mode)
	{
		m_mode = mode;

		switch(mode)
		{
		case STATE_AWAKE:
			//TODO
			break;
		case STATE_SNOOZE:
			//TODO
			break;
		case STATE_COMA:
			//TODO
			break;
		}
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd65031_device - constructor
//-------------------------------------------------

upd65031_device::upd65031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD65031, "NEC uPD65031", tag, owner, clock, "upd65031", __FILE__),
	m_read_kb(*this),
	m_write_int(*this),
	m_write_nmi(*this),
	m_write_spkr(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd65031_device::device_start()
{
	// resolve callbacks
	m_read_kb.resolve_safe(0);
	m_write_int.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_spkr.resolve_safe();

	// bind delegates
	m_screen_update_cb.bind_relative_to(*owner());
	m_out_mem_cb.bind_relative_to(*owner());

	// allocate timers
	m_rtc_timer = timer_alloc(TIMER_RTC);
	m_flash_timer = timer_alloc(TIMER_FLASH);
	m_speaker_timer = timer_alloc(TIMER_SPEAKER);
	m_rtc_timer->adjust(attotime::from_msec(5), 0, attotime::from_msec(5));
	m_flash_timer->adjust(attotime::from_hz(2), 0, attotime::from_hz(2));
	m_speaker_timer->reset();

	// state saving
	save_item(NAME(m_mode));
	save_item(NAME(m_lcd_regs));
	save_item(NAME(m_tim));
	save_item(NAME(m_sr));
	save_item(NAME(m_sta));
	save_item(NAME(m_int));
	save_item(NAME(m_ack));
	save_item(NAME(m_tsta));
	save_item(NAME(m_tmk));
	save_item(NAME(m_tack));
	save_item(NAME(m_com));
	save_item(NAME(m_flash));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd65031_device::device_reset()
{
	memset(m_lcd_regs, 0, sizeof(m_lcd_regs));
	memset(m_tim, 0, sizeof(m_tim));
	memset(m_sr, 0, sizeof(m_sr));
	m_sta = 0;
	m_int = 0;
	m_ack = 0;
	m_tsta = 0;
	m_tmk = TSTA_TICK | TSTA_SEC | TSTA_MIN;
	m_tack = 0;
	m_com = 0;
	m_flash = 0;
	m_mode = 0;
	set_mode(STATE_AWAKE);

	if (!m_out_mem_cb.isnull())
	{
		// reset bankswitch
		m_out_mem_cb(0, 0, 0);
		m_out_mem_cb(1, 0, 0);
		m_out_mem_cb(2, 0, 0);
		m_out_mem_cb(3, 0, 0);
	}
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void upd65031_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RTC:

		// if a key is pressed sets the interrupt
		if ((m_int & INT_GINT) && (m_int & INT_KEY) && m_read_kb(0) != 0xff)
		{
			if (LOG) logerror("uPD65031 '%s': Keyboard interrupt!\n", tag());

			// awakes CPU from snooze on key down
			if (m_mode == STATE_SNOOZE)
				set_mode(STATE_AWAKE);

			m_sta |= STA_KEY;
		}
		else
		{
			m_sta &= ~STA_KEY;
		}

		// hold clock at reset? - in this mode it doesn't update
		if (!(m_com & COM_RESTIM))
		{
			bool irq_change = false;

			// update 5 millisecond counter
			m_tim[0]++;

			// tick
			if (m_tim[0] & 1)
			{
				// set tick int has occurred
				if (m_tmk & TSTA_TICK)
				{
					m_tsta |= TSTA_TICK;
					irq_change = true;
				}
			}

			if (m_tim[0] == 200)
			{
				m_tim[0] = 0;

				// set seconds int has occurred
				if (m_tmk & TSTA_SEC)
				{
					m_tsta |= TSTA_SEC;
					irq_change = true;
				}

				m_tim[1]++;

				if (m_tim[1] == 60)
				{
					// set minutes int has occurred
					if (m_tmk & TSTA_MIN)
					{
						m_tsta |= TSTA_MIN;
						irq_change = true;
					}
					m_tim[1] = 0;
					m_tim[2]++;

					if (m_tim[2] == 0) // overflowed from 255
					{
						m_tim[3]++;

						if (m_tim[3] == 0) // overflowed from 255
						{
							m_tim[4]++;

							if (m_tim[4] == 32)
								m_tim[4] = 0;
						}
					}
				}
			}

			if ((m_int & INT_GINT) && (m_int & INT_TIME) && irq_change && !(m_sta & STA_FLAPOPEN))
			{
				set_mode(STATE_AWAKE);

				update_rtc_interrupt();
			}

			// refresh interrupt
			interrupt_refresh();
		}
		break;
	case TIMER_FLASH:
		m_flash = !m_flash;
		break;
	case TIMER_SPEAKER:
		m_speaker_state = !m_speaker_state;
		m_write_spkr(m_speaker_state ? 1 : 0);
		break;
	}
}


//-------------------------------------------------
//  screen_update
//-------------------------------------------------

UINT32 upd65031_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_screen_update_cb.isnull() && (m_com & COM_LCDON))
		m_screen_update_cb(bitmap, m_lcd_regs[4], m_lcd_regs[2], m_lcd_regs[3], m_lcd_regs[0], m_lcd_regs[1], m_flash);
	else
		bitmap.fill(0, cliprect);

	return 0;
}

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( upd65031_device::read )
{
	UINT8 port = offset & 0xff;

	switch (port)
	{
		case REG_STA:   // read interrupt status
			return m_sta;

		case REG_KBD:
		{
			// if set, reading the keyboard will put into snooze
			if (m_int & INT_KWAIT)
			{
				set_mode(STATE_SNOOZE);

				if (LOG) logerror("uPD65031 '%s': entering snooze!\n", tag());
			}

			UINT8 data = m_read_kb(offset>>8);

			if (LOG) logerror("uPD65031 '%s': key r %02x: %02x\n", tag(), offset>>8, data);

			return data;
		}

		// read real time clock status
		case REG_TSTA:
			if (LOG) logerror("uPD65031 '%s': tsta r: %02x\n", tag(), m_tsta);
			return m_tsta & 0x07;

		// read real time clock counters
		case REG_TIM0:
			if (LOG) logerror("uPD65031 '%s': TIM0 r: %02x\n", tag(), m_tim[0]);
			return m_tim[0];
		case REG_TIM1:
			if (LOG) logerror("uPD65031 '%s': TIM1 r: %02x\n", tag(), m_tim[1]);
			return m_tim[1];
		case REG_TIM2:
			if (LOG) logerror("uPD65031 '%s': TIM2 r: %02x\n", tag(), m_tim[2]);
			return m_tim[2];
		case REG_TIM3:
			if (LOG) logerror("uPD65031 '%s': TIM3 r: %02x\n", tag(), m_tim[3]);
			return m_tim[3];
		case REG_TIM4:
			if (LOG) logerror("uPD65031 '%s': TIM4 r: %02x\n", tag(), m_tim[4]);
			return m_tim[4];

		// UART
		case REG_RXD:
		case REG_RXE:
		case REG_UIT:
			// TODO
			return 0;

		default:
			logerror("uPD65031 '%s': blink r: %04x\n", tag(), offset);
			return 0;
	}
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( upd65031_device::write )
{
	UINT8 port = offset & 0xff;

	switch (port)
	{
		// gfx registers
		case REG_PB0:
		case REG_PB1:
		case REG_PB2:
		case REG_PB3:
		case REG_SBR:
			m_lcd_regs[port - REG_PB0] = ((offset & 0xff00) | data);
			break;

		case REG_COM:   // command register
			if (LOG) logerror("uPD65031 '%s': com w: %02x\n", tag(), data);

			// reset clock?
			if (data & COM_RESTIM)
				m_tim[0] = m_tim[1] = m_tim[2] = m_tim[3] = m_tim[4] = 0;

			if ((data & COM_SRUN) && !(data & COM_SBIT))
			{
				// constant tone used for keyclick and alarm
				m_speaker_timer->adjust(SPEAKER_ALARM_FREQ, 0, SPEAKER_ALARM_FREQ);
			}
			else
			{
				if (!(data & COM_SRUN))
				{
					// speaker controlled by SBIT
					m_speaker_state = BIT(data, 6);
					m_write_spkr(m_speaker_state);
				}
				else
				{
					// speaker controlled by txd line
					// TODO
				}

				m_speaker_timer->reset();
			}

			// bit 2 controls the lower 8kb of memory
			if (BIT(m_com^data, 2) && !m_out_mem_cb.isnull())
				m_out_mem_cb(0, m_sr[0], BIT(data, 2));

			m_com = data;
			break;

		case REG_INT:   // interrupt control
			if (LOG) logerror("uPD65031 '%s': int w: %02x\n", tag(), data);

			m_int = data;

			// refresh ints
			update_rtc_interrupt();
			interrupt_refresh();
			break;

		case REG_EPR:   // EPROM programming register
			if (LOG) logerror("uPD65031 '%s': epr w: %02x\n", tag(), data);
			// TODO
			break;

		case REG_TACK:  // rtc interrupt acknowledge
			if (LOG) logerror("uPD65031 '%s': tack w: %02x\n", tag(), data);

			// clear ints that have occurred
			m_tsta &= ~(data & 0x07);
			m_tack = data;

			// refresh ints
			update_rtc_interrupt();
			interrupt_refresh();
			break;

		case REG_TMK:   // write rtc interrupt mask
			if (LOG) logerror("uPD65031 '%s': tmk w: %02x\n", tag(), data);

			m_tmk = data & 0x07;
			break;

		case REG_ACK:   // acknowledge ints
			if (LOG) logerror("uPD65031 '%s': ack w: %02x\n", tag(), data);

			m_ack = data;
			m_sta &= ~(data & 0x7f);

			// refresh ints
			interrupt_refresh();
			break;

		// Segment registers
		case REG_SR0:
		case REG_SR1:
		case REG_SR2:
		case REG_SR3:
			if (!m_out_mem_cb.isnull() && m_sr[port & 3] != data)
				m_out_mem_cb(port & 3, data, BIT(m_com, 2));

			m_sr[port & 3] = data;
			break;

		// UART
		case REG_RXC:
		case REG_TXD:
		case REG_TXC:
		case REG_UMK:
		case REG_UAK:
			if (LOG) logerror("uPD65031 '%s': UART w: %02x %02x\n", tag(), port & 7 , data);
			// TODO
			break;

		default:
			logerror("uPD65031 '%s': blink w: %04x %02x\n", tag(), offset, data);
			break;
	}
}


//-------------------------------------------------
//  flp line
//-------------------------------------------------

WRITE_LINE_MEMBER( upd65031_device::flp_w )
{
	if (!(m_sta & STA_FLAPOPEN) && state)
	{
		// set interrupt on rising edge
		m_sta |= STA_FLAP;

		interrupt_refresh();
	}

	if (state)
		m_sta |= STA_FLAPOPEN;
	else
		m_sta &= ~STA_FLAPOPEN;
}

//-------------------------------------------------
//  battery low line
//-------------------------------------------------

WRITE_LINE_MEMBER( upd65031_device::btl_w )
{
	if (state)
		m_sta |= STA_BTL;
	else
		m_sta &= ~STA_BTL;
}
