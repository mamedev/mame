// license:BSD-3-Clause
// copyright-holders:Mike Harris, Aaron Giles
/***************************************************************************

    Namco 06XX

    This chip is used as an interface to up to 4 other custom chips.
    It signals IRQs to the custom MCUs when writes happen, and generates
    NMIs to the controlling CPU to drive reads based on a clock.

    It uses a clock divider that's used to pulse the NMI and custom chip select
    lines.

    The control register controls chip as such: the low 4 bits are chip selects
    (active high), the 5th bit is read/!write, and the upper 3 bits are the
    clock divide.

    SD0-SD7 are data I/O lines connecting to the controlling CPU
    SEL selects either control (1) or data (0), usually connected to
        an address line of the controlling CPU
    /NMI is an NMI signal line for the controlling CPU

    ID0-ID7 are data I/O lines connecting to the other custom chips
    /IO1-/IO4 are IRQ signal lines for each custom chip

                   +------+
                [1]|1   28|Vcc
                ID7|2   27|SD7
                ID6|3   26|SD6
                ID5|4   25|SD5
                ID4|5   24|SD4
                ID3|6   23|SD3
                ID2|7   22|SD2
                ID1|8   21|SD1
                ID0|9   20|SD0
               /IO1|10  19|/NMI
               /IO2|11  18|/CS
               /IO3|12  17|CLOCK
               /IO4|13  16|R/W
                GND|14  15|SEL
                   +------+

    [1] on polepos, galaga, xevious, and bosco: connected to K3 of the 51xx
        on bosco and xevious, connected to R8 of the 50xx

    Note that Xevious has a potential race condition at 0xE9. It checks if
    the control register is 0x10 (chip disabled), then 4 instructions later
    acts on that check. If an NMI occurs between, it can crash.


    06XX interface:
    ---------------
    Galaga                  51XX  ----  ----  54XX
    Bosconian (CPU board)   51XX  ----  50XX  54XX
    Bosconian (Video board) 50XX  52XX  ----  ----
    Xevious                 51XX  ----  50XX  54XX
    Dig Dug                 51XX  53XX  ----  ----
    Pole Position / PP II   51XX  53XX  52XX  54XX


    Galaga writes:
        control = 10(000), data = FF at startup
        control = 71(011), read 3, control = 10
        control = A1(101), write 4, control = 10
        control = A8(101), write 12, control = 10

    Xevious writes:
        control = 10 at startup
        control = A1(101), write 6, control = 10
        control = 71(011), read 3, control = 10
        control = 64(011), write 1, control = 10
        control = 74(011), read 4, control = 10
        control = 68(011), write 7, control = 10

    Dig Dug writes:
        control = 10(000), data = 10 at startup
        control = A1(101), write 3, control = 10
        control = 71(011), read 3, control = 10
        control = D2(110), read 2, control = 10

    Bosco writes:
        control = 10(000), data = FF at startup
        control = C8(110), write 17, control = 10
        control = 61(011), write 1, control = 10
        control = 71(011), read 3, control = 10
        control = 94(100), read 4, control = 10
        control = 64(011), write 1, control = 10
        control = 84(100), write 5, control = 10


        control = 34(001), write 1, control = 10

***************************************************************************/

#include "emu.h"
#include "namco06.h"

#define VERBOSE 0
#include "logmacro.h"


TIMER_CALLBACK_MEMBER( namco_06xx_device::nmi_generate )
{
	// This timer runs at twice the clock, since we do work on both the
	// rising and falling edges.
	// m_timer_state == true is the falling clock edge
	m_timer_state = !m_timer_state;

	if (m_timer_state)
	{
		m_rw[0](0, BIT(m_control, 4));
		m_rw[1](0, BIT(m_control, 4));
		m_rw[2](0, BIT(m_control, 4));
		m_rw[3](0, BIT(m_control, 4));
	}

	// During reads, the first NMI pulse is suppressed to give the chip a
	// cycle to write.
	if (m_timer_state && !m_read_stretch)
	{
		set_nmi(ASSERT_LINE);
	}
	else
	{
		set_nmi(CLEAR_LINE);
	}
	m_read_stretch = false;

	m_chipsel[0](0, BIT(m_control, 0) && m_timer_state);
	m_chipsel[1](0, BIT(m_control, 1) && m_timer_state);
	m_chipsel[2](0, BIT(m_control, 2) && m_timer_state);
	m_chipsel[3](0, BIT(m_control, 3) && m_timer_state);
}

