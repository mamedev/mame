/*********************************************************************

    snapquik.h

    Snapshots and quickloads

*********************************************************************/

#ifndef __SNAPQUIK_H__
#define __SNAPQUIK_H__

typedef int (*snapquick_load_func)(device_image_interface &image, const char *file_type, int file_size);

// ======================> snapshot_image_device
class snapshot_image_device :	public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	snapshot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	snapshot_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	virtual ~snapshot_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual iodevice_t image_type() const { return IO_SNAPSHOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return m_file_extensions; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	void timer_callback();
	void set_handler(snapquick_load_func load, const char *ext, seconds_t sec) { m_load = load; m_file_extensions = ext; m_delay_seconds = sec; };
protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();

	snapquick_load_func	m_load;					/* loading function */
	const char *		m_file_extensions;		/* file extensions */
	seconds_t			m_delay_seconds;		/* loading delay (seconds) */
	attoseconds_t		m_delay_attoseconds;	/* loading delay (attoseconds) */
	emu_timer			*m_timer;
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
#define SNAPSHOT_LOAD_NAME(name)	snapshot_load_##name
#define SNAPSHOT_LOAD(name)			int SNAPSHOT_LOAD_NAME(name)(device_image_interface &image, const char *file_type, int snapshot_size)

#define QUICKLOAD_LOAD_NAME(name)	quickload_load_##name
#define QUICKLOAD_LOAD(name)		int QUICKLOAD_LOAD_NAME(name)(device_image_interface &image, const char *file_type, int quickload_size)


#define MCFG_SNAPSHOT_ADD(_tag, _load, _file_extensions, _delay) \
	MCFG_DEVICE_ADD(_tag, SNAPSHOT, 0) \
	static_cast<snapshot_image_device *>(device)->set_handler(SNAPSHOT_LOAD_NAME(_load), _file_extensions, _delay);

#define MCFG_QUICKLOAD_ADD(_tag, _load, _file_extensions, _delay)	\
	MCFG_DEVICE_ADD(_tag, QUICKLOAD, 0) \
	static_cast<quickload_image_device *>(device)->set_handler(QUICKLOAD_LOAD_NAME(_load), _file_extensions, _delay);

#endif /* __SNAPQUIK_H__ */
