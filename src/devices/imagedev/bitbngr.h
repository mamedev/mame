// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    bitbngr.h

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_BITBNGR_H
#define MAME_DEVICES_IMAGEDEV_BITBNGR_H

class bitbanger_device : public device_t,
	public device_image_interface
{
public:
	static void static_set_interface(device_t &device, const char *_interface) { downcast<bitbanger_device &>(device).m_interface = _interface; }

	// construction/destruction
	bitbanger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	// image device
	virtual iodevice_t image_type() const override { return IO_SERIAL; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *image_interface() const override { return m_interface; }
	virtual const char *file_extensions() const override { return ""; }
	virtual const char *custom_instance_name() const override { return "bitbanger"; }
	virtual const char *custom_brief_instance_name() const override { return "bitb"; }

	void output(uint8_t data);
	uint32_t input(void *buffer, uint32_t length);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	char const *m_interface;
};

#define MCFG_BITBANGER_INTERFACE(_interface) \
	bitbanger_image_device::static_set_interface(*device, _interface);

// device type definition
extern const device_type BITBANGER;

#endif // MAME_DEVICES_IMAGEDEV_BITBNGR_H
