// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***********************************************

    CBM Quickloads

 ***********************************************/

#ifndef __CBM_SNQK_H__
#define __CBM_SNQK_H__

#include "imagedev/snapquik.h"

#define CBM_QUICKLOAD_DELAY_SECONDS 3

int general_cbm_loadsnap( device_image_interface &image, const char *file_type, int snapshot_size,
	address_space &space, offs_t offset, void (*cbm_sethiaddress)(address_space &space, UINT16 hiaddress) );

void cbm_quick_sethiaddress( address_space &space, UINT16 hiaddress );

#endif  /* __CBM_SNQK_H__ */
