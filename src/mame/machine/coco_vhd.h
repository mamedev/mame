// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_vhd.h

    Color Computer Virtual Hard Drives

***************************************************************************/

#ifndef MAME_MACHINE_COCO_VHD_H
#define MAME_MACHINE_COCO_VHD_H

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> coco_vhd_image_device

class coco_vhd_image_device :   public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	template <typename T>
	coco_vhd_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: coco_vhd_image_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
	}
	coco_vhd_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~coco_vhd_image_device();

	// image-level overrides
	virtual image_init_result call_load() override;

	virtual iodevice_t image_type() const noexcept override { return IO_HARDDISK; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "vhd"; }

	// specific implementation
	DECLARE_READ8_MEMBER(read) { return read(offset); }
	DECLARE_WRITE8_MEMBER(write) { write(offset, data); }
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

	void coco_vhd_readwrite(uint8_t data);

private:
	required_device<cpu_device> m_cpu;
	address_space *             m_cpu_space;
	uint32_t                    m_logical_record_number;
	uint32_t                    m_buffer_address;
	uint8_t                     m_status;
};

// device type definition
DECLARE_DEVICE_TYPE(COCO_VHD, coco_vhd_image_device)

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


#endif // MAME_MACHINE_COCO_VHD_H
