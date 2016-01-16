// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/****************************************************************************

    printer.h

    Code for handling printer devices

****************************************************************************/

#ifndef __PRINTER_H__
#define __PRINTER_H__

#define MCFG_PRINTER_ONLINE_CB(_devcb) \
	devcb = &printer_image_device::set_online_callback(*device, DEVCB_##_devcb);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> printer_image_device

class printer_image_device : public device_t,
	public device_image_interface
{
public:
	// construction/destruction
	printer_image_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_online_callback(device_t &device, _Object object) { return downcast<printer_image_device &>(device).m_online_cb.set_callback(object); }

	// image-level overrides
	virtual bool call_load() override;
	virtual bool call_create(int format_type, option_resolution *format_options) override;
	virtual void call_unload() override;

	// image device
	virtual iodevice_t image_type() const override { return IO_PRINTER; }
	virtual bool is_readable()  const override { return 0; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *file_extensions() const override { return "prn"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	// specific implementation

	/* checks to see if a printer is ready */
	int is_ready();
	/* outputs data to a printer */
	void output(UINT8 data);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	devcb_write_line m_online_cb;
};


// device type definition
extern const device_type PRINTER;

#endif /* __PRINTER_H__ */
