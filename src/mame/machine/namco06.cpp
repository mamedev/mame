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
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


TIMER_CALLBACK_MEMBER( namco_06xx_device::nmi_generate )
{
	if (!m_nmicpu->suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		LOG(("NMI cpu '%s'\n",m_nmicpu->tag()));
		m_nmicpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	else
		LOG(("NMI not generated because cpu '%s' is suspended\n",m_nmicpu->tag()));
}


READ8_MEMBER( namco_06xx_device::data_r )
{
	UINT8 result = 0xff;

	LOG(("%s: 06XX '%s' read offset %d\n",machine().describe_context(),tag(),offset));

	if (!(m_control & 0x10))
	{
		logerror("%s: 06XX '%s' read in write mode %02x\n",machine().describe_context(),tag(),m_control);
		return 0;
	}

	if ((m_control & (1 << 0)) && !m_read_0.isnull()) result &= m_read_0(space, 0);
	if ((m_control & (1 << 1)) && !m_read_1.isnull()) result &= m_read_1(space, 0);
	if ((m_control & (1 << 2)) && !m_read_2.isnull()) result &= m_read_2(space, 0);
	if ((m_control & (1 << 3)) && !m_read_3.isnull()) result &= m_read_3(space, 0);

	return result;
}


WRITE8_MEMBER( namco_06xx_device::data_w )
{
	LOG(("%s: 06XX '%s' write offset %d = %02x\n",machine().describe_context(),tag(),offset,data));

	if (m_control & 0x10)
	{
		logerror("%s: 06XX '%s' write in read mode %02x\n",machine().describe_context(),tag(),m_control);
		return;
	}
	if ((m_control & (1 << 0)) && !m_write_0.isnull()) m_write_0(space, 0, data);
	if ((m_control & (1 << 1)) && !m_write_1.isnull()) m_write_1(space, 0, data);
	if ((m_control & (1 << 2)) && !m_write_2.isnull()) m_write_2(space, 0, data);
	if ((m_control & (1 << 3)) && !m_write_3.isnull()) m_write_3(space, 0, data);
}


READ8_MEMBER( namco_06xx_device::ctrl_r )
{
	LOG(("%s: 06XX '%s' ctrl_r\n",machine().describe_context(),tag()));
	return m_control;
}

WRITE8_MEMBER( namco_06xx_device::ctrl_w )
{
	LOG(("%s: 06XX '%s' control %02x\n",space.machine().describe_context(),tag(),data));

	m_control = data;

	if ((m_control & 0x0f) == 0)
	{
		LOG(("disabling nmi generate timer\n"));
		m_nmi_timer->adjust(attotime::never);
	}
	else
	{
		LOG(("setting nmi generate timer to 200us\n"));

		// this timing is critical. Due to a bug, Bosconian will stop responding to
		// inputs if a transfer terminates at the wrong time.
		// On the other hand, the time cannot be too short otherwise the 54XX will
		// not have enough time to process the incoming controls.
		m_nmi_timer->adjust(attotime::from_usec(200), 0, attotime::from_usec(200));

		if (m_control & 0x10) {
			if ((m_control & (1 << 0)) && !m_readreq_0.isnull()) m_readreq_0(space, 0);
			if ((m_control & (1 << 1)) && !m_readreq_1.isnull()) m_readreq_1(space, 0);
			if ((m_control & (1 << 2)) && !m_readreq_2.isnull()) m_readreq_2(space, 0);
			if ((m_control & (1 << 3)) && !m_readreq_3.isnull()) m_readreq_3(space, 0);
		}
	}
}


const device_type NAMCO_06XX = &device_creator<namco_06xx_device>;

namco_06xx_device::namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_06XX, "Namco 06xx", tag, owner, clock, "namco06xx", __FILE__),
	m_control(0),
	m_nmicpu(*this),
	m_read_0(*this),
	m_read_1(*this),
	m_read_2(*this),
	m_read_3(*this),
	m_readreq_0(*this),
	m_readreq_1(*this),
	m_readreq_2(*this),
	m_readreq_3(*this),
	m_write_0(*this),
	m_write_1(*this),
	m_write_2(*this),
	m_write_3(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_06xx_device::device_start()
{
	m_read_0.resolve();
	m_read_1.resolve();
	m_read_2.resolve();
	m_read_3.resolve();
	m_readreq_0.resolve();
	m_readreq_1.resolve();
	m_readreq_2.resolve();
	m_readreq_3.resolve();
	m_write_0.resolve();
	m_write_1.resolve();
	m_write_2.resolve();
	m_write_3.resolve();
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
