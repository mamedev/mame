// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/****************************************************************************

    Psion Solid State Disk emulation


    Memory Type                           No. devices       Memory Size
    D7 D6 D5                              D4 D3             D2 D1 D0
    0  0  0  RAM                          0  0  1 device    0  0  0  No memory
    0  0  1  Type 1 Flash                 0  1  2 devices   0  0  1  32K
    0  1  0  Type 2 Flash                 1  0  3 devices   0  1  0  64K
    0  1  1  Type 3 Flash                 1  1  4 devices   0  1  1  128K
    1  0  0  Type 4 Flash                                   1  0  0  256K
    1  0  1  Type 5 Flash                                   1  0  1  512K
    1  1  0  Read only SSD (ROM)                            1  1  0  1M
    1  1  1  Hardware Write protected SSD                   1  1  1  2M

    Info Byte (* = confirmed)
    Psion Solid State Disk 64K Flash    001 01 001 0x29
    Psion Solid State Disk 128K Flash * 001 00 011 0x23
    - MC200 System Disk               * 111 00 011 0xE3
    - MC400 System Disk               * 111 00 011 0xE3
    Psion Solid State Disk 256K Flash * 001 01 011 0x2B
    - MC Word System Disk             * 111 01 011 0xEB
    Psion Solid State Disk 512K Flash   001 01 100 0x2C
    Psion Solid State Disk 1MB Flash  * 001 11 100 0x3C
    Psion Solid State Disk 2MB Flash  * 001 11 101 0x3D
    Psion Solid State Disk 4MB Flash    001 11 110 0x3E
    Psion Solid State Disk 8MB Flash  * 001 11 111 0x3F

    Psion Solid State Disk 64K RAM      000 01 001 0x09
    Psion Solid State Disk 128K RAM     000 11 001 0x19
    Psion Solid State Disk 512K RAM     000 11 011 0x1B
    Psion Solid State Disk 1MB RAM      000 11 100 0x1C
    Psion Solid State Disk 2MB RAM      000 11 101 0x1D

****************************************************************************/

#include "emu.h"
#include "psion_ssd.h"

#include "softlist_dev.h"

#include <tuple>


DEFINE_DEVICE_TYPE(PSION_SSD, psion_ssd_device, "psion_ssd", "Psion Solid State Disk")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void psion_ssd_device::device_add_mconfig(machine_config &config)
{
	PSION_ASIC5(config, m_asic5, DERIVED_CLOCK(1, 1)).set_mode(psion_asic5_device::PACK_MODE);
	m_asic5->readpa_handler().set([this]() { return m_ssd_data[latched_addr()]; });
	m_asic5->writepa_handler().set([this](uint8_t data) { m_ssd_data[latched_addr()] = data; });
	m_asic5->writepb_handler().set([this](uint8_t data) { m_port_latch = (m_port_latch & 0xffff00) | (data << 0); });
	m_asic5->writepd_handler().set([this](uint8_t data) { m_port_latch = (m_port_latch & 0xff00ff) | (data << 8); });
	m_asic5->writepc_handler().set([this](uint8_t data) { m_port_latch = (m_port_latch & 0x00ffff) | (data << 16); });
}


//-------------------------------------------------
//  psion_ssd_device - constructor
//-------------------------------------------------

psion_ssd_device::psion_ssd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_SSD, tag, owner, clock)
	, device_memcard_image_interface(mconfig, *this)
	, m_region(*this, DEVICE_SELF)
	, m_asic5(*this, "asic5")
	, m_door_cb(*this)
	, m_door_timer(nullptr)
	, m_info_byte(0)
	, m_port_latch(0)
	, m_mem_width(0)
{
}


//-------------------------------------------------
//  psion_ssd_device - destructor
//-------------------------------------------------

