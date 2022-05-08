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
    - UART Loopback mode

*********************************************************************/


#include "emu.h"
#include "upd65031.h"

#define VERBOSE 0
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(UPD65031, upd65031_device, "upd65031", "NEC uPD65031")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

namespace {

static constexpr uint32_t SPEAKER_ALARM_FREQ = 3200;

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
static constexpr uint8_t STA_FLAPOPEN = 0x80;   // Flap status
static constexpr uint8_t STA_A19      = 0x40;   // High level on A19 occurred during Coma
static constexpr uint8_t STA_FLAP     = 0x20;   // Flap interrupt
static constexpr uint8_t STA_UART     = 0x10;   // UART interrupt
static constexpr uint8_t STA_BTL      = 0x08;   // Battery low interrupt
static constexpr uint8_t STA_KEY      = 0x04;   // Keyboard interrupt
static constexpr uint8_t STA_TIME     = 0x01;   // RTC interrupt

// interrupt control
static constexpr uint8_t INT_KWAIT    = 0x80;   // Reading the keyboard will Snooze
static constexpr uint8_t INT_A19      = 0x40;   // A19 high will exit Coma mode
static constexpr uint8_t INT_FLAP     = 0x20;   // Enable Flap open interrupt
static constexpr uint8_t INT_UART     = 0x10;   // Enable UART interrupt
static constexpr uint8_t INT_BTL      = 0x08;   // Enable Battery low interrupt
static constexpr uint8_t INT_KEY      = 0x04;   // Enable Keyboard interrupt
static constexpr uint8_t INT_TIME     = 0x02;   // Enable RTC interrupt
static constexpr uint8_t INT_GINT     = 0x01;   // Global interrupts mask

// acknowledge interrupts
static constexpr uint8_t ACK_A19     = 0x40;   // Acknowledge A19 interrupt
static constexpr uint8_t ACK_FLAP    = 0x20;   // Acknowledge Flap interrupt
static constexpr uint8_t ACK_BTL     = 0x08;   // Acknowledge battery low interrupt
static constexpr uint8_t ACK_KEY     = 0x04;   // Acknowledge keyboard interrupt

// command register
static constexpr uint8_t COM_SRUN     = 0x80;   // Speaker source (0: manual, 1: auto)
static constexpr uint8_t COM_SBIT     = 0x40;   // Speaker source for SRUN=1 (0: 3200Hz, 1: TxD)
static constexpr uint8_t COM_OVERP    = 0x20;   // Overprogram EPROMs
static constexpr uint8_t COM_RESTIM   = 0x10;   // RTC reset
static constexpr uint8_t COM_PROGRAM  = 0x08;   // EPROM programming
static constexpr uint8_t COM_RAMS     = 0x04;   // Enable boot ROM bank
static constexpr uint8_t COM_VPPON    = 0x02;   // Programming voltage ON
static constexpr uint8_t COM_LCDON    = 0x01;   // LCD ON

// EPROM programming register
static constexpr uint8_t EPR_PD       = 0xc0;   // Two bits representing the length of delay period
static constexpr uint8_t EPR_PGMD     = 0x20;   // State of program pulse during delay period
static constexpr uint8_t EPR_EOED     = 0x10;   // State of EOE during delay period
static constexpr uint8_t EPR_SE3D     = 0x08;   // State of slot 3 select during delay period
static constexpr uint8_t EPR_PGMP     = 0x04;   // State of program pulse during porch period
static constexpr uint8_t EPR_EOEP     = 0x02;   // State of EOE during porch period
static constexpr uint8_t EPR_SE3P     = 0x01;   // State of slot 3 select during porch period

// RTC interrupt status
static constexpr uint8_t TSTA_MIN     = 0x04;   // Minute interrupt has occurred
static constexpr uint8_t TSTA_SEC     = 0x02;   // Second interrupt has occurred
static constexpr uint8_t TSTA_TICK    = 0x01;   // Tick interrupt has occurred

// UART extended receive data
static constexpr uint8_t RXE_FE       = 0x20;   // Frame error
static constexpr uint8_t RXE_RXDB     = 0x10;   // RXD line state
static constexpr uint8_t RXE_TCLK     = 0x08;   // Transmit clock
static constexpr uint8_t RXE_RCLK     = 0x04;   // Receive clock
static constexpr uint8_t RXE_PAR      = 0x02;   // Parity bit
static constexpr uint8_t RXE_START    = 0x01;   // Start bit (should be zero)

// UART receive control
static constexpr uint8_t RXC_SHTW     = 0x80;   // Short word mode
static constexpr uint8_t RXC_LOOP     = 0x40;   // Loopback mode
static constexpr uint8_t RXC_UART     = 0x20;   // Reset
static constexpr uint8_t RXC_ARTS     = 0x10;   // Auto RTS mode
static constexpr uint8_t RXC_IRTS     = 0x08;   // Invert RTS
static constexpr uint8_t RXC_BAUD     = 0x07;   // Baud rate

// UART transmit control
static constexpr uint8_t TXC_UTEST    = 0x80;   // Fast baud rate
static constexpr uint8_t TXC_IDCD     = 0x40;   // DCD interrupt when low (0 for when high)
static constexpr uint8_t TXC_ICTS     = 0x20;   // CTD interrupt when low (0 for when high)
static constexpr uint8_t TXC_ATX      = 0x10;   // Auto transmit mode
static constexpr uint8_t TXC_ITX      = 0x08;   // Invert Tx
static constexpr uint8_t TXC_BAUD     = 0x07;   // Baud rate

// UART interrupt status
static constexpr uint8_t UIT_RSRD     = 0x80;   // Receive shift register full
static constexpr uint8_t UIT_DCDI     = 0x40;   // DCD interrupt
static constexpr uint8_t UIT_CTSI     = 0x20;   // CTS interrupt
static constexpr uint8_t UIT_TDRE     = 0x10;   // Transmit register empty
static constexpr uint8_t UIT_RDRF     = 0x04;   // Receive register full
static constexpr uint8_t UIT_DCD      = 0x02;   // Inverse of the DCD line level
static constexpr uint8_t UIT_CTS      = 0x01;   // Inverse of the CTS line level

// UART interrupt mask
static constexpr uint8_t UMK_DCD      = 0x40;   // DCD interrupts are enabled
static constexpr uint8_t UMK_CTS      = 0x20;   // CTS interrupts are enabled
static constexpr uint8_t UMK_TDRE     = 0x10;   // Transmit data register empty interrupt enabled
static constexpr uint8_t UMK_RDRF     = 0x04;   // Receive data register full interrupt enabled

// UART interrupt acknowledge register
static constexpr uint8_t UAK_DCD      = 0x40;   // Acknowledge DCD interrupt
static constexpr uint8_t UAK_CTS      = 0x20;   // Acknowledge CTS interrupt

} // anonymous namespace

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void upd65031_device::interrupt_refresh()
{
	if ((m_int & INT_GINT) && ((m_int & m_sta & 0x7c) || ((m_int & INT_TIME) && (m_sta & STA_TIME))))
	{
		LOG("%s: set int\n", machine().describe_context());

		m_write_int(ASSERT_LINE);
	}
	else
	{
		LOG("%s: clear int\n", machine().describe_context());

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

inline void upd65031_device::update_uart_interrupt()
{
	if ((m_int & INT_UART) && (m_uit & m_umk))
		m_sta |= STA_UART;
	else
		m_sta &= ~STA_UART;

	interrupt_refresh();
}

inline void upd65031_device::update_tx(int state)
{
	m_txd_line = state;
	m_write_txd(m_txd_line);

	if ((m_com & COM_SRUN) && (m_com & COM_SBIT))
		m_write_spkr(m_txd_line);
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

upd65031_device::upd65031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, UPD65031, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_read_kb(*this),
	m_write_int(*this),
	m_write_nmi(*this),
	m_write_spkr(*this),
	m_write_txd(*this),
	m_write_rts(*this),
	m_write_dtr(*this),
	m_write_vpp(*this),
	m_screen_update_cb(*this),
	m_out_mem_cb(*this),
	m_sta(0),
	m_int(0)
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
	m_write_txd.resolve_safe();
	m_write_rts.resolve_safe();
	m_write_dtr.resolve_safe();
	m_write_vpp.resolve_safe();

	// bind delegates
	m_screen_update_cb.resolve();
	m_out_mem_cb.resolve();

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
	save_item(NAME(m_uit));
	save_item(NAME(m_umk));
	save_item(NAME(m_txc));
	save_item(NAME(m_rxe));
	save_item(NAME(m_rxc));
	save_item(NAME(m_txd_line));
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
	m_uit = UIT_TDRE;   // Transmit register empty
	m_umk = 0x00;
	m_rxe = 0x00;
	m_rxc = RXC_SHTW | 0x05;            // 9600 baud, 1 Stop Bit
	m_txc = TXC_IDCD | TXC_ICTS | 0x05; // 9600 baud
	m_txd_line = 0;
	set_mode(STATE_AWAKE);

	if (!m_out_mem_cb.isnull())
	{
		// reset bankswitch
		m_out_mem_cb(0, 0, 0);
		m_out_mem_cb(1, 0, 0);
		m_out_mem_cb(2, 0, 0);
		m_out_mem_cb(3, 0, 0);
	}

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(9600);
	transmit_register_reset();
	receive_register_reset();
	m_write_rts(1);
	m_write_dtr(1);
	m_write_vpp(0);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void upd65031_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_RTC:

		// if a key is pressed sets the interrupt
		if ((m_int & INT_GINT) && (m_int & INT_KEY) && m_read_kb(0) != 0xff)
		{
			LOG("%s: Keyboard interrupt!\n", machine().describe_context());

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

			if (m_tim[0] == 128)    // on the rising edge of TIM0 bit 7
			{
				// set seconds int has occurred
				if (m_tmk & TSTA_SEC)
				{
					m_tsta |= TSTA_SEC;
					irq_change = true;
				}
			}

			if (m_tim[0] == 200)
			{
				m_tim[0] = 0;
				m_tim[1]++;

				if (m_tim[1] == 32) // on the rising edge of TIM1 bit 5
				{
					// set minutes int has occurred
					if (m_tmk & TSTA_MIN)
					{
						m_tsta |= TSTA_MIN;
						irq_change = true;
					}
				}

				if (m_tim[1] == 60)
				{
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

uint32_t upd65031_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

uint8_t upd65031_device::read(offs_t offset)
{
	uint8_t port = offset & 0xff;

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

				LOG("%s: entering snooze!\n", machine().describe_context());
			}

			uint8_t data = m_read_kb(offset>>8);

			LOG("%s: key r %02x %02x\n", machine().describe_context(), offset>>8, data);

			return data;
		}

		// read real time clock status
		case REG_TSTA:
			LOG("%s: tsta r %02x\n", machine().describe_context(), m_tsta);
			return m_tsta & 0x07;

		// read real time clock counters
		case REG_TIM0:
			LOG("%s: TIM0 r %02x\n", machine().describe_context(), m_tim[0]);
			return m_tim[0];
		case REG_TIM1:
			LOG("%s: TIM1 r %02x\n", machine().describe_context(), m_tim[1]);
			return m_tim[1];
		case REG_TIM2:
			LOG("%s: TIM2 r %02x\n", machine().describe_context(), m_tim[2]);
			return m_tim[2];
		case REG_TIM3:
			LOG("%s: TIM3 r %02x\n", machine().describe_context(), m_tim[3]);
			return m_tim[3];
		case REG_TIM4:
			LOG("%s: TIM4 r %02x\n", machine().describe_context(), m_tim[4]);
			return m_tim[4];

		// UART
		case REG_RXD:   // UART receive data register
			m_uit &= ~UIT_RDRF;
			update_uart_interrupt();
			if (m_rxc & RXC_ARTS)  // Auto RTS mode
				m_write_rts(1);
			return get_received_char();

		case REG_RXE:   // UART extended receive data
			return m_rxe;

		case REG_UIT:   // UART interrupt status
			return m_uit;

		default:
			logerror("%s: blink r %04x\n", machine().describe_context(), offset);
			return 0;
	}
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void upd65031_device::write(offs_t offset, uint8_t data)
{
	static const int uart_div[] = { 1 << 17, 1 << 15, 1 << 14, 1 << 13, 1 << 12, 1 << 10, 1 << 9, 1 << 8 };
	uint8_t port = offset & 0xff;

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
			LOG("%s: com w %02x\n", machine().describe_context(), data);

			// reset clock?
			if (data & COM_RESTIM)
				m_tim[0] = m_tim[1] = m_tim[2] = m_tim[3] = m_tim[4] = 0;

			if ((data & COM_SRUN) && !(data & COM_SBIT))
			{
				// constant tone used for keyclick and alarm
				m_speaker_timer->adjust(attotime::from_hz(SPEAKER_ALARM_FREQ), 0, attotime::from_hz(SPEAKER_ALARM_FREQ));
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
					m_write_spkr(m_txd_line);
				}

				m_speaker_timer->reset();
			}

			// bit 2 controls the lower 8kb of memory
			if (BIT(m_com^data, 2) && !m_out_mem_cb.isnull())
				m_out_mem_cb(0, m_sr[0], BIT(data, 2));

			m_write_vpp(BIT(data, 1));

			m_com = data;
			break;

		case REG_INT:   // interrupt control
			LOG("%s: int w %02x\n", machine().describe_context(), data);

			m_int = data;

			// refresh ints
			update_rtc_interrupt();
			interrupt_refresh();
			break;

		case REG_EPR:   // EPROM programming register
			LOG("%s: epr w %02x\n", machine().describe_context(), data);
			break;

		case REG_TACK:  // rtc interrupt acknowledge
			LOG("%s: tack w %02x\n", machine().describe_context(), data);

			// clear ints that have occurred
			m_tsta &= ~(data & 0x07);
			m_tack = data;

			// refresh ints
			update_rtc_interrupt();
			interrupt_refresh();
			break;

		case REG_TMK:   // write rtc interrupt mask
			LOG("%s: tmk w %02x\n", machine().describe_context(), data);

			m_tmk = data & 0x07;
			break;

		case REG_ACK:   // acknowledge ints
			LOG("%s: ack w %02x\n", machine().describe_context(), data);

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
		case REG_RXC:   // UART receive control
			LOG("%s: UART receive control %02x\n", machine().describe_context(), data);

			if ((m_rxc & RXC_BAUD) != (data & RXC_BAUD))
				set_rcv_rate(clock() / uart_div[data & RXC_BAUD]);

			if ((m_rxc ^ data) & RXC_SHTW)
				set_data_frame(1, 8, PARITY_NONE, (data & RXC_SHTW) ? STOP_BITS_1 : STOP_BITS_2);

			if (data & RXC_LOOP)
				logerror("%s: Unsupported UART Loopback mode\n", machine().describe_context());

			if (!(data & RXC_ARTS))
				m_write_rts((data & RXC_IRTS) ? 0 : 1);

			m_rxc = data;
			break;

		case REG_TXD:   // UART transmit data
			transmit_register_setup(data);
			m_uit &= ~UIT_TDRE;
			update_uart_interrupt();
			break;

		case REG_TXC:   // UART transmit control
			LOG("%s: UART transmit control %02x\n", machine().describe_context(), data);

			if ((m_txc & TXC_BAUD) != (data & TXC_BAUD))
				set_tra_rate(clock() / uart_div[data & TXC_BAUD]);

			if (!(data & TXC_ATX) && ((m_txc ^ data) & TXC_ITX))
				update_tx((data & TXC_ITX) ? 0 : 1);

			m_txc = data;
			break;

		case REG_UMK:   // UART interrupt mask
			LOG("%s: UART interrupt mask %02x\n", machine().describe_context(), data);

			m_umk = data;
			update_uart_interrupt();
			break;

		case REG_UAK:   // UART interrupt acknowledge
			LOG("%s: UART interrupt acknowledge %02x\n", machine().describe_context(), data);

			m_uit &= ~(data & m_umk & (UAK_CTS | UAK_DCD));
			update_uart_interrupt();
			break;

		default:
			logerror("%s: blink w %04x = %02x\n", machine().describe_context(), offset, data);
			break;
	}
}

void upd65031_device::tra_callback()
{
	update_tx(transmit_register_get_data_bit() ^ BIT(m_txc, 3));
}

void upd65031_device::tra_complete()
{
	m_uit |= UIT_TDRE;
	update_uart_interrupt();
}

void upd65031_device::rcv_complete()
{
	receive_register_extract();

	m_uit |= UIT_RDRF;

	if (m_rxc & RXC_ARTS)  // Auto RTS mode
		m_write_rts(0);

	// Frame error
	if (is_receive_framing_error())
		m_rxe |= RXE_FE;
	else
		m_rxe &= ~RXE_FE;

	update_uart_interrupt();
}

WRITE_LINE_MEMBER( upd65031_device::cts_w )
{
	if (state == BIT(m_uit, 0))
	{
		m_uit = (m_uit & ~UIT_CTS) | (state ? 0 : UIT_CTS);
		if (state != BIT(m_txc, 5))
		{
			m_uit |= UIT_CTSI;
			update_uart_interrupt();
		}
	}
}

WRITE_LINE_MEMBER( upd65031_device::dcd_w )
{
	if (state == BIT(m_uit, 1))
	{
		m_uit = (m_uit & ~UIT_DCD) | (state ? 0 : UIT_DCD);
		if (state != BIT(m_txc, 6))
		{
			m_uit |= UIT_DCDI;
			update_uart_interrupt();
		}
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
