// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    hashfile.h

    Code for parsing hash info (*.hsi) files

*********************************************************************/

#ifndef MAME_EMU_HASHFILE_H
#define MAME_EMU_HASHFILE_H

#pragma once


bool hashfile_extrainfo(device_image_interface &image, std::string &result);
bool hashfile_extrainfo(const char *hash_path, const game_driver &driver, const util::hash_collection &hashes, std::string &result);

#endif // MAME_EMU_HASHFILE_H
