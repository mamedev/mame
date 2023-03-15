// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-104 Memory Expander Plus emulation

**********************************************************************/

#include "emu.h"
#include "hpc104.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(POFO_HPC104,   pofo_hpc104_device,   "pofo_hpc104",   "Atari Portfolio HPC-104")
DEFINE_DEVICE_TYPE(POFO_HPC104_2, pofo_hpc104_2_device, "pofo_hpc104_2", "Atari Portfolio HPC-104 (Unit 2)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pofo_hpc104_device::device_add_mconfig(machine_config &config)
{
	PORTFOLIO_MEMORY_CARD_SLOT(config, m_ccm, portfolio_memory_cards, nullptr);

	PORTFOLIO_EXPANSION_SLOT(config, m_exp, XTAL(4'915'200), portfolio_expansion_cards, nullptr);
	m_exp->eint_wr_callback().set(DEVICE_SELF_OWNER, FUNC(portfolio_expansion_slot_device::eint_w));
	m_exp->nmio_wr_callback().set(DEVICE_SELF_OWNER, FUNC(portfolio_expansion_slot_device::nmio_w));
	m_exp->wake_wr_callback().set(DEVICE_SELF_OWNER, FUNC(portfolio_expansion_slot_device::wake_w));
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

ioport_constructor pofo_hpc104_device::device_input_ports() const
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

ioport_constructor pofo_hpc104_2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hpc104_2 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pofo_hpc104_device - constructor
//-------------------------------------------------

pofo_hpc104_device::pofo_hpc104_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_portfolio_expansion_slot_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_ccm(*this, PORTFOLIO_MEMORY_CARD_SLOT_B_TAG),
	m_exp(*this, "exp"),
	m_nvram(*this, "nvram", 0x40000, ENDIANNESS_LITTLE),
	m_io_sw1(*this, "SW1")
{
}

pofo_hpc104_device::pofo_hpc104_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pofo_hpc104_device(mconfig, POFO_HPC104, tag, owner, clock)
{
}


//-------------------------------------------------
//  pofo_hpc104_2_device - constructor
//-------------------------------------------------

pofo_hpc104_2_device::pofo_hpc104_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pofo_hpc104_device(mconfig, POFO_HPC104_2, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pofo_hpc104_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pofo_hpc104_device::device_reset()
{
	m_sw1 = BIT(m_io_sw1->read(), 0);
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

uint8_t pofo_hpc104_device::nrdi_r(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1)
{
	data = m_exp->nrdi_r(offset, data, iom, bcom, m_ncc1_out || ncc1);

	if (!iom)
	{
		if (!(!m_ncc1_out || ncc1))
		{
			data = m_ccm->nrdi_r(offset & 0x1ffff);

			if (LOG) logerror("%s %s CCM1 read %05x:%02x\n", machine().time().as_string(), machine().describe_context(), offset & 0x1ffff, data);
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
				data = m_nvram[offset - 0x1f000];
			}
		}
	}

	return data;
}


//-------------------------------------------------
//  nwri_w - write
//-------------------------------------------------

void pofo_hpc104_device::nwri_w(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1)
{
	m_exp->nwri_w(offset, data, iom, bcom, m_ncc1_out || ncc1);

	if (!iom)
	{
		if (!(!m_ncc1_out || ncc1))
		{
			if (LOG) logerror("%s %s CCM1 write %05x:%02x\n", machine().time().as_string(), machine().describe_context(), offset & 0x1ffff, data);

			m_ccm->nwri_w(offset & 0x1ffff, data);
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
	else
	{
		if (!bcom)
		{
			if ((offset & 0x0f) == 0x0c)
			{
				m_ncc1_out = BIT(data, 0);

				if (LOG) logerror("%s %s NCC1 out %u\n", machine().time().as_string(), machine().describe_context(), m_ncc1_out);
			}
		}
	}
}
