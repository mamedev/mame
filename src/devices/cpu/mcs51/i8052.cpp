// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#include "emu.h"
#include "i8052.h"
#include "mcs51dasm.h"

DEFINE_DEVICE_TYPE(I8032, i8032_device, "i8032", "Intel 8032")
DEFINE_DEVICE_TYPE(I8052, i8052_device, "i8052", "Intel 8052")
DEFINE_DEVICE_TYPE(I8752, i8752_device, "i8752", "Intel 8752")

i8052_device::i8052_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width)
	: mcs51_cpu_device(mconfig, type, tag, owner, clock, program_width, 8)
{
	m_num_interrupts = 6;
}

i8052_device::i8052_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, I8052, tag, owner, clock, 13)
{
}

i8032_device::i8032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, I8032, tag, owner, clock, 0)
{
}

i8752_device::i8752_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, I8752, tag, owner, clock, 13)
{
}

void i8052_device::update_timer_t2(int cycles)
{
	/* Update Timer 2 */
	if (BIT(m_t2con, T2CON_TR2))
	{
		int mode = ((BIT(m_t2con, T2CON_TCLK) | BIT(m_t2con, T2CON_RCLK)) << 1) | BIT(m_t2con, T2CON_CP);
		int delta = BIT(m_t2con, T2CON_CT2) ? m_t2_cnt : (mode & 2) ? cycles * (12/2) : cycles;

		u32 count = m_t2 + delta;
		m_t2_cnt = 0;

		switch (mode)
		{
			case 0: /* 16 Bit Auto Reload */
				if (count & 0xffff0000)
				{
					set_tf2(1);
					count += m_rcap2;
				}
				else if (BIT(m_t2con, T2CON_EXEN2) && m_t2ex_cnt > 0)
				{
					count += m_rcap2;
					m_t2ex_cnt = 0;
				}
				m_t2 = count;
				break;
			case 1: /* 16 Bit Capture */
				if (count & 0xffff0000)
					set_tf2(1);
				m_t2 = count;

				if (BIT(m_t2con, T2CON_EXEN2) && m_t2ex_cnt > 0)
				{
					m_rcap2 = m_t2;
					m_t2ex_cnt = 0;
				}
				break;
			case 2:
			case 3: /* Baud rate */
				if (count & 0xffff0000)
				{
					count += m_rcap2;
					transmit_receive(2);
				}
				m_t2 = count;
				break;
		}
	}
}

void i8052_device::handle_8bit_uart_clock(int source)
{
	if (source == 1)
	{
		m_uart.tx_clk += (BIT(m_t2con, T2CON_TCLK) ? 0 : !m_uart.smod_div);
		m_uart.rx_clk += (BIT(m_t2con, T2CON_RCLK) ? 0 : !m_uart.smod_div);
	}
	if (source == 2)
	{
		m_uart.tx_clk += (BIT(m_t2con, T2CON_TCLK) ? 1 : 0);
		m_uart.rx_clk += (BIT(m_t2con, T2CON_RCLK) ? 1 : 0);
	}
}

void i8052_device::irqs_complete_and_mask(u8 &ints, u8 int_mask)
{
	ints |= ((BIT(m_t2con, T2CON_TF2) | BIT(m_t2con, T2CON_EXF2)) << 5);
	ints &= int_mask;
}

void i8052_device::handle_irq(int irqline, int state, u32 new_state, u32 tr_state)
{
	switch (irqline)
	{
		case MCS51_T2_LINE:
			if (BIT(tr_state, MCS51_T2_LINE) && BIT(m_tcon, TCON_TR1))
				m_t2_cnt++;
			break;

		case MCS51_T2EX_LINE:
			if (BIT(tr_state, MCS51_T2EX_LINE))
			{
				set_exf2(1);
				m_t2ex_cnt++;
			}
			break;
		default:
			mcs51_cpu_device::handle_irq(irqline, state, new_state, tr_state);
	}
}


void i8052_device::device_start()
{
	mcs51_cpu_device::device_start();
	save_item(NAME(m_t2con));
	save_item(NAME(m_rcap2));
	save_item(NAME(m_t2));
}

void i8052_device::device_reset()
{
	mcs51_cpu_device::device_reset();
	m_t2con = 0;
	m_rcap2 = 0;
	m_t2 = 0;
}

void i8052_device::sfr_map(address_map &map)
{
	mcs51_cpu_device::sfr_map(map);
	map(0xc8, 0xc8).rw(FUNC(i8052_device::t2con_r), FUNC(i8052_device::t2con_w));
	map(0xca, 0xcb).rw(FUNC(i8052_device::rcap2_r), FUNC(i8052_device::rcap2_w));
	map(0xcc, 0xcd).rw(FUNC(i8052_device::t2_r   ), FUNC(i8052_device::t2_w   ));
}

u8   i8052_device::t2con_r ()
{
	return m_t2con;
}

void i8052_device::t2con_w (u8 data)
{
	m_t2con = data;
}

u8   i8052_device::rcap2_r (offs_t offset)
{
	return m_rcap2 >> (offset*8);
}

void i8052_device::rcap2_w (offs_t offset, u8 data)
{
	m_rcap2 = (m_rcap2 & ~(0xff << (offset*8))) | (data << (offset*8));
}

u8   i8052_device::t2_r (offs_t offset)
{
	return m_t2 >> (offset*8);
}

void i8052_device::t2_w (offs_t offset, u8 data)
{
	m_t2 = (m_t2 & ~(0xff << (offset*8))) | (data << (offset*8));
}

std::unique_ptr<util::disasm_interface> i8052_device::create_disassembler()
{
	return std::make_unique<i8052_disassembler>();
}
