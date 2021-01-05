// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus_image.c - synthetic NuBus card to allow reading/writing "raw"
  HFS images, including floppy images (DD and HD) and vMac/Basilisk HDD
  volumes up to 256 MB in size.

***************************************************************************/

#include "emu.h"
#include "nubus_image.h"
#include "osdcore.h"

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
	virtual const char *file_extensions() const noexcept override { return "img,dsk"; }
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
DEFINE_DEVICE_TYPE_NS(MESSIMG_DISK, nubus_image_device, messimg_disk_image_device, "messimg_disk_image", "Mac image")

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

	if (exists() && fseek(0, SEEK_END) == 0)
	{
		m_size = (uint32_t)ftell();
	}
}

image_init_result nubus_image_device::messimg_disk_image_device::call_load()
{
	fseek(0, SEEK_END);
	m_size = (uint32_t)ftell();
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
	nubus().install_device(superslotspace, superslotspace+((256*1024*1024)-1), read32s_delegate(*this, FUNC(nubus_image_device::image_super_r)), write32s_delegate(*this, FUNC(nubus_image_device::image_super_w)));

	m_image = subdevice<messimg_disk_image_device>(IMAGE_DISK0_TAG);
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