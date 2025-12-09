// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus_image.cpp - synthetic NuBus card to allow reading/writing "raw"
  HFS images, including floppy images (DD and HD) and vMac/Basilisk HDD
  volumes up to 256 MB in size.

  TODO:
  * Get get directory and get listing commands have no way to indicate
    that the path/name is too long for the buffer.
  * The set directory command doesn't work well with host filesystems that
    have roots (e.g. Windows drive letters).
  * The set directory command assumes '/' is a valid host directory
    separator character.
  * The get listing commands have no way to indicate whether an entry is
    a directory.

***************************************************************************/

#include "emu.h"
#include "nubus_image.h"

#include "osdcore.h"
#include "osdfile.h"

#include <algorithm>

constexpr auto MAMEIMG_DISK_SECTOR_SIZE = 512;

class nubus_image_device : public device_t,
							public device_nubus_card_interface
{
public:
	class mameimg_disk_image_device;

	// construction/destruction
	nubus_image_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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
		u32 curcmd;
		char filename[128];
		std::string curdir;
		osd::directory::ptr dirp;
		osd_file::ptr fd;
		u64 filelen;
		u32 bytecount;
	};

	nubus_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void card_map(address_map &map);
	void card_super_map(address_map &map);

	u32 image_status_r();
	void image_status_w(u32 data);
	u32 image_r();
	void image_w(u32 data);
	u32 image_super_r(offs_t offset, u32 mem_mask = ~0);
	void image_super_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 file_cmd_r();
	void file_cmd_w(u32 data);
	u32 file_data_r();
	void file_data_w(u32 data);
	u32 file_len_r();
	void file_len_w(u32 data);
	u32 file_name_r(offs_t offset);
	void file_name_w(offs_t offset, u32 data);

	mameimg_disk_image_device *m_image;
	nbfilectx filectx;
};

// nubus_image_device::mameimg_disk_image_device

class nubus_image_device::mameimg_disk_image_device : public device_t, public device_image_interface
{
public:
	// construction/destruction
	mameimg_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// device_image_interface implementation
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "img"; }
	virtual const char *image_type_name() const noexcept override { return "disk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "disk"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

public:
	u32 m_size;
	std::unique_ptr<u8[]> m_data;
	bool m_ejected;
};

DEFINE_DEVICE_TYPE(MAMEIMG_DISK, nubus_image_device::mameimg_disk_image_device, "messimg_disk_image", "Mac image")

nubus_image_device::mameimg_disk_image_device::mameimg_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MAMEIMG_DISK, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_size(0), m_data(nullptr), m_ejected(false)
{
}

void nubus_image_device::mameimg_disk_image_device::device_start()
{
	m_data = nullptr;

	if (exists() && !fseek(0, SEEK_END))
	{
		m_size = u32(ftell());
	}
}

std::pair<std::error_condition, std::string> nubus_image_device::mameimg_disk_image_device::call_load()
{
	m_size = length();
	if (m_size > (256*1024*1024))
	{
		m_size = 0;
		return std::make_pair(image_error::INVALIDLENGTH, "Image file is too large (must be no more than 256MB)");
	}

	m_data.reset(new (std::nothrow) u8 [m_size]);
	if (!m_data)
		return std::make_pair(std::errc::not_enough_memory, "Error allocating memory for volume image");

	fseek(0, SEEK_SET);
	fread(m_data.get(), m_size);
	m_ejected = false;

	return std::make_pair(std::error_condition(), std::string());
}

void nubus_image_device::mameimg_disk_image_device::call_unload()
{
	// TODO: track dirty sectors and only write those
	fseek(0, SEEK_SET);
	fwrite(m_data.get(), m_size);
	m_size = 0;
	//free(m_data);
}

ROM_START( image )
	ROM_REGION(0x2000, "declrom", 0)
	ROM_LOAD( "nb_fake.bin",  0x000000, 0x002000, CRC(9264bac5) SHA1(540c2ce3c90382b2da6e1e21182cdf8fc3f0c930) )
ROM_END

void nubus_image_device::device_add_mconfig(machine_config &config)
{
	MAMEIMG_DISK(config, "nb_disk", 0);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_image_device::device_rom_region() const
{
	return ROM_NAME( image );
}

nubus_image_device::nubus_image_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_image_device(mconfig, NUBUS_IMAGE, tag, owner, clock)
{
}

nubus_image_device::nubus_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_image(nullptr)
{
}

void nubus_image_device::card_map(address_map &map)
{
	map(0x0000, 0x0003).rw(FUNC(nubus_image_device::image_r), FUNC(nubus_image_device::image_w));
	map(0x0004, 0x0007).rw(FUNC(nubus_image_device::image_status_r), FUNC(nubus_image_device::image_status_w));
	map(0x0008, 0x000b).rw(FUNC(nubus_image_device::file_cmd_r), FUNC(nubus_image_device::file_cmd_w));
	map(0x000c, 0x000f).rw(FUNC(nubus_image_device::file_data_r), FUNC(nubus_image_device::file_data_w));
	map(0x0010, 0x0013).rw(FUNC(nubus_image_device::file_len_r), FUNC(nubus_image_device::file_len_w));
	map(0x0014, 0x009b).rw(FUNC(nubus_image_device::file_name_r), FUNC(nubus_image_device::file_name_w));
}

void nubus_image_device::card_super_map(address_map &map)
{
	map(0x0000'0000, 0x0fff'ffff).rw(FUNC(nubus_image_device::image_super_r), FUNC(nubus_image_device::image_super_w));
}

void nubus_image_device::device_start()
{
	install_declaration_rom("declrom");
	nubus().install_map(*this, &nubus_image_device::card_map);
	nubus().install_super_map(*this, &nubus_image_device::card_super_map);

	m_image = subdevice<mameimg_disk_image_device>("nb_disk");

	filectx.curdir = ".";
	filectx.dirp.reset();
	filectx.fd.reset();
}

void nubus_image_device::image_status_w(u32 data)
{
	m_image->m_ejected = true;
}

u32 nubus_image_device::image_status_r()
{
	if(m_image->m_ejected) {
		return 0;
	}

	if(m_image->m_size) {
		return 1;
	}
	return 0;
}

void nubus_image_device::image_w(u32 data)
{
}

u32 nubus_image_device::image_r()
{
	return m_image->m_size;
}

void nubus_image_device::image_super_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 *image = (u32*)m_image->m_data.get();
	data = swapendian_int32(data);
	mem_mask = swapendian_int32(mem_mask);

	COMBINE_DATA(&image[offset]);
}

