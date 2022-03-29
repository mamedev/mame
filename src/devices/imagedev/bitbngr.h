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
	void set_interface(const char *interface) { m_interface = interface; }
	void set_readonly(bool is_readonly) { m_is_readonly = is_readonly; }

	// construction/destruction
	bitbanger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	// image device
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return !m_is_readonly; }
	virtual bool is_creatable() const noexcept override { return !m_is_readonly; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return !m_is_readonly; }
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return ""; }
	virtual const char *image_type_name() const noexcept override { return "bitbanger"; }
	virtual const char *image_brief_type_name() const noexcept override { return "bitb"; }

	void output(uint8_t data);
	uint32_t input(void *buffer, uint32_t length);

protected:
	// device_t implementation
	virtual void device_start() override;

	// device_image_interface implementation
	virtual software_list_loader const &get_software_list_loader() const override;

private:
	char const *m_interface;
	bool m_is_readonly;
};


// device type definition
DECLARE_DEVICE_TYPE(BITBANGER, bitbanger_device)

#endif // MAME_DEVICES_IMAGEDEV_BITBNGR_H
