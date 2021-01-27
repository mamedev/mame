// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***********************************************

    CBM Quickloads

 ***********************************************/

#ifndef MAME_MACHINE_CBM_SNQK_H
#define MAME_MACHINE_CBM_SNQK_H

#pragma once

#include "imagedev/snapquik.h"

#define CBM_QUICKLOAD_DELAY (attotime::from_seconds(3))

image_init_result general_cbm_loadsnap(
		device_image_interface &image,
		address_space &space,
		offs_t offset,
		void (*cbm_sethiaddress)(address_space &space, uint16_t hiaddress));

void cbm_quick_sethiaddress(address_space &space, uint16_t hiaddress);

#endif // MAME_MACHINE_CBM_SNQK_H
