// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/apollo_dsk.h

    apollo format

*********************************************************************/

#ifndef APOLLO_DSK_H_
#define APOLLO_DSK_H_

#include "upd765_dsk.h"

class apollo_format : public upd765_format {
public:
	apollo_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_APOLLO_FORMAT;

#endif
