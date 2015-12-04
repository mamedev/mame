// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    bitbngr.h

*********************************************************************/

#ifndef __BITBNGR_H__
#define __BITBNGR_H__

class bitbanger_device : public device_t,
	public device_image_interface
{
public:
	// construction/destruction
	bitbanger_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual bool call_load();
	virtual bool call_create(int format_type, option_resolution *format_options);
	virtual void call_unload();

	// image device
	virtual iodevice_t image_type() const { return IO_SERIAL; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *file_extensions() const { return ""; }
	virtual const option_guide *create_option_guide() const { return nullptr; }

	void output(UINT8 data);
	UINT32 input(void *buffer, UINT32 length);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
};

// device type definition
extern const device_type BITBANGER;

#endif /* __BITBNGR_H__ */
