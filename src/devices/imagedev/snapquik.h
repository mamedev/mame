// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    snapquik.h

    Snapshots and quickloads

*********************************************************************/

#ifndef __SNAPQUIK_H__
#define __SNAPQUIK_H__

typedef delegate<int (device_image_interface &,const char *, int)> snapquick_load_delegate;

// ======================> snapshot_image_device
class snapshot_image_device :   public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	snapshot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	snapshot_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~snapshot_image_device();

	static void static_set_interface(device_t &device, const char *_interface) { downcast<snapshot_image_device &>(device).m_interface = _interface; }

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }
	virtual iodevice_t image_type() const { return IO_SNAPSHOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return m_file_extensions; }
	virtual const option_guide *create_option_guide() const { return nullptr; }

	TIMER_CALLBACK_MEMBER(process_snapshot_or_quickload);
	void set_handler(snapquick_load_delegate load, const char *ext, seconds_t sec) { m_load = load; m_file_extensions = ext; m_delay_seconds = sec; };
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	snapquick_load_delegate m_load;                 /* loading function */
	const char *        m_file_extensions;      /* file extensions */
	const char *        m_interface;
	seconds_t           m_delay_seconds;        /* loading delay (seconds) */
	attoseconds_t       m_delay_attoseconds;    /* loading delay (attoseconds) */
	emu_timer           *m_timer;
};

// device type definition
extern const device_type SNAPSHOT;

// ======================> quickload_image_device

class quickload_image_device : public snapshot_image_device
{
public:
	// construction/destruction
	quickload_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual iodevice_t image_type() const { return IO_QUICKLOAD; }
};

// device type definition
extern const device_type QUICKLOAD;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define SNAPSHOT_LOAD_MEMBER_NAME(_name)           snapshot_load_##_name
#define SNAPSHOT_LOAD_NAME(_class,_name)           _class::SNAPSHOT_LOAD_MEMBER_NAME(_name)
#define DECLARE_SNAPSHOT_LOAD_MEMBER(_name)        int SNAPSHOT_LOAD_MEMBER_NAME(_name)(device_image_interface &image, const char *file_type, int snapshot_size)
#define SNAPSHOT_LOAD_MEMBER(_class,_name)         int SNAPSHOT_LOAD_NAME(_class,_name)(device_image_interface &image, const char *file_type, int snapshot_size)
#define SNAPSHOT_LOAD_DELEGATE(_class,_name)       snapquick_load_delegate(&SNAPSHOT_LOAD_NAME(_class,_name),#_class "::snapshot_load_" #_name, downcast<_class *>(device->owner()))

#define QUICKLOAD_LOAD_MEMBER_NAME(_name)           quickload_load##_name
#define QUICKLOAD_LOAD_NAME(_class,_name)           _class::QUICKLOAD_LOAD_MEMBER_NAME(_name)
#define DECLARE_QUICKLOAD_LOAD_MEMBER(_name)        int QUICKLOAD_LOAD_MEMBER_NAME(_name)(device_image_interface &image, const char *file_type, int quickload_size)
#define QUICKLOAD_LOAD_MEMBER(_class,_name)         int QUICKLOAD_LOAD_NAME(_class,_name)(device_image_interface &image, const char *file_type, int quickload_size)
#define QUICKLOAD_LOAD_DELEGATE(_class,_name)       snapquick_load_delegate(&QUICKLOAD_LOAD_NAME(_class,_name),#_class "::quickload_load_" #_name, downcast<_class *>(device->owner()))

#define MCFG_SNAPSHOT_ADD(_tag, _class, _load, _file_extensions, _delay) \
	MCFG_DEVICE_ADD(_tag, SNAPSHOT, 0) \
	static_cast<snapshot_image_device *>(device)->set_handler(SNAPSHOT_LOAD_DELEGATE(_class,_load), _file_extensions, _delay);

#define MCFG_SNAPSHOT_INTERFACE(_interface)                         \
	snapshot_image_device::static_set_interface(*device, _interface);

#define MCFG_QUICKLOAD_ADD(_tag, _class, _load, _file_extensions, _delay)   \
	MCFG_DEVICE_ADD(_tag, QUICKLOAD, 0) \
	static_cast<quickload_image_device *>(device)->set_handler(QUICKLOAD_LOAD_DELEGATE(_class,_load), _file_extensions, _delay);

#define MCFG_QUICKLOAD_INTERFACE(_interface)                         \
	quickload_image_device::static_set_interface(*device, _interface);

#endif /* __SNAPQUIK_H__ */
