// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Namco 06XX

    This chip is used as an interface to up to 4 other custom chips.
    It signals IRQs to the custom MCUs when writes happen, and generates
    NMIs to the controlling CPU to drive reads based on a clock.

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
#include "machine/namco06.h"

#define VERBOSE 0
#include "logmacro.h"


TIMER_CALLBACK_MEMBER( namco_06xx_device::nmi_generate )
{
	if (!m_nmicpu->suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		LOG("NMI cpu '%s'\n",m_nmicpu->tag());
		m_nmicpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	else
		LOG("NMI not generated because cpu '%s' is suspended\n",m_nmicpu->tag());
}


READ8_MEMBER( namco_06xx_device::data_r )
{
	uint8_t result = 0xff;

	LOG("%s: 06XX '%s' read offset %d\n",machine().describe_context(),tag(),offset);

	if (!(m_control & 0x10))
	{
		logerror("%s: 06XX '%s' read in write mode %02x\n",machine().describe_context(),tag(),m_control);
		return 0;
	}

	if (BIT(m_control, 0)) result &= m_read[0](space, 0);
	if (BIT(m_control, 1)) result &= m_read[1](space, 0);
	if (BIT(m_control, 2)) result &= m_read[2](space, 0);
	if (BIT(m_control, 3)) result &= m_read[3](space, 0);

	return result;
}


WRITE8_MEMBER( namco_06xx_device::data_w )
{
	LOG("%s: 06XX '%s' write offset %d = %02x\n",machine().describe_context(),tag(),offset,data);

	if (m_control & 0x10)
	{
		logerror("%s: 06XX '%s' write in read mode %02x\n",machine().describe_context(),tag(),m_control);
		return;
	}
	if (BIT(m_control, 0)) m_write[0](space, 0, data);
	if (BIT(m_control, 1)) m_write[1](space, 0, data);
	if (BIT(m_control, 2)) m_write[2](space, 0, data);
	if (BIT(m_control, 3)) m_write[3](space, 0, data);
}


READ8_MEMBER( namco_06xx_device::ctrl_r )
{
	LOG("%s: 06XX '%s' ctrl_r\n",machine().describe_context(),tag());
	return m_control;
}

WRITE8_MEMBER( namco_06xx_device::ctrl_w )
{
	LOG("%s: 06XX '%s' control %02x\n",machine().describe_context(),tag(),data);

	m_control = data;

	if ((m_control & 0x0f) == 0)
	{
		LOG("disabling nmi generate timer\n");
		m_nmi_timer->adjust(attotime::never);
	}
	else
	{
		LOG("setting nmi generate timer to 200us\n");

		// this timing is critical. Due to a bug, Bosconian will stop responding to
		// inputs if a transfer terminates at the wrong time.
		// On the other hand, the time cannot be too short otherwise the 54XX will
		// not have enough time to process the incoming controls.
		m_nmi_timer->adjust(attotime::from_usec(200), 0, attotime::from_usec(200));

		if (m_control & 0x10)
		{
			if (BIT(m_control, 0)) m_readreq[0](space, 0);
			if (BIT(m_control, 1)) m_readreq[1](space, 0);
			if (BIT(m_control, 2)) m_readreq[2](space, 0);
			if (BIT(m_control, 3)) m_readreq[3](space, 0);
		}
	}
}


DEFINE_DEVICE_TYPE(NAMCO_06XX, namco_06xx_device, "namco06", "Namco 06xx")

namco_06xx_device::namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_06XX, tag, owner, clock)
	, m_control(0)
	, m_nmicpu(*this, finder_base::DUMMY_TAG)
	, m_read{ { *this }, { *this }, { *this }, { *this } }
	, m_readreq{ { *this }, { *this }, { *this }, { *this } }
	, m_write{ { *this }, { *this }, { *this }, { *this } }
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_06xx_device::device_start()
{
	for (devcb_read8 &cb : m_read)
		cb.resolve_safe(0xff);

	for (devcb_write_line &cb : m_readreq)
		cb.resolve_safe();

	for (devcb_write8 &cb : m_write)
		cb.resolve_safe();

	/* allocate a timer */
	m_nmi_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(namco_06xx_device::nmi_generate),this));

	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_06xx_device::device_reset()
{
	m_control = 0;
}
