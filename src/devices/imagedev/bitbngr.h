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
	bitbanger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	// image device
	virtual iodevice_t image_type() const override { return IO_SERIAL; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return ""; }

	void output(uint8_t data);
	uint32_t input(void *buffer, uint32_t length);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;
};

// device type definition
extern const device_type BITBANGER;

#endif /* __BITBNGR_H__ */
