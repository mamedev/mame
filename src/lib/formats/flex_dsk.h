// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * flex_dsk.h
 *
 *  Created on: 24/06/2014
 */

#ifndef FLEX_DSK_H_
#define FLEX_DSK_H_

#include "flopimg.h"

class flex_format : public floppy_image_format_t {
public:
	flex_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool supports_save() const override;

private:
	struct sysinfo_sector
	{
		UINT8 unused1[16];
		UINT8 disk_name[8];
		UINT8 disk_ext[3];
		UINT8 disk_number[2];
		UINT8 fc_start_trk;
		UINT8 fc_start_sec;
		UINT8 fc_end_trk;
		UINT8 fc_end_sec;
		UINT8 free[2];
		UINT8 month;
		UINT8 day;
		UINT8 year;
		UINT8 last_trk;
		UINT8 last_sec;
		UINT8 unused2[216];
	} info;
};

extern const floppy_format_type FLOPPY_FLEX_FORMAT;

#endif /* FLEX_DSK_H_ */
