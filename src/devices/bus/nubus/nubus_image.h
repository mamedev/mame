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
	uint32_t curcmd;
	uint8_t filename[128];
	uint8_t curdir[1024];
		osd::directory::ptr dirp;
	osd_file::ptr fd;
	uint64_t filelen;
	uint32_t bytecount;
};

// ======================> nubus_image_device

class nubus_image_device :
		public device_t,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
		nubus_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const tiny_rom_entry *device_rom_region() const override;

protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

		uint32_t image_status_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void image_status_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t image_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void image_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t image_super_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void image_super_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t file_cmd_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void file_cmd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t file_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void file_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t file_len_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void file_len_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
		uint32_t file_name_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
		void file_name_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

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
