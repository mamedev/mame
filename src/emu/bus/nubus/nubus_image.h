// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __NUBUS_IMAGE_H__
#define __NUBUS_IMAGE_H__

#include "emu.h"
#include "nubus.h"
#include "osdcore.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class messimg_disk_image_device;

struct nbfilectx {
	UINT32 curcmd;
	UINT8 filename[128];
	UINT8 curdir[1024];
	osd_directory *dirp;
	osd_file *fd;
	UINT64 filelen;
	UINT32 bytecount;
};

// ======================> nubus_image_device

class nubus_image_device :
		public device_t,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		nubus_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;

protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();

		DECLARE_READ32_MEMBER(image_status_r);
		DECLARE_WRITE32_MEMBER(image_status_w);
		DECLARE_READ32_MEMBER(image_r);
		DECLARE_WRITE32_MEMBER(image_w);
		DECLARE_READ32_MEMBER(image_super_r);
		DECLARE_WRITE32_MEMBER(image_super_w);
		DECLARE_READ32_MEMBER(file_cmd_r);
		DECLARE_WRITE32_MEMBER(file_cmd_w);
		DECLARE_READ32_MEMBER(file_data_r);
		DECLARE_WRITE32_MEMBER(file_data_w);
		DECLARE_READ32_MEMBER(file_len_r);
		DECLARE_WRITE32_MEMBER(file_len_w);
		DECLARE_READ32_MEMBER(file_name_r);
		DECLARE_WRITE32_MEMBER(file_name_w);

public:
	messimg_disk_image_device *m_image;
	struct nbfilectx filectx;
};


// device type definition
extern const device_type NUBUS_IMAGE;

enum {
	kFileCmdGetDir = 1,
	kFileCmdSetDir,
	kFileCmdGetFirstListing,
	kFileCmdGetNextListing,
	kFileCmdGetFile,
	kFileCmdPutFile
};

#endif  /* __NUBUS_IMAGE_H__ */
