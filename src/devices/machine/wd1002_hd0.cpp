// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Western Digital WD1002-HD0 Winchester Disk Controller board

    See wd1002_hd0.h.  Generalised from the hand-wired Kaypro 10 controller.

***************************************************************************/

#include "emu.h"
#include "wd1002_hd0.h"


DEFINE_DEVICE_TYPE(WD1002_HD0, wd1002_hd0_device, "wd1002_hd0", "Western Digital WD1002-HD0 Winchester Disk Controller")


namespace {

// WD1010 command opcodes (high nibble).  Board-level codes the WD1015 front-end
// services itself (e.g. the 0x9x controller self-test) are deliberately NOT here.
enum : uint8_t {
	CMD_RESTORE      = 0x10,
	CMD_READ_SECTOR  = 0x20,
	CMD_WRITE_SECTOR = 0x30,
	CMD_SCAN_ID      = 0x40,
	CMD_WRITE_FORMAT = 0x50,
	CMD_SEEK         = 0x70
};

// SDH register fields
enum : uint8_t {
	SDH_HEAD  = 0x07, // head select
	SDH_DRIVE = 0x18  // drive select (collapsed to the single board drive)
};

} // anonymous namespace


wd1002_hd0_device::wd1002_hd0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd1002_hd0_device(mconfig, WD1002_HD0, tag, owner, clock)
{
}

wd1002_hd0_device::wd1002_hd0_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_hdc(*this, "hdc")
	, m_hdd(*this, "hdc:0")
	, m_intrq_cb(*this),
	, m_ptr(0)
	, m_sector_bytes(BUFFER_SIZE)
{
}


void wd1002_hd0_device::device_add_mconfig(machine_config &config)
{
	WD1010(config, m_hdc, 5'000'000); // the WD1010 Winchester controller (5 MHz)
	m_hdc->out_intrq_callback().set(FUNC(wd1002_hd0_device::intrq_w));
	m_hdc->out_bcr_callback().set(FUNC(wd1002_hd0_device::bcr_w));
	m_hdc->in_data_callback().set(FUNC(wd1002_hd0_device::buf_in));
	m_hdc->out_data_callback().set(FUNC(wd1002_hd0_device::buf_out));
	HARDDISK(config, "hdc:0", 0); // drive 0 (the WD1010 finds it as its subdevice "0")
}

void wd1002_hd0_device::device_start()
{
	m_buf = std::make_unique<uint8_t[]>(BUFFER_SIZE);

	save_pointer(NAME(m_buf), BUFFER_SIZE);
	save_item(NAME(m_ptr));
	save_item(NAME(m_sector_bytes));
}

void wd1002_hd0_device::device_reset()
{
	m_hdc->drdy_w(m_hdd->exists() ? 1 : 0);          // drive ready once its CHD is mounted
	m_hdc->sc_w(1);                                  // seek complete (heads at a known track)
	m_ptr = 0;
	// the buffer-full point follows the drive's sector size (128/256/512), the same
	// size the WD1010 transfers; this is what lets one board serve 256-byte machines
	// (TRS-80, EM-II rigid) and 512-byte ones (Kaypro) without a rebuild.
	m_sector_bytes = (m_hdd->exists() && m_hdd->get_info().sectorbytes <= BUFFER_SIZE)
		? m_hdd->get_info().sectorbytes : BUFFER_SIZE;
}


uint8_t wd1002_hd0_device::read(offs_t offset)
{
	if (offset == 0) // host reads the buffered sector
	{
		uint8_t const data = m_buf[m_ptr & (BUFFER_SIZE - 1)];
		if (!machine().side_effects_disabled())
			m_ptr++;
		return data;
	}
	return m_hdc->read(offset); // 1-7 -> WD1010 task file
}

void wd1002_hd0_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0) // host fills the sector buffer
	{
		m_buf[m_ptr & (BUFFER_SIZE - 1)] = data;
		if ((m_ptr++ & (m_sector_bytes - 1)) == (m_sector_bytes - 1))
			m_hdc->brdy_w(1); // a full sector has arrived
		return;
	}
	if (offset == 6) // SDH: head select feeds the WD1010 via head_w
	{
		m_hdc->head_w(data & SDH_HEAD);
		// single-drive board: collapse the drive-select onto the one configured drive
		// so the controller always finds it (matches the Kaypro drive-select latch).
		data &= ~SDH_DRIVE;
	}
	if (offset == 7) // command
	{
		m_hdc->brdy_w(0);
		m_ptr = 0;
		uint8_t const cmd = data & 0xf0;
		// Only the WD1010's own opcodes reach the chip.  Board-level / diagnostic commands --
		// e.g. the WD1002 controller self-test (0x90), which the host issues before it touches a
		// drive -- are serviced by the WD1015 front-end and must NOT reach the WD1010, which has
		// no such opcode and would latch BUSY forever (hanging the host's status-poll loop).
		if (cmd == CMD_RESTORE || cmd == CMD_READ_SECTOR || cmd == CMD_WRITE_SECTOR
			|| cmd == CMD_SCAN_ID || cmd == CMD_WRITE_FORMAT || cmd == CMD_SEEK)
			m_hdc->write(offset, data);
		return;
	}
	m_hdc->write(offset, data); // registers 1-6 -> WD1010 task file
}


uint8_t wd1002_hd0_device::buf_in()
{
	uint8_t const data = m_buf[m_ptr & (BUFFER_SIZE - 1)];
	m_ptr++;
	return data;
}

void wd1002_hd0_device::buf_out(uint8_t data)
{
	m_buf[m_ptr & (BUFFER_SIZE - 1)] = data;
	if ((m_ptr++ & (m_sector_bytes - 1)) == (m_sector_bytes - 1))
		m_hdc->brdy_w(1); // a full sector has been read off the disk
}

void wd1002_hd0_device::intrq_w(int state)
{
	m_intrq_cb(state);
}

void wd1002_hd0_device::bcr_w(int state)
{
	if (state)
		m_ptr = 0;
}
