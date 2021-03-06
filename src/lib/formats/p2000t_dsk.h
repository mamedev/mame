// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/*********************************************************************

    p2000t_dsk.h

    P2000T disk images

*********************************************************************/
#ifndef MAME_FORMATS_P2000T_DSK_H
#define MAME_FORMATS_P2000T_DSK_H


#pragma once

#include "upd765_dsk.h"

class p2000t_format: public upd765_format
{
public:
    p2000t_format();

    virtual const char *name() const override;
    virtual const char *description() const override;
    virtual const char *extensions() const override;

private:
    static const format formats[];
};

extern const floppy_format_type FLOPPY_P2000T_FORMAT;


#endif // MAME_FORMATS_P2000T_DSK_H
