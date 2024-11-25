// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_vhd.h

    Color Computer Virtual Hard Drives

***************************************************************************/

#ifndef MAME_TRS_COCO_VHD_H
#define MAME_TRS_COCO_VHD_H

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

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "vhd"; }
	virtual const char *image_type_name() const noexcept override { return "harddisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "hard"; }

	// specific implementation
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void coco_vhd_readwrite(uint8_t data);

private:
	required_device<cpu_device> m_cpu;
	address_space *             m_cpu_space = nullptr;
	uint32_t                    m_logical_record_number = 0;
	uint32_t                    m_buffer_address = 0;
	uint8_t                     m_status = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(COCO_VHD, coco_vhd_image_device)

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


#endif // MAME_TRS_COCO_VHD_H
