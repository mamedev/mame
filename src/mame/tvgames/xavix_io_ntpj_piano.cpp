// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    epo_ntpj keyboard and guide-light controller. Serialises a
    24-bit snapshot of the piano keys on host clock transitions, signals data
    availability through an I/O event, and drives the multiplexed white-key
    guide lights from the parallel ports' row and column levels.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ntpj_piano.h"

DEFINE_DEVICE_TYPE(XAVIX_PIANO_CONTROLLER, xavix_piano_controller_device, "xavix_piano", "XaviX Piano Controller")

xavix_piano_controller_device::xavix_piano_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_PIANO_CONTROLLER, tag, owner, clock)
	, m_event_out_cb(*this)
	, m_keys_cb(*this, 0)
	, m_led_pwm(*this, "led_pwm")
	, m_key_lights(*this, "piano_key_light%u", 0U)
{
}

void xavix_piano_controller_device::device_add_mconfig(machine_config &config)
{
	PWM_DISPLAY(config, m_led_pwm).set_size(3, 5);
	m_led_pwm->output_x().set(FUNC(xavix_piano_controller_device::light_w));
}

void xavix_piano_controller_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(xavix_piano_controller_device::event_timer), this);

	save_item(NAME(m_shift));
	save_item(NAME(m_bit));
	save_item(NAME(m_data));
	save_item(NAME(m_clock));
	save_item(NAME(m_scanning));
}

void xavix_piano_controller_device::device_reset()
{
	m_event_timer->adjust(attotime::never);
	m_shift = 0;
	m_bit = 0;
	m_data = 0;
	m_clock = false;
	m_scanning = false;
	m_led_pwm->clear();
}

void xavix_piano_controller_device::host_w(u8 data, u8 direction)
{
	bool const clock = BIT(data, 5);
	if (!BIT(direction, 5))
	{
		m_event_timer->adjust(attotime::never);
		m_bit = 0;
		m_data = 0;
		m_clock = clock;
		m_scanning = false;
		return;
	}
	if (clock == m_clock)
		return;

	if (!m_scanning)
	{
		m_shift = u32(m_keys_cb[0]()) | (u32(m_keys_cb[1]()) << 8) | (u32(m_keys_cb[2]()) << 16);
		m_bit = 0;
		m_data = BIT(m_shift, m_bit);
		m_scanning = true;
		m_event_timer->adjust(attotime::from_usec(10));
	}
	else if (++m_bit < 24)
	{
		m_data = BIT(m_shift, m_bit);
		m_event_timer->adjust(attotime::from_usec(10));
	}
	else
	{
		m_data = 0;
		m_scanning = false;
	}

	m_clock = clock;
}

void xavix_piano_controller_device::pio1_w(u8 data, u8 direction)
{
	// The five column lines sink current; released pins leave their LEDs off.
	m_led_pwm->write_mx(BIT(~data & direction, 3, 5));
}

void xavix_piano_controller_device::pio2_w(u8 data, u8 direction)
{
	m_led_pwm->write_my(data & direction & 0x07);
}

void xavix_piano_controller_device::light_w(offs_t offset, u8 data)
{
	// White keys run F3-E5; PIO2.2/PIO1.3 is the unpopulated matrix position.
	u8 const key = GUIDE_KEYS[offset & 0x3f][offset >> 6];
	if (key < 14)
		m_key_lights[key] = data;
}

TIMER_CALLBACK_MEMBER(xavix_piano_controller_device::event_timer)
{
	m_event_out_cb(1);
}
