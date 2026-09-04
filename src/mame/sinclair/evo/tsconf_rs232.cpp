// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    RS-232 enhanced interface for TS-Configuration
    Mainly used for ZiFi module. Current state implies minimal implementation for this purpose.

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


static constexpr u8 UART_DAT_DLL_REG = 0xf8;
static constexpr u8 UART_IER_DLM_REG = 0xf9;
static constexpr u8 UART_FCR_ISR_REG = 0xfa;
static constexpr u8 UART_LCR_REG     = 0xfb;
static constexpr u8 UART_MCR_REG     = 0xfc;
static constexpr u8 UART_LSR_REG     = 0xfd;
static constexpr u8 UART_MSR_REG     = 0xfe;
static constexpr u8 UART_SPR_REG     = 0xff;

static constexpr u8 ZF_DR_REG_LIM    = 0xbf;
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


// device type definition
DEFINE_DEVICE_TYPE(TSCONF_RS232, tsconf_rs232_device, "tsconf_rs232", "TS-Conf UART")

tsconf_rs232_device::tsconf_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TSCONF_RS232, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_out_txd_cb(*this)
	, m_out_rts_cb(*this)
{
}


void tsconf_rs232_device::rcv_callback()
{
	if (!is_receive_register_synchronized())
		return;

	receive_register_extract();
	const u8 data = get_received_char();
	if (m_select_zf)
		m_zf_rxbuff[inc_ptr(m_zf_rx_hd)] = data;
	else
		m_rs_rxbuff[inc_ptr(m_rs_rx_hd)] = data;
}

void tsconf_rs232_device::tra_complete()
{
	if (!is_transmit_register_empty())
		return;

	if (m_select_zf)
	{
		if (m_zf_tx_tl != m_zf_tx_hd)
			transmit_register_setup(m_zf_txbuff[inc_ptr(m_zf_tx_tl)]);
	}
	else
	{
		if (m_rs_tx_tl != m_rs_tx_hd)
			transmit_register_setup(m_rs_txbuff[inc_ptr(m_rs_tx_tl)]);
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
					inc_ptr(m_zf_rx_tl);
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
					inc_ptr(m_rs_rx_tl);
			}
		}
	}

	//LOGDIN("DR = %02x(%c)\n", data, data >= 0x20 ? data : '?');
	return data;
}

void tsconf_rs232_device::dr_w(u8 data)
{
	if (m_select_zf)
	{
		if (m_zf_api == 1)
		{
			if ((m_zf_tx_tl - 1 - m_zf_tx_hd) & 0xff)
			{
				m_zf_txbuff[inc_ptr(m_zf_tx_hd)] = data;
				tra_complete();
			}
		}
	}
	else
	{
		// enhanced RS-232
		if (m_zf_api)
		{
			if ((m_rs_tx_tl - 1 - m_rs_tx_hd) & 0xff)
			{
				m_rs_txbuff[inc_ptr(m_rs_tx_hd)] = data;
				tra_complete();
			}

			if (((m_rs_tx_tl - 1 - m_rs_tx_hd) & 0xff) == 0)
				m_regs.lsr &= ~0x60;
		}
	}
	LOGDOUT("DR: %02x(%c)\n", data, data >= 0x20 ? data : '?');
}

u8 tsconf_rs232_device::reg_r(offs_t offset)
{
	if (!m_zf_api)
		return 0xff;

	u8 data = 0xff;
	switch (offset & 0xf)
	{
		case 0: // ZIFR
			if (!machine().side_effects_disabled())
				m_select_zf = 1;
			data = std::min<u16>(zf_ifr_r(), ZF_DR_REG_LIM);
			//LOGDIN("ZIFR: %02x\n", data);
			break;
		case 1: // ZOFR
			{
				const u8 tmp = m_zf_tx_tl - 1 - m_zf_tx_hd;
				data = (tmp > ZF_DR_REG_LIM) ? ZF_DR_REG_LIM : tmp;
				if (!machine().side_effects_disabled())
					m_select_zf = 1;
			}
			LOGDOUT("ZOFR: %02x\n", data);
			break;
		case 2: // RIFR
			if (!machine().side_effects_disabled())
				m_select_zf = 0;
			data = std::min<u16>(rs_ifr_r(), ZF_DR_REG_LIM);
			LOGDIN("RIFR: %02x\n", data);
			break;
		case 3: // ROFR
			{
				const u8 tmp = m_rs_tx_tl - 1 - m_rs_tx_hd;
				data = (tmp > ZF_DR_REG_LIM) ? ZF_DR_REG_LIM : tmp;
				if (!machine().side_effects_disabled())
					m_select_zf = 0;
			}
			LOGDOUT("ROFR: %02x\n", data);
			break;
		case 4: // ISR
			data = m_zf_int_src;
			if (!machine().side_effects_disabled())
				m_zf_int_src = 0;   // clear flags
			break;
		case 5: // ZIBTR
			data = m_zf_ibtr;
			break;
		case 6: // ZITOR
			data = m_zf_itor;
			break;
		case 7: // ER
			data = m_zf_err;
			break;
		case 8: // RIBTR
			data = m_rs_ibtr;
			break;
		case 9: // RITOR
			data = m_rs_itor;
			break;
		default:
			LOG("Unknown register read: %02x\n", offset);
			break;
	}

	return data;
}

