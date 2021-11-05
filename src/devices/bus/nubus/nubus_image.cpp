// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus_image.c - synthetic NuBus card to allow reading/writing "raw"
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

#include <algorithm>


#define IMAGE_ROM_REGION    "image_rom"
#define IMAGE_DISK0_TAG     "nb_disk"

#define MESSIMG_DISK_SECTOR_SIZE (512)


// nubus_image_device::messimg_disk_image_device

class nubus_image_device::messimg_disk_image_device : public device_t, public device_image_interface
{
public:
	// construction/destruction
	messimg_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual iodevice_t image_type() const noexcept override { return IO_QUICKLOAD; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "img"; }
	virtual const char *custom_instance_name() const noexcept override { return "disk"; }
	virtual const char *custom_brief_instance_name() const noexcept override { return "disk"; }

	virtual image_init_result call_load() override;
	virtual void call_unload() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

public:
	uint32_t m_size;
	std::unique_ptr<uint8_t[]> m_data;
	bool m_ejected;
};


// device type definition
DEFINE_DEVICE_TYPE(MESSIMG_DISK, nubus_image_device::messimg_disk_image_device, "messimg_disk_image", "Mac image")

nubus_image_device::messimg_disk_image_device::messimg_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MESSIMG_DISK, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_size(0), m_data(nullptr), m_ejected(false)
{
}


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void nubus_image_device::messimg_disk_image_device::device_start()
{
	m_data = nullptr;

	if (exists() && !fseek(0, SEEK_END))
	{
		m_size = uint32_t(ftell());
	}
}

image_init_result nubus_image_device::messimg_disk_image_device::call_load()
{
	fseek(0, SEEK_END);
	m_size = uint32_t(ftell());
	if (m_size > (256*1024*1024))
	{
		osd_printf_error("Mac image too large: must be 256MB or less!\n");
		m_size = 0;
		return image_init_result::FAIL;
	}

	fseek(0, SEEK_SET);
	fread(m_data, m_size);
	m_ejected = false;

	return image_init_result::PASS;
}

