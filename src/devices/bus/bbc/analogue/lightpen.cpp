// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro Light Pens:

    Datapen Light Pen

    The Robin Educational Light Pen
      https://www.computinghistory.org.uk/det/33102/The-Robin-Pen/

    Stack Light Pen
      https://www.computinghistory.org.uk/det/12414/Stack-Lightpen/

    Stack Light Rifle

    Note:
    All lightpen behaviour is derived from the software provided with
    each pen. Analysis of actual pens is required to confirm button
    polarity, when the strobe is active, and whether light intensity
    is captured by ADVAL.

**********************************************************************/

#include "emu.h"
#include "lightpen.h"


namespace {

class bbc_lightpen_device : public device_t, public device_bbc_analogue_interface
{
protected:
	bbc_lightpen_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_bbc_analogue_interface(mconfig, *this)
		, m_light(*this, "LIGHT%u", 0)
		, m_button(*this, "BUTTON")
		, m_pen_timer(nullptr)
		, m_lpstb_timer(nullptr)
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t pb_r() override
	{
		return m_button->read() & 0x30;
	}

	virtual uint16_t ch_r(offs_t channel) override
	{
		// intensity read on ADVAL(1) - not sure which pens require this, maybe only Datapen and Robin.
		return 0xfff0;
	}

private:
	required_ioport_array<2> m_light;
	required_ioport m_button;

	TIMER_CALLBACK_MEMBER(update_pen);
	TIMER_CALLBACK_MEMBER(lpstb);

	emu_timer *m_pen_timer;
	emu_timer *m_lpstb_timer;
};


// ======================> bbc_datapen_device

class bbc_datapen_device : public bbc_lightpen_device
{
public:
	bbc_datapen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_lightpen_device(mconfig, BBC_DATAPEN, tag, owner, clock)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbc_robinlp_device

class bbc_robinlp_device : public bbc_lightpen_device
{
public:
	bbc_robinlp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_lightpen_device(mconfig, BBC_ROBINLP, tag, owner, clock)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbc_stacklp_device

class bbc_stacklp_device : public bbc_lightpen_device
{
public:
	bbc_stacklp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_lightpen_device(mconfig, BBC_STACKLP, tag, owner, clock)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbc_stacklr_device

class bbc_stacklr_device : public bbc_lightpen_device
{
public:
	bbc_stacklr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_lightpen_device(mconfig, BBC_STACKLR, tag, owner, clock)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( lightpen )
	PORT_START("LIGHT0")
	PORT_BIT(0x3ff, 0x200, IPT_LIGHTGUN_X) PORT_MINMAX(0x00, 0x3ff) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
	PORT_START("LIGHT1")
	PORT_BIT(0x3ff, 0x200, IPT_LIGHTGUN_Y) PORT_MINMAX(0x00, 0x3ff) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END

static INPUT_PORTS_START( contact_low )
	PORT_INCLUDE(lightpen)

	PORT_START("BUTTON")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Contact")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( contact_high )
	PORT_INCLUDE(lightpen)

	PORT_START("BUTTON")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Contact")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( trigger )
	PORT_INCLUDE(lightpen)

	PORT_START("BUTTON")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Trigger")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


ioport_constructor bbc_datapen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( contact_low );
}

ioport_constructor bbc_robinlp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( contact_low );
}

ioport_constructor bbc_stacklp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( contact_high );
}

ioport_constructor bbc_stacklr_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trigger );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_lightpen_device::device_start()
{
	if (!screen())
		fatalerror("Can't find screen device required for %s\n", name());

	if (!screen()->started())
		throw device_missing_dependencies();

	m_pen_timer = timer_alloc(FUNC(bbc_lightpen_device::update_pen), this);
	m_pen_timer->adjust(attotime::zero, 0, screen()->frame_period());

	m_lpstb_timer = timer_alloc(FUNC(bbc_lightpen_device::lpstb), this);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_CALLBACK_MEMBER(bbc_lightpen_device::update_pen)
{
	int x_val = m_light[0]->read();
	int y_val = m_light[1]->read();
	const rectangle &vis_area = screen()->visible_area();

	int xt = x_val * vis_area.width() / 1024 + vis_area.min_x;
	int yt = y_val * vis_area.height() / 1024 + vis_area.min_y;

	m_lpstb_timer->adjust(screen()->time_until_pos(yt, xt));
}

TIMER_CALLBACK_MEMBER(bbc_lightpen_device::lpstb)
{
	bool lpstb_active = false;

	if (device().type() == BBC_STACKLP)
	{
		// strobe always active
		lpstb_active = true;
	}
	else if ((device().type() == BBC_DATAPEN) || (device().type() == BBC_ROBINLP) || (device().type() == BBC_STACKLR))
	{
		// strobe active when button low
		if (!BIT(m_button->read(), 4))
			lpstb_active = true;
	}

	if (lpstb_active)
	{
		m_slot->lpstb_w(1);
		m_slot->lpstb_w(0);
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_DATAPEN, device_bbc_analogue_interface, bbc_datapen_device, "datapen", "Datapen Light Pen (BBC Micro)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_ROBINLP, device_bbc_analogue_interface, bbc_robinlp_device, "robinlp", "The Robin Educational Light Pen")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_STACKLP, device_bbc_analogue_interface, bbc_stacklp_device, "stacklp", "Stack Light Pen (BBC Micro)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_STACKLR, device_bbc_analogue_interface, bbc_stacklr_device, "stacklr", "Stack Light Rifle (BBC Micro)")
