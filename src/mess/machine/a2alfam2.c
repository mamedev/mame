/*********************************************************************

    a2alfsm2.c

    Implementation of the ALF Apple Music II card

*********************************************************************/

#include "a2alfam2.h"
#include "includes/apple2.h"
#include "sound/sn76496.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_ALFAM2 = &device_creator<a2bus_alfam2_device>;

#define SN1_TAG         "sn76489_1" // left
#define SN2_TAG         "sn76489_2" // center
#define SN3_TAG         "sn76489_3" // right

//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
    DEVCB_NULL
};

MACHINE_CONFIG_FRAGMENT( a2alfam2 )
    MCFG_SPEAKER_STANDARD_STEREO("alf_l", "alf_r")
    MCFG_SOUND_ADD(SN1_TAG, SN76489_NEW, XTAL_3_579545MHz)
	MCFG_SOUND_CONFIG(psg_intf)
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "alf_l", 0.50)
    MCFG_SOUND_ADD(SN2_TAG, SN76489_NEW, XTAL_3_579545MHz)
	MCFG_SOUND_CONFIG(psg_intf)
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "alf_l", 0.50)
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "alf_r", 0.50)
    MCFG_SOUND_ADD(SN3_TAG, SN76489_NEW, XTAL_3_579545MHz)
	MCFG_SOUND_CONFIG(psg_intf)
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "alf_r", 0.50)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_alfam2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2alfam2 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_alfam2_device::a2bus_alfam2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, type, name, tag, owner, clock),
    device_a2bus_card_interface(mconfig, *this),
    m_sn1(*this, SN1_TAG),
    m_sn2(*this, SN2_TAG),
    m_sn3(*this, SN3_TAG)
{
	m_shortname = "a2alfam2";
}

a2bus_alfam2_device::a2bus_alfam2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
    device_t(mconfig, A2BUS_ALFAM2, "ALF Apple Music II", tag, owner, clock),
    device_a2bus_card_interface(mconfig, *this),
    m_sn1(*this, SN1_TAG),
    m_sn2(*this, SN2_TAG),
    m_sn3(*this, SN3_TAG)
{
	m_shortname = "a2alfam2";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_alfam2_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_alfam2_device::device_reset()
{
}

void a2bus_alfam2_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
    switch (offset)
    {
        case 0:
            m_sn1->write(space, 0, data);
            break;

        case 1:
            m_sn2->write(space, 0, data);
            break;

        case 2:
            m_sn3->write(space, 0, data);
            break;
    }
}

bool a2bus_alfam2_device::take_c800()
{
    return false;
}

