/***************************************************************************

    Covox Speech Thing

***************************************************************************/

#ifndef __CENTRONICS_COVOX_H__
#define __CENTRONICS_COVOX_H__

#include "machine/ctronics.h"
#include "sound/dac.h"

// ======================> centronics_covox_device

class centronics_covox_device :
		public device_t,
		public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_covox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void write(UINT8 data);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "covox"; }
private:
	dac_device *m_dac;
};
// device type definition
extern const device_type CENTRONICS_COVOX;

// ======================> centronics_covox_stereo_device

class centronics_covox_stereo_device :
		public device_t,
		public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_covox_stereo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void write(UINT8 data);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "covox_stereo"; }
private:
	dac_device *m_dac_left;
	dac_device *m_dac_right;
};
// device type definition
extern const device_type CENTRONICS_COVOX_STEREO;

#endif /* __CENTRONICS_COVOX_H__ */
