// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    cassette.h

    Interface to the cassette image abstraction code

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_CASSETTE_H
#define MAME_DEVICES_IMAGEDEV_CASSETTE_H

#include "formats/cassimg.h"


enum cassette_state
{
	// this part of the state is controlled by the UI
	CASSETTE_STOPPED            = 0,
	CASSETTE_PLAY               = 1,
	CASSETTE_RECORD             = 2,

	// this part of the state is controlled by drivers
	CASSETTE_MOTOR_ENABLED      = 0,
	CASSETTE_MOTOR_DISABLED     = 4,
	CASSETTE_SPEAKER_ENABLED    = 0,
	CASSETTE_SPEAKER_MUTED      = 8,

	// masks
	CASSETTE_MASK_UISTATE       = 3,
	CASSETTE_MASK_MOTOR         = 4,
	CASSETTE_MASK_SPEAKER       = 8,
	CASSETTE_MASK_DRVSTATE      = 12
};

DECLARE_ENUM_BITWISE_OPERATORS(cassette_state)


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

	void set_formats(const cassette_image::Format*  const *formats) { m_formats = formats; }
	void set_create_opts(const cassette_image::Options  *create_opts) { m_create_opts = create_opts; }
	void set_default_state(cassette_state default_state) { m_default_state = default_state; }
	void set_interface(const char *interface) { m_interface = interface; }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual std::string call_display() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return m_extension_list; }
	virtual const char *image_type_name() const noexcept override { return "cassette"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cass"; }

	double input();
	void output(double value);

	// specific implementation
	cassette_state get_state() { return m_state; }
	void set_state(cassette_state state) { change_state(state, cassette_state(~0)); }
	void change_state(cassette_state state, cassette_state mask);

	// state getters/setters
	bool is_stopped() { return (m_state & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED; }
	bool is_playing() { return (m_state & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY; }
	bool is_recording() { return (m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD; }

	void set_motor(bool state) { change_state(state ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR); } // aka remote control
	bool motor_on() { return (m_state & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED; }
	void set_speaker(bool state) { change_state(state ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED, CASSETTE_MASK_SPEAKER); }
	bool speaker_on() { return (m_state & CASSETTE_MASK_SPEAKER) == CASSETTE_SPEAKER_ENABLED; }

	cassette_image *get_image() { return m_cassette.get(); }
	double get_position();
	double get_length();
	void set_speed(double speed);
	void set_channel(int channel);
	void go_forward();
	void go_reverse();
	void seek(double time, int origin);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	device_sound_interface& set_stereo() { m_stereo = true; return *this; }

protected:
	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual bool use_software_list_file_extension_for_filetype() const noexcept override { return true; }

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

	void update();

private:
	cassette_image::ptr m_cassette;
	cassette_state  m_state;
	double          m_position;
	double          m_position_time;
	int32_t         m_value;
	int             m_channel;
	double          m_speed; // speed multiplier for tape speeds other than standard 1.875ips (used in adam driver)
	int             m_direction; // direction select
	char            m_extension_list[256];
	const cassette_image::Format*    const *m_formats;
	const cassette_image::Options    *m_create_opts;
	cassette_state                  m_default_state;
	const char *                    m_interface;

	std::error_condition internal_load(bool is_create);
	bool has_any_extension(std::string_view candidate_extensions) const;
	bool            m_stereo;
	std::vector<s16> m_samples;
};

// device type definition
DECLARE_DEVICE_TYPE(CASSETTE, cassette_image_device)

// device iterator
typedef device_type_enumerator<cassette_image_device> cassette_device_enumerator;

#endif // MAME_DEVICES_IMAGEDEV_CASSETTE_H
