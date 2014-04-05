#ifndef NULL_MODEM_H_
#define NULL_MODEM_H_

#include "bus/rs232/rs232.h"
#include "imagedev/bitbngr.h"

class null_modem_device : public device_t,
	public device_rs232_port_interface
{
public:
	null_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;

	WRITE_LINE_MEMBER( read ) { output_rxd(state); } /// HACK for DEVCB
	virtual WRITE_LINE_MEMBER( input_txd ) { if (started()) m_bitbanger->output(state); }

protected:
	virtual void device_start() { output_dcd(0); output_dsr(0); output_cts(0); }
	virtual void device_reset() { output_rxd(1); }

private:
	required_device<bitbanger_device> m_bitbanger;
};

extern const device_type NULL_MODEM;

#endif