void tsconf_rs232_device::reg_w(offs_t offset, u8 data)
{
	switch (offset & 0xf)
	{
		case 4: // IMR
			m_zf_int_mask |= data;
			break;
		case 5: // ZIBTR
			m_zf_ibtr = data;
			break;
		case 6: // ZITOR
			m_zf_itor = data;
			break;
		case 7: // CR
			if ((data & ZF_SETAPI_MASK) == ZF_SETAPI)
			{
				// set API mode
				m_zf_api = data & ~ZF_SETAPI_MASK;
				if (m_zf_api > ZF_VER)
					m_zf_api = 0;
				update_serial(0);
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
						receive_register_reset();
						m_zf_rx_hd = m_zf_rx_tl = 0;
						LOGDIN("ZFCLR IN\n");
						machine().debug_break();
					}
					if (data & ZF_CLRFIFO_OUT)
					{
						transmit_register_reset();
						m_zf_tx_hd = m_zf_tx_tl = 0;
						LOGDOUT("ZFCLR OUT\n");
					}
				}
				else if ((data & RS_CLRFIFO_MASK) == RS_CLRFIFO)
				{
					if (data & RS_CLRFIFO_IN)
					{
						receive_register_reset();
						m_rs_rx_hd = m_rs_rx_tl = 0;
						LOGDIN("RSCLR IN\n");
					}
					if (data & RS_CLRFIFO_OUT)
					{
						transmit_register_reset();
						m_rs_tx_hd = m_rs_tx_tl = 0;
						LOGDOUT("RSCLR OUT\n");
						m_rs_rx_hd = m_rs_rx_tl = 0;
					}
				}
				else
				{
					LOGCMD("Unknown CMD %02x\n", data);
				}
			}
			break;
		case 8: // RIBTR
			m_rs_ibtr = data;
			break;
		case 9: // RITOR
			m_rs_itor = data;
			break;
		default:
			LOG("Unknown register write: %02x = %02x\n", offset, data);
			break;
	}
}

void tsconf_rs232_device::update_serial(int state)
{
	if (m_zf_api)
	{
		set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
		set_tra_rate(115200);
		set_rcv_rate(115200);

		receive_register_reset();
		transmit_register_reset();
		m_out_txd_cb(1);
		m_out_rts_cb(1);
	}
	else
	{
		set_tra_rate(0);
		set_rcv_rate(0);
	}
}

void tsconf_rs232_device::device_start()
{
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

	m_select_zf  =  0;
	m_zf_int_mask = 0;
	m_zf_int_src = 0;
	m_zf_api = 0;
	m_zf_err = 0;

	std::fill(std::begin(m_rs_txbuff), std::end(m_rs_txbuff), 0);
	std::fill(std::begin(m_rs_rxbuff), std::end(m_rs_rxbuff), 0);
	m_rs_tx_hd = m_rs_tx_tl = m_rs_rx_hd = m_rs_rx_tl = 0;
	m_rs_ibtr = 0x80;
	m_rs_itor = 1;
	m_rs_tmo_cnt = 0;

	std::fill(std::begin(m_zf_txbuff), std::end(m_zf_txbuff), 0);
	std::fill(std::begin(m_zf_rxbuff), std::end(m_zf_rxbuff), 0);
	m_zf_tx_hd = m_zf_tx_tl = m_zf_rx_hd = m_zf_rx_tl = 0;
	m_zf_ibtr = 0x80;
	m_zf_itor = 1;
	m_zf_tmo_cnt = 0;

	update_serial(0);
}
