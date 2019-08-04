// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, Miodrag Milanovic
/*********************************************************************

    harddriv.h

    Interface to the CHD code

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_HARDDRIV_H
#define MAME_DEVICES_IMAGEDEV_HARDDRIV_H

#include "harddisk.h"
#include "softlist_dev.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> harddisk_image_device

class harddisk_image_device :   public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	harddisk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intf)
		: harddisk_image_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_interface(intf);
	}
	harddisk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~harddisk_image_device();

	template <typename... T> void set_device_load(T &&... args) { m_device_image_load = load_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_device_unload(T &&... args) { m_device_image_unload = unload_delegate(std::forward<T>(args)...); }
	void set_interface(const char *interface) { m_interface = interface; }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int create_format, util::option_resolution *create_args) override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	virtual iodevice_t image_type() const override { return IO_HARDDISK; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *image_interface() const override { return m_interface; }
	virtual const char *file_extensions() const override { return "chd,hd,hdv,2mg"; }
	virtual const util::option_guide &create_option_guide() const override;

	// specific implementation
	hard_disk_file *get_hard_disk_file() { return m_hard_disk_handle; }
	chd_file *get_chd_file();

protected:
	harddisk_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_stop() override;

	image_init_result internal_load_hd();

	chd_file        *m_chd;
	chd_file        m_origchd;              /* handle to the original CHD */
	chd_file        m_diffchd;              /* handle to the diff CHD */
	hard_disk_file  *m_hard_disk_handle;

	load_delegate   m_device_image_load;
	unload_delegate m_device_image_unload;
	const char *    m_interface;
};

// device type definition
DECLARE_DEVICE_TYPE(HARDDISK, harddisk_image_device)

#endif // MAME_DEVICES_IMAGEDEV_HARDDRIV_H
