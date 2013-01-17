#ifndef PSXANALOG_H_
#define PSXANALOG_H_

#include "machine/psxcport.h"

extern const device_type PSX_ANALOG_CONTROLLER;

class psx_analog_controller_device :	public device_t,
										public device_psx_controller_interface
{
public:
	psx_analog_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;
	DECLARE_INPUT_CHANGED_MEMBER(change_mode);
protected:
	virtual void device_reset();
	virtual void device_start() {}
	virtual void device_config_complete() { m_shortname = "psx_analog_controller"; }
private:
	virtual bool get_pad(int count, UINT8 *odata, UINT8 idata);
	UINT8 pad_data(int count, bool analog);

	bool m_confmode;
	bool m_analogmode;
	bool m_analoglock;

	UINT8 m_temp;
	UINT8 m_cmd;
};

#endif /* PSXANALOG_H_ */
