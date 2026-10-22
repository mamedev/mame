// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviX image-sensor interfaces used by reflective motion controllers and
    duelmast's enclosed card imager. Commands program sensor registers;
    independent frame/pixel clocks expose a held analogue sample to the ADC.
    The ROM performs image subtraction and recognition.

    Reflective-controller inputs describe the target in the camera's field,
    not recognised gestures.

    ndpbj reads commands/pixels on PIO0 with active-low write/read strobes
    on PIO2 bits 7/6. Its white gloves use the shared target renderer; a mounted
    opaque 32x32 monochrome picture supplies raw bytes unchanged. The ROM
    subtracts 0x3f before tracking. The separate 40-bit panel is not part of
    the camera interface.

    udance receives host-clocked tracking reports, not raw pixels. Each 40-bit
    report contains three leading signature bits, two sets of six-bit X, Y and
    apparent-size values, then the final signature bit. A zero size represents
    a tag outside the sensing zone.
    Tracking reports have their own interface and do not use the 32x32 renderer.

    TODO:
    - Measure lens geometry, exposure response and target brightness.
    - Identify ndpbj's setup registers, exposure commands and frame latch.
    - Measure udance's sensor-side tracking, including overlapping reflectors.

***************************************************************************/

#include "emu.h"
#include "xavix_io_image_sensor.h"

#include "imagedev/picture.h"

#include <algorithm>
#include <cmath>

#define LOG_COMMAND (1U << 1)
#define LOG_PROTOCOL (1U << 2)
#define LOG_UNHANDLED (1U << 3)
#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_DIGITAL_IMAGE_SENSOR, xavix_digital_image_sensor_device, "xavix_digital_image_sensor", "XaviX 32x32 Digital Image Sensor")
DEFINE_DEVICE_TYPE(XAVIX_ANALOG_IMAGE_SENSOR, xavix_analog_image_sensor_device, "xavix_analog_image_sensor", "XaviX Analogue Image Sensor")
DEFINE_DEVICE_TYPE(XAVIX_SERIAL_TRACKING_SENSOR, xavix_serial_tracking_sensor_device, "xavix_serial_tracking_sensor", "XaviX Serial Tracking Sensor")

