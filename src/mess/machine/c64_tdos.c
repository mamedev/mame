/**********************************************************************

    Triton QD TDOS cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    PCB Layout
    ----------

    XM-2206-A (top)
    XM-2205-A (bottom)

                |===========================|
                |            CN4            |
                |                           |
                |      ULA                  |
                |                           |
                |      6.5MHz               |
                |                           |
                |      SSDA                 |
                |                 LS175     |
                |                 LS367     |
                |            CN3            |
                |=========|||||||||=========|
                          |||||||||
            |=============|||||||||============|
            |                CN2               |
            |       LS00     LS02     LS138    |
    |=======|                                  |
    |=|             LS245        ROM           |
    |=|                                        |
    |=|                                        |
    |=|                                     CN1|
    |=|                                        |
    |=|                                        |
    |=|                                        |
    |=|                             SW1        |
    |==========================================|

    ROM    - Hitachi HN482764G 8Kx8 EPROM "TDOS 1.2"
    ULA    - Ferranti ULA5RB073E1 XZ-2085-1 custom (?)
    SSDA   - Motorola MC68A52P SSDA
    CN1    - C64 expansion connector (pass-thru)
    CN2,3  - 19x1 flat ribbon cable to other PCB
    CN4    - 9 wires to 3" drive
    SW1    - cartridge on/off switch

*/

#include "c64_tdos.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC68A52P_TAG		"mc6852"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_TDOS = &device_creator<c64_tdos_cartridge_device>;


//-------------------------------------------------
//  MC6852_INTERFACE( ssda_intf )
//-------------------------------------------------

static MC6852_INTERFACE( ssda_intf )
{
	0,
	0,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  floppy_interface tdos_floppy_interface
//-------------------------------------------------

static const floppy_interface tdos_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	"floppy_2_8",
	NULL
};


//-------------------------------------------------
//  C64_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

READ8_MEMBER( c64_tdos_cartridge_device::dma_cd_r )
{
	return m_slot->dma_cd_r(offset);
}

WRITE8_MEMBER( c64_tdos_cartridge_device::dma_cd_w )
{
	m_slot->dma_cd_w(offset, data);
}

WRITE_LINE_MEMBER( c64_tdos_cartridge_device::irq_w )
{
	m_slot->irq_w(state);
}

WRITE_LINE_MEMBER( c64_tdos_cartridge_device::nmi_w )
{
	m_slot->nmi_w(state);
}

WRITE_LINE_MEMBER( c64_tdos_cartridge_device::dma_w )
{
	m_slot->dma_w(state);
}

WRITE_LINE_MEMBER( c64_tdos_cartridge_device::reset_w )
{
	m_slot->reset_w(state);
}

static C64_EXPANSION_INTERFACE( expansion_intf )
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_tdos_cartridge_device, dma_cd_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_tdos_cartridge_device, dma_cd_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_tdos_cartridge_device, irq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_tdos_cartridge_device, nmi_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_tdos_cartridge_device, dma_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_tdos_cartridge_device, reset_w)
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_multiscreen )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_tdos )
	MCFG_MC6852_ADD(MC68A52P_TAG, XTAL_6_5MHz, ssda_intf)
	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, tdos_floppy_interface)

	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, 0, expansion_intf, c64_expansion_cards, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_tdos_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_tdos );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_tdos_cartridge_device - constructor
//-------------------------------------------------

c64_tdos_cartridge_device::c64_tdos_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_TDOS, "C64 TDOS cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ssda(*this, MC68A52P_TAG),
    m_exp(*this, C64_EXPANSION_SLOT_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_tdos_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_tdos_cartridge_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_tdos_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);

	// TODO

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_tdos_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	// TODO
	m_exp->cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_tdos_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return m_exp->game_r(offset, sphi2, ba, rw, hiram);
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_tdos_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, hiram);
}
