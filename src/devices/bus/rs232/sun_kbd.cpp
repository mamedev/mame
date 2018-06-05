#include "emu.h"
#include "sun_kbd.h"


DEFINE_DEVICE_TYPE(SUN_KBD_ADAPTOR, sun_keyboard_adaptor_device, "sunkbd_adaptor", "Sun Keyboard Adaptor")


sun_keyboard_adaptor_device::sun_keyboard_adaptor_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, SUN_KBD_ADAPTOR, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_keyboard_port(*this, "keyboard")
{
}


sun_keyboard_adaptor_device::~sun_keyboard_adaptor_device()
{
}


MACHINE_CONFIG_START(sun_keyboard_adaptor_device::device_add_mconfig)
	MCFG_DEVICE_ADD(m_keyboard_port, SUNKBD_PORT, default_sun_keyboard_devices, nullptr)
	MCFG_SUNKBD_RXD_HANDLER(WRITELINE(*this, sun_keyboard_adaptor_device, output_rxd))
MACHINE_CONFIG_END


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
