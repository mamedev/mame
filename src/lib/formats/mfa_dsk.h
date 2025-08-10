// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, rfka01
/*********************************************************************

    formats/mfa_dsk.h

    MFA Mikrocomputer für Ausbildung

*********************************************************************/
#ifndef MAME_FORMATS_MFA_DSK_H
#define MAME_FORMATS_MFA_DSK_H

#pragma once

#include "upd765_dsk.h"

class mfa_format : public upd765_format
{
public:
	mfa_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const mfa_format FLOPPY_MFA_FORMAT;

#endif // MAME_FORMATS_MFA_DSK_H
