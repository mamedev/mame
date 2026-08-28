// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

    Mitsubishi M37409M2 8-bit microcontroller (M740 core)

    Local bus (13-bit, mirrored so the vectors at 1FEC-1FFF appear at FFEC):
      0000-007F RAM, 00C0-00D7 access flags, 00E0-00FF I/O,
      0200-02BF dual-port RAM, 1000-1FFF ROM.
    System bus (8-bit address, from the master CPU):
      00-BF dual-port RAM, C0-D7 access flags, F5 port P1 (read), F6 port P0,
      F7 P0 direction, F8-FB IPC mode (write) / IPC error (read),
      FE collision detect, FF IPC semaphore.

    Interrupt vectors (M740 line n at FFFC-2n): UART1/2/3 receive, IPCM0,
    timer X, collision, UART1/2/3 transmit.

    TODO:
    - external baud clock (CLK pin)
    - collision detection between the two buses
    - the prescaler and timer are read back as their latches

***************************************************************************/

#include "emu.h"
#include "m37409.h"

DEFINE_DEVICE_TYPE(M37409, m37409_device, "m37409", "Mitsubishi M37409M2")
DEFINE_DEVICE_TYPE(M37409_UART, m37409_uart_device, "m37409_uart", "Mitsubishi M37409M2 UART")


//**************************************************************************
//  UART
//**************************************************************************

// mode register
static constexpr u8 MODE_PARITY_ENABLE = 0x01;
static constexpr u8 MODE_PARITY_EVEN   = 0x02;
static constexpr u8 MODE_8BIT          = 0x04;
static constexpr u8 MODE_2STOP         = 0x08;
static constexpr u8 MODE_DIV32         = 0x10;
static constexpr u8 MODE_EXT_CLOCK     = 0x20;
static constexpr u8 MODE_CTS_OUTPUT    = 0x80;

// control register
static constexpr u8 CTRL_TX_ENABLE     = 0x01;
static constexpr u8 CTRL_TX_IRQ_ENABLE = 0x02;
static constexpr u8 CTRL_RX_ENABLE     = 0x04;
static constexpr u8 CTRL_RX_IRQ_ENABLE = 0x08;
static constexpr u8 CTRL_CTS_FUNCTION  = 0x10;
static constexpr u8 CTRL_CTS_DATA      = 0x20;
static constexpr u8 CTRL_TX_INIT       = 0x40;
static constexpr u8 CTRL_ERROR_RESET   = 0x80;

// status register
static constexpr u8 STATUS_TX_READY    = 0x01;
static constexpr u8 STATUS_RX_READY    = 0x02;
static constexpr u8 STATUS_TX_EMPTY    = 0x04;
static constexpr u8 STATUS_PARITY_ERR  = 0x08;
static constexpr u8 STATUS_OVERRUN_ERR = 0x10;
static constexpr u8 STATUS_FRAMING_ERR = 0x20;
static constexpr u8 STATUS_CTS         = 0x80;

m37409_uart_device::m37409_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, M37409_UART, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_txd_handler(*this)
	, m_cts_handler(*this)
	, m_rx_irq_handler(*this)
	, m_tx_irq_handler(*this)
	, m_mode(0)
	, m_control(0)
	, m_status(0)
	, m_divider(0)
	, m_tx_buffer(0)
	, m_rx_buffer(0)
	, m_cts_in(1)
	, m_cts_out(1)
{
}

void m37409_uart_device::device_start()
{
	save_item(NAME(m_mode));
	save_item(NAME(m_control));
	save_item(NAME(m_status));
	save_item(NAME(m_divider));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_cts_in));
	save_item(NAME(m_cts_out));
}

void m37409_uart_device::device_reset()
{
	m_mode = 0;
	m_control = 0;
	m_status = STATUS_TX_READY | STATUS_TX_EMPTY;
	m_divider = 0;
	m_cts_out = 1;
	transmit_register_reset();
	receive_register_reset();
	update_frame();
	m_txd_handler(1);
	m_cts_handler(1);
}

void m37409_uart_device::update_frame()
{
	parity_t parity = PARITY_NONE;
	if (m_mode & MODE_PARITY_ENABLE)
		parity = (m_mode & MODE_PARITY_EVEN) ? PARITY_EVEN : PARITY_ODD;
	set_data_frame(1, (m_mode & MODE_8BIT) ? 8 : 7, parity, (m_mode & MODE_2STOP) ? STOP_BITS_2 : STOP_BITS_1);

	if (m_mode & MODE_EXT_CLOCK)
	{
		logerror("external baud rate clock not supported\n");
		set_rate(attotime::never);
	}
	else
	{
		const u32 divider = ((m_mode & MODE_DIV32) ? 512 : 32) * (m_divider + 1);
		set_rate(clock() / divider);
	}
}

