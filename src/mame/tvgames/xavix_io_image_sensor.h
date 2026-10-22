// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_IMAGE_SENSOR_H
#define MAME_TVGAMES_XAVIX_IO_IMAGE_SENSOR_H

#pragma once

#include <array>
#include <utility>

class picture_image_device;

class xavix_image_sensor_device_base : public device_t
{
public:
	enum class target_shape { DISK, ROD };

protected:
	static constexpr unsigned IMAGE_WIDTH = 32;
	static constexpr unsigned IMAGE_HEIGHT = 32;

	std::array<u8, IMAGE_WIDTH * IMAGE_HEIGHT> m_image;

	xavix_image_sensor_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
	{ }

	void render_target(u8 x, u8 y, u8 size, u8 angle = 0, target_shape shape = target_shape::DISK, u8 brightness = 144);
};

// Timed, parallel-command image sensor with an analogue pixel output.
class xavix_analog_image_sensor_device_base : public xavix_image_sensor_device_base
{
public:
	enum class event_source { NONE, FRAME_START, FRAME_END, ROW_START, EXPOSURE_START };

	void set_half_pixel_clocks(u32 clocks) { set_pixel_clocks(clocks, clocks); }
	void set_pixel_clocks(u32 low_clocks, u32 high_clocks) { m_pixel_low_clocks = low_clocks; m_pixel_high_clocks = high_clocks; }
	void set_frame_gap_pixel_high(bool high) { m_frame_gap_pixel_high = high; }
	void set_exposure_period(attotime period) { m_exposure_period = period; }
	void set_event_source(event_source source) { m_event_source = source; }
	template <typename... T> void set_event_callback(T &&... args) { m_event_cb.set(std::forward<T>(args)...); }

	void setup_w(u8 data, u8 direction);
	void control_w(u8 data, u8 direction);
	u8 status_r() const;
	u8 pixel_r() const;

protected:
	xavix_analog_image_sensor_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 half_pixel_clocks);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void capture_image() = 0;
	bool illuminated() const { return BIT(m_control, 5); }

private:
	// TODO: Measure the sensor oscillator, pixel period and exposure-time register units.
	static constexpr unsigned FRAME_GAP_CLOCKS = 2048;

	u32 m_pixel_low_clocks;
	u32 m_pixel_high_clocks;
	attotime m_exposure_period;
	bool m_frame_gap_pixel_high;
	event_source m_event_source;
	device_delegate<void ()> m_event_cb;
	emu_timer *m_scan_timer;
	std::array<u8, 16> m_registers;
	u8 m_setup_bus;
	u8 m_control;
	u8 m_register_select;
	u8 m_register_data;
	u8 m_status;
	u8 m_pixel;
	u8 m_row;
	u8 m_column;
	bool m_running;
	std::array<u8, IMAGE_WIDTH * IMAGE_HEIGHT> m_exposure_scene;
	std::array<double, IMAGE_WIDTH * IMAGE_HEIGHT> m_exposure_sum;
	attotime m_exposure_last_update;
	attotime m_exposure_end;
	bool m_exposing;

	void command_w(u8 command);
	void stop_scan();
	void begin_exposure();
	void sample_exposure_scene();
	void integrate_exposure();
	void finish_exposure();
	TIMER_CALLBACK_MEMBER(scan_tick);
};

class xavix_analog_image_sensor_device : public xavix_analog_image_sensor_device_base
{
public:
	static constexpr feature_type imperfect_features() { return feature::CAMERA; }

	enum class controller_type { GLOVES, REFLECTORS, ARM_COVERS };

	xavix_analog_image_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	void set_target_count(unsigned count) { m_target_count = count; }
	void set_controller_type(controller_type type) { m_controller_type = type; }
	void set_target_shape(target_shape shape) { m_shape = shape; }
	void set_target_size_small(bool small) { m_target_size_small = small; }
	void set_target_y_clipped(bool clipped) { m_target_y_clipped = clipped; }

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void capture_image() override;

private:
	static constexpr unsigned MAX_TARGETS = 4;

	optional_ioport_array<MAX_TARGETS> m_target_x;
	optional_ioport_array<MAX_TARGETS> m_target_y;
	optional_ioport_array<MAX_TARGETS> m_target_size;
	optional_ioport_array<MAX_TARGETS> m_target_hidden;
	required_ioport m_target_angle;
	unsigned m_target_count;
	controller_type m_controller_type;
	target_shape m_shape;
	bool m_target_size_small;
	bool m_target_y_clipped;
};

class xavix_digital_image_sensor_device : public xavix_image_sensor_device_base
{
public:
	static constexpr feature_type imperfect_features() { return feature::CAMERA; }

	xavix_digital_image_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~xavix_digital_image_sensor_device() override;

	u8 data_r() const;
	void data_w(offs_t offset, u8 data, u8 mem_mask);
	void strobe_w(offs_t offset, u8 data, u8 mem_mask);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	static constexpr u8 WRITE_STROBE = 0x80;
	static constexpr u8 READ_STROBE = 0x40;

	required_device<picture_image_device> m_picture;
	required_ioport_array<2> m_glove_x;
	required_ioport_array<2> m_glove_y;
	required_ioport_array<2> m_glove_size;
	required_ioport_array<2> m_glove_hidden;
	util::notifier_subscription m_media_notifier;
	bool m_source_valid;

	std::array<u8, 4> m_setup;
	u8 m_data;
	u8 m_data_mask;
	u8 m_strobes;
	u8 m_control;
	u8 m_row;
	u8 m_column;
	bool m_frame_active;
	bool m_row_active;
	bool m_read_active;

	bool validate_picture();
	void capture_image();
	void command_w(u8 data);
};

// Host-clocked optical tracking reports, rather than a raw pixel stream.
class xavix_serial_tracking_sensor_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::CAMERA; }

	xavix_serial_tracking_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	int data_r() const { return m_data; }
	void clock_w(int state);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport_array<2> m_target_x;
	required_ioport_array<2> m_target_y;
	required_ioport_array<2> m_target_size;
	required_ioport m_target_hidden;

	std::array<u8, 6> m_report;
	u8 m_data;
	u8 m_bit_count;
	bool m_clock;

	void latch_report();
};

DECLARE_DEVICE_TYPE(XAVIX_DIGITAL_IMAGE_SENSOR, xavix_digital_image_sensor_device)
DECLARE_DEVICE_TYPE(XAVIX_ANALOG_IMAGE_SENSOR, xavix_analog_image_sensor_device)
DECLARE_DEVICE_TYPE(XAVIX_SERIAL_TRACKING_SENSOR, xavix_serial_tracking_sensor_device)

#endif // MAME_TVGAMES_XAVIX_IO_IMAGE_SENSOR_H
