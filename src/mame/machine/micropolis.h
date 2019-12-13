// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    micropolis.h

    Implementations of the Micropolis
    floppy disk controller for the Sorcerer

*********************************************************************/

#ifndef MAME_MACHINE_MICROPOLIS_H
#define MAME_MACHINE_MICROPOLIS_H

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

	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( data_r );

	DECLARE_WRITE8_MEMBER( command_w );
	DECLARE_WRITE8_MEMBER( data_w );

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state

	devcb_read_line m_read_dden;
	devcb_write_line m_write_intrq;
	devcb_write_line m_write_drq;

	optional_device_array<legacy_floppy_image_device, 4> m_floppy_drive;

	/* register */
	uint8_t m_data;
	uint8_t m_drive_num;
	uint8_t m_track;
	uint8_t m_sector;
	uint8_t m_command;
	uint8_t m_status;

	uint8_t   m_write_cmd;              /* last write command issued */

	uint8_t   m_buffer[6144];           /* I/O buffer (holds up to a whole track) */
	uint32_t  m_data_offset;            /* offset into I/O buffer */
	int32_t   m_data_count;             /* transfer count from/into I/O buffer */

	uint32_t  m_sector_length;          /* sector length (byte) */

	/* this is the drive currently selected */
	legacy_floppy_image_device *m_drive;

	void read_sector();
	void write_sector();
};

DECLARE_DEVICE_TYPE(MICROPOLIS, micropolis_device)


#endif // MAME_MACHINE_MICROPOLIS_H