void m37409_uart_device::update_cts()
{
	const int level = (m_mode & MODE_CTS_OUTPUT) ? BIT(m_control, 5) : 1;
	if (level != m_cts_out)
	{
		m_cts_out = level;
		m_cts_handler(level);
	}
}

void m37409_uart_device::cts_w(int state)
{
	m_cts_in = state;
	try_transmit();
}

// moves the buffered byte into the shift register when transmission is allowed
void m37409_uart_device::try_transmit()
{
	if (m_status & STATUS_TX_READY)
		return;
	if (!(m_control & CTRL_TX_ENABLE) || !(m_control & CTRL_TX_INIT))
		return;
	if ((m_control & CTRL_CTS_FUNCTION) && m_cts_in)
		return;
	if (!is_transmit_register_empty())
		return;

	transmit_register_setup(m_tx_buffer);
	m_status |= STATUS_TX_READY;
	m_status &= ~STATUS_TX_EMPTY;
	m_tx_irq_handler(1);
}

void m37409_uart_device::tra_callback()
{
	m_txd_handler(transmit_register_get_data_bit());
}

void m37409_uart_device::tra_complete()
{
	if (m_status & STATUS_TX_READY)
		m_status |= STATUS_TX_EMPTY;
	else
		try_transmit();
}

void m37409_uart_device::rcv_complete()
{
	receive_register_extract();
	if (!(m_control & CTRL_RX_ENABLE))
		return;

	if (m_status & STATUS_RX_READY)
		m_status |= STATUS_OVERRUN_ERR;
	m_rx_buffer = get_received_char();
	m_status |= STATUS_RX_READY;
	if (is_receive_framing_error())
		m_status |= STATUS_FRAMING_ERR;
	if (is_receive_parity_error())
		m_status |= STATUS_PARITY_ERR;
	m_rx_irq_handler(1);
}

u8 m37409_uart_device::read(offs_t offset)
{
	switch (offset & 3)
	{
	case 0:
		if (!machine().side_effects_disabled())
			m_status &= ~STATUS_RX_READY;
		return m_rx_buffer;
	case 1:
	{
		const int cts = (m_mode & MODE_CTS_OUTPUT) ? m_cts_out : m_cts_in;
		return (m_status & 0x3f) | (cts ? STATUS_CTS : 0);
	}
	case 2:
		return m_control;
	case 3:
		return m_divider;
	}
	return 0;
}

void m37409_uart_device::write(offs_t offset, u8 data)
{
	switch (offset & 3)
	{
	case 0:
		m_tx_buffer = data;
		m_status &= ~STATUS_TX_READY;
		try_transmit();
		break;
	case 1:
		m_mode = data;
		update_frame();
		update_cts();
		break;
	case 2:
		m_control = data & ~CTRL_ERROR_RESET;
		if (data & CTRL_ERROR_RESET)
			m_status &= ~(STATUS_PARITY_ERR | STATUS_OVERRUN_ERR | STATUS_FRAMING_ERR);
		if (!(data & CTRL_TX_INIT))
		{
			transmit_register_reset();
			m_status |= STATUS_TX_READY | STATUS_TX_EMPTY;
			m_txd_handler(1);
		}
		if (!(data & CTRL_RX_ENABLE))
		{
			receive_register_reset();
			m_status &= ~STATUS_RX_READY;
		}
		update_cts();
		try_transmit();
		break;
	case 3:
		m_divider = data;
		update_frame();
		break;
	}
}


//**************************************************************************
//  MCU
//**************************************************************************

m37409_device::m37409_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m740_device(mconfig, M37409, tag, owner, clock)
	, m_uart(*this, "uart%u", 1U)
	, m_rom(*this, DEVICE_SELF)
	, m_read_p(*this, 0xff)
	, m_write_p(*this)
	, m_txd_handler(*this)
	, m_cts_handler(*this)
	, m_timer(nullptr)
{
	m_program_config.m_internal_map = address_map_constructor(FUNC(m37409_device::map), this);
}