static INPUT_PORTS_START(xavix_analog_image_sensor)
	PORT_START("TARGET_X")
	// Club and sword cursor projections reverse the sensor's image X.
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_NAME("Reflector X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X) PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_REVERSE
	PORT_START("TARGET_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Reflector Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y) PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)

	PORT_START("TARGET_SIZE")
	// Mouse axes move the reflector; size and rotation must not inherit Mouse X.
	PORT_BIT( 0xff, 0x20, IPT_PADDLE ) PORT_NAME("Reflector Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)

	PORT_START("TARGET_HIDDEN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Reflector Outside Camera View") PORT_CODE(KEYCODE_SPACE)

	PORT_START("TARGET_ANGLE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_analog_image_sensor_small)
	PORT_INCLUDE(xavix_analog_image_sensor)

	PORT_MODIFY("TARGET_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Reflector Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_reflective_rod)
	PORT_INCLUDE(xavix_analog_image_sensor)

	PORT_MODIFY("TARGET_ANGLE")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Reflector Angle") PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_reflective_rod_clipped)
	PORT_INCLUDE(xavix_reflective_rod)

	PORT_MODIFY("TARGET_Y")
	// Keep the centre in rows 1-30 for trackers that discard the first acquired row.
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Reflector Y") PORT_MINMAX(0x08, 0xf7) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y) PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_analog_image_sensor_dual)
	PORT_INCLUDE(xavix_analog_image_sensor)

	// The ROM assigns the two reflectors from their positions in the image.
	PORT_MODIFY("TARGET_X")
	PORT_BIT( 0xff, 0x60, IPT_AD_STICK_X ) PORT_NAME("Glove 1 X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_REVERSE
	PORT_MODIFY("TARGET_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Glove 1 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)
	PORT_MODIFY("TARGET_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Glove 1 Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
	PORT_MODIFY("TARGET_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "Glove 1 Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )

	PORT_START("TARGET2_X")
	PORT_BIT( 0xff, 0xa0, IPT_AD_STICK_X ) PORT_NAME("Glove 2 X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(KEYCODE_F) PORT_CODE_INC(KEYCODE_H) PORT_REVERSE
	PORT_START("TARGET2_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Glove 2 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_G)
	PORT_START("TARGET2_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Glove 2 Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_MINUS) PORT_CODE_INC(KEYCODE_EQUALS)
	PORT_START("TARGET2_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "Glove 2 Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_digital_image_sensor)
	PORT_INCLUDE(xavix_analog_image_sensor_dual)

	// The descending image store and glove projection reverse both camera axes.
	PORT_MODIFY("TARGET_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Glove 1 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K) PORT_REVERSE
	PORT_MODIFY("TARGET2_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Glove 2 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_G) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_reflector_pair_camera)
	PORT_INCLUDE(xavix_analog_image_sensor_dual)

	PORT_MODIFY("TARGET_X")
	PORT_BIT( 0xff, 0x60, IPT_AD_STICK_X ) PORT_NAME("Reflector 1 X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_REVERSE
	PORT_MODIFY("TARGET_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Reflector 1 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)
	PORT_MODIFY("TARGET_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Reflector 1 Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
	PORT_MODIFY("TARGET_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "Reflector 1 Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )

	PORT_MODIFY("TARGET2_X")
	PORT_BIT( 0xff, 0xa0, IPT_AD_STICK_X ) PORT_NAME("Reflector 2 X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(KEYCODE_F) PORT_CODE_INC(KEYCODE_H) PORT_REVERSE
	PORT_MODIFY("TARGET2_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Reflector 2 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_G)
	PORT_MODIFY("TARGET2_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Reflector 2 Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_MINUS) PORT_CODE_INC(KEYCODE_EQUALS)
	PORT_MODIFY("TARGET2_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "Reflector 2 Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_analog_image_sensor_triple)
	PORT_INCLUDE(xavix_reflector_pair_camera)

	// The ankle reflector starts out of view; its optical size and orientation are unmeasured.
	PORT_START("TARGET3_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_NAME("Ankle Reflector X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(2)) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_REVERSE
	PORT_START("TARGET3_Y")
	PORT_BIT( 0xff, 0xff, IPT_AD_STICK_Y ) PORT_NAME("Ankle Reflector Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(2)) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_START("TARGET3_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Ankle Reflector Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_7_PAD) PORT_CODE_INC(KEYCODE_9_PAD)
	PORT_START("TARGET3_HIDDEN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Ankle Reflector in Camera View") PORT_CODE(KEYCODE_5_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_arm_cover_camera)
	PORT_INCLUDE(xavix_analog_image_sensor_dual)

	PORT_MODIFY("TARGET_X")
	PORT_BIT( 0xff, 0x60, IPT_AD_STICK_X ) PORT_NAME("Arm Cover 1 X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_REVERSE
	PORT_MODIFY("TARGET_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Arm Cover 1 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)
	PORT_MODIFY("TARGET_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Arm Cover 1 Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
	PORT_MODIFY("TARGET_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "Arm Cover 1 Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )

	PORT_MODIFY("TARGET2_X")
	PORT_BIT( 0xff, 0xa0, IPT_AD_STICK_X ) PORT_NAME("Arm Cover 2 X") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(KEYCODE_F) PORT_CODE_INC(KEYCODE_H) PORT_REVERSE
	PORT_MODIFY("TARGET2_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("Arm Cover 2 Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_G)
	PORT_MODIFY("TARGET2_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("Arm Cover 2 Size (Far / Near)") PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_MINUS) PORT_CODE_INC(KEYCODE_EQUALS)
	PORT_MODIFY("TARGET2_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "Arm Cover 2 Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_analog_image_sensor_quad)
	PORT_INCLUDE(xavix_analog_image_sensor)

	PORT_MODIFY("TARGET_X")
	// Four separated starting positions preserve the ROM's left-to-right player/hand assignment.
	PORT_BIT( 0xff, 0xe6, IPT_AD_STICK_X ) PORT_NAME("P1 Left Pom-Pom X") PORT_PLAYER(1) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_REVERSE
	PORT_MODIFY("TARGET_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("P1 Left Pom-Pom Y") PORT_PLAYER(1) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)
	PORT_MODIFY("TARGET_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("P1 Left Pom-Pom Size") PORT_PLAYER(1) PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
	PORT_MODIFY("TARGET_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "P1 Left Pom-Pom Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )

	PORT_START("TARGET2_X")
	PORT_BIT( 0xff, 0xa5, IPT_AD_STICK_X ) PORT_NAME("P1 Right Pom-Pom X") PORT_PLAYER(1) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(KEYCODE_F) PORT_CODE_INC(KEYCODE_H) PORT_REVERSE
	PORT_START("TARGET2_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("P1 Right Pom-Pom Y") PORT_PLAYER(1) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(KEYCODE_T) PORT_CODE_INC(KEYCODE_G)
	PORT_START("TARGET2_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("P1 Right Pom-Pom Size") PORT_PLAYER(1) PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_MINUS) PORT_CODE_INC(KEYCODE_EQUALS)
	PORT_START("TARGET2_HIDDEN")
	PORT_CONFNAME( 0x01, 0x00, "P1 Right Pom-Pom Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )

	PORT_START("TARGET3_X")
	PORT_BIT( 0xff, 0x63, IPT_AD_STICK_X ) PORT_NAME("P2 Left Pom-Pom X") PORT_PLAYER(2) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(2)) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_REVERSE
	PORT_START("TARGET3_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("P2 Left Pom-Pom Y") PORT_PLAYER(2) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(2)) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_START("TARGET3_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("P2 Left Pom-Pom Size") PORT_PLAYER(2) PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_7_PAD) PORT_CODE_INC(KEYCODE_9_PAD)
	PORT_START("TARGET3_HIDDEN")
	PORT_CONFNAME( 0x01, 0x01, "P2 Left Pom-Pom Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )

	PORT_START("TARGET4_X")
	PORT_BIT( 0xff, 0x21, IPT_AD_STICK_X ) PORT_NAME("P2 Right Pom-Pom X") PORT_PLAYER(2) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_X_INDEXED(3)) PORT_CODE_DEC(KEYCODE_A) PORT_CODE_INC(KEYCODE_D) PORT_REVERSE
	PORT_START("TARGET4_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_NAME("P2 Right Pom-Pom Y") PORT_PLAYER(2) PORT_SENSITIVITY(25) PORT_KEYDELTA(16) PORT_CENTERDELTA(0) PORT_CODE(MOUSECODE_Y_INDEXED(3)) PORT_CODE_DEC(KEYCODE_W) PORT_CODE_INC(KEYCODE_S)
	PORT_START("TARGET4_SIZE")
	PORT_BIT( 0xff, 0x10, IPT_PADDLE ) PORT_NAME("P2 Right Pom-Pom Size") PORT_PLAYER(2) PORT_MINMAX(0x08, 0x80) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_R) PORT_CODE_INC(KEYCODE_Y)
	PORT_START("TARGET4_HIDDEN")
	PORT_CONFNAME( 0x01, 0x01, "P2 Right Pom-Pom Visibility" )
	PORT_CONFSETTING(    0x00, "Visible" )
	PORT_CONFSETTING(    0x01, "Hidden" )
INPUT_PORTS_END

static INPUT_PORTS_START(xavix_serial_tracking_sensor)
	// udance assigns the two passive reflectors to feet by proximity to their previous positions.
	PORT_START("LEFT_X")
	PORT_BIT(0x3f, 0x33, IPT_AD_STICK_X) PORT_NAME("Left Foot X") PORT_PLAYER(1) PORT_REVERSE
		PORT_MINMAX(0x00, 0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_A) PORT_CODE_INC(KEYCODE_D)

	PORT_START("LEFT_Y")
	PORT_BIT(0x3f, 0x10, IPT_AD_STICK_Y) PORT_NAME("Left Foot Y") PORT_PLAYER(1) PORT_REVERSE
		PORT_MINMAX(0x00, 0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_W) PORT_CODE_INC(KEYCODE_S)

	PORT_START("LEFT_SIZE")
	PORT_BIT(0x3f, 0x20, IPT_AD_STICK_Z) PORT_NAME("Left Motion Tag Size / Depth") PORT_PLAYER(1)
		PORT_MINMAX(0x01, 0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_E) PORT_CODE_INC(KEYCODE_R)

	PORT_START("RIGHT_X")
	PORT_BIT(0x3f, 0x17, IPT_AD_STICK_X) PORT_NAME("Right Foot X") PORT_PLAYER(2) PORT_REVERSE
		PORT_MINMAX(0x00, 0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L)

	PORT_START("RIGHT_Y")
	PORT_BIT(0x3f, 0x11, IPT_AD_STICK_Y) PORT_NAME("Right Foot Y") PORT_PLAYER(2) PORT_REVERSE
		PORT_MINMAX(0x00, 0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K)

	PORT_START("RIGHT_SIZE")
	PORT_BIT(0x3f, 0x20, IPT_AD_STICK_Z) PORT_NAME("Right Motion Tag Size / Depth") PORT_PLAYER(2)
		PORT_MINMAX(0x01, 0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CENTERDELTA(0)
		PORT_CODE_DEC(KEYCODE_O) PORT_CODE_INC(KEYCODE_P)

	PORT_START("TAG_HIDE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hide Left Motion Tag") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hide Right Motion Tag") PORT_CODE(KEYCODE_U)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

xavix_analog_image_sensor_device_base::xavix_analog_image_sensor_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 half_pixel_clocks)
	: xavix_image_sensor_device_base(mconfig, type, tag, owner, clock)
	, m_pixel_low_clocks(half_pixel_clocks)
	, m_pixel_high_clocks(half_pixel_clocks)
	, m_exposure_period(attotime::zero)
	, m_frame_gap_pixel_high(false)
	, m_event_source(event_source::NONE)
	, m_event_cb(*this)
{
}

xavix_analog_image_sensor_device::xavix_analog_image_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	// Representative cadence allows initial edge polling and paired capture between video interrupts.
	: xavix_analog_image_sensor_device_base(mconfig, XAVIX_ANALOG_IMAGE_SENSOR, tag, owner, clock, 48)
	, m_target_x(*this, { "TARGET_X", "TARGET2_X", "TARGET3_X", "TARGET4_X" })
	, m_target_y(*this, { "TARGET_Y", "TARGET2_Y", "TARGET3_Y", "TARGET4_Y" })
	, m_target_size(*this, { "TARGET_SIZE", "TARGET2_SIZE", "TARGET3_SIZE", "TARGET4_SIZE" })
	, m_target_hidden(*this, { "TARGET_HIDDEN", "TARGET2_HIDDEN", "TARGET3_HIDDEN", "TARGET4_HIDDEN" })
	, m_target_angle(*this, "TARGET_ANGLE")
	, m_target_count(1)
	, m_controller_type(controller_type::GLOVES)
	, m_shape(target_shape::DISK)
	, m_target_size_small(false)
	, m_target_y_clipped(false)
{
}

xavix_digital_image_sensor_device::xavix_digital_image_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_image_sensor_device_base(mconfig, XAVIX_DIGITAL_IMAGE_SENSOR, tag, owner, clock)
	, m_picture(*this, "picture")
	, m_glove_x(*this, { "TARGET_X", "TARGET2_X" })
	, m_glove_y(*this, { "TARGET_Y", "TARGET2_Y" })
	, m_glove_size(*this, { "TARGET_SIZE", "TARGET2_SIZE" })
	, m_glove_hidden(*this, { "TARGET_HIDDEN", "TARGET2_HIDDEN" })
	, m_source_valid(false)
	, m_data(0)
	, m_data_mask(0)
	, m_strobes(WRITE_STROBE | READ_STROBE)
{
}

xavix_digital_image_sensor_device::~xavix_digital_image_sensor_device() = default;

xavix_serial_tracking_sensor_device::xavix_serial_tracking_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_SERIAL_TRACKING_SENSOR, tag, owner, clock)
	, m_target_x(*this, { "LEFT_X", "RIGHT_X" })
	, m_target_y(*this, { "LEFT_Y", "RIGHT_Y" })
	, m_target_size(*this, { "LEFT_SIZE", "RIGHT_SIZE" })
	, m_target_hidden(*this, "TAG_HIDE")
{
}

void xavix_digital_image_sensor_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}

ioport_constructor xavix_analog_image_sensor_device::device_input_ports() const
{
	switch (m_target_count)
	{
	case 2:
		if (m_controller_type == controller_type::REFLECTORS)
			return INPUT_PORTS_NAME(xavix_reflector_pair_camera);
		if (m_controller_type == controller_type::ARM_COVERS)
			return INPUT_PORTS_NAME(xavix_arm_cover_camera);
		return INPUT_PORTS_NAME(xavix_analog_image_sensor_dual);
	case 3:
		return INPUT_PORTS_NAME(xavix_analog_image_sensor_triple);
	case 4:
		return INPUT_PORTS_NAME(xavix_analog_image_sensor_quad);
	default:
		if (m_shape == target_shape::ROD)
			return m_target_y_clipped ? INPUT_PORTS_NAME(xavix_reflective_rod_clipped) : INPUT_PORTS_NAME(xavix_reflective_rod);
		return m_target_size_small ? INPUT_PORTS_NAME(xavix_analog_image_sensor_small) : INPUT_PORTS_NAME(xavix_analog_image_sensor);
	}
}

ioport_constructor xavix_digital_image_sensor_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_digital_image_sensor);
}

ioport_constructor xavix_serial_tracking_sensor_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_serial_tracking_sensor);
}

void xavix_analog_image_sensor_device::device_validity_check(validity_checker &valid) const
{
	xavix_analog_image_sensor_device_base::device_validity_check(valid);
	if (m_target_count < 1 || m_target_count > MAX_TARGETS)
		osd_printf_error("Image sensor target count %u is outside 1-%u\n", m_target_count, MAX_TARGETS);
}

void xavix_analog_image_sensor_device_base::device_start()
{
	m_event_cb.resolve_safe();

	assert(clock() && m_pixel_low_clocks && m_pixel_high_clocks);
	if (m_exposure_period < attotime::zero || m_exposure_period.is_never()
			|| (m_exposure_period.is_zero() && m_event_source == event_source::EXPOSURE_START)
			|| (!m_exposure_period.is_zero() && m_frame_gap_pixel_high))
		throw emu_fatalerror("%s: invalid image sensor exposure configuration", tag());
	m_scan_timer = timer_alloc(FUNC(xavix_analog_image_sensor_device_base::scan_tick), this);

	save_item(NAME(m_image));
	save_item(NAME(m_registers));
	save_item(NAME(m_setup_bus));
	save_item(NAME(m_control));
	save_item(NAME(m_register_select));
	save_item(NAME(m_register_data));
	save_item(NAME(m_status));
	save_item(NAME(m_pixel));
	save_item(NAME(m_row));
	save_item(NAME(m_column));
	save_item(NAME(m_running));
	save_item(NAME(m_exposure_scene));
	save_item(NAME(m_exposure_sum));
	save_item(NAME(m_exposure_last_update));
	save_item(NAME(m_exposure_end));
	save_item(NAME(m_exposing));
}

void xavix_analog_image_sensor_device::device_start()
{
	if (m_target_count < 1 || m_target_count > MAX_TARGETS)
		throw emu_fatalerror("%s: image sensor target count %u is outside 1-%u", tag(), m_target_count, MAX_TARGETS);
	for (unsigned target = 0; target < m_target_count; ++target)
	{
		if (!m_target_x[target].found() || !m_target_y[target].found()
				|| !m_target_size[target].found() || !m_target_hidden[target].found())
			throw emu_fatalerror("%s: missing inputs for image sensor target %u", tag(), target + 1);
	}
	xavix_analog_image_sensor_device_base::device_start();
}

void xavix_analog_image_sensor_device_base::device_reset()
{
	m_setup_bus = 0;
	m_control = 0;
	m_register_select = 0;
	m_register_data = 0;
	m_registers.fill(0);
	m_image.fill(0);
	stop_scan();
}

void xavix_digital_image_sensor_device::device_start()
{
	save_item(NAME(m_image));
	save_item(NAME(m_setup));
	save_item(NAME(m_data));
	save_item(NAME(m_data_mask));
	save_item(NAME(m_strobes));
	save_item(NAME(m_control));
	save_item(NAME(m_row));
	save_item(NAME(m_column));
	save_item(NAME(m_frame_active));
	save_item(NAME(m_row_active));
	save_item(NAME(m_read_active));

	m_media_notifier = m_picture->add_media_change_notifier(
			[this] (device_image_interface::media_change_event)
			{
				m_source_valid = validate_picture();
			});
}

void xavix_digital_image_sensor_device::device_reset()
{
	m_image.fill(0);
	m_setup.fill(0);
	m_control = 0;
	m_row = 0;
	m_column = 0;
	m_frame_active = false;
	m_row_active = false;
	m_read_active = false;
}

void xavix_digital_image_sensor_device::device_post_load()
{
	// Media validation is a cache; restoring it must not recapture the held image.
	m_source_valid = validate_picture();
}

void xavix_serial_tracking_sensor_device::device_start()
{
	save_item(NAME(m_report));
	save_item(NAME(m_data));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_clock));
}

void xavix_serial_tracking_sensor_device::device_reset()
{
	m_data = 0;
	m_bit_count = 40;
	m_clock = false;
	latch_report();
}

//-------------------------------------------------
//  Analogue Sensor Transport
//-------------------------------------------------

void xavix_analog_image_sensor_device_base::setup_w(u8 data, u8 direction)
{
	// Only the driven setup strobe clocks the seven-bit command payload.
	u8 const driven = data & direction;
	if (BIT(direction, 7) && BIT(driven, 7) && !BIT(m_setup_bus, 7) && BIT(m_control, 0))
		command_w(driven & 0x7f);
	m_setup_bus = driven;
}

void xavix_analog_image_sensor_device_base::control_w(u8 data, u8 direction)
{
	u8 const control = data & direction & 0x21;
	bool const changed = control != m_control;
	if (m_exposing && changed)
		integrate_exposure();
	if (!BIT(control, 0) && BIT(m_control, 0))
	{
		stop_scan();
		m_register_select = 0;
		m_register_data = 0;
		m_registers.fill(0);
	}
	m_control = control;
	if (m_exposing && changed)
		sample_exposure_scene();
}

u8 xavix_analog_image_sensor_device_base::status_r() const
{
	return m_status;
}

u8 xavix_analog_image_sensor_device_base::pixel_r() const
{
	return m_pixel;
}

void xavix_analog_image_sensor_device_base::command_w(u8 command)
{
	// Register select/data/commit and run/reset match the sensor interface in US7662047B2.
	switch (command & 0x70)
	{
	case 0x10:
		m_register_select = command & 0x0f;
		break;
	case 0x20:
		m_register_data = command & 0x0f;
		break;
	case 0x30:
		m_registers[m_register_select] = m_register_data;
		break;
	case 0x40:
		if (!m_running)
		{
			m_running = true;
			if (!m_exposure_period.is_zero())
			{
				begin_exposure();
			}
			else
			{
				m_status = m_frame_gap_pixel_high ? 0x04 : 0;
				m_scan_timer->adjust(clocks_to_attotime(FRAME_GAP_CLOCKS));
			}
		}
		break;
	case 0x50:
		stop_scan();
		break;
	}
}

void xavix_analog_image_sensor_device_base::stop_scan()
{
	m_scan_timer->adjust(attotime::never);
	m_running = false;
	m_status = 0;
	m_pixel = 0;
	m_row = 0;
	m_column = 0;
	m_exposure_scene.fill(0);
	m_exposure_sum.fill(0);
	m_exposure_last_update = attotime::zero;
	m_exposure_end = attotime::zero;
	m_exposing = false;
}

void xavix_analog_image_sensor_device_base::begin_exposure()
{
	m_status = 0;
	m_row = 0;
	m_column = 0;
	m_exposure_sum.fill(0);
	m_exposure_last_update = machine().time();
	m_exposure_end = m_exposure_last_update + m_exposure_period;
	m_exposing = true;
	sample_exposure_scene();
	m_scan_timer->adjust(m_exposure_period);
	if (m_event_source == event_source::EXPOSURE_START)
		m_event_cb();
}

void xavix_analog_image_sensor_device_base::sample_exposure_scene()
{
	// Approximate motion as constant between exposure and illumination changes.
	m_image.swap(m_exposure_scene);
	capture_image();
	m_image.swap(m_exposure_scene);
}

void xavix_analog_image_sensor_device_base::integrate_exposure()
{
	attotime const now = std::min(machine().time(), m_exposure_end);
	if (now <= m_exposure_last_update)
		return;
	double const weight = (now - m_exposure_last_update).as_double() / m_exposure_period.as_double();
	for (unsigned pixel = 0; pixel < m_exposure_sum.size(); ++pixel)
		m_exposure_sum[pixel] += double(m_exposure_scene[pixel]) * weight;
	m_exposure_last_update = now;
}

void xavix_analog_image_sensor_device_base::finish_exposure()
{
	integrate_exposure();
	for (unsigned pixel = 0; pixel < m_image.size(); ++pixel)
		m_image[pixel] = std::clamp(int(m_exposure_sum[pixel] + 0.5), 0, 255);
	m_exposing = false;
}

TIMER_CALLBACK_MEMBER(xavix_analog_image_sensor_device_base::scan_tick)
{
	bool event = false;
	if (!BIT(m_status, 1))
	{
		if (m_exposing)
			finish_exposure();
		else
			capture_image();
		m_row = 0;
		m_column = 0;
		m_pixel = m_image[0];
		m_status = 0x02;
		event = m_event_source == event_source::FRAME_START || m_event_source == event_source::ROW_START;
	}
	else if (!BIT(m_status, 2))
	{
		// Hold each pixel across both clock phases, including the row's ADC-priming pulse.
		m_pixel = m_image[m_row * IMAGE_WIDTH + std::min<unsigned>(m_column, IMAGE_WIDTH - 1)];
		m_status |= 0x04;
	}
	else
	{
		m_status &= ~0x04;
		if (++m_column == IMAGE_WIDTH + 1)
		{
			m_column = 0;
			if (++m_row == IMAGE_HEIGHT)
			{
				if (!m_exposure_period.is_zero())
				{
					begin_exposure();
				}
				else
				{
					m_status = m_frame_gap_pixel_high ? 0x04 : 0;
					m_scan_timer->adjust(clocks_to_attotime(FRAME_GAP_CLOCKS));
				}
				if (m_event_source == event_source::FRAME_END)
					m_event_cb();
				return;
			}
			event = m_event_source == event_source::ROW_START;
		}
	}
	m_scan_timer->adjust(clocks_to_attotime(BIT(m_status, 2) ? m_pixel_high_clocks : m_pixel_low_clocks));
	if (event)
		m_event_cb();
}

//-------------------------------------------------
//  Target Rendering
//-------------------------------------------------

void xavix_analog_image_sensor_device::capture_image()
{
	m_image.fill(16);
	if (!illuminated())
		return;

	// Each target contributes independently; the ROM assigns tracked objects from the image, including overlaps.
	for (unsigned target = 0; target < m_target_count; ++target)
	{
		if (!m_target_hidden[target]->read())
			render_target(m_target_x[target]->read(), m_target_y[target]->read(), m_target_size[target]->read(),
					target ? 0 : m_target_angle->read(), target ? target_shape::DISK : m_shape);
	}
}

void xavix_image_sensor_device_base::render_target(u8 x_position, u8 y_position, u8 size, u8 angle, target_shape shape, u8 brightness)
{
	// Reflectors are blurred disks/rods; their optical dimensions are unmeasured.
	double const centre_x = double(x_position) * (IMAGE_WIDTH - 1) / 255;
	double const centre_y = double(y_position) * (IMAGE_HEIGHT - 1) / 255;
	double const radius = double(size) / 16;
	double const radians = double(angle) * (2 * std::acos(-1.0)) / 256;
	double const cosine = std::cos(radians);
	double const sine = std::sin(radians);
	double const width = shape == target_shape::ROD ? radius / 3 : radius;
	double const length = shape == target_shape::ROD ? radius * 3 : radius;
	for (unsigned y = 0; y < IMAGE_HEIGHT; ++y)
	{
		for (unsigned x = 0; x < IMAGE_WIDTH; ++x)
		{
			double const dx = x - centre_x;
			double const dy = y - centre_y;
			double const along = (cosine * dx + sine * dy) / length;
			double const across = (cosine * dy - sine * dx) / width;
			double const distance_squared = along * along + across * across;
			u8 &pixel = m_image[y * IMAGE_WIDTH + x];
			pixel = std::min(255, pixel + std::clamp(int(brightness * (1 - distance_squared / 2)), 0, int(brightness)));
		}
	}
}

//-------------------------------------------------
//  Digital Image Interface
//-------------------------------------------------

bool xavix_digital_image_sensor_device::validate_picture()
{
	if (!m_picture->exists())
		return false;

	bitmap_argb32 const &picture = m_picture->get_bitmap();
	if (!picture.valid() || picture.width() != int(IMAGE_WIDTH) || picture.height() != int(IMAGE_HEIGHT))
	{
		popmessage("Raw camera input rejected: use a 32x32 opaque monochrome picture.");
		return false;
	}

	for (unsigned y = 0; y < IMAGE_HEIGHT; ++y)
	{
		for (unsigned x = 0; x < IMAGE_WIDTH; ++x)
		{
			rgb_t const colour(picture.pix(y, x));
			if (colour.a() != 0xff || colour.r() != colour.g() || colour.r() != colour.b())
			{
				popmessage("Raw camera input rejected: use a 32x32 opaque monochrome picture.");
				return false;
			}
		}
	}
	return true;
}

void xavix_digital_image_sensor_device::capture_image()
{
	if (!m_picture->exists())
	{
		// Representative levels straddle the ROM's threshold; actual camera brightness is unmeasured.
		m_image.fill(0x3f);
		for (unsigned glove = 0; glove < m_glove_x.size(); ++glove)
		{
			if (!m_glove_hidden[glove]->read())
				render_target(m_glove_x[glove]->read(), m_glove_y[glove]->read(), m_glove_size[glove]->read(), 0, target_shape::DISK, 192);
		}
		return;
	}

	m_image.fill(0);
	if (!m_source_valid)
		return;

	bitmap_argb32 const &picture = m_picture->get_bitmap();
	for (unsigned y = 0; y < IMAGE_HEIGHT; ++y)
	{
		for (unsigned x = 0; x < IMAGE_WIDTH; ++x)
			m_image[y * IMAGE_WIDTH + x] = rgb_t(picture.pix(y, x)).r();
	}
}

u8 xavix_digital_image_sensor_device::data_r() const
{
	if (!m_read_active || !m_row_active || m_column >= IMAGE_WIDTH)
		return 0xff;

	return m_image[m_row * IMAGE_WIDTH + m_column];
}

void xavix_digital_image_sensor_device::data_w(offs_t, u8 data, u8 mem_mask)
{
	m_data = data;
	m_data_mask = mem_mask;
}

void xavix_digital_image_sensor_device::strobe_w(offs_t, u8 data, u8 mem_mask)
{
	u8 const previous = m_strobes;
	m_strobes = (data | ~mem_mask) & (WRITE_STROBE | READ_STROBE);

	if (!(previous & READ_STROBE) && (m_strobes & READ_STROBE) && m_read_active)
	{
		m_read_active = false;
		if (++m_column == IMAGE_WIDTH)
		{
			m_row_active = false;
			if (m_row == IMAGE_HEIGHT - 1)
				m_frame_active = false;
		}
	}

	if (!(previous & WRITE_STROBE) && (m_strobes & WRITE_STROBE) && (mem_mask & WRITE_STROBE))
	{
		if (m_data_mask == 0xff)
			command_w(m_data);
		else
			LOGMASKED(LOG_PROTOCOL, "%s: partially driven command %02x (mask %02x)\n", machine().describe_context(), m_data, m_data_mask);
	}

	if ((previous & READ_STROBE) && !(m_strobes & READ_STROBE) && m_row_active)
	{
		if (!m_frame_active && !m_row && !m_column)
		{
			// Provisional raw-image input boundary, not an established command-0x90 exposure latch.
			capture_image();
			m_frame_active = true;
		}
		m_read_active = true;
	}
}

void xavix_digital_image_sensor_device::command_w(u8 data)
{
	LOGMASKED(LOG_COMMAND, "%s: camera command %02x\n", machine().describe_context(), data);
	if (!(data & 0xc1))
	{
		m_row = BIT(data, 1, 5);
		m_column = 0;
		m_row_active = false;
		m_read_active = false;
		return;
	}

	switch (data)
	{
	case 0x40:
		m_column = 0;
		m_row_active = true;
		m_read_active = false;
		break;

	case 0xb5:
		m_setup[0] = data;
		break;
	case 0x81:
		m_setup[1] = data;
		break;
	case 0xa0:
		m_setup[2] = data;
		break;
	case 0xc4:
		m_setup[3] = data;
		break;

	case 0x90:
	case 0x94:
	case 0x95:
	case 0x98:
	case 0x99:
		m_control = data;
		break;

	default:
		LOGMASKED(LOG_UNHANDLED, "%s: unhandled camera command %02x\n", machine().describe_context(), data);
		break;
	}
}

//-------------------------------------------------
//  Serial Tracking Reports
//-------------------------------------------------

void xavix_serial_tracking_sensor_device::latch_report()
{
	// Visible blobs precede empty slots; the ROM's single-foot path only reads slot 0.
	u8 const hidden = m_target_hidden->read();
	m_report.fill(0);
	unsigned report = 0;
	for (unsigned target = 0; target != 2; ++target)
	{
		if (!BIT(hidden, target))
		{
			m_report[report++] = m_target_x[target]->read() & 0x3f;
			m_report[report++] = m_target_y[target]->read() & 0x3f;
			m_report[report++] = m_target_size[target]->read() & 0x3f;
		}
	}
}

void xavix_serial_tracking_sensor_device::clock_w(int state)
{
	bool const clock = bool(state);
	if (m_clock && !clock)
	{
		// A falling edge after the complete report latches the next frame. The
		// following 40 falling edges clock signature bits 1/0/1, six six-bit
		// sensor values (LSB first), then the final signature bit 1.
		if (m_bit_count >= 40)
		{
			latch_report();
			m_bit_count = 0;
		}
		else
		{
			unsigned const bit = m_bit_count++;
			if (bit < 3)
				m_data = BIT(0x05, bit);
			else if (bit == 39)
				m_data = 1;
			else
			{
				unsigned const data_bit = bit - 3;
				m_data = BIT(m_report[data_bit / 6], data_bit % 6);
			}
		}
	}

	m_clock = clock;
}
