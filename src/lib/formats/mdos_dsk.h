// license:BSD-3-Clause
// copyright-holders:68bit
/*
 * mdos_dsk.h  -  Motorola MDOS compatible disk images
 */
#ifndef MAME_FORMATS_MDOS_DSK_H
#define MAME_FORMATS_MDOS_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

class mdos_format : public wd177x_format
{
public:
	mdos_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual const wd177x_format::format &get_track_format(const format &f, int head, int track) override;

private:
	struct disk_id_sector
	{
		uint8_t id[8];
		uint8_t version[2];
		uint8_t revision[2];
		uint8_t date[6];
		uint8_t username[20];
		uint8_t rib_addr[20];
		uint8_t unused[70];
	} info;
	static const format formats[];
	static const format formats_head1[];
	bool check_ascii(uint8_t *, size_t len, const char* name);
	int parse_date_field(uint8_t *str);
};

extern const floppy_format_type FLOPPY_MDOS_FORMAT;

#endif // MAME_FORMATS_MDOS_DSK_H
