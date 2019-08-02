// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    modules.h

    Code that creates the "canonical" Imgtool library

****************************************************************************/

#ifndef MODULES_H
#define MODULES_H

#include "library.h"

imgtoolerr_t imgtool_create_canonical_library(bool omit_untested, std::unique_ptr<imgtool::library> &library);

#endif /* MODULES_H */
