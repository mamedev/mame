// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * Semidisk Card
 *
 *******************************************************************/

#include "emu.h"
#include "semidisk.h"

//**************************************************************************
//  EPSON SEMIDISK DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_SEMIDISK, bus::epson_qx::semidisk_device, "epson_qx_option_semidisk", "Semidisk RAM Disk")

namespace bus::epson_qx {

static constexpr uint32_t BANK_SIZE = 512 * 1024;
static constexpr uint8_t  NUM_BANKS = 4;
static constexpr uint32_t RAM_SIZE = BANK_SIZE * NUM_BANKS;

INPUT_PORTS_START( semidisk )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x01, "Battery Backup")
	PORT_CONFSETTING(0x01, "Enabled")
	PORT_CONFSETTING(0x00, "Disabled")
INPUT_PORTS_END

//-------------------------------------------------
//  semidisk_device - constructor
//-------------------------------------------------
semidisk_device::semidisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_SEMIDISK, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	device_option_expansion_interface(mconfig, *this),
	m_config(*this, "CONFIG")
{
}

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------
ioport_constructor semidisk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( semidisk );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void semidisk_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(RAM_SIZE);

	address_space &space = m_bus->iospace();
	space.install_device(0xe0, 0xe3, *this, &semidisk_device::map);

	save_pointer(NAME(m_ram), RAM_SIZE);
	save_item(NAME(m_track));
	save_item(NAME(m_sector));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void semidisk_device::device_reset()
{
	m_sector = 0;
	m_track = 0;
}

//-------------------------------------------------
//  nvram implementation
//-------------------------------------------------
void semidisk_device::nvram_default()
{
	memset(m_ram.get(), 0, RAM_SIZE);
}

bool semidisk_device::nvram_read(util::read_stream &file)
{
	if (!(m_config->read() & 0x01)) {
		return false;
	}

	auto const [err, actual] = util::read(file, m_ram.get(), RAM_SIZE);
	return !err && (actual == RAM_SIZE);
}

bool semidisk_device::nvram_write(util::write_stream &file)
{
	if (!(m_config->read() & 0x01)) {
		memset(m_ram.get(), 0, RAM_SIZE);
	}

	auto const [err, actual] = util::write(file, m_ram.get(), RAM_SIZE);
	return !err;
}

//-------------------------------------------------
//  I/O implementation
//-------------------------------------------------
void semidisk_device::map(address_map &map)
{
	map(0x00, 0x00).select(0xff00).rw(FUNC(semidisk_device::data_r), FUNC(semidisk_device::data_w));
	map(0x02, 0x03).mirror(0xff00).w(FUNC(semidisk_device::reg_w));
}

uint8_t semidisk_device::data_r(offs_t offset)
{
	uint8_t bank = (m_sector >> 4) & 0x0f;
	uint8_t sector = m_sector & 0x0f;
	uint8_t byte_addr = (offset >> 8) & 0x7f;
	uint32_t ram_offset = (bank * BANK_SIZE) + (m_track * 16 * 128) + (sector * 128) + byte_addr;

	if (bank < NUM_BANKS) {
		return m_ram[ram_offset & (RAM_SIZE - 1)];
	}
	return 0xff;
}

void semidisk_device::data_w(offs_t offset, uint8_t data)
{
	uint8_t bank = (m_sector >> 4) & 0x0f;
	uint8_t sector = m_sector & 0x0f;
	uint8_t byte_addr = (offset >> 8) & 0x7f;
	uint32_t ram_offset = (bank * BANK_SIZE) + (m_track * 16 * 128) + (sector * 128) + byte_addr;

	if (bank < NUM_BANKS) {
		m_ram[ram_offset & (RAM_SIZE - 1)] = data;
	}
}

void semidisk_device::reg_w(offs_t offset, uint8_t data)
{
	switch(offset & 0xff) {
	case 0:
		m_track = data;
		break;
	case 1:
		m_sector = data;
		break;
	}
}

} // namespace bus::epson_qx
