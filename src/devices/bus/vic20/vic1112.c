// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1112 IEEE-488 Interface Cartridge emulation

**********************************************************************/

#include "vic1112.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6522_0_TAG     "u4"
#define M6522_1_TAG     "u5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1112 = &device_creator<vic1112_device>;


WRITE_LINE_MEMBER( vic1112_device::via0_irq_w )
{
	m_via0_irq = state;

	m_slot->irq_w(m_via0_irq | m_via1_irq);
}

READ8_MEMBER( vic1112_device::via0_pb_r )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3     _EOI
	    PB4     _DAV IN
	    PB5     _NRFD IN
	    PB6     _NDAC IN
	    PB7     _ATN IN

	*/

	UINT8 data = 0;

	data |= m_bus->eoi_r() << 3;
	data |= m_bus->dav_r() << 4;
	data |= m_bus->nrfd_r() << 5;
	data |= m_bus->ndac_r() << 6;
	data |= m_bus->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( vic1112_device::via0_pb_w )
{
	/*

	    bit     description

	    PB0     _DAV OUT
	    PB1     _NRFD OUT
	    PB2     _NDAC OUT
	    PB3
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	m_bus->dav_w(BIT(data, 0));
	m_bus->nrfd_w(BIT(data, 1));
	m_bus->ndac_w(BIT(data, 2));
}


WRITE_LINE_MEMBER( vic1112_device::via1_irq_w )
{
	m_via1_irq = state;

	m_slot->irq_w(m_via0_irq | m_via1_irq);
}


//-------------------------------------------------
//  MACHINE_DRIVER( vic1112 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vic1112 )
	MCFG_DEVICE_ADD(M6522_0_TAG, VIA6522, 0)
	MCFG_VIA6522_READPB_HANDLER(READ8(vic1112_device, via0_pb_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(vic1112_device, via0_pb_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(vic1112_device, via0_irq_w))

	MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, 0)
	MCFG_VIA6522_READPB_HANDLER(DEVREAD8(IEEE488_TAG, ieee488_device, dio_r))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, atn_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, eoi_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(vic1112_device, via1_irq_w))

	MCFG_CBM_IEEE488_ADD(NULL)
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE(M6522_1_TAG, via6522_device, write_cb1))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vic1112_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vic1112 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1112_device - constructor
//-------------------------------------------------

vic1112_device::vic1112_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIC1112, "VIC1112", tag, owner, clock, "vic1112", __FILE__),
		device_vic20_expansion_card_interface(mconfig, *this),
		m_via0(*this, M6522_0_TAG),
		m_via1(*this, M6522_1_TAG),
		m_bus(*this, IEEE488_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1112_device::device_start()
{
	// state saving
	save_item(NAME(m_via0_irq));
	save_item(NAME(m_via1_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic1112_device::device_reset()
{
	m_bus->ifc_w(0);
	m_bus->ifc_w(1);
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic1112_device::vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!io2)
	{
		if (offset & 0x10)
		{
			data = m_via1->read(space, offset & 0x0f);
		}
		else
		{
			data = m_via0->read(space, offset & 0x0f);
		}
	}
	else if (!blk5)
	{
		if (offset & 0x1000)
		{
			data = m_blk5[offset & 0x17ff];
		}
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic1112_device::vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!io2)
	{
		if (offset & 0x10)
		{
			m_via1->write(space, offset & 0x0f, data);
		}
		else
		{
			m_via0->write(space, offset & 0x0f, data);
		}
	}
}
