// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-801 Disk Controller

***************************************************************************/

#include "sv801.h"
#include "softlist.h"
#include "formats/svi_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV801 = &device_creator<sv801_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( sv801_device::floppy_formats )
	FLOPPY_SVI_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( svi_floppies )
	SLOT_INTERFACE("dd", FLOPPY_525_DD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( sv801 )
	MCFG_FD1793_ADD("fdc", XTAL_8MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(sv801_device, intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(sv801_device, drq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", svi_floppies, "dd", sv801_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", svi_floppies, "dd", sv801_device::floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("disk_list", "svi318_flop")
MACHINE_CONFIG_END

machine_config_constructor sv801_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sv801 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv801_device - constructor
//-------------------------------------------------

sv801_device::sv801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SV801, "SV-801 Disk Controller", tag, owner, clock, "sv801", __FILE__),
	device_svi_slot_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_floppy(nullptr),
	m_irq(0), m_drq(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv801_device::device_start()
{
	// register for savestates
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sv801_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( sv801_device::intrq_w )
{
	m_irq = state;
}

WRITE_LINE_MEMBER( sv801_device::drq_w )
{
	m_drq = state;
}

WRITE8_MEMBER( sv801_device::motor_w )
{
	m_floppy = nullptr;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy0->get_device())
		m_floppy0->get_device()->mon_w(!BIT(data, 2));
	if (m_floppy1->get_device())
		m_floppy1->get_device()->mon_w(!BIT(data, 3));
}

READ8_MEMBER( sv801_device::iorq_r )
{
	switch (offset)
	{
	case 0x30: return m_fdc->status_r(space, 0);
	case 0x31: return m_fdc->track_r(space, 0);
	case 0x32: return m_fdc->sector_r(space, 0);
	case 0x33: return m_fdc->data_r(space, 0);
	case 0x34: return (m_drq << 6) | (m_irq << 7);
	}

	return 0xff;
}

WRITE8_MEMBER( sv801_device::iorq_w )
{
	switch (offset)
	{
	case 0x30: m_fdc->cmd_w(space, 0, data); break;
	case 0x31: m_fdc->track_w(space, 0, data); break;
	case 0x32: m_fdc->sector_w(space, 0, data); break;
	case 0x33: m_fdc->data_w(space, 0, data); break;
	case 0x34: motor_w(space, 0, data); break;
	case 0x38:
		m_fdc->dden_w(BIT(data, 0));
		if (m_floppy)
			m_floppy->ss_w(BIT(data, 1));
		break;
	}
}
