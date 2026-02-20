// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/***************************************************************************

    TOSHIBA TLCS900 - TMP94C241 SERIAL

***************************************************************************/

#include "emu.h"
#include "tmp94c241.h"
#include "tmp94c241_serial.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TMP94C241_SERIAL, tmp94c241_serial_device, "tmp94c241_serial", "TMP94C241 Serial Channel")

tmp94c241_serial_device::tmp94c241_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t channel, uint32_t clock) :
	device_t(mconfig, TMP94C241_SERIAL, tag, owner, clock),
	m_channel(channel),
	m_serial_control(0),
	m_serial_mode(0), /* I/O interface mode and clock source at TO2 trigger */
	m_baud_rate(0),
	m_hz(0),
	m_rx_clock_count(8),
	m_rx_shift_register(0),
	m_rx_buffer(0),
	m_rxd(0),
	m_rxd_prev(0),
	m_sioclk_state(0),
	m_tx_clock_count(0),
	m_tx_shift_register(0),
	m_txd(0),
	m_sclk_out(0),
	m_txd_cb(*this),
	m_sclk_in_cb(*this),
	m_sclk_out_cb(*this)
{
}

void tmp94c241_serial_device::device_start()
{
	m_timer = timer_alloc(FUNC(tmp94c241_serial_device::timer_callback), this);
	m_cpu = dynamic_cast<tmp94c241_device *>(owner());

	save_item(NAME(m_serial_control));
	save_item(NAME(m_serial_mode));
	save_item(NAME(m_baud_rate));
	save_item(NAME(m_hz));
	save_item(NAME(m_rx_clock_count));
	save_item(NAME(m_rx_shift_register));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rxd));
	save_item(NAME(m_rxd_prev));
	save_item(NAME(m_sioclk_state));
	save_item(NAME(m_tx_clock_count));
	save_item(NAME(m_tx_shift_register));
	save_item(NAME(m_txd));
	save_item(NAME(m_sclk_out));

	m_sclk_out_cb(m_sclk_out);
	m_txd_cb(m_txd);
}

void tmp94c241_serial_device::device_reset()
{
	m_serial_control &= 0x80;
	m_serial_mode &= 0x80;
	m_baud_rate = 0x00;
}

void tmp94c241_serial_device::TO2_trigger(int state)
{
//	logerror("TO2_trigger state=%d\n", state);
	if ((m_serial_mode & 3) == 0 && BIT(m_serial_control, 1))
	{
		/* Clock source: TO2 output compare trigger */
		sioclk(state);
	}
}

void tmp94c241_serial_device::sioclk(int state)
{
	if (m_sioclk_state == state)
		return;

	m_sioclk_state = state;
	// logerror("sioclk state=%d rxd=%d m_rx_clock_count=%d txd=%d m_tx_clock_count=%d\n", m_sioclk_state, m_rxd, m_rx_clock_count, m_txd, m_tx_clock_count);

	if (m_rx_clock_count){
		m_rx_clock_count--;

		m_rx_shift_register >>= 1;
		m_rx_shift_register |= (m_rxd << 7);

		if (m_rx_clock_count == 0)
		{
			m_rx_clock_count = 8;
			m_rx_buffer = m_rx_shift_register;
			m_cpu->m_int_reg[(m_channel == 0) ? INTES0 : INTES1] |= 0x08;
			m_cpu->m_check_irqs = 1;
		}
	}

	if (m_tx_clock_count){
		logerror("send bit #%d: %d\n", 8-m_tx_clock_count, m_tx_shift_register & 1);

		m_txd_cb(m_tx_shift_register & 1);
		m_sclk_out_cb(1);
		m_sclk_out_cb(0);
		m_tx_shift_register >>= 1;
		if (--m_tx_clock_count == 0) {
			logerror("Finished sending byte.\n");
			// We finished sending the data:
			m_cpu->m_int_reg[(m_channel == 0) ? INTES0 : INTES1] |= 0x80;
			m_cpu->m_check_irqs = 1;
		}
	}
}

void tmp94c241_serial_device::rxd(int state)
{
	if (m_rxd != state)
	{
		m_rxd = state;
	}
}

uint8_t tmp94c241_serial_device::scNbuf_r()
{
	return m_rx_buffer;
}

