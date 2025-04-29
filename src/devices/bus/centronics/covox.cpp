// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Covox Speech Thing

    Notes:
    - "Jast Sound" is very similar conceptually to a Covox Speech Thing.
      https://www.generation-msx.nl/hardware/jast/jast-sound/1638/

***************************************************************************/

#include "emu.h"
#include "covox.h"
#include "speaker.h"

//**************************************************************************
//  COVOX DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CENTRONICS_COVOX, centronics_covox_device, "covox", "Covox Speech Thing")


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_covox_device - constructor
//-------------------------------------------------

centronics_covox_device::centronics_covox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CENTRONICS_COVOX, tag, owner, clock),
	device_centronics_peripheral_interface( mconfig, *this ),
	m_dac(*this, "dac"),
	m_data(0)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void centronics_covox_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

void centronics_covox_device::device_start()
{
	save_item(NAME(m_data));
}

void centronics_covox_device::update_dac()
{
	if (started())
		m_dac->write(m_data);
}

//**************************************************************************
//  COVOX STEREO DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CENTRONICS_COVOX_STEREO, centronics_covox_stereo_device, "covox_stereo", "Covox (Stereo-in-1)")


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_covox_stereo_device - constructor
//-------------------------------------------------

centronics_covox_stereo_device::centronics_covox_stereo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CENTRONICS_COVOX_STEREO, tag, owner, clock),
	device_centronics_peripheral_interface( mconfig, *this ),
	m_ldac(*this, "ldac"),
	m_rdac(*this, "rdac"),
	m_strobe(0),
	m_data(0),
	m_autofd(0)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void centronics_covox_stereo_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
	DAC_8BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 0); // unknown DAC
	DAC_8BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1); // unknown DAC
}

void centronics_covox_stereo_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_strobe));
	save_item(NAME(m_autofd));
}

void centronics_covox_stereo_device::update_dac()
{
	if (started())
	{
		if (m_strobe)
			m_ldac->write(m_data);

		if (m_autofd)
			m_rdac->write(m_data);
	}
}
