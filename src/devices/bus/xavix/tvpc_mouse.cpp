// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Mouse and port devices for the original and Super TV-PC families. Expose
    cumulative X/Y motion and button levels to the console's input counters,
    with microphone audio routed by the compatible mouse variants.

    The original and Super TV-PC connectors differ; the model supplies a
    shared motion and button interface.

    TODO:
    - Confirm connector pinouts and physical signalling for original and
      Super TV-PC mice.

***************************************************************************/

#include "emu.h"
#include "tvpc_mouse.h"

#include "speaker.h"

namespace {

INPUT_PORTS_START(tvpc_mouse)
	// Device-type validation constructs the mouse without its port.
	auto const *const port = dynamic_cast<tvpc_mouse_port_device const *>(owner.owner());
	unsigned const index = port ? port->mouse_index() : 0;

	PORT_START("X")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_NAME(index ? "Mouse 2 X" : "Mouse 1 X") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_CODE(MOUSECODE_X_INDEXED(index)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)

	PORT_START("Y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_NAME(index ? "Mouse 2 Y" : "Mouse 1 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_CODE(MOUSECODE_Y_INDEXED(index)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)

	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME(index ? "Mouse 2 Button" : "Mouse 1 Button") PORT_CODE(MOUSECODE_BUTTON1_INDEXED(index))
INPUT_PORTS_END

class tvpc_mouse_device : public device_t, public device_tvpc_mouse_interface
{
public:
	tvpc_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: tvpc_mouse_device(mconfig, TVPC_MOUSE, tag, owner, clock)
	{
	}

	virtual u8 x_r() override { return m_axes[0]->read(); }
	virtual u8 y_r() override { return m_axes[1]->read(); }
	virtual int button_r() override { return BIT(m_button->read(), 0); }

protected:
	tvpc_mouse_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_tvpc_mouse_interface(mconfig, *this)
		, m_axes(*this, "%c", 'X')
		, m_button(*this, "BUTTON")
	{
	}

	virtual void device_start() override ATTR_COLD { }
	virtual ioport_constructor device_input_ports() const override ATTR_COLD { return INPUT_PORTS_NAME(tvpc_mouse); }

private:
	required_ioport_array<2> m_axes;
	required_ioport m_button;
};

class tvpc_hk_mouse_device : public tvpc_mouse_device
{
public:
	tvpc_hk_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: tvpc_mouse_device(mconfig, TVPC_HK_MOUSE, tag, owner, clock)
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		MICROPHONE(config, "microphone", 1).front_center().add_route(0, DEVICE_SELF_OWNER, 1.0);
	}
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(TVPC_MOUSE, device_tvpc_mouse_interface, tvpc_mouse_device, "tvpc_mouse", "TV-PC Mouse")
DEFINE_DEVICE_TYPE_PRIVATE(TVPC_HK_MOUSE, device_tvpc_mouse_interface, tvpc_hk_mouse_device, "tvpc_hk_mouse", "Hello Kitty TV-PC Mouse with Flower Microphone")
DEFINE_DEVICE_TYPE(TVPC_MOUSE_PORT, tvpc_mouse_port_device, "tvpc_mouse_port", "TV-PC Mouse Port")

tvpc_mouse_port_device::tvpc_mouse_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TVPC_MOUSE_PORT, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, device_single_card_slot_interface<device_tvpc_mouse_interface>(mconfig, *this)
	, m_mouse_index(0)
{
	// Microphone mice are added by boards with a connected audio input.
	set_options([] (device_slot_interface &port)
	{
		port.option_add("mouse", TVPC_MOUSE);
	}, nullptr, false);
}

device_tvpc_mouse_interface::device_tvpc_mouse_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "tvpcmouse")
{
}

void tvpc_mouse_port_device::device_start()
{
	m_mouse = get_card_device();
}

u8 device_tvpc_mouse_interface::x_r()
{
	return 0;
}

u8 device_tvpc_mouse_interface::y_r()
{
	return 0;
}

int device_tvpc_mouse_interface::button_r()
{
	return 0;
}

u8 tvpc_mouse_port_device::x_r()
{
	return m_mouse ? m_mouse->x_r() : 0;
}

u8 tvpc_mouse_port_device::y_r()
{
	return m_mouse ? m_mouse->y_r() : 0;
}

int tvpc_mouse_port_device::button_r()
{
	return m_mouse ? m_mouse->button_r() : 0;
}