u32 nubus_image_device::image_super_r(offs_t offset, u32 mem_mask)
{
	u32 *image = (u32*)m_image->m_data.get();
	return swapendian_int32(image[offset]);
}

void nubus_image_device::file_cmd_w(u32 data)
{
	filectx.curcmd = data;
	switch (data) {
	case kFileCmdGetDir:
		strncpy(filectx.filename, filectx.curdir.c_str(), std::size(filectx.filename));
		break;
	case kFileCmdSetDir:
		if ((filectx.filename[0] == '/') || (filectx.filename[0] == '$')) {
			filectx.curdir.assign(std::begin(filectx.filename), std::find(std::begin(filectx.filename), std::end(filectx.filename), '\0'));
		} else {
			filectx.curdir += '/';
			filectx.curdir.append(std::begin(filectx.filename), std::find(std::begin(filectx.filename), std::end(filectx.filename), '\0'));
		}
		break;
	case kFileCmdGetFirstListing:
		filectx.dirp = osd::directory::open(filectx.curdir);
		[[fallthrough]];
	case kFileCmdGetNextListing:
		if (filectx.dirp) {
			osd::directory::entry const *const dp = filectx.dirp->read();
			if (dp) {
				strncpy(filectx.filename, dp->name, std::size(filectx.filename));
			} else {
				std::fill(std::begin(filectx.filename), std::end(filectx.filename), '\0');
			}
		}
		else {
			std::fill(std::begin(filectx.filename), std::end(filectx.filename), '\0');
		}
		break;
	case kFileCmdGetFile:
		{
			std::string fullpath(filectx.curdir);
			fullpath += PATH_SEPARATOR;
			fullpath.append(std::begin(filectx.filename), std::find(std::begin(filectx.filename), std::end(filectx.filename), '\0'));
			std::error_condition const filerr = osd_file::open(fullpath, OPEN_FLAG_READ, filectx.fd, filectx.filelen);
			if (filerr)
				osd_printf_error("%s: Error opening %s (%s:%d %s)\n", tag(), fullpath, filerr.category().name(), filerr.value(), filerr.message());
			filectx.bytecount = 0;
		}
		break;
	case kFileCmdPutFile:
		{
			std::string fullpath(filectx.curdir);
			fullpath += PATH_SEPARATOR;
			fullpath.append(std::begin(filectx.filename), std::find(std::begin(filectx.filename), std::end(filectx.filename), '\0'));
			u64 filesize; // unused, but it's an output from the open call
			std::error_condition const filerr = osd_file::open(fullpath, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, filectx.fd, filesize);
			if (filerr)
				osd_printf_error("%s: Error opening %s (%s:%d %s)\n", tag(), fullpath, filerr.category().name(), filerr.value(), filerr.message());
			filectx.bytecount = 0;
		}
		break;
	}
}

u32 nubus_image_device::file_cmd_r()
{
	return 0;
}

void nubus_image_device::file_data_w(u32 data)
{
	u32 count = 4;
	u32 actualcount = 0;

	data = swapendian_int32(data);
	if(filectx.fd) {
		//data = big_endianize_int32(data);
		if((filectx.bytecount + count) > filectx.filelen) count = filectx.filelen - filectx.bytecount;
		filectx.fd->write(&data, filectx.bytecount, count, actualcount);
		filectx.bytecount += actualcount;

		if(filectx.bytecount >= filectx.filelen) {
			filectx.fd.reset();
		}
	}
}

u32 nubus_image_device::file_data_r()
{
	if(filectx.fd) {
		u32 ret;
		u32 actual = 0;
		filectx.fd->read(&ret, filectx.bytecount, sizeof(ret), actual);
		filectx.bytecount += actual;
		if(actual < sizeof(ret)) {
			filectx.fd.reset();
		}
		return big_endianize_int32(ret);
	}
	return 0;
}

void nubus_image_device::file_len_w(u32 data)
{
	data = swapendian_int32(data);
	filectx.filelen = big_endianize_int32(data);
}

u32 nubus_image_device::file_len_r()
{
	return filectx.filelen;
}

void nubus_image_device::file_name_w(offs_t offset, u32 data)
{
	reinterpret_cast<u32 *>(filectx.filename)[offset] = big_endianize_int32(data);
}

u32 nubus_image_device::file_name_r(offs_t offset)
{
	return big_endianize_int32(reinterpret_cast<u32 const *>(filectx.filename)[offset]);
}

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_IMAGE, device_nubus_card_interface, nubus_image_device, "nb_image", "NuBus Disk Image Pseudo-Card")
