// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    SPC-1000 VDP expansion unit

***************************************************************************/

#include "emu.h"
#include "vdp.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

WRITE_LINE_MEMBER(spc1000_vdp_exp_device::vdp_interrupt)
{
	// nothing here?
}

static MACHINE_CONFIG_FRAGMENT(scp1000_vdp)

	MCFG_DEVICE_ADD("tms", TMS9928A, XTAL_10_738635MHz / 2) // TODO: which clock?
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(spc1000_vdp_exp_device, vdp_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_NTSC("tms_screen")
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms9928a_device, screen_update)
MACHINE_CONFIG_END

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor spc1000_vdp_exp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( scp1000_vdp );
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SPC1000_VDP_EXP = &device_creator<spc1000_vdp_exp_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spc1000_vdp_exp_device - constructor
//-------------------------------------------------

spc1000_vdp_exp_device::spc1000_vdp_exp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, SPC1000_VDP_EXP, "SPC1000 VDP expansion", tag, owner, clock, "spc1000_vdp_exp", __FILE__),
			device_spc1000_card_interface(mconfig, *this),
			m_vdp(*this, "tms")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_vdp_exp_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_vdp_exp_device::device_reset()
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/
READ8_MEMBER(spc1000_vdp_exp_device::read)
{
	if (!(offset & 0x800))
		return 0xff;

	if (offset & 1)
		return m_vdp->register_read(space, offset);
	else
		return m_vdp->vram_read(space, offset);
}

//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(spc1000_vdp_exp_device::write)
{
	if (offset & 0x800)
	{
		if (offset & 1)
			m_vdp->register_write(space, offset, data);
		else
			m_vdp->vram_write(space, offset, data);
	}
}
