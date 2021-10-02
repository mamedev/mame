#include "emu.h"
#include "sun_kbd.h"

#include "bus/sunkbd/sunkbd.h"


namespace {

class sun_keyboard_adaptor_device : public device_t, public device_rs232_port_interface
{
public:
	sun_keyboard_adaptor_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock)
		: device_t(mconfig, SUN_KBD_ADAPTOR, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_keyboard_port(*this, "keyboard")
	{
	}

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { m_keyboard_port->write_txd(state); }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		SUNKBD_PORT(config, m_keyboard_port, default_sun_keyboard_devices, nullptr);
		m_keyboard_port->rxd_handler().set(FUNC(sun_keyboard_adaptor_device::output_rxd));
	}

	virtual void device_start() override
	{
	}

	virtual void device_reset() override
	{
		output_rxd(1);
		output_dcd(0);
		output_dsr(0);
		output_cts(0);
	}

private:
	required_device<sun_keyboard_port_device> m_keyboard_port;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SUN_KBD_ADAPTOR, device_rs232_port_interface, sun_keyboard_adaptor_device, "sunkbd_adaptor", "Sun Keyboard Adaptor")
