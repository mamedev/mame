// license:BSD-2-Clause
// copyright-holders:Lubomir Rintel

/***************************************************************************

    Altos 586 Hard Disk Controller emulation

    The controller is based around the 8089 I/O Processor.
    Its memory bus is connected (via modified multibus) to the main memory,
    shared with the main 8086 processor. The I/O bus contains the internal
    controller registers and a 16K SRAM for caching the I/O programs
    I/O parameter blocks (command blocks) and staging the sector data.

    The formatting is custom, enforces 16 sectors per tracks.
    Heads/Cylinder combinations used by different Altos 586 models
    as supported by the diagnostic software (and XENIX, I think):

    Model        Heads   Cylinders
    ACS586-10    4       306
    ACS586-20    6       306
    ACS586-30    6       512
    ACS586-40    8       512
    H-H 20 MB    4       612

    Literature:

    [1] Notes on the Altos 586 Computer & Firmware disassembly
        https://github.com/lkundrak/altos586/

    [2] Preliminary 8600 User Manual
        Section 7. System Specs.
    4.5. Rigid Disk Controllers and Interface
    Specification Revision 4.2, May 27, 1982, Page 52-62

    [3] Preliminary 8600 User Manual
        Section 7. System Specs.
    A. Rigid Disk Controller Documentation
    Specification Revision 4.2, May 27, 1982, Page 95-108

    Note: The Altos 8600 manuals [2] and [3] describe a controller
    wired differently (the 8089 seems to be in local mode, not remote),
    the register addresses are different, but their function and command
    set seems very different. I've written this before discovering those
    manuals, so the command names are slightly different. My intent is to
    eventually align that and review some of my assumptions too.

    TODO: How is the command completion interrupt signalled?
    I've only verified it to work with the diags floppy and that
    one is not interrupt-driven. XENIX almost certainly is.

***************************************************************************/

#include "emu.h"
#include "altos586_hdc.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ALTOS586_HDC, altos586_hdc_device, "altos586_hdc", "Disk Controller board for Altos 586")

altos586_hdc_device::altos586_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ALTOS586_HDC, tag, owner, clock)
	, m_bus(*this, finder_base::DUMMY_TAG, -1)
	, m_iop(*this, "iop")
	, m_hdd(*this, "hdd%u", 0)
{
}

bool altos586_hdc_device::sector_exists(uint8_t index)
{
	if (!m_geom[m_drive]) {
		logerror("drive not present: %d", m_drive);
	} else if (m_head > m_geom[m_drive]->heads) {
		logerror("head %d not present in drive that has %d heads", m_head, m_geom[m_drive]->heads);
	} else if (index > m_geom[m_drive]->sectors) {
		logerror("sector %d not present in drive that has %d sectors per track", index, m_geom[m_drive]->sectors);
	} else {
		LOG("drive %d sector CHS=%d/%d/%d found\n", m_drive, m_cyl[m_drive], m_head, index);
		return true;
	}

	m_status |= 0x40; // Record not found.
	return false;
}

uint32_t altos586_hdc_device::sector_lba(uint8_t const index)
{
	return (m_cyl[m_drive] * m_geom[m_drive]->heads + m_head) * m_geom[m_drive]->sectors + index;
}

void altos586_hdc_device::sector_read(uint8_t index)
{
	if (!sector_exists(index))
		return;
	m_hdd[m_drive]->read(sector_lba(index), &m_sector[5]);
	m_iop->drq1_w(ASSERT_LINE);
}

void altos586_hdc_device::sector_write(uint8_t index)
{
	if (!sector_exists(index))
		return;
	m_hdd[m_drive]->write(sector_lba(index), &m_sector[5]);
}

uint16_t altos586_hdc_device::mem_r(offs_t offset, uint16_t mem_mask)
{
	return m_bus->read_word(offset << 1, mem_mask);
}

void altos586_hdc_device::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_bus->write_word(offset << 1, data, mem_mask);
}

void altos586_hdc_device::altos586_hdc_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(altos586_hdc_device::mem_r), FUNC(altos586_hdc_device::mem_w));
}

uint16_t altos586_hdc_device::data_r(offs_t offset)
{
	uint8_t value = m_sector[m_secoffset];

	if (!machine().side_effects_disabled()) {
		m_secoffset++;
		m_secoffset %= std::size(m_sector);

		if (m_secoffset == 0) {
			LOG("read reached the end of the data buffer\n");
			m_iop->drq1_w(CLEAR_LINE);
		}
	}

	return value;
}

