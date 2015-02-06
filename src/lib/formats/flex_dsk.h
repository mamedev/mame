/*
 * flex_dsk.h
 *
 *  Created on: 24/06/2014
 */

#ifndef FLEX_DSK_H_
#define FLEX_DSK_H_

#include "wd177x_dsk.h"

class flex_format : public wd177x_format {
public:
	flex_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
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
	static const format formats[];
};

extern const floppy_format_type FLOPPY_FLEX_FORMAT;

#endif /* FLEX_DSK_H_ */
