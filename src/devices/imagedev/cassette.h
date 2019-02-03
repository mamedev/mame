// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    cassette.h

    Interface to the cassette image abstraction code

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_CASSETTE_H
#define MAME_DEVICES_IMAGEDEV_CASSETTE_H

#include "formats/cassimg.h"
#include "softlist_dev.h"


enum cassette_state
{
	/* this part of the state is controlled by the UI */
	CASSETTE_STOPPED            = 0,
	CASSETTE_PLAY               = 1,
	CASSETTE_RECORD             = 2,

	/* this part of the state is controlled by drivers */
	CASSETTE_MOTOR_ENABLED      = 0,
	CASSETTE_MOTOR_DISABLED     = 4,
	CASSETTE_SPEAKER_ENABLED    = 0,
	CASSETTE_SPEAKER_MUTED      = 8,

	/* masks */
	CASSETTE_MASK_UISTATE       = 3,
	CASSETTE_MASK_MOTOR         = 4,
	CASSETTE_MASK_SPEAKER       = 8,
	CASSETTE_MASK_DRVSTATE      = 12
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> cassette_image_device

class cassette_image_device :   public device_t,
								public device_image_interface,
								public device_sound_interface
{
public:
	// construction/destruction
	cassette_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~cassette_image_device();

	void set_formats(const struct CassetteFormat*  const *formats) { m_formats = formats; }
	void set_create_opts(const struct CassetteOptions  *create_opts) { m_create_opts = create_opts; }
	void set_default_state(cassette_state default_state) { m_default_state = default_state; }
	void set_default_state(int default_state) { m_default_state = (cassette_state)default_state; }
	void set_interface(const char *interface) { m_interface = interface; }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual std::string call_display() override;
	virtual const software_list_loader &get_software_list_loader() const override { return image_software_list_loader::instance(); }

	virtual iodevice_t image_type() const override { return IO_CASSETTE; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *image_interface() const override { return m_interface; }
	virtual const char *file_extensions() const override { return m_extension_list; }

	// specific implementation
	cassette_state get_state() { return m_state; }
	void set_state(cassette_state state) { change_state(state, (cassette_state)(~0)); }
	void change_state(cassette_state state, cassette_state mask);

	double input();
	void output(double value);

	cassette_image *get_image() { return m_cassette; }
	double get_position();
	double get_length();
	void set_speed(double speed);
	void set_channel(int channel);
	void go_forward();
	void go_reverse();
	void seek(double time, int origin);

	// sound stream update overrides
	void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
	device_sound_interface& set_stereo() { m_stereo = true; return *this; }

protected:
	bool is_motor_on();
	void update();

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual const bool use_software_list_file_extension_for_filetype() const override { return true; }

private:
	cassette_image  *m_cassette;
	cassette_state  m_state;
	double          m_position;
	double          m_position_time;
	int32_t           m_value;
	int             m_channel;
	double          m_speed; // speed multiplier for tape speeds other than standard 1.875ips (used in adam driver)
	int             m_direction; // direction select
	char            m_extension_list[256];
	const struct CassetteFormat*    const *m_formats;
	const struct CassetteOptions    *m_create_opts;
	cassette_state                  m_default_state;
	const char *                    m_interface;

	image_init_result internal_load(bool is_create);
	bool            m_stereo;
};

// device type definition
DECLARE_DEVICE_TYPE(CASSETTE, cassette_image_device)

// device iterator
typedef device_type_iterator<cassette_image_device> cassette_device_iterator;

#endif // MAME_DEVICES_IMAGEDEV_CASSETTE_H
