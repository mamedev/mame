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

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

MACHINE_CONFIG_MEMBER(spc1000_vdp_exp_device::device_add_mconfig)

	MCFG_DEVICE_ADD("tms", TMS9928A, XTAL_10_738635MHz / 2) // TODO: which clock?
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(spc1000_vdp_exp_device, vdp_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_NTSC("tms_screen")
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms9928a_device, screen_update)
MACHINE_CONFIG_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SPC1000_VDP_EXP, spc1000_vdp_exp_device, "spc1000_vdp_exp", "SPC1000 VDP expansion")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spc1000_vdp_exp_device - constructor
//-------------------------------------------------

spc1000_vdp_exp_device::spc1000_vdp_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPC1000_VDP_EXP, tag, owner, clock)
	, device_spc1000_card_interface(mconfig, *this)
	, m_vdp(*this, "tms")
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
