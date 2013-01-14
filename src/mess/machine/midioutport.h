/*********************************************************************

    midioutport.h

    MIDI Out serial port - glues the image device to the pluggable serial port

*********************************************************************/

#ifndef _MIDIOUTPORT_H_
#define _MIDIOUTPORT_H_

#include "emu.h"
#include "machine/serial.h"
#include "imagedev/midiout.h"

class midiout_port_device :
		public device_t,
		public device_serial_port_interface
{
public:
	midiout_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
	DECLARE_WRITE_LINE_MEMBER( read ) { }
	virtual void tx(UINT8 state) { m_midiout->tx(state); }
protected:
	virtual void device_start() { }
	virtual void device_reset() { }
	virtual void device_config_complete() { m_shortname = "midiout_port"; }
private:
	serial_port_device *m_owner;
	required_device<midiout_device> m_midiout;
};

extern const device_type MIDIOUT_PORT;
#endif
