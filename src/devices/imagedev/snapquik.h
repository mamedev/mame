// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    snapquik.h

    Snapshots and quickloads

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_SNAPQUIK_H
#define MAME_DEVICES_IMAGEDEV_SNAPQUIK_H

#pragma once

#include "softlist_dev.h"

// ======================> snapshot_image_device
class snapshot_image_device :   public device_t,
								public device_image_interface
{
public:
	typedef device_delegate<image_init_result (device_image_interface &)> load_delegate;

	// construction/destruction
	snapshot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, const char* extensions, attotime delay = attotime::zero)
		: snapshot_image_device(mconfig, tag, owner, 0U)
	{
		set_extensions(extensions);
		set_delay(delay);
	}
	snapshot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~snapshot_image_device();

	void set_interface(const char *interface) { m_interface = interface; }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual iodevice_t image_type() const noexcept override { return IO_SNAPSHOT; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return m_file_extensions; }

	void set_extensions(const char *ext) { m_file_extensions = ext; }
	void set_delay(attotime delay) { m_delay = delay; }
	template <typename... T> void set_load_callback(T &&... args) { m_load.set(std::forward<T>(args)...); }

protected:
	snapshot_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return image_software_list_loader::instance(); }

	TIMER_CALLBACK_MEMBER(process_snapshot_or_quickload);

	load_delegate   m_load;             /* loading function */
	const char *    m_file_extensions;  /* file extensions */
	const char *    m_interface;
	attotime        m_delay;            /* loading delay */
	emu_timer       *m_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(SNAPSHOT, snapshot_image_device)

// ======================> quickload_image_device

class quickload_image_device : public snapshot_image_device
{
public:
	// construction/destruction
	quickload_image_device(const machine_config &mconfig, const char *tag, device_t *owner, const char* extensions, attotime delay = attotime::zero)
		: quickload_image_device(mconfig, tag, owner, 0U)
	{
		set_extensions(extensions);
		set_delay(delay);
	}
	quickload_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	virtual iodevice_t image_type() const noexcept override { return IO_QUICKLOAD; }
};

// device type definition
DECLARE_DEVICE_TYPE(QUICKLOAD, quickload_image_device)

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define SNAPSHOT_LOAD_MEMBER(_name)                 image_init_result _name(device_image_interface &image)
#define DECLARE_SNAPSHOT_LOAD_MEMBER(_name)         SNAPSHOT_LOAD_MEMBER(_name)

#define QUICKLOAD_LOAD_MEMBER(_name)                image_init_result _name(device_image_interface &image)
#define DECLARE_QUICKLOAD_LOAD_MEMBER(_name)        QUICKLOAD_LOAD_MEMBER(_name)

#endif // MAME_DEVICES_IMAGEDEV_SNAPQUIK_H
