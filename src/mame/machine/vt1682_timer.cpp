// license:BSD-3-Clause
// copyright-holders:David Haywood

// Do timers auto reload?
// Does writing to Enable actually turn on the timer, or writing to preload MSB? (which appears to be done AFTER turning on in many cases)

#include "emu.h"
#include "machine/vt1682_timer.h"

#define LOG_TIMER     (1U << 1)

#define LOG_ALL           ( LOG_TIMER )

#define VERBOSE             (LOG_TIMER)
#include "logmacro.h"

// NOTE, system timer base clock can also be changed with "0x210B TSYNEN"
// this can be handled externally by changing device clock?

DEFINE_DEVICE_TYPE(VT_VT1682_TIMER, vrt_vt1682_timer_device, "vt1682timer", "VRT VT1682 Timer")

vrt_vt1682_timer_device::vrt_vt1682_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_VT1682_TIMER, tag, owner, clock),
	m_timer(*this, "snd_timera"),
	m_irq_cb(*this)
{
}

void vrt_vt1682_timer_device::device_start()
{
	save_item(NAME(m_timer_preload_7_0));
	save_item(NAME(m_timer_preload_15_8));
	save_item(NAME(m_timer_enable));

	m_irq_cb.resolve();
}

void vrt_vt1682_timer_device::device_reset()
{
	m_timer_preload_7_0 = 0;
	m_timer_preload_15_8 = 0;
	m_timer_enable = 0;
}



/*
    Address 0x2100 r/w (SOUND CPU, Timer A)
    Address 0x2110 r/w (SOUND CPU, Timer B)

    Address 0x2101 r/w (MAIN CPU, Timer)

    0x80 - Timer xx Preload:7
    0x40 - Timer xx Preload:6
    0x20 - Timer xx Preload:5
    0x10 - Timer xx Preload:4
    0x08 - Timer xx Preload:3
    0x04 - Timer xx Preload:2
    0x02 - Timer xx Preload:1
    0x01 - Timer xx Preload:0
*/

READ8_MEMBER(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_r)
{
	uint8_t ret = m_timer_preload_7_0;
	if (!m_is_sound_timer) LOGMASKED(LOG_TIMER,"%s: vt1682_timer_preload_7_0_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vrt_vt1682_timer_device::vt1682_timer_preload_7_0_w)
{
	if (!m_is_sound_timer) LOGMASKED(LOG_TIMER,"%s: vt1682_timer_preload_7_0_w writing: %02x\n", machine().describe_context(), data);
	m_timer_preload_7_0 = data;
}


/*
    Address 0x2101 r/w (SOUND CPU, Timer A)
    Address 0x2111 r/w (SOUND CPU, Timer B)

    Address 0x2104 r/w (MAIN CPU, Timer)

    0x80 - Timer xx Preload:15
    0x40 - Timer xx Preload:14
    0x20 - Timer xx Preload:13
    0x10 - Timer xx Preload:12
    0x08 - Timer xx Preload:11
    0x04 - Timer xx Preload:10
    0x02 - Timer xx Preload:9
    0x01 - Timer xx Preload:8
*/

READ8_MEMBER(vrt_vt1682_timer_device::vt1682_timer_preload_15_8_r)
{
	uint8_t ret = m_timer_preload_15_8;
	if (!m_is_sound_timer) LOGMASKED(LOG_TIMER,"%s: vt1682_timer_preload_15_8_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

void vrt_vt1682_timer_device::update_timer()
{
	if (m_timer_enable & 0x01)
	{
		uint16_t preload = (m_timer_preload_15_8 << 8) | m_timer_preload_7_0;
		int realpreload = 65536 - preload;
		m_timer->adjust(attotime::from_hz(clock()) * realpreload, 0, attotime::from_hz(clock()) * realpreload);
	}
}

WRITE8_MEMBER(vrt_vt1682_timer_device::vt1682_timer_preload_15_8_w)
{
	if (!m_is_sound_timer) LOGMASKED(LOG_TIMER,"%s: vt1682_timer_preload_15_8_w writing: %02x\n", machine().describe_context(), data);
	m_timer_preload_15_8 = data;

	update_timer();
}





/*
    Address 0x2102 r/w (SOUND CPU, Timer A)
    Address 0x2112 r/w (SOUND CPU, Timer B)

    Address 0x2102 r/w (MAIN CPU, Timer)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - TMRxx IRQ
    0x01 - TMRxx EN
*/

READ8_MEMBER(vrt_vt1682_timer_device::vt1682_timer_enable_r)
{
	uint8_t ret = m_timer_enable;
	if (!m_is_sound_timer) LOGMASKED(LOG_TIMER,"%s: vt1682_timer_enable_r returning: %02x\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vrt_vt1682_timer_device::vt1682_timer_enable_w)
{
	// Timer A notes
	// For NTSC
	//Period = (65536 - Timer _PreLoad) / 21.4772 MHz
	//Timer PreLoad = 65536 - (Period in seconds) * 21.4772 * 1000000 )

	// For PAL
	// Period = (65536 - Timer PreLoad) / 26.601712 MHz
	//Timer PreLoad = 65536 - (Period in seconds) * 26.601712 * 1000000 )

	if (!m_is_sound_timer) LOGMASKED(LOG_TIMER, "%s: vt1682_timer_enable_w writing: %02x\n", machine().describe_context(), data);
	m_timer_enable = data;

	if (m_timer_enable & 0x01)
	{
		update_timer();
	}
	else
	{
		m_timer->adjust(attotime::never);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(vrt_vt1682_timer_device::timer_expired)
{
	if (m_timer_enable & 0x02)
	{
		m_irq_cb(true);
	}
}

/*
    Address 0x2103 r/w (SOUND CPU, Timer A)
    Address 0x2113 r/w (SOUND CPU, Timer B)

    Address 0x2103 r/w (MAIN CPU, Timer) (read or write?)

    0x80 - Timer xx IRQ Clear
    0x40 - Timer xx IRQ Clear
    0x20 - Timer xx IRQ Clear
    0x10 - Timer xx IRQ Clear
    0x08 - Timer xx IRQ Clear
    0x04 - Timer xx IRQ Clear
    0x02 - Timer xx IRQ Clear
    0x01 - Timer xx IRQ Clear

*/

WRITE8_MEMBER(vrt_vt1682_timer_device::vt1682_timer_irqclear_w)
{
	//if (!m_is_sound_timer) LOGMASKED(LOG_TIMER,"%s: vt1682_timer_irqclear_w writing: %02x\n", machine().describe_context(), data);
	m_irq_cb(false);
}

void vrt_vt1682_timer_device::device_add_mconfig(machine_config& config)
{
	TIMER(config, m_timer).configure_periodic(FUNC(vrt_vt1682_timer_device::timer_expired), attotime::never);
}

void vrt_vt1682_timer_device::change_clock()
{
	notify_clock_changed();
}
