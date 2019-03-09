// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-801 Disk Controller

***************************************************************************/

#include "emu.h"
#include "sv801.h"
#include "softlist.h"
#include "formats/svi_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SV801, sv801_device, "sv801", "SV-801 Disk Controller")

FLOPPY_FORMATS_MEMBER( sv801_device::floppy_formats )
	FLOPPY_SVI_FORMAT
FLOPPY_FORMATS_END

static void svi_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_525_DD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sv801_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(sv801_device::intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(sv801_device::drq_w));

	FLOPPY_CONNECTOR(config, m_floppy0, svi_floppies, "dd", sv801_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, svi_floppies, "dd", sv801_device::floppy_formats);

	SOFTWARE_LIST(config, "disk_list").set_original("svi318_flop");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv801_device - constructor
//-------------------------------------------------

sv801_device::sv801_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV801, tag, owner, clock),
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

void sv801_device::motor_w(uint8_t data)
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

uint8_t sv801_device::iorq_r(offs_t offset)
{
	switch (offset)
	{
	case 0x30: return m_fdc->status_r();
	case 0x31: return m_fdc->track_r();
	case 0x32: return m_fdc->sector_r();
	case 0x33: return m_fdc->data_r();
	case 0x34: return (m_drq << 6) | (m_irq << 7);
	}

	return 0xff;
}

void sv801_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x30: m_fdc->cmd_w(data); break;
	case 0x31: m_fdc->track_w(data); break;
	case 0x32: m_fdc->sector_w(data); break;
	case 0x33: m_fdc->data_w(data); break;
	case 0x34: motor_w(data); break;
	case 0x38:
		m_fdc->dden_w(BIT(data, 0));
		if (m_floppy)
			m_floppy->ss_w(BIT(data, 1));
		break;
	}
}
