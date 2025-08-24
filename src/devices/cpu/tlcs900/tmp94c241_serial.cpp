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
	m_clock_count(0),
	m_tx_shift_register(0),
	m_serial_in(0),
	m_serial_in_prev(0),
	m_sck_in(0),
	m_serial_out(0),
	m_sck_out(0),
	m_serial_out_cb(*this),
	m_sck_cb(*this)
{
}

void tmp94c241_serial_device::device_start()
{
	m_tx_timer = timer_alloc(FUNC(tmp94c241_serial_device::tx_timer_callback), this);
	m_cpu = dynamic_cast<tmp94c241_device *>(owner());

	save_item(NAME(m_serial_control));
	save_item(NAME(m_serial_mode));
	save_item(NAME(m_baud_rate));
	save_item(NAME(m_hz));
	save_item(NAME(m_clock_count));
	save_item(NAME(m_tx_shift_register));
	save_item(NAME(m_serial_in));
	save_item(NAME(m_serial_in_prev));
	save_item(NAME(m_sck_in));
	save_item(NAME(m_serial_out));
	save_item(NAME(m_sck_out));

	m_sck_cb(m_sck_out);
	m_serial_out_cb(m_serial_out);
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
		sck(state);
	}
}

void tmp94c241_serial_device::sck(int state)
{
	if (m_sck_in != state)
	{
		m_sck_in = state;
		// logerror("sck state=%d\n", state);

		update_serial();
	}
}

void tmp94c241_serial_device::serial_in(int state)
{
	if (m_serial_in != state)
	{
		m_serial_in = state;

		update_serial();
	}
}

uint8_t tmp94c241_serial_device::scNbuf_r()
{
	return 0;
}

void tmp94c241_serial_device::scNbuf_w(uint8_t data)
{
	logerror("buf write: %02X\n", data);
	m_tx_shift_register = data;
	m_clock_count = 8;
	//if (m_channel == 1) machine().debug_break();
}

uint8_t tmp94c241_serial_device::scNcr_r()
{
	return m_serial_control;
}

void tmp94c241_serial_device::scNcr_w(uint8_t data)
{
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
		m_tx_timer->adjust(attotime::from_hz(m_hz), 0, attotime::from_hz(m_hz));
		logerror("tx_timer set to %d Hz.\n", m_hz);
	} else {
		m_tx_timer->reset(attotime::never);
		m_hz = 0;
		logerror("tx_timer disabled.\n");
	}
	//if (m_channel == 1) machine().debug_break();
}

void tmp94c241_serial_device::update_serial()
{
	//logerror("update_serial sck=%d serial_in=%d m_clock_count=%d\n", m_sck_in, m_serial_in, m_clock_count);
	if (m_clock_count){
		logerror("send bit #%d: %d\n", 8-m_clock_count, m_tx_shift_register & 1);

		m_serial_out_cb(m_tx_shift_register & 1);
		m_sck_cb(1);
		m_sck_cb(0);
		m_tx_shift_register >>= 1;
		if (--m_clock_count == 0) {
			logerror("Finished sending byte.\n");
			// We finished sending the data:
			m_cpu->m_int_reg[(m_channel == 0) ? INTES0 : INTES1] |= 0x80;
			m_cpu->m_check_irqs = 1;
		}
	}
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

TIMER_CALLBACK_MEMBER(tmp94c241_serial_device::tx_timer_callback)
{
	if (m_hz && m_clock_count && (m_cpu->m_port_function[PORT_F] & (1 << (m_channel==0 ? 2: 6))))
	{
		//logerror("m_clock_count=%d and we'll call sck(%d).\n", m_clock_count, m_sck_in ^ 1);
		sck(m_sck_in ^ 1);
	}
}

