// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    RS-232 enhanced interface for TS-Configuration
    ZiFi (ZXWifi) module support, Kondratyev 16550-compatible registers

Refs:
    https://github.com/tslabs/zx-evo/blob/master/pentevo/docs/ZiFi/zifi.md

**********************************************************************/

#include "emu.h"
#include "tsconf_rs232.h"


#define LOG_CMD     (1U << 1)
#define LOG_DAT_OUT (1U << 2)
#define LOG_DAT_IN  (1U << 3)

//#define VERBOSE ( LOG_CMD | LOG_DAT_OUT | LOG_DAT_IN | LOG_GENERAL )
#include "logmacro.h"

#define LOGCMD(...)     LOGMASKED(LOG_CMD,     __VA_ARGS__)
#define LOGDOUT(...)    LOGMASKED(LOG_DAT_OUT, __VA_ARGS__)
#define LOGDIN(...)     LOGMASKED(LOG_DAT_IN,  __VA_ARGS__)


// 16550 (Kondratyev) registers
static constexpr u8 UART_DAT_DLL_REG = 0xf8;
static constexpr u8 UART_IER_DLM_REG = 0xf9;
static constexpr u8 UART_FCR_ISR_REG = 0xfa;
static constexpr u8 UART_LCR_REG     = 0xfb;
static constexpr u8 UART_MCR_REG     = 0xfc;
static constexpr u8 UART_LSR_REG     = 0xfd;
static constexpr u8 UART_MSR_REG     = 0xfe;
static constexpr u8 UART_SPR_REG     = 0xff;

// ZiFi registers
static constexpr u8 ZF_DR_REG_LIM    = 0xbf;
static constexpr u8 ZF_REG_LIM       = 0xcf;
static constexpr u8 ZF_IFR_REG       = 0xc0;
static constexpr u8 ZF_OFR_REG       = 0xc1;
static constexpr u8 RS_IFR_REG       = 0xc2;
static constexpr u8 RS_OFR_REG       = 0xc3;
static constexpr u8 ZF_IMR_REG       = 0xc4;
static constexpr u8 ZF_IBTR_REG      = 0xc5;
static constexpr u8 ZF_ITOR_REG      = 0xc6;
static constexpr u8 ZF_CR_REG        = 0xc7;
static constexpr u8 RS_IBTR_REG      = 0xc8;
static constexpr u8 RS_ITOR_REG      = 0xc9;

static constexpr u8 ZF_CLRFIFO       = 0b00000000;
static constexpr u8 ZF_CLRFIFO_MASK  = 0b11111100;
static constexpr u8 ZF_CLRFIFO_IN    = 0b00000001;
static constexpr u8 ZF_CLRFIFO_OUT   = 0b00000010;
static constexpr u8 RS_CLRFIFO       = 0b00000100;
static constexpr u8 RS_CLRFIFO_MASK  = 0b11111100;
static constexpr u8 RS_CLRFIFO_IN    = 0b00000001;
static constexpr u8 RS_CLRFIFO_OUT   = 0b00000010;

static constexpr u8 ZF_SETAPI        = 0b11110000;
static constexpr u8 ZF_SETAPI_MASK   = 0b11111000;

static constexpr u8 ZF_GETVER        = 0b11111111;
static constexpr u8 ZF_GETVER_MASK   = 0b11111111;

static constexpr u8 ZF_OK_RES        = 0x00;

static constexpr u8 ZF_VER           = 0x01;


// Interrupt Mask / Source Register bits
static constexpr u8 ZF_IMR_IBT       = 0b00000001;
static constexpr u8 ZF_IMR_ITO       = 0b00000010;
static constexpr u8 RS_IMR_IBT       = 0b00000100;
static constexpr u8 RS_IMR_ITO       = 0b00001000;

// Line Status Register bits
static constexpr u8 LSR_DR           = 0b00000001;
static constexpr u8 LSR_OE           = 0b00000010;
static constexpr u8 LSR_THRE         = 0b00100000;
static constexpr u8 LSR_TEMT         = 0b01000000;

