// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    ataintf.c

    ATA Interface implementation.

***************************************************************************/

#include "emu.h"
#include "ataintf.h"

#include "atapicdr.h"
#include "idehd.h"

#include "debugger.h"

void abstract_ata_interface_device::set_default_ata_devices(const char* _master, const char* _slave)
{
	for (size_t slot_index = 0; slot_index < SLOT_COUNT; slot_index++)
	{
		slot(slot_index).option_add("hdd", IDE_HARDDISK);
		slot(slot_index).option_add("cdrom", ATAPI_CDROM);
	}
	slot(SLOT_MASTER).set_default_option(_master);
	slot(SLOT_SLAVE).set_default_option(_slave);
}

ata_slot_device &abstract_ata_interface_device::slot(int index)
{
	assert(index < 2);
	return *subdevice<ata_slot_device>(m_slot[index].finder_tag());
}

void abstract_ata_interface_device::set_irq(int state)
{
//  logerror( "%s: irq %d\n", machine().describe_context(), state );

	m_irq_handler(state);
}

void abstract_ata_interface_device::set_dmarq(int state)
{
//  logerror( "%s: dmarq %d\n", machine().describe_context(), state );

	m_dmarq_handler(state);
}

void abstract_ata_interface_device::set_dasp(int state)
{
//  logerror( "%s: dasp %d\n", machine().describe_context(), state );

	m_dasp_handler(state);
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::irq0_write_line )
{
	if (m_irq[0] != state)
	{
		m_irq[0] = state;

		set_irq(m_irq[0] == ASSERT_LINE || m_irq[1] == ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::irq1_write_line )
{
	if (m_irq[1] != state)
	{
		m_irq[1] = state;

		set_irq(m_irq[0] == ASSERT_LINE || m_irq[1] == ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::dasp0_write_line )
{
	if (m_dasp[0] != state)
	{
		m_dasp[0] = state;

		set_dasp(m_dasp[0] == ASSERT_LINE || m_dasp[1] == ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::dasp1_write_line )
{
	if (m_dasp[1] != state)
	{
		m_dasp[1] = state;

		device_ata_interface *dev = m_slot[0]->dev();
		if (dev != nullptr)
			dev->write_dasp(state);

		set_dasp(m_dasp[0] == ASSERT_LINE || m_dasp[1] == ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::dmarq0_write_line )
{
	if (m_dmarq[0] != state)
	{
		m_dmarq[0] = state;

		set_dmarq(m_dmarq[0] == ASSERT_LINE || m_dmarq[1] == ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::dmarq1_write_line )
{
	if (m_dmarq[1] != state)
	{
		m_dmarq[1] = state;

		set_dmarq(m_dmarq[0] == ASSERT_LINE || m_dmarq[1] == ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::pdiag0_write_line )
{
	m_pdiag[0] = state;
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::pdiag1_write_line )
{
	if (m_pdiag[1] != state)
	{
		m_pdiag[1] = state;

		device_ata_interface *dev = m_slot[0]->dev();
		if (dev != nullptr)
			dev->write_pdiag(state);
	}
}

/*************************************
 *
 *  ATA interface read
 *
 *************************************/

uint16_t abstract_ata_interface_device::read_dma()
{
	uint16_t result = 0xffff;
	for (auto & elem : m_slot)
		if (elem->dev() != nullptr)
			result &= elem->dev()->read_dma();

//  logerror( "%s: read_dma %04x\n", machine().describe_context(), result );
	return result;
}

uint16_t abstract_ata_interface_device::internal_read_cs0(offs_t offset, uint16_t mem_mask)
{
	uint16_t result = mem_mask;
	for (auto & elem : m_slot)
		if (elem->dev() != nullptr)
			result &= elem->dev()->read_cs0(offset, mem_mask);

//  { static int last_status = -1; if (offset == 7 ) { if( result == last_status ) return last_status; last_status = result; } else last_status = -1; }

//  logerror( "%s: read cs0 %04x %04x %04x\n", machine().describe_context(), offset, result, mem_mask );

	return result;
}

uint16_t abstract_ata_interface_device::internal_read_cs1(offs_t offset, uint16_t mem_mask)
{
	uint16_t result = mem_mask;
	for (auto & elem : m_slot)
		if (elem->dev() != nullptr)
			result &= elem->dev()->read_cs1(offset, mem_mask);

//  logerror( "%s: read cs1 %04x %04x %04x\n", machine().describe_context(), offset, result, mem_mask );

	return result;
}

/*************************************
 *
 *  ATA interface write
 *
 *************************************/

void abstract_ata_interface_device::write_dma( uint16_t data )
{
//  logerror( "%s: write_dma %04x\n", machine().describe_context(), data );

	for (auto & elem : m_slot)
		if (elem->dev() != nullptr)
			elem->dev()->write_dma(data);
}

void abstract_ata_interface_device::internal_write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  logerror( "%s: write cs0 %04x %04x %04x\n", machine().describe_context(), offset, data, mem_mask );

	for (auto & elem : m_slot)
		if (elem->dev() != nullptr)
			elem->dev()->write_cs0(offset, data, mem_mask);
}

void abstract_ata_interface_device::internal_write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  logerror( "%s: write cs1 %04x %04x %04x\n", machine().describe_context(), offset, data, mem_mask );

	for (auto & elem : m_slot)
		if (elem->dev() != nullptr)
			elem->dev()->write_cs1(offset, data, mem_mask);
}

WRITE_LINE_MEMBER( abstract_ata_interface_device::write_dmack )
{
//  logerror( "%s: write_dmack %04x\n", machine().describe_context(), state );

	for (auto & elem : m_slot)
		if (elem->dev() != nullptr)
			elem->dev()->write_dmack(state);
}

void ata_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
	device.option_add("cdrom", ATAPI_CDROM);
}

abstract_ata_interface_device::abstract_ata_interface_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_slot(*this, "%u", 0U),
	m_irq_handler(*this),
	m_dmarq_handler(*this),
	m_dasp_handler(*this)
{
}


DEFINE_DEVICE_TYPE(ATA_INTERFACE, ata_interface_device, "ata_interface", "ATA Interface")

ata_interface_device::ata_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abstract_ata_interface_device(mconfig, ATA_INTERFACE, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abstract_ata_interface_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_dmarq_handler.resolve_safe();
	m_dasp_handler.resolve_safe();

	for (int i = 0; i < 2; i++)
	{
		m_irq[i] = 0;
		m_dmarq[i] = 0;
		m_dasp[i] = 0;
		m_pdiag[i] = 0;

		device_ata_interface *dev = m_slot[i]->dev();
		if (dev)
		{
			if (i == 0)
			{
				dev->m_irq_handler.bind().set(*this, FUNC(abstract_ata_interface_device::irq0_write_line));
				dev->m_dmarq_handler.bind().set(*this, FUNC(abstract_ata_interface_device::dmarq0_write_line));
				dev->m_dasp_handler.bind().set(*this, FUNC(abstract_ata_interface_device::dasp0_write_line));
				dev->m_pdiag_handler.bind().set(*this, FUNC(abstract_ata_interface_device::pdiag0_write_line));
			}
			else
			{
				dev->m_irq_handler.bind().set(*this, FUNC(abstract_ata_interface_device::irq1_write_line));
				dev->m_dmarq_handler.bind().set(*this, FUNC(abstract_ata_interface_device::dmarq1_write_line));
				dev->m_dasp_handler.bind().set(*this, FUNC(abstract_ata_interface_device::dasp1_write_line));
				dev->m_pdiag_handler.bind().set(*this, FUNC(abstract_ata_interface_device::pdiag1_write_line));
			}

			dev->write_csel(i);
		}
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abstract_ata_interface_device::device_add_mconfig(machine_config &config)
{
	for (size_t slot = 0; slot < SLOT_COUNT; slot++)
		ATA_SLOT(config, m_slot[slot]);
}


//**************************************************************************
//  ATA SLOT DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ATA_SLOT, ata_slot_device, "ata_slot", "ATA Connector")

//-------------------------------------------------
//  ata_slot_device - constructor
//-------------------------------------------------

ata_slot_device::ata_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ATA_SLOT, tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_dev(nullptr)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ata_slot_device::device_config_complete()
{
	m_dev = dynamic_cast<device_ata_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ata_slot_device::device_start()
{
}
