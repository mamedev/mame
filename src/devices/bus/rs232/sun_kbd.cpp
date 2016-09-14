#include "sun_kbd.h"


device_type const SUN_KBD_ADAPTOR = &device_creator<sun_keyboard_adaptor_device>;


namespace {
MACHINE_CONFIG_FRAGMENT(sun_keyboard_adaptor)
	MCFG_SUNKBD_PORT_ADD("keyboard", default_sun_keyboard_devices, nullptr)
	MCFG_SUNKBD_RXD_HANDLER(WRITELINE(sun_keyboard_adaptor_device, output_rxd))
MACHINE_CONFIG_END
} // anonymous namespace


sun_keyboard_adaptor_device::sun_keyboard_adaptor_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		UINT32 clock)
	: device_t(mconfig, SUN_KBD_ADAPTOR, "Sun Keyboard Adaptor", tag, owner, clock, "sunkbd_adaptor", __FILE__)
	, device_rs232_port_interface(mconfig, *this)
	, m_keyboard_port(*this, "keyboard")
{
}


sun_keyboard_adaptor_device::~sun_keyboard_adaptor_device()
{
}


machine_config_constructor sun_keyboard_adaptor_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(sun_keyboard_adaptor);
}


WRITE_LINE_MEMBER( sun_keyboard_adaptor_device::input_txd )
{
	m_keyboard_port->write_txd(state);
}


void sun_keyboard_adaptor_device::device_start()
{
}


void sun_keyboard_adaptor_device::device_reset()
{
	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}