void tsconf_rs232_device::rcv_complete()
{
	receive_register_extract();
	const u8 data = get_received_char();
	if (m_select_zf)
	{
		if (((m_zf_rx_tl - 1 - m_zf_rx_hd) & 0x1ff) != 0)
			m_zf_rxbuff[inc_rx_ptr(m_zf_rx_hd)] = data;
		m_zf_tmo_cnt = 0;
	}
	else
	{
		if (((m_rs_rx_tl - 1 - m_rs_rx_hd) & 0x1ff) != 0)
		{
			m_rs_rxbuff[inc_rx_ptr(m_rs_rx_hd)] = data;
			m_regs.lsr |= LSR_DR;
		}
		else
			m_regs.lsr |= LSR_DR | LSR_OE;
		m_rs_tmo_cnt = 0;
	}
	LOGDIN("RX %02x(%c)\n", data, data >= 0x20 ? data : '?');
	check_int();
}
// device type definition
DEFINE_DEVICE_TYPE(TSCONF_RS232, tsconf_rs232_device, "tsconf_rs232", "TS-Conf UART")

tsconf_rs232_device::tsconf_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TSCONF_RS232, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_out_txd_cb(*this)
	, m_out_rts_cb(*this)
	, m_out_int_cb(*this)
{
}

void tsconf_rs232_device::tra_complete()
{
	if (!is_transmit_register_empty())
		return;

	if (m_select_zf)
	{
		if (m_zf_tx_tl != m_zf_tx_hd)
			transmit_register_setup(m_zf_txbuff[inc_tx_ptr(m_zf_tx_tl)]);
	}
	else
	{
		if (m_rs_tx_tl != m_rs_tx_hd)
		{
			transmit_register_setup(m_rs_txbuff[inc_tx_ptr(m_rs_tx_tl)]);
			m_regs.lsr |= LSR_THRE;
		}
		else
			m_regs.lsr |= LSR_THRE | LSR_TEMT;
	}
}

void tsconf_rs232_device::tra_callback()
{
	const int txd = transmit_register_get_data_bit();
	m_out_txd_cb(txd);
}

u8 tsconf_rs232_device::dr_r()
{
	u8 data = 0xff;
	if (m_select_zf)
	{
		// ZiFi data read
		if (m_zf_api == 1)
		{
			if (m_zf_rx_tl != m_zf_rx_hd)
			{
				data = m_zf_rxbuff[m_zf_rx_tl];
				if (!machine().side_effects_disabled())
					inc_rx_ptr(m_zf_rx_tl);
			}
		}
	}
	else
	{
		// Enhanced RS-232 data read
		if (m_zf_api)
		{
			if (m_rs_rx_tl != m_rs_rx_hd)
			{
				data = m_rs_rxbuff[m_rs_rx_tl];
				if (!machine().side_effects_disabled())
					inc_rx_ptr(m_rs_rx_tl);
			}
		}
	}

	//LOGDIN("DR = %02x(%c)\n", data, data >= 0x20 ? data : '?');
	return data;
}

void tsconf_rs232_device::zf_tx_w(u8 data)
{
	if (u8(m_zf_tx_tl - 1 - m_zf_tx_hd))
	{
		m_zf_txbuff[inc_tx_ptr(m_zf_tx_hd)] = data;
		tra_complete();
	}
}

void tsconf_rs232_device::rs_tx_w(u8 data)
{
	if (u8(m_rs_tx_tl - 1 - m_rs_tx_hd))
	{
		m_rs_txbuff[inc_tx_ptr(m_rs_tx_hd)] = data;
		tra_complete();
	}
	if (!u8(m_rs_tx_tl - 1 - m_rs_tx_hd))
		m_regs.lsr &= ~(LSR_THRE | LSR_TEMT);
}

void tsconf_rs232_device::dr_w(u8 data)
{
	if (m_select_zf)
	{
		if (m_zf_api == 1)
			zf_tx_w(data);
	}
	else
	{
		// enhanced RS-232
		if (m_zf_api)
			rs_tx_w(data);
	}
	LOGDOUT("DR: %02x(%c)\n", data, data >= 0x20 ? data : '?');
}

