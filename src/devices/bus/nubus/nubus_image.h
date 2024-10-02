// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_NUBUS_NUBUS_IMAGE_H
#define MAME_BUS_NUBUS_NUBUS_IMAGE_H

#pragma once

#include "nubus.h"
#include "osdfile.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_image_device

class nubus_image_device :
		public device_t,
		public device_nubus_card_interface
{
public:
	class messimg_disk_image_device;

	// construction/destruction
	nubus_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	enum
	{
		kFileCmdGetDir = 1,
		kFileCmdSetDir,
		kFileCmdGetFirstListing,
		kFileCmdGetNextListing,
		kFileCmdGetFile,
		kFileCmdPutFile
	};

	struct nbfilectx
	{
		uint32_t curcmd;
		char filename[128];
		std::string curdir;
		osd::directory::ptr dirp;
		osd_file::ptr fd;
		uint64_t filelen;
		uint32_t bytecount;
	};

	nubus_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	uint32_t image_status_r();
	void image_status_w(uint32_t data);
	uint32_t image_r();
	void image_w(uint32_t data);
	uint32_t image_super_r(offs_t offset, uint32_t mem_mask = ~0);
	void image_super_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t file_cmd_r();
	void file_cmd_w(uint32_t data);
	uint32_t file_data_r();
	void file_data_w(uint32_t data);
	uint32_t file_len_r();
	void file_len_w(uint32_t data);
	uint32_t file_name_r(offs_t offset);
	void file_name_w(offs_t offset, uint32_t data);

	messimg_disk_image_device *m_image;
	nbfilectx filectx;
};


// device type definition
DECLARE_DEVICE_TYPE(NUBUS_IMAGE, nubus_image_device)

#endif // MAME_BUS_NUBUS_NUBUS_IMAGE_H
