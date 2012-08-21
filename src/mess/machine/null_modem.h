#ifndef NULL_MODEM_H_
#define NULL_MODEM_H_

#include "emu.h"
#include "machine/serial.h"
#include "imagedev/bitbngr.h"

class null_modem_device :
		public device_t,
		public device_serial_port_interface
{
public:
	null_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
	DECLARE_WRITE_LINE_MEMBER( read ) { m_rbit = state; m_owner->out_rx(state); }
	virtual void tx(UINT8 state) { m_tbit = state; m_bitbanger->output(state); }
protected:
	virtual void device_start() { m_owner = dynamic_cast<serial_port_device *>(owner()); }
	virtual void device_reset() { m_owner->out_rx(1); m_rbit = 1; }
	virtual void device_config_complete() { m_shortname = "null_modem"; }
private:
	serial_port_device *m_owner;
	required_device<bitbanger_device> m_bitbanger;
};

extern const device_type NULL_MODEM;
#endif