u8 tsconf_rs232_device::reg_r(offs_t offset)
{
	// Data Registers
	if (offset <= ZF_DR_REG_LIM)
		return dr_r();

	u8 data = 0xff;
	if (offset <= ZF_REG_LIM)
	{
		// Other ZiFi registers
		if (m_zf_api)
		{
			switch (offset)
			{
			case ZF_IFR_REG: // ZIFR
				data = std::min<u16>(zf_ifr_r(), ZF_DR_REG_LIM);
				//LOGDIN("ZIFR: %02x\n", data);
				[[fallthrough]];
			case ZF_OFR_REG: // ZOFR
				if (offset == ZF_OFR_REG)
				{
					const u8 tmp = m_zf_tx_tl - 1 - m_zf_tx_hd;
					data = (tmp > ZF_DR_REG_LIM) ? ZF_DR_REG_LIM : tmp;
					LOGDOUT("ZOFR: %02x\n", data);
				}
				if (!machine().side_effects_disabled())
				{
					m_select_zf = 1;
					update_serial();
				}
				break;
			case RS_IFR_REG: // RIFR
				data = std::min<u16>(rs_ifr_r(), ZF_DR_REG_LIM);
				LOGDIN("RIFR: %02x\n", data);
				[[fallthrough]];
			case RS_OFR_REG: // ROFR
				if (offset == RS_OFR_REG)
				{
					const u8 tmp = m_rs_tx_tl - 1 - m_rs_tx_hd;
					data = (tmp > ZF_DR_REG_LIM) ? ZF_DR_REG_LIM : tmp;
					LOGDOUT("ROFR: %02x\n", data);
				}
				if (!machine().side_effects_disabled())
				{
					m_select_zf = 0;
					update_serial();
				}
				break;
			case ZF_IMR_REG: // ISR
				data = m_zf_int_src;
				if (!machine().side_effects_disabled())
					m_zf_int_src = 0;   // clear flags
				break;
			case ZF_IBTR_REG: // ZIBTR
				data = m_zf_ibtr;
				break;
			case ZF_ITOR_REG: // ZITOR
				data = m_zf_itor;
				break;
			case ZF_CR_REG: // ER
				data = m_zf_err;
				break;
			case RS_IBTR_REG: // RIBTR
				data = m_rs_ibtr;
				break;
			case RS_ITOR_REG: // RITOR
				data = m_rs_itor;
				break;
			default:
				LOG("Unknown register read: %02x\n", offset);
				break;
			}
		}
	}
	else if (offset >= UART_DAT_DLL_REG)
	{
		// Kondratyev 16550 registers
		data = 0;
		switch (offset)
		{
		case UART_DAT_DLL_REG:
			if (m_regs.lcr & 0x80)
				data = m_regs.dll;
			else
			{
				if (m_rs_rx_tl != m_rs_rx_hd)
					data = m_rs_rxbuff[m_rs_rx_tl];
				if (!machine().side_effects_disabled())
				{
					if (m_rs_rx_tl != m_rs_rx_hd)
						inc_rx_ptr(m_rs_rx_tl);
					if (m_rs_rx_tl == m_rs_rx_hd)
						m_regs.lsr &= ~LSR_DR;
				}
			}
			break;
		case UART_IER_DLM_REG:
			data = (m_regs.lcr & 0x80) ? m_regs.dlm : m_regs.ier;
			break;
		case UART_FCR_ISR_REG:
			data = m_regs.isr;
			break;
		case UART_LCR_REG:
			data = m_regs.lcr;
			break;
		case UART_MCR_REG:
			data = m_regs.mcr;
			break;
		case UART_LSR_REG:
			data = m_regs.lsr;
			break;
		case UART_MSR_REG:
			//DSR=CD=1
			data = m_regs.msr;
			if (!machine().side_effects_disabled())
				m_regs.msr &= 0xf0;   //clear flags
			break;
		case UART_SPR_REG:
			data = m_regs.scr;
			break;
		}
	}
	else
	{
		// 0xd0..0xf7 not used
		data = 0;
	}

	return data;
}

