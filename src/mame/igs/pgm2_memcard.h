// license:BSD-3-Clause
// copyright-holders:David Haywood, Miodrag Milanovic
/*********************************************************************

    pgm2_memcard.h

    PGM2 Memory card functions.
    (based on ng_memcard.h)

*********************************************************************/
#ifndef MAME_IGS_PGM2_MEMCARD_H
#define MAME_IGS_PGM2_MEMCARD_H

#pragma once

#include "imagedev/memcard.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// ======================> pgm2_memcard_device

class pgm2_memcard_device :  public device_t,
							public device_memcard_image_interface
{
public:
	// construction/destruction
	pgm2_memcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "pg2,bin,mem"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 read_prot(offs_t offset);
	void write_prot(offs_t offset, u8 data);
	u8 read_sec(offs_t offset);
	void write_sec(offs_t offset, u8 data);
	void auth(u8 p1, u8 p2, u8 p3);

	/* returns the index of the current memory card, or -1 if none */
	int present() { return is_loaded() ? 0 : -1; }

private:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	u8 m_memcard_data[0x100];
	u8 m_protection_data[4];
	u8 m_security_data[4];
	bool m_authenticated = false;
};


// device type definition
DECLARE_DEVICE_TYPE(PGM2_MEMCARD, pgm2_memcard_device)


#endif // MAME_IGS_PGM2_MEMCARD_H
