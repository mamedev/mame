// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    snapquik.h

    Snapshots and quickloads

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_SNAPQUIK_H
#define MAME_DEVICES_IMAGEDEV_SNAPQUIK_H

#pragma once

#include <string>
#include <system_error>
#include <utility>


// ======================> snapshot_image_device
class snapshot_image_device :   public device_t,
								public device_image_interface
{
public:
	typedef device_delegate<std::pair<std::error_condition, std::string> (snapshot_image_device &)> load_delegate;

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

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return m_file_extensions; }
	virtual const char *image_type_name() const noexcept override { return "snapshot"; }
	virtual const char *image_brief_type_name() const noexcept override { return "dump"; }

	void set_extensions(const char *ext) { m_file_extensions = ext; }
	void set_delay(attotime delay) { m_delay = delay; }
	template <typename... T> void set_load_callback(T &&... args) { m_load.set(std::forward<T>(args)...); }

	template <typename Format, typename... Params> void message(Format &&fmt, Params &&...args)
	{
		// format the message
		show_message(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

protected:
	snapshot_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

	TIMER_CALLBACK_MEMBER(process_snapshot_or_quickload);

	void show_message(util::format_argument_pack<char> const &args);

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

	virtual const char *image_type_name() const noexcept override { return "quickload"; }
	virtual const char *image_brief_type_name() const noexcept override { return "quik"; }
};

// device type definition
DECLARE_DEVICE_TYPE(QUICKLOAD, quickload_image_device)

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define SNAPSHOT_LOAD_MEMBER(_name)                 std::pair<std::error_condition, std::string> _name(snapshot_image_device &image)
#define DECLARE_SNAPSHOT_LOAD_MEMBER(_name)         SNAPSHOT_LOAD_MEMBER(_name)

#define QUICKLOAD_LOAD_MEMBER(_name)                std::pair<std::error_condition, std::string> _name(snapshot_image_device &image)
#define DECLARE_QUICKLOAD_LOAD_MEMBER(_name)        QUICKLOAD_LOAD_MEMBER(_name)

#endif // MAME_DEVICES_IMAGEDEV_SNAPQUIK_H