psion_ssd_device::~psion_ssd_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_ssd_device::device_start()
{
	m_ssd_data = make_unique_clear<uint8_t[]>(0x800000);

	// insert default System disk
	if (m_region.found())
	{
		uint32_t size = m_region->bytes();
		memcpy(&m_ssd_data[0], m_region->base(), size);
		set_info_byte(size, SSD_FLASH1);
	}

	m_door_timer = timer_alloc(FUNC(psion_ssd_device::close_door), this);
	m_door_timer->reset();

	save_pointer(NAME(m_ssd_data), 0x800000);
	save_item(NAME(m_info_byte));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_ssd_device::device_reset()
{
	m_port_latch = 0x00;

	// open the door
	m_door_cb(ASSERT_LINE);

	// setup the timer to close the door
	m_door_timer->adjust(attotime::from_msec(200));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void psion_ssd_device::device_config_complete()
{
	add_format("ssd", "Psion Solid State Disk image", "bin,rom", "");
}


//-------------------------------------------------
//  get_software_list_loader -
//-------------------------------------------------

const software_list_loader &psion_ssd_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}


TIMER_CALLBACK_MEMBER(psion_ssd_device::close_door)
{
	// close the door
	m_door_cb(CLEAR_LINE);
}


std::pair<std::error_condition, std::string> psion_ssd_device::call_load()
{
	uint32_t const size = length();

	if (size < 0x10000 || size > 0x800000 || (size & (size - 1)) != 0)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size, must be 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M");

	std::error_condition err;
	size_t actual;
	std::tie(err, m_ssd_data, actual) = read(image_core_file(), size);
	if (err || (actual != size))
		return std::make_pair(err ? err : std::errc::io_error, std::string());

	// check for Flash header
	if ((m_ssd_data[0] | (m_ssd_data[1] << 8)) == 0xf1a5) // Flash
		set_info_byte(size, SSD_FLASH1);
	else
		set_info_byte(size, SSD_RAM);

	if (loaded_through_softlist())
		battery_load(m_ssd_data.get(), size, nullptr);

	// open the door
	m_door_cb(ASSERT_LINE);

	// setup the timer to close the door
	m_door_timer->adjust(attotime::from_msec(200));

	return std::make_pair(std::error_condition(), std::string());
}


std::pair<std::error_condition, std::string> psion_ssd_device::call_create(int format_type, util::option_resolution *create_args)
{
	uint32_t const size = 0x20000;

	// 128k RAM Solid State Disk by default
	set_info_byte(size, SSD_RAM);

	std::fill_n(m_ssd_data.get(), sizeof(m_ssd_data), 0);

	fwrite(m_ssd_data.get(), size);

	// open the door
	m_door_cb(ASSERT_LINE);

	// setup the timer to close the door
	m_door_timer->adjust(attotime::from_msec(200));

	return std::make_pair(std::error_condition(), std::string());
}


void psion_ssd_device::call_unload()
{
	uint32_t const size = length();

	if ((m_info_byte & 0xe0) == 0x00) // not write protected
	{
		if (!loaded_through_softlist())
		{
			// write to original file
			fseek(0, SEEK_SET);
			fwrite(m_ssd_data.get(), size);
		}
		else
		{
			// write to nvram
			battery_save(m_ssd_data.get(), size);
		}
	}

	set_info_byte(0);

	std::fill_n(m_ssd_data.get(), sizeof(m_ssd_data), 0);

	// open the door
	m_door_cb(ASSERT_LINE);

	// setup the timer to close the door
	m_door_timer->adjust(attotime::from_msec(200));
}


uint32_t psion_ssd_device::latched_addr()
{
	return (m_port_latch & make_bitmask<uint32_t>(m_mem_width)) | (BIT(m_port_latch, 22, 2) << m_mem_width);
}


void psion_ssd_device::set_info_byte(uint32_t size, uint8_t type)
{
	m_info_byte = 0x00;

	// Type 1 Flash or RAM
	if (type != SSD_RAM)
		m_info_byte |= 0xe0; // write protected

	// No. devices and size
	switch (size)
	{
	case 0x010000: m_info_byte |= 0x09; m_mem_width = 15; break; // 64K  (2 x 32K)
	case 0x020000: m_info_byte |= 0x03; m_mem_width = 17; break; // 128K (1 x 128K)
	case 0x040000: m_info_byte |= 0x0b; m_mem_width = 17; break; // 256K (2 x 128K)
	case 0x080000: m_info_byte |= 0x0c; m_mem_width = 18; break; // 512K (2 x 256K)
	case 0x100000: m_info_byte |= 0x1c; m_mem_width = 18; break; // 1M   (4 x 256K)
	case 0x200000: m_info_byte |= 0x1d; m_mem_width = 19; break; // 2M   (4 x 512K)
	case 0x400000: m_info_byte |= 0x1e; m_mem_width = 20; break; // 4M   (4 x 1M)
	case 0x800000: m_info_byte |= 0x1f; m_mem_width = 21; break; // 8M   (4 x 2M)
	}

	// set pull-ups on ASIC5
	m_asic5->set_info_byte(m_info_byte);
}


uint8_t psion_ssd_device::data_r()
{
	if (m_info_byte & 7)
	{
		return m_asic5->data_r();
	}
	return 0x00;
}

void psion_ssd_device::data_w(uint16_t data)
{
	if (m_info_byte & 7)
	{
		m_asic5->data_w(data);
	}
}