uint8_t namco_06xx_device::data_r(offs_t offset)
{
	uint8_t result = 0xff;

	if (!BIT(m_control, 4))
	{
		logerror("%s: 06XX '%s' read in write mode %02x\n",machine().describe_context(),tag(),m_control);
		return 0;
	}

	if (BIT(m_control, 0)) result &= m_read[0](0);
	if (BIT(m_control, 1)) result &= m_read[1](0);
	if (BIT(m_control, 2)) result &= m_read[2](0);
	if (BIT(m_control, 3)) result &= m_read[3](0);

	return result;
}


void namco_06xx_device::data_w(offs_t offset, uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_06xx_device::write_sync),this), data);
}

TIMER_CALLBACK_MEMBER( namco_06xx_device::write_sync )
{
	if (BIT(m_control, 4))
	{
		logerror("%s: 06XX '%s' write in read mode %02x\n",machine().describe_context(),tag(),m_control);
		return;
	}

	if (BIT(m_control, 0)) m_write[0](0, param);
	if (BIT(m_control, 1)) m_write[1](0, param);
	if (BIT(m_control, 2)) m_write[2](0, param);
	if (BIT(m_control, 3)) m_write[3](0, param);
}


uint8_t namco_06xx_device::ctrl_r()
{
	return m_control;
}

void namco_06xx_device::ctrl_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_06xx_device::ctrl_w_sync),this), data);
}

TIMER_CALLBACK_MEMBER( namco_06xx_device::ctrl_w_sync )
{
	m_control = param;

	// The upper 3 control bits are the clock divider.
	if ((m_control & 0xe0) == 0)
	{
		// If the divider is zero, stop the timer.
		m_nmi_timer->adjust(attotime::never);
		m_timer_state = false;
		set_nmi(CLEAR_LINE);
		m_chipsel[0](0, CLEAR_LINE);
		m_chipsel[1](0, CLEAR_LINE);
		m_chipsel[2](0, CLEAR_LINE);
		m_chipsel[3](0, CLEAR_LINE);
		// RW is left as-is
	}
	else
	{
		// NMI is cleared immediately if this is a read.
		// It will be suppressed the next clock cycle.
		if (BIT(m_control, 4))
		{
			set_nmi(CLEAR_LINE);
			m_read_stretch = true;
		}
		else
		{
			m_read_stretch = false;
		}

		uint8_t num_shifts = (m_control & 0xe0) >> 5;
		uint8_t divisor = 1 << num_shifts;
		attotime period = attotime::from_hz(clock() / divisor) / 2;

		// Delay to the next falling clock edge.
		attotime now = machine().time();
		u64 total_ticks = now.as_ticks(clock());
		attotime delay = attotime::from_ticks(total_ticks + 1, clock()) - now;
		m_nmi_timer->adjust(delay, 0, period);
	}
}


void namco_06xx_device::set_nmi(int state)
{
	if (!m_nmicpu->suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		m_nmicpu->set_input_line(INPUT_LINE_NMI, state);
	}
}


DEFINE_DEVICE_TYPE(NAMCO_06XX, namco_06xx_device, "namco06", "Namco 06xx")

namco_06xx_device::namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_06XX, tag, owner, clock)
	, m_control(0)
	, m_timer_state(false)
	, m_read_stretch(false)
	, m_nmicpu(*this, finder_base::DUMMY_TAG)
	, m_chipsel(*this)
	, m_rw(*this)
	, m_read(*this, 0xff)
	, m_write(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_06xx_device::device_start()
{
	/* allocate a timer */
	m_nmi_timer = timer_alloc(FUNC(namco_06xx_device::nmi_generate), this);

	save_item(NAME(m_control));
	save_item(NAME(m_timer_state));
	save_item(NAME(m_read_stretch));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_06xx_device::device_reset()
{
	m_control = 0;
}