void m37409_device::device_add_mconfig(machine_config &config)
{
	for (int i = 0; i < 3; i++)
	{
		M37409_UART(config, m_uart[i], DERIVED_CLOCK(1, 1));
		m_uart[i]->txd_handler().set([this, i](int state) { m_txd_handler[i](state); });
		m_uart[i]->cts_handler().set([this, i](int state) { m_cts_handler[i](state); });
	}
	m_uart[0]->rx_irq_handler().set(FUNC(m37409_device::uart_rx_irq_w<0>));
	m_uart[1]->rx_irq_handler().set(FUNC(m37409_device::uart_rx_irq_w<1>));
	m_uart[2]->rx_irq_handler().set(FUNC(m37409_device::uart_rx_irq_w<2>));
	m_uart[0]->tx_irq_handler().set(FUNC(m37409_device::uart_tx_irq_w<0>));
	m_uart[1]->tx_irq_handler().set(FUNC(m37409_device::uart_tx_irq_w<1>));
	m_uart[2]->tx_irq_handler().set(FUNC(m37409_device::uart_tx_irq_w<2>));
}

void m37409_device::map(address_map &map)
{
	map(0x0000, 0x007f).ram();
	map(0x00c0, 0x00d7).r(FUNC(m37409_device::access_flag_r));
	map(0x00e0, 0x00e0).rw(FUNC(m37409_device::p1_r), FUNC(m37409_device::p1_w));
	map(0x00e1, 0x00e1).lr8(NAME([this]() { return m_p1_dir; })).w(FUNC(m37409_device::p1_dir_w));
	map(0x00e2, 0x00e2).lrw8(NAME([this]() { return m_dpram_dir; }), NAME([this](u8 data) { m_dpram_dir = data & 0x3f; }));
	map(0x00e4, 0x00e7).rw(m_uart[0], FUNC(m37409_uart_device::read), FUNC(m37409_uart_device::write));
	map(0x00e8, 0x00eb).rw(m_uart[1], FUNC(m37409_uart_device::read), FUNC(m37409_uart_device::write));
	map(0x00ec, 0x00ef).rw(m_uart[2], FUNC(m37409_uart_device::read), FUNC(m37409_uart_device::write));
	map(0x00f0, 0x00f3).r(FUNC(m37409_device::ipcm_r));
	map(0x00f4, 0x00f7).rw(FUNC(m37409_device::ipce_r), FUNC(m37409_device::ipce_w));
	map(0x00f9, 0x00f9).rw(FUNC(m37409_device::semaphore_r), FUNC(m37409_device::semaphore_w));
	map(0x00fa, 0x00fa).rw(FUNC(m37409_device::collision_r), FUNC(m37409_device::collision_w));
	map(0x00fb, 0x00fb).rw(FUNC(m37409_device::int_enable_r), FUNC(m37409_device::int_enable_w));
	map(0x00fc, 0x00fc).rw(FUNC(m37409_device::int_request_r), FUNC(m37409_device::int_request_w));
	map(0x00fd, 0x00fd).rw(FUNC(m37409_device::prescaler_r), FUNC(m37409_device::prescaler_w));
	map(0x00fe, 0x00fe).rw(FUNC(m37409_device::timer_r), FUNC(m37409_device::timer_w));
	map(0x00ff, 0x00ff).rw(FUNC(m37409_device::timer_control_r), FUNC(m37409_device::timer_control_w));
	map(0x0200, 0x02bf).rw(FUNC(m37409_device::dpram_r), FUNC(m37409_device::dpram_w));
	map(0x1000, 0x1fff).mirror(0xe000).rom().region(DEVICE_SELF, 0);
	map(0xffec, 0xffff).r(FUNC(m37409_device::vector_r));
}

void m37409_device::device_start()
{
	m740_device::device_start();

	m_timer = timer_alloc(FUNC(m37409_device::timer_tick), this);

	save_item(NAME(m_dpram));
	save_item(NAME(m_access_flag));
	save_item(NAME(m_dpram_dir));
	save_item(NAME(m_p0_latch));
	save_item(NAME(m_p0_dir));
	save_item(NAME(m_p1_latch));
	save_item(NAME(m_p1_dir));
	save_item(NAME(m_ipcm));
	save_item(NAME(m_ipce));
	save_item(NAME(m_semaphore));
	save_item(NAME(m_collision));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_request));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_timer_latch));
	save_item(NAME(m_timer_control));
	save_item(NAME(m_last_irqs));
}

void m37409_device::device_reset()
{
	m740_device::device_reset();

	std::fill(std::begin(m_access_flag), std::end(m_access_flag), 0);
	m_dpram_dir = 0;
	m_p0_latch = 0xff;
	m_p0_dir = 0;
	m_p1_latch = 0xff;
	m_p1_dir = 0;
	std::fill(std::begin(m_ipcm), std::end(m_ipcm), 0);
	std::fill(std::begin(m_ipce), std::end(m_ipce), 0);
	m_semaphore = 0;
	m_collision = 0;
	m_int_enable = 0;
	m_int_request = 0;
	m_prescaler = 0xff;
	m_timer_latch = 0x01;
	m_timer_control = 0;
	m_last_irqs = 0;
	restart_timer();
	update_p0();
	update_p1();
}


