// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    micropolis.h

    Implementations of the Micropolis
    floppy disk controller for the Sorcerer

*********************************************************************/

#ifndef MAME_EXIDY_MICROPOLIS_H
#define MAME_EXIDY_MICROPOLIS_H

#pragma once

#include "imagedev/flopdrv.h"


/***************************************************************************
    MACROS
***************************************************************************/

class micropolis_device : public device_t
{
public:
	micropolis_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto dden_rd_callback() { return m_read_dden.bind(); }
	auto intrq_wr_callback() { return m_write_intrq.bind(); }
	auto drq_wr_callback() { return m_write_drq.bind(); }

	template<typename T, typename U, typename V, typename W>
	void set_drive_tags(T &&tag1, U &&tag2, V &&tag3, W &&tag4)
	{
		m_floppy_drive[0].set_tag(std::forward<T>(tag1));
		m_floppy_drive[1].set_tag(std::forward<U>(tag2));
		m_floppy_drive[2].set_tag(std::forward<V>(tag3));
		m_floppy_drive[3].set_tag(std::forward<W>(tag4));
	}

	void set_drive(uint8_t drive); // set current drive (0-3)

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state

	devcb_read_line m_read_dden;
	devcb_write_line m_write_intrq;
	devcb_write_line m_write_drq;

	optional_device_array<legacy_floppy_image_device, 4> m_floppy_drive;

	/* register */
	uint8_t m_data = 0U;
	uint8_t m_drive_num = 0U;
	uint8_t m_track = 0U;
	uint8_t m_sector = 0U;
	uint8_t m_command = 0U;
	uint8_t m_status = 0U;

	uint8_t   m_write_cmd = 0U;           // last write command issued

	uint8_t   m_buffer[6144]{};           // I/O buffer (holds up to a whole track)
	uint32_t  m_data_offset = 0U;         // offset into I/O buffer
	int32_t   m_data_count = 0;           // transfer count from/into I/O buffer

	uint32_t  m_sector_length = 0U;       // sector length (byte)

	// this is the drive currently selected
	legacy_floppy_image_device *m_drive = 0;

	void read_sector();
	void write_sector();

	uint8_t status_r(offs_t offset);
	uint8_t data_r();

	void command_w(uint8_t data);
	void data_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(MICROPOLIS, micropolis_device)


#endif // MAME_EXIDY_MICROPOLIS_H
