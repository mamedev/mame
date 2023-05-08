// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***********************************************

    CBM Quickloads

 ***********************************************/

#ifndef MAME_COMMODORE_CBM_SNQK_H
#define MAME_COMMODORE_CBM_SNQK_H

#pragma once

#include <cstdint>
#include <string>
#include <system_error>
#include <utility>


#define CBM_QUICKLOAD_DELAY (attotime::from_seconds(3))

std::pair<std::error_condition, std::string> general_cbm_loadsnap(
		device_image_interface &image,
		address_space &space,
		offs_t offset,
		void (*cbm_sethiaddress)(address_space &space, uint16_t hiaddress));

void cbm_quick_sethiaddress(address_space &space, uint16_t hiaddress);

#endif // MAME_COMMODORE_CBM_SNQK_H
