// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    82919.cpp

    82919 module (RS232 interface)

    Main reference for this module is:
    HP 82919-90009, feb 85, HP82919A Integral PC Serial Interface -
    Component level service manual

*********************************************************************/

#include "emu.h"
#include "82919.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

hp82919_io_card_device::hp82919_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP82919_IO_CARD , tag , owner , clock)
	, device_hp_ipc_io_interface(mconfig, *this)
	, m_rs232_prim(*this , "rs232_prim")
	, m_rs232_sec(*this , "rs232_sec")
	, m_uart(*this , "uart")
	, m_loopback_en(*this, "loop")
{
}

hp82919_io_card_device::~hp82919_io_card_device()
{
}

uint8_t hp82919_io_card_device::read(offs_t addr)
{
	uint8_t res = 0;
	offs_t uart_addr = (addr ^ 0xc) & 0xf;

	if (BIT(addr , 4)) {
		res = m_uart->read(uart_addr);
	} else {
		// 2 is 82919 ID code
		res = 2;
		res |= (m_int_level << 4);
		if (m_int_pending) {
			BIT_SET(res , 6);
		}
		if (m_int_en) {
			BIT_SET(res , 7);
		}
		m_uart->write(uart_addr , res);
	}
	LOG("RD %04x=%02x\n" , addr , res);
	return res;
}

void hp82919_io_card_device::write(offs_t addr , uint8_t data)
{
	LOG("WR %04x=%02x\n" , addr , data);
	offs_t uart_addr = (addr ^ 0xc) & 0xf;
	m_uart->write(uart_addr , data);
	if (!BIT(addr , 4)) {
		m_int_level = (data >> 4) & 3;
		m_int_forced = BIT(data , 6);
		m_int_en = BIT(data , 7);
		update_irq();
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::uart_irq)
{
	m_uart_int = state;
	update_irq();
}

WRITE_LINE_MEMBER(hp82919_io_card_device::uart_a_tx)
{
	m_rs232_prim->write_txd(state);
	if (m_loopback) {
		m_uart->rx_b_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::uart_b_tx)
{
	m_rs232_sec->write_txd(state);
	if (m_loopback) {
		m_uart->rx_a_w(state);
	}
}

void hp82919_io_card_device::uart_output(uint8_t data)
{
	m_rs232_prim->write_dtr(BIT(data , 0));
	m_rs232_prim->write_rts(BIT(data , 1));
	// b2 is TxClock
	m_rs232_prim->write_spds(BIT(data , 3));
	m_rs232_sec->write_rts(BIT(data , 4));
	if (m_loopback) {
		m_uart->ip0_w(BIT(data , 0));
		m_uart->ip1_w(BIT(data , 1));
		m_uart->ip2_w(BIT(data , 2));
		m_uart->ip3_w(BIT(data , 3));
		m_uart->ip4_w(BIT(data , 4));
		m_uart->ip5_w(BIT(data , 4));
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::prim_rxd)
{
	if (!m_loopback) {
		m_uart->rx_a_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::prim_dcd)
{
	if (!m_loopback) {
		m_uart->ip3_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::prim_dsr)
{
	if (!m_loopback) {
		m_uart->ip1_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::prim_ri)
{
	if (!m_loopback) {
		m_uart->ip2_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::prim_cts)
{
	if (!m_loopback) {
		m_uart->ip0_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::sec_rxd)
{
	if (!m_loopback) {
		m_uart->rx_b_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::sec_dcd)
{
	if (!m_loopback) {
		m_uart->ip5_w(state);
	}
}

WRITE_LINE_MEMBER(hp82919_io_card_device::sec_cts)
{
	if (!m_loopback) {
		m_uart->ip4_w(state);
	}
}

void hp82919_io_card_device::install_read_write_handlers(address_space& space , uint32_t base_addr)
{
	offs_t end_addr = base_addr + 0xffffU;

	// Supervisor mode
	space.install_readwrite_handler(base_addr, end_addr, read8sm_delegate(*this, FUNC(hp82919_io_card_device::read)), write8sm_delegate(*this, FUNC(hp82919_io_card_device::write)), 0x00ff);
	// User mode
	space.install_readwrite_handler(base_addr + USER_SPACE_OFFSET, end_addr + USER_SPACE_OFFSET, read8sm_delegate(*this, FUNC(hp82919_io_card_device::read)), write8sm_delegate(*this, FUNC(hp82919_io_card_device::write)), 0x00ff);
}

void hp82919_io_card_device::device_start()
{
}

void hp82919_io_card_device::device_reset()
{
	m_loopback = m_loopback_en->read() != 0;
	m_int_level = 0;
	m_int_forced = false;
	m_int_en = false;
	m_uart_int = false;
	m_irq = true;
	update_irq();
}

void hp82919_io_card_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_rs232_prim, default_rs232_devices, nullptr);
	RS232_PORT(config, m_rs232_sec, default_rs232_devices, nullptr);

	MC68681(config , m_uart , 3.6864_MHz_XTAL);
	m_uart->irq_cb().set(FUNC(hp82919_io_card_device::uart_irq));
	m_uart->a_tx_cb().set(FUNC(hp82919_io_card_device::uart_a_tx));
	m_uart->b_tx_cb().set(FUNC(hp82919_io_card_device::uart_b_tx));
	m_uart->outport_cb().set(FUNC(hp82919_io_card_device::uart_output));

	m_rs232_prim->rxd_handler().set(FUNC(hp82919_io_card_device::prim_rxd));
	m_rs232_prim->dcd_handler().set(FUNC(hp82919_io_card_device::prim_dcd));
	m_rs232_prim->dsr_handler().set(FUNC(hp82919_io_card_device::prim_dsr));
	m_rs232_prim->ri_handler().set(FUNC(hp82919_io_card_device::prim_ri));
	m_rs232_prim->cts_handler().set(FUNC(hp82919_io_card_device::prim_cts));

	m_rs232_sec->rxd_handler().set(FUNC(hp82919_io_card_device::sec_rxd));
	m_rs232_sec->dcd_handler().set(FUNC(hp82919_io_card_device::sec_dcd));
	m_rs232_sec->cts_handler().set(FUNC(hp82919_io_card_device::sec_cts));
}

static INPUT_PORTS_START(hp82919_port)
	PORT_START("loop")
	PORT_CONFNAME(1 , 0 , "Test loopback")
	PORT_CONFSETTING(0 , DEF_STR(Off))
	PORT_CONFSETTING(1 , DEF_STR(On))
INPUT_PORTS_END

ioport_constructor hp82919_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp82919_port);
}

void hp82919_io_card_device::update_irq()
{
	m_int_pending = m_int_forced || m_uart_int;
	bool irq = m_int_en && m_int_pending;
	if (m_irq != irq) {
		m_irq = irq;
		LOG("IRQ %u=%d\n" , m_int_level , irq);
		irq_w(m_int_level , irq);
	}
}

// device type definition
DEFINE_DEVICE_TYPE(HP82919_IO_CARD, hp82919_io_card_device, "hp82919", "HP82919 card")
