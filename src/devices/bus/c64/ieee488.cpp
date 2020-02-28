// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore IEEE-488 cartridge emulation

**********************************************************************/

#include "emu.h"
#include "ieee488.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MOS6525_TAG     "u3"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_IEEE488, c64_ieee488_device, "c64_ieee488", "C64 IEEE-488 cartridge")


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

	uint8_t data = 0;

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

	m_bus->host_ren_w(BIT(data, 2));
	m_bus->host_atn_w(BIT(data, 3));
	m_bus->host_dav_w(BIT(data, 4));
	m_bus->host_eoi_w(BIT(data, 5));
	m_bus->host_ndac_w(BIT(data, 6));
	m_bus->host_nrfd_w(BIT(data, 7));
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

	uint8_t data = 0;

	data |= m_bus->ifc_r();
	data |= m_bus->srq_r() << 1;

	data |= m_exp->exrom_r(offset, 1, 1, 1, 0, 0) << 7;

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

	m_bus->host_ifc_w(BIT(data, 0));
	m_bus->host_srq_w(BIT(data, 1));

	m_exrom = !BIT(data, 3);

	m_roml_sel = BIT(data, 4);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_ieee488_device::device_add_mconfig(machine_config &config)
{
	TPI6525(config, m_tpi, 0);
	m_tpi->in_pa_cb().set(FUNC(c64_ieee488_device::tpi_pa_r));
	m_tpi->out_pa_cb().set(FUNC(c64_ieee488_device::tpi_pa_w));
	m_tpi->in_pb_cb().set(m_bus, FUNC(ieee488_device::dio_r));
	m_tpi->out_pb_cb().set(m_bus, FUNC(ieee488_device::host_dio_w));
	m_tpi->in_pc_cb().set(FUNC(c64_ieee488_device::tpi_pc_r));
	m_tpi->out_pc_cb().set(FUNC(c64_ieee488_device::tpi_pc_w));

	IEEE488(config, m_bus, 0);
	ieee488_slot_device::add_cbm_defaults(config, nullptr);

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ieee488_device - constructor
//-------------------------------------------------

c64_ieee488_device::c64_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_IEEE488, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_tpi(*this, MOS6525_TAG),
	m_bus(*this, IEEE488_TAG),
	m_exp(*this, "exp"),
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

uint8_t c64_ieee488_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);

	if (!roml && m_roml_sel)
	{
		data = m_roml[offset & 0xfff];
	}
	else if (!io2)
	{
		data = m_tpi->read(offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ieee488_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		m_tpi->write(offset & 0x07, data);
	}

	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_ieee488_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}