//-------------------------------------------------
//  dual-port RAM and access flags
//-------------------------------------------------

u8 m37409_device::access_flag_r(offs_t offset)
{
	return m_access_flag[offset];
}

u8 m37409_device::dpram_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && BIT(m_dpram_dir, offset >> 5))
		m_access_flag[offset >> 3] &= ~(1 << (offset & 7));
	return m_dpram[offset];
}

void m37409_device::dpram_w(offs_t offset, u8 data)
{
	m_access_flag[offset >> 3] |= 1 << (offset & 7);
	m_dpram[offset] = data;
}

u8 m37409_device::dpram_system_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && !BIT(m_dpram_dir, offset >> 5))
		m_access_flag[offset >> 3] &= ~(1 << (offset & 7));
	return m_dpram[offset];
}

void m37409_device::dpram_system_w(offs_t offset, u8 data)
{
	m_access_flag[offset >> 3] |= 1 << (offset & 7);
	m_dpram[offset] = data;
}


//-------------------------------------------------
//  ports: P0 belongs to the system bus, P1 to the local bus (readable from both)
//-------------------------------------------------

u8 m37409_device::p0_r()
{
	return (m_p0_latch & m_p0_dir) | (m_read_p[0]() & ~m_p0_dir);
}

void m37409_device::p0_w(u8 data)
{
	m_p0_latch = data;
	update_p0();
}

void m37409_device::p0_dir_w(u8 data)
{
	m_p0_dir = data;
	update_p0();
}

void m37409_device::update_p0()
{
	m_write_p[0](0, m_p0_latch | ~m_p0_dir, m_p0_dir);
}

u8 m37409_device::p1_r()
{
	return (m_p1_latch & m_p1_dir) | (m_read_p[1]() & ~m_p1_dir);
}

void m37409_device::p1_w(u8 data)
{
	m_p1_latch = data;
	update_p1();
}

void m37409_device::p1_dir_w(u8 data)
{
	m_p1_dir = data;
	update_p1();
}

void m37409_device::update_p1()
{
	m_write_p[1](0, m_p1_latch | ~m_p1_dir, m_p1_dir);
}


//-------------------------------------------------
//  IPC registers
//-------------------------------------------------

u8 m37409_device::ipcm_r(offs_t offset)
{
	return m_ipcm[offset];
}

u8 m37409_device::ipce_r(offs_t offset)
{
	return m_ipce[offset];
}

void m37409_device::ipce_w(offs_t offset, u8 data)
{
	m_ipce[offset] = data;
}

u8 m37409_device::semaphore_r()
{
	return m_semaphore;
}

// bits 0-2 select the flag (0-5 = BS0-BS5, 7 = RDY), bit 7 is the value written
void m37409_device::semaphore_w(u8 data)
{
	const int bit = data & 7;
	if (bit == 6)
		return;
	if (BIT(data, 7))
		m_semaphore |= 1 << bit;
	else
		m_semaphore &= ~(1 << bit);
}

u8 m37409_device::collision_r()
{
	return m_collision;
}

void m37409_device::collision_w(u8 data)
{
	// only the enable bit is writable; the request bit can be cleared
	m_collision = (m_collision & 0x80 & data) | (data & 0x40) | (m_collision & 0x3f);
	recalc_irqs();
}

u8 m37409_device::int_enable_r()
{
	return m_int_enable;
}

void m37409_device::int_enable_w(u8 data)
{
	m_int_enable = data;
	recalc_irqs();
}

u8 m37409_device::int_request_r()
{
	return m_int_request;
}

// request bits can only be cleared by software
void m37409_device::int_request_w(u8 data)
{
	m_int_request &= data;
	recalc_irqs();
}


//-------------------------------------------------
//  timer X: 8-bit prescaler and counter clocked by Xin/16
//-------------------------------------------------

u8 m37409_device::prescaler_r()
{
	return m_prescaler;
}

void m37409_device::prescaler_w(u8 data)
{
	m_prescaler = data;
	restart_timer();
}

u8 m37409_device::timer_r()
{
	return m_timer_latch;
}

void m37409_device::timer_w(u8 data)
{
	m_timer_latch = data;
	restart_timer();
}

u8 m37409_device::timer_control_r()
{
	return m_timer_control;
}

