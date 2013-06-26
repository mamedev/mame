/*********************************************************************

    formats/d80_dsk.h

    Commodore 8050/8250/SFD-1001 sector disk image format

*********************************************************************/

#ifndef D80_DSK_H_
#define D80_DSK_H_

#include "d64_dsk.h"

class d80_format : public d64_format {
public:
	d80_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

protected:
	virtual int get_physical_track(const format &f, int track);
	virtual UINT32 get_cell_size(const format &f, int track);
	virtual int get_sectors_per_track(const format &f, int track);
	virtual int get_disk_id_offset(const format &f);

	static const format formats[];

	static const UINT32 cell_size[];
	static const int speed_zone[];
	static const int sectors_per_track[];
};

extern const floppy_format_type FLOPPY_D80_FORMAT;



#endif
