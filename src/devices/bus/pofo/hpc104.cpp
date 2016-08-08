// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-104 Memory Expander Plus emulation

**********************************************************************/

#include "hpc104.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type HPC104 = &device_creator<hpc104_t>;
const device_type HPC104_2 = &device_creator<hpc104_2_t>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( hpc104 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( hpc104 )
	MCFG_PORTFOLIO_MEMORY_CARD_SLOT_ADD(PORTFOLIO_MEMORY_CARD_SLOT_B_TAG, portfolio_memory_cards, nullptr)

	MCFG_PORTFOLIO_EXPANSION_SLOT_ADD(PORTFOLIO_EXPANSION_SLOT_TAG, XTAL_4_9152MHz, portfolio_expansion_cards, nullptr)
	MCFG_PORTFOLIO_EXPANSION_SLOT_IINT_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, portfolio_expansion_slot_t, iint_w))
	MCFG_PORTFOLIO_EXPANSION_SLOT_EINT_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, portfolio_expansion_slot_t, eint_w))
	MCFG_PORTFOLIO_EXPANSION_SLOT_NMIO_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, portfolio_expansion_slot_t, nmio_w))
	MCFG_PORTFOLIO_EXPANSION_SLOT_WAKE_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, portfolio_expansion_slot_t, wake_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor hpc104_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hpc104 );
}


//-------------------------------------------------
//  INPUT_PORTS( hpc104 )
//-------------------------------------------------

static INPUT_PORTS_START( hpc104 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Unit Number" )
	PORT_DIPSETTING(    0x00, "1 (0x1F000)" )
	PORT_DIPSETTING(    0x01, "2 (0x5F000)" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor hpc104_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( hpc104 );
}


//-------------------------------------------------
//  INPUT_PORTS( hpc104_2 )
//-------------------------------------------------

static INPUT_PORTS_START( hpc104_2 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Unit Number" )
	PORT_DIPSETTING(    0x00, "1 (0x1F000)" )
	PORT_DIPSETTING(    0x01, "2 (0x5F000)" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor hpc104_2_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( hpc104_2 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hpc104_t - constructor
//-------------------------------------------------

hpc104_t::hpc104_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_portfolio_expansion_slot_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_ccm(*this, PORTFOLIO_MEMORY_CARD_SLOT_B_TAG),
	m_exp(*this, PORTFOLIO_EXPANSION_SLOT_TAG),
	m_nvram(*this, "nvram"),
	m_io_sw1(*this, "SW1")
{
}

hpc104_t::hpc104_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, HPC104, "Atari Portfolio HPC-104", tag, owner, clock, "hpc104", __FILE__),
	device_portfolio_expansion_slot_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_ccm(*this, PORTFOLIO_MEMORY_CARD_SLOT_B_TAG),
	m_exp(*this, PORTFOLIO_EXPANSION_SLOT_TAG),
	m_nvram(*this, "nvram"),
	m_io_sw1(*this, "SW1")
{
}


//-------------------------------------------------
//  hpc104_2_t - constructor
//-------------------------------------------------

hpc104_2_t::hpc104_2_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	hpc104_t(mconfig, HPC104_2, "Atari Portfolio HPC-104 (Unit 2)", tag, owner, clock, "hpc104_2", __FILE__) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hpc104_t::device_start()
{
	// allocate memory
	m_nvram.allocate(0x40000);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hpc104_t::device_reset()
{
	m_sw1 = BIT(m_io_sw1->read(), 0);
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

UINT8 hpc104_t::nrdi_r(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1)
{
	data = m_exp->nrdi_r(space, offset, data, iom, bcom, m_ncc1_out || ncc1);

	if (!(!m_ncc1_out || ncc1))
	{
		data = m_ccm->nrdi_r(space, offset & 0x1ffff);
	}

	if (m_sw1)
	{
		if (offset >= 0x5f000 && offset < 0x9f000)
		{
			data = m_nvram[offset - 0x5f000];
		}
	}
	else
	{
		if (offset >= 0x1f000 && offset < 0x5f000)
		{
			data = m_nvram[offset - 0x1f000] = data;
		}
	}

	return data;
}


//-------------------------------------------------
//  nwri_w - write
//-------------------------------------------------

void hpc104_t::nwri_w(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1)
{
	m_exp->nwri_w(space, offset, data, iom, bcom, m_ncc1_out || ncc1);

	if (!bcom)
	{
		if ((offset & 0x0f) == 0x0c)
		{
			m_ncc1_out = BIT(data, 0);
		}
	}

	if (!(!m_ncc1_out || ncc1))
	{
		m_ccm->nwri_w(space, offset & 0x1ffff, data);
	}

	if (m_sw1)
	{
		if (offset >= 0x5f000 && offset < 0x9f000)
		{
			m_nvram[offset - 0x5f000] = data;
		}
	}
	else
	{
		if (offset >= 0x1f000 && offset < 0x5f000)
		{
			m_nvram[offset - 0x1f000] = data;	
		}
	}
}
