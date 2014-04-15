#include "null_modem.h"

const device_type NULL_MODEM = &device_creator<null_modem_device>;

null_modem_device::null_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NULL_MODEM, "Null Modem", tag, owner, clock, "null_modem", __FILE__),
	device_rs232_port_interface(mconfig, *this),
	m_bitbanger(*this, "bitbanger")
{
}

static MACHINE_CONFIG_FRAGMENT(null_modem_config)
	MCFG_DEVICE_ADD("bitbanger", BITBANGER, 0)
	MCFG_BITBANGER_INPUT_CB(WRITELINE(null_modem_device, read))		/* callback */
	MCFG_BITBANGER_DEFAULT_MODE(BITBANGER_MODEM)		/* default mode */
	MCFG_BITBANGER_DEFAULT_BAUD(BITBANGER_9600)		/* default output baud */
	MCFG_BITBANGER_DEFAULT_TUNE(BITBANGER_0PERCENT)		/* default fine tune adjustment */
MACHINE_CONFIG_END

machine_config_constructor null_modem_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(null_modem_config);
}