void tmp94c241_serial_device::scNbuf_w(uint8_t data)
{
	logerror("buf write: %02X\n", data);
	m_tx_shift_register = data;
	m_tx_clock_count = 8;
	//if (m_channel == 1) machine().debug_break();
}

uint8_t tmp94c241_serial_device::scNcr_r()
{
	return m_serial_control;
}

void tmp94c241_serial_device::scNcr_w(uint8_t data)
{
	m_rx_clock_count = 8;
	m_serial_control = data;
}

uint8_t tmp94c241_serial_device::scNmod_r()
{
	return m_serial_mode;
}

void tmp94c241_serial_device::scNmod_w(uint8_t data)
{
	switch((data >> 2) & 3)
	{
		case 0: logerror("I/O interface mode\n"); break;
		case 1: logerror("7-bit uart mode (Not implemented yet)\n"); break;
		case 2: logerror("8-bit uart mode (Not implemented yet)\n"); break;
		case 3: logerror("9-bit uart mode (Not implemented yet)\n"); break;
	}
	switch(data & 3)
	{
		case 0: logerror("clk source: TO2 trigger\n"); break;
		case 1: logerror("clk source: Baud rate generator (Not implemented yet)\n"); break;
		case 2: logerror("clk source: Internal clock at ϕ1 (Not implemented yet)\n"); break;
		case 3: logerror("clk source: external clock (SCLK%d) (Not implemented yet)\n", m_channel); break;
	}
	m_serial_mode = data;
	m_cpu->m_int_reg[(m_channel == 0) ? INTES0 : INTES1] |= 0x80;
	m_cpu->m_check_irqs = 1;
}

uint8_t tmp94c241_serial_device::brNcr_r()
{
	return m_baud_rate;
}

void tmp94c241_serial_device::brNcr_w(uint8_t data)
{
	m_baud_rate = data;
	uint8_t divisor = data & 0x0f;
	uint8_t input_clocks[] = {0, 2, 8, 32};
	uint8_t shift_amount = (((data >> 4) & 3) + 1) * 2;
	logerror("baud rate: Divisor=%d  Internal Clock T%d\n", divisor, input_clocks[(data >> 4) & 3]);
	if (divisor)
	{
		long int fc = 16'000'000; // TODO: set this from the cpu.
		m_hz = (fc >> shift_amount) / divisor;
		m_timer->adjust(attotime::from_hz(m_hz), 0, attotime::from_hz(m_hz));
		logerror("timer set to %d Hz.\n", m_hz);
	} else {
		m_timer->reset(attotime::never);
		m_hz = 0;
		logerror("timer disabled.\n");
	}
	//if (m_channel == 1) machine().debug_break();
}

/*
	0x3f means m_port_function[PORT_F]

reset:
...
ef03d8: 08 3f 73              ld (0x3f),0x73	; disable ch.0 (bit 2) / enable ch.1 (bit 6)


Reached from which routine ?:
...
fc4068: c1 8f 8d 3e 50        or (0x8d8f),0x50	; enable channel 1 (bit 6)
fc406d: c1 8f 8d 21           ld A,(0x8d8f)
fc4071: f0 3f 41              ld (0x3f),A


SERIAL_METHOD_2:      ; FC45A8
...
fc45ab: c1 8f 8d 3e 50        or (0x8d8f),0x50	; enable channel 1 (bit 6)
fc45b0: c1 8f 8d 21           ld A,(0x8d8f)
fc45b4: f0 3f 41              ld (0x3f),A

SERIAL_METHOD_4:       ; FC460D
...
fc4610: c1 8f 8d 3e 50        or (0x8d8f),0x50	; enable channel 1 (bit 6)
fc4615: c1 8f 8d 21           ld A,(0x8d8f)
fc4619: f0 3f 41              ld (0x3f),A


*/

TIMER_CALLBACK_MEMBER(tmp94c241_serial_device::timer_callback)
{

	if (m_hz && m_tx_clock_count && (m_cpu->m_port_function[PORT_F] & (1 << (m_channel==0 ? 2: 6))))
	{
		//logerror("m_tx_clock_count=%d and we'll call sioclk(%d).\n", m_tx_clock_count, m_sioclk_state ^ 1);
		sioclk(m_sioclk_state ^ 1);
	}
}

