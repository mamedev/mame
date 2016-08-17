// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore IEEE-488 cartridge emulation

**********************************************************************/

#include "ieee488.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MOS6525_TAG     "u3"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_IEEE488 = &device_creator<c64_ieee488_device>;


//-------------------------------------------------
//  tpi6525_interface tpi_intf
//-------------------------------------------------

READ8_MEMBER( c64_ieee488_device::tpi_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2     REN
	    PA3     ATN
	    PA4     DAV
	    PA5     EOI
	    PA6     NDAC
	    PA7     NRFD

	*/

	UINT8 data = 0;

	data |= m_bus->ren_r() << 2;
	data |= m_bus->atn_r() << 3;
	data |= m_bus->dav_r() << 4;
	data |= m_bus->eoi_r() << 5;
	data |= m_bus->ndac_r() << 6;
	data |= m_bus->nrfd_r() << 7;

	return data;
}

WRITE8_MEMBER( c64_ieee488_device::tpi_pa_w )
{
	/*

	    bit     description

	    PA0     U4 DC
	    PA1     U4/U5 TE
	    PA2     REN
	    PA3     ATN
	    PA4     DAV
	    PA5     EOI
	    PA6     NDAC
	    PA7     NRFD

	*/

	m_bus->ren_w(BIT(data, 2));
	m_bus->atn_w(BIT(data, 3));
	m_bus->dav_w(BIT(data, 4));
	m_bus->eoi_w(BIT(data, 5));
	m_bus->ndac_w(BIT(data, 6));
	m_bus->nrfd_w(BIT(data, 7));
}

READ8_MEMBER( c64_ieee488_device::tpi_pc_r )
{
	/*

	    bit     description

	    PC0     IFC
	    PC1     SRQ
	    PC2
	    PC3
	    PC4
	    PC5
	    PC6
	    PC7     slot _EXROM

	*/

	UINT8 data = 0;

	data |= m_bus->ifc_r();
	data |= m_bus->srq_r() << 1;

	data |= m_exp->exrom_r(offset, 1, 1, 1, 0) << 7;

	return data;
}

WRITE8_MEMBER( c64_ieee488_device::tpi_pc_w )
{
	/*

	    bit     description

	    PC0     IFC
	    PC1     SRQ
	    PC2
	    PC3     C64 _EXROM
	    PC4     _ROML select
	    PC5
	    PC6
	    PC7

	*/

	m_bus->ifc_w(BIT(data, 0));
	m_bus->srq_w(BIT(data, 1));

	m_exrom = !BIT(data, 3);

	m_roml_sel = BIT(data, 4);
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_ieee488 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_ieee488 )
	MCFG_DEVICE_ADD(MOS6525_TAG, TPI6525, 0)
	MCFG_TPI6525_IN_PA_CB(READ8(c64_ieee488_device, tpi_pa_r))
	MCFG_TPI6525_OUT_PA_CB(WRITE8(c64_ieee488_device, tpi_pa_w))
	MCFG_TPI6525_IN_PB_CB(DEVREAD8(IEEE488_TAG, ieee488_device, dio_r))
	MCFG_TPI6525_OUT_PB_CB(DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_TPI6525_IN_PC_CB(READ8(c64_ieee488_device, tpi_pc_r))
	MCFG_TPI6525_OUT_PC_CB(WRITE8(c64_ieee488_device, tpi_pc_w))

	MCFG_CBM_IEEE488_ADD(nullptr)
	MCFG_C64_PASSTHRU_EXPANSION_SLOT_ADD()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_ieee488_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_ieee488 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ieee488_device - constructor
//-------------------------------------------------

c64_ieee488_device::c64_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_IEEE488, "IEEE-488", tag, owner, clock, "c64_ieee488", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_tpi(*this, MOS6525_TAG),
	m_bus(*this, IEEE488_TAG),
	m_exp(*this, C64_EXPANSION_SLOT_TAG),
	m_roml_sel(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_ieee488_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_ieee488_device::device_reset()
{
	m_exrom = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_ieee488_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);

	if (!roml && m_roml_sel)
	{
		data = m_roml[offset & 0xfff];
	}
	else if (!io2)
	{
		data = m_tpi->read(space, offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ieee488_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		m_tpi->write(space, offset & 0x07, data);
	}

	m_exp->cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_ieee488_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->game_r(offset, sphi2, ba, rw, m_slot->hiram());
}
