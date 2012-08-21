/***************************************************************************

    Covox Speech Thing

***************************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "cntr_covox.h"

//**************************************************************************
//  COVOX DEVICE
//**************************************************************************

// device type definition
const device_type CENTRONICS_COVOX = &device_creator<centronics_covox_device>;

static MACHINE_CONFIG_FRAGMENT( covox )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_covox_device - constructor
//-------------------------------------------------

centronics_covox_device::centronics_covox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, CENTRONICS_COVOX, "Covox Speech Thing", tag, owner, clock),
	  device_centronics_peripheral_interface( mconfig, *this )
{
}
//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor centronics_covox_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( covox );
}


void centronics_covox_device::device_start()
{
	m_dac = subdevice<dac_device>("dac");
}

void centronics_covox_device::write(UINT8 data)
{
	m_dac->write_unsigned8(data);
}

//**************************************************************************
//  COVOX STEREO DEVICE
//**************************************************************************

// device type definition
const device_type CENTRONICS_COVOX_STEREO = &device_creator<centronics_covox_stereo_device>;

static MACHINE_CONFIG_FRAGMENT( covox_stereo )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac_left", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_SOUND_ADD("dac_right", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_covox_stereo_device - constructor
//-------------------------------------------------

centronics_covox_stereo_device::centronics_covox_stereo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, CENTRONICS_COVOX_STEREO, "Covox (Stereo-in-1)", tag, owner, clock),
	  device_centronics_peripheral_interface( mconfig, *this )
{
}
//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor centronics_covox_stereo_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( covox_stereo );
}


void centronics_covox_stereo_device::device_start()
{
	m_dac_left  = subdevice<dac_device>("dac_left");
	m_dac_right = subdevice<dac_device>("dac_right");
}

void centronics_covox_stereo_device::write(UINT8 data)
{
	if (m_strobe)  m_dac_left->write_unsigned8(data);
	if (m_auto_fd) m_dac_right->write_unsigned8(data);
}
