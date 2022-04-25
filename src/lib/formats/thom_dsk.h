// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_FORMATS_THOM_DSK_H
#define MAME_FORMATS_THOM_DSK_H

#pragma once

#include "wd177x_dsk.h"

class thomson_525_format : public wd177x_format
{
public:
  thomson_525_format();

  virtual const char *name() const override;
  virtual const char *description() const override;
  virtual const char *extensions() const override;

private:
  static const format formats[];
};

class thomson_35_format : public wd177x_format
{
public:
  thomson_35_format();

  virtual const char *name() const override;
  virtual const char *description() const override;
  virtual const char *extensions() const override;

private:
  static const format formats[];
};


extern const thomson_525_format FLOPPY_THOMSON_525_FORMAT;
extern const thomson_35_format FLOPPY_THOMSON_35_FORMAT;

#endif // MAME_FORMATS_THOM_DSK_H