void altos586_hdc_device::data_w(offs_t offset, uint16_t data)
{
	m_sector[m_secoffset++] = data;
	m_secoffset %= std::size(m_sector);

	if (m_secoffset == 5) {
		// If we reached this watermark in the data buffer, we've completed
		// writing the sector formatting data.
		// We don't do anything the data beyond checking it.
		if (m_sector[1] != 0xfe)
			logerror("suspicious sector mark\n");
		if ((((m_sector[3] & 0xf) << 8) | m_sector[2]) != m_cyl[m_drive])
			logerror("cylinder number mismatch\n");
		if ((m_sector[3] >> 4) != m_head)
			logerror("head number mismatch\n");
		if (m_sector[0] != m_sector[4]) {
			// I think (not sure), that this might be okay for interleaved access.
			LOG("sector number mismatch (probably okay)\n");
		}

		LOG("writing drive %d sector CHS=%d/%d/%d format finished\n", m_drive, m_cyl[m_drive], m_head, m_sector[0]);
		LOG("  sector mark = 0x%02x\n", m_sector[1]);
		LOG("  cylinder low = 0x%02x\n", m_sector[2]);
		LOG("  head | cylinder hi = 0x%02x\n", m_sector[3]);
		LOG("  sector = 0x%02x\n", m_sector[4]);

		m_iop->drq1_w(CLEAR_LINE);
		m_secoffset = 0;
	} else if (m_secoffset == 0) {
		LOG("write reached the end of the data buffer\n");
		m_iop->drq1_w(CLEAR_LINE);
		sector_write(m_sector[0]);
	}
}

void altos586_hdc_device::head_select_w(offs_t offset, uint16_t data)
{
	// Not Ready.
	m_status &= ~0x80;

	switch (data & 0xf0) {
	case 0x10:
		m_drive = 0;
		break;
	case 0x20:
		m_drive = 1;
		break;
	default:
		logerror("unsupported drive select\n");
		return;
	}

	if (m_geom[m_drive] == nullptr) {
		logerror("drive %d not present\n", m_drive);
		return;
	}

	m_head = data & 0x0f;
	if (m_head > m_geom[m_drive]->heads) {
		logerror("head %d invalid for drive %d\n", m_head, m_drive);
		return;
	}

	// Ready.
	m_status |= 0x80;

	LOG("selected drive %d head %d\n", m_drive, m_head);
}

uint16_t altos586_hdc_device::seek_status_r(offs_t offset)
{
	return m_seek_status;
}

void altos586_hdc_device::cyl_w(offs_t offset, uint16_t data)
{
	m_cyl_latch >>= 8;
	m_cyl_latch |= data << 8;
}

uint16_t altos586_hdc_device::status_r(offs_t offset)
{
	return m_status;
}

void altos586_hdc_device::command_w(offs_t offset, uint16_t data)
{
	switch (data) {
	case 0x01:
		// Read.
		// Sector number now in data buffer, data readout follows.
		LOG("READ command\n");
		if (m_secoffset != 1)
			logerror("expected one value in data buffer, has %d\n", m_secoffset);

		sector_read(m_sector[0]);
		// Skip the sector format, just read the data.
		m_secoffset = 5;
		break;

	case 0x01 | 0x08:
		// Long Read.
		// Sector number now in data buffer, header + data readout follows.
		LOG("READ LONG command\n");
		if (m_secoffset != 1)
			logerror("expected one value in data buffer, has %d\n", m_secoffset);

		// Sector header.
		m_sector[2] = m_sector[0];
		m_sector[0] = m_cyl[m_drive] & 0xff;
		// Bit 3 indicates bad sector (we don't ever set it).
		m_sector[1] = m_head << 4 | m_cyl[m_drive] >> 8;
		m_sector[3] = m_sector[4] = 0;

		sector_read(m_sector[2]);
		m_secoffset = 0;
		break;

	case 0x02:
		// Write.
		// Sector number now in data buffer, data write follows.
		LOG("WRITE command\n");
		if (m_secoffset != 1)
			logerror("expected one value in data buffer, has %d\n", m_secoffset);

		// No sector format, just data.
		m_secoffset = 5;
		m_iop->drq1_w(ASSERT_LINE);
		break;

	case 0x04:
		// Write Sector Format.
		// Sector number now in data buffer, header write follows.
		LOG("WRITE FORMAT command\n");
		if (m_secoffset != 1)
			logerror("expected one value in data buffer, has %d\n", m_secoffset);

		m_secoffset = 1; // Leave the sector number in.
		m_iop->drq1_w(ASSERT_LINE);
		break;

	case 0x10:
		// Seek to cylinder.
		// Current cylinder is in data buffer, new one in a separate latch.
		LOG("SEEK from cylinder %d to %d\n", m_sector[1] << 8 | m_sector[0], m_cyl_latch);
		if (m_secoffset != 2)
			logerror("expected two values in data buffer, has %d\n", m_secoffset);

		m_secoffset = 0;
		m_cyl[m_drive] = m_cyl_latch;
		// Seek done.
		m_seek_status |= 0x02;
		// Not busy.
		m_status &= ~0x01;
		break;

	case 0x20:
		// Not sure, actually.
		LOG("SELECT command\n");
		// Head/drive selected?
		m_seek_status |= 0x01;
		break;

	case 0x80:
		// This looks like a reset of some sort.
		LOG("RESET command\n");
		device_reset();
		break;

	default:
		logerror("unknown command 0x%02x\n", data);
	}
}

