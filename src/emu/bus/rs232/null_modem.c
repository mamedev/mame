#include "null_modem.h"

const device_type NULL_MODEM = &device_creator<null_modem_device>;

null_modem_device::null_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NULL_MODEM, "Null Modem", tag, owner, clock, "null_modem", __FILE__),
	device_rs232_port_interface(mconfig, *this),
	m_bitbanger(*this, "bitbanger")
{
}

static bitbanger_config null_modem_image_config =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, null_modem_device, read),
	BITBANGER_MODEM,
	BITBANGER_9600,
	BITBANGER_0PERCENT
};

static MACHINE_CONFIG_FRAGMENT(null_modem_config)
	MCFG_BITBANGER_ADD("bitbanger", null_modem_image_config);
MACHINE_CONFIG_END

machine_config_constructor null_modem_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(null_modem_config);
}