void m37409_device::timer_control_w(u8 data)
{
	const bool was_stopped = BIT(m_timer_control, 5);
	m_timer_control = data;
	if (was_stopped != BIT(data, 5))
		restart_timer();
	recalc_irqs();
}

void m37409_device::restart_timer()
{
	if (BIT(m_timer_control, 5))
		m_timer->adjust(attotime::never);
	else
	{
		const attotime period = clocks_to_attotime(16 * (m_prescaler + 1) * (m_timer_latch + 1));
		m_timer->adjust(period, 0, period);
	}
}

TIMER_CALLBACK_MEMBER(m37409_device::timer_tick)
{
	set_irq_request(IRQ_TIMER);
}


//-------------------------------------------------
//  interrupts
//-------------------------------------------------

template <int N> void m37409_device::uart_rx_irq_w(int state)
{
	if (state)
		set_irq_request(IRQ_UART1_RX >> N);
}

template <int N> void m37409_device::uart_tx_irq_w(int state)
{
	if (state)
		set_irq_request(IRQ_UART1_TX >> N);
}

void m37409_device::set_irq_request(u8 mask)
{
	m_int_request |= mask;
	recalc_irqs();
}

void m37409_device::recalc_irqs()
{
	const u8 active = m_int_request & m_int_enable;
	u16 irqs = 0;

	for (int i = 0; i < 3; i++)
	{
		if ((active & (IRQ_UART1_RX >> i)) && m_uart[i]->rx_irq_enabled())
			irqs |= 1 << i;
		if ((active & (IRQ_UART1_TX >> i)) && m_uart[i]->tx_irq_enabled())
			irqs |= 1 << (6 + i);
	}
	if ((active & IRQ_IPCM0) && BIT(m_timer_control, 7))
		irqs |= 1 << 3;
	if ((active & IRQ_TIMER) && BIT(m_timer_control, 6))
		irqs |= 1 << 4;
	if ((m_collision & 0xc0) == 0xc0)
		irqs |= 1 << 5;

	for (int i = 0; i < 9; i++)
		if (BIT(irqs, i) != BIT(m_last_irqs, i))
			m740_device::execute_set_input(M740_INT0_LINE + i, BIT(irqs, i) ? ASSERT_LINE : CLEAR_LINE);
	m_last_irqs = irqs;
}

// fetching a vector acknowledges the interrupt, clearing its request bit
u8 m37409_device::vector_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && (offset & 1))
	{
		static const u8 request_bits[10] = { IRQ_UART3_TX, IRQ_UART2_TX, IRQ_UART1_TX, 0, IRQ_TIMER, IRQ_IPCM0, IRQ_UART3_RX, IRQ_UART2_RX, IRQ_UART1_RX, 0 };
		const int vector = offset >> 1;
		if (vector == 3)
			m_collision &= ~0x80;
		else
			m_int_request &= ~request_bits[vector];
		recalc_irqs();
	}
	return m_rom[0xfec + offset];
}


//-------------------------------------------------
//  system bus
//-------------------------------------------------

u8 m37409_device::system_r(offs_t offset)
{
	offset &= 0xff;
	if (offset < 0xc0)
		return dpram_system_r(offset);
	if (offset < 0xd8)
		return m_access_flag[offset - 0xc0];

	switch (offset)
	{
	case 0xf5:
		return p1_r();
	case 0xf6:
		return p0_r();
	case 0xf7:
		return m_p0_dir;
	case 0xf8: case 0xf9: case 0xfa: case 0xfb:
	{
		const u8 data = m_ipce[offset & 3];
		if (!machine().side_effects_disabled())
		{
			m_ipce[offset & 3] = 0;
			if (offset == 0xf8)
				set_irq_request(IRQ_IPCM0);
		}
		return data;
	}
	case 0xfe:
		return m_collision;
	case 0xff:
		return m_semaphore;
	default:
		logerror("%s: unmapped system bus read %02X\n", machine().describe_context(), offset);
		return 0xff;
	}
}

void m37409_device::system_w(offs_t offset, u8 data)
{
	offset &= 0xff;
	if (offset < 0xc0)
	{
		dpram_system_w(offset, data);
		return;
	}

	switch (offset)
	{
	case 0xf6:
		p0_w(data);
		break;
	case 0xf7:
		p0_dir_w(data);
		break;
	case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		m_ipcm[offset & 3] = data;
		if (offset == 0xf8)
		{
			m_semaphore &= ~0x80;
			set_irq_request(IRQ_IPCM0);
		}
		break;
	case 0xff:
		semaphore_w(data);
		break;
	default:
		logerror("%s: unmapped system bus write %02X = %02X\n", machine().describe_context(), offset, data);
		break;
	}
}