void altos586_hdc_device::altos586_hdc_io(address_map &map)
{
	map(0x0000, 0x3fff).ram(); // 16K SRAM.
	map(0xffd0, 0xffd1).rw(FUNC(altos586_hdc_device::data_r), FUNC(altos586_hdc_device::data_w));
	map(0xffd2, 0xffd3).w(FUNC(altos586_hdc_device::head_select_w));
	map(0xffd4, 0xffd5).rw(FUNC(altos586_hdc_device::seek_status_r), FUNC(altos586_hdc_device::cyl_w));
	map(0xffd6, 0xffd7).rw(FUNC(altos586_hdc_device::status_r), FUNC(altos586_hdc_device::command_w));
	// Diags write 0x10 to 0xfff8 and XENIX writes 0x82 to 0xffe6.
	// Not sure what either does.
	map(0xfff8, 0xfff9).nopw();
}

template <uint8_t Index>
std::error_condition altos586_hdc_device::hdd_load(device_image_interface &image)
{
	if (m_hdd[Index]->get_info().sectorbytes != 512) {
		logerror("expected 512 bytes per sector, got %d", m_geom[Index]->sectorbytes);
		return image_error::INVALIDLENGTH;
	}

	m_geom[Index] = &m_hdd[Index]->get_info();
	return std::error_condition();
}

void altos586_hdc_device::device_add_mconfig(machine_config &config)
{
	I8089(config, m_iop, XTAL(15'000'000) / 3);
	m_iop->set_addrmap(AS_PROGRAM, &altos586_hdc_device::altos586_hdc_mem);
	m_iop->set_addrmap(AS_IO, &altos586_hdc_device::altos586_hdc_io);
	m_iop->set_data_width(16);

	harddisk_image_device &hdd0(HARDDISK(config, "hdd0", 0));
	hdd0.set_device_load(FUNC(altos586_hdc_device::hdd_load<0U>));
	hdd0.set_device_unload(FUNC(altos586_hdc_device::hdd_unload<0U>));

	harddisk_image_device &hdd1(HARDDISK(config, "hdd1", 0));
	hdd1.set_device_load(FUNC(altos586_hdc_device::hdd_load<1U>));
	hdd1.set_device_unload(FUNC(altos586_hdc_device::hdd_unload<1U>));
}

void altos586_hdc_device::device_reset()
{
	m_status = 0;
	m_seek_status = 0xfc;

	m_cyl_latch = 0;
	m_cyl[m_drive] = 0;

	memset(m_sector, 0, sizeof(m_sector));
	m_secoffset = 0;
}

void altos586_hdc_device::attn_w(uint16_t data)
{
	m_iop->ca_w(ASSERT_LINE);
	m_iop->ca_w(CLEAR_LINE);
}

void altos586_hdc_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_seek_status));
	save_item(NAME(m_cyl_latch));
	save_item(NAME(m_cyl[0]));
	save_item(NAME(m_cyl[1]));
	save_pointer(NAME(m_sector), std::size(m_sector));
	save_item(NAME(m_secoffset));
	save_item(NAME(m_drive));
	save_item(NAME(m_head));
}
