// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 Winchester controller card emulation

**********************************************************************/

#include "emu.h"
#include "hdc.h"

#include "softlist_dev.h"

#include <algorithm>



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1010_TAG  "hdc"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TIKI100_HDC, tiki100_hdc_device, "tiki100_hdc", "TIKI-100 Winchester controller")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tiki100_hdc_device::device_add_mconfig(machine_config & config)
{
	WD2010(config, m_hdc, 5000000);
	m_hdc->out_bdrq_callback().set(FUNC(tiki100_hdc_device::bdrq_w));
	m_hdc->out_bcr_callback().set(FUNC(tiki100_hdc_device::bcr_w));
	m_hdc->out_bcs_callback().set(FUNC(tiki100_hdc_device::bcs_w));
	m_hdc->out_wg_callback().set(FUNC(tiki100_hdc_device::wg_w));
	m_hdc->in_drdy_callback().set(FUNC(tiki100_hdc_device::drdy_r));
	m_hdc->in_index_callback().set_constant(1);
	m_hdc->in_wf_callback().set_constant(0);
	m_hdc->in_tk000_callback().set_constant(1);
	m_hdc->in_sc_callback().set_constant(1);

	HARDDISK(config, m_hd[0], "tiki100_hdd");
	HARDDISK(config, m_hd[1], "tiki100_hdd");

	SOFTWARE_LIST(config, "hdd_list").set_original("tiki100_hdd");
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tiki100_hdc_device - constructor
//-------------------------------------------------

tiki100_hdc_device::tiki100_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TIKI100_HDC, tag, owner, clock),
	device_tiki100bus_card_interface(mconfig, *this),
	m_hdc(*this, WD1010_TAG),
	m_hd(*this, "hard%u", 0U),
	m_counter(0),
	m_bdrq(false),
	m_bcs(false),
	m_wg(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiki100_hdc_device::device_start()
{
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);

	save_item(NAME(m_buffer));
	save_item(NAME(m_counter));
	save_item(NAME(m_bdrq));
	save_item(NAME(m_bcs));
	save_item(NAME(m_wg));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tiki100_hdc_device::device_reset()
{
	m_hdc->reset();

	m_counter = 0;
}


harddisk_image_device *tiki100_hdc_device::selected_drive()
{
	harddisk_image_device *hd = nullptr;

	switch ((m_hdc->read(6) >> 3) & 0x03)
	{
	case 2: hd = m_hd[0]; break;
	case 1: hd = m_hd[1]; break;
	}

	return (hd && hd->exists()) ? hd : nullptr;
}


uint32_t tiki100_hdc_device::selected_lba(harddisk_image_device *hd)
{
	const auto &info = hd->get_info();
	uint32_t cylinder = ((m_hdc->read(5) & 0x07) << 8) | m_hdc->read(4);
	uint32_t head = m_hdc->read(6) & 0x07;
	uint32_t sector = m_hdc->read(3);

	return (cylinder * info.heads + head) * info.sectors + sector - 1;
}


int tiki100_hdc_device::sector_size()
{
	static constexpr int SIZES[4] = { 256, 512, 1024, 128 };

	return SIZES[(m_hdc->read(6) >> 5) & 0x03];
}


void tiki100_hdc_device::bdrq_w(int state)
{
	m_bdrq = state;
}


void tiki100_hdc_device::bcr_w(int state)
{
	if (state)
		m_counter = 0;
}


void tiki100_hdc_device::bcs_w(int state)
{
	if (m_bcs && !state && !m_wg)
	{
		harddisk_image_device *hd = selected_drive();

		if (hd && hd->get_info().sectorbytes == sector_size())
			hd->read(selected_lba(hd), m_buffer);

		m_hdc->buffer_ready(true);
	}

	m_bcs = state;
}


void tiki100_hdc_device::wg_w(int state)
{
	if (m_wg && !state)
	{
		harddisk_image_device *hd = selected_drive();

		if (hd && hd->get_info().sectorbytes == sector_size())
			hd->write(selected_lba(hd), m_buffer);

		m_hdc->buffer_ready(false);
	}

	m_wg = state;
}


int tiki100_hdc_device::drdy_r()
{
	return selected_drive() ? 1 : 0;
}


//-------------------------------------------------
//  tiki100bus_iorq_r - I/O read
//-------------------------------------------------

uint8_t tiki100_hdc_device::iorq_r(offs_t offset, uint8_t data)
{
	if ((offset & 0xf8) == 0x20)
	{
		if (offset & 0x07)
		{
			data = m_hdc->read(offset & 0x07);
		}
		else
		{
			data = m_buffer[m_counter];

			if (!machine().side_effects_disabled())
				m_counter = (m_counter + 1) & 0x3ff;
		}
	}

	return data;
}


//-------------------------------------------------
//  tiki100bus_iorq_w - I/O write
//-------------------------------------------------

void tiki100_hdc_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xf8) == 0x20)
	{
		if (offset & 0x07)
		{
			if ((offset & 0x07) == 7)
			{
				m_counter = 0;
				m_hdc->buffer_ready(false);
			}

			m_hdc->write(offset & 0x07, data);
		}
		else
		{
			m_buffer[m_counter] = data;
			m_counter = (m_counter + 1) & 0x3ff;

			if (m_bdrq && m_counter == sector_size())
				m_hdc->buffer_ready(true);
		}
	}
}
