// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/hp_ipc_dsk.h

    hp_ipc format

*********************************************************************/

#ifndef HP_IPC_DSK_H_
#define HP_IPC_DSK_H_

#include "wd177x_dsk.h"

class hp_ipc_format : public wd177x_format {
public:
	hp_ipc_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_HP_IPC_FORMAT;

#endif