void tsconf_rs232_device::reg_w(offs_t offset, u8 data)
{
	// Data Registers
	if (offset <= ZF_DR_REG_LIM)
	{
		dr_w(data);
		return;
	}

	switch (offset)
	{
	// ZiFi registers
	case ZF_IMR_REG: // IMR
		m_zf_int_mask |= data;
		check_int();
		break;
	case ZF_IBTR_REG: // ZIBTR
		m_zf_ibtr = data;
		check_int();
		break;
	case ZF_ITOR_REG: // ZITOR
		m_zf_itor = data;
		check_int();
		break;
	case ZF_CR_REG: // CR
		if ((data & ZF_SETAPI_MASK) == ZF_SETAPI)
		{
			// set API mode
			m_zf_api = data & ~ZF_SETAPI_MASK;
			if (m_zf_api > ZF_VER)
				m_zf_api = 0;
			m_zf_err = ZF_OK_RES;
			LOGCMD("API: %d\n", m_zf_api);
		}
		else if (m_zf_api)
		{
			// get API version
			if ((data & ZF_GETVER_MASK) == ZF_GETVER)
			{
				m_zf_err = ZF_VER;
				LOGCMD("VER\n");
			}
			else if ((data & ZF_CLRFIFO_MASK) == ZF_CLRFIFO)
			{
				if (data & ZF_CLRFIFO_IN)
				{
					m_zf_rx_hd = m_zf_rx_tl = 0;
					LOGDIN("ZFCLR IN\n");
				}
				if (data & ZF_CLRFIFO_OUT)
				{
					m_zf_tx_hd = m_zf_tx_tl = 0;
					LOGDOUT("ZFCLR OUT\n");
				}
			}
			else if ((data & RS_CLRFIFO_MASK) == RS_CLRFIFO)
			{
				if (data & RS_CLRFIFO_IN)
				{
					m_rs_rx_hd = m_rs_rx_tl = 0;
					LOGDIN("RSCLR IN\n");
				}
				if (data & RS_CLRFIFO_OUT)
				{
					m_rs_tx_hd = m_rs_tx_tl = 0;
					LOGDOUT("RSCLR OUT\n");
				}
			}
			else
			{
				LOGCMD("Unknown CMD %02x\n", data);
			}
		}
		break;
	case RS_IBTR_REG: // RIBTR
		m_rs_ibtr = data;
		check_int();
		break;
	case RS_ITOR_REG: // RITOR
		m_rs_itor = data;
		check_int();
		break;

	// Kondratyev 16550 registers
	case UART_DAT_DLL_REG:
		if (m_regs.lcr & 0x80)
		{
			m_regs.dll = data;
			if (!m_select_zf)
				update_serial();
		}
		else
			rs_tx_w(data);
		break;
	case UART_IER_DLM_REG:
		if (m_regs.lcr & 0x80)
		{
			m_regs.dlm = data;
			if (!m_select_zf)
				update_serial();
		}
		else
		{
			//bit 7-4 not used and set to '0'
			m_regs.ier = data & 0x0f;
		}
		break;
	case UART_FCR_ISR_REG:
		if (data & 1)
		{
			//receive FIFO reset
			if (data & (1 << 1))
			{
				m_rs_rx_hd = m_rs_rx_tl = 0;
				m_regs.lsr &= ~(LSR_DR | LSR_OE);   //set empty FIFO flag and clear overrun flag
			}

			//transmit FIFO reset
			if (data & (1 << 2))
			{
				m_rs_tx_hd = m_rs_tx_tl = 0;
				m_regs.lsr |= LSR_THRE | LSR_TEMT;  //set fifo is empty flag
			}
		}
		//m_regs.fcr = data & 0xc9;
		break;
	case UART_LCR_REG:
		m_regs.lcr = data;
		set_format();
		break;
	case UART_MCR_REG:
		//bit 7-5 not used and set to '0'
		m_regs.mcr = data & 0x1f;
		m_out_rts_cb(BIT(data, 1) ? 0 : 1);
		break;
	case UART_LSR_REG:
	case UART_MSR_REG:
		//not writable
		break;
	case UART_SPR_REG:
		m_regs.scr = data;
		break;

	default:
		LOG("Unknown register write: %02x = %02x\n", offset, data);
		break;
	}
}

