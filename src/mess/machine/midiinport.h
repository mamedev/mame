/*********************************************************************

    midiinport.h

    MIDI In serial port - glues the image device to the pluggable serial port

*********************************************************************/

#ifndef _MIDIINPORT_H_
#define _MIDIINPORT_H_

#include "emu.h"
#include "machine/serial.h"
#include "imagedev/midiin.h"

class midiin_port_device :
		public device_t,
		public device_serial_port_interface
{
public:
	midiin_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
	DECLARE_WRITE_LINE_MEMBER( read ) { m_owner->out_rx(state); }
	virtual void tx(UINT8 state) { }
protected:
	virtual void device_start() { m_owner = dynamic_cast<serial_port_device *>(owner()); }
	virtual void device_reset() { }
	virtual void device_config_complete() { m_shortname = "midiin_port"; }
private:
	serial_port_device *m_owner;
	required_device<midiin_device> m_midiin;
};

extern const device_type MIDIIN_PORT;
#endif
