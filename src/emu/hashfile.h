// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    hashfile.h

    Code for parsing hash info (*.hsi) files

*********************************************************************/

#ifndef __HASHFILE_H__
#define __HASHFILE_H__

#include "emu.h"


bool hashfile_extrainfo(device_image_interface &image, std::string &result);

#endif /* __HASHFILE_H__ */