void tsconf_rs232_device::check_int()
{
	// ZiFi Input Buffer over threshold interrupt
	if ((m_zf_int_mask & ZF_IMR_IBT) && (zf_ifr_r() >= m_zf_ibtr))
	{
		m_zf_int_mask &= ~ZF_IMR_IBT;
		m_zf_int_src |= ZF_IMR_IBT;
	}

	// RS-232 Input Buffer over threshold interrupt
	if ((m_zf_int_mask & RS_IMR_IBT) && (rs_ifr_r() >= m_rs_ibtr))
	{
		m_zf_int_mask &= ~RS_IMR_IBT;
		m_zf_int_src |= RS_IMR_IBT;
	}

	// ZiFi Input Buffer timeout interrupt
	if ((m_zf_int_mask & ZF_IMR_ITO) && (m_zf_tmo_cnt >= m_zf_itor) && zf_ifr_r())
	{
		m_zf_int_mask &= ~ZF_IMR_ITO;
		m_zf_int_src |= ZF_IMR_ITO;
	}

	// RS-232 Input Buffer timeout interrupt
	if ((m_zf_int_mask & RS_IMR_ITO) && (m_rs_tmo_cnt >= m_rs_itor) && rs_ifr_r())
	{
		m_zf_int_mask &= ~RS_IMR_ITO;
		m_zf_int_src |= RS_IMR_ITO;
	}

	if (m_zf_int_src)
		m_out_int_cb(ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(tsconf_rs232_device::systick)
{
	if (m_rs_tmo_cnt < 255)
		m_rs_tmo_cnt++;
	if (m_zf_tmo_cnt < 255)
		m_zf_tmo_cnt++;

	check_int();
}

u32 tsconf_rs232_device::rs_baud() const
{
	const u16 divisor = (u16(m_regs.dlm) << 8) | m_regs.dll;
	if (!divisor)
		return 230400;   // divisor 0 selects 230400 baud

	const unsigned fcpu16 = clock() / 16;
	if (m_regs.dlm & 0x80)
	{
		//AVR mode - direct load UBRR
		const u16 ubrr = ((m_regs.dlm & 0x7f) << 8) | m_regs.dll;
		return fcpu16 / (ubrr + 1);
	}

	//default mode - like 16550
	const unsigned i = 115200 / divisor;
	return fcpu16 / (fcpu16 / i);
}

void tsconf_rs232_device::set_format()
{
	//set word length and stop bits
	m_word_len = 5 + (m_regs.lcr & 0b11);
	m_stop_bits = (m_regs.lcr & 0b100) ? STOP_BITS_2 : STOP_BITS_1;

	//set parity (only "No parity","Odd","Even" supported)
	switch (m_regs.lcr & 0b00111000)
	{
	case 0b00001000:
		m_parity = PARITY_ODD;
		break;
	case 0b00011000:
		m_parity = PARITY_EVEN;
		break;
	default:
		m_parity = PARITY_NONE;
		break;
	}

	if (!m_select_zf)
		update_serial();
}

void tsconf_rs232_device::update_serial()
{
	// set_data_frame() skips reconfiguration when the frame is unchanged, but
	// set_*_rate() resets the shift registers and stops the clocks, so only apply
	// when the rate actually changes to not murder reception mid-byte
	u32 baud;
	if (m_select_zf)
	{
		// UART0 (ZiFi): 115200, 8 data, 2 stop bits (transmit)
		set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
		baud = 115200;
	}
	else
	{
		// UART1 (RS-232): rate from divisor latch, format from LCR
		set_data_frame(1, m_word_len, parity_t(m_parity), stop_bits_t(m_stop_bits));
		baud = rs_baud();
	}

	if (baud != m_baud_applied)
	{
		m_baud_applied = baud;
		set_tra_rate(baud);
		set_rcv_rate(baud);
	}
}

void tsconf_rs232_device::cts_w(int state)
{
	if (state)
	{
		//CTS changed - set flag
		if (m_regs.msr & 0x10)
			m_regs.msr |= 0x01;

		m_regs.msr &= ~0x10;
	}
	else
	{
		//CTS changed - set flag
		if (!(m_regs.msr & 0x10))
			m_regs.msr |= 0x01;

		m_regs.msr |= 0x10;
	}
}

void tsconf_rs232_device::device_start()
{
	m_systick_timer = timer_alloc(FUNC(tsconf_rs232_device::systick), this);

	save_item(NAME(m_regs.dll));
	save_item(NAME(m_regs.dlm));
	save_item(NAME(m_regs.ier));
	save_item(NAME(m_regs.isr));
	//save_item(NAME(m_regs.fcr));
	save_item(NAME(m_regs.lcr));
	save_item(NAME(m_regs.mcr));
	save_item(NAME(m_regs.lsr));
	save_item(NAME(m_regs.msr));
	save_item(NAME(m_regs.scr));

	save_item(NAME(m_word_len));
	save_item(NAME(m_parity));
	save_item(NAME(m_stop_bits));

	save_item(NAME(m_select_zf));
	save_item(NAME(m_zf_int_mask));
	save_item(NAME(m_zf_int_src));
	save_item(NAME(m_zf_api));
	save_item(NAME(m_zf_err));

	save_item(NAME(m_rs_txbuff));
	save_item(NAME(m_rs_rxbuff));
	save_item(NAME(m_rs_tx_hd));
	save_item(NAME(m_rs_tx_tl));
	save_item(NAME(m_rs_rx_hd));
	save_item(NAME(m_rs_rx_tl));
	save_item(NAME(m_rs_ibtr));
	save_item(NAME(m_rs_itor));
	save_item(NAME(m_rs_tmo_cnt));

	save_item(NAME(m_zf_txbuff));
	save_item(NAME(m_zf_rxbuff));
	save_item(NAME(m_zf_tx_hd));
	save_item(NAME(m_zf_tx_tl));
	save_item(NAME(m_zf_rx_hd));
	save_item(NAME(m_zf_rx_tl));
	save_item(NAME(m_zf_ibtr));
	save_item(NAME(m_zf_itor));
	save_item(NAME(m_zf_tmo_cnt));
}

void tsconf_rs232_device::device_reset()
{
	m_regs.dlm = 0;
	m_regs.dll = 0x01;
	m_regs.ier = 0;
	//m_regs.fcr = 0x01; //FIFO always enable
	m_regs.isr = 0x01;
	m_regs.lcr = 0;
	m_regs.mcr = 0;
	m_regs.lsr = 0x60;
	m_regs.msr = 0xa0; //DSR=CD=1, RI=0
	m_regs.scr = 0xff;

	// frame format follows LCR only after LCR write, power-on state is 8N2
	m_word_len = 8;
	m_parity = PARITY_NONE;
	m_stop_bits = STOP_BITS_2;

	m_select_zf  =  0;
	m_zf_int_mask = 0;
	m_zf_int_src = 0;
	m_zf_api = 0;
	m_zf_err = 0;

	std::fill(std::begin(m_rs_txbuff), std::end(m_rs_txbuff), 0);
	std::fill(std::begin(m_rs_rxbuff), std::end(m_rs_rxbuff), 0);
	m_rs_tx_hd = m_rs_tx_tl = 0;
	m_rs_rx_hd = m_rs_rx_tl = 0;
	m_rs_ibtr = 0x80;
	m_rs_itor = 1;
	m_rs_tmo_cnt = 0;

	std::fill(std::begin(m_zf_txbuff), std::end(m_zf_txbuff), 0);
	std::fill(std::begin(m_zf_rxbuff), std::end(m_zf_rxbuff), 0);
	m_zf_tx_hd = m_zf_tx_tl = 0;
	m_zf_rx_hd = m_zf_rx_tl = 0;
	m_zf_ibtr = 0x80;
	m_zf_itor = 1;
	m_zf_tmo_cnt = 0;

	m_baud_applied = 0;
	update_serial();
	m_out_txd_cb(1);   // idle line
	m_out_rts_cb(0);   // RTS active, allow the peer to transmit

	const attotime period = attotime::from_ticks(11059 + 1, clock());   // TIMER1 CTC: OCR1A = 11059 @ F_CPU -> 1 ms
	m_systick_timer->adjust(period, 0, period);
}