void nubus_image_device::messimg_disk_image_device::call_unload()
{
	// TODO: track dirty sectors and only write those
	fseek(0, SEEK_SET);
	fwrite(m_data.get(), m_size);
	m_size = 0;
	//free(m_data);
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void nubus_image_device::messimg_disk_image_device::device_reset()
{
}

ROM_START( image )
	ROM_REGION(0x2000, IMAGE_ROM_REGION, 0)
	ROM_LOAD( "nb_fake.bin",  0x000000, 0x002000, CRC(9264bac5) SHA1(540c2ce3c90382b2da6e1e21182cdf8fc3f0c930) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_IMAGE, nubus_image_device, "nb_image", "NuBus Disk Image Pseudo-Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_image_device::device_add_mconfig(machine_config &config)
{
	MESSIMG_DISK(config, IMAGE_DISK0_TAG, 0);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_image_device::device_rom_region() const
{
	return ROM_NAME( image );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_image_device - constructor
//-------------------------------------------------

nubus_image_device::nubus_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_image_device(mconfig, NUBUS_IMAGE, tag, owner, clock)
{
}

nubus_image_device::nubus_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_image(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_image_device::device_start()
{
	uint32_t slotspace;
	uint32_t superslotspace;

	install_declaration_rom(this, IMAGE_ROM_REGION);

	slotspace = get_slotspace();
	superslotspace = get_super_slotspace();

//  printf("[image %p] slotspace = %x, super = %x\n", this, slotspace, superslotspace);

	nubus().install_device(slotspace, slotspace+3, read32smo_delegate(*this, FUNC(nubus_image_device::image_r)), write32smo_delegate(*this, FUNC(nubus_image_device::image_w)));
	nubus().install_device(slotspace+4, slotspace+7, read32smo_delegate(*this, FUNC(nubus_image_device::image_status_r)), write32smo_delegate(*this, FUNC(nubus_image_device::image_status_w)));
	nubus().install_device(slotspace+8, slotspace+11, read32smo_delegate(*this, FUNC(nubus_image_device::file_cmd_r)), write32smo_delegate(*this, FUNC(nubus_image_device::file_cmd_w)));
	nubus().install_device(slotspace+12, slotspace+15, read32smo_delegate(*this, FUNC(nubus_image_device::file_data_r)), write32smo_delegate(*this, FUNC(nubus_image_device::file_data_w)));
	nubus().install_device(slotspace+16, slotspace+19, read32smo_delegate(*this, FUNC(nubus_image_device::file_len_r)), write32smo_delegate(*this, FUNC(nubus_image_device::file_len_w)));
	nubus().install_device(slotspace+20, slotspace+147, read32sm_delegate(*this, FUNC(nubus_image_device::file_name_r)), write32sm_delegate(*this, FUNC(nubus_image_device::file_name_w)));
	nubus().install_device(superslotspace, superslotspace+((256*1024*1024)-1), read32s_delegate(*this, FUNC(nubus_image_device::image_super_r)), write32s_delegate(*this, FUNC(nubus_image_device::image_super_w)));

	m_image = subdevice<messimg_disk_image_device>(IMAGE_DISK0_TAG);

	filectx.curdir = ".";
	filectx.dirp.reset();
	filectx.fd.reset();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_image_device::device_reset()
{
}

void nubus_image_device::image_status_w(uint32_t data)
{
	m_image->m_ejected = true;
}

uint32_t nubus_image_device::image_status_r()
{
	if(m_image->m_ejected) {
		return 0;
	}

	if(m_image->m_size) {
		return 1;
	}
	return 0;
}

void nubus_image_device::image_w(uint32_t data)
{
}

uint32_t nubus_image_device::image_r()
{
	return m_image->m_size;
}

void nubus_image_device::image_super_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t *image = (uint32_t*)m_image->m_data.get();
	data = ((data & 0xff) << 24) | ((data & 0xff00) << 8) | ((data & 0xff0000) >> 8) | ((data & 0xff000000) >> 24);
	mem_mask = ((mem_mask & 0xff) << 24) | ((mem_mask & 0xff00) << 8) | ((mem_mask & 0xff0000) >> 8) | ((mem_mask & 0xff000000) >> 24);

	COMBINE_DATA(&image[offset]);
}

uint32_t nubus_image_device::image_super_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t *image = (uint32_t*)m_image->m_data.get();
	uint32_t data = image[offset];
	return ((data & 0xff) << 24) | ((data & 0xff00) << 8) | ((data & 0xff0000) >> 8) | ((data & 0xff000000) >> 24);
}

void nubus_image_device::file_cmd_w(uint32_t data)
{
//  data = ((data & 0xff) << 24) | ((data & 0xff00) << 8) | ((data & 0xff0000) >> 8) | ((data & 0xff000000) >> 24);
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
			uint64_t filesize; // unused, but it's an output from the open call
			std::error_condition const filerr = osd_file::open(fullpath, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, filectx.fd, filesize);
			if (filerr)
				osd_printf_error("%s: Error opening %s (%s:%d %s)\n", tag(), fullpath, filerr.category().name(), filerr.value(), filerr.message());
			filectx.bytecount = 0;
		}
		break;
	}
}

uint32_t nubus_image_device::file_cmd_r()
{
	return 0;
}

void nubus_image_device::file_data_w(uint32_t data)
{
	std::uint32_t count = 4;
	std::uint32_t actualcount = 0;

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

uint32_t nubus_image_device::file_data_r()
{
	if(filectx.fd) {
		std::uint32_t ret;
		std::uint32_t actual = 0;
		filectx.fd->read(&ret, filectx.bytecount, sizeof(ret), actual);
		filectx.bytecount += actual;
		if(actual < sizeof(ret)) {
			filectx.fd.reset();
		}
		return big_endianize_int32(ret);
	}
	return 0;
}

void nubus_image_device::file_len_w(uint32_t data)
{
	data = swapendian_int32(data);
	filectx.filelen = big_endianize_int32(data);
}

uint32_t nubus_image_device::file_len_r()
{
	return filectx.filelen;
}

void nubus_image_device::file_name_w(offs_t offset, uint32_t data)
{
	reinterpret_cast<uint32_t *>(filectx.filename)[offset] = big_endianize_int32(data);
}

uint32_t nubus_image_device::file_name_r(offs_t offset)
{
	return big_endianize_int32(reinterpret_cast<uint32_t const *>(filectx.filename)[offset]);
}
